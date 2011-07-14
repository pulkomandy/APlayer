/******************************************************************************/
/* RIFF-WAVE Microsoft PCM class.                                             */
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
#include "RIFF-WAVEConverter.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
RIFFWAVEMicrosoftPCM::RIFFWAVEMicrosoftPCM(PResource *resource) : RIFFWAVEFormat(resource)
{
	decodeBuffer = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
RIFFWAVEMicrosoftPCM::~RIFFWAVEMicrosoftPCM(void)
{
}



/******************************************************************************/
/* LoaderInitialize() initialize the loader.                                  */
/*                                                                            */
/* Input:  "dataLength" is the size of the data block.                        */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void RIFFWAVEMicrosoftPCM::LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo)
{
	// Allocate buffer to hold the loaded data
	decodeBuffer = new int8[BUFFER_SIZE];
	if (decodeBuffer == NULL)
		throw PMemoryException();

	// Initialize member variables
	fileSize = dataLength;
}



/******************************************************************************/
/* LoaderCleanup() frees all the resources allocated.                         */
/******************************************************************************/
void RIFFWAVEMicrosoftPCM::LoaderCleanup(void)
{
	// Delete the decode buffer
	delete[] decodeBuffer;
	decodeBuffer = NULL;
}



/******************************************************************************/
/* DecodeSampleData() loads and decode a block of sample data.                */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "buffer" is a pointer to the buffer you has to fill with the data. */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples filled in the buffer.                        */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftPCM::DecodeSampleData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 filled = 0;
	uint32 i, todo;
	uint8 sampSize, shift;

	// Calculate the number of bytes used for each sample
	sampSize = (convInfo->bitSize + 7) / 8;
	shift    = sampSize * 8 - convInfo->bitSize;

	while (length > 0)
	{
		// Do we need to load some data from the file?
		if (samplesLeft == 0)
		{
			// Yes, do it
			samplesLeft = GetFileData(file, decodeBuffer, BUFFER_SIZE);
			offset      = 0;

			if (samplesLeft == 0)
				break;			// End of file, stop filling
		}

		// Find the number of samples to return
		todo = min(length, samplesLeft / sampSize);

		// Copy the sample data
		switch (sampSize)
		{
			// 1-8 bit samples
			case 1:
			{
				for (i = 0; i < todo; i++)
					*buffer++ = ((decodeBuffer[offset++] >> shift) - 128) / 128.0f;

				break;
			}

			// 9-16 bits samples
			case 2:
			{
				for (i = 0; i < todo; i++)
				{
					*buffer++ = ((((decodeBuffer[offset + 1] & 0xff) << 8) | (decodeBuffer[offset] & 0xff)) >> shift) / 32768.0f;
					offset += 2;
				}
				break;
			}

			// 17-24 bits samples
			case 3:
			{
				for (i = 0; i < todo; i++)
				{
					*buffer++ = ((((decodeBuffer[offset + 2] & 0xff) << 16) | ((decodeBuffer[offset + 1] & 0xff) << 8) | (decodeBuffer[offset] & 0xff)) >> shift) / 8388608.0f;
					offset += 3;
				}
				break;
			}

			// 25-32 bits samples
			case 4:
			{
				for (i = 0; i < todo; i++)
				{
					*buffer++ = ((((decodeBuffer[offset + 3] & 0xff) << 24) | ((decodeBuffer[offset + 2] & 0xff) << 16) | ((decodeBuffer[offset + 1] & 0xff) << 8) | (decodeBuffer[offset] & 0xff)) >> shift) / 2147483648.0f;
					offset += 4;
				}
				break;
			}
		}

		// Update the counter variables
		length      -= todo;
		filled      += todo;
		samplesLeft -= (todo * sampSize);
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
uint32 RIFFWAVEMicrosoftPCM::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	return (fileSize / ((convInfo->bitSize + 7) / 8));
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
uint32 RIFFWAVEMicrosoftPCM::CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	return (position * ((convInfo->bitSize + 7) / 8));
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
uint32 RIFFWAVEMicrosoftPCM::CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	return (position / ((convInfo->bitSize + 7) / 8));
}



/******************************************************************************/
/* GetFormatNumber() returns the format number.                               */
/*                                                                            */
/* Output: The format number.                                                 */
/******************************************************************************/
uint16 RIFFWAVEMicrosoftPCM::GetFormatNumber(void) const
{
	return (WAVE_FORMAT_PCM);
}



/******************************************************************************/
/* GetAverageBytesSecond() returns the average bytes per second.              */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The average bytes per second.                                      */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftPCM::GetAverageBytesSecond(const APConverter_SampleFormat *convInfo)
{
	return ((convInfo->channels * convInfo->bitSize * convInfo->frequency + 7) / 8);
}



/******************************************************************************/
/* GetBlockAlign() returns the block align.                                   */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The block align.                                                   */
/******************************************************************************/
uint16 RIFFWAVEMicrosoftPCM::GetBlockAlign(const APConverter_SampleFormat *convInfo)
{
	return ((convInfo->channels * convInfo->bitSize + 7) / 8);
}



/******************************************************************************/
/* WriteData() write a block of data.                                         */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/*         "buffer" is a pointer to the buffer with the data to write.        */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of bytes written.                                       */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftPCM::WriteData(PFile *file, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 written;
	uint32 i;

	if (convInfo->bitSize == 8)
	{
		int8 *buf = buffer;

		// Convert to unsigned
		for (i = 0; i < length; i++)
			*buf++ += 128;

		// Write the data
		written = file->Write(buffer, length);
	}
	else
	{
		int16 *buf = (int16 *)buffer;

		// Convert to little endian
		for (i = 0; i < length; i++)
			*buf++ = P_HOST_TO_LENDIAN_INT16(*buf);

		// Write the data
		written = file->Write(buffer, length * 2);
	}

	return (written);
}
