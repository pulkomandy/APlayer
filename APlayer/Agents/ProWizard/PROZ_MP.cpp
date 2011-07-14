/******************************************************************************/
/* ProWizard MP class.                                                        */
/*                                                                            */
/* Module Protector format.                                                   */
/* Created by Matrix / LSD (David Counter 1992)                               */
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
uint32 PROZ_MP::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 periodNum;
	uint16 temp;
	uint16 i, j, k;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Some Module Protector modules have an ID in the beginning. Check for this
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) == 'TRK1')
		mod += 4;	// Skip the ID

	// Call Cryptoburner check
	if (!(Check(mod, &mod[0xf8])))
		return (0);

	// Get the size of the position table
	posNum = mod[0xf8];

	// Check heighest pattern number
	if (GetPGP(&mod[0xfa], posNum) > 64)
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8]));
		sampSize += (temp * 2);

		if (temp == 0)
		{
			// Check replen
			if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 8 + 6])) != 0x0001)
				return (0);
		}
	}

	// Init period table
	InitPeriods();

	// Check periods in the first pattern
	periodNum = 0;
	for (i = 0; i < pattNum; i++)
	{
		// Check first 16 notes in each pattern
		for (j = 0; j < (16 * 4); j++)
		{
			temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x17a + i * 1024 + j * 4])) & 0x0fff;
			if (temp != 0)
			{
				uint8 hi = (temp & 0xff00) >> 8;
				uint8 lo = temp & 0x00ff;
				bool found = false;

				for (k = 0; k < 36; k++)
				{
					// Try to find the period in the period table
					if ((period[k][0] == hi) && (period[k][1] == lo))
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
	}

	if (periodNum == 0)
		return (0);

	// Check the module length
	if ((0x17a + pattNum * 1024 + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_MP::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint32 pattOffset;
	uint16 i;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Skip ID if any
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) == 'TRK1')
		mod += 4;

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample informations
		destFile->Write(&mod[i * 8], 8);
	}

	// Write position table length
	destFile->Write_UINT8(mod[0xf8]);

	// Write NTK byte
	destFile->Write_UINT8(mod[0xf9]);

	// Write position table
	destFile->Write(&mod[0xfa], 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Find the pattern offset
	pattOffset = 0x17a;
	if (*((uint32 *)&mod[0x17a]) == 0)
		pattOffset += 4;

	// Write pattern data
	destFile->Write(&mod[pattOffset], pattNum * 1024);

	// Write sample data
	destFile->Write(&mod[pattOffset + pattNum * 1024], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_MP::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_MP);
}
