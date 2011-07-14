/******************************************************************************/
/* ProWizard HRT class.                                                       */
/*                                                                            */
/* Hornet Packer format.                                                      */
/* Created by Hornet / Alcatraz (19xx)                                        */
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
uint32 PROZ_HRT::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint32 temp1;
	uint16 numToCheck;
	uint16 i;
	bool gotNotes;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check ID
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x438])) != 'HRT!')
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 22]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);

		// Check volume & finetune
		if ((mod[0x14 + i * 30 + 25] > 0x40) || (mod[0x14 + i * 30 + 24] > 0x0f))
			return (0);

		// Check repeat values
		if (temp != 0)
		{
			if ((temp - (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 28])))) < -0x10)
				return (0);
		}
		else
		{
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x14 + i * 30 + 28]))) > 2)
				return (0);
		}
	}

	// Check number of positions
	posNum = mod[0x3b6];
	if ((posNum == 0) || (posNum > 0x7f))
		return (0);

	// Check first two pattern numbers in the position table
	if ((mod[0x3b8] > 0x3f) || (mod[0x3b9] > 0x3f))
		return (0);

	// Check NTK byte
	if (mod[0x3b7] > 0x7f)
		return (0);

	// Check number of patterns
	if (GetPGP(&mod[0x3b8], 128) > 64)
		return (0);

	// Check the module length
	if ((0x43c + pattNum * 1024 + sampSize) > (module.GetLength() + 256))
		return (0);

	biggestPatt = pattNum * 256;

	// Check the first 5 patterns
	numToCheck = (biggestPatt < 0x500 ? biggestPatt : 0x500);
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
			if (temp > 0x48)
				return (0);

			// Check sample number
			temp = (temp1 & 0xff000000) >> 24;
			if (temp > 0x3e)
				return (0);

			gotNotes = true;
		}
	}

	// Did we get any notes?
	if (!gotNotes)
		return (0);

	// Set the Hornet flag
	hornet = true;

	// Calculate the total size of the PTK module
	calcSize = pattNum * 1024 + sampSize + 1084;

	return (calcSize);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_HRT::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_HRT);
}
