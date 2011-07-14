/******************************************************************************/
/* ProWizard GMC class.                                                       */
/*                                                                            */
/* Game Music Creator format.                                                 */
/* Created by Andreas Tadic (1989)                                            */
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
uint32 PROZ_GMC::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp, temp1, temp2;
	uint16 periodNum;
	uint16 i, j;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check size of position table
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xf0])) > 0x64)
		return (0);

	// Check the first pattern offsets
	temp  = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xf4]));
	temp |= P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xf8]));
	temp |= P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xfc]));
	temp |= P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x100]));
	temp &= 0x03ff03ff;
	if (temp != 0)
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 15; i++)
	{
		// Check start address
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16]));
		if (temp > 0xf8000)
			return (0);

		// Check sample size
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 4]));
		if (temp1 >= 0x8000)
			return (0);

		if ((temp1 != 0) && (temp == 0))
			return (0);

		sampSize += (temp1 * 2);

		// Check volume
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 6])) > 0x40)
			return (0);

		// Get repeat address
		temp2 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16 + 8]));
		if (temp2 != 0)
		{
			if ((temp2 > 0xf8000) || (temp == 0) || (temp1 == 0))
				return (0);
		}
	}

	// Get size of position table
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0xf2]));

	// Check position table and find number of patterns
	temp1 = 0;
	for (i = 0; i < posNum; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0xf4 + i * 2]));
		if (((temp / 1024) * 1024) != temp)
			return (0);

		if (temp > temp1)
			temp1 = temp;
	}

	pattNum = temp1 / 1024 + 1;

	// Init period table
	InitPeriods();

	// Check periods in the first pattern
	periodNum = 0;
	for (i = 0; i < 256; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1bc + i * 4])) & 0x0fff;
		if ((temp != 0) && (temp != 0xffe))
		{
			uint8 hi = (temp & 0xff00) >> 8;
			uint8 lo = temp & 0x00ff;
			bool found = false;

			for (j = 0; j < 36; j++)
			{
				// Try to find the period in the period table
				if ((period[j][0] == hi) && (period[j][1] == lo))
				{
					found = true;
					break;
				}
			}

			if (!found)
				return (0);

			periodNum++;
		}
	}

	if (periodNum == 0)
		return (0);

	// Check the module length
	if ((0x1bc + pattNum * 1024 + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_GMC::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 posTable[128];
	uint32 pattern[4 * 64];
	uint16 i, j;
	uint16 temp;
	uint32 pattData, temp1;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 15; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size + Volume
		destFile->Write(&mod[i * 16 + 4], 4);

		// Get repeat length
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 12]));
		if ((temp != 0) && (temp != 2))
		{
			// Loop
			destFile->Write_B_UINT16((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16 + 8])) - P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16]))) / 2);
			destFile->Write_B_UINT16(temp);
		}
		else
		{
			// No loop
			destFile->Write_B_UINT16(0);
			destFile->Write_B_UINT16(1);
		}
	}

	// Write empty samples
	for (; i < 31; i++)
	{
		// Write sample name + size + volume + repeat point
		destFile->Write(zeroBuf, 28);

		// Write repeat length
		destFile->Write_B_UINT16(1);
	}

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Build the position table
	for (i = 0, j = 0; i < posNum; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0xf4 + i * 2]));
		if (temp >= 0x8000)
		{
			posNum--;
			continue;
		}

		posTable[j++] = temp / 1024;
	}

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x00);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Now it's time to convert the effects and build the pattern data
	for (i = 0; i < pattNum; i++)
	{
		for (j = 0; j < (4 * 64); j++)
		{
			// Get the pattern data
			pattData = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1bc + i * 1024 + j * 4]));

			// Mask out the effect
			temp1 = pattData & 0x00000f00;

			// Convert the effect
			switch (temp1)
			{
				// No effect, remove the effect data
				case 0x000:
				{
					pattData &= 0xffffff00;
					break;
				}

				// Set volume (3 -> C)
				case 0x300:
				{
					pattData &= 0xfffff0ff;
					pattData |= 0x00000c00;
					break;
				}

				// Pattern break (4 -> D)
				case 0x400:
				{
					pattData &= 0xfffff0ff;
					pattData |= 0x00000d00;
					break;
				}

				// Pattern jump (5 -> B)
				case 0x500:
				{
					pattData &= 0xfffff0ff;
					pattData |= 0x00000b00;
					break;
				}

				// Set filter on (6 -> E00)
				case 0x600:
				{
					pattData &= 0xfffff000;
					pattData |= 0x00000E00;
					break;
				}

				// Set filter off (7 -> E01)
				case 0x700:
				{
					pattData &= 0xfffff000;
					pattData |= 0x00000E01;
					break;
				}

				// Set speed (8 -> F)
				case 0x800:
				{
					pattData &= 0xfffff0ff;
					pattData |= 0x00000f00;
					break;
				}
			}

			// Check the period
			temp1 = (pattData & 0xffff0000) >> 16;
			if (temp1 == 0xfffe)	// Mute command
				pattData = 0x00000c00;
			else
			{
				if (temp1 > 0x358)
				{
					pattData &= 0x0000ffff;
					pattData |= 0x03580000;
				}
			}

			// Write the pattern data
			pattern[j] = P_HOST_TO_BENDIAN_INT32(pattData);
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[0x1bc + pattNum * 1024], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_GMC::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_GMC);
}
