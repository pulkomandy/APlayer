/******************************************************************************/
/* ProWizard NTPK class.                                                      */
/*                                                                            */
/* NoiseTracker Pak format.                                                   */
/* Created by United Forces (1990)                                            */
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
uint32 PROZ_NTPK::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp, temp1, temp2, temp3;
	uint32 temp4;
	uint16 i;
	uint32 sampOffset, offset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the first pattern mark
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x1f6])) != 'PATT')
		return (0);

	// Get number of positions
	posNum = mod[0x174];
	if ((posNum == 0) || (posNum >= 128))
		return (0);

	// Find heighest pattern number
	if (GetPGP(&mod[0x176], posNum) > 64)
		return (0);

	// Check offset to first sample
	sampOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]));
	if (sampOffset & 0x1)
		return (0);

	// Check number of patterns
	offset = 0x1f6;
	for (i = 0; i < pattNum; i++)
	{
		for (;;)
		{
			temp4 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[offset]));
			offset += 2;

			if (temp4 == 'PATT')
				break;	// Go to next pattern

			if (offset >= sampOffset)
				return (0);
		}
	}

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check pointer to sample
		sampOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 12]));
		if (sampOffset & 0x1)
			return (0);

		// Check sample length
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 12 + 4]));
		if (temp >= 0x8000)
			return (0);

		// Get volume + repeat + repeat length
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 12 + 6]));
		temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 12 + 8]));
		temp3 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 12 + 10]));

		// Check volume & finetune
		if (((temp1 & 0x00ff) > 0x40) || ((temp1 & 0xff00) > 0x0f00))
			return (0);

		if (temp == 0)
		{
			// Vol + repeat + repeat length
			if ((temp1 + temp2 + temp3) > 0x0001)
				return (0);
		}
		else
		{
			// Check repeat
			if ((temp2 + temp3) > temp)
				return (0);

			sampSize += (temp * 2);
		}
	}

	// Check the module length
	if ((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_NTPK::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint16 temp;
	uint8 temp1;
	uint32 offset;
	uint16 i, j, k;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size + finetune + volume + repeat
		destFile->Write(&mod[i * 12 + 4], 6);

		// Replen
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 12 + 10]));
		if (temp == 0)
			destFile->Write_B_UINT16(0x0001);
		else
			destFile->Write_B_UINT16(temp);
	}

	// Write position table length + NTK byte + position table
	destFile->Write(&mod[0x174], 1 + 1 + 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert patterns
	offset = 0x1f6;
	for (i = 0; i < pattNum; i++)
	{
		// Make sure we have an even offset
		if (offset & 0x1)
			offset++;

		// Skip PATT + 3 words
		offset += (4 + 3 * 2);

		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Loop the voices
		for (j = 0; j < 4; j++)
		{
			// Loop the rows
			for (k = 0; k < 64; k++)
			{
				// Get pattern byte
				temp1 = mod[offset];

				if (temp1 == 0xff)
				{
					// Empty lines
					k      += (mod[offset + 1] - 1);
					offset += 2;
					continue;
				}

				if (temp1 >= 0xc0)
				{
					// Sample 0x1x and no effect
					temp1 -= 0xc0;

					// Get note
					if (temp1 != 0)
					{
						temp1--;
						pattern[k * 16 + j * 4]     = period[temp1][0];
						pattern[k * 16 + j * 4 + 1] = period[temp1][1];
					}

					// Set hi sample bit
					pattern[k * 16 + j * 4] |= 0x10;

					// Get sample number
					pattern[k * 16 + j * 4 + 2] = mod[offset + 1];

					offset += 2;
					continue;
				}

				if (temp1 >= 0x80)
				{
					// Sample 0x1x and effect
					temp1 -= 0x80;

					// Get note
					if (temp1 != 0)
					{
						temp1--;
						pattern[k * 16 + j * 4]     = period[temp1][0];
						pattern[k * 16 + j * 4 + 1] = period[temp1][1];
					}

					// Set hi sample bit
					pattern[k * 16 + j * 4] |= 0x10;

					// Get sample number + effect
					pattern[k * 16 + j * 4 + 2] = mod[offset + 1];
					pattern[k * 16 + j * 4 + 3] = mod[offset + 2];

					offset += 3;
					continue;
				}

				if (temp1 >= 0x40)
				{
					// Sample 0x0x and no effect
					temp1 -= 0x40;

					// Get note
					if (temp1 != 0)
					{
						temp1--;
						pattern[k * 16 + j * 4]     = period[temp1][0];
						pattern[k * 16 + j * 4 + 1] = period[temp1][1];
					}

					// Get sample number
					pattern[k * 16 + j * 4 + 2] = mod[offset + 1];

					offset += 2;
					continue;
				}

				// Sample 0x0x and effect
				//
				// Get note
				if (temp1 != 0)
				{
					temp1--;
					pattern[k * 16 + j * 4]     = period[temp1][0];
					pattern[k * 16 + j * 4 + 1] = period[temp1][1];
				}

				// Get sample number + effect
				pattern[k * 16 + j * 4 + 2] = mod[offset + 1];
				pattern[k * 16 + j * 4 + 3] = mod[offset + 2];

				offset += 3;
			}
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]))], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_NTPK::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_NTPK);
}
