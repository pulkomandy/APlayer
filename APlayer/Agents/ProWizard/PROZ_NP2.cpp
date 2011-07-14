/******************************************************************************/
/* ProWizard NP2 class.                                                       */
/*                                                                            */
/* Noise Packer v1 - v2 format.                                               */
/* Created by Twins / Phenomena (1990)                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PFile.h"
#include "PBinary.h"

// Agent headers
#include "ProWizard.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* CheckModule() will be check the module to see if it's a known module.      */
/*                                                                            */
/* Input:  "module" is a reference to where the packed module is stored.      */
/*                                                                            */
/* Output: Is the unpacked module size or 0 if not recognized.                */
/******************************************************************************/
uint32 PROZ_NP2::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 offset;
	uint16 temp;
	uint32 temp1;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the extra 'C' in number of samples
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0]));
	if ((temp & 0xf00f) != 0x000c)
		return (0);

	// Check the number of samples
	temp &= 0x0ff0;
	if ((temp == 0) || (temp > 0x01f0))
		return (0);

	sampNum = temp >> 4;

	// Check number of positions
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + sampNum * 16]));
	if ((temp == 0) || (temp > 0xfe) || (temp & 0x01))
		return (0);

	// The "number of positions" is stored twice, check it
	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2])) != temp)
		return (0);

	// Check number of tracks
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[4]));
	if ((temp == 0) || (temp & 0x01))
		return (0);

	// Check sample offset
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[6]));
	if ((temp == 0) || (temp & 0x01) || (temp < 0x00c0))
		return (0);

	// So far, we guess it's a NP2
	mark = 'NP20';

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < sampNum; i++)
	{
		// Check pointer to sample
		temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[8 + i * 16]));
		if (temp1 > 0xfe000)
			return (0);

		// Check loop pointer
		temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[8 + i * 16 + 8]));
		if (temp1 > 0xfe000)
			return (0);

		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 16 + 4]));
		if (temp > 0x8000)
			return (0);

		sampSize += (temp * 2);

		// No empty samples are written in this format. Check it
		if (temp == 0)
			return (0);

		// Check to see if we have a NP1 module
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 16 + 12])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 16 + 14]));
		if (temp1 > temp)
		{
			// Well, it's definately not a NP2
			temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 16 + 12])) + (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 16 + 14])) / 2);
			if (temp1 > temp)
				return (0);

			// It's a NP1
			mark = 'NP10';
		}

		// Check volume & finetune
		if ((mod[8 + i * 16 + 7] > 0x40) || (mod[8 + i * 16 + 6] > 0x0f))
			return (0);
	}

	// Check position table and find heigest pattern number
	offset  = 8 + sampNum * 16 + 4;
	pattNum = 0;
	posNum  = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset - 4])) / 2;

	for (i = 0; i < posNum; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));
		if (((temp / 8) * 8) != temp)
			return (0);

		if (temp > pattNum)
			pattNum = temp;

		offset += 2;
	}

	pattNum = pattNum / 8 + 1;

	// Check the module length
	temp1  = 8 + sampNum * 16 + 4;
	temp1 += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2]));
	temp1 += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[4]));
	temp1 += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[6]));
	temp1 += sampSize;
	if (temp1 > (module.GetLength() + 256))
		return (0);

	// Calculate the total size of the PTK module
	calcSize = pattNum * 1024 + sampSize + 1084;

	return (calcSize);
}



