/******************************************************************************/
/* IFF-16SV Converter Interface.                                              */
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
#include "PSettings.h"
#include "PFile.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"

// Converter headers
#include "IFF-16SVConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define ConverterVersion		2.00f



/******************************************************************************/
/* IFF16SVConverter class                                                     */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
IFF16SVConverter::IFF16SVConverter(APGlobalData *global, PString fileName) : APAddOnConverter(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	format     = NULL;
	file2      = NULL;
//XX	stereoFile = NULL;
//XX	saveBuffer = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
IFF16SVConverter::~IFF16SVConverter(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float IFF16SVConverter::GetVersion(void)
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
uint32 IFF16SVConverter::GetSupportFlags(int32 index)
{
	return (apcLoader | apcSaver | apcSupport16Bit | apcSupportMono | apcSupportStereo);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString IFF16SVConverter::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_IFF16SV_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString IFF16SVConverter::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_IFF16SV_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetExtension() returns the file extension if any.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The file extension.                                                */
/******************************************************************************/
PString IFF16SVConverter::GetExtension(int32 index)
{
	PString ext;

	ext.LoadString(res, IDS_IFF16SV_EXTENSION_TO_USE);
	return (ext);
}



/******************************************************************************/
/* GetTypeString() returns the sample type string.                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The sample type string.                                            */
/******************************************************************************/
PString IFF16SVConverter::GetTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_IFF16SV_MIME);
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
ap_result IFF16SVConverter::FileCheck(int32 index, PFile *file)
{
	// Seek to the start of the file
	file->SeekToBegin();

	// Check the chunk names
	if (file->Read_B_UINT32() == 'FORM')
	{
		file->Seek(4, PFile::pSeekCurrent);
		if (file->Read_B_UINT32() == '16SV')
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
bool IFF16SVConverter::LoaderInit(int32 index)
{
	// Initialize loader variables
	sampFormat = 0;
	format     = NULL;
	file2      = NULL;

	return (true);
}



/******************************************************************************/
/* LoaderEnd() cleanup the loader.                                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/******************************************************************************/
void IFF16SVConverter::LoaderEnd(int32 index)
{
	if (format != NULL)
	{
		// Call the cleanup functions
		format->LoaderCleanup();
		format->CleanupBasicLoader();

		delete format;
		format = NULL;
	}

	// Delete the other file handle if any
	delete file2;
	file2 = NULL;
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
ap_result IFF16SVConverter::LoadHeader(PFile *file, APConverter_SampleFormat *convInfo, PString &errorStr)
{
	PCharSet_Amiga charSet;
	uint32 chunkName, chunkSize;
	char *strBuf;
	bool gotVHDR = false;
	bool gotBODY = false;

	try
	{
		// Initialize some of the converter structure to default values
		convInfo->name.MakeEmpty();
		convInfo->author.MakeEmpty();
		convInfo->flags      = 0;
		convInfo->bitSize    = 16;
		convInfo->channels   = 1;
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
				// Name chunk
				case 'NAME':
				{
					// Read in the string
					strBuf = new char[chunkSize + 1];
					if (strBuf == NULL)
						throw PMemoryException();

					file->ReadString(strBuf, chunkSize);
					convInfo->name.SetString(strBuf, &charSet);
					delete[] strBuf;

					// Skip any pad bytes
					if ((chunkSize % 2) != 0)
						file->Seek(1, PFile::pSeekCurrent);

					break;
				}

				// Author chunk
				case 'AUTH':
				{
					// Read in the string
					strBuf = new char[chunkSize + 1];
					if (strBuf == NULL)
						throw PMemoryException();

					file->ReadString(strBuf, chunkSize);
					convInfo->author.SetString(strBuf, &charSet);
					delete[] strBuf;

					// Skip any pad bytes
					if ((chunkSize % 2) != 0)
						file->Seek(1, PFile::pSeekCurrent);

					break;
				}

				// Copyright chunk
				case '(c) ':
				{
					// Read in the string
					strBuf = new char[chunkSize + 1];
					if (strBuf == NULL)
						throw PMemoryException();

					file->ReadString(strBuf, chunkSize);
					copyright.SetString(strBuf, &charSet);
					delete[] strBuf;

					// Skip any pad bytes
					if ((chunkSize % 2) != 0)
						file->Seek(1, PFile::pSeekCurrent);

					break;
				}

				// Annotation chunk
				case 'ANNO':
				{
					// Read in the string
					strBuf = new char[chunkSize + 1];
					if (strBuf == NULL)
						throw PMemoryException();

					file->ReadString(strBuf, chunkSize);
					annotation.SetString(strBuf, &charSet);
					delete[] strBuf;

					// Skip any pad bytes
					if ((chunkSize % 2) != 0)
						file->Seek(1, PFile::pSeekCurrent);

					break;
				}

				// Channel chunk
				case 'CHAN':
				{
					uint32 chanVal;

					chanVal = file->Read_B_UINT32();

					// 2 = Left channel
					// 4 = Right channel
					// 6 = Stereo
					//
					// We do only check for stereo. Mono samples
					// will be played in both speakers
					if (chanVal == 6)
						convInfo->channels = 2;

					// Skip any extra data
					file->Seek((chunkSize - 4 + 1) & 0xfffffffe, PFile::pSeekCurrent);
					break;
				}

				// Voice header chunk
				case 'VHDR':
				{
					uint32 oneShotLength, repeatLength, length;
					uint8 i, compression;

					// Begin to read the chunk data
					oneShotLength       = file->Read_B_UINT32();	// Number of samples in the one-shot part
					repeatLength        = file->Read_B_UINT32();	// Number of samples in the repeat part
					file->Seek(4, PFile::pSeekCurrent);				// Skip the samples in high octave
					convInfo->frequency = file->Read_B_UINT16();	// Sample frequency
					octaves             = file->Read_UINT8();		// Number of octaves in the file
					compression         = file->Read_UINT8();		// Compression type
					file->Seek(4, PFile::pSeekCurrent);				// Skip the volume

					if (octaves == 1)
					{
						// Only one octave, set the loop points if looping
						if (repeatLength != 0)
						{
							convInfo->loopStart  = oneShotLength;
							convInfo->loopLength = repeatLength;
							convInfo->flags     |= APSAMP_LOOP;
						}

						totalLength = oneShotLength + repeatLength;
					}
					else
					{
						// More than one octave, so fix the total length
						length       = oneShotLength + repeatLength;
						totalLength  = length;
						for (i = 1; i < octaves; i++)
						{
							length      *= 2;
							totalLength += length;
						}
					}

					// See if the sample compression format is one we know
					switch (compression)
					{
						// Plain 16-bit samples
						case IFF16SV_FORMAT_PCM:
						{
							sampFormat = 1;
							format     = new IFF16SVPCM(res);
							break;
						}

						// Unknown
						default:
						{
							errorStr.LoadString(res, IDS_IFF16SV_ERR_UNKNOWN_COMPRESSION);
							return (AP_ERROR);
						}
					}

					// Did we get a loader
					if (format == NULL)
						throw PMemoryException();

					// Initialize the loader
					format->InitBasicLoader();

					// Skip any extra data
					file->Seek((chunkSize - 20 + 1) & 0xfffffffe, PFile::pSeekCurrent);
					gotVHDR = true;
					break;
				}

				// Body chunk
				case 'BODY':
				{
					// Convert the total length from samples-pair to samples
					if (convInfo->channels == 2)
						totalLength *= 2;

					// Check for save bugs in old versions of SoundBox
					if (totalLength == chunkSize)
						totalLength /= 2;

					// Remember the position where the data starts
					dataStart1 = file->GetPosition();

					// Initialize the loader
					format->LoaderInitialize(totalLength * 2, convInfo);

					file->Seek((chunkSize + 1) & 0xfffffffe, PFile::pSeekCurrent);
					gotBODY = true;
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

					if (ok)
						file->Seek((chunkSize + 1) & 0xfffffffe, PFile::pSeekCurrent);
					else
						file->SeekToEnd();

					break;
				}
			}
		}

		// Did we get any VHDR chunk?
		if (!gotVHDR)
		{
			errorStr.LoadString(res, IDS_IFF16SV_ERR_NOVHDR);
			return (AP_ERROR);
		}

		// Did we get any BODY chunk?
		if (!gotBODY)
		{
			errorStr.LoadString(res, IDS_IFF16SV_ERR_NOBODY);
			return (AP_ERROR);
		}

		// Okay, now we got all the chunks. If the file is a stereo
		// file, we need to open the file with another file handle,
		// because the file format will store the whole sample for
		// the left channel and then the right channel
		if (convInfo->channels == 2)
		{
			file2      = file->DuplicateFile();
			dataStart2 = dataStart1 + format->GetRightChannelPosition(totalLength * 2);
		}
	}
	catch(PUserException e)
	{
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
uint32 IFF16SVConverter::LoadData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	ASSERT(format != NULL);

	return (format->DecodeSampleData(file, file2, buffer, length, convInfo));
}



/******************************************************************************/
/* GetTotalSampleLength() calculates how many samples that will be returned.  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples the file is.                                 */
/******************************************************************************/
uint32 IFF16SVConverter::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	ASSERT(format != NULL);

	return (format->GetTotalSampleLength(convInfo));
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
uint32 IFF16SVConverter::SetSamplePosition(PFile *file, uint32 position, const APConverter_SampleFormat *convInfo)
{
	uint32 filePos, sampPos;

	ASSERT(format != NULL);

	// Reset loader
	format->ResetBasicLoader();
	format->ResetLoader(position);

	// Calculate the position in bytes
	filePos = format->CalcFilePosition(position, convInfo);
	sampPos = format->CalcSamplePosition(position, convInfo);

	// Seek to the right position in the data chunk
	file->Seek(dataStart1 + filePos, PFile::pSeekBegin);

	if (file2 != NULL)
		file2->Seek(dataStart2 + filePos, PFile::pSeekBegin);

	return (sampPos);
}



/******************************************************************************/
/* GetInfoString() returns the description and value string on the line       */
/*      given. If the line is out of range, false is returned.                */
/*                                                                            */
/* Input:  "line" is the line starting from 0.                                */
/*         "description" is a reference to where to store the description.    */
/*         "value" is a reference to where to store the value.                */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool IFF16SVConverter::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 4)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Sample format
		case 0:
		{
			description.LoadString(res, IDS_IFF16SV_INFODESCLINE0);
			value.LoadString(res, IDS_IFF16SV_FORMATSTART + sampFormat);
			break;
		}

		// Octaves
		case 1:
		{
			description.LoadString(res, IDS_IFF16SV_INFODESCLINE1);
			value.SetUNumber(octaves);
			break;
		}

		// Copyright
		case 2:
		{
			description.LoadString(res, IDS_IFF16SV_INFODESCLINE2);
			if (copyright.IsEmpty())
				value.LoadString(res, IDS_IFF16SV_NA);
			else
				value = copyright;

			break;
		}

		// Annotation
		case 3:
		{
			description.LoadString(res, IDS_IFF16SV_INFODESCLINE3);
			if (annotation.IsEmpty())
				value.LoadString(res, IDS_IFF16SV_NA);
			else
				value = annotation;

			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* SaverInit() initialize the saver.                                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool IFF16SVConverter::SaverInit(int32 index, const APConverter_SampleFormat *convInfo)
{
	PString formatStr, testStr;

	// Initialize buffer variables
	stereoFile = NULL;
	saveBuffer = NULL;
	bufLength  = 0;
	total      = 0;

	// Allocate the format object
	format = new IFF16SVPCM(res);
	if (format == NULL)
		return (false);

	// Initialize the saver
	format->SaverInitialize(convInfo);

	return (true);
}



/******************************************************************************/
/* SaverEnd() cleanup the saver.                                              */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void IFF16SVConverter::SaverEnd(int32 index, const APConverter_SampleFormat *convInfo)
{
	// Cleanup the saver
	if (format != NULL)
		format->SaverCleanup();

	// Delete the buffer
	delete[] saveBuffer;
	saveBuffer = NULL;

	// Delete the format object
	delete format;
	format = NULL;
}



/******************************************************************************/
/* SaveHeader() saves the sample header.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to save to.    */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result IFF16SVConverter::SaveHeader(PFile *file, const APConverter_SampleFormat *convInfo)
{
	PCharSet_Amiga charSet;
	PString tempStr;
	char *str;
	int32 strLen;

	// Write the RIFF header
	file->Write_B_UINT32('FORM');
	file->Write_B_UINT32(0);										// File size
	file->Write_B_UINT32('16SV');

	// Write the VHDR chunk
	file->Write_B_UINT32('VHDR');
	file->Write_B_UINT32(20);										// Chunk size
	file->Write_B_UINT32(0);										// One-shot part length

	if (convInfo->loopLength <= 2)
		loopLength = 0;
	else
		loopLength = convInfo->loopLength;

	file->Write_B_UINT32(loopLength);								// Repeat part length
	file->Write_B_UINT32(0);										// Samples/cycle in the high octave (0 means unknown)
	file->Write_B_UINT16(convInfo->frequency);						// Samples per second
	file->Write_UINT8(1);											// Number of octaves
	file->Write_UINT8(format->GetFormatNumber());					// Format value
	file->Write_B_UINT32(0x00010000);								// Volume

	// Write the NAME chunk
	if (!convInfo->name.IsEmpty())
	{
		str = convInfo->name.GetString(&charSet, &strLen);

		file->Write_B_UINT32('NAME');
		file->Write_B_UINT32(strLen);								// Chunk size
		file->Write(str, strLen);									// The string

		if ((strLen % 2) != 0)
			file->Write_UINT8(0);

		convInfo->name.FreeBuffer(str);
	}

	// Write the AUTH chunk
	if (!convInfo->author.IsEmpty())
	{
		str = convInfo->author.GetString(&charSet, &strLen);

		file->Write_B_UINT32('AUTH');
		file->Write_B_UINT32(strLen);								// Chunk size
		file->Write(str, strLen);									// The string

		if ((strLen % 2) != 0)
			file->Write_UINT8(0);

		convInfo->author.FreeBuffer(str);
	}

	// Write the ANNO chunk
	tempStr.LoadString(res, IDS_IFF16SV_ANNO);
	str = tempStr.GetString(&charSet, &strLen);

	file->Write_B_UINT32('ANNO');
	file->Write_B_UINT32(strLen);									// Chunk size
	file->Write(str, strLen);										// The string

	if ((strLen % 2) != 0)
		file->Write_UINT8(0);

	tempStr.FreeBuffer(str);

	// Should we write in stereo?
	if (convInfo->channels == 2)
	{
		PString newName;

		// Yes, write a CHAN chunk
		file->Write_B_UINT32('CHAN');
		file->Write_B_UINT32(4);									// Chunk size
		file->Write_B_UINT32(6);									// Stereo indicator

		// We need to open an extra file, because IFF-16SV
		// stereo files is not stored like normal left, right, left, right...
		// IFF-16SV files are stored with all the left channel samples
		// first and then the right channel samples. The extra file
		// is used to store the right channel samples. When all the samples
		// are written, the two files (main + extra) will be appended together
		stereoFile = file->DuplicateFile();
		if (stereoFile == NULL)
			throw PMemoryException();

		// Close the duplicated file handle
		stereoFile->Close();

		// Build a new file name
		newName = file->GetFullPath() + "aps" + PString::CreateHexNumber((uintptr_t)stereoFile);
		stereoFile->Open(newName, PFile::pModeReadWrite | PFile::pModeCreate);
	}

	// Write the BODY chunk
	dataPos = file->GetPosition();	// Remember the postion of the data chunk

	file->Write_B_UINT32('BODY');
	file->Write_B_UINT32(0);										// Chunk size

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
ap_result IFF16SVConverter::SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	int16 *dest;
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
			saveBuffer = new int16[length];
			if (saveBuffer == NULL)
				return (AP_ERROR);

			// Remember the new length
			bufLength = length;
		}

		dest = saveBuffer;

		// Convert all the samples from float to 16-bit
		for (i = 0; i < length; i++)
			*dest++ = ((int16)(*buffer++ * 32768.0f));

		// Write the buffer to disk
		total += format->WriteData(file, stereoFile, saveBuffer, length, convInfo);
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
ap_result IFF16SVConverter::SaveTail(PFile *file, const APConverter_SampleFormat *convInfo)
{
	PString mime;
	uint32 bodyLen;

	// Write anything left
	total += format->WriteEnd(file, stereoFile, convInfo);

	// Change the one-shot length
	file->Seek(20, PFile::pSeekBegin);
	if (convInfo->loopStart != 0)
		file->Write_B_UINT32(convInfo->loopStart);
	else
		file->Write_B_UINT32(total - loopLength);

	// Append the right channel file with the main file if any
	if (stereoFile != NULL)
	{
		PString stereoName;
		int8 *tempBuffer;
		int32 toCopy;
		ap_result result = AP_OK;

		// Set the file pointers to the right positions
		stereoFile->SeekToBegin();
		file->SeekToEnd();

		// Allocate a temporary buffer
		tempBuffer = new int8[BUFFER_SIZE];
		if (tempBuffer != NULL)
		{
			for (;;)
			{
				toCopy = stereoFile->Read(tempBuffer, BUFFER_SIZE);
				if (toCopy == 0)
					break;

				file->Write(tempBuffer, toCopy);
				total += toCopy;
			}

			delete[] tempBuffer;
		}
		else
			result = AP_ERROR;

		// Get the full path to the file
		stereoName = stereoFile->GetFullPath();

		// Close the file
		stereoFile->Close();

		// Delete it
		stereoFile->Remove(stereoName);

		// And delete the object
		delete stereoFile;
		stereoFile = NULL;

		if (result != AP_OK)
			return (result);
	}

	// Change the BODY chunk size
	bodyLen = file->GetLength() - dataPos - 8;
	file->Seek(dataPos + 4, PFile::pSeekBegin);
	file->Write_B_UINT32(bodyLen);

	// Change the FORM size
	file->Seek(4, PFile::pSeekBegin);
	file->Write_B_UINT32(dataPos + 8 + bodyLen - 8);

	// Set the mime-type on the file
	mime.LoadString(res, IDS_IFF16SV_MIME);
	file->SetFileType(mime);

	return (AP_OK);
}





/******************************************************************************/
/* IFF16SVFormat class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
IFF16SVFormat::IFF16SVFormat(PResource *resource)
{
	// Initialize member variables
	res         = resource;
	loadBuffer1 = NULL;
	loadBuffer2 = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
IFF16SVFormat::~IFF16SVFormat(void)
{
}



/******************************************************************************/
/* ResetLoader() resets the loader variables.                                 */
/*                                                                            */
/* Input:  "position" is the new position.                                    */
/******************************************************************************/
void IFF16SVFormat::ResetLoader(uint32 position)
{
}



/******************************************************************************/
/* InitBasicLoader() initialize the loader part.                              */
/******************************************************************************/
void IFF16SVFormat::InitBasicLoader(void)
{
	// Allocate load buffer
	loadBuffer1 = new int8[BUFFER_SIZE];
	if (loadBuffer1 == NULL)
		throw PMemoryException();

	loadBuffer2 = new int8[BUFFER_SIZE];
	if (loadBuffer2 == NULL)
		throw PMemoryException();

	// Initialize loader variables
	ResetBasicLoader();
}



/******************************************************************************/
/* CleanupBasicLoader() clean up the loader part.                             */
/******************************************************************************/
void IFF16SVFormat::CleanupBasicLoader(void)
{
	// Deallocate the data buffer
	delete[] loadBuffer2;
	loadBuffer2 = NULL;

	delete[] loadBuffer1;
	loadBuffer1 = NULL;
}



/******************************************************************************/
/* ResetBasicLoader() resets the loader variables.                            */
/******************************************************************************/
void IFF16SVFormat::ResetBasicLoader(void)
{
	loadBuffer1FillCount = 0;
	loadBuffer1Offset    = 0;
	loadBuffer2FillCount = 0;
	loadBuffer2Offset    = 0;
	samplesLeft          = 0;
}



/******************************************************************************/
/* SaverInitialize() initialize the saver.                                    */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void IFF16SVFormat::SaverInitialize(const APConverter_SampleFormat *convInfo)
{
}



/******************************************************************************/
/* SaverCleanup() frees all the resources allocated.                          */
/******************************************************************************/
void IFF16SVFormat::SaverCleanup(void)
{
}



/******************************************************************************/
/* GetFormatNumber() returns the format number.                               */
/*                                                                            */
/* Output: The format number.                                                 */
/******************************************************************************/
uint16 IFF16SVFormat::GetFormatNumber(void) const
{
	// You need to implement this function!
	ASSERT(false);
	return (0);
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
uint32 IFF16SVFormat::WriteData(PFile *file, PFile *stereoFile, int16 *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	return (0);
}



/******************************************************************************/
/* WriteEnd() write the last data or fixing up chunks.                        */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/*         "stereoFile" is a pointer to the right channel file object to      */
/*         write to or NULL.                                                  */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples written.                                     */
/******************************************************************************/
uint32 IFF16SVFormat::WriteEnd(PFile *file, PFile *stereoFile, const APConverter_SampleFormat *convInfo)
{
	return (0);
}



/******************************************************************************/
/* GetFileData1() loads some part of the sample data from the file and return */
/*      them.                                                                 */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "buffer" is a pointer to the buffer to fill with the data.         */
/*         "length" is the length of the buffer in bytes.                     */
/*                                                                            */
/* Output: The number of bytes filled in the buffer.                          */
/******************************************************************************/
uint32 IFF16SVFormat::GetFileData1(PFile *file, int8 *buffer, uint32 length)
{
	uint32 todo;
	uint32 total = 0;

	while (length > 0)
	{
		// If the number of bytes taken from the buffer is equal to the fill
		// count, load some new data from the file
		if (loadBuffer1Offset == loadBuffer1FillCount)
		{
			// Load the data
			loadBuffer1FillCount = file->Read(loadBuffer1, BUFFER_SIZE);
			loadBuffer1Offset    = 0;

			// Well, there isn't more data to read
			if (loadBuffer1FillCount == 0)
				break;
		}

		// Find out how many bytes to copy
		todo = min(length, loadBuffer1FillCount - loadBuffer1Offset);

		// Copy the data
		memcpy(buffer, loadBuffer1 + loadBuffer1Offset, todo);

		// Adjust the variables
		loadBuffer1Offset += todo;
		length            -= todo;
		total             += todo;
	}

	return (total);
}



/******************************************************************************/
/* GetFileData2() loads some part of the sample data from the file and return */
/*      them.                                                                 */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "buffer" is a pointer to the buffer to fill with the data.         */
/*         "length" is the length of the buffer in bytes.                     */
/*                                                                            */
/* Output: The number of bytes filled in the buffer.                          */
/******************************************************************************/
uint32 IFF16SVFormat::GetFileData2(PFile *file, int8 *buffer, uint32 length)
{
	uint32 todo;
	uint32 total = 0;

	while (length > 0)
	{
		// If the number of bytes taken from the buffer is equal to the fill
		// count, load some new data from the file
		if (loadBuffer2Offset == loadBuffer2FillCount)
		{
			// Load the data
			loadBuffer2FillCount = file->Read(loadBuffer2, BUFFER_SIZE);
			loadBuffer2Offset    = 0;

			// Well, there isn't more data to read
			if (loadBuffer2FillCount == 0)
				break;
		}

		// Find out how many bytes to copy
		todo = min(length, loadBuffer2FillCount - loadBuffer2Offset);

		// Copy the data
		memcpy(buffer, loadBuffer2 + loadBuffer2Offset, todo);

		// Adjust the variables
		loadBuffer2Offset += todo;
		length            -= todo;
		total             += todo;
	}

	return (total);
}
