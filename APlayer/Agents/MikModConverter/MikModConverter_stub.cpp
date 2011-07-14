/******************************************************************************/
/* MikModConverter Agent Stub Interface.                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_APLAYER_ADDON_

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APGlobalData.h"
#include "APAddOns.h"

// Agent headers
#include "MikAgent.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Add-On function: Load() is called when the add-on is loaded into the       */
/*      memory.                                                               */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/*         "fileName" is the file name for this add-on.                       */
/******************************************************************************/
void Load(APGlobalData *global, PString fileName)
{
	PResource res(fileName);
	PString type, ext;
	PString longDescr, shortDescr;

	// Get and install the 669 file type
	type.LoadString(&res, IDS_MIKC_MIME_669);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_669);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_669);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_669);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_669, type, ext, longDescr, shortDescr);

	// Get and install the AMF file type
	type.LoadString(&res, IDS_MIKC_MIME_AMF);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_AMF);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_AMF);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_AMF);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_AMF, type, ext, longDescr, shortDescr);

	// Get and install the DSM file type
	type.LoadString(&res, IDS_MIKC_MIME_DSM);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_DSM);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_DSM);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_DSM);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_DSM, type, ext, longDescr, shortDescr);

	// Get and install the FAR file type
	type.LoadString(&res, IDS_MIKC_MIME_FAR);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_FAR);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_FAR);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_FAR);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_FAR, type, ext, longDescr, shortDescr);

	// Get and install the GDM file type
	type.LoadString(&res, IDS_MIKC_MIME_GDM);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_GDM);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_GDM);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_GDM);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_GDM, type, ext, longDescr, shortDescr);

	// Get and install the IMF file type
	type.LoadString(&res, IDS_MIKC_MIME_IMF);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_IMF);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_IMF);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_IMF);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_IMF, type, ext, longDescr, shortDescr);

	// Get and install the IT file type
	type.LoadString(&res, IDS_MIKC_MIME_IT);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_IT);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_IT);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_IT);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_IT, type, ext, longDescr, shortDescr);

	// Get and install the S3M file type
	type.LoadString(&res, IDS_MIKC_MIME_S3M);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_S3M);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_S3M);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_S3M);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_S3M, type, ext, longDescr, shortDescr);

	// Get and install the STM file type
	type.LoadString(&res, IDS_MIKC_MIME_STM);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_STM);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_STM);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_STM);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_STM, type, ext, longDescr, shortDescr);

	// Get and install the STX file type
	type.LoadString(&res, IDS_MIKC_MIME_STX);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_STX);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_STX);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_STX);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_STX, type, ext, longDescr, shortDescr);

	// Get and install the ULT file type
	type.LoadString(&res, IDS_MIKC_MIME_ULT);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_ULT);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_ULT);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_ULT);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_ULT, type, ext, longDescr, shortDescr);

	// Get and install the XM file type
	type.LoadString(&res, IDS_MIKC_MIME_XM);
	ext.LoadString(&res, IDS_MIKC_EXTENSION_XM);
	longDescr.LoadString(&res, IDS_MIKC_LONG_DESCRIPTION_XM);
	shortDescr.LoadString(&res, IDS_MIKC_SHORT_DESCRIPTION_XM);
	global->fileTypes->RegisterFileType(&res, IDI_MIKC_FILEICON_XM, type, ext, longDescr, shortDescr);
}



/******************************************************************************/
/* Add-On function: Unload() is called just before the add-on is freed from   */
/*      the memory.                                                           */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/******************************************************************************/
void Unload(APGlobalData *global)
{
}



/******************************************************************************/
/* Add-On function: AllocateInstance() will allocate a new add-on instance    */
/*      and return the pointer to it.                                         */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/*         "fileName" is the file name for this add-on.                       */
/*                                                                            */
/* Output: A new add-on instance allocated with new.                          */
/******************************************************************************/
APAddOnBase *AllocateInstance(APGlobalData *global, PString fileName)
{
	return (new MikAgent(global, fileName));
}



/******************************************************************************/
/* Add-On function: DeleteInstance() will delete the add-on instance given.   */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/*         "addOn" is a pointer to the add-on to delete.                      */
/******************************************************************************/
void DeleteInstance(APGlobalData *global, APAddOnBase *addOn)
{
	delete addOn;
}
