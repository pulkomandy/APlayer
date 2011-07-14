/******************************************************************************/
/* ProWizard PRU2 class.                                                      */
/*                                                                            */
/* ProRunner v2 format.                                                       */
/* Created by Cosmos / Sanity (1992)                                          */
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
uint32 PROZ_PRU2::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp, temp1;
	uint16 i;
	uint32 sampOffset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 8]));
		if (temp > 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[8 + i * 8 + 3] > 0x40) || (mod[8 + i * 8 + 2] > 0x0f))
			return (0);

		// Check repeat values
		if (temp != 0)
		{
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 8 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 8 + 6]))) > temp)
				return (0);
		}
	}

	// Check the module length
	sampOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4]));
	if ((sampOffset + sampSize) > (module.GetLength() + 256))
		return (0);

	// Check NTK byte
	if (mod[0x101] != 0x7f)
		return (0);

	// Check number of positions
	posNum = mod[0x100];
	if ((posNum == 0) || (posNum > 0x7f))
		return (0);

	// Check the position table and find heighest pattern number
	pattNum = 0;
	for (i = 0; i < posNum; i++)
	{
		if (mod[0x102 + i] > 0x3f)
			return (0);

		if (mod[0x102 + i] > pattNum)
			pattNum = mod[0x102 + i];
	}

	pattNum++;

	// Check pattern pointer table
	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x282])) != 0)
		return (0);

	if (pattNum >= 2)
	{
		for (i = 0; i < (pattNum - 1); i++)
		{
			// Get two offsets
			temp  = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x282 + i * 2]));
			temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x282 + i * 2 + 2]));

			if (temp1 >= sampOffset)
				return (0);

			if (temp >= temp1)
				return (0);
		}
	}

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
ap_result PROZ_PRU2::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint8 prevNote[4][4];
	uint32 pattOffset;
	uint8 temp;
	uint8 patt, patt1, patt2, patt3;
	uint16 i, j, k;

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
		destFile->Write(&mod[8 + i * 8], 8);
	}

	// Write position table length, NTK byte and the position table itself
	destFile->Write(&mod[0x100], 1 + 1 + 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert the pattern data
	for (i = 0; i < pattNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Find pattern offset
		pattOffset = 0x302 + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x282 + i * 2]));

		for (j = 0; j < 64; j++)
		{
			for (k = 0; k < 4; k++)
			{
				// Initialize pattern bytes
				patt  = 0;
				patt1 = 0;
				patt2 = 0;
				patt3 = 0;

				// Get first byte
				temp = mod[pattOffset++];

				// Empty note
				if (temp == 0x80)
					continue;

				// Copy previous note
				if (temp == 0xc0)
				{
					pattern[j * 16 + k * 4]     = prevNote[k][0];
					pattern[j * 16 + k * 4 + 1] = prevNote[k][1];
					pattern[j * 16 + k * 4 + 2] = prevNote[k][2];
					pattern[j * 16 + k * 4 + 3] = prevNote[k][3];
					continue;
				}

				// Normal pattern line
				if (temp & 0x1)
				{
					patt2 = 0x10;		// Low bit in sample number
					temp &= 0xfe;
				}

				// Convert note
				if (temp != 0)
				{
					temp -= 2;
					temp /= 2;
					patt  = period[temp][0];
					patt1 = period[temp][1];
				}

				// Get sample number
				temp = (mod[pattOffset] & 0xf0) >> 3;
				if (temp >= 0x10)
				{
					patt |= 0x10;		// Hi bit in sample number
					temp -= 0x10;
				}

				patt2 |= temp << 4;

				// Get the effect
				patt2 |= mod[pattOffset++] & 0x0f;
				patt3  = mod[pattOffset++];

				// Store the line in the pattern
				pattern[j * 16 + k * 4]     = patt;
				pattern[j * 16 + k * 4 + 1] = patt1;
				pattern[j * 16 + k * 4 + 2] = patt2;
				pattern[j * 16 + k * 4 + 3] = patt3;

				// Remember the line
				if ((patt != 0) || (patt1 != 0) || (patt2 != 0) || (patt3 != 0))
				{
					prevNote[k][0] = patt;
					prevNote[k][1] = patt1;
					prevNote[k][2] = patt2;
					prevNote[k][3] = patt3;
				}
			}
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4]))], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PRU2::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PRU2);
}
