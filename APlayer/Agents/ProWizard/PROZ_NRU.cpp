/******************************************************************************/
/* ProWizard NRU class.                                                       */
/*                                                                            */
/* NoiseRunner format.                                                        */
/* Created by Chaos / Sanity (19xx)                                           */
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
uint32 PROZ_NRU::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp, temp1;
	uint16 temp2;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check pointer to sample
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16 + 2]));
		if (temp == 0)
			return (0);

		// Check loop pointer
		temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16 + 8]));
		if (temp1 == 0)
			return (0);

		// Take sample size
		temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 6]));
		if (temp2 == 0)
		{
			if (temp != temp1)
				return (0);
		}
		else
		{
			// Check loop size
			temp = (temp1 - temp) / 2 + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 12]));
			if (temp > temp2)
				return (0);
		}

		sampSize += (temp2 * 2);

		// Check volume
		if (P_BENDIAN_TO_HOST_INT16(*(uint16 *)&mod[i * 16]) > 0x40)
			return (0);
	}

	// Check size of position table
	if ((mod[0x3b6] == 0) || (mod[0x3b6] > 0x7f))
		return (0);

	// Check NTK byte
	if ((mod[0x3b7] != 0x7f) && (mod[0x3b7] != 0x00))
		return (0);

	// Check first two bytes in the position table
	if ((mod[0x3b8] > 0x3f) || (mod[0x3b9] > 0x3f))
		return (0);

	// Check some of the "left-over" in the module
	temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3ae]));	// Sample length
	if (temp2 >= 0x8000)
		return (0);

	if (temp2 == 0)
	{
		temp2  = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3b0]));	// Finetune + volume
		temp2 += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3b2]));	// Repeat
		temp2 += P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3b4]));	// Replen
		if (temp2 != 1)
			return (0);
	}
	else
	{
		if ((mod[0x3b0] > 0x0f) || (mod[0x3b1] > 0x40))
			return (0);

		if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3b2])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3b4]))) > temp2)
			return (0);
	}

	// Check heighest pattern number
	if (GetPGP(&mod[0x3b8], mod[0x3b6]) > 64)
		return (0);

	// Check the module length
	if ((0x43c + pattNum * 1024 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Check notes values
	for (i = 0; i < (64 * 4); i++)
	{
		if (mod[0x43c + i * 4 + 2] > 0x48)
			return (0);
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
ap_result PROZ_NRU::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint16 temp;
	uint8 temp1, fineTune;
	uint16 i, j;

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

		// Sample size
		destFile->Write(&mod[i * 16 + 6], 2);

		// Finetune
		fineTune = 0;
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 16 + 14]));
		if (temp != 0)
		{
			if ((temp & 0xf000) == 0xf000)
			{
				// Yes, we have a finetune
				fineTune = (0x1000 - temp & 0x0fff) / 0x48;
			}
		}

		destFile->Write_UINT8(fineTune);

		// Volume
		destFile->Write_UINT8(mod[i * 16 + 1]);

		// Repeat point
		destFile->Write_B_UINT16((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16 + 8])) - P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 16 + 2]))) / 2);

		// Replen
		destFile->Write(&mod[i * 16 + 12], 2);
	}

	// Write position table length + NTK byte + position table
	destFile->Write(&mod[0x3b6], 1 + 1 + 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	for (i = 0; i < pattNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Loop the pattern
		for (j = 0; j < 64 * 4; j++)
		{
			// Convert sample number
			temp1 = mod[0x43c + i * 1024 + j * 4 + 3] / 8;
			if (temp1 >= 0x10)
			{
				pattern[j * 4] = 0x10;
				temp1 -= 0x10;
			}

			pattern[j * 4 + 2] = temp1 << 4;

			// Convert note
			temp1 = mod[0x43c + i * 1024 + j * 4 + 2];
			if (temp1 != 0)
			{
				temp1 = (temp1 - 2) / 2;
				pattern[j * 4]    |= period[temp1][0];
				pattern[j * 4 + 1] = period[temp1][1];
			}

			// Convert effect value
			pattern[j * 4 + 3] = mod[0x43c + i * 1024 + j * 4 + 1];

			// Convert effect
			temp1 = mod[0x43c + i * 1024 + j * 4];

			switch (temp1)
			{
				// Tone portamento (0 -> 3)
				case 0x0:
				{
					temp1 = 0x3;
					break;
				}

				// No effect (C -> 0)
				case 0xc:
				{
					temp1 = 0x0;
					break;
				}

				// All the other effects
				default:
				{
					temp1 /= 4;
					break;
				}
			}

			pattern[j * 4 + 2] |= temp1;
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[0x43c + pattNum * 1024], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_NRU::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_NRU);
}
