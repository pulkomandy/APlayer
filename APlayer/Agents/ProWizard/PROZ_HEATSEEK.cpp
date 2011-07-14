/******************************************************************************/
/* ProWizard HEATSEEK class.                                                  */
/*                                                                            */
/* Heatseeker mc1.0 format.                                                   */
/* Created by Heatseeker / Cryptoburners (1991)                               */
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
uint32 PROZ_HEATSEEK::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 temp;
	uint16 emptyNum;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Call Cryptoburner check
	if (!(Check(mod, &mod[0xf8])))
		return (0);

	// Get the number of positions
	posNum = mod[0xf8];

	// Check heighest pattern number
	if (GetPGP(&mod[0xfa], posNum) > 64)
		return (0);

	// Check pattern data
	emptyNum = 0;
	for (i = 0; i < 512; i++)
	{
		// Get first part of pattern data
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0x17a + i * 4]));

		if (temp == 0x8000)
			emptyNum++;
		else
		{
			if ((temp != 0x0000) && (temp != 0xc000))
			{
				// Check period
				if ((temp < 0x71) || (temp > 0x1358))
					return (0);
			}
		}
	}

	if (emptyNum == 0)
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
ap_result PROZ_HEATSEEK::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint32 pattern[4 * 64];
	uint32 *tracks, *destTrack, *copyTrack;
	uint16 i, j, k, m;
	uint32 pattData, temp;
	const uint32 *pattPoi;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Write sample informations
	for (i = 0; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample informations
		destFile->Write(&mod[i * 8], 8);
	}

	// Write position table length
	destFile->Write_UINT8(mod[0xf8]);

	// Write NTK byte
	destFile->Write_UINT8(mod[0xf9]);

	// Write position table
	destFile->Write(&mod[0xfa], 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Allocate a temporary track buffer
	tracks = new uint32[4 * 64 * 64];
	if (tracks == NULL)
		throw PMemoryException();

	try
	{
		pattPoi   = (uint32 *)&mod[0x17a];
		destTrack = tracks;

		// Loop all the patterns
		for (i = 0; i < pattNum; i++)
		{
			// Clear the pattern data
			memset(pattern, 0, sizeof(pattern));

			// Loop the voices
			for (j = 0; j < 4; j++)
			{
				// Loop the rows
				for (k = 0; k < 64; k++)
				{
					// Get the pattern data
					pattData = P_BENDIAN_TO_HOST_INT32(*pattPoi++);

					// Check for commands
					temp = (pattData & 0xffff0000) >> 16;

					// Copy track?
					if (temp == 0xc000)
					{
						copyTrack = &tracks[((pattData & 0x0000ffff) / 4) * 64];

						for (m = 0; m < 64; m++)
						{
							pattern[m * 4 + j] = *copyTrack;
							*destTrack++       = *copyTrack++;
						}
						break;
					}

					// Normal note?
					if (temp < 0x8000)
					{
						pattern[k * 4 + j] = P_HOST_TO_BENDIAN_INT32(pattData);
						*destTrack++       = P_HOST_TO_BENDIAN_INT32(pattData);
					}
					else
					{
						// Empty lines
						temp = pattData & 0x000000ff;
						for (m = 0; m <= temp; m++)
						{
							*destTrack++ = 0;
							k++;
						}

						// Decrement the row counter, because it will
						// be incremented in the loop
						k--;
					}
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}

		// Calculate size of packed patterns
		pattPoi = (uint32 *)&mod[0x17a];
		for (i = 0; i < (pattNum * 64 * 4); i++)
		{
			// Get the pattern data
			pattData = P_BENDIAN_TO_HOST_INT32(*pattPoi++);

			// Check for commands
			temp = (pattData & 0xffff0000) >> 16;

			// Empty lines?
			if (temp == 0x8000)
			{
				i += (pattData & 0x000000ff);
				continue;
			}

			// Copy track?
			if (temp == 0xc000)
				i += 63;
		}

		// Write sample data
		destFile->Write(&mod[(uint8 *)pattPoi - mod], sampSize);
	}
	catch(...)
	{
		// Delete the track buffer
		delete[] tracks;
		throw;
	}

	// Delete the track buffer
	delete[] tracks;

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_HEATSEEK::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_HEATSEEK);
}
