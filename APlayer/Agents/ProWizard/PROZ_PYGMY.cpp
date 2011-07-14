/******************************************************************************/
/* ProWizard PYGMY class.                                                     */
/*                                                                            */
/* Pygmy Packer format.                                                       */
/* Created by Flame / Pygmy Projects (19xx)                                   */
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
/* Global variables                                                           */
/******************************************************************************/
extern int16 tuning[16][36];



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a known module.      */
/*                                                                            */
/* Input:  "module" is a reference to where the packed module is stored.      */
/*                                                                            */
/* Output: Is the unpacked module size or 0 if not recognized.                */
/******************************************************************************/
uint32 PROZ_PYGMY::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp, temp1;
	uint16 temp2;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the last values in the period table
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x2a0])) != 0x00780071)
		return (0);

	// Check sample addresses
	for (i = 0; i < 30; i++)
	{
		// Get start address
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16]));
		if (temp == 0)
			return (0);

		// Get the next sample start address
		temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[(i + 1) * 16]));

		// The next start address has to be lower than the previous one
		if (temp1 < temp)
			return (0);
	}

	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) == temp1)
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample length
		temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 4]));

		if (temp2 != 0)
		{
			sampSize += (temp2 * 2);

			// Check repeat
			temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16 + 6])) - P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16]));
			temp = (temp / 2) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 10]));
			if (temp > temp2)
				return (0);
		}

		// Check volume & finetune
		if ((mod[i * 16 + 13] > 0x40) || (mod[i * 16 + 12] > 0x0f))
			return (0);
	}

	// Check sample size
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1f0])) != sampSize)
		return (0);

	// Check period table
	for (i = 0; i < 36; i++)
	{
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x25c + i * 2])) != tuning[0][i])
			return (0);
	}

	// Check offset to patterns
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) - P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x59c]));
	if ((temp == 0) || (temp & 0x1))
		return (0);

	// Get number of patterns
	pattNum = temp / 1024;
	if (pattNum > 64)
		return (0);

	// Check the module length
	if ((pattNum * 1024 + sampSize + 128) > module.GetLength())
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
ap_result PROZ_PYGMY::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 posTab[128];
	uint8 pattern[1024];
	uint8 byte1, byte2, byte3, byte4;
	uint8 temp;
	uint32 offset;
	uint16 i, j, k;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));
	memset(posTab, 0, sizeof(posTab));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Find first real sample
	j = 0;
	while (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[j * 16 + 4])) == 0)
	{
		j++;
	}

	// Write sample informations
	for (i = 0; i < (31 - j); i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size
		destFile->Write(&mod[(i + j) * 16 + 4], 2);

		// Finetune + volume
		destFile->Write(&mod[(i + j) * 16 + 12], 2);

		// Repeat
		destFile->Write_B_UINT16((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[(i + j) * 16 + 6])) - P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[(i + j) * 16]))) / 2);

		// Replen
		destFile->Write(&mod[(i + j) * 16 + 10], 2);
	}

	// Write empty samples
	for (; i < 31; i++)
	{
		// Write sample name + size + finetune + volume + repeat point
		destFile->Write(zeroBuf, 28);

		// Write repeat length
		destFile->Write_B_UINT16(1);
	}

	// Create position table
	offset = 0x636 + pattNum * 1024 + sampSize;

	for (i = 0; i < 128; i++)
	{
		if (mod[offset] == 0xff)
			break;

		posTab[i] = mod[offset++];
	}

	// Write the size of the position table
	destFile->Write_UINT8(i);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write the position table
	destFile->Write(posTab, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert patterns
	offset = 0x636;
	for (i = 0; i < pattNum; i++)
	{
		// Loop the voices
		for (j = 0; j < 4; j++)
		{
			// Loop the rows
			for (k = 64; k > 0; k--)
			{
				// Get note
				byte1 = mod[offset++];
				if (byte1 != 0)
				{
					if (byte1 >= 0xb8)
						byte1 = -(int8)byte1;

					byte1 -= 2;
					byte1 /= 2;
					byte2 = period[byte1][1];
					byte1 = period[byte1][0];
				}
				else
					byte2 = 0x00;

				// Get sample
				byte3 = mod[offset++];
				if (byte3 & 0x80)
					byte1 |= 0x10;

				byte3 <<= 1;

				// Get effect and value
				temp   = mod[offset++];
				byte4  = mod[offset++];
				byte3 |= ConvertEffect(temp, byte4);

				// Store the pattern bytes
				pattern[(k - 1) * 16 + j * 4]     = byte1;
				pattern[(k - 1) * 16 + j * 4 + 1] = byte2;
				pattern[(k - 1) * 16 + j * 4 + 2] = byte3;
				pattern[(k - 1) * 16 + j * 4 + 3] = byte4;
			}
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[0x636 + pattNum * 1024], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PYGMY::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PYGMY);
}



/******************************************************************************/
/* ConvertEffect() will convert the effect number to a ProTracker number.     */
/*                                                                            */
/* Input:  "eff" is the Pygmy effect number.                                  */
/*         "effVal" is a reference to the effect value.                       */
/*                                                                            */
/* Output: The ProTracker effect number.                                      */
/******************************************************************************/
uint8 PROZ_PYGMY::ConvertEffect(uint8 eff, uint8 &effVal)
{
	switch (eff)
	{
		// Arpeggio
		case 0x00:
		case 0x10:
			return (0x00);

		// Portamento up
		case 0x14:
			return (0x01);

		// Portamento down
		case 0x18:
			return (0x02);

		// Tone portamento
		case 0x04:
			return (0x03);

		// Vibrato
		case 0x24:
		{
			// Swap the value
			effVal = ((effVal & 0xf0) >> 4) | ((effVal & 0x0f) << 4);
			return (0x04);
		}

		// Tone + vol
		case 0x0c:
			return (0x05);

		// Vibrato + vol
		case 0x2c:
			return (0x06);

		// Vol slide up
		case 0x1c:
		{
			effVal <<= 4;
			return (0x0a);
		}

		// Vol slide down
		case 0x20:
			return (0x0a);

		// Set volume
		case 0x34:
			return (0x0c);

		// Pattern break
		case 0x38:
			return (0x0d);

		// Extra effect
		case 0x3c:
		{
			if (effVal == 0x02)
				effVal = 0x01;

			return (0x0e);
		}

		// Set speed
		case 0x40:
			return (0x0f);
	}

	// Well, unknown effect, clear it
	effVal = 0x00;
	return (0x00);
}
