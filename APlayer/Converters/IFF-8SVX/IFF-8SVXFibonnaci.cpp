/******************************************************************************/
/* IFF-8SVX Fibonnaci class.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PFile.h"

// APlayerKit headers
#include "APAddOns.h"

// Converter headers
#include "IFF-8SVXConverter.h"


/******************************************************************************/
/* Fibonnaci table                                                            */
/******************************************************************************/
int8 fibTable[16] = { -34, -21, -13, -8, -5, -3, -2, -1, 0, 1, 2, 3, 5, 8, 13, 21 };



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
IFF8SVXFibonnaci::IFF8SVXFibonnaci(PResource *resource) : IFF8SVXFormat(resource)
{
	decodeBuffer = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
IFF8SVXFibonnaci::~IFF8SVXFibonnaci(void)
{
}



/******************************************************************************/
/* LoaderInitialize() initialize the loader.                                  */
/*                                                                            */
/* Input:  "dataLength" is the size of the data block.                        */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void IFF8SVXFibonnaci::LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo)
{
	// Allocate buffer to hold the loaded data
	decodeBuffer = new int8[BUFFER_SIZE];
	if (decodeBuffer == NULL)
		throw PMemoryException();

	// Initialize member variables
	decompressedSize = dataLength;
	loadFirstBuf     = true;
}



/******************************************************************************/
/* LoaderCleanup() frees all the resources allocated.                         */
/******************************************************************************/
void IFF8SVXFibonnaci::LoaderCleanup(void)
{
	// Delete the decode buffer
	delete[] decodeBuffer;
	decodeBuffer = NULL;
}



/******************************************************************************/
/* ResetLoader() resets the loader variables.                                 */
/*                                                                            */
/* Input:  "position" is the new position.                                    */
/******************************************************************************/
void IFF8SVXFibonnaci::ResetLoader(uint32 position)
{
	if (position == 0)
		loadFirstBuf = true;
}



/******************************************************************************/
/* DecodeSampleData() loads and decode a block of sample data.                */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "file2" is a pointer to a PFile object with the right channel.     */
/*         "buffer" is a pointer to the buffer you has to fill with the data. */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples filled in the buffer.                        */
/******************************************************************************/
uint32 IFF8SVXFibonnaci::DecodeSampleData(PFile *file, PFile *file2, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 filled = 0;
	uint32 bufSize;
	uint32 todo;

	if (file2 == NULL)
		bufSize = BUFFER_SIZE;
	else
		bufSize = BUFFER_SIZE / 2;

	while (length > 0)
	{
		// Do we need to load some data from the file?
		if (samplesLeft < 2)
		{
			// Yes, do it
			if (file2 != NULL)
			{
				samplesLeft = GetFileData2(file2, decodeBuffer + bufSize, bufSize);

				if (samplesLeft == 0)
					break;			// End of file, stop filling

				samplesLeft += GetFileData1(file, decodeBuffer, samplesLeft);
			}
			else
			{
				samplesLeft = GetFileData1(file, decodeBuffer, bufSize);

				if (samplesLeft == 0)
					break;			// End of file, stop filling
			}

			offset = 0;
		}

		// Copy the sample data
		if (file2 == NULL)
		{
			// Mono sample
			//
			// Find the number of bytes to decode
			todo = min(length / 2, samplesLeft);

			// Should we get the start value?
			if (loadFirstBuf)
			{
				loadFirstBuf = false;
				startVal1    = decodeBuffer[offset + 1];
				offset      += 2;
				todo        -= 2;
				samplesLeft -= 2;
				*buffer++    = startVal1 / 128.0f;		// Store the value twice, so we get an even number to store in the buffer
				*buffer++    = startVal1 / 128.0f;
				length      -= 2;
				filled      += 2;
			}

			startVal1 = DecodeBuffer((uint8 *)decodeBuffer + offset, buffer, todo, startVal1, false);
			offset   += todo;
			buffer   += todo * 2;
		}
		else
		{
			// Stereo sample
			//
			// Find the number of bytes to decode
			todo = min(length / 2, samplesLeft) / 2;

			// Should we get the start value?
			if (loadFirstBuf)
			{
				loadFirstBuf = false;
				startVal1    = decodeBuffer[offset + 1];
				startVal2    = decodeBuffer[bufSize + offset + 1];
				offset      += 2;
				todo        -= 4;
				samplesLeft -= 4;
				*buffer++    = startVal1 / 128.0f;		// Store the value twice, so we get an even number to store in the buffer
				*buffer++    = startVal2 / 128.0f;
				*buffer++    = startVal1 / 128.0f;
				*buffer++    = startVal2 / 128.0f;
				length      -= 4;
				filled      += 4;
			}

			startVal1 = DecodeBuffer((uint8 *)decodeBuffer + offset, buffer, todo, startVal1, true);
			startVal2 = DecodeBuffer((uint8 *)decodeBuffer + bufSize + offset, buffer + 1, todo, startVal2, true);
			offset   += todo;
			todo     *= 2;
			buffer   += todo * 2;
		}

		// Update the counter variables
		length      -= todo * 2;
		filled      += todo * 2;
		samplesLeft -= todo;
	}

	return (filled);
}