/******************************************************************************/
/* ConvertModule() will convert the module to ProTracker format.              */
/*                                                                            */
/* Input:  "module" is a reference to the packed module.                      */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result PROZ_NP2::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[128];
	uint16 pattern[512];
	uint16 trackNum;
	uint16 temp;
	uint16 i, j, k;
	const uint8 *addyPos, *addyTracks, *addyNotes, *addySamples;
	const uint8 *notePoi;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < sampNum; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size + finetune + volume
		destFile->Write(&mod[8 + i * 16 + 4], 4);

		// Repeat point
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 16 + 14]));
		if (mark == 'NP10')
			temp /= 2;

		destFile->Write_B_UINT16(temp);

		// Replen
		destFile->Write(&mod[8 + i * 16 + 12], 2);
	}

	// Write empty samples
	for (; i < 31; i++)
	{
		// Write sample name + size + volume + repeat point
		destFile->Write(zeroBuf, 28);

		// Write repeat length
		destFile->Write_B_UINT16(1);
	}

	// Get the size of the position table
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2]));

	// Get the number of tracks
	trackNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[4]));

	// Find pointers to the different tables in the module
	addyPos    = &mod[8 + sampNum * 16 + 4];
	addyTracks = addyPos + posNum;
	addyNotes  = addyTracks + trackNum;

	// Write position table length
	destFile->Write_UINT8(posNum / 2);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	for (i = 0; i < (posNum / 2); i++)
		destFile->Write_UINT8(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&addyPos[i * 2])) / 8);

	destFile->Write(zeroBuf, 128 - i);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert and write the pattern data
	addySamples = NULL;
	for (i = 0; i < pattNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Loop voices
		for (j = 0; j < 4; j++)
		{
			// Find pointer to the track to copy
			notePoi = &addyNotes[P_BENDIAN_TO_HOST_INT16(*((uint16 *)addyTracks))];
			addyTracks += 2;

			// Loop rows
			for (k = 0; k < 64; k++)
			{
				uint8 dat1, dat2, dat3;

				// Get pattern data
				dat1 = *notePoi++;
				dat2 = *notePoi++;
				dat3 = *notePoi++;

				// Convert the pattern data to PTK
				BuildVal1(dat1);
				BuildVal2(dat2, dat3);

				// Store the converted values
				pattern[k * 8 + (3 - j) * 2]     = P_HOST_TO_BENDIAN_INT16(val1);
				pattern[k * 8 + (3 - j) * 2 + 1] = P_HOST_TO_BENDIAN_INT16(val2);
			}

			// Did we reach the heighest address -> sample address?
			if (notePoi > addySamples)
				addySamples = notePoi;
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(addySamples, sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_NP2::GetModuleType(void)
{
	return (mark == 'NP10' ? IDS_PROZ_TYPE_NP1 : IDS_PROZ_TYPE_NP2);
}



/******************************************************************************/
/* BuildVal1() will build the first 16 bit of the pattern data.               */
/*                                                                            */
/* Input:  "dat1" is the first byte in the NP pattern data.                   */
/******************************************************************************/
void PROZ_NP2::BuildVal1(uint8 dat1)
{
	uint8 note;

	note = dat1 & 0xfe;
	if (note == dat1)
		val1 = 0;
	else
		val1 = 0x1000;

	// Is there any note?
	if (dat1 > 0x01)
	{
		// Yup, find the period and "or" it together with the other bits
		note -= 2;
		note /= 2;
		val1 |= ((period[note][0] << 8) | (period[note][1]));
	}
}



/******************************************************************************/
/* BuildVal2() will build the last 16 bit of the pattern data.                */
/*                                                                            */
/* Input:  "dat2" is the second byte in the NP pattern data.                  */
/*         "dat3" is the third byte in the NP pattern data.                   */
/******************************************************************************/
void PROZ_NP2::BuildVal2(uint8 dat2, uint8 dat3)
{
	// Start to store the sample number
	val2 = (dat2 & 0xf0) << 8;

	// Extract the effect
	dat2 &= 0x0f;

	// Should the effect be converted?
	switch (dat2)
	{
		// Pattern break
		case 0xb:
		{
			dat3 += 2;
			dat3 /= 2;
			break;
		}

		// Volume slide
		case 0x7:
		{
			dat2 = 0xa;

			// Falling through
		}

		// (Vibrato + volume slide) + (portamento + volume slide)
		case 0x6:
		case 0x5:
		{
			if ((int8)dat3 < 0)
				dat3 = -(int8)dat3;
			else
				dat3 <<= 4;

			break;
		}

		// Arpeggio
		case 0x8:
		{
			dat2 = 0x0;
			break;
		}

		// Filter
		case 0xe:
		{
			if (((dat3 & 0xf0) == 0x00) && (dat3 != 0x00))
				dat3 = 0x01;

			break;
		}
	}

	// Merge the effect with the sample number
	val2 |= ((dat2 << 8) | dat3);
}
