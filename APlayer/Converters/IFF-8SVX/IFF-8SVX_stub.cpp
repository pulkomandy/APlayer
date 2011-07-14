/******************************************************************************/
/* IFF-8SVX Converter Stub Interface.                                         */
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

// Converter headers
#include "IFF-8SVXConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/
void FixSettings(PResource *res, PSettings *settings);



/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
PSettings *useSettings = NULL;
PSettings *saveSettings = NULL;



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
	type.LoadString(&res, IDS_IFF8SVX_MIME);
	ext.LoadString(&res, IDS_IFF8SVX_EXTENSION);
	shortDescr.LoadString(&res, IDS_IFF8SVX_SHORT_DESCRIPTION);
	longDescr.LoadString(&res, IDS_IFF8SVX_LONG_DESCRIPTION);

	// Install the mime type + icon
	global->fileTypes->RegisterFileType(&res, IDI_IFF8SVX_FILEICON, type, ext, longDescr, shortDescr);

	// Now allocate the settings
	useSettings = new PSettings();
	if (useSettings == NULL)
		throw PMemoryException();

	saveSettings = new PSettings();
	if (saveSettings == NULL)
	{
		delete useSettings;
		useSettings = NULL;
		throw PMemoryException();
	}

	// Load the settings into the memory
	try
	{
		saveSettings->LoadFile("IFF-8SVX.ini", "Polycode", "APlayer");
	}
	catch(PFileException e)
	{
		// Well, the file does probably not exists, so ignore the error
		;
	}

	// Copy the settings in the other instance
	useSettings->CloneSettings(saveSettings);

	// Fix the settings
	FixSettings(&res, useSettings);
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
	saveSettings->SaveFile("IFF-8SVX.ini", "Polycode", "APlayer");

	// Delete the settings objects again
	delete saveSettings;
	saveSettings = NULL;

	delete useSettings;
	useSettings = NULL;
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
	return (new IFF8SVXConverter(global, fileName));
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
void FixSettings(PResource *res, PSettings *settings)
{
	PString tempStr;

	if (!settings->EntryExist("General", "OutputFormat"))
	{
		tempStr.LoadString(res, IDS_IFF8SVX_FILE_PCM);
		settings->WriteStringEntryValue("General", "OutputFormat", tempStr);
	}

	// Set window positions
	if (!settings->EntryExist("Window", "WinX"))
		settings->WriteIntEntryValue("Window", "WinX", 50);

	if (!settings->EntryExist("Window", "WinY"))
		settings->WriteIntEntryValue("Window", "WinY", 50);
}
