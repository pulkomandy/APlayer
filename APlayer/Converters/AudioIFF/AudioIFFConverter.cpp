/******************************************************************************/
/* AudioIFF Converter Interface.                                              */
/*                                                                            */
/* By Thomas Neumann.                                                         */
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
#include "PResource.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"

// Converter headers
#include "AudioIFFConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define ConverterVersion		2.05f



/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define BUFFER_SIZE				16384



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
AudioIFFConverter::AudioIFFConverter(APGlobalData *global, PString fileName) : APAddOnConverter(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	saveBuffer = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
AudioIFFConverter::~AudioIFFConverter(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float AudioIFFConverter::GetVersion(void)
{
	return (ConverterVersion);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 AudioIFFConverter::GetSupportFlags(int32 index)
{
	return (apcLoader | apcSaver | apcSupport8Bit | apcSupport16Bit | apcSupportMono | apcSupportStereo);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString AudioIFFConverter::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_AUDIOIFF_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString AudioIFFConverter::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_AUDIOIFF_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetExtension() returns the file extension if any.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The file extension.                                                */
/******************************************************************************/
PString AudioIFFConverter::GetExtension(int32 index)
{
	PString ext;

	ext.LoadString(res, IDS_AUDIOIFF_EXTENSION_TO_USE);
	return (ext);
}



/******************************************************************************/
/* GetTypeString() returns the sample type string.                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The sample type string.                                            */
/******************************************************************************/
PString AudioIFFConverter::GetTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_AUDIOIFF_MIME);
	return (type);
}



/******************************************************************************/
/* FileCheck() checks the file to see if it's a supported sample.             */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result AudioIFFConverter::FileCheck(int32 index, PFile *file)
{
	// Seek to the start of the file
	file->SeekToBegin();

	// Check the chunk names
	if (file->Read_B_UINT32() == 'FORM')
	{
		file->Seek(4, PFile::pSeekCurrent);
		if (file->Read_B_UINT32() == 'AIFF')
			return (AP_OK);
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* LoaderInit() initialize the loader.                                        */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool AudioIFFConverter::LoaderInit(int32 index)
{
	// Allocate data buffer
	loadBuffer = new int8[BUFFER_SIZE];
	if (loadBuffer == NULL)
		throw PMemoryException();

	fillCount = 0;

	return (true);
}



/******************************************************************************/
/* LoaderEnd() cleanup the loader.                                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/******************************************************************************/
void AudioIFFConverter::LoaderEnd(int32 index)
{
	// Deallocate the data buffer
	delete[] loadBuffer;
	loadBuffer = NULL;
}



/******************************************************************************/
/* LoadHeader() loads the sample header.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "convInfo" is a pointer to the info structure you has to fill out. */
/*         "errorStr" is a reference to a string to store the error in if any.*/
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result AudioIFFConverter::LoadHeader(PFile *file, APConverter_SampleFormat *convInfo, PString &errorStr)
{
	uint32 chunkName, chunkSize;
	uint8 freq[10];
	bool gotCOMM = false;
	bool gotSSND = false;

	// Initialize some fields in the converter structure to default values
	convInfo->name.MakeEmpty();
	convInfo->author.MakeEmpty();
	convInfo->flags      = 0;
	convInfo->volume     = 256;
	convInfo->panning    = -1;
	convInfo->loopStart  = 0;
	convInfo->loopLength = 0;

	// Skip the header
	file->Seek(12, PFile::pSeekBegin);

	for (;;)
	{
		// Read the chunk name and size
		chunkName = file->Read_B_UINT32();
		chunkSize = file->Read_B_UINT32();

		// Check if we reached the end of the file
		if (file->IsEOF())
			break;

		// Interpret the known chunks
		switch (chunkName)
		{
			// Common chunk
			case 'COMM':
			{
				// Begin to read the chunk data
				convInfo->channels = file->Read_B_UINT16();	// Number of channels
				file->Read_B_UINT32();						// Sample frames
				convInfo->bitSize = file->Read_B_UINT16();	// Sample size
				file->Read(freq, 10);						// Extended sample rate
				convInfo->frequency = (uint32)ConvertFromIeeeExtended(freq);
				gotCOMM = true;

				// Skip any extra data
				file->Seek((chunkSize - 18 + 1) & 0xfffffffe, PFile::pSeekCurrent);
				break;
			}

			// Sound Data chunk
			case 'SSND':
			{
				ssndLength = chunkSize;		// Remember the chunk size
				ssndStart  = file->GetPosition() + 8;
				file->Seek((chunkSize + 1) & 0xfffffffe, PFile::pSeekCurrent);
				gotSSND = true;
				break;
			}

			// Unknown chunks
			default:
			{
				int8 i;
				bool ok = true;

				// Check for valid chunk name
				for (i = 0; i < 4; i++)
				{
					uint8 byte = (uint8)(chunkName & 0xff);
					if ((byte < 32) || (byte > 127))
					{
						ok = false;
						break;
					}

					chunkName = chunkName >> 8;
				}

				// Skip the chunk
				if (ok)
					file->Seek((chunkSize + 1) & 0xfffffffe, PFile::pSeekCurrent);
				else
					file->SeekToEnd();

				break;
			}
		}
	}

	// Did we get any COMM chunk?
	if (!gotCOMM)
	{
		errorStr.LoadString(res, IDS_AUDIOIFF_ERR_NOCOMM);
		return (AP_ERROR);
	}

	// Did we get any SSND chunk?
	if (!gotSSND)
	{
		errorStr.LoadString(res, IDS_AUDIOIFF_ERR_NOSSND);
		return (AP_ERROR);
	}

	// Is the number of channels one we support
	if (convInfo->channels > 2)
	{
		errorStr.Format(res, IDS_AUDIOIFF_ERR_ILLEGALCHANNEL, convInfo->channels);
		return (AP_ERROR);
	}

	return (AP_OK);
}



/******************************************************************************/
/* LoadData() loads some part of the sample data.                             */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "buffer" is a pointer to the buffer you has to fill with the data. */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples filled in the buffer.                        */
/******************************************************************************/
uint32 AudioIFFConverter::LoadData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
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
		if (fillCount == 0)
		{
			// Yes, do it
			fillCount = file->Read(loadBuffer, BUFFER_SIZE);
			offset    = 0;

			if (fillCount == 0)
				break;			// End of file, stop filling
		}

		// Find the number of samples to return
		todo = min(length, fillCount / sampSize);

		// Copy the sample data
		switch (sampSize)
		{
			// 1-8 bit samples
			case 1:
			{
				for (i = 0; i < todo; i++)
					*buffer++ = (loadBuffer[offset++] >> shift) / 128.0f;

				break;
			}

			// 9-16 bits samples
			case 2:
			{
				for (i = 0; i < todo; i++)
				{
					*buffer++ = ((((loadBuffer[offset] & 0xff) << 8) | (loadBuffer[offset + 1] & 0xff)) >> shift) / 32768.0f;
					offset   += 2;
				}
				break;
			}

			// 17-24 bits samples
			case 3:
			{
				for (i = 0; i < todo; i++)
				{
					*buffer++ = ((((loadBuffer[offset] & 0xff) << 16) | ((loadBuffer[offset + 1] & 0xff) << 8) | (loadBuffer[offset + 2] & 0xff)) >> shift) / 8388608.0f;
					offset   += 3;
				}
				break;
			}

			// 25-32 bits samples
			case 4:
			{
				for (i = 0; i < todo; i++)
				{
					*buffer++ = ((((loadBuffer[offset] & 0xff) << 24) | ((loadBuffer[offset + 1] & 0xff) << 16) | ((loadBuffer[offset + 2] & 0xff) << 8) | (loadBuffer[offset + 3] & 0xff)) >> shift) / 2147483648.0f;
					offset   += 4;
				}
				break;
			}
		}

		// Update the counter variables
		length    -= todo;
		filled    += todo;
		fillCount -= (todo * sampSize);
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
uint32 AudioIFFConverter::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	return ((ssndLength - 8) / ((convInfo->bitSize + 7) / 8));
}



/******************************************************************************/
/* SetSamplePosition() sets the file position to the sample position given.   */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "position" is the start position from the start of the sample.     */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The real position in the sample in samples.                        */
/******************************************************************************/
uint32 AudioIFFConverter::SetSamplePosition(PFile *file, uint32 position, const APConverter_SampleFormat *convInfo)
{
	uint32 newPosition;

	// Calculate the position in bytes
	newPosition = position * ((convInfo->bitSize + 7) / 8);

	// Seek to the right position in the SSND chunk
	file->Seek(ssndStart + newPosition, PFile::pSeekBegin);

	return (position);
}



/******************************************************************************/
/* SaverInit() initialize the saver.                                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool AudioIFFConverter::SaverInit(int32 index, const APConverter_SampleFormat *convInfo)
{
	// Initialize buffer variables
	saveBuffer = NULL;
	bufLength  = 0;
	total      = 0;

	return (true);
}



/******************************************************************************/
/* SaverEnd() cleanup the saver.                                              */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void AudioIFFConverter::SaverEnd(int32 index, const APConverter_SampleFormat *convInfo)
{
	delete[] saveBuffer;
	saveBuffer = NULL;
}



/******************************************************************************/
/* SaveHeader() saves the sample header.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to save to.    */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result AudioIFFConverter::SaveHeader(PFile *file, const APConverter_SampleFormat *convInfo)
{
	PString tempStr;
	PCharSet_Macintosh_Roman charSet;
	char *str;
	int32 len;
	uint8 freq[10];

	// Write the IFF header
	file->Write_B_UINT32('FORM');
	file->Write_B_UINT32(0);					// File size
	file->Write_B_UINT32('AIFF');

	// Write the annotation chunk
	tempStr.LoadString(res, IDS_AUDIOIFF_ANNO);
	str = tempStr.GetString(&charSet, &len);

	file->Write_B_UINT32('ANNO');
	file->Write_B_UINT32(len);					// Chunk size
	file->Write(str, len);						// The annotation
	if ((len % 2) != 0)
		file->Write_UINT8(0);

	tempStr.FreeBuffer(str);

	// Write the common chunk
	commPos = file->GetPosition();				// Remember the position of the common chunk

	file->Write_B_UINT32('COMM');
	file->Write_B_UINT32(18);					// Chunk size
	file->Write_B_UINT16(convInfo->channels);	// Number of channels
	file->Write_B_UINT32(0);					// Sample frames
	file->Write_B_UINT16(convInfo->bitSize);	// Sample size

	ConvertToIeeeExtended(convInfo->frequency, freq);
	file->Write(freq, sizeof(freq));

	// Write the sound data chunk
	ssndPos = file->GetPosition();				// Remember the postion of the sound chunk

	file->Write_B_UINT32('SSND');
	file->Write_B_UINT32(0);					// Chunk size
	file->Write_B_UINT32(0);					// Offset
	file->Write_B_UINT32(0);					// Block size

	return (AP_OK);
}



/******************************************************************************/
/* SaveData() saves some part of the sample data.                             */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to save to.    */
/*         "buffer" is a pointer to the buffer with the sample data to save.  */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result AudioIFFConverter::SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 i;

	if (length > 0)
	{
		// Do we need to reallocate the buffer?
		if (length > bufLength)
		{
			// First free the old buffer
			delete[] saveBuffer;
			saveBuffer = NULL;

			// Allocate new buffer to store the converted samples into
			if (convInfo->bitSize == 8)
				saveBuffer = new int8[length];
			else
				saveBuffer = new int8[length * sizeof(int16)];

			if (saveBuffer == NULL)
				return (AP_ERROR);

			// Remember the new length
			bufLength = length;
		}

		if (convInfo->bitSize == 8)
		{
			// 8-bit output
			int8 *dest = saveBuffer;

			// Convert all the samples from float to 8-bit
			for (i = 0; i < length; i++)
				*dest++ = (int8)(*buffer++ * 128.0f);

			total += length;
		}
		else
		{
			// 16-bit output
			int16 *dest = (int16 *)saveBuffer;

			// Convert all the samples from float to 16-bit
			for (i = 0; i < length; i++)
				*dest++ = P_HOST_TO_BENDIAN_INT16((int16)(*buffer++ * 32768.0f));

			total += length * 2;
		}

		// Write the buffer to disk
		if (convInfo->bitSize == 8)
			file->Write(saveBuffer, length);
		else
			file->Write(saveBuffer, length * sizeof(int16));
	}

	return (AP_OK);
}



/******************************************************************************/
/* SaveTail() saves the sample tail.                                          */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to save to.    */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result AudioIFFConverter::SaveTail(PFile *file, const APConverter_SampleFormat *convInfo)
{
	PString mime;
	uint32 sampleFrame;

	// Change the SSND chunk size
	file->Seek(ssndPos + 4, PFile::pSeekBegin);
	file->Write_B_UINT32(total + 8);		// Add offset + block size to the total size

	// Change the sample frames in the COMM chunk
	sampleFrame = total;
	if (convInfo->bitSize == 16)
		sampleFrame /= 2;

	if (convInfo->channels == 2)
		sampleFrame /= 2;

	file->Seek(commPos + 10, PFile::pSeekBegin);
	file->Write_B_UINT32(sampleFrame);

	// Change the FORM size
	file->Seek(4, PFile::pSeekBegin);
	file->Write_B_UINT32(ssndPos + (8 + 8 + total) - 8);

	// Set the mime-type on the file
	mime.LoadString(res, IDS_AUDIOIFF_MIME);
	file->SetFileType(mime);

	return (AP_OK);
}



/******************************************************************************/
/* C O N V E R T   T O   I E E E   E X T E N D E D                            */
/*                                                                            */
/* Copyright (C) 1988-1991 Apple Computer, Inc.                               */
/* All rights reserved.                                                       */
/*                                                                            */
/* Machine-independent I/O routines for IEEE floating-point numbers.          */
/*                                                                            */
/* NaN's and infinities are converted to HUGE_VAL or HUGE, which happens to   */
/* be infinity on IEEE machines.  Unfortunately, it is impossible to preserve */
/* NaN's in a machine-independent way.                                        */
/* Infinities are, however, preserved on IEEE machines.                       */
/*                                                                            */
/* These routines have been tested on the following machines:                 */
/*    Apple Macintosh, MPW 3.1 C compiler                                     */
/*    Apple Macintosh, THINK C compiler                                       */
/*    Silicon Graphics IRIS, MIPS compiler                                    */
/*    Cray X/MP and Y/MP                                                      */
/*    Digital Equipment VAX                                                   */
/*                                                                            */
/*                                                                            */
/* Implemented by Malcolm Slaney and Ken Turkowski.                           */
/*                                                                            */
/* Malcolm Slaney contributions during 1988-1990 include big- and little-     */
/* endian file I/O, conversion to and from Motorola's extended 80-bit         */
/* floating-point format, and conversions to and from IEEE single-            */
/* precision floating-point format.                                           */
/*                                                                            */
/* In 1991, Ken Turkowski implemented the conversions to and from             */
/* IEEE double-precision format, added more precision to the extended         */
/* conversions, and accommodated conversions involving +/- infinity,          */
/* NaN's, and denormalized numbers.                                           */
/******************************************************************************/
#define FloatToUnsigned(f)	((unsigned long)(((long)(f - 2147483648.0)) + 2147483647L) + 1)

void AudioIFFConverter::ConvertToIeeeExtended(double num, uint8 *bytes)
{
	int sign;
	int expon;
	double fMant, fsMant;
	unsigned long hiMant, loMant;

	if (num < 0)
	{
		sign = 0x8000;
		num *= -1.0;
	}
	else
	{
		sign = 0;
	}

	if (num == 0.0)
	{
		expon  = 0;
		hiMant = 0;
		loMant = 0;
	}
	else
	{
		fMant = frexp(num, &expon);
		if ((expon > 16384) || !(fMant < 1.0))
		{
			// Infinity or NaN
			expon  = sign | 0x7FFF;
			hiMant = 0;
			loMant = 0;	// Infinity
		}
		else
		{
			// Finite
			expon += 16382;
			if (expon < 0)
			{
				// Denormalized
				fMant = ldexp(fMant, expon);
				expon = 0;
			}

			expon |= sign;
			fMant  = ldexp(fMant, 32);
			fsMant = floor(fMant);
			hiMant = FloatToUnsigned(fsMant);
			fMant  = ldexp(fMant - fsMant, 32);
			fsMant = floor(fMant);
			loMant = FloatToUnsigned(fsMant);
		}
	}

	bytes[0] = expon >> 8;
	bytes[1] = expon;
	bytes[2] = hiMant >> 24;
	bytes[3] = hiMant >> 16;
	bytes[4] = hiMant >> 8;
	bytes[5] = hiMant;
	bytes[6] = loMant >> 24;
	bytes[7] = loMant >> 16;
	bytes[8] = loMant >> 8;
	bytes[9] = loMant;
}



/******************************************************************************/
/* C O N V E R T   F R O M   I E E E   E X T E N D E D                        */
/*                                                                            */
/* Copyright (C) 1988-1991 Apple Computer, Inc.                               */
/* All rights reserved.                                                       */
/*                                                                            */
/* Machine-independent I/O routines for IEEE floating-point numbers.          */
/*                                                                            */
/* NaN's and infinities are converted to HUGE_VAL or HUGE, which              */
/* happens to be infinity on IEEE machines.  Unfortunately, it is             */
/* impossible to preserve NaN's in a machine-independent way.                 */
/* Infinities are, however, preserved on IEEE machines.                       */
/*                                                                            */
/* These routines have been tested on the following machines:                 */
/*    Apple Macintosh, MPW 3.1 C compiler                                     */
/*    Apple Macintosh, THINK C compiler                                       */
/*    Silicon Graphics IRIS, MIPS compiler                                    */
/*    Cray X/MP and Y/MP                                                      */
/*    Digital Equipment VAX                                                   */
/*                                                                            */
/*                                                                            */
/* Implemented by Malcolm Slaney and Ken Turkowski.                           */
/*                                                                            */
/* Malcolm Slaney contributions during 1988-1990 include big- and little-     */
/* endian file I/O, conversion to and from Motorola's extended 80-bit         */
/* floating-point format, and conversions to and from IEEE single-            */
/* precision floating-point format.                                           */
/*                                                                            */
/* In 1991, Ken Turkowski implemented the conversions to and from             */
/* IEEE double-precision format, added more precision to the extended         */
/* conversions, and accommodated conversions involving +/- infinity,          */
/* NaN's, and denormalized numbers.                                           */
/******************************************************************************/
#ifndef HUGE_VAL
#define HUGE_VAL HUGE
#endif

#define UnsignedToFloat(u)	(((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

double AudioIFFConverter::ConvertFromIeeeExtended(const uint8 *bytes)
{
	double f;
	int expon;
	unsigned long hiMant, loMant;

	expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
	hiMant = ((unsigned long)(bytes[2] & 0xFF) << 24) |
			 ((unsigned long)(bytes[3] & 0xFF) << 16) |
			 ((unsigned long)(bytes[4] & 0xFF) << 8) |
			 ((unsigned long)(bytes[5] & 0xFF));
	loMant = ((unsigned long)(bytes[6] & 0xFF) << 24) |
			 ((unsigned long)(bytes[7] & 0xFF) << 16) |
			 ((unsigned long)(bytes[8] & 0xFF) << 8) |
			 ((unsigned long)(bytes[9] & 0xFF));

	if ((expon == 0) && (hiMant == 0) && (loMant == 0))
		f = 0;
	else
	{
		if (expon == 0x7FFF)
		{
			// Infinity or NaN
			f = HUGE_VAL;
		}
		else
		{
			expon -= 16383;
			f  = ldexp(UnsignedToFloat(hiMant), expon -= 31);
			f += ldexp(UnsignedToFloat(loMant), expon -= 32);
		}
	}

	if (bytes[0] & 0x80)
		return (-f);

	return (f);
}
