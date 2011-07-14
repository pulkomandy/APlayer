/******************************************************************************/
/* ProWizard TP3 class.                                                       */
/*                                                                            */
/* Tracker Packer v2 format.                                                  */
/* Created by Crazy Crack / MEXX (1993-1994)                                  */
/*                                                                            */
/* Tracker Packer v3 format.                                                  */
/* Created by Crazy Crack / Complex (1993-1994)                               */
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
uint32 PROZ_TP3::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp, temp1;
	uint16 i;
	uint32 offset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Start to check id
	type = 3;
	if (memcmp(mod, "CPLX_TP3", 8) != 0)
	{
		type = 2;
		if (memcmp(mod, "MEXX_TP2", 8) != 0)
			return (0);
	}

	// Get number of samples
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1c]));
	if (temp == 0)
		return (0);

	temp /= 8;

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < temp; i++)
	{
		// Check sample size
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1e + i * 8 + 2]));
		if (temp1 == 0)
			return (0);

		sampSize += (temp1 * 2);
	}

	// Get size of position table
	offset  = 0x1e + temp * 8;
	posNum  = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));
	offset += 2;

	// Find heighest pattern number
	pattNum = 0;
	for (i = 0; i < posNum; i++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset])) / 8;
		if (temp > pattNum)
			pattNum = temp;

		offset += 2;
	}

	pattNum++;

	// Store the offsets
	tracks  = offset;
	notes   = tracks + pattNum * 8 + 2;
	samples = notes + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[notes - 2]));

	// Check the module length
	if ((samples + sampSize) > (module.GetLength() + 256))
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
ap_result PROZ_TP3::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[128];
	uint8 pattern[1024];
	uint32 trackOffset, noteOffset;
	uint16 temp;
	uint8 temp1, temp2;
	uint16 i, j, k;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(&mod[8], 20);

	// Write sample informations
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1c])) / 8;
	for (i = 0; i < temp; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample size
		destFile->Write(&mod[0x1e + i * 8 + 2], 2);

		// Write finetune + volume
		destFile->Write(&mod[0x1e + i * 8], 2);

		// Write loop values
		destFile->Write(&mod[0x1e + i * 8 + 4], 4);
	}

	// Write empty samples
	for (; i < 31; i++)
	{
		// Write sample name + size + finetune + volume + repeat point
		destFile->Write(zeroBuf, 28);

		// Write repeat length
		destFile->Write_B_UINT16(1);
	}

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	for (i = 0; i < posNum; i++)
		destFile->Write_UINT8(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x1e + temp * 8 + 2 + i * 2])) / 8);

	destFile->Write(zeroBuf, 128 - posNum);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert the pattern data
	trackOffset = tracks;
	for (i = 0; i < pattNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		for (j = 0; j < 4; j++)
		{
			// Get the offset to the notes
			noteOffset   = notes + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[trackOffset]));
			trackOffset += 2;

			for (k = 0; k < 64; k++)
			{
				// Get first byte
				temp1 = mod[noteOffset++];

				// Is it empty lines?
				if (temp1 >= 0xc0)
				{
					k += -(int8)temp1 - 1;
					continue;
				}

				// Effect only?
				if (temp1 >= 0x80)
				{
					// Yup
					temp1 -= 0x80;
					temp1 >>= 1;
					if (type == 2)
						temp1 >>= 1;

					temp1 &= 0x0f;
					temp2  = mod[noteOffset++];
					ConvertEffect(temp1, temp2);

					pattern[k * 16 + j * 4 + 2] = temp1;
					pattern[k * 16 + j * 4 + 3] = temp2;
					continue;
				}

				// Find out if the sample if >= 0x10
				if (type == 2)
				{
					if (temp1 & 0x01)
					{
						pattern[k * 16 + j * 4] = 0x10;	// Hi bit in sample number
						temp1 &= 0xfe;
					}
				}
				else
				{
					if (temp1 >= 0x5b)
					{
						pattern[k * 16 + j * 4] = 0x10;	// Hi bit in sample number
						temp1 = -(int8)temp1 - 0x81;
					}
				}

				if (temp1 != 0)
				{
					// Got a note, convert it to a period
					if (type == 2)
					{
						temp1 -= 2;
						temp1 /= 2;
					}
					else
						temp1--;

					pattern[k * 16 + j * 4]    |= period[temp1][0];
					pattern[k * 16 + j * 4 + 1] = period[temp1][1];
				}

				// Store sample number + effect + effect value
				pattern[k * 16 + j * 4 + 2] = mod[noteOffset] & 0xf0;
				temp1 = mod[noteOffset++] & 0x0f;

				if (temp1 != 0)
				{
					temp2 = mod[noteOffset++];
					ConvertEffect(temp1, temp2);

					pattern[k * 16 + j * 4 + 2] |= temp1;
					pattern[k * 16 + j * 4 + 3]  = temp2;
				}
			}
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[samples], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_TP3::GetModuleType(void)
{
	return (type == 2 ? IDS_PROZ_TYPE_TP2 : IDS_PROZ_TYPE_TP3);
}



/******************************************************************************/
/* ConvertEffect() convert the mapped effect back to ProTracker and change    */
/*      the effect value if needed.                                           */
/*                                                                            */
/* Input:  "effect" is the effect to convert.                                 */
/*         "effectVal" is the effect value.                                   */
/******************************************************************************/
void PROZ_TP3::ConvertEffect(uint8 &effect, uint8 &effectVal)
{
	switch (effect)
	{
		// Arpeggio
		case 0x8:
		{
			effect = 0;
			break;
		}

		// Volume slide commands
		case 0x05:
		case 0x06:
		case 0x0a:
		{
			if ((int8)effectVal < 0)
				effectVal = -(int8)effectVal;
			else
				effectVal <<= 4;

			break;
		}
	}
}
