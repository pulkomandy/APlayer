/******************************************************************************/
/* ProWizard LAX class.                                                       */
/*                                                                            */
/* Laxity Tracker format.                                                       */
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
uint32 PROZ_LAX::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the position table size
	if ((mod[0x3a2] > 0x7f) || (mod[0x3a2] == 0x00))
		return (0);

	// Check NTK byte
	if ((mod[0x3a3] > 0x3f) && (mod[0x3a3] != 0x7f))
		return (0);

	// Check the first 2 pattern numbers in the position table
	if ((mod[0x3a4] > 0x3f) || (mod[0x3a5] > 0x3f))
		return (0);

	// No 'M.K.' in a Laxity module
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0x438])) == 'M.K.')
		return (0);

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check volume & finetune
		if ((mod[i * 30 + 25] > 0x40) || (mod[i * 30 + 24] != 0x00))
			return (0);

		// Check sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 30 + 22]));
		if (temp >= 0x8000)
			return (0);

		if (temp == 0)
		{
			// Check loop length
			if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 30 + 28])) != 0x0001)
				return (0);
		}
		else
		{
			sampSize += (temp * 2);

			// Check loop
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 30 + 26])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[i * 30 + 28]))) > temp)
				return (0);
		}
	}

	// Find heighest pattern
	if (GetPGP(&mod[0x3a4], mod[0x3a2]) > 0x40)
		return (0);

	// Check module size
	if ((0x424 + pattNum * 0x300 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Check for ProTracker patterns
	if (CheckPatterns(&mod[0x424]))
		return (0);

	// Check Unic pattern format
	for (i = 0; i < (pattNum * 256); i++)
	{
		// Check note
		if (mod[0x424 + i * 3] > 0x66)
			return (0);
	}

	// Set type
	type = 'LAXI';

	// Calculate the total size of the PTK module
	calcSize = pattNum * 1024 + sampSize + 1084;

	return (calcSize);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_LAX::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_LAX);
}
