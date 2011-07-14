/******************************************************************************/
/* ProWizard TP1 class.                                                       */
/*                                                                            */
/* Tracker Packer v1 format.                                                  */
/* Created by Crazy Crack / MEXX                                              */
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
uint32 PROZ_TP1::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp;
	uint16 temp1;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Start to check id
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x0])) != 'MEXX')
		return (0);

	// Check module length stored in module
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4]));
	if ((temp == 0) || (temp & 0x1))
		return (0);

	// Check the module length
	if (temp > (module.GetLength() + 256))
		return (0);

	// Get the sample offset
	samples = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1c]));
	if ((samples == 0) || (samples & 0x1))
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x20 + i * 8 + 2]));
		if (temp1 == 0)
		{
			// Check replen
			if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x20 + i * 8 + 6])) != 0x0001)
				return (0);
		}
		else
		{
			sampSize += (temp1 * 2);

			// Check volume
			if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x20 + i * 8])) > 0x40)
				return (0);
		}
	}

	// Check size of pattern list
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x118])) + 1;
	if (posNum == 0)
		return (0);

	// Find number of patterns to build
	Speco(mod);

	if (pattNum > 64)
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
ap_result PROZ_TP1::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	bool taken[128];
	uint16 patternOffset;
	uint8 temp;
	uint16 i, j;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	for (i = 0; i < 128; i++)
		taken[i] = false;

	// Write the module name
	destFile->Write(&mod[8], 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size
		destFile->Write(&mod[0x20 + i * 8 + 2], 2);

		// Write volume
		destFile->Write(&mod[0x20 + i * 8], 2);

		// Write loop values
		destFile->Write(&mod[0x20 + i * 8 + 4], 4);
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
	for (i = 0; i < posNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Have we already taken this position (have the same
		// pattern as a previous position)?
		if (taken[i])
			continue;

		// Get the pattern offset
		patternOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x11c + i * 4]));
		if (patternOffset == 0)
			continue;		// No pattern, skip it

		// Check to see if this pattern is somewhere else. If so, mark it
		// as taken
		for (j = i; j < posNum; j++)
		{
			if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x11c + j * 4])) == patternOffset)
				taken[j] = true;
		}

		// Loop the pattern
		for (j = 0; j < (64 * 4); j++)
		{
			// Get first byte
			temp = mod[patternOffset++];

			// Is it an empty note?
			if (temp == 0xc0)
				continue;

			// Effect only?
			if (temp >= 0x80)
			{
				// Yup
				pattern[j * 4 + 2] = (temp >> 2) & 0x0f;
				pattern[j * 4 + 3] = mod[patternOffset++];
				continue;
			}

			// Note + sample + effect
			if (temp & 0x01)
			{
				pattern[j * 4] = 0x10;	// Hi bit in sample number
				temp &= 0xfe;
			}

			if (temp != 0)
			{
				// Got a note, convert it to a period
				temp -= 2;
				temp /= 2;
				pattern[j * 4]    |= period[temp][0];
				pattern[j * 4 + 1] = period[temp][1];
			}

			// Store sample number + effect + effect value
			pattern[j * 4 + 2] = mod[patternOffset++];
			pattern[j * 4 + 3] = mod[patternOffset++];
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[samples], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_TP1::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_TP1);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_TP1::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint16 *posTab1, *posTab2;
	uint16 temp1;
	bool found;

	// Jump to the track offset table
	mod += 0x11c;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = (uint16 *)mod + 2;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offset
		temp1 = P_BENDIAN_TO_HOST_INT16(*posTab1);
		posTab1 += 2;

		// Compare the offset with the rest to see if there is an equal one
		posTab2 = (uint16 *)mod;
		found   = false;

		for (j = 0; j < i; j++)
		{
			if (temp1 == P_BENDIAN_TO_HOST_INT16(posTab2[0]))
			{
				// Found an equal track joinment
				posTable[i] = posTable[j];
				found       = true;
				break;
			}

			// Go to the next offset pair
			posTab2 += 2;
		}

		if (!found)
			posTable[i] = ++pattNum;
	}

	// Add one extra pattern number, because we skipped the first one
	pattNum++;
}
