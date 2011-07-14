/******************************************************************************/
/* SpinSquare Agent Stub Interface.                                           */
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
#include "SpinSquareAgent.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/
void FixSettings(void);



/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
PSettings *spinSettings = NULL;



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
	spinSettings = new PSettings();
	if (spinSettings == NULL)
		throw PMemoryException();

	// Load the settings into the memory
	try
	{
		spinSettings->LoadFile("SpinSquare.ini", "Polycode", "APlayer");
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
	spinSettings->SaveFile("SpinSquare.ini", "Polycode", "APlayer");

	// Delete the settings object again
	delete spinSettings;
	spinSettings = NULL;
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
	return (new SpinSquareAgent(global, fileName));
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
	// Set window positions and size
	if (!spinSettings->EntryExist("Window", "WinX"))
		spinSettings->WriteIntEntryValue("Window", "WinX", 50);

	if (!spinSettings->EntryExist("Window", "WinY"))
		spinSettings->WriteIntEntryValue("Window", "WinY", 200);

//XX	if (!spinSettings->EntryExist("Window", "WinW"))
/*		spinSettings->WriteIntEntryValue("Window", "WinW", 1);

	if (!spinSettings->EntryExist("Window", "WinH"))
		spinSettings->WriteIntEntryValue("Window", "WinH", 1);
*/
	if (!spinSettings->EntryExist("Window", "OpenWindow"))
		spinSettings->WriteStringEntryValue("Window", "OpenWindow", "No");
}
