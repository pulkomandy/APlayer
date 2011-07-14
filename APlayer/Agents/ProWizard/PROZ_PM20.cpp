/******************************************************************************/
/* ProWizard PM20 class.                                                      */
/*                                                                            */
/* Promizer v2.0 format.                                                      */
/* Created by MC68000 / Masque / TRSI (19xx)                                  */
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
uint32 PROZ_PM20::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check some of the code
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) != 0x60000016)
		return (0);

	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4])) != 0x60000140)
		return (0);

	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x16])) != 0x4e75)
		return (0);

	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x18])) != 0x48e77ffe)
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check volume
		if (mod[0x1552 + i * 8 + 3] > 0x40)
			return (0);

		// Get sample size
		sampSize += (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1552 + i * 8])) * 2);
	}

	// Check the module length
	if ((0x1452 + P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1552 + 31 * 8])) + sampSize) > (module.GetLength() + 256))
		return (0);

	// Find number of positions
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1450])) / 2;

	// Create position table
	Speco(mod);

	// Check position table and find heighest pattern number
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
ap_result PROZ_PM20::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 addSamples, addNotes, notesTable;
	uint32 adrSamples, adrNotes, noteEnd;
	uint32 pattOffset, offset;
	uint8 byte1, byte2, byte3, byte4;
	int8 lastPos;
	bool breakFlag;
	uint16 i, j, k;

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

		// Write sample length
		destFile->Write(&mod[0x1552 + i * 8], 2);

		// Write finetune
		destFile->Write_UINT8(mod[0x1552 + i * 8 + 2] / 2);

		// Write volume
		destFile->Write_UINT8(mod[0x1552 + i * 8 + 3]);

		// Write repeat
		destFile->Write(&mod[0x1552 + i * 8 + 4], 2);

		// Write replen
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1552 + i * 8 + 6])) == 0)
			destFile->Write_B_UINT16(0x0001);
		else
			destFile->Write(&mod[0x1552 + i * 8 + 6], 2);
	}

	// Write number of positions
	destFile->Write_UINT8(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1450])) / 2);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Find all the offsets needed
	addSamples = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1552 + 31 * 8]));
	addNotes   = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1552 + 31 * 8 + 4]));
	notesTable = 0x1552 + 31 * 8 + 8;
	adrSamples = 0x1452 + addSamples;
	adrNotes   = 0x1452 + addNotes;
	noteEnd    = adrNotes - 4;

	// Convert the pattern data
	lastPos = -1;

	for (i = 0; i < posNum; i++)
	{
		if (posTable[i] > lastPos)
		{
			lastPos++;

			// Find offset to pattern data
			offset    = notesTable + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1452 + i * 2]));
			breakFlag = false;

			// Clear pattern
			memset(pattern, 0, sizeof(pattern));

			for (j = 0; j < 64; j++)
			{
				for (k = 0; k < 4; k++)
				{
					if (offset < noteEnd)
					{
						// Get offset to reference table
						pattOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));
						offset += 2;
						if (pattOffset != 0)
						{
							pattOffset = adrNotes + (pattOffset - 1) * 4;

							// Get sample number
							byte3 = mod[pattOffset] >> 2;
							if (byte3 >= 0x10)
							{
								byte3 -= 0x10;
								byte1  = 0x10;
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

							// Store the pattern data
							pattern[j * 16 + k * 4]     = byte1;
							pattern[j * 16 + k * 4 + 1] = byte2;
							pattern[j * 16 + k * 4 + 2] = byte3;
							pattern[j * 16 + k * 4 + 3] = byte4;

							// Have we reached a pattern stop effect
							byte3 &= 0x0f;
							if ((byte3 == 0x0d) || (byte3 == 0x0b))
								breakFlag = true;
						}
					}
				}

				// We have reached the end of the pattern
				if (breakFlag)
					break;
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[adrSamples - 4], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PM20::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PM20);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_PM20::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint16 *posTab1, *posTab2;
	uint16 temp1;
	bool found;

	// Jump to the pattern offset table
	mod += 0x1452;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = (uint16 *)mod + 1;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offset
		temp1 = P_BENDIAN_TO_HOST_INT16(*posTab1);
		posTab1++;

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
			posTab2++;
		}

		if (!found)
			posTable[i] = ++pattNum;
	}

	// Add one extra pattern number, because we skipped the first one
	pattNum++;
}
