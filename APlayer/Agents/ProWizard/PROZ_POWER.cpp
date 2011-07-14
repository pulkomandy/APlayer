/******************************************************************************/
/* ProWizard POWER class.                                                     */
/*                                                                            */
/* Power Music format.                                                        */
/* Created by Joakim Ogren (19xx)                                             */
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
uint32 PROZ_POWER::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp, temp2;
	int32 temp1;
	uint16 periodNum;
	uint16 i, j;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Start to check the mark
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x438])) != '!PM!')
		return (0);

	// Check size of position table
	if ((mod[0x3b6] == 0) || (mod[0x3b6] > 0x7f))
		return (0);

	// Check first two bytes in the position table
	if ((mod[0x3b8] > 0x3f) || (mod[0x3b9] > 0x3f))
		return (0);

	// Check NTK byte
	if ((mod[0x3b7] > 0x7f) && (mod[0x3b7] != 0xff))
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Take sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 22]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[20 + i * 30 + 25] > 0x40) || (mod[20 + i * 30 + 24] > 0x0f))
			return (0);

		if (temp != 0)
		{
			temp1 = temp - P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 28]));
			if (temp1 < -4)
				return (0);
		}
		else
		{
			temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 28]));
			if (temp1 > 2)
				return (0);
		}
	}

	// Check heighest pattern number
	if (GetPGP(&mod[0x3b8], mod[0x3b6]) > 64)
		return (0);

	// Check the module length
	if ((0x43c + pattNum * 1024 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Init period table
	InitPeriods();

	// Check periods in the first pattern
	periodNum = 0;
	temp2 = pattNum * 256;
	if (temp2 > 0x600)
		temp2 = 0x600;

	for (i = 0; i < temp2; i++)
	{
		// Get period
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x43c + i * 4])) & 0x0fff;

		if (temp != 0)
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
ap_result PROZ_POWER::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	int8 *sampData;
	uint32 offset;
	uint32 i;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Well, the whole header is equal to ProTracker, so just copy it
	destFile->Write(mod, 0x3b7);

	// Copy NTK byte
	if (mod[0x3b7] == 0xff)
		destFile->Write_UINT8(0x7f);
	else
		destFile->Write_UINT8(mod[0x3b7]);

	// Copy the rest of the header
	destFile->Write(&mod[0x3b8], 0x438 - 0x3b8);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Write pattern data
	destFile->Write(&mod[0x43c], pattNum * 1024);

	// Convert sample data
	sampData = new int8[sampSize];
	if (sampData == NULL)
		throw PMemoryException();

	try
	{
		offset = 0x43c + pattNum * 1024;
		memset(sampData, 0, sampSize);
		sampData[0] = mod[offset];

		for (i = 1; i < sampSize; i++)
			sampData[i] = sampData[i - 1] + (int8)mod[offset + i];

		// Write sample data
		destFile->Write(sampData, sampSize);
	}
	catch(...)
	{
		delete[] sampData;
		throw;
	}

	// Delete buffer again
	delete[] sampData;

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_POWER::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_POWER);
}
