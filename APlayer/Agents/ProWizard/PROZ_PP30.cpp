/******************************************************************************/
/* ProWizard PP30 class.                                                      */
/*                                                                            */
/* ProPacker v2.1 format.                                                     */
/* Created by C Estrup / Static Bytes (1991)                                  */
/*                                                                            */
/* ProPacker v3.0 format.                                                     */
/* Created by C Estrup / Static Bytes (1994)                                  */
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
uint32 PROZ_PP30::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint32 temp1;
	uint32 trackOffset;
	uint16 count;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check first value in track "data"
	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x2fa])) != 0)
		return (0);

	// Call Cryptoburner check
	if (!(Check(mod, &mod[0xf8])))
		return (0);

	// Find the heighest track number
	trackNum = 0;
	for (i = 0; i < 512; i++)
	{
		if (mod[0xfa + i] > trackNum)
			trackNum = mod[0xfa + i];
	}

	trackNum = (trackNum + 1) * 64 * 2;

	// Check the module length
	trackOffset = 0x2fa + trackNum;
	temp1       = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[trackOffset]));
	temp1      += 4 + sampSize;

	if (temp1 & 0x1)
		return (0);

	if (temp1 > (module.GetLength() + 256))
		return (0);

	// Get reference table size
	temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[trackOffset]));
	if (temp1 == 0)
		return (0);

	temp1       /= 4;
	trackOffset += 4;

	// Check notes in the reference table
	if (temp1 > (64 * 2))
		temp1 = 64 * 2;

	count = 0;
	for (i = 0; i < temp1; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[trackOffset + i * 4]));
		if (temp != 0)
		{
			if ((temp < 0x71) || (temp > 0x1358))
				return (0);

			count++;
		}
	}

	if (count == 0)
		return (0);

	// Check the track data and find out which version of ProPacker it is
	temp1 = (trackNum / 128) * 64;
	if (temp1 > (64 * 4))
		temp1 = 64 * 4;

	count = 0;
	for (i = 0; i < temp1; i++)
	{
		// Get the reference value
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x2fa + i * 2]));
		if (temp >= 0x1800)
			return (0);

		if (temp != ((temp / 4) * 4))
			count++;
	}

	if (count != 0)
		type = 1;		// 2.1
	else
		type = 4;		// 3.0

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
ap_result PROZ_PP30::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 trackOffset, refOffset, refStart;
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
	refStart = 0x2fa + trackNum + 4;
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
				trackOffset = 0x2fa + trackTable[i * 4 + j] * 128;

				// Copy the track
				for (k = 0; k < 64; k++)
				{
					// Get the reference offset
					refOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[trackOffset + k * 2]));
					if (type == 1)
						refOffset *= 4;		// 2.1

					refOffset += refStart;

					// Copy the pattern data
					pattern[j * 4 + k * 16]     = mod[refOffset];
					pattern[j * 4 + k * 16 + 1] = mod[refOffset + 1];
					pattern[j * 4 + k * 16 + 2] = mod[refOffset + 2];
					pattern[j * 4 + k * 16 + 3] = mod[refOffset + 3];
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x2fa + trackNum])) + refStart], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PP30::GetModuleType(void)
{
	return (type == 1 ? IDS_PROZ_TYPE_PP21 : IDS_PROZ_TYPE_PP30);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/******************************************************************************/
void PROZ_PP30::Speco(void)
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