/******************************************************************************/
/* GetTotalSampleLength() calculates how many samples that will be returned.  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples the file is.                                 */
/******************************************************************************/
uint32 IFF8SVXFibonnaci::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	return (decompressedSize);
}



/******************************************************************************/
/* GetRightChannelPosition() will return the position where the right channel */
/*      starts relative to the left channel.                                  */
/*                                                                            */
/* Input:  "totalLen" is the total length of the sample in bytes.             */
/*                                                                            */
/* Output: The right channel position.                                        */
/******************************************************************************/
int64 IFF8SVXFibonnaci::GetRightChannelPosition(uint32 totalLen)
{
	return (totalLen / 4 + 2);
}



/******************************************************************************/
/* CalcFilePosition() calculates the number of bytes to go into the file to   */
/*      reach the position given.                                             */
/*                                                                            */
/* Input:  "position" is the start position from the start of the sample.     */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The position in the file.                                          */
/******************************************************************************/
uint32 IFF8SVXFibonnaci::CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	if (position == 0)
	{
		loadFirstBuf = true;
		return (0);
	}

	// Because the Fibonnaci algorithm depends on all previous
	// samples to get the next value, it isn't right possible
	// to jump around in the file. We does support in anyway,
	// but sometimes the sound will overpeak. To reduce that
	// situation, we reset the start values
	startVal1 = 0;
	startVal2 = 0;

	if (convInfo->channels == 2)
		return ((position / 4) + 2);

	return ((position / 2) + 2);
}



/******************************************************************************/
/* CalcSamplePosition() calculates the number of samples from the byte        */
/*      position given.                                                       */
/*                                                                            */
/* Input:  "position" is the position in bytes.                               */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The position in samples.                                           */
/******************************************************************************/
uint32 IFF8SVXFibonnaci::CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	return (position);
}



/******************************************************************************/
/* SaverInitialize() initialize the saver.                                    */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void IFF8SVXFibonnaci::SaverInitialize(const APConverter_SampleFormat *convInfo)
{
	saveFirstBuf = true;
}



/******************************************************************************/
/* GetFormatNumber() returns the format number.                               */
/*                                                                            */
/* Output: The format number.                                                 */
/******************************************************************************/
uint16 IFF8SVXFibonnaci::GetFormatNumber(void) const
{
	return (IFF8SVX_FORMAT_FIBONNACI);
}



