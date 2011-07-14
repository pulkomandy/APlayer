/******************************************************************************/
/* ProWizard FC-M class.                                                      */
/*                                                                            */
/* FC-M Packer format.                                                        */
/* Created by ?? (19xx)                                                       */
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
#include "PBinary.h"

// Agent headers
#include "ProWizard.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* CheckModule() will be check the module to see if it's a known module.      */
/*                                                                            */
/* Input:  "module" is a reference to where the packed module is stored.      */
/*                                                                            */
/* Output: Is the unpacked module size or 0 if not recognized.                */
/******************************************************************************/
uint32 PROZ_FC_M::CheckModule(const PBinary &module)
{
	const uint8 *mod, *tempPoi;
	uint32 length;
	uint32 temp;
	uint8 posSize;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check the mark
	if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0])) != 'FC-M')
		return (0);

	// Get length of module
	length = module.GetLength();

	// Find sample informations
	tempPoi = FindPart(mod, length, 'INST');
	if (tempPoi == NULL)
		return (0);

	// Calculate sample size
	sampSize = 0;
	for (i = 0; i < 31; i++)
	{
		// Get sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&tempPoi[i * 8]));
		sampSize += (temp * 2);
	}

	// Find the position table size
	tempPoi = FindPart(mod, length, 'LONG');
	if (tempPoi == NULL)
		return (0);

	posSize = *tempPoi;

	// Find position table
	tempPoi = FindPart(mod, length, 'PATT');
	if (tempPoi == NULL)
		return (0);	

	// Find heighest pattern number
	GetPGP(tempPoi, posSize);

	// Calculate the total size of the PTK module
	calcSize = pattNum * 1024 + sampSize + 1084;

	return (calcSize);
}



/******************************************************************************/
/* ConvertModule() will convert the module to ProTracker format.              */
/*                                                                            */
/* Input:  "module" is a reference to the packed module.                      */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result PROZ_FC_M::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[128];
	uint32 length;
	uint16 i;
	const uint8 *namePtr, *instPtr, *longPtr, *pattPtr, *songPtr, *sampPtr;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Get the module length
	length = module.GetLength();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Start to find all the pointers to the chunks
	namePtr = FindPart(mod, length, 'NAME');
	if (namePtr == NULL)
		return (AP_ERROR);

	instPtr = FindPart(mod, length, 'INST');
	if (instPtr == NULL)
		return (AP_ERROR);

	longPtr = FindPart(mod, length, 'LONG');
	if (longPtr == NULL)
		return (AP_ERROR);

	pattPtr = FindPart(mod, length, 'PATT');
	if (pattPtr == NULL)
		return (AP_ERROR);

	songPtr = FindPart(mod, length, 'SONG');
	if (songPtr == NULL)
		return (AP_ERROR);

	sampPtr = FindPart(mod, length, 'SAMP');
	if (sampPtr == NULL)
		return (AP_ERROR);

	// Write the module name
	destFile->Write(namePtr, 20);

	// Write the sample informations
	for (i = 0; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Other informations
		destFile->Write(&instPtr[i * 8], 8);
	}

	// Write song length + NTK byte
	destFile->Write(longPtr, 2);

	// Write position table
	destFile->Write(pattPtr, *longPtr);
	destFile->Write(zeroBuf, 128 - *longPtr);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Write pattern data
	destFile->Write(songPtr, pattNum * 1024);

	// Write sample data
	destFile->Write(sampPtr, sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_FC_M::GetModuleType(void)
{
	return (IDS_PROZ_TYPE_FC_M);
}



/******************************************************************************/
/* FindPart() will search the loaded module after a specific chunk.           */
/*                                                                            */
/* Input:  "start" is a pointer to the start of the module.                   */
/*         "length" is the length of the module.                              */
/*         "chunk" is the chunk to search after.                              */
/*                                                                            */
/* Output: A pointer to the chunk data or NULL for not found.                 */
/******************************************************************************/
const uint8 *PROZ_FC_M::FindPart(const uint8 *start, uint32 length, uint32 chunk)
{
	uint32 count = 0;
	bool found = false;

	// Convert the chunk to big endian to make the search faster
	chunk = P_HOST_TO_BENDIAN_INT32(chunk);

	while (count < (length - 3))
	{
		if (*((uint32 *)&start[count]) == chunk)
		{
			found = true;
			break;
		}

		count += 2;
	}

	if (found)
		return (&start[count + 4]);

	return (NULL);
}
