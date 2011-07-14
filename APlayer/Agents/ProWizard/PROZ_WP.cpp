/******************************************************************************/
/* ProWizard WP class.                                                        */
/*                                                                            */
/* Wanton Packer format.                                                      */
/* Created by Wanton / Bloodsuckers (19xx)                                    */
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
uint32 PROZ_WP::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Start to check id
	if ((mod[0x438] != 'W') || (mod[0x439] != 'N') || (mod[0x43a] != 0x00))
		return (0);

	// Check number of patterns
	pattNum = mod[0x43b];
	if ((pattNum == 0) || (pattNum > 64))
		return (0);

	// Check some of the pattern data in first pattern
	for (i = 0; i < (64 * 4); i++)
	{
		// Check sample number
		if (mod[0x43c + i * 4 + 1] > 0x1f)
			return (0);
	}

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 22]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[0x14 + i * 30 + 25] > 0x40) || (mod[0x14 + i * 30 + 24] > 0x0f))
			return (0);

		// Check repeat values
		if (temp != 0)
		{
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 28]))) > temp)
				return (0);
		}
	}

	// Check the module length
	if ((0x43c + pattNum * 1024 + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_WP::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Write the whole header
	destFile->Write(mod, 0x438);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Get the biggest pattern number
	biggestPatt = mod[0x43b];

	// Convert the pattern data
	ConvertPatternData(&mod[0x43c], destFile);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_WP::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_WP);
}



/******************************************************************************/
/* ConvertPatternData() will convert the pattern data.                        */
/*                                                                            */
/* Input:  "pattStart" is a pointer to the start of the pattern data.         */
/*         "destFile" is where to write the converted data.                   */
/******************************************************************************/
void PROZ_WP::ConvertPatternData(const uint8 *pattStart, PFile *destFile)
{
	uint8 pattern[1024];
	uint16 i, j;
	uint8 temp;

	// Init period table
	InitPeriods();

	for (i = 0; i < biggestPatt; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		for (j = 0; j < 64 * 4; j++)
		{
			// Get the sample number
			temp = pattStart[1];

			if (temp >= 0x10)
			{
				pattern[j * 4] = 0x10;		// Hi bit in sample number
				temp -= 0x10;
			}

			// Store sample number
			pattern[j * 4 + 2] = temp << 4;

			// Convert note
			temp = pattStart[0];
			if (temp != 0)
			{
				temp -= 2;
				temp /= 2;
				pattern[j * 4]    |= period[temp][0];
				pattern[j * 4 + 1] = period[temp][1];
			}

			// Copy effect
			pattern[j * 4 + 2] |= pattStart[2];
			pattern[j * 4 + 3]  = pattStart[3];

			pattStart += 4;
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(pattStart, sampSize);
}
