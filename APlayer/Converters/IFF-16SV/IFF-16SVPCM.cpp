/******************************************************************************/
/* IFF-16SV PCM class.                                                        */
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
#include "IFF-16SVConverter.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
IFF16SVPCM::IFF16SVPCM(PResource *resource) : IFF16SVFormat(resource)
{
	decodeBuffer = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
IFF16SVPCM::~IFF16SVPCM(void)
{
}



/******************************************************************************/
/* LoaderInitialize() initialize the loader.                                  */
/*                                                                            */
/* Input:  "dataLength" is the size of the data block.                        */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void IFF16SVPCM::LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo)
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
void IFF16SVPCM::LoaderCleanup(void)
{
	// Delete the decode buffer
	delete[] decodeBuffer;
	decodeBuffer = NULL;
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
uint32 IFF16SVPCM::DecodeSampleData(PFile *file, PFile *file2, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 filled = 0;
	uint32 bufSize;
	uint32 i, todo;

	if (file2 == NULL)
		bufSize = BUFFER_SIZE;
	else
		bufSize = BUFFER_SIZE / 2;

	while (length > 0)
	{
		// Do we need to load some data from the file?
		if (samplesLeft == 0)
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
			// Find the number of samples to return
			todo = min(length, samplesLeft / 2);

			for (i = 0; i < todo; i++)
			{
				*buffer++ = (((decodeBuffer[offset] & 0xff) << 8) | (decodeBuffer[offset + 1] & 0xff)) / 32768.0f;
				offset += 2;
			}
		}
		else
		{
			// Stereo sample
			//
			// Find the number of samples to return
			todo = min(length, samplesLeft / 2) / 2;

			for (i = 0; i < todo; i++)
			{
				*buffer++ = (((decodeBuffer[offset] & 0xff) << 8) | (decodeBuffer[offset + 1] & 0xff)) / 32768.0f;
				*buffer++ = (((decodeBuffer[bufSize + offset] & 0xff) << 8) | (decodeBuffer[bufSize + offset + 1] & 0xff)) / 32768.0f;
				offset += 2;
			}

			todo *= 2;
		}

		// Update the counter variables
		length      -= todo;
		filled      += todo;
		samplesLeft -= todo * 2;
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
uint32 IFF16SVPCM::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	return (fileSize / 2);
}



/******************************************************************************/
/* GetRightChannelPosition() will return the position where the right channel */
/*      starts relative to the left channel.                                  */
/*                                                                            */
/* Input:  "totalLen" is the total length of the sample in bytes.             */
/*                                                                            */
/* Output: The right channel position.                                        */
/******************************************************************************/
int64 IFF16SVPCM::GetRightChannelPosition(uint32 totalLen)
{
	return (totalLen / 2);
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
uint32 IFF16SVPCM::CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	if (convInfo->channels == 2)
		return (position / 2 * 2);

	return (position * 2);
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
uint32 IFF16SVPCM::CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	return (position);
}



/******************************************************************************/
/* GetFormatNumber() returns the format number.                               */
/*                                                                            */
/* Output: The format number.                                                 */
/******************************************************************************/
uint16 IFF16SVPCM::GetFormatNumber(void) const
{
	return (IFF16SV_FORMAT_PCM);
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
/* Output: The number of samples written.                                     */
/******************************************************************************/
uint32 IFF16SVPCM::WriteData(PFile *file, PFile *stereoFile, int16 *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 i, written;

	if (convInfo->channels == 2)
	{
		// Stereo sample
		int16 *newBuf[2];
		uint32 j, count;

		// Split the data up in two parts
		newBuf[0] = new int16[length / 2];
		newBuf[1] = new int16[length / 2];

		if ((newBuf[0] == NULL) || (newBuf[1] == NULL))
		{
			delete[] newBuf[1];
			delete[] newBuf[0];
			return (0);
		}

		count = length / 2;
		for (i = 0, j = 0; i < count; i++, j += 2)
		{
			newBuf[0][i] = P_HOST_TO_BENDIAN_INT16(buffer[j]);
			newBuf[1][i] = P_HOST_TO_BENDIAN_INT16(buffer[j + 1]);
		}

		// Write the data in the two files
		try
		{
			written = file->Write(newBuf[0], count * 2);
			stereoFile->Write(newBuf[1], count * 2);
		}
		catch(...)
		{
			written = 0;
		}

		// Delete the temporary buffers again
		delete[] newBuf[1];
		delete[] newBuf[0];
	}
	else
	{
		// Mono sample
		//
		int16 *buf = buffer;

		// Convert to big endian
		for (i = 0; i < length; i++)
			*buf++ = P_HOST_TO_BENDIAN_INT16(*buf);

		// Write the data
		written = file->Write(buffer, length * 2);
	}

	return (written / 2);
}
