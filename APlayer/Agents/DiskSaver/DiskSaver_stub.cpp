/******************************************************************************/
/* DiskSaver Agent Stub Interface.                                            */
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
#include "PSettings.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APGlobalData.h"
#include "APAddOns.h"

// Agent headers
#include "DiskSaverAgent.h"


/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
PSettings *diskSaverSettings = NULL;



/******************************************************************************/
/* Add-On function: Load() is called when the add-on is loaded into the       */
/*      memory.                                                               */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/*         "fileName" is the file name for this add-on.                       */
/******************************************************************************/
void Load(APGlobalData *global, PString fileName)
{
}



/******************************************************************************/
/* Add-On function: Unload() is called just before the add-on is freed from   */
/*      the memory.                                                           */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/******************************************************************************/
void Unload(APGlobalData *global)
{
	// Delete the settings object again
	delete diskSaverSettings;
	diskSaverSettings = NULL;
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
	return (new DiskSaverAgent(global, fileName));
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
