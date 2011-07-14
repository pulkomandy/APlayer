/******************************************************************************/
/* ProWizard EUREKA class.                                                    */
/*                                                                            */
/* Eureka Packer format.                                                      */
/* Created by Eureka / Concept (1990-1991)                                    */
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
uint32 PROZ_EUREKA::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp;
	uint16 temp1, temp2;
	uint32 offset;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check position length
	if ((mod[950] > 0x7f) || (mod[950] == 0x00))
		return (0);

	// Check NTK byte
	if ((mod[951] != 0x7f) && (mod[951] != 0x00))
		return (0);

	// Check first two patterns in the position table
	if ((mod[952] > 0x3f) || (mod[953] > 0x3f))
		return (0);

	// No mark is allowed
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[1080]));
	if ((temp == 'M.K.') || (temp == 'SNT.'))
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Get sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[42 + i * 30]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check repeat + replen
		if (temp != 0)
		{
			if (((uint32)P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[42 + i * 30 + 4])) + (uint32)P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[42 + i * 30 + 6]))) > temp)
				return (0);
		}

		// Check volume & finetune
		if ((mod[42 + i * 30 + 3] > 0x40) || (mod[42 + i * 30 + 2] > 0x0f))
			return (0);
	}

	// Check the module length
	if ((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[1080])) + sampSize) > (module.GetLength() + 256))
		return (0);

	// Check number of patterns
	if (GetPGP(&mod[952], mod[950]) > 64)
		return (0);

	// Check the pattern offsets to see if they are in ascending order
	offset = 1084;
	temp1  = 0;

	for (i = 0; i < (pattNum * 4); i++)
	{
		temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));
		if (temp2 < temp1)
			return (0);

		temp1   = temp2;
		offset += 2;
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
ap_result PROZ_EUREKA::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 pattern[1024];
	uint32 sampOffset;
	uint16 i, j, k;
	const uint16 *trackTable;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Well, the header is the same, so just copy it
	destFile->Write(&mod[0], 1080);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Get the offset to the samples
	sampOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[1080]));

	// Get the track table pointer
	trackTable = (const uint16 *)&mod[1084];

	// Now it's time to convert the pattern data
	for (i = 0; i < pattNum; i++)
	{
		uint32 trackOffset, writeOffset;
		uint8 tempByte;

		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Loop each channel
		for (j = 0; j < 4; j++)
		{
			// Find the track + write offset
			trackOffset = P_BENDIAN_TO_HOST_INT16(*trackTable++);
			writeOffset = j * 4;

			// Loop each line
			for (k = 0; k < 64; k++)
			{
				tempByte = mod[trackOffset] & 0xf0;

				if ((tempByte == 0x00) || (tempByte == 0x10))
				{
					// Entire note
					pattern[writeOffset]     = mod[trackOffset];
					pattern[writeOffset + 1] = mod[trackOffset + 1];
					pattern[writeOffset + 2] = mod[trackOffset + 2];
					pattern[writeOffset + 3] = mod[trackOffset + 3];

					// Update offsets
					trackOffset += 4;
					writeOffset += 16;
					continue;
				}

				if (tempByte == 0x40)
				{
					// Only effect
					pattern[writeOffset + 2] = mod[trackOffset] & 0x0f;
					pattern[writeOffset + 3] = mod[trackOffset + 1];

					// Update offsets
					trackOffset += 2;
					writeOffset += 16;
					continue;
				}

				if (tempByte == 0x80)
				{
					// Only note
					pattern[writeOffset]     = mod[trackOffset + 1];
					pattern[writeOffset + 1] = mod[trackOffset + 2];
					pattern[writeOffset + 2] = (mod[trackOffset] & 0x0f) << 4;

					// Update offsets
					trackOffset += 3;
					writeOffset += 16;
					continue;
				}

				// Empty lines
				tempByte     = mod[trackOffset] - 0xc0;
				writeOffset += ((tempByte + 1) * 16);
				k           += tempByte;
				trackOffset++;
			}
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
int32 PROZ_EUREKA::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_EUREKA);
}
