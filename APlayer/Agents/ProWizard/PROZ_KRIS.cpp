/******************************************************************************/
/* ProWizard KRIS class.                                                      */
/*                                                                            */
/* Kris Tracker (Chip Tracker) format.                                        */
/* Created by Kris / Anarchy (19xx)                                           */
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
/* Period table with extra octaves.                                           */
/******************************************************************************/
static const uint16 krisPeriod[] =
{
	3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1812,
	1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  906,
	 856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480,  453,
	 428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240,  226,
	 214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 107,  101,   95,   90,   85,   80,   75,   71,   67,   63,   60,   56,
	  53,   50,   47,   45,   42,   40,   37,   35,   33,   31,   30,   28
};



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a known module.      */
/*                                                                            */
/* Input:  "module" is a reference to where the packed module is stored.      */
/*                                                                            */
/* Output: Is the unpacked module size or 0 if not recognized.                */
/******************************************************************************/
uint32 PROZ_KRIS::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check mark
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x3b8])) != 'KRIS')
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check volume
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x16 + i * 30 + 24])) > 0x40)
			return (0);

		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x16 + i * 30 + 22]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);
	}

	// Find number of positions
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3bc]));
	if (temp == 0)
		return (0);

	posNum = temp / 256;

	// Find biggest pattern number
	biggestPatt = 0;
	for (i = 0; i < (posNum * 4); i++)
	{
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x3be + i * 2])) & 0xff00;
		if (temp > biggestPatt)
			biggestPatt = temp;
	}

	biggestPatt += 0x0100;

	// Check the module length
	if ((0x7c0 + biggestPatt + sampSize) > (module.GetLength() + 256))
		return (0);

	// Create position table
	Speco(mod);

	// Check first 4 tracks
	temp = 0;
	for (i = 0; i < (4 * 64); i++)
	{
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x7c0 + i * 4])) == 0xa800)
			temp++;		// Count number of empty rows
	}

	if (temp == 0)
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
ap_result PROZ_KRIS::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	int8 lastPatt, transpose;
	uint32 trackOffset;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j, k;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(&mod[0], 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Sample name
		if (mod[0x16 + i * 30] == 0x01)
			destFile->Write(zeroBuf, 22);
		else
			destFile->Write(&mod[0x16 + i * 30], 22);

		// Sample size + volume
		destFile->Write(&mod[0x16 + i * 30 + 22], 4);

		// Loop start
		destFile->Write_B_UINT16(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x16 + i * 30 + 26])) / 2);

		// Loop length
		destFile->Write(&mod[0x16 + i * 30 + 28], 2);
	}

	// Write position table length
	posNum = mod[0x3bc];
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Begin to convert the patterns
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		// Get pattern number to build
		if (posTable[i] > lastPatt)
		{
			lastPatt++;

			// Loop voices
			for (j = 0; j < 4; j++)
			{
				// Get track offset and transpose
				trackOffset = 0x7c0 + mod[0x3be + i * 8 + j * 2] * 256;
				transpose   = (int8)mod[0x3be + i * 8 + j * 2 + 1];

				// Loop rows
				for (k = 0; k < 64; k++)
				{
					// Get the sample number
					byte3 = mod[trackOffset + k * 4 + 1];
					if (byte3 >= 0x10)
					{
						byte1  = 0x10;
						byte3 -= 0x10;
					}
					else
						byte1 = 0x00;

					byte3 <<= 4;

					// Get effect + effect value
					byte3 |= mod[trackOffset + k * 4 + 2];
					byte4  = mod[trackOffset + k * 4 + 3];

					// Get note
					byte2 = mod[trackOffset + k * 4];
					if (byte2 == 0xa8)
					{
						// No note
						byte2 = 0x00;
					}
					else
					{
						// Got a note
						byte2 += (transpose * 2);
						byte2 -= 0x18;
						byte2 /= 2;

						byte1 |= ((krisPeriod[byte2] & 0xff00) >> 8);
						byte2  = krisPeriod[byte2] & 0x00ff;
					}

					// Store the new pattern data
					pattern[k * 16 + j * 4]     = byte1;
					pattern[k * 16 + j * 4 + 1] = byte2;
					pattern[k * 16 + j * 4 + 2] = byte3;
					pattern[k * 16 + j * 4 + 3] = byte4;
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
	}

	// Write sample data
	destFile->Write(&mod[0x7c0 + biggestPatt], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_KRIS::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_KRIS);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_KRIS::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint16 *posTab1, *posTab2;
	uint16 temp1, temp2, temp3, temp4;
	bool found;

	// Jump to the track offset table
	mod += 0x3be;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = (uint16 *)mod + 4;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = *posTab1++;
		temp2 = *posTab1++;
		temp3 = *posTab1++;
		temp4 = *posTab1++;

		// Compare these offsets with the rest to see if there is an equal one
		posTab2 = (uint16 *)mod;
		found   = false;

		for (j = 0; j < i; j++)
		{
			if ((temp1 == posTab2[0]) && (temp2 == posTab2[1]) && (temp3 == posTab2[2]) && (temp4 == posTab2[3]))
			{
				// Found an equal track joinment
				posTable[i] = posTable[j];
				found       = true;
				break;
			}

			// Go to the next offset pair
			posTab2 += 4;
		}

		if (!found)
			posTable[i] = ++pattNum;
	}

	// Add one extra pattern number, because we skipped the first one
	pattNum++;
}