/******************************************************************************/
/* WriteData() write a block of data.                                         */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/*         "stereoFile" is a pointer to the right channel file object to      */
/*         write to or NULL.                                                  */
/*         "buffer" is a pointer to the buffer with the data to write.        */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of bytes written.                                       */
/******************************************************************************/
uint32 IFF8SVXFibonnaci::WriteData(PFile *file, PFile *stereoFile, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint8 *fibBuf[2];
	uint32 i, j, count;
	uint32 written;

	if (convInfo->channels == 2)
	{
		// Stereo sample
		//
		// Allocate buffer to hold the encoded data
		fibBuf[0] = new uint8[length / 4 + 2];
		fibBuf[1] = new uint8[length / 4 + 2];

		if ((fibBuf[0] == NULL) || (fibBuf[1] == NULL))
		{
			delete[] fibBuf[1];
			delete[] fibBuf[0];
			return (0);
		}

		// Calculate the number of samples written
		written = length / 2;

		// If it is the first buffer, write the start byte
		i = 0;
		j = 0;

		if (saveFirstBuf)
		{
			saveFirstBuf = false;
			lastVal1     = buffer[0];
			lastVal2     = buffer[1];
			fibBuf[0][0] = lastVal1;
			fibBuf[0][1] = lastVal1;
			fibBuf[1][0] = lastVal2;
			fibBuf[1][1] = lastVal2;
			fibBuf[0][2] = 0x80 | GetNextNibble(buffer[2], lastVal1);
			fibBuf[1][2] = 0x80 | GetNextNibble(buffer[3], lastVal2);
			written     -= 4;
			i = 3;
			j = 4;
		}

		// Encode the data
		count = length / 4;
		for (; i < count; i++, j += 4)
		{
			// Left channel
			fibBuf[0][i]  = GetNextNibble(buffer[j], lastVal1) << 4;
			fibBuf[0][i] |= GetNextNibble(buffer[j + 2], lastVal1);

			// Right channel
			fibBuf[1][i]  = GetNextNibble(buffer[j + 1], lastVal2) << 4;
			fibBuf[1][i] |= GetNextNibble(buffer[j + 3], lastVal2);
		}

		// Write the data
		try
		{
			file->Write(fibBuf[0], i);
			stereoFile->Write(fibBuf[1], i);
		}
		catch(...)
		{
			written = 0;
		}

		// Delete the temporary buffer again
		delete[] fibBuf[1];
		delete[] fibBuf[0];
	}
	else
	{
		// Mono sample
		//
		// Allocate buffer to hold the encoded data
		fibBuf[0] = new uint8[length / 2 + 2];
		if (fibBuf[0] == NULL)
			return (0);

		// Calculate the number of samples written
		written = length;

		// If it is the first buffer, write the start byte
		i = 0;
		j = 0;

		if (saveFirstBuf)
		{
			saveFirstBuf = false;
			lastVal1     = buffer[0];
			fibBuf[0][0] = lastVal1;
			fibBuf[0][1] = lastVal1;
			fibBuf[0][2] = 0x80 | GetNextNibble(buffer[1], lastVal1);
			written     -= 4;
			i = 3;
			j = 2;
		}

		// Encode the data
		count = length / 2;
		for (; i < count; i++, j += 2)
		{
			fibBuf[0][i]  = GetNextNibble(buffer[j], lastVal1) << 4;
			fibBuf[0][i] |= GetNextNibble(buffer[j + 1], lastVal1);
		}

		// Write the data
		try
		{
			file->Write(fibBuf[0], i);
		}
		catch(...)
		{
			i = 0;
		}

		// Delete the temporary buffer again
		delete[] fibBuf[0];
	}

	return (written);
}



/******************************************************************************/
/* DecodeBuffer() will decode the source buffer using the Fibonnaci Delta     */
/*      algorithm and store the result in the destination buffer.             */
/*                                                                            */
/* Input:  "source" is a pointer to the source buffer.                        */
/*         "dest" is a pointer to the destination buffer.                     */
/*         "todo" is the number of bytes to decode.                           */
/*         "startVal" is the start value.                                     */
/*         "skip" indicate if you want to skip an extra sample.               */
/*                                                                            */
/* Output: The last value written.                                            */
/******************************************************************************/
int8 IFF8SVXFibonnaci::DecodeBuffer(uint8 *source, float *dest, uint32 todo, int8 startVal, bool skip)
{
	uint32 i;
	uint32 add = 1 + (skip == true ? 1 : 0);
	int8 val = startVal;

	for (i = 0; i < todo; i++)
	{
		val  += fibTable[(source[i] & 0xf0) >> 4];
		*dest = val / 128.0f;
		dest += add;

		val  += fibTable[source[i] & 0x0f];
		*dest = val / 128.0f;
		dest += add;
	}

	return (val);
}



/******************************************************************************/
/* GetNextNibble() will calculate the next Fibonnaci value.                   */
/*                                                                            */
/* Input:  "sample" is the new sample value.                                  */
/*         "lastVal" is a the previous sample value.                          */
/*                                                                            */
/* Output: The Fibonnaci value.                                               */
/******************************************************************************/
uint8 IFF8SVXFibonnaci::GetNextNibble(int8 sample, int8 &lastVal)
{
	uint8 i;
	int16 calcVal;

	for (i = 1; i < 16; i++)
	{
		if ((lastVal + fibTable[i]) > sample)
			break;
	}

	i--;
	calcVal = lastVal + fibTable[i];
	if (calcVal < -128)
		i++;

	// Remember the new value
	lastVal = lastVal + fibTable[i];

	return (i);
}
