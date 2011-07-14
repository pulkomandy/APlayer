/******************************************************************************/
/* ProWizard SKYT class.                                                      */
/*                                                                            */
/* SKYT Packer format.                                                        */
/* Created by Mr. Bluesky / Drifters                                          */
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
uint32 PROZ_SKYT::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Start to check id
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x100])) != 'SKYT')
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[i * 8 + 3] > 0x40) || (mod[i * 8 + 2] > 0x0f))
			return (0);

		if (temp != 0)
		{
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 6]))) > temp)
				return (0);
		}
	}

	// Get number of patterns
	pattNum = mod[0x104] + 1;

	// Check track numbers
	for (i = 0; i < pattNum; i++)
	{
		if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x106 + i * 2])) & 0x00ff) != 0x0000)
			return (0);
	}

	// Check the module length
	if ((0x106 + pattNum * 2 * 4 + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_SKYT::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[128];
	uint8 pattern[1024];
	uint16 trackStartOffset, trackOffset;
	uint16 sampleOffset;
	uint8 temp;
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

		// Sample size + finetune + volume + loop start
		destFile->Write(&mod[i * 8], 6);

		// Replen
		if (*((uint16 *)&mod[i * 8 + 6]) != 0)
			destFile->Write(&mod[i * 8 + 6], 2);
		else
			destFile->Write_B_UINT16(0x0001);
	}

	// Write position table length
	pattNum = mod[0x104] + 1;
	destFile->Write_UINT8(pattNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	for (i = 0; i < pattNum; i++)
		destFile->Write_UINT8(i);

	destFile->Write(zeroBuf, 128 - pattNum);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert the pattern data
	trackStartOffset = 0x106 + pattNum * 2 * 4;
	sampleOffset     = 0;

	for (i = 0; i < pattNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Loop the pattern
		for (j = 0; j < 4; j++)
		{
			// Get track offset
			trackOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x106 + i * 2 * 4 + j * 2]));
			if (trackOffset > sampleOffset)
				sampleOffset = trackOffset;

			if (trackOffset != 0)
				trackOffset -= 0x0100;

			trackOffset += trackStartOffset;

			for (k = 0; k < 64; k++)
			{
				// Convert sample number
				temp = mod[trackOffset + 1] & 0x1f;
				if (temp >= 0x10)
				{
					pattern[k * 16 + j * 4] = 0x10;
					temp -= 0x10;
				}

				pattern[k * 16 + j * 4 + 2] = temp << 4;

				// Convert note
				temp = mod[trackOffset];
				if (temp != 0)
				{
					temp--;
					pattern[k * 16 + j * 4]    |= period[temp][0];
					pattern[k * 16 + j * 4 + 1] = period[temp][1];
				}

				// Convert effect + effect value
				pattern[k * 16 + j * 4 + 2] |= (mod[trackOffset + 2] & 0x0f);
				pattern[k * 16 + j * 4 + 3]  = mod[trackOffset + 3];

				// Take next line
				trackOffset += 4;
			}
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[trackStartOffset + sampleOffset], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_SKYT::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_SKYT);
}
