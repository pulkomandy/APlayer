/******************************************************************************/
/* Raw Converter Interface.                                                   */
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
#include "RawConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define ConverterVersion		2.02f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
RawConverter::RawConverter(APGlobalData *global, PString fileName) : APAddOnConverter(global)
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
RawConverter::~RawConverter(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float RawConverter::GetVersion(void)
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
uint32 RawConverter::GetSupportFlags(int32 index)
{
	return (apcSaver | apcSupport8Bit | apcSupport16Bit | apcSupportMono | apcSupportStereo);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString RawConverter::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_RAW_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString RawConverter::GetDescription(int32 index)
{
	return ("");
}



/******************************************************************************/
/* GetExtension() returns the file extension if any.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The file extension.                                                */
/******************************************************************************/
PString RawConverter::GetExtension(int32 index)
{
	PString ext;

	ext.LoadString(res, IDS_RAW_EXTENSION_TO_USE);
	return (ext);
}



/******************************************************************************/
/* SaverInit() initialize the saver.                                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool RawConverter::SaverInit(int32 index, const APConverter_SampleFormat *convInfo)
{
	// Initialize buffer variables
	saveBuffer = NULL;
	bufLength  = 0;

	return (true);
}



/******************************************************************************/
/* SaverEnd() cleanup the saver.                                              */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void RawConverter::SaverEnd(int32 index, const APConverter_SampleFormat *convInfo)
{
	delete[] saveBuffer;
	saveBuffer = NULL;
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
ap_result RawConverter::SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 i;

	if (length > 0)
	{
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

			for (i = 0; i < length; i++)
				*dest++ = (int8)(*buffer++ * 128.0f);
		}
		else
		{
			// 16-bit output
			int16 *dest = (int16 *)saveBuffer;

			// Convert all the samples from float to 16-bit
			for (i = 0; i < length; i++)
				*dest++ = (int16)(*buffer++ * 32768.0f);
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
ap_result RawConverter::SaveTail(PFile *file, const APConverter_SampleFormat *convInfo)
{
	PString type;

	// Set the mime-type on the file
	type.LoadString(res, IDS_RAW_MIME);
	file->SetFileType(type);

	return (AP_OK);
}
