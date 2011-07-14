/******************************************************************************/
/* ProWizard PM40 class.                                                      */
/*                                                                            */
/* Promizer v4.0 format.                                                      */
/* Created by MC68000 (1994)                                                  */
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
uint32 PROZ_PM40::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 i;
	uint16 temp;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the ID
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) != 'PM40')
		return (0);

	// Check size of the position table
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4])) > 0x7f)
		return (0);

	// Check the position table
	for (i = 0; i < 128; i++)
	{
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 2])) & 0x1)
			return (0);
	}

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Get sample length
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x108 + i * 8]));
		if (temp >= 0x8000)
			return (0);

		if (temp == 0)
		{
			temp += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x108 + i * 8 + 2]));
			temp += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x108 + i * 8 + 4]));
			temp += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x108 + i * 8 + 6]));
			if (temp != 0)
				return (0);
		}

		// Get sample size
		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[0x108 + i * 8 + 3] > 0x40) || (mod[0x108 + i * 8 + 2] > 0x0f))
			return (0);
	}

	// Check the module length
	if ((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x200])) + 4 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Find number of positions
	posNum = mod[7];

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
ap_result PROZ_PM40::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 adrSamples, adrRealNotes, notesTable;
	uint32 pattOffset, offset;
	uint8 byte1, byte2, byte3, byte4;
	int8 lastPos;
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

		// Write the rest of the information
		destFile->Write(&mod[0x108 + i * 8], 6);

		// Well, test replen
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x108 + i * 8 + 6])) != 0)
			destFile->Write(&mod[0x108 + i * 8 + 6], 2);
		else
			destFile->Write_B_UINT16(0x0001);
	}

	// Find all the offsets needed
	adrSamples   = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x200])) + 4;
	adrRealNotes = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x204])) + 4;
	notesTable   = 0x208;

	// Write number of positions
	destFile->Write_UINT8(mod[7]);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert the pattern data
	lastPos = -1;

	for (i = 0; i < posNum; i++)
	{
		if (posTable[i] > lastPos)
		{
			lastPos++;

			// Find offset to pattern data
			offset = notesTable + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x8 + i * 2]));

			for (j = 0; j < (64 * 4); j++)
			{
				// Get offset to reference table
				pattOffset = adrRealNotes + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset])) * 4;
				offset += 2;

				// Get sample number
				byte3 = mod[pattOffset];
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
					byte2--;

					byte1 |= period[byte2][0];
					byte2  = period[byte2][1];
				}

				// Get effect
				byte3 |= (mod[pattOffset + 2] & 0x0f);
				byte4  = mod[pattOffset + 3];

				// Store the pattern data
				pattern[j * 4]     = byte1;
				pattern[j * 4 + 1] = byte2;
				pattern[j * 4 + 2] = byte3;
				pattern[j * 4 + 3] = byte4;
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[adrSamples], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PM40::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PM40);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_PM40::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint16 *posTab1, *posTab2;
	uint16 temp1;
	bool found;

	// Jump to the pattern offset table
	mod += 0x8;

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
