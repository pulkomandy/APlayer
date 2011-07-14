/******************************************************************************/
/* ProWizard UNIC class.                                                      */
/*                                                                            */
/* Unic Tracker format.                                                       */
/* Created by Anders E. Hansen (Laxity / Kefrens) (1991-1993)                 */
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
uint32 PROZ_UNIC::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	bool ok;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the position table size
	if ((mod[0x3b6] > 0x7f) || (mod[0x3b6] == 0x00))
		return (0);

	// Check NTK byte
	if ((mod[0x3b7] > 0x3f) && (mod[0x3b7] != 0x7f))
		return (0);

	// Check the first 2 pattern numbers in the position table
	if ((mod[0x3b8] > 0x3f) || (mod[0x3b9] > 0x3f))
		return (0);

	// Check for special mark
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x438])) == 'SNT.')
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check volume & finetune
		if ((mod[20 + i * 30 + 25] > 0x40) || (mod[20 + i * 30 + 24] != 0x00))
			return (0);

		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 22]));
		if (temp >= 0x8000)
			return (0);

		if (temp == 0)
		{
			// Check loop length
			if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 28])) != 0x0001)
				return (0);
		}
		else
		{
			sampSize += (temp * 2);

			// Check loop
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 30 + 28]))) > temp)
				return (0);
		}
	}

	// Find heighest pattern
	if (GetPGP(&mod[0x3b8], 127) > 0x40)
		return (0);

	// Check module size
	if ((0x43c + pattNum * 0x300 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Check for ProTracker patterns
	if (CheckPatterns(&mod[0x43c]))
		return (0);

	// Check Unic pattern format
	ok = true;
	for (i = 0; i < (pattNum * 256); i++)
	{
		// Check note
		if (mod[0x43c + i * 3] > 0x66)
		{
			ok = false;
			break;
		}
	}

	// Set type
	type = 'UNIC';

	if (!ok)
	{
		// Well, try again without mark. Maybe it's a Unic without mark
		for (i = 0; i < (pattNum * 256); i++)
		{
			// Check note
			if (mod[0x438 + i * 3] > 0x66)
				return (0);
		}

		// Set type
		type = 'UNIa';
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
ap_result PROZ_UNIC::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint8 temp;
	uint16 temp1;
	bool setFinetune;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write module name
	if ((type == 'UNIC') || (type == 'UNIa'))
	{
		destFile->Write(&mod[0], 20);
		mod += 20;
	}
	else
		destFile->Write(zeroBuf, 20);

	// Should we set finetune?
	setFinetune = ((mod[31 * 30 + 1] == 0x00) || (mod[31 * 8 + 1] == 0x7f) ? true : false);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Write sample name
		destFile->Write(&mod[0], 20);
		destFile->Write(zeroBuf, 2);

		// Write sample length
		destFile->Write(&mod[22], 2);

		// Write finetune
		if (setFinetune)
		{
			temp = (-(int8)mod[21]) & 0x0f;
			destFile->Write_UINT8(temp);
		}
		else
			destFile->Write_UINT8(0);

		// Write volume
		destFile->Write_UINT8(mod[25]);

		// Write loop start
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[26]));

		if (((temp1 * 2) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[28]))) > P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[22])))
			destFile->Write_B_UINT16(temp1);
		else
			destFile->Write_B_UINT16(temp1 * 2);

		// Write loop length
		destFile->Write(&mod[28], 2);

		mod += 30;
	}

	// Write position table length
	destFile->Write_UINT8(mod[0]);

	// Write NTK byte
	destFile->Write_UINT8(mod[1]);

	// Write position table
	destFile->Write(&mod[2], 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Set pointer to start of pattern data
	mod += 130;
	if (type == 'UNIC')
		mod += 4;

	// Begin to convert the patterns
	for (i = 0; i < pattNum; i++)
	{
		// Clear pattern
		memset(pattern, 0, sizeof(pattern));

		for (j = 0; j < (64 * 4); j++)
		{
			// Get sample number
			if (mod[i * 0x300 + j * 3] & 0x40)
				byte1 = 0x10;
			else
				byte1 = 0x00;

			// Low 4 bit of sample number, effect + effect value
			byte3 = mod[i * 0x300 + j * 3 + 1];
			byte4 = mod[i * 0x300 + j * 3 + 2];

			// Get note
			byte2 = mod[i * 0x300 + j * 3] & 0x3f;
			if ((byte2 != 0x00) && (byte2 != 0x3f))
			{
				byte2--;
				byte1 |= period[byte2][0];
				byte2  = period[byte2][1];
			}
			else
				byte2 = 0x00;

			// Convert the pattern break effect argument from hex to decimal
			if ((byte3 & 0x0f) == 0xd)
				byte4 = (byte4 / 10) * 0x10 + (byte4 % 10);

			// Copy the pattern data
			pattern[j * 4]     = byte1;
			pattern[j * 4 + 1] = byte2;
			pattern[j * 4 + 2] = byte3;
			pattern[j * 4 + 3] = byte4;
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[pattNum * 0x300], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_UNIC::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_UNIC);
}



/******************************************************************************/
/* CheckPatterns() will check the patterns to see if they are in PTK format.  */
/*                                                                            */
/* Input:  "patt" is a pointer to the patterns.                               */
/*                                                                            */
/* Output: True if there is PTK patterns, false if not.                       */
/******************************************************************************/
bool PROZ_UNIC::CheckPatterns(const uint8 *patt)
{
	bool gotNote = false;
	uint32 temp;
	uint16 i;

	for (i = 0; i < (pattNum * 128); i++)
	{
		// Get the note
		temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&patt[i * 4]));
		if (temp != 0)
		{
			gotNote = true;
			temp   &= 0x0fff0000;

			if (temp != 0)
			{
				if ((temp < 0x00710000) || (temp > 0x03580000))
					return (false);
			}
		}
	}

	if (gotNote)
		return (true);

	return (false);
}
