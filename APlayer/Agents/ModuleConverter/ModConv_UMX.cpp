/******************************************************************************/
/* ModuleConverter Unreal class.                                              */
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

// Agent headers
#include "ModuleConverter.h"
#include "ModuleConverterAgent.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* CheckModule() will be check the module to see if it's a Unreal module.     */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a converter info structure where to     */
/*         read and store information needed.                                 */
/******************************************************************************/
bool ModConv_UMX::CheckModule(APAgent_ConvertModule *convInfo)
{
	PString tempStr;
	PFile *file = convInfo->moduleFile;

	// Check the module size
	if (file->GetLength() < 256)
		return (false);

	// Read the start mark
	file->SeekToBegin();
	if (file->Read_L_UINT32() != 0x9e2a83c1)
		return (false);

	// Check the second mark
	file->Seek(54, PFile::pSeekBegin);
	if (file->Read_B_UINT32() == 'DEST')
	{
		// Unreal files
		//
		// Seek to the type strings
		file->Seek(2, PFile::pSeekCurrent);
		if (file->Read_B_UINT32() != 'None')
			return (true);

		file->Seek(5, PFile::pSeekCurrent);
		if (file->Read_B_UINT32() == 'WAV\0')
			return (false);
	}
	else
	{
		// Unreal Tournament files
		int32 offset;

		// Get the offset to the types
		file->Seek(16, PFile::pSeekBegin);
		offset = file->Read_L_UINT32();

		// Seek to the type strings
		file->Seek(offset + 1, PFile::pSeekBegin);

		// Double check the next two strings so we won't play samples
		if (file->Read_B_UINT32() != 'None')
			return (true);

		file->Seek(6, PFile::pSeekCurrent);
		if (file->Read_B_UINT32() == 'WAV\0')
			return (false);
	}

	return (true);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from Unreal to whatever there */
/*      are inside the "box".                                                 */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a converter info structure where to     */
/*         read and store information needed.                                 */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result ModConv_UMX::ConvertModule(APAgent_ConvertModule *convInfo, PFile *destFile)
{
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		PCharSet_MS_WIN_1252 charSet;
		char strBuffer[100];
		uint32 typeOffset, endOffset, curOffset, temp;
		uint32 extra = 0;
		uint16 numStr, i;
		PString kind, workStr;
		bool tournament = false;

		// Get the number of strings
		file->Seek(12, PFile::pSeekBegin);
		numStr = file->Read_L_UINT32() - 1;

		// Get the type offset
		typeOffset = file->Read_L_UINT32();

		// Get the end offset
		file->Seek(32, PFile::pSeekBegin);
		endOffset = file->Read_L_UINT32();

		// Find out if the module is an Unreal or Unreal Tournament
		file->Seek(54, PFile::pSeekBegin);
		if (file->Read_B_UINT32() != 'DEST')
		{
			tournament = true;
			extra      = 1;
		}

		// Read the module kind
		curOffset = typeOffset;
		file->Seek(curOffset, PFile::pSeekBegin);
		file->Read(strBuffer, sizeof(strBuffer));

		if (strBuffer[0] < 0x40)
		{
			kind.SetString(&strBuffer[1], &charSet);
			curOffset++;

			tournament = true;
			extra      = 1;
		}
		else
		{
			kind.SetString(strBuffer, &charSet);
			extra = 0;
		}

		curOffset += kind.GetLength() + 1 + 4 + extra;
		file->Seek(curOffset, PFile::pSeekBegin);

		// Skip all the strings
		for (i = 0; i < numStr; i++)
		{
			// Read the chunk string
			file->Read(strBuffer, sizeof(strBuffer));
			workStr.SetString(strBuffer, &charSet);

			// Did we get an EOF
			if (file->IsEOF())
			{
				ShowError(IDS_MODC_ERR_LOADING_HEADER);
				throw PUserException();
			}

			// If the kind is "None", then use the second kind
			// string instead
			if (kind == "None")
				kind = workStr;

			// Seek to just after the string + 4 bytes
			curOffset += workStr.GetLength() + 1 + 4 + extra;
			file->Seek(curOffset, PFile::pSeekBegin);
		}

		// Got all the strings, now skip some extra bytes
		if (tournament)
			file->Seek(9, PFile::pSeekCurrent);
		else
			file->Seek(5, PFile::pSeekCurrent);

		// This is a hack, so the EndEx.umx module can be loaded.
		// For some reason, there is an extra bytes just before
		// the module itself. I can't figure out why this byte is
		// there, therefore the hack.
		//
		// Most of the tournament files also have this byte
		if (file->Read_UINT8() > 0x08)
			file->Seek(-1, PFile::pSeekCurrent);

		// Another hack for the Mission.umx module
		temp = file->Read_B_UINT32();
		if ((kind == "it") && (temp != 'IMPM'))
			file->Seek(-5, PFile::pSeekCurrent);
		else
			file->Seek(-4, PFile::pSeekCurrent);

		// Now copy the module
		CopyData(file, destFile, endOffset - (curOffset + 5));

		// Set the module type
		if (tournament)
			convInfo->modKind.Format_S1(res, IDS_MODC_NAME_UMX_TOUR, kind);
		else
			convInfo->modKind.Format_S1(res, IDS_MODC_NAME_UMX, kind);

		convInfo->fileType.LoadString(res, IDS_MODC_MIME_UMX);
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	return (retVal);
}
