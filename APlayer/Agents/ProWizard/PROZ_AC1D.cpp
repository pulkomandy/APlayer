/******************************************************************************/
/* ProWizard AC1D class.                                                      */
/*                                                                            */
/* AC1D packer format.                                                        */
/* Created by Slammer / Anarchy (19xx)                                        */
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
uint32 PROZ_AC1D::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp, i;
	int32 temp1;
	uint32 sampOffset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the ID
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) & 0x00ffffff;
	if ((temp != 0x007fac1d) && (temp != 0x007fd1ca))
		return (0);

	// Check number of positions
	if ((mod[0] == 0) || (mod[0] > 127))
		return (0);

	// Check offset to samples
	sampOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4]));
	if ((sampOffset == 0) || (sampOffset & 0x1))
		return (0);

	// Check the sample information
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Get sample size
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[8 + i * 8 + 0]));
		if (temp1 < 0)
			return (0);

		sampSize += (temp1 * 2);

		// Check volume & finetune
		if ((mod[8 + i * 8 + 3] > 0x40) || (mod[8 + i * 8 + 2] > 0x0f))
			return (0);
	}

	// Check the calculated length
	if ((sampOffset + sampSize) > (module.GetLength() + 256))
		return (0);

	// Find heighest pattern number
	pattNum = 0;
	for (i = 0; i < 128; i++)
	{
		if (mod[0x300 + i] > pattNum)
			pattNum = mod[0x300 + i];
	}

	// Check number of patterns
	pattNum++;
	if (pattNum > 64)
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
ap_result PROZ_AC1D::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod, *tempPoi;
	uint8 zeroBuf[128];
	uint8 pattern[1024];
	uint32 ac1dOffset;
	uint16 i, j, k;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Get the number of positions
	posNum = mod[0];

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample info
	for (i = 0; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample info
		destFile->Write(&mod[8 + i * 8], 8);
	}

	// Write song length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write pattern numbers
	destFile->Write(&mod[0x300], posNum);
	destFile->Write(zeroBuf, 128 - posNum);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Now it's time to convert the pattern data
	//
	// Get the start offset
	ac1dOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x100]));

	// Initialize temporary pointers
	tempPoi = &mod[0x100];

	for (i = 0; i < pattNum; i++)
	{
		uint32 pattOffset, writeOffset;
		uint8 tempByte;

		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Find the pattern offset
		pattOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)tempPoi)) - ac1dOffset + 0x380 + 12;
		tempPoi   += 4;

		// Loop each channel
		for (j = 0; j < 4; j++)
		{
			writeOffset = j * 4;

			// Loop each line
			for (k = 0; k < 64; k++)
			{
				tempByte = mod[pattOffset];
				if (tempByte >= 0x81)
				{
					// Empty lines
					pattOffset++;

					// Skip the lines
					tempByte    -= 0x80;
					writeOffset += (tempByte * 16);
					k           += (tempByte - 1);
					continue;
				}

				if (tempByte == 0x7f)
				{
					// Special 0x7f -> No note, only sample
					pattern[writeOffset]     = 0x10;
					pattern[writeOffset + 1] = 0x00;
					pattern[writeOffset + 2] = mod[pattOffset + 1];
					pattern[writeOffset + 3] = mod[pattOffset + 2];

					// Update offsets
					pattOffset  += 3;
					writeOffset += 16;
					continue;
				}

				if (tempByte == 0x3f)
				{
					// Special 0x3f -> No note, no sample
					pattern[writeOffset]     = 0x00;
					pattern[writeOffset + 1] = 0x00;
					pattern[writeOffset + 2] = mod[pattOffset + 1];
					pattern[writeOffset + 3] = mod[pattOffset + 2];

					// Update offsets
					pattOffset  += 3;
					writeOffset += 16;
					continue;
				}

				if (tempByte >= 0x4c)
				{
					// Note + sample >= 0x10
					pattern[writeOffset] = 0x10;
					tempByte -= 0x40;
				}

				// Normal note
				tempByte -= 0x0c;

				pattern[writeOffset]    |= period[tempByte][0];
				pattern[writeOffset + 1] = period[tempByte][1];

				if ((mod[pattOffset + 1] & 0x0f) == 0x07)
				{
					// No effect, only sample
					pattern[writeOffset + 2] = mod[pattOffset + 1] & 0xf0;

					// Update offsets
					pattOffset  += 2;
					writeOffset += 16;
					continue;
				}

				// Note + sample + effect
				pattern[writeOffset + 2] = mod[pattOffset + 1];
				pattern[writeOffset + 3] = mod[pattOffset + 2];

				// Update offsets
				pattOffset  += 3;
				writeOffset += 16;
			}
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	ac1dOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4]));
	destFile->Write(&mod[ac1dOffset], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_AC1D::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_AC1D);
}
