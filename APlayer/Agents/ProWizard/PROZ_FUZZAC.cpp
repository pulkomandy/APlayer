/******************************************************************************/
/* ProWizard FUZZAC class.                                                    */
/*                                                                            */
/* Fuzzac Packer format.                                                      */
/* Created by Fuzzac / Silents (1990)                                         */
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
uint32 PROZ_FUZZAC::CheckModule(const PBinary &module)
{
	const uint8 *mod, *tempPoi;
	uint32 temp;
	uint16 temp1;
	uint16 offset, maxOffset;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check mark
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) != 'M1.0')
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Get sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[66 + i * 68]));
		if (temp >= 0x8000)
			return (0);

		if (temp != 0)
		{
			sampSize += (temp * 2);

			// Check repeat + replen
			if (((uint32)P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[66 + i * 68 + 2])) + (uint32)P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[66 + i * 68 + 4]))) > temp)
				return (0);

			// Check volume
			if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[66 + i * 68 + 6])) > 0x40)
				return (0);
		}
	}

	// Check position table size
	posNum = mod[0x842];
	if ((posNum == 0x00) || (posNum > 0x7f))
		return (0);

	// Get max track offset
	offset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x843]));
	if (offset == 0)
		return (0);

	// Scan all the track offsets and find the heighest one
	tempPoi   = &mod[0x846];
	maxOffset = 0;

	for (i = 0; i < (posNum * 4); i++)
	{
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)tempPoi));
		if (temp1 > maxOffset)
			maxOffset = temp1;

		tempPoi += 4;
	}

	// Do the offset table have the same heighest offset as stored in the module?
	if ((maxOffset + 0x100) != offset)
		return (0);

	// Remember different pointers
	notes   = tempPoi - 0x40;
	samples = notes + offset + 4;

	// Check the end mark
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)(samples - 4))) != 'SEnd')
		return (0);

	// Check first pattern
	for (i = 0; i < 64; i++)
	{
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)tempPoi)) & 0x0fff;
		if (temp1 == 0)
			continue;

		if ((temp1 < 0x71) || (temp1 > 0x358))
			return (0);
	}

	// Check the module length
	if ((samples - mod + sampSize) > (module.GetLength() + 256))
		return (0);

	// Allocate fuzzac table
	fuzzacTable = new uint16[127 * 4];
	if (fuzzacTable == NULL)
		return (0);

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
ap_result PROZ_FUZZAC::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	int8 lastPattNum = -1;
	uint16 i, j, k;
	const uint8 *trackPoi;

	try
	{
		// Get the module pointer
		mod = module.GetBufferForReadOnly();

		// Zero the buffer
		memset(zeroBuf, 0, sizeof(zeroBuf));

		// Write the module name
		destFile->Write(zeroBuf, 20);

		// Write sample informations
		for (i = 0; i < 31; i++)
		{
			// Sample name
			destFile->Write(&mod[6 + i * 68], 22);

			// Sample size
			destFile->Write(&mod[6 + i * 68 + 60], 2);

			// Finetune + volume
			destFile->Write(&mod[6 + i * 68 + 66], 2);

			// Repeat point
			destFile->Write(&mod[6 + i * 68 + 62], 2);

			// Repeat length
			if (*(uint16 *)&mod[6 + i * 68 + 64] != 0)
				destFile->Write(&mod[6 + i * 68 + 64], 2);
			else
				destFile->Write_B_UINT16(0x0001);
		}

		// Write position table length
		destFile->Write_UINT8(posNum);

		// Write NTK byte
		destFile->Write_UINT8(0x7f);

		// Write position table
		destFile->Write(posTable, 128);

		// Write PTK mark
		destFile->Write_B_UINT32('M.K.');

		// Now it's time to convert join the tracks and build the pattern data
		for (i = 0; i < posNum; i++)
		{
			if (posTable[i] > lastPattNum)
			{
				lastPattNum++;

				// Build each channel from the tracks
				for (j = 0; j < 4; j++)
				{
					// Get the track offset
					trackPoi = &notes[fuzzacTable[i * 4 + j]];

					// Copy track data
					for (k = 0; k < 64; k++)
					{
						pattern[j * 4 + k * 16]     = *trackPoi++;
						pattern[j * 4 + k * 16 + 1] = *trackPoi++;
						pattern[j * 4 + k * 16 + 2] = *trackPoi++;
						pattern[j * 4 + k * 16 + 3] = *trackPoi++;
					}
				}

				// Write the pattern
				destFile->Write(pattern, sizeof(pattern));
			}
		}

		// Write sample data
		destFile->Write(samples, sampSize);
	}
	catch(...)
	{
		// Free the temporary buffers
		delete[] fuzzacTable;
		fuzzacTable = NULL;
		throw;
	}

	// Free the temporary buffers
	delete[] fuzzacTable;
	fuzzacTable = NULL;

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_FUZZAC::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_FUZZAC);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_FUZZAC::Speco(const uint8 *mod)
{
	uint16 i, j;
	uint16 *fuzTab1, *fuzTab2;
	uint16 temp1, temp2, temp3, temp4;
	bool found;

	// Jump to the track offset table
	mod += 0x846;

	// Convert the 4 tables into a single one
	for (i = 0; i < posNum; i++)
	{
		fuzzacTable[i * 4]     = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 4]));
		fuzzacTable[i * 4 + 1] = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 4 + posNum * 4]));
		fuzzacTable[i * 4 + 2] = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 4 + posNum * 4 * 2]));
		fuzzacTable[i * 4 + 3] = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 4 + posNum * 4 * 3]));
	}

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	fuzTab1 = fuzzacTable + 4;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = *fuzTab1++;
		temp2 = *fuzTab1++;
		temp3 = *fuzTab1++;
		temp4 = *fuzTab1++;

		// Compare these offsets with the rest to see if there is an equal one
		fuzTab2 = fuzzacTable;
		found   = false;

		for (j = 0; j < i; j++)
		{
			if ((temp1 == fuzTab2[0]) && (temp2 == fuzTab2[1]) && (temp3 == fuzTab2[2]) && (temp4 == fuzTab2[3]))
			{
				// Found an equal track joinment
				posTable[i] = posTable[j];
				found       = true;
				break;
			}

			// Go to the next offset pair
			fuzTab2 += 4;
		}

		if (!found)
			posTable[i] = ++pattNum;
	}

	// Add one extra pattern number, because we skipped the first one
	pattNum++;
}
