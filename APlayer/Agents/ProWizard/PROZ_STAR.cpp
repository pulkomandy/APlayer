/******************************************************************************/
/* ProWizard STAR class.                                                      */
/*                                                                            */
/* StarTrekker Packer format.                                                 */
/* Created by Mr. Spiv / Cave (1991)                                          */
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
uint32 PROZ_STAR::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint32 temp1;
	uint32 offset;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the loop start and length on the last 2 samples
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x100])) != 0x00000001)
		return (0);

	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x108])) != 0x00000001)
		return (0);

	// Check the size, finetune and volume on the last 2 samples
	if ((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xfc])) + P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x104]))) != 0)
		return (0);

	// Check the size of the position table
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x10c]));
	if ((temp == 0) || (temp & 0x3))
		return (0);

	// Check module name
	for (i = 0; i < 20; i++)
	{
		if ((mod[i] != 0x00) && (mod[i] < 0x20))
			return (0);
	}

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 8]));
		if (temp >= 0x8000)
			return (0);

		// Check volume & finetune
		if ((mod[20 + i * 8 + 3] > 0x40) || (mod[20 + i * 8 + 2] > 0x0f))
			return (0);

		if (temp == 0)
		{
			// Check loop and volume
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 8 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 8 + 6])) + mod[20 + i * 8 + 3]) != 0x0001)
				return (0);
		}
		else
		{
			// Check loop
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 8 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 8 + 6]))) > temp)
				return (0);

			sampSize += (temp * 2);
		}
	}

	// Get number of positions
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x10c])) / 4;

	// Check sample offset
	temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x310]));
	if (temp1 == 0)
		return (0);

	// Check module size
	if ((temp1 + sampSize) > module.GetLength())
		return (0);

	// Check the first 64 notes
	offset = 0x314;
	for (i = 0; i < 64; i++)
	{
		if (mod[offset++] != 0x80)
		{
			// Get the period
			temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset - 1])) & 0x0fff;
			if ((temp != 0) && ((temp < 0x71) || (temp > 0x358)))
				return (0);

			offset += 3;
		}
	}

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
ap_result PROZ_STAR::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 pattOffset, sampOffset;
	int8 lastPatt;
	uint8 byte1, byte3;
	uint16 i, j;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write module name
	destFile->Write(&mod[0], 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Write sample name
		destFile->Write(zeroBuf, 22);

		// Write the rest
		destFile->Write(&mod[20 + i * 8], 8);
	}

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x00);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('FLT4');

	// Get sample offset
	sampOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x310])) + 0x314;

	// Begin to convert the patterns
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		// Get pattern number to build
		if (posTable[i] > lastPatt)
		{
			lastPatt++;

			// Clear pattern
			memset(pattern, 0, sizeof(pattern));

			// Get pattern offset
			pattOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x110 + i * 4])) + 0x314;

			for (j = 0; j < (64 * 4); j++)
			{
				// Out of range?
				if (pattOffset >= sampOffset)
					break;

				// Empty line?
				if (mod[pattOffset] == 0x80)
				{
					// Yup
					pattOffset++;
					continue;
				}

				// Normal line. Divide the sample number with 4
				byte3   = ((mod[pattOffset + 2] >> 4) + (mod[pattOffset] & 0xf0)) / 4;
				byte1   = byte3 & 0xf0;
				byte3 <<= 4;

				// Copy the pattern data
				pattern[j * 4]     = (mod[pattOffset++] & 0x0f) | byte1;
				pattern[j * 4 + 1] = mod[pattOffset++];
				pattern[j * 4 + 2] = (mod[pattOffset++] & 0x0f) | byte3;
				pattern[j * 4 + 3] = mod[pattOffset++];

				// Convert the volume effect argument from decimal to hex
				if ((pattern[j * 4 + 2] & 0x0f) == 0xc)
				{
					byte1 = pattern[j * 4 + 3];
					pattern[j * 4 + 3] = (byte1 / 0x10) * 10 + (byte1 % 0x10);
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[sampOffset], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_STAR::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_STAR);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_STAR::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint32 *posTab1, *posTab2;
	uint32 temp1;
	bool found;

	// Jump to the pattern offset table
	mod += 0x110;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = (uint32 *)mod + 1;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offset
		temp1 = P_BENDIAN_TO_HOST_INT32(*posTab1);
		posTab1++;

		// Compare the offset with the rest to see if there is an equal one
		posTab2 = (uint32 *)mod;
		found   = false;

		for (j = 0; j < i; j++)
		{
			if (temp1 == P_BENDIAN_TO_HOST_INT32(posTab2[0]))
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
