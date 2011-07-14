/******************************************************************************/
/* ProWizard STIM class.                                                      */
/*                                                                            */
/* STIM (SlamTilt) format.                                                    */
/* Created by ???                                                             */
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
uint32 PROZ_STIM::CheckModule(const PBinary &module)
{
	const uint8 *mod;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check module ID
	if ((mod[0] != 'S') || (mod[1] != 'T') || (mod[2] != 'I') || (mod[3] != 'M'))
		return (0);

	// Check sample address
	pw_j = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[4]));
	if (pw_j < 406)
		return (0);

	// Check size of the pattern list
	pw_k = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[18]));
	if (pw_k > 128)
		return (0);

	// Check number of patterns saved
	pw_k = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20]));
	if ((pw_k > 64) || (pw_k == 0))
		return (0);

	// Check pattern list
	for (pw_l = 0; pw_l < 128; pw_l++)
	{
		if (mod[22 + pw_l] > pw_k)
			return (0);
	}

	// Check sample sizes
	pw_wholeSampleSize = 0;
	for (pw_l = 0; pw_l < 31; pw_l++)
	{
		pw_o = pw_j + pw_l * 4;
		pw_k = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[pw_o]));
		pw_m = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[pw_o + pw_k - pw_l * 4])) * 2;
		pw_wholeSampleSize += pw_m;
	}

	if (pw_wholeSampleSize <= 4)
		return (0);

	// pw_wholeSampleSize is the size of the sample data
	// pw_j is the address of the sample descriptions
	return (pw_j + pw_wholeSampleSize);
}



/******************************************************************************/
/* ConvertModule() will be convert the module to ProTracker format.           */
/*                                                                            */
/* Input:  "module" is a reference to the packed module.                      */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result PROZ_STIM::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 *whatEver;
	uint8 c1, c2, c3, c4;
	uint8 max;
	uint8 note, smp, fx, fxVal;
	int16 tracksAdd[4];
	int32 i, j, k;
	int32 wholeSampleSize = 0;
	int32 smpDescAdd;
	int32 patAdds[64];
	int32 smpDataAdds[31];
	int32 smpSizes[31];
	int32 where = 0;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Initialize the period table
	InitPeriods();

	// Initialize arrays
	memset(patAdds, 0, sizeof(patAdds));
	memset(smpDataAdds, 0, sizeof(smpDataAdds));
	memset(smpSizes, 0, sizeof(smpSizes));

	// Allocate a buffer holding everything we need
	whatEver = new uint8[1024];
	if (whatEver == NULL)
		throw PMemoryException();

	memset(whatEver, 0, 1024);

	// Write title
	destFile->Write(whatEver, 20);

	// Bypass ID
	where += 4;

	// Read address of sample description
	smpDescAdd = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[where]));

	// Convert and write header
	for (i = 0; i < 31; i++)
	{
		where = smpDescAdd + i * 4;

		smpDataAdds[i]  = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[where]));
		smpDataAdds[i] += smpDescAdd;

		where = smpDataAdds[i];
		smpDataAdds[i] += 8;

		// Write sample name
		destFile->Write(whatEver, 22);

		// Sample size
		smpSizes[i] = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[where])) * 2;
		wholeSampleSize += (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[where])) * 2);

		// Size, finetune, volume, loops
		destFile->Write(&mod[where], 8);
	}

	// Size of the pattern list
	where = 19;
	destFile->Write_UINT8(mod[where++]);
	destFile->Write_UINT8(0x7f);

	// Pattern table
	where += 1;
	max = mod[where++];
	destFile->Write(&mod[where], 128);
	where += 128;

	// Write the ID
	destFile->Write_B_UINT32('M.K.');

	// Read pattern addresses
	for (i = 0; i < 64; i++)
	{
		patAdds[i]  = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[where]));
		patAdds[i] += 0x0c;
		where += 4;
	}

	// Pattern data
	for (i = 0; i < max; i++)
	{
		where = patAdds[i];
		for (k = 0; k < 4; k++)
		{
			tracksAdd[k] = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[where]));
			where += 2;
		}

		memset(whatEver, 0, 1024);
		for (k = 0; k < 4; k++)
		{
			where = patAdds[i] + tracksAdd[k];
			for (j = 0; j < 64; j++)
			{
				c1 = mod[where++];
				if ((c1 & 0x80) == 0x80)
				{
					j += (c1 & 0x7f);
					continue;
				}

				c2 = mod[where++];
				c3 = mod[where++];

				smp   = c1 & 0x1f;
				note  = c2 & 0x3f;
				fx    = ((c1 >> 5) & 0x03);

				c4    = ((c2 >> 4) & 0x0c);
				fx   |= c4;
				fxVal = c3;

				whatEver[j * 16 + k * 4] = (smp & 0xf0);

				if (note != 0)
				{
					whatEver[j * 16 + k * 4]    |= period[note - 1][0];
					whatEver[j * 16 + k * 4 + 1] = period[note - 1][1];
				}

				whatEver[j * 16 + k * 4 + 2]  = ((smp << 4) & 0xf0);
				whatEver[j * 16 + k * 4 + 2] |= fx;
				whatEver[j * 16 + k * 4 + 3]  = fxVal;
			}
		}

		destFile->Write(whatEver, 1024);
	}

	// Free the temporary buffer
	delete[] whatEver;

	// Write sample data
	for (i = 0; i < 31; i++)
	{
		where = smpDataAdds[i];
		destFile->Write(&mod[where], smpSizes[i]);
	}

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_STIM::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_STIM);
}
