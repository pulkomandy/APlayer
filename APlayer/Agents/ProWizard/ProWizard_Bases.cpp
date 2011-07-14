/******************************************************************************/
/* APlayer ProWizard base classes.                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Agent headers
#include "ProWizard.h"


/******************************************************************************/
/* BASE_CRYPTO class                                                          */
/******************************************************************************/

/******************************************************************************/
/* Check() makes some standard Cryptoburners checks.                          */
/*                                                                            */
/* Input:  "modStart" is a pointer to the start of the module.                */
/*         "checkStart" is a pointer to start the check.                      */
/*                                                                            */
/* Output: True for success, false if it couldn't be recognized.              */
/******************************************************************************/
bool BASE_CRYPTO::Check(const uint8 *modStart, const uint8 *checkStart)
{
	uint32 temp;
	uint16 temp1;
	uint16 i;

	// Check position table size
	if ((checkStart[0] == 0) || (checkStart[0] > 127))
		return (false);

	// Check first two bytes in the position table
	if ((checkStart[2] > 63) || (checkStart[3] > 63))
		return (false);

	// Check NTK byte
	if ((checkStart[1] != 0x7f) && (checkStart[1] != 0x78))
		return (false);

	// Check mark if any
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&checkStart[0x17a]));

	if ((temp == 'M.K.') || (temp == 'M&K!') || (temp == 'FLT4') ||
		(temp == 'EXO4') || (temp == 'UNIC') || (temp == 'SNT.'))
	{
		return (false);
	}

	// Check sample informations
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Check volume & finetune
		if ((modStart[i * 8 + 3] > 0x40) || (modStart[i * 8 + 2] > 0x0f))
			return (false);

		// Check sample size
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&modStart[i * 8]));
		if (temp1 != 0)
		{
			sampSize += (temp1 * 2);

			// Check repeat + replen
			if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&modStart[i * 8 + 4])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&modStart[i * 8 + 6]))) > temp1)
				return (0);
		}
	}

	return (true);
}
