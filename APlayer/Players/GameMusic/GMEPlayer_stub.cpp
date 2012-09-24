/* GMA Player stub interface
 * Copyright 2012, Adrien Destugues <pulkomandy@gmail.com>
 * This file is distributed under the terms of the MIT Licence
 */

#define _BUILDING_APLAYER_ADDON_

#include "PString.h"
#include "PResource.h"

#include "APGlobalData.h"

#include "ResourceIDs.h"
#include "GMEPlayer.h"

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
	PString shortDescr, longDescr;

	// Get the file type strings
	type.LoadString(&res, IDS_GME_MIME);
	ext.LoadString(&res, IDS_GME_EXTENSION);
	shortDescr.LoadString(&res, IDS_GME_SHORT_DESCRIPTION);
	longDescr.LoadString(&res, IDS_GME_LONG_DESCRIPTION);

	// Install the mime type + icon
	global->fileTypes->RegisterFileType(&res, IDI_GME_FILEICON, type, ext, longDescr, shortDescr);
}



/******************************************************************************/
/* Add-On function: Unload() is called just before the add-on is freed from   */
/*      the memory.                                                           */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/******************************************************************************/
void Unload(APGlobalData* /*global*/)
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
	return new GMEPlayer(global, fileName);
}



/******************************************************************************/
/* Add-On function: DeleteInstance() will delete the add-on instance given.   */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/*         "addOn" is a pointer to the add-on to delete.                      */
/******************************************************************************/
void DeleteInstance(APGlobalData * /*global*/, APAddOnBase *addOn)
{
	delete addOn;
}
