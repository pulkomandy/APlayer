/******************************************************************************/
/* RIFF-WAVE Converter Interface.                                             */
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
#include "RIFF-WAVEConverter.h"
#include "RIFF-WAVESettingsWindow.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define ConverterVersion		2.06f



/******************************************************************************/
/* Extern global functions and variables                                      */
/******************************************************************************/
extern PSettings *useSettings;



/******************************************************************************/
/* RIFFWAVEConverter class                                                    */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
RIFFWAVEConverter::RIFFWAVEConverter(APGlobalData *global, PString fileName) : APAddOnConverter(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	saveBuffer = NULL;
	format     = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
RIFFWAVEConverter::~RIFFWAVEConverter(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float RIFFWAVEConverter::GetVersion(void)
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
uint32 RIFFWAVEConverter::GetSupportFlags(int32 index)
{
	return (apcLoader | apcSaver | apcSaverSettings | apcSupport8Bit | apcSupport16Bit | apcSupportMono | apcSupportStereo);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString RIFFWAVEConverter::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_RIFFWAVE_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString RIFFWAVEConverter::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_RIFFWAVE_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetExtension() returns the file extension if any.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The file extension.                                                */
/******************************************************************************/
PString RIFFWAVEConverter::GetExtension(int32 index)
{
	PString ext;

	ext.LoadString(res, IDS_RIFFWAVE_EXTENSION_TO_USE);
	return (ext);
}



/******************************************************************************/
/* GetTypeString() returns the sample type string.                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The sample type string.                                            */
/******************************************************************************/
PString RIFFWAVEConverter::GetTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_RIFFWAVE_MIME);
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
ap_result RIFFWAVEConverter::FileCheck(int32 index, PFile *file)
{
	// Seek to the start of the file
	file->SeekToBegin();

	// Check the chunk names
	if (file->Read_B_UINT32() == 'RIFF')
	{
		file->Seek(4, PFile::pSeekCurrent);
		if (file->Read_B_UINT32() == 'WAVE')
		{
			uint32 chunkName, chunkSize;

			// See if we can find the 'fmt ' chunk
			for (;;)
			{
				// Read the chunk name and size
				chunkName = file->Read_B_UINT32();
				chunkSize = file->Read_L_UINT32();

				// Check if we reached the end of the file
				if (file->IsEOF())
					return (AP_UNKNOWN);

				if (chunkName == 'fmt ')
				{
					// Got it, check the format
					switch (file->Read_L_UINT16())
					{
						case WAVE_FORMAT_PCM:
						case WAVE_FORMAT_ADPCM:
							return (AP_OK);
					}

					// Not a known format :(
					return (AP_UNKNOWN);
				}

				// Skip the chunk
				file->Seek((chunkSize + 1) & 0xfffffffe, PFile::pSeekCurrent);
			}
		}
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
bool RIFFWAVEConverter::LoaderInit(int32 index)
{
	// Initialize loader variables
	sampFormat = 0;
	format     = NULL;

	return (true);
}



/******************************************************************************/
/* LoaderEnd() cleanup the loader.                                            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/******************************************************************************/
void RIFFWAVEConverter::LoaderEnd(int32 index)
{
	if (format != NULL)
	{
		// Call the cleanup functions
		format->LoaderCleanup();
		format->CleanupBasicLoader();

		delete format;
		format = NULL;
	}
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
ap_result RIFFWAVEConverter::LoadHeader(PFile *file, APConverter_SampleFormat *convInfo, PString &errorStr)
{
	uint32 chunkName, chunkSize;
	bool gotFmt = false;
	bool gotData = false;
	bool gotFact = false;
	bool needFact = true;

	try
	{
		// Initialize some of the converter structure to default values
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
			chunkSize = file->Read_L_UINT32();

			// Check if we reached the end of the file
			if (file->IsEOF())
				break;

			// Interpret the known chunks
			switch (chunkName)
			{
				// Format chunk
				case 'fmt ':
				{
					uint32 extraData;

					// Begin to read the chunk data
					sampFormat          = file->Read_L_UINT16();	// Sample format
					convInfo->channels  = file->Read_L_UINT16();	// Number of channels
					convInfo->frequency = file->Read_L_UINT32();	// Sample rate
					bytesPerSecond      = file->Read_L_UINT32();	// Average bytes per second
					blockAlign          = file->Read_L_UINT16();	// Block align
					convInfo->bitSize   = file->Read_L_UINT16();	// Sample size

					// See if the sample format is one we know
					// and then read the extra data for the format
					switch (sampFormat)
					{
						case WAVE_FORMAT_PCM:
						{
							needFact = false;
							format = new RIFFWAVEMicrosoftPCM(res);
							break;
						}

						case WAVE_FORMAT_ADPCM:
						{
							needFact = false;
							format = new RIFFWAVEMicrosoftADPCM(res);
							break;
						}
					}

					// Did we get a loader
					if (format == NULL)
						throw PMemoryException();

					// Initialize the loader
					format->InitBasicLoader(blockAlign);

					// Read extra header informations
					extraData = format->LoadExtraHeaderInfo(file, convInfo, errorStr);

					// Skip any extra data
					file->Seek((chunkSize - 16 - extraData + 1) & 0xfffffffe, PFile::pSeekCurrent);
					gotFmt = true;
					break;
				}

				// Fact chunk
				case 'fact':
				{
					uint32 factSize = 0;

					// Load the fact chunk
					if (format != NULL)
					{
						factSize = format->LoadFactChunk(file);
						gotFact  = true;
					}

					// Skip any extra data
					file->Seek((chunkSize - factSize + 1) & 0xfffffffe, PFile::pSeekCurrent);
					break;
				}

				// Data chunk
				case 'data':
				{
					// Remember the position where the data starts
					dataStart = file->GetPosition();

					// Initialize the loader
					format->LoaderInitialize(chunkSize, convInfo);

					file->Seek((chunkSize + 1) & 0xfffffffe, PFile::pSeekCurrent);
					gotData = true;
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

		// Did we get any fmt chunk?
		if (!gotFmt)
		{
			errorStr.LoadString(res, IDS_RIFFWAVE_ERR_NOFMT);
			return (AP_ERROR);
		}

		// Did we get any fact chunk?
		if (needFact && !gotFact)
		{
			errorStr.LoadString(res, IDS_RIFFWAVE_ERR_NOFACT);
			return (AP_ERROR);
		}

		// Did we get any data chunk?
		if (!gotData)
		{
			errorStr.LoadString(res, IDS_RIFFWAVE_ERR_NODATA);
			return (AP_ERROR);
		}

		// Is the number of channels one we support
		if (convInfo->channels > 2)
		{
			errorStr.Format(res, IDS_RIFFWAVE_ERR_ILLEGALCHANNEL, convInfo->channels);
			return (AP_ERROR);
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
uint32 RIFFWAVEConverter::LoadData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	ASSERT(format != NULL);

	return (format->DecodeSampleData(file, buffer, length, convInfo));
}



/******************************************************************************/
/* GetTotalSampleLength() calculates how many samples that will be returned.  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples the file is.                                 */
/******************************************************************************/
uint32 RIFFWAVEConverter::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
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
uint32 RIFFWAVEConverter::SetSamplePosition(PFile *file, uint32 position, const APConverter_SampleFormat *convInfo)
{
	uint32 sampPos;

	ASSERT(format != NULL);

	// Reset loader
	format->ResetBasicLoader();

	// Calculate the position in bytes
	position = format->CalcFilePosition(position, convInfo);
	sampPos  = format->CalcSamplePosition(position, convInfo);

	// Seek to the right position in the data chunk
	file->Seek(dataStart + position, PFile::pSeekBegin);

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
bool RIFFWAVEConverter::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 1)
		return (false);

	// Return the format of the sample
	description.LoadString(res, IDS_RIFFWAVE_INFODESCLINE0);
	value.LoadString(res, IDS_RIFFWAVE_FORMATSTART + sampFormat);

	return (true);
}



/******************************************************************************/
/* ShowSaverSettings() will open a window showing the parameters to change in */
/*      the saver routine.                                                    */
/*                                                                            */
/* Output: Pointer to the window that have been opened.                       */
/******************************************************************************/
BWindow *RIFFWAVEConverter::ShowSaverSettings(void)
{
	PString title;
	RIFFWAVESettingsWindow *window;

	// Create the window
	title.LoadString(res, IDS_RIFFWAVE_SETTINGS_TITLE);
	window = new RIFFWAVESettingsWindow(res, title);
	if (window == NULL)
		throw PMemoryException();

	// Show the window for the user
	window->Show();

	return (window);
}



/******************************************************************************/
/* SaverInit() initialize the saver.                                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool RIFFWAVEConverter::SaverInit(int32 index, const APConverter_SampleFormat *convInfo)
{
	PString formatStr, testStr;

	// Initialize buffer variables
	saveBuffer = NULL;
	bufLength  = 0;
	total      = 0;

	// Allocate the format object
	formatStr = useSettings->GetStringEntryValue("General", "OutputFormat");
	if (formatStr.IsEmpty())
		return (false);

	testStr.LoadString(res, IDS_RIFFWAVE_FORMAT_PCM);
	if (formatStr == testStr)
		format = new RIFFWAVEMicrosoftPCM(res);
	else
	{
		testStr.LoadString(res, IDS_RIFFWAVE_FORMAT_MSADPCM);
		if (formatStr == testStr)
			format = new RIFFWAVEMicrosoftADPCM(res);
		else
			return (false);
	}

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
void RIFFWAVEConverter::SaverEnd(int32 index, const APConverter_SampleFormat *convInfo)
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
ap_result RIFFWAVEConverter::SaveHeader(PFile *file, const APConverter_SampleFormat *convInfo)
{
	int32 pos;

	// Write the RIFF header
	file->Write_B_UINT32('RIFF');
	file->Write_L_UINT32(0);										// File size
	file->Write_B_UINT32('WAVE');

	// Write the fmt chunk
	file->Write_B_UINT32('fmt ');
	file->Write_L_UINT32(0);										// Chunk size
	file->Write_L_UINT16(format->GetFormatNumber());				// Format ID
	file->Write_L_UINT16(convInfo->channels);						// Number of channels
	file->Write_L_UINT32(convInfo->frequency);						// Sampling rate
	file->Write_L_UINT32(format->GetAverageBytesSecond(convInfo));	// Average bytes per second
	file->Write_L_UINT16(format->GetBlockAlign(convInfo));			// Block align
	file->Write_L_UINT16(format->GetSampleSize(convInfo->bitSize));	// Sample size

	// Write extra fmt chunk information
	format->WriteExtraFmtInfo(file);

	// Write the chunk size
	pos = file->GetPosition();
	file->Seek(16, PFile::pSeekBegin);
	file->Write_L_UINT32(pos - 36 + 16);
	file->SeekToEnd();

	// Write the fact chunk
	format->WriteFactChunk(file);

	// Write the data chunk
	dataPos = file->GetPosition();	// Remember the postion of the data chunk

	file->Write_B_UINT32('data');
	file->Write_L_UINT32(0);										// Chunk size

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
ap_result RIFFWAVEConverter::SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
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
				*dest++ = ((int8)(*buffer++ * 128.0f));
		}
		else
		{
			// 16-bit output
			int16 *dest = (int16 *)saveBuffer;

			// Convert all the samples from float to 16-bit
			for (i = 0; i < length; i++)
				*dest++ = ((int16)(*buffer++ * 32768.0f));
		}

		// Write the buffer to disk
		total += format->WriteData(file, saveBuffer, length, convInfo);
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
ap_result RIFFWAVEConverter::SaveTail(PFile *file, const APConverter_SampleFormat *convInfo)
{
	PString mime;

	// Write anything left
	total += format->WriteEnd(file, convInfo);

	// Change the data chunk size
	file->Seek(dataPos + 4, PFile::pSeekBegin);
	file->Write_L_UINT32(total);

	// Change the RIFF size
	file->Seek(4, PFile::pSeekBegin);
	file->Write_L_UINT32(dataPos + (8 + total) - 8);

	// Set the mime-type on the file
	mime.LoadString(res, IDS_RIFFWAVE_MIME);
	file->SetFileType(mime);

	return (AP_OK);
}





/******************************************************************************/
/* RIFFWAVEFormat class                                                       */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
RIFFWAVEFormat::RIFFWAVEFormat(PResource *resource)
{
	// Initialize member variables
	res        = resource;
	loadBuffer = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
RIFFWAVEFormat::~RIFFWAVEFormat(void)
{
}



/******************************************************************************/
/* LoadExtraHeaderInfo() loads and extra header informations from the 'fmt '  */
/*      chunk.                                                                */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to read from.               */
/*         "convInfo" is a pointer to the info structure.                     */
/*         "errorStr" is a reference to a string to store the error in if any.*/
/*                                                                            */
/* Output: Number of bytes read.                                              */
/******************************************************************************/
uint32 RIFFWAVEFormat::LoadExtraHeaderInfo(PFile *file, const APConverter_SampleFormat *convInfo, PString &errorStr)
{
	return (0);
}



/******************************************************************************/
/* LoadFactChunk() loads the 'fact' chunk.                                    */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to read from.               */
/*                                                                            */
/* Output: Number of bytes read.                                              */
/******************************************************************************/
uint32 RIFFWAVEFormat::LoadFactChunk(PFile *file)
{
	return (0);
}



/******************************************************************************/
/* InitBasicLoader() initialize the loader part.                              */
/*                                                                            */
/* Input:  "blkAlign" is the block align in the fmt chunk.                    */
/******************************************************************************/
void RIFFWAVEFormat::InitBasicLoader(uint16 blkAlign)
{
	// Allocate load buffer
	loadBuffer = new int8[BUFFER_SIZE];
	if (loadBuffer == NULL)
		throw PMemoryException();

	// Initialize loader variables
	ResetBasicLoader();

	// Initialize other member variables
	blockAlign = blkAlign;
}



/******************************************************************************/
/* CleanupBasicLoader() clean up the loader part.                             */
/******************************************************************************/
void RIFFWAVEFormat::CleanupBasicLoader(void)
{
	// Deallocate the data buffer
	delete[] loadBuffer;
	loadBuffer = NULL;
}



/******************************************************************************/
/* ResetBasicLoader() resets the loader variables.                            */
/******************************************************************************/
void RIFFWAVEFormat::ResetBasicLoader(void)
{
	loadBufferFillCount = 0;
	loadBufferOffset    = 0;
	samplesLeft         = 0;
}



/******************************************************************************/
/* SaverInitialize() initialize the saver.                                    */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void RIFFWAVEFormat::SaverInitialize(const APConverter_SampleFormat *convInfo)
{
}



/******************************************************************************/
/* SaverCleanup() frees all the resources allocated.                          */
/******************************************************************************/
void RIFFWAVEFormat::SaverCleanup(void)
{
}



/******************************************************************************/
/* GetFormatNumber() returns the format number.                               */
/*                                                                            */
/* Output: The format number.                                                 */
/******************************************************************************/
uint16 RIFFWAVEFormat::GetFormatNumber(void) const
{
	// You need to implement this function!
	ASSERT(false);
	return (0);
}



/******************************************************************************/
/* GetAverageBytesSecond() returns the average bytes per second.              */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The average bytes per second.                                      */
/******************************************************************************/
uint32 RIFFWAVEFormat::GetAverageBytesSecond(const APConverter_SampleFormat *convInfo)
{
	// You need to implement this function!
	ASSERT(false);
	return (0);
}



/******************************************************************************/
/* GetBlockAlign() returns the block align.                                   */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The block align.                                                   */
/******************************************************************************/
uint16 RIFFWAVEFormat::GetBlockAlign(const APConverter_SampleFormat *convInfo)
{
	// You need to implement this function!
	ASSERT(false);
	return (0);
}



/******************************************************************************/
/* GetSampleSize() returns the bit size of each sample.                       */
/*                                                                            */
/* Input:  "sampSize" is the input sample size.                               */
/*                                                                            */
/* Output: The new sample size.                                               */
/******************************************************************************/
uint16 RIFFWAVEFormat::GetSampleSize(uint16 sampSize)
{
	return (sampSize);
}



/******************************************************************************/
/* WriteExtraFmtInfo() writes any extra information into the fmt chunk.       */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/******************************************************************************/
void RIFFWAVEFormat::WriteExtraFmtInfo(PFile *file)
{
}



/******************************************************************************/
/* WriteFactChunk() writes the fact chunk.                                    */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/******************************************************************************/
void RIFFWAVEFormat::WriteFactChunk(PFile *file)
{
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
uint32 RIFFWAVEFormat::WriteData(PFile *file, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	return (0);
}



/******************************************************************************/
/* WriteEnd() write the last data or fixing up chunks.                        */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of bytes written as sample data.                        */
/******************************************************************************/
uint32 RIFFWAVEFormat::WriteEnd(PFile *file, const APConverter_SampleFormat *convInfo)
{
	return (0);
}



/******************************************************************************/
/* GetFileData() loads some part of the sample data from the file and return  */
/*      them.                                                                 */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "buffer" is a pointer to the buffer to fill with the data.         */
/*         "length" is the length of the buffer in bytes.                     */
/*                                                                            */
/* Output: The number of bytes filled in the buffer.                          */
/******************************************************************************/
uint32 RIFFWAVEFormat::GetFileData(PFile *file, int8 *buffer, uint32 length)
{
	uint32 todo;
	uint32 total = 0;

	while (length > 0)
	{
		// If the number of bytes taken from the buffer is equal to the fill
		// count, load some new data from the file
		if (loadBufferOffset == loadBufferFillCount)
		{
			// Load the data
			loadBufferFillCount = file->Read(loadBuffer, BUFFER_SIZE);
			loadBufferOffset    = 0;

			// Well, there isn't more data to read
			if (loadBufferFillCount == 0)
				break;
		}

		// Find out how many bytes to copy
		todo = min(length, loadBufferFillCount - loadBufferOffset);

		// Copy the data
		memcpy(buffer, loadBuffer + loadBufferOffset, todo);

		// Adjust the variables
		loadBufferOffset += todo;
		length           -= todo;
		total            += todo;
	}

	return (total);
}
