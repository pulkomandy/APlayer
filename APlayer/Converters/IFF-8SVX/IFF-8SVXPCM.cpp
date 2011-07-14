/******************************************************************************/
/* IFF-8SVX PCM class.                                                        */
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
/* Constructor                                                                */
/******************************************************************************/
IFF8SVXPCM::IFF8SVXPCM(PResource *resource) : IFF8SVXFormat(resource)
{
	decodeBuffer = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
IFF8SVXPCM::~IFF8SVXPCM(void)
{
}



/******************************************************************************/
/* LoaderInitialize() initialize the loader.                                  */
/*                                                                            */
/* Input:  "dataLength" is the size of the data block.                        */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void IFF8SVXPCM::LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo)
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
void IFF8SVXPCM::LoaderCleanup(void)
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
uint32 IFF8SVXPCM::DecodeSampleData(PFile *file, PFile *file2, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
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
			todo = min(length, samplesLeft);

			for (i = 0; i < todo; i++)
				*buffer++ = decodeBuffer[offset++] / 128.0f;
		}
		else
		{
			// Stereo sample
			//
			// Find the number of samples to return
			todo = min(length, samplesLeft) / 2;

			for (i = 0; i < todo; i++)
			{
				*buffer++ = decodeBuffer[offset] / 128.0f;
				*buffer++ = decodeBuffer[bufSize + offset++] / 128.0f;
			}

			todo *= 2;
		}

		// Update the counter variables
		length      -= todo;
		filled      += todo;
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
uint32 IFF8SVXPCM::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	return (fileSize);
}



/******************************************************************************/
/* GetRightChannelPosition() will return the position where the right channel */
/*      starts relative to the left channel.                                  */
/*                                                                            */
/* Input:  "totalLen" is the total length of the sample in bytes.             */
/*                                                                            */
/* Output: The right channel position.                                        */
/******************************************************************************/
int64 IFF8SVXPCM::GetRightChannelPosition(uint32 totalLen)
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
uint32 IFF8SVXPCM::CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	if (convInfo->channels == 2)
		return (position / 2);

	return (position);
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
uint32 IFF8SVXPCM::CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	return (position);
}



/******************************************************************************/
/* GetFormatNumber() returns the format number.                               */
/*                                                                            */
/* Output: The format number.                                                 */
/******************************************************************************/
uint16 IFF8SVXPCM::GetFormatNumber(void) const
{
	return (IFF8SVX_FORMAT_PCM);
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
uint32 IFF8SVXPCM::WriteData(PFile *file, PFile *stereoFile, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 written;

	if (convInfo->channels == 2)
	{
		// Stereo sample
		int8 *newBuf[2];
		uint32 i, j, count;

		// Split the data up in two parts
		newBuf[0] = new int8[length / 2];
		newBuf[1] = new int8[length / 2];

		if ((newBuf[0] == NULL) || (newBuf[1] == NULL))
		{
			delete[] newBuf[1];
			delete[] newBuf[0];
			return (0);
		}

		count = length / 2;
		for (i = 0, j = 0; i < count; i++, j += 2)
		{
			newBuf[0][i] = buffer[j];
			newBuf[1][i] = buffer[j + 1];
		}

		// Write the data in the two files
		try
		{
			written = file->Write(newBuf[0], count);
			stereoFile->Write(newBuf[1], count);
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
		// Write the data
		written = file->Write(buffer, length);
	}

	return (written);
}
