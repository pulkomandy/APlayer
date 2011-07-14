/******************************************************************************/
/* ProWizard PM18 class.                                                      */
/*                                                                            */
/* Promizer v1.0c + v1.8a format.                                             */
/* Created by MC68000 / TECH (19xx)                                           */
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
uint32 PROZ_PM18::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 i;
	uint32 baseOffset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check some of the code
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[14])) != 0x48e780c0)
		return (0);

	// Is the module 1.0 or 1.8?
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1164])) == 0x3de)
	{
		baseOffset = 0x1168;
		type       = 'PM18';
	}
	else
	{
		baseOffset = 0x1164;
		type       = 'PM10';
	}

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check volume & finetune
		if ((mod[baseOffset + 8 + i * 8 + 3] > 0x40) || (mod[baseOffset + 8 + i * 8 + 2] > 0x0f))
			return (0);

		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[baseOffset + 8 + i * 8]));
		if (temp != 0)
		{
			sampSize += (temp * 2);

			// Check loop
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[baseOffset + 8 + i * 8 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[baseOffset + 8 + i * 8 + 6]))) > temp)
				return (0);
		}
	}

	// Find number of positions
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[baseOffset + 0x100])) / 4;

	// Check the module length
	if ((baseOffset + P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[baseOffset])) + 4 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Create position table
	baseOffset += 0x102;
	Speco(mod + baseOffset);

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
ap_result PROZ_PM18::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 *pattern;
	uint32 offset1, offset2, offset3;
	uint32 base, offset;
	uint32 writeOffset;
	uint32 noteNum, notesTable, refOffset;
	uint32 pattData;
	uint16 posPtr;
	uint16 temp, temp1;
	int8 lastPos;
	bool breakFlag;
	uint16 i, j;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write module name
	destFile->Write(zeroBuf, 20);

	// Find offsets
	if (type == 'PM10')
	{
		offset1 = 0x1168;
		offset2 = 0x116c;
		offset3 = 0x1164;
	}
	else
	{
		offset1 = 0x116c;
		offset2 = 0x1170;
		offset3 = 0x1168;
	}

	// Get number of notes
	noteNum = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[offset1]));

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Write sample name
		destFile->Write(zeroBuf, 22);

		// Write the rest of the information
		destFile->Write(&mod[offset2 + i * 8], 8);

		// Store the finetune value to later use
		finetune[i] = mod[offset2 + i * 8 + 2];
	}

	// Write number of positions
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Find base and notes table offset
	base       = offset2 + 31 * 8 + 2;
	notesTable = base + 512;

	// Find heighest pattern number
	offset = notesTable;
	temp   = 0;
	for (i = 0; i < (noteNum / 2); i++)
	{
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));
		if (temp1 > temp)
			temp = temp1;

		offset += 2;
	}

	refOffset = offset;

	// Allocate memory to hold the pattern data
	pattern = new uint8[pattNum * 1024];
	if (pattern == NULL)
		throw PMemoryException();

	try
	{
		// Convert the pattern data
		posPtr      = 0;
		lastPos     = -1;
		breakFlag   = false;
		writeOffset = 0;

		for (i = 0; i < posNum; i++)
		{
			if (posTable[i] > lastPos)
			{
				lastPos++;

				// Find offset to pattern data
				offset = notesTable + P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[base + i * 4]));

				for (j = 0; j < (64 * 4); j++)
				{
					if (offset >= refOffset)
					{
						offset  += 2;
						pattData = 0;
					}
					else
					{
						pattData = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset])) * 4 + refOffset]));
						offset  += 2;
					}

					// Store the pattern data
					*((uint32 *)&pattern[writeOffset]) = P_HOST_TO_BENDIAN_INT32(pattData);
					writeOffset += 4;
					posPtr      += 4;

					// Did we reach a break effect?
					pattData &= 0x0f00;
					if ((pattData == 0x0d00) || (pattData == 0x0b00))
						breakFlag = true;
				}

				if (breakFlag)
				{
					// Got a pattern break
					breakFlag = false;

					if (posPtr > 1024)
						posPtr = 1024;

					// Adjust pattern offset
					writeOffset += (1024 - posPtr);
					posPtr       = 0;
				}
				else
				{
					if (posPtr >= 1024)
						posPtr = 0;
				}
			}
		}

		// Adjust finetunes
		AdjustFinetunes(pattern);

		// Write the patterns
		destFile->Write(pattern, pattNum * 1024);
	}
	catch(...)
	{
		// Delete the pattern data
		delete[] pattern;
		throw;
	}

	// Delete the pattern data
	delete[] pattern;

	// Write sample data
	destFile->Write(&mod[P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[offset3])) + offset3 + 4], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PM18::GetModuleType(void)
{
	return (type == 'PM10' ? IDS_PROZ_TYPE_PM10 : IDS_PROZ_TYPE_PM18);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_PM18::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint32 *posTab1, *posTab2;
	uint32 temp1;
	bool found;

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
