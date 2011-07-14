/******************************************************************************/
/* ProWizard PHA class.                                                       */
/*                                                                            */
/* PhaPacker format.                                                          */
/* Created by Azatoth / Phenomena (19xx)                                      */
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
uint32 PROZ_PHA::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp, temp1;
	uint32 temp2;
	int16 temp3;
	uint32 offset;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the first sample address
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[8])) != 0x3c0)
		return (0);

	// Check number of positions
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1b4]));
	if ((temp == 0) || (temp > 0x200) || (temp & 0x3))
		return (0);

	posNum = temp / 4;

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 14]));
		sampSize += (temp * 2);

		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 14 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 14 + 6]));

		if (temp == 0)
		{
			// Check loop
			if (temp1 != 0x0001)
				return (0);
		}
		else
		{
			// Check loop
			if ((temp1 == 0) || (temp1 > temp))
				return (0);
		}

		// Check volume
		if (mod[i * 14 + 3] > 0x40)
			return (0);

		// Check finetune
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 14 + 12]));
		if (((temp / 0x48) * 0x48) != temp)
			return (0);
	}

	// Find heigest pattern offset
	offset = 0;
	for (i = 0; i < 128; i++)
	{
		temp2 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1c0 + i * 4]));
		if (temp2 > offset)
			offset = temp2;
	}

	// Check last pattern and find end of module
	temp3 = 0;
	for (i = 0; i < (64 * 4); i++)
	{
		// Increment counter
		temp3++;
		if (temp3 >= 0)
		{
			// Read next row
			offset += 4;
			temp    = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));

			if (temp >= 0x8000)
			{
				// Get new counter value
				offset += 2;
				temp3   = temp;

				if ((temp3 & 0xff00) == 0xff00)
					continue;

				if (i != (64 * 4 - 1))
					return (0);

				offset -= 2;
			}
		}
	}

	// Remember the end of the module
	endMod = offset;

	// Check module size
	if (offset > module.GetLength())
		return (0);

	// Create position table
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
ap_result PROZ_PHA::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 pattOffset, writeOffset, tempOffset;
	uint32 pattData;
	int8 lastPatt, lines;
	int8 k;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Init period table
	InitPeriods();

	// Write module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Write sample name
		destFile->Write(zeroBuf, 22);

		// Write sample size
		destFile->Write(&mod[i * 14], 2);

		// Write finetune
		destFile->Write_UINT8(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 14 + 12])) / 0x48);

		// Write volume
		destFile->Write_UINT8(mod[i * 14 + 3]);

		// Write loop start and replen
		destFile->Write(&mod[i * 14 + 4], 4);
	}

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Begin to convert the patterns
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		// Get pattern number to build
		if (posTable[i] > lastPatt)
		{
			lastPatt++;

			// Get pattern offset
			pattOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1c0 + i * 4]));
			if (pattOffset == 0)
				break;

			// Clear pattern
			memset(pattern, 0, sizeof(pattern));

			// Convert the pattern
			writeOffset = 0;
			for (j = 0; j < 256; j++)
			{
				// Did we get an offset bigger than the end offset
				if (pattOffset >= endMod)
					break;

				// Check for repeat command
				if (mod[pattOffset] == 0xff)
				{
					// Get the number of lines to repeat
					lines = -(int8)mod[pattOffset + 1];

					// Adjust counter
					j--;

					// Get the pattern data
					tempOffset = writeOffset - 4;
					pattData = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&pattern[tempOffset]));
					if (pattData == 0x00000000)
						pattData = 0xeeeeeeee;

					// Write the pattern data
					for (k = 0; k < lines; k++)
					{
						*((uint32 *)&pattern[tempOffset]) = P_HOST_TO_BENDIAN_INT32(pattData);
						tempOffset += 16;
					}

					// Skip the repeat command
					pattOffset += 2;
				}
				else
				{
					// Just a normal note, but convert it first
					//
					// Get sample
					byte3 = mod[pattOffset];
					if (byte3 >= 0x10)
					{
						byte1  = 0x10;
						byte3 -= 0x10;
					}
					else
						byte1 = 0x00;

					byte3 <<= 4;

					// Get note
					byte2 = mod[pattOffset + 1];
					if (byte2 != 0)
					{
						byte2 -= 2;
						byte2 /= 2;

						byte1 |= period[byte2][0];
						byte2  = period[byte2][1];
					}

					// Get effect
					byte3 |= (mod[pattOffset + 2] & 0x0f);
					byte4  = mod[pattOffset + 3];

					// Fix position jump
					if ((byte3 & 0x0f) == 0x0b)
						byte4++;

					// Fix for "Raw 3 - Intro"
					if (((byte3 & 0x0f) == 0x0f) && (byte4 == 0x20))
						byte4 = 0x1f;

					// Find an empty row
					while (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&pattern[writeOffset])) != 0x00000000)
					{
						writeOffset += 4;

						j++;
						if (j == 256)
							break;
					}

					if (j == 256)
						break;

					// Write the pattern data
					pattern[writeOffset++] = byte1;
					pattern[writeOffset++] = byte2;
					pattern[writeOffset++] = byte3;
					pattern[writeOffset++] = byte4;

					// Skip the note
					pattOffset += 4;
				}
			}

			// Convert special 0xeeeeeeee to empty lines
			for (j = 0; j < 256; j++)
			{
				if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&pattern[j * 4])) == 0xeeeeeeee)
					*((uint32 *)&pattern[j * 4]) = 0x00000000;
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[0x3c0], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PHA::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PHA);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_PHA::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint32 *posTab1, *posTab2;
	uint32 temp1;
	bool found;

	// Jump to the pattern offset table
	mod += 0x1c0;

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
