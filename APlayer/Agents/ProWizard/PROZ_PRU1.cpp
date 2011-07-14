/******************************************************************************/
/* ProWizard PRU1 class.                                                      */
/*                                                                            */
/* ProRunner v1 format.                                                       */
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
uint32 PROZ_PRU1::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint32 temp1;
	uint16 i;
	bool gotNotes;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 22]));
		if (temp > 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[0x14 + i * 30 + 25] > 0x40) || (mod[0x14 + i * 30 + 24] > 0x0f))
			return (0);

		// Check repeat values
		if (temp != 0)
		{
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 28]))) > temp)
				return (0);
		}
	}

	// Get number of positions
	posNum = mod[0x3b6];

	// Check the position table and find heighest pattern number
	pattNum = 0;
	for (i = 0; i < posNum; i++)
	{
		if (mod[0x3b8 + i] > 0x3f)
			return (0);

		if (mod[0x3b8 + i] > pattNum)
			pattNum = mod[0x3b8 + i];
	}

	pattNum++;
	biggestPatt = pattNum * 256;

	// Check the module length
	if ((0x43c + biggestPatt * 4 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Check the pattern data
	gotNotes = false;
	for (i = 0; i < biggestPatt; i++)
	{
		temp1 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x43c + i * 4]));
		if (temp1 != 0)
		{
			// The unused nibble has to be zero
			if (temp1 & 0x0000f000)
				return (0);

			// Check note
			temp = (temp1 & 0x00ff0000) >> 16;
			if (temp > 0x24)
				return (0);

			// Check sample number
			temp = (temp1 & 0xff000000) >> 24;
			if (temp > 0x1f)
				return (0);

			gotNotes = true;
		}
	}

	// Did we get any notes?
	if (!gotNotes)
		return (0);

	// Set the Hornet flag
	hornet = false;

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
ap_result PROZ_PRU1::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 pattern[1024];
	uint32 pattOffset;
	uint8 temp;
	uint16 i, j;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Init period table
	InitPeriods();

	// Write module name
	destFile->Write(&mod[0], 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Write sample name
		if (hornet)
		{
			destFile->Write(&mod[20 + i * 30], 18);
			destFile->Write(zeroBuf, 4);
		}
		else
			destFile->Write(&mod[20 + i * 30], 22);

		// Write the rest of the information
		destFile->Write(&mod[20 + i * 30 + 22], 8);
	}

	// Write position table length, NTK byte and the position table itself
	destFile->Write(&mod[0x3b6], 1 + 1 + 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Convert the pattern data
	pattOffset = 0x43c;
	for (i = 0; i < pattNum; i++)
	{
		// Clear the pattern data
		memset(pattern, 0, sizeof(pattern));

		for (j = 0; j < (64 * 4); j++)
		{
			// Get the sample number
			temp = mod[pattOffset++];

			if (hornet)
				temp /= 2;

			if (temp >= 0x10)
			{
				pattern[j * 4] = 0x10;		// Hi bit in sample number
				temp -= 0x10;
			}

			// Store sample number
			pattern[j * 4 + 2] = temp << 4;

			// Convert note
			temp = mod[pattOffset++];

			if (hornet)
				temp /= 2;

			if (temp != 0)
			{
				temp--;
				pattern[j * 4]    |= period[temp][0];
				pattern[j * 4 + 1] = period[temp][1];
			}

			// Copy effect
			pattern[j * 4 + 2] |= mod[pattOffset++];
			pattern[j * 4 + 3]  = mod[pattOffset++];
		}

		// Write the pattern
		destFile->Write(pattern, sizeof(pattern));
	}

	// Write sample data
	destFile->Write(&mod[pattOffset], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_PRU1::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_PRU1);
}
