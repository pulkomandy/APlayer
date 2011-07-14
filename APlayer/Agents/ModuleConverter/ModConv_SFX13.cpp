/******************************************************************************/
/* ModuleConverter SoundFX 1.3 class.                                         */
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
/* CheckModule() will be check the module to see if it's a SFX13 module.      */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a converter info structure where to     */
/*         read and store information needed.                                 */
/******************************************************************************/
bool ModConv_SFX13::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint32 sampleSizes[15];
	uint32 total, fileSize;
	int8 i;
	PFile *file = convInfo->moduleFile;

	// Check the module size
	fileSize = file->GetLength();
	if (fileSize < 80)
		return (false);

	// Read the sample size tabel
	file->SeekToBegin();
	file->ReadArray_B_UINT32s(sampleSizes, 15);

	// Check the mark
	if (file->Read_B_UINT32() != 'SONG')
		return (false);

	// Check the sample sizes
	total = 0;
	for (i = 0; i < 15; i++)
		total += sampleSizes[i];

	if (total > fileSize)
		return (false);

	return (true);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from SFX13 to SFX20.          */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a converter info structure where to     */
/*         read and store information needed.                                 */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result ModConv_SFX13::ConvertModule(APAgent_ConvertModule *convInfo, PFile *destFile)
{
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		uint8 name[22];
		uint8 i;

		// Copy the sample size tabel
		CopyData(file, destFile, 15 * 4);

		// Fill the rest of the sample size table with zeros
		for (i = 0; i < 16; i++)
			destFile->Write_B_UINT32(0);

		// Write the ID mark
		file->Seek(4, PFile::pSeekCurrent);
		destFile->Write_B_UINT32('SO31');

		// Copy the delay value and pads
		CopyData(file, destFile, 2 + 14);

		// Copy the first 15 sample informations
		CopyData(file, destFile, 30 * 15);

		// Write 16 empty samples
		memset(name, 0, 22);
		for (i = 0; i < 16; i++)
		{
			destFile->Write(name, 22);		// Name
			destFile->Write_B_UINT16(1);	// Length
			destFile->Write_B_UINT16(0);	// Volume
			destFile->Write_B_UINT16(0);	// Loop start
			destFile->Write_B_UINT16(0);	// Loop length
		}

		// Copy the song length and orders tabel
		CopyData(file, destFile, 1 + 1 + 128);

		// Write pad
		destFile->Write_B_UINT32(0);

		if (file->IsEOF())
		{
			ShowError(IDS_MODC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Just copy the rest of the file
		CopyData(file, destFile, file->GetLength() - file->GetPosition());

		// Set the module type
		convInfo->modKind.LoadString(res, IDS_MODC_NAME_SFX13);
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	return (retVal);
}
