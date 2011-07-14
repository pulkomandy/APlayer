/******************************************************************************/
/* ProWizard TDD class.                                                       */
/*                                                                            */
/* The Dark Demon format.                                                     */
/* Created by ???                                                             */
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
uint32 PROZ_TDD::CheckModule(const PBinary &module)
{
	const uint8 *mod;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check sample sizes
	pw_wholeSampleSize = 0;
	for (pw_j = 0; pw_j < 31; pw_j++)
	{
		// Sample address
		pw_k = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[pw_j * 14 + 130]));

		// Sample size
		pw_l = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[pw_j * 14 + 134])) * 2;

		// Loop start address
		pw_m = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[pw_j * 14 + 138]));

		// Loop size
		pw_n = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[pw_j * 14 + 142])) * 2;

		// Check volume
		if (mod[pw_j * 14 + 137] > 0x40)
			return (0);

		// Loop start address < sample address
		if (pw_m < pw_k)
			return (0);

		// Addresses < 564
		if ((pw_k < 564) || (pw_m < 564))
			return (0);

		// Loop start > size
		if ((pw_m - pw_k) > pw_l)
			return (0);

		// Loop start + replen > size
		if (((pw_m - pw_k) + pw_n) > (pw_l + 2))
			return (0);

		pw_wholeSampleSize += pw_l;
	}

	if ((pw_wholeSampleSize <= 2) || (pw_wholeSampleSize > (31 * 65535)))
		return (0);

	// Are there some patterns in the file?
	if ((pw_wholeSampleSize + 564) > module.GetLength())
		return (0);

	// Check size of the pattern list
	if ((mod[0] > 0x7f) || (mod[0] == 0x00))
		return (0);

	// Check pattern list
	pw_k = 0;
	for (pw_j = 0; pw_j < 128; pw_j++)
	{
		if (mod[pw_j + 2] > 0x7f)
			return (0);

		if (mod[pw_j + 2] > pw_k)
			pw_k = mod[pw_j + 2];
	}

	pw_k += 1;
	pw_k *= 1024;

	// Check end of pattern list
	for (pw_j = (mod[0] + 2); pw_j < 128; pw_j++)
	{
		if (mod[pw_j + 2] != 0)
			return (0);
	}

	// Check if not out of file range
	if ((pw_wholeSampleSize + 564 + pw_k) > module.GetLength())
		return (0);

	// pw_wholeSampleSize is the size of the sample data
	// pw_k is the whole pattern data size
	//
	// Check pattern data
	pw_l = 564 + pw_wholeSampleSize;

	for (pw_j = 0; pw_j < pw_k; pw_j += 4)
	{
		// Sample number > 31?
		if (mod[pw_l + pw_j] > 0x1f)
			return (0);

		// Note > 0x48 (36 * 2)
		if ((mod[pw_l + pw_j + 1] > 0x48) || (mod[pw_l + pw_j + 1] & 0x1))
			return (0);

		// Effect = 0xc and value > 64?
		if (((mod[pw_l + pw_j + 2] & 0x0f) == 0x0c) && (mod[pw_l + pw_j + 3] > 0x40))
			return (0);

		// Effect = 0xd and value > 64?
		if (((mod[pw_l + pw_j + 2] & 0x0f) == 0x0d) && (mod[pw_l + pw_j + 3] > 0x40))
			return (0);

		// Effect = 0xb and value > 127?
		if (((mod[pw_l + pw_j + 2] & 0x0f) == 0x0b) && (mod[pw_l + pw_j + 3] > 0x7f))
			return (0);
	}

	// Return the converted module size
	return (1084 + pw_wholeSampleSize + pw_k);
}



/******************************************************************************/
/* ConvertModule() will be convert the module to ProTracker format.           */
/*                                                                            */
/* Input:  "module" is a reference to the packed module.                      */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result PROZ_TDD::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 patMax;
	int32 i, j;
	int32 wholeSampleSize = 0;
	int32 sampleAddresses[31];
	int32 sampleSizes[31];
	uint8 pattern[1024];
	uint32 pattOffset;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Initialize the period table
	InitPeriods();

	// Initialize arrays
	memset(sampleAddresses, 0, sizeof(sampleAddresses));
	memset(sampleSizes, 0, sizeof(sampleSizes));
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Write sample name
		destFile->Write(zeroBuf, 22);

		// Remember sample address
		sampleAddresses[i] = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[130 + i * 14]));

		// Remember sample size
		sampleSizes[i]   = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[130 + i * 14 + 4])) * 2;
		wholeSampleSize += sampleSizes[i];

		// Write sample size
		destFile->Write_B_UINT16(sampleSizes[i] / 2);

		// Write finetune + volume
		destFile->Write_UINT8(mod[130 + i * 14 + 6]);
		destFile->Write_UINT8(mod[130 + i * 14 + 7]);

		// Get loop address
		j  = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[130 + i * 14 + 8]));
		j -= sampleAddresses[i];
		j /= 2;

		// Write loop start
		destFile->Write_B_UINT16(j);

		// Write replen
		destFile->Write(&mod[130 + i * 14 + 12], 2);
	}

	// Write size of position table, NTK byte + the position table
	destFile->Write(&mod[0], 1 + 1 + 128);

	// Write the ID
	destFile->Write_B_UINT32('M.K.');

	// Find max pattern
	patMax = 0;
	for (i = 0; i < 128; i++)
	{
		if (mod[2 + i] > patMax)
			patMax = mod[2 + i];
	}

	// Convert patterns
	for (i = 0; i <= patMax; i++)
	{
		// Find offset to pattern
		pattOffset = 564 + wholeSampleSize + i * 1024;

		for (j = 0; j < (64 * 4); j++)
		{
			// Effect value
			pattern[j * 4 + 3]  = mod[pattOffset + j * 4 + 3];

			// Effect
			pattern[j * 4 + 2]  = mod[pattOffset + j * 4 + 2] & 0x0f;

			// Sample
			pattern[j * 4]      = mod[pattOffset + j * 4] & 0xf0;
			pattern[j * 4 + 2] |= (mod[pattOffset + j * 4] << 4) & 0xf0;

			// Note
			if (mod[pattOffset + j * 4 + 1] != 0)
			{
				pattern[j * 4]     |= period[(mod[pattOffset + j * 4 + 1] / 2) - 1][0];
				pattern[j * 4 + 1]  = period[(mod[pattOffset + j * 4 + 1] / 2) - 1][1];
			}
			else
				pattern[j * 4 + 1] = 0x00;
		}

		// Write pattern
		destFile->Write(pattern, 1024);
	}

	// Write sample data
	for (i = 0; i < 31; i++)
	{
		if (sampleSizes[i] != 0)
			destFile->Write(&mod[sampleAddresses[i]], sampleSizes[i]);
	}

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_TDD::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_TDD);
}
