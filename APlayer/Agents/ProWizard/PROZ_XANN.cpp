/******************************************************************************/
/* ProWizard XANN class.                                                      */
/*                                                                            */
/* XANN Packer format.                                                        */
/* Created by XANN / The Silents (1992)                                       */
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
uint32 PROZ_XANN::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp, temp1, temp4;
	uint16 temp2, temp3;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the two first pattern offsets
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]));
	if (temp > 0xf8000)
		return (0);

	temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4]));
	if ((temp1 == 0) || (temp1 > 0xf8000))
		return (0);

	if (temp1 > temp)
		temp = temp1 - temp;
	else
		temp -= temp1;

	temp1 = (temp / 1024) * 1024;
	if (temp1 != temp)
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check volume & finetune
		if ((mod[0x206 + i * 16 + 1] > 0x40) || (mod[0x206 + i * 16] > 0x0f))
			return (0);

		// Check sample size + replen
		temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x206 + i * 16 + 12]));
		temp3 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x206 + i * 16 + 6]));

		if (temp2 == 0)
		{
			if (temp3 != 0x0001)
				return (0);

			if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x206 + i * 16 + 2])) != P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x206 + i * 16 + 8])))
				return (0);
		}
		else
		{
			if (temp2 > 0x8000)
				return (0);

			// replen > length?
			if (temp3 > temp2)
				return (0);

			sampSize += (temp2 * 2);
		}
	}

	// Check first pattern
	for (i = 0; i < 1024; i += 4)
	{
		// Check note
		if (mod[0x43c + i + 1] > 0x48)
			return (0);

		// Check volume command if any
		if (mod[0x43c + i + 2] == 0x48)
		{
			if (mod[0x43c + i + 3] > 0x40)
				return (0);
		}
	}

	// Check pattern offsets
	lowPattOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]));
	hiPattOffset  = lowPattOffset;
	posNum        = 1;

	for (i = 1; i < 127; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 4]));
		if (temp != 0)
		{
			if (temp < lowPattOffset)
				lowPattOffset = temp;

			if (temp > hiPattOffset)
				hiPattOffset = temp;

			posNum++;
		}
	}

	temp = hiPattOffset - lowPattOffset;	// Pattern difference
	if (temp == 0)
		return (0);

	temp1 = (temp / 1024) * 1024;
	if (temp != temp1)
		return (0);

	pattNum = (temp / 1024) + 1;

	// Find "origine"
	origine = lowPattOffset & 0xf000;

	// Build new pattern offsets
	for (i = 0; i < posNum; i++)
		newPattOffset[i] = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[i * 4])) - origine;

	// Check sample addresses
	temp4 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x20e]));
	if ((hiPattOffset + 1024) != temp4)
		return (0);

	for (i = 0; i < 30; i++)
	{
		// Get address to sample
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x206 + i * 16 + 8]));
		if (temp == 0)
			return (0);

		// Get address to next sample
		temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x206 + (i + 1) * 16 + 8]));
		if (temp1 < temp)
			return (0);
	}

	if (temp1 == temp4)
		return (0);

	// Convert the sample addresses
	for (i = 0; i < 31; i++)
	{
		newSampLoopAddr[i] = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x206 + i * 16 + 2])) - origine;
		newSampAddr[i]     = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x206 + i * 16 + 8])) - origine;
	}

	// Check the module length
	if ((newSampAddr[0] + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_XANN::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[128];
	uint8 pattern[1024];
	uint32 pattOffset;
	uint8 temp, temp1;
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
		destFile->Write(&mod[0x206 + i * 16 + 12], 2);

		// Write finetune + volume
		destFile->Write(&mod[0x206 + i * 16], 2);

		// Write repeat start
		destFile->Write_B_UINT16((newSampLoopAddr[i] - newSampAddr[i]) / 2);

		// Write replen
		destFile->Write(&mod[0x206 + i * 16 + 6], 2);
	}

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	for (i = 0; i < posNum; i++)
		destFile->Write_UINT8((newPattOffset[i] + origine - lowPattOffset) / 1024);

	destFile->Write(zeroBuf, 128 - posNum);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert the pattern data
	pattOffset = lowPattOffset - origine;
	for (i = 0; i < pattNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		for (j = 0; j < (64 * 4); j++)
		{
			// Get sample number
			temp = mod[pattOffset++] >> 3;
			if (temp >= 0x10)
			{
				pattern[j * 4] = 0x10;	// Hi bit in sample number
				temp -= 0x10;
			}

			pattern[j * 4 + 2] = temp << 4;

			// Get note number
			temp = mod[pattOffset++];
			if (temp != 0)
			{
				temp -= 2;
				temp /= 2;
				pattern[j * 4]    |= period[temp][0];
				pattern[j * 4 + 1] = period[temp][1];
			}

			// Get effect + value
			temp  = mod[pattOffset++];
			temp1 = mod[pattOffset++];

			// Convert the effect
			switch (temp)
			{
				// Effect 0x1??
				case 0x08:
				{
					temp = 0x1;
					break;
				}

				// Effect 0x2??
				case 0x0c:
				{
					temp = 0x2;
					break;
				}

				// Effect 0x300
				case 0x10:
				{
					temp  = 0x3;
					temp1 = 0;
					break;
				}

				// Effect 0x3??
				case 0x14:
				{
					temp = 0x3;
					break;
				}

				// Effect 0x400
				case 0x18:
				{
					temp  = 0x4;
					temp1 = 0;
					break;
				}

				// Effect 0x4??
				case 0x1c:
				{
					temp = 0x4;
					break;
				}

				// Effect 0x5?0
				case 0x20:
				{
					temp    = 0x5;
					temp1 <<= 4;
					break;
				}

				// Effect 0x50?
				case 0x24:
				{
					temp = 0x5;
					break;
				}

				// Effect 0x6?0
				case 0x28:
				{
					temp    = 0x6;
					temp1 <<= 4;
					break;
				}

				// Effect 0x60?
				case 0x2c:
				{
					temp = 0x6;
					break;
				}

				// Effect 0x9??
				case 0x38:
				{
					temp = 0x9;
					break;
				}

				// Effect 0xA?0
				case 0x3c:
				{
					temp    = 0xa;
					temp1 <<= 4;
					break;
				}

				// Effect 0xA0?
				case 0x40:
				{
					temp = 0xa;
					break;
				}

				// Effect 0xB??
				case 0x44:
				{
					temp = 0xb;
					break;
				}

				// Effect 0xC??
				case 0x48:
				{
					temp = 0xc;
					break;
				}

				// Effect 0xD??
				case 0x4c:
				{
					temp = 0xd;
					break;
				}

				// Effect 0xF??
				case 0x50:
				{
					temp = 0xf;
					break;
				}

				// Effect 0xE01
				case 0x58:
				{
					temp  = 0xe;
					temp1 = 0x01;
					break;
				}

				// Effect 0xE1?
				case 0x5c:
				{
					temp   = 0xe;
					temp1 |= 0x10;
					break;
				}

				// Effect 0xE2?
				case 0x60:
				{
					temp   = 0xe;
					temp1 |= 0x20;
					break;
				}

				// Effect 0xE6?
				case 0x74:
				case 0x78:
				{
					temp   = 0xe;
					temp1 |= 0x60;
					break;
				}

				// Effect 0xE9?
				case 0x84:
				{
					temp   = 0xe;
					temp1 |= 0x90;
					break;
				}

				// Effect 0xEA?
				case 0x88:
				{
					temp   = 0xe;
					temp1 |= 0xa0;
					break;
				}

				// Effect 0xEB?
				case 0x8c:
				{
					temp   = 0xe;
					temp1 |= 0xb0;
					break;
				}

				// Effect 0xED?
				case 0x94:
				{
					temp   = 0xe;
					temp1 |= 0xd0;
					break;
				}

				// Effect 0xEE?
				case 0x98:
				{
					temp   = 0xe;
					temp1 |= 0xe0;
					break;
				}

				// Unknown or none
				default:
				{
					temp  = 0x0;
					temp1 = 0;
					break;
				}
			}

			// Store the effect
			pattern[j * 4 + 2] |= temp;
			pattern[j * 4 + 3]  = temp1;
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[newSampAddr[0]], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_XANN::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_XANN);
}
