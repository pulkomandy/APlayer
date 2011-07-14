/******************************************************************************/
/* Decruncher for PowerPacker class.                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PBinary.h"
#include "PFile.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "Decruncher.h"


/******************************************************************************/
/* Determine() will be check the file to see if it's packed with PowerPacker. */
/*                                                                            */
/* Input:  "info" is a pointer to a info structure where to read and store    */
/*         information needed.                                                */
/******************************************************************************/
bool Decrunch_PowerPacker::Determine(APAgent_DecrunchFile *decrunchInfo)
{
	PFile *file = decrunchInfo->file;

	// Check the module size
	if (file->GetLength() < 12)
		return (false);

	// Check the mark
	file->SeekToBegin();

	if (file->Read_B_UINT32() != 'PP20')
		return (false);

	// Check the offset sizes
	if ((file->Read_UINT8() > 16) || (file->Read_UINT8() > 16) || (file->Read_UINT8() > 16) || (file->Read_UINT8() > 16))
		return (false);

	return (true);
}



/******************************************************************************/
/* GetUnpackedSize() returns the size of the unpacked data.                   */
/*                                                                            */
/* Input:  "info" is a pointer to a info structure where to read and store    */
/*         information needed.                                                */
/******************************************************************************/
uint32 Decrunch_PowerPacker::GetUnpackedSize(APAgent_DecrunchFile *decrunchInfo)
{
	// Seek to the last 4 bytes
	decrunchInfo->file->Seek(-4, PFile::pSeekEnd);

	return (decrunchInfo->file->Read_B_UINT32() >> 8);
}



/******************************************************************************/
/* Unpack() will unpack the module.                                           */
/*                                                                            */
/* Input:  "sourceBuf" is a reference to the packed data.                     */
/*         "destBuf" is a reference where to write the unpacked data.         */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result Decrunch_PowerPacker::Unpack(PBinary &sourceBuf, PBinary &destBuf)
{
	uint8 *dest, *destStart;
	uint32 destLen;
	uint8 offsetSizes[4];
	uint32 bytes, offset, i;
	int32 to_add, idx, numBits;

	// Add safety buffer to the destination buffer
	destLen = destBuf.GetLength();
	destBuf.SetLength(1024 + destLen);

	// Get the buffer addresses
	source    = sourceBuf.GetBufferForReadOnly();
	destStart = destBuf.GetBufferForWriting() + 1024;

	// Start to get the offset sizes
	offsetSizes[0] = source[4];
	offsetSizes[1] = source[5];
	offsetSizes[2] = source[6];
	offsetSizes[3] = source[7];

	// Initialize pointers and other stuff
	source += sourceBuf.GetLength() - 4;
	dest    = destStart + destLen;

	counter = 0;

	// Skip bits
	GetBits(source[3]);

	// Do it forever, i.e., while the whole file isn't unpacked
	for (;;)
	{
		// Copy some bytes from the source anyway
		if (GetBits(1) == 0)
		{
			bytes = 0;

			do
			{
				to_add = GetBits(2);
				bytes += to_add;
			}
			while (to_add == 3);

			for (i = 0; i <= bytes; i++)
				*--dest = GetBits(8);

			if (dest <= destStart)
				break;					// Stop depacking
		}

		// Decode what to copy from the destination file
		idx     = GetBits(2);
		numBits = offsetSizes[idx];

		// Bytes to copy
		bytes = idx + 1;

		if (bytes == 4)		// 4 means >= 4
		{
			// And maybe a bigger offset
			if (GetBits(1) == 0)
				offset = GetBits(7);
			else
				offset = GetBits(numBits);

			do
			{
				to_add = GetBits(3);
				bytes += to_add;
			}
			while (to_add == 7);
		}
		else
			offset = GetBits(numBits);

		for (i = 0; i <= bytes; i++)
		{
			dest[-1] = dest[offset];
			dest--;
		}

		if (dest <= destStart)
			break;					// Stop depacking
	}

	// Check to see if the file is corrupt
	if (dest < destStart)
		return (AP_ERROR);

	// Copy the data back in memory
	memcpy(destBuf.GetBufferForWriting(), destStart, destLen);

	return (AP_OK);
}



/******************************************************************************/
/* GetBits() will get a number of bits from the packed data and return it.    */
/*                                                                            */
/* Input:  "num" is the number of bits to return.                             */
/*                                                                            */
/* Output: The the retrieved number.                                          */
/******************************************************************************/
uint32 Decrunch_PowerPacker::GetBits(uint32 num)
{
	uint32 result = 0;
	uint32 i;

	for (i = 0; i < num; i++)
	{
		if (counter == 0)
		{
			counter  = 8;
			shift_in = *--source;
		}

		result = (result << 1) | (shift_in & 1);
		shift_in >>= 1;
		counter--;
	}

	return (result);
}
