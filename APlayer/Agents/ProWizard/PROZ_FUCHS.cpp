/******************************************************************************/
/* ProWizard FUCHS class.                                                     */
/*                                                                            */
/* Fuchs Tracker format.                                                      */
/* Created by Andreas Fuchs (1990)                                            */
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
uint32 PROZ_FUCHS::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the module ID
	if ((mod[192] != 'S') || (mod[193] != 'O') || (mod[194] != 'N') || (mod[195] != 'G'))
		return (0);

	// All sample size
	pw_j = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10]));
	if ((pw_j <= 2) || (pw_j >= (65535 * 16)))
		return (0);

	// Sample descriptions
	pw_m = 0;
	for (pw_k = 0; pw_k < 16; pw_k++)
	{
		// Size
		pw_o = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[pw_k * 2 + 14]));

		// Loop start
		pw_n = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[pw_k * 2 + 78]));

		// Volumes
		if (mod[46 + pw_k * 2] > 0x40)
			return (0);

		// Size < loop start?
		if (pw_o < pw_n)
			return (0);

		pw_m += pw_o;
	}

	// pw_m is the size of all samples (in descriptions)
	// pw_j is the sample data sizes (header)
	//
	// size < 2 or size > header sample size?
	if ((pw_m <= 2) || (pw_m > pw_j))
		return (0);

	// Get highest pattern number in pattern list
	pw_k = 0;
	for (pw_j = 0; pw_j < 40; pw_j++)
	{
		pw_n = mod[pw_j * 2 + 113];
		if (pw_n > 40)
			return (0);

		if (pw_n > pw_k)
			pw_k = pw_n;
	}

	// pw_m is the size of all samples (in descriptions)
	// pw_k is the highest pattern data - 1
	//
	// Input file not long enough?
	pw_k += 1;
	pw_k *= 1024;
	if ((pw_k + 200) > module.GetLength())
		return (0);

	// pw_m is the size of all samples (in descriptions)
	// pw_k is the pattern data size
	//
	// Calculate the total size of the PTK module
	calcSize = pw_k + pw_m + 1084;

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
ap_result PROZ_FUCHS::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 *whatEver;
	uint8 c1;
	int32 wholeSampleSize = 0;
	int32 sampleSizes[16];
	int32 loopStart[16];
	int32 i, j;
	int32 where = 0;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Initialize arrays
	memset(sampleSizes, 0, sizeof(sampleSizes));
	memset(loopStart, 0, sizeof(loopStart));

	// Allocate a buffer holding everything we need
	whatEver = new uint8[1080];
	if (whatEver == NULL)
		throw PMemoryException();

	memset(whatEver, 0, 1080);

	// Write empty ptk header
	destFile->Write(whatEver, 1080);

	// Write title
	destFile->SeekToBegin();
	destFile->Write(&mod[where], 10);
	where += 10;

	// Read all sample data size
	wholeSampleSize = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[where]));
	where += 4;

	// Read/write sample sizes
	// Have to halve these :(
	for (i = 0; i < 16; i++)
	{
		destFile->Seek(42 + i * 30, PFile::pSeekBegin);

		whatEver[0] = mod[where];
		whatEver[1] = mod[where + 1];
		sampleSizes[i] = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&whatEver[0]));

		whatEver[1] /= 2;
		if ((whatEver[0] / 2) * 2 != whatEver[0])
		{
			if (whatEver[1] < 0x80)
				whatEver[1] += 0x80;
			else
			{
				whatEver[1] -= 0x80;
				whatEver[0] += 0x01;
			}
		}
		whatEver[0] /= 2;

		destFile->Write(whatEver, 2);
		where += 2;
	}

	// Read/write volumes
	for (i = 0; i < 16; i++)
	{
		destFile->Seek(45 + i * 30, PFile::pSeekBegin);

		where += 1;
		destFile->Write_UINT8(mod[where++]);
	}

	// Read/write loop start
	// Have to halve these :(
	for (i = 0; i < 16; i++)
	{
		destFile->Seek(46 + i * 30, PFile::pSeekBegin);

		whatEver[0] = mod[where];
		whatEver[1] = mod[where + 1];
		loopStart[i] = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&whatEver[0]));

		whatEver[1] /= 2;
		if ((whatEver[0] / 2) * 2 != whatEver[0])
		{
			if (whatEver[1] < 0x80)
				whatEver[1] += 0x80;
			else
			{
				whatEver[1] -= 0x80;
				whatEver[0] += 0x01;
			}
		}
		whatEver[0] /= 2;

		destFile->Write(whatEver, 2);
		where += 2;
	}

	// Write replen
	// Have to halve there :(
	whatEver[49] = 0x01;
	for (i = 0; i < 16; i++)
	{
		destFile->Seek(48 + i * 30, PFile::pSeekBegin);

		j = sampleSizes[i] - loopStart[i];
		if ((j == 0) || (loopStart[i] == 0))
		{
			destFile->Write(&whatEver[48], 2);
			continue;
		}

		j /= 2;
		destFile->Write_B_UINT16(j);
	}

	// Fill replens up to 31st sample wiz $0001
	for (i = 16; i < 31; i++)
	{
		destFile->Seek(48 + i * 30, PFile::pSeekBegin);
		destFile->Write(&whatEver[48], 2);
	}

	// That's it for the samples!
	// Now, the pattern list
	//
	// Read number of pattern to play
	destFile->Seek(950, PFile::pSeekBegin);

	// Bypass empty byte (saved wiz a WORD ..)
	where += 1;
	destFile->Write_UINT8(mod[where++]);

	// Write NoiseTracker byte
	destFile->Write_UINT8(0x7f);

	// Read/write pattern list
	for (i = 0; i < 40; i++)
	{
		where += 1;
		destFile->Write_UINT8(mod[where++]);
	}

	// Write the ID
	destFile->Seek(1080, PFile::pSeekBegin);
	destFile->Write_B_UINT32('M.K.');

	// Now the pattern data
	//
	// Bypass the "SONG" id
	where += 4;

	// Read pattern data size
	j = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[where]));
	where += 4;

	// Reallocate the temporary buffer to hold the pattern data
	delete[] whatEver;
	whatEver = new uint8[j];
	if (whatEver == NULL)
		throw PMemoryException();

	// Convert the shits
	for (i = 0; i < j; i += 4)
	{
		whatEver[i]     = mod[where++];
		whatEver[i + 1] = mod[where++];
		whatEver[i + 2] = mod[where++];
		whatEver[i + 3] = mod[where++];

		// Convert fx C arg back to hex value
		if ((whatEver[i + 2] & 0x0f) == 0x0c)
		{
			c1 = whatEver[i + 3];

			if (c1 <= 9)
			{
				whatEver[i + 3] = c1;
				continue;
			}

			if ((c1 >= 16) && (c1 <= 25))
			{
				whatEver[i + 3] = (c1 - 6);
				continue;
			}

			if ((c1 >= 32) && (c1 <= 41))
			{
				whatEver[i + 3] = (c1 - 12);
				continue;
			}

			if ((c1 >= 48) && (c1 <= 57))
			{
				whatEver[i + 3] = (c1 - 18);
				continue;
			}

			if ((c1 >= 64) && (c1 <= 73))
			{
				whatEver[i + 3] = (c1 - 24);
				continue;
			}

			if ((c1 >= 80) && (c1 <= 89))
			{
				whatEver[i + 3] = (c1 - 30);
				continue;
			}

			if ((c1 >= 96) && (c1 <= 100))
			{
				whatEver[i + 3] = (c1 - 36);
				continue;
			}
		}
	}

	// Write pattern data
	destFile->Write(whatEver, j);

	// Free the temporary buffer
	delete[] whatEver;

	// Read/write sample data
	where += 4;
	for (i = 0; i < 16; i++)
	{
		if (sampleSizes[i] != 0)
		{
			destFile->Write(&mod[where], sampleSizes[i]);
			where += sampleSizes[i];
		}
	}

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_FUCHS::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_FUCHS);
}
