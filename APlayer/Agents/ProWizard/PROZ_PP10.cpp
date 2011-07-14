/******************************************************************************/
/* ProWizard PP10 class.                                                      */
/*                                                                            */
/* ProPacker v1.0 format.                                                     */
/* Created by ???                                                             */
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
uint32 PROZ_PP10::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 i;
	uint32 j;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Call Cryptoburner check
	if (!(Check(mod, &mod[0xf8])))
		return (0);

	// The first two track numbers in list 2 may not be 0
	if ((mod[0x17a] == 0) || (mod[0x17b] == 0))
		return (0);

	// Find the heighest track number
	trackNum = 0;
	for (i = 0; i < 512; i++)
	{
		if (mod[0xfa + i] > trackNum)
			trackNum = mod[0xfa + i];
	}

	trackNum = (trackNum + 1) * 64;

	// Check notes in the tracks
	for (j = 0; j < trackNum; j++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x2fa + j * 4]));
		if (temp != 0)
		{
			if ((temp < 0x71) || (temp > 0x1358))
				return (0);
		}
	}

	// Check the module length
	if ((0x2fa + trackNum * 4 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Clear the track table
	memset(trackTable, 0, sizeof(trackTable));

	// Find number of positions
	posNum = mod[0xf8];

	// Build the track table
	for (i = 0; i < posNum; i++)
	{
		// Copy the track numbers
		trackTable[i * 4]     = mod[0xfa + i];
		trackTable[i * 4 + 1] = mod[0x17a + i];
		trackTable[i * 4 + 2] = mod[0x1fa + i];
		trackTable[i * 4 + 3] = mod[0x27a + i];
	}

	// Find number of patterns to build
	Speco();

	if (pattNum > 0x40)
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
ap_result PROZ_PP10::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 trackOffset;
	int8 lastPatt;
	uint16 i, j, k;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Write sample name
		destFile->Write(zeroBuf, 22);

		// Write the rest of the information
		destFile->Write(&mod[i * 8], 8);
	}

	// Write number of positions
	destFile->Write_UINT8(mod[0xf8]);

	// Write NTK byte
	destFile->Write_UINT8(mod[0xf9]);

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

			// Clear the pattern data
			memset(pattern, 0, sizeof(pattern));

			for (j = 0; j < 4; j++)
			{
				// Get the track offset
				trackOffset = 0x2fa + trackTable[i * 4 + j] * 256;

				// Copy the track
				for (k = 0; k < 64; k++)
				{
					pattern[j * 4 + k * 16]     = mod[trackOffset + k * 4];
					pattern[j * 4 + k * 16 + 1] = mod[trackOffset + k * 4 + 1];
					pattern[j * 4 + k * 16 + 2] = mod[trackOffset + k * 4 + 2];
					pattern[j * 4 + k * 16 + 3] = mod[trackOffset + k * 4 + 3];
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[0x2fa + trackNum * 4], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PP10::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PP10);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/******************************************************************************/
void PROZ_PP10::Speco(void)
{
	uint16 i, j;
	const uint8 *posTab1, *posTab2;
	uint8 temp1, temp2, temp3, temp4;
	bool found;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = &trackTable[4];
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = *posTab1++;
		temp2 = *posTab1++;
		temp3 = *posTab1++;
		temp4 = *posTab1++;

		// Compare these offsets with the rest to see if there is an equal one
		posTab2 = trackTable;
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
