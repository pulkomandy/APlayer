/******************************************************************************/
/* Scope Agent Stub Interface.                                                */
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
#include "PSettings.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APGlobalData.h"
#include "APAddOns.h"

// Agent headers
#include "ScopeAgent.h"
#include "ScopeView.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/
void FixSettings(void);



/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
PSettings *scopeSettings = NULL;



/******************************************************************************/
/* Add-On function: Load() is called when the add-on is loaded into the       */
/*      memory.                                                               */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/*         "fileName" is the file name for this add-on.                       */
/******************************************************************************/
void Load(APGlobalData *global, PString fileName)
{
	// Now allocate the settings
	scopeSettings = new PSettings();
	if (scopeSettings == NULL)
		throw PMemoryException();

	// Load the settings into the memory
	try
	{
		scopeSettings->LoadFile("Scope.ini", "Polycode", "APlayer");
	}
	catch(PFileException e)
	{
		// Well, the file does probably not exists, so ignore the error
		;
	}

	// Fix the settings
	FixSettings();
}



/******************************************************************************/
/* Add-On function: Unload() is called just before the add-on is freed from   */
/*      the memory.                                                           */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/******************************************************************************/
void Unload(APGlobalData *global)
{
	// Save the settings if needed
	scopeSettings->SaveFile("Scope.ini", "Polycode", "APlayer");

	// Delete the settings object again
	delete scopeSettings;
	scopeSettings = NULL;
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
	return (new ScopeAgent(global, fileName));
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



/******************************************************************************/
/* FixSettings() puts all the default setting values into the settings object.*/
/******************************************************************************/
void FixSettings(void)
{
	// General settings
	if (!scopeSettings->EntryExist("General", "Type"))
		scopeSettings->WriteIntEntryValue("General", "Type", ScopeView::eFilled);

	// Set window positions and size
	if (!scopeSettings->EntryExist("Window", "WinX"))
		scopeSettings->WriteIntEntryValue("Window", "WinX", 50);

	if (!scopeSettings->EntryExist("Window", "WinY"))
		scopeSettings->WriteIntEntryValue("Window", "WinY", 200);

	if (!scopeSettings->EntryExist("Window", "WinW"))
		scopeSettings->WriteIntEntryValue("Window", "WinW", 1);

	if (!scopeSettings->EntryExist("Window", "WinH"))
		scopeSettings->WriteIntEntryValue("Window", "WinH", 1);

	if (!scopeSettings->EntryExist("Window", "OpenWindow"))
		scopeSettings->WriteStringEntryValue("Window", "OpenWindow", "No");
}
