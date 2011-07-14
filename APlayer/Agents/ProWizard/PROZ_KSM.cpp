/******************************************************************************/
/* ProWizard KSM class.                                                       */
/*                                                                            */
/* Kefrens Sound Machine format.                                              */
/* Created by Razmo / Kefrens (1989)                                          */
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
uint32 PROZ_KSM::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp;
	uint16 temp1;
	uint32 offset;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check mark
	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) != 'M.')
		return (0);

	// Check sample informations
	sampSize = 0;
	offset   = 0;

	for (i = 0; i < 15; i++)
	{
		// Check offset to sample
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[32 + i * 32 + 16]));
		if ((temp == 0) || (temp <= offset))
			return (0);

		offset = temp;

		// Check sample size
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[32 + i * 32 + 20]));
		if ((temp1 == 0) || (temp1 & 0x1))
			return (0);

		sampSize += temp1;

		// Check volume
		if (mod[32 + i * 32 + 22] > 0x40)
			return (0);
	}

	// Check for "too small" file
	if (module.GetLength() < 0x600)
		return (0);

	// Find number of positions
	posNum = 0;
	offset = 0x200;

	for (;;)
	{
		// Did we reach the end of file?
		if (offset > 0x600)
			return (0);

		// Found the end of the position table?
		if (mod[offset] == 0xff)
			break;

		posNum++;
		offset += 4;
	}

	// The first part of the track data should be zero'ed. Check it
	temp = 0;
	for (i = 0; i < 48; i++)
		temp += *((uint32 *)&mod[0x600 + i * 4]);

	if (temp != 0)
		return (0);

	// Check notes in the track
	for (i = 0; i < 48; i++)
	{
		if (mod[0x6c0 + i * 3] > 0x24)
			return (0);
	}

	// Check the module length
	if ((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x30])) + sampSize) > (module.GetLength() + 256))
		return (0);

	// Create position table
	Speco(mod);

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
ap_result PROZ_KSM::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	int8 patt, lastPatt;
	const uint8 *trackPoi;
	uint16 i, j, k;
	uint16 temp, temp1;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(&mod[2], 13);
	destFile->Write(zeroBuf, 7);

	// Write sample informations
	for (i = 0; i < 15; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[32 + i * 32 + 20])) / 2;
		destFile->Write_B_UINT16(temp);

		// Finetune
		destFile->Write_UINT8(0);

		// Volume
		destFile->Write_UINT8(mod[32 + i * 32 + 22]);

		// Repeat start
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[32 + i * 32 + 24]));
		if (temp1 != 0)
		{
			// Loop
			temp1 /= 2;
			destFile->Write_B_UINT16(temp1);
			destFile->Write_B_UINT16(temp - temp1);
		}
		else
		{
			// No loop
			destFile->Write_B_UINT16(0);
			destFile->Write_B_UINT16(1);
		}
	}

	// Write empty samples
	for (; i < 31; i++)
	{
		// Write sample name + size + volume + repeat point
		destFile->Write(zeroBuf, 28);

		// Write repeat length
		destFile->Write_B_UINT16(1);
	}

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x00);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Begin to convert the patterns
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		// Get pattern number to build
		patt = posTable[i];
		if (patt <= lastPatt)
			continue;

		lastPatt++;

		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Loop voices
		for (j = 0; j < 4; j++)
		{
			// Get track offset
			trackPoi = &mod[0x600 + mod[0x200 + i * 4 + j] * 192];

			// Loop rows
			for (k = 0; k < 64; k++)
			{
				// Get note
				temp = *trackPoi++;
				if (temp != 0)
				{
					// There is a note, convert it to a period
					temp--;
					pattern[k * 16 + j * 4]     = period[temp][0];
					pattern[k * 16 + j * 4 + 1] = period[temp][1];
				}

				// Copy sample and effect
				pattern[k * 16 + j * 4 + 2] = *trackPoi++;
				pattern[k * 16 + j * 4 + 3] = *trackPoi++;

				// Convert effect D to A
				if ((pattern[k * 16 + j * 4 + 2] & 0x0f) == 0x0d)
				{
					pattern[k * 16 + j * 4 + 2] &= 0xf0;
					pattern[k * 16 + j * 4 + 2] |= 0x0a;
				}
			}
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x30]))], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_KSM::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_KSM);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_KSM::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint8 *posTab1, *posTab2;
	uint8 temp1, temp2, temp3, temp4;
	bool found;

	// Jump to the track offset table
	mod += 0x200;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = mod + 4;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = *posTab1++;
		temp2 = *posTab1++;
		temp3 = *posTab1++;
		temp4 = *posTab1++;

		// Compare these offsets with the rest to see if there is an equal one
		posTab2 = mod;
		found   = false;

		for (j = 0; j < i; j++)
		{
			if ((temp1 == posTab2[0]) && (temp2 == posTab2[1]) && (temp3 == posTab2[2]) && (temp4 == posTab2[3]))
			{
				// Found an equal track joinment
				posTable[i] = posTable[j];
				found       = true;
				break;
			}

			// Go to the next offset pair
			posTab2 += 4;
		}

		if (!found)
			posTable[i] = ++pattNum;
	}

	// Add one extra pattern number, because we skipped the first one
	pattNum++;
}
