/******************************************************************************/
/* ProWizard PM01 class.                                                      */
/*                                                                            */
/* Promizer v0.1 format.                                                      */
/* Created by Franck Hulsmann (MC68000/Masque) (19xx)                         */
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
/* Global variables                                                           */
/******************************************************************************/
extern int16 tuning[16][36];



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a known module.      */
/*                                                                            */
/* Input:  "module" is a reference to where the packed module is stored.      */
/*                                                                            */
/* Output: Is the unpacked module size or 0 if not recognized.                */
/******************************************************************************/
uint32 PROZ_PM01::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint32 temp1;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check size of position table
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0xf8]));
	if ((temp > 0x200) || (temp & 0x1))
		return (0);

	// Check the first 4 pattern numbers
	temp1  = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xfa]));
	temp1 |= P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xfe]));
	temp1 |= P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x102]));
	temp1 |= P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x106]));
	temp1 &= 0xffff03ff;
	if (temp1 != 0)
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8]));
		if (temp >= 0x8000)
			return (0);

		if (temp == 0)
		{
			temp += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 2]));
			temp += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 4]));
			temp += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 6]));
			if (temp != 0x0001)
				return (0);
		}
		else
		{
			// Check loop
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 6]))) > temp)
				return (0);

			sampSize += (temp * 2);

			// Check volume & finetune
			if ((mod[i * 8 + 3] > 0x40) || (mod[i * 8 + 2] > 0x0f))
				return (0);
		}
	}

	// Find number of positions
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0xf8])) / 4;

	// Check position table and find heighest pattern number
	pattNum = 0;
	for (i = 0; i < 128; i++)
	{
		temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xfa + i * 4]));
		if (((temp1 / 1024) * 1024) != temp1)
			return (0);

		if (temp1 > pattNum)
			pattNum = temp1;
	}

	pattNum /= 1024;
	pattNum++;
	if (pattNum > 64)
		return (0);

	// Check the size of all patterns
	temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x2fa]));
	if ((temp1 == 0) || (temp1 & 0x1) || (temp1 & 0x2))
		return (0);

	if (temp1 != (uint32)(pattNum * 1024))
		return (0);

	// Check the first pattern
	for (i = 0; i < (64 * 4); i++)
	{
		temp  = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x2fe + i * 4])) ^ 0xffff;
		temp &= 0x0fff;
		if (temp != 0)
		{
			if ((temp < 0x6c) || (temp > 0x38b))
				return (0);
		}
	}

	// Check the module length
	if ((0x2fe + pattNum * 1024 + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_PM01::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 *pattern;
	uint32 readOffset, writeOffset;
	uint16 i;
	uint32 j;

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

		// Store the finetune value to later use
		finetune[i] = mod[i * 8 + 2];
	}

	// Write number of positions
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	for (i = 0; i < 128; i++)
		destFile->Write_UINT8(P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0xfa + i * 4])) / 1024);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Allocate memory to hold the pattern data
	pattern = new uint8[pattNum * 1024];
	if (pattern == NULL)
		throw PMemoryException();

	try
	{
		// Convert the pattern data
		readOffset  = 0x02fe;
		writeOffset = 0x0000;
		for (j = 0; j < (uint32)(pattNum * 64 * 4); j++)
		{
			pattern[writeOffset++] = mod[readOffset++] ^ 0xff;
			pattern[writeOffset++] = mod[readOffset++] ^ 0xff;
			pattern[writeOffset++] = mod[readOffset++] ^ 0xff;
			pattern[writeOffset++] = mod[readOffset++] ^ 0xf0;
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
	destFile->Write(&mod[0x2fe + pattNum * 1024], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PM01::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PM01);
}



/******************************************************************************/
/* AdjustFinetunes() will scan the patterns to adjust the periods.            */
/*                                                                            */
/* Input:  "pattern" is a pointer to the pattern data.                        */
/******************************************************************************/
void PROZ_PM01::AdjustFinetunes(uint8 *pattern)
{
	uint16 curPeriod;
	uint32 sampNum;
	uint32 temp;
	uint16 i, k;
	uint32 j;

	// Initialize the period table
	InitPeriods();

	for (i = 0; i < 31; i++)
	{
		// Has the sample a finetune?
		if (finetune[i] != 0)
		{
			// Yup, scan all the patterns after the sample
			//
			// Create sample number in pattern format
			sampNum = (((i + 1) & 0x10) << 24) | (((i + 1) & 0x0f) << 12);

			for (j = 0; j < (uint32)(pattNum * 64 * 4); j++)
			{
				// Did we found the sample number?
				temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&pattern[j * 4]));
				if ((temp & 0xf000f000) == sampNum)
				{
					// Yip, convert the period
					curPeriod = (temp & 0x0fff0000) >> 16;

					// Find the period in the period table
					for (k = 0; k < 36; k++)
					{
						if (tuning[finetune[i]][k] == curPeriod)
						{
							// Found the period, now convert it
							pattern[j * 4]     = (pattern[j * 4] & 0xf0) | period[k][0];
							pattern[j * 4 + 1] = period[k][1];
							break;
						}
					}
				}
			}
		}
	}
}
