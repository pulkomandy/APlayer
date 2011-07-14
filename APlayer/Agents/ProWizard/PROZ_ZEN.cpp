/******************************************************************************/
/* ProWizard ZEN class.                                                       */
/*                                                                            */
/* Zen Packer format.                                                         */
/* Created by Dweezil / Stellar (1992)                                        */
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
uint32 PROZ_ZEN::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp, temp2;
	uint16 temp1;
	uint8 lineNum;
	uint16 i;
	uint32 baseOffset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the pattern offset
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]));
	if ((temp & 0x1) || (temp > 0x200000))
		return (0);

	// Check number of patterns
	if (mod[4] > 0x3f)
		return (0);

	pattNum = mod[4] + 1;

	// Check number of positions
	posNum = mod[5];
	if (posNum > 0x7f)
		return (0);

	// Check the first sample pointer
	temp = posNum * 4 + 4 + P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]));
	if (temp != P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[14])))
		return (0);

	// Check sample informations
	sampSize = 0;
	temp2    = 0;
	for (i = 0; i < 31; i++)
	{
		// Check finetune
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[6 + i * 16]));
		if (temp1 != 0)
		{
			if (temp1 != ((temp1 / 0x48) * 0x48))
				return (0);
		}

		// Check volume
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[6 + i * 16 + 2]));
		if (temp1 > 0x40)
			return (0);

		// Check sample size
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[6 + i * 16 + 4]));
		if (temp1 >= 0x8000)
			return (0);

		if (temp1 == 0x0000)
		{
			// Check replen
			temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[6 + i * 16 + 6]));
			if (temp1 != 0x0001)
				return (0);
		}
		else
			sampSize += (temp1 * 2);

		// Check addresses
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[6 + i * 16 + 8]));

		if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[6 + i * 16 + 12])) < temp)
			return (0);

		if (temp2 > temp)
			return (0);

		temp2 = temp;
	}

	// Find 0xffffffff
	temp2 = 0x1f6;
	while (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[temp2])) != 0xffffffff)
	{
		temp2 += 2;
		if (temp2 > module.GetLength())
			return (0);
	}

	// Found the end mark, check the last pattern line
	tableOffset = temp2 - posNum * 4;
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[tableOffset - 4]));
	if (temp != 0xff000000)
		return (0);

	// Check the module length
	if ((temp2 + 4 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Find base offset
	baseOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[tableOffset]));

	for (i = 1; i < posNum; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[tableOffset + i * 4]));
		if (temp < baseOffset)
			baseOffset = temp;
	}

	baseOffset -= 0x1f6;

	// Build new pattern offsets
	for (i = 0; i < posNum; i++)
		newPattOffset[i] = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[tableOffset + i * 4])) - baseOffset;

	// Build new sample addresses
	for (i = 0; i < 31; i++)
	{
		newSampAddr[i]     = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[6 + i * 16 + 8])) - baseOffset;
		newSampLoopAddr[i] = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[6 + i * 16 + 12])) - baseOffset;
	}

	// Find number of patterns to build
	temp1 = pattNum;
	Speco();

	if (pattNum != temp1)
		return (0);

	// Check position numbers in first pattern
	lineNum = mod[0x1f6];
	if (lineNum != 0xff)
	{
		temp1 = 0x1f6 + 4;
		while (mod[temp1] != 0xff)
		{
			if (mod[temp1] <= lineNum)
				return (0);

			lineNum = mod[temp1];
			temp1  += 4;
		}
	}

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
ap_result PROZ_ZEN::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 pattOffset, line;
	int8 lastPatt;
	uint8 temp;
	uint16 i;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size
		destFile->Write(&mod[6 + i * 16 + 4], 2);

		// Write finetune
		destFile->Write_UINT8(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[6 + i * 16])) / 0x48);

		// Write volume
		destFile->Write_UINT8(mod[6 + i * 16 + 3]);

		// Write repeat start
		destFile->Write_B_UINT16((newSampLoopAddr[i] - newSampAddr[i]) / 2);

		// Write replen
		destFile->Write(&mod[6 + i * 16 + 6], 2);
	}

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert the pattern data
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		if (posTable[i] > lastPatt)
		{
			lastPatt++;

			// Get pattern offset
			pattOffset = newPattOffset[i];

			// Clear the pattern data
			memset(pattern, 0, sizeof(pattern));

			while ((line = mod[pattOffset++]) != 0xff)
			{
				line *= 4;

				// Get note number
				temp = mod[pattOffset++];

				// Is hi bit set in sample number?
				if (temp & 0x1)
				{
					pattern[line] = 0x10;
					temp &= 0xfe;
				}

				if (temp != 0)
				{
					temp -= 2;
					temp /= 2;
					pattern[line]    |= period[temp][0];
					pattern[line + 1] = period[temp][1];
				}

				// Copy sample number + effect + effect value
				pattern[line + 2] = mod[pattOffset++];
				pattern[line + 3] = mod[pattOffset++];
			}

			// Do it one more time for the last note
			//
			// Get note number
			temp = mod[pattOffset++];

			// Is hi bit set in sample number?
			if (temp & 0x1)
			{
				pattern[0xff * 4] = 0x10;
				temp &= 0xfe;
			}

			if (temp != 0)
			{
				temp -= 2;
				temp /= 2;
				pattern[0xff * 4]    |= period[temp][0];
				pattern[0xff * 4 + 1] = period[temp][1];
			}

			// Copy sample number + effect + effect value
			pattern[0xff * 4 + 2] = mod[pattOffset++];
			pattern[0xff * 4 + 3] = mod[pattOffset++];

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[newSampAddr[0]], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_ZEN::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_ZEN);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/******************************************************************************/
void PROZ_ZEN::Speco(void)
{
	uint16 i, j;
	const uint32 *posTab1, *posTab2;
	uint32 temp1;
	bool found;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = &newPattOffset[1];
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offset
		temp1 = *posTab1;
		posTab1++;

		// Compare the offset with the rest to see if there is an equal one
		posTab2 = &newPattOffset[0];
		found   = false;

		for (j = 0; j < i; j++)
		{
			if (temp1 == posTab2[0])
			{
				// Found an equal track joinment
				posTable[i] = posTable[j];
				found       = true;
				break;
			}

			// Go to the next offset pair
			posTab2++;
		}

		if (!found)
			posTable[i] = ++pattNum;
	}

	// Add one extra pattern number, because we skipped the first one
	pattNum++;
}
