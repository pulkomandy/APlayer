/******************************************************************************/
/* ProWizard DI class.                                                        */
/*                                                                            */
/* Digital Illusions format.                                                  */
/* Created by TSL (The Silents) for Digital Illusion (1991)                   */
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
uint32 PROZ_DI::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp;
	uint16 temp1, temp2;
	uint32 offset;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check number of samples
	sampNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0]));
	if (sampNum > 0x1f)
		return (0);

	// Check upper word on all the offsets
	if ((*((uint16 *)&mod[2]) != 0x0000) || (*((uint16 *)&mod[6]) != 0x0000) || (*((uint16 *)&mod[10]) != 0x0000))
		return (0);

	// Check lower word on all the offsets
	if ((*((uint16 *)&mod[4]) == 0x0000) || (*((uint16 *)&mod[8]) == 0x0000) || (*((uint16 *)&mod[12]) == 0x0000))
		return (0);

	// Check pattern data offset
	if (P_BENDIAN_TO_HOST_INT16(*(uint16 *)&mod[8]) != P_BENDIAN_TO_HOST_INT16(*(uint16 *)&mod[14 + sampNum * 8]))
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < sampNum; i++)
	{
		// Get sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[14 + i * 8]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[14 + i * 8 + 3] > 0x40) || (mod[14 + i * 8 + 2] > 0x0f))
			return (0);

		// Check repeat + replen
		if (temp != 0)
		{
			if (((uint32)P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[14 + i * 8 + 4])) + (uint32)P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[14 + i * 8 + 6]))) > temp)
				return (0);
		}
	}

	// Check "end mark" in the position table
	if (mod[P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[6])) - 1] != 0xff)
		return (0);

	// Get the position table offset
	offset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[2]));

	// Find number of positions + number of patterns
	pattNum = 0;
	posNum  = 0;
	while (mod[offset + posNum] != 0xff)
	{
		if (mod[offset + posNum] > pattNum)
			pattNum = mod[offset + posNum];

		posNum++;
	}

	pattNum++;

	// Check the pattern offsets to see if they are in ascending order
	if (pattNum > 2)
	{
		offset = 14 + sampNum * 8;
		temp1  = 0;

		for (i = 0; i < pattNum; i++)
		{
			temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));
			if (temp2 < temp1)
				return (0);

			temp1   = temp2;
			offset += 2;
		}
	}

	// Check the module length
	if ((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10])) + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_DI::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[128];
	uint8 pattern[1024];
	uint32 pattOffset, posOffset, pattDataOffset, sampOffset;
	uint16 i;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample info
	for (i = 0; i < sampNum; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample info
		destFile->Write(&mod[14 + i * 8], 8);
	}

	// Write the rest of the sample info
	for (; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample info
		destFile->Write_B_UINT16(0);	// Sample length
		destFile->Write_B_UINT16(0);	// Finetune + Volume
		destFile->Write_B_UINT16(0);	// Repeat
		destFile->Write_B_UINT16(1);	// Replen
	}

	// Write song length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write pattern numbers
	posOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[2]));
	destFile->Write(&mod[posOffset], posNum);
	destFile->Write(zeroBuf, 128 - posNum);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Now it's time to convert the pattern data
	//
	// Get all the offsets
	pattOffset = 14 + sampNum * 8;
	sampOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10]));

	for (;;)
	{
		uint32 writeOffset = 0;

		if (pattOffset >= posOffset)
			break;		// Done with all the patterns

		// Get offset to the pattern data
		pattDataOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[pattOffset]));
		pattOffset    += 2;

		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		for (i = 0; i < 64 * 4; i++)
		{
			uint8 tempByte, sampNum, noteNum;

			// Check for invalid offset. We will then stop the filling
			if (pattDataOffset >= sampOffset)
				break;

			// Get first pattern byte
			tempByte = mod[pattDataOffset++];
			if (tempByte == 0xff)
			{
				// Blank note
				writeOffset += 4;
				continue;
			}

			if (tempByte < 0x80)
			{
				// Sample + note + effect without value
				sampNum = (tempByte >> 2) & 0x1f;
				noteNum = ((mod[pattDataOffset] & 0xf0) >> 4) | ((tempByte & 0x03) << 4);

				pattern[writeOffset + 2]  = mod[pattDataOffset] & 0x0f;
				pattern[writeOffset + 2] |= ((sampNum & 0x0f) << 4);

				if (noteNum > 0)
				{
					pattern[writeOffset + 1]  = period[noteNum - 1][1];
					pattern[writeOffset]      = period[noteNum - 1][0];
				}

				if (sampNum >= 0x10)
					pattern[writeOffset] |= 0x10;

				pattDataOffset++;
				writeOffset += 4;
				continue;
			}

			// Sample + note + effect with value
			sampNum = (tempByte >> 2) & 0x1f;
			noteNum = ((mod[pattDataOffset] & 0xf0) >> 4) | ((tempByte & 0x03) << 4);

			pattern[writeOffset + 3]  = mod[pattDataOffset + 1];
			pattern[writeOffset + 2]  = mod[pattDataOffset] & 0x0f;
			pattern[writeOffset + 2] |= ((sampNum & 0x0f) << 4);

			if (noteNum > 0)
			{
				pattern[writeOffset + 1]  = period[noteNum - 1][1];
				pattern[writeOffset]      = period[noteNum - 1][0];
			}

			if (sampNum >= 0x10)
				pattern[writeOffset] |= 0x10;

			pattDataOffset += 2;
			writeOffset    += 4;
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[sampOffset], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_DI::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_DI);
}
