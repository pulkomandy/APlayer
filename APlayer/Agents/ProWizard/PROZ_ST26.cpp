/******************************************************************************/
/* ProWizard ST26 class.                                                      */
/*                                                                            */
/* SoundTracker 2.6 / IceTracker 1.0 format.                                  */
/* Created by Mnemotron / Spreadpoint (STK 2.6) (1989?)                       */
/*            IcePic (IceTracker 1.0) (1992)                                  */
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
uint32 PROZ_ST26::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Start to check id
	mark = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x5b8]));
	if ((mark != 'IT10') && ((mark & 0xffffff00) != 'MTN\0'))
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 22]));
		if (temp >= 0x8000)
			return (0);

		if (temp != 0)
		{
// TN: Uncommented because my ST26 module won't pass this test!
//			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 28]))) > temp)
//				return (0);

			sampSize += (temp * 2);
		}

		// Check volume & finetune
		if ((mod[20 + i * 30 + 25] > 0x40) || (mod[20 + i * 30 + 24] > 0x0f))
			return (0);
	}

	// Get size of position table
	posNum = mod[0x3b6];
	if (posNum == 0)
		return (0);

	// Get number of tracks
	temp = mod[0x3b7];

	// Find heighest track number
	pattNum = 0;
	for (i = 0; i < posNum * 4; i++)
	{
		if (mod[0x3b8 + i] > pattNum)
			pattNum = mod[0x3b8 + i];
	}

	pattNum++;
	if (pattNum != temp)
		return (0);

	// Check the module length
	if ((0x5bc + pattNum * 256 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Find number of patterns to build
	Speco(mod);

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
ap_result PROZ_ST26::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint32 pattern[256];
	int8 patt, lastPatt;
	const uint32 *trackPoi;
	uint16 i, j, k;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Write the whole header
	destFile->Write(mod, 0x3b6);

	// Write position table length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x00);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Begin to convert the patterns
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		// Get pattern number to build
		patt = posTable[i];
		if (patt <= lastPatt)
			continue;

		lastPatt++;

		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		// Loop voices
		for (j = 0; j < 4; j++)
		{
			// Get track offset
			trackPoi = (uint32 *)(&mod[0x5bc + mod[0x3b8 + i * 4 + j] * 256]);

			// Loop rows
			for (k = 0; k < 64; k++)
				pattern[k * 4 + j] = *trackPoi++;
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[0x5bc + mod[0x3b7] * 256], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_ST26::GetModuleType(void)
{
	return (mark == 'IT10' ? IDS_PROZ_TYPE_ICE10 : IDS_PROZ_TYPE_ST26);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_ST26::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint8 *posTab1, *posTab2;
	uint8 temp1, temp2, temp3, temp4;
	bool found;

	// Jump to the track offset table
	mod += 0x3b8;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = mod + 4;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = *posTab1++;
		temp2 = *posTab1++;
		temp3 = *posTab1++;
		temp4 = *posTab1++;

		// Compare these offsets with the rest to see if there is an equal one
		posTab2 = mod;
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
