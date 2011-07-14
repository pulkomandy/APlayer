/******************************************************************************/
/* SID Player Stub Interface.                                                 */
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
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PSettings.h"
#include "PSynchronize.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APGlobalData.h"
#include "APAddOns.h"

// Player headers
#include "SIDView.h"
#include "SIDPlayer.h"
#include "SIDStil.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/
void FixSettings(void);



/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
PSettings *sidSettings = NULL;
PMutex *stilLock = NULL;
SIDStil *sidStil = NULL;



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

	// Get the SID file type strings
	type.LoadString(&res, IDS_SID_MIME);
	ext.LoadString(&res, IDS_SID_EXTENSION);
	shortDescr.LoadString(&res, IDS_SID_SHORT_DESCRIPTION);
	longDescr.LoadString(&res, IDS_SID_LONG_DESCRIPTION);

	// Install the mime type + icon
	global->fileTypes->RegisterFileType(&res, IDI_SID_FILEICON, type, ext, longDescr, shortDescr);

	// Now allocate the settings
	sidSettings = new PSettings();
	if (sidSettings == NULL)
		throw PMemoryException();

	// Load the settings into the memory
	try
	{
		sidSettings->LoadFile("SidPlay.ini", "Polycode", "APlayer");
	}
	catch(PFileException e)
	{
		// Well, the file does probably not exists, so ignore the error
		;
	}

	// Fix the settings
	FixSettings();

	// Allocate the STIL objects
	stilLock = new PMutex("SID STIL Lock", false);
	if (stilLock == NULL)
		throw PMemoryException();

	sidStil = new SIDStil();
	if (sidStil == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Add-On function: Unload() is called just before the add-on is freed from   */
/*      the memory.                                                           */
/*                                                                            */
/* Input:  "global" is a pointer to the global data class.                    */
/******************************************************************************/
void Unload(APGlobalData *global)
{
	// Delete the STIL
	delete stilLock;
	stilLock = NULL;

	delete sidStil;
	sidStil = NULL;

	// Delete the settings object again
	delete sidSettings;
	sidSettings = NULL;
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
	return (new SIDPlayer(global, fileName));
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
	// Set default "General settings"
	if (!sidSettings->EntryExist("General", "Filter"))
		sidSettings->WriteStringEntryValue("General", "Filter", "Yes");

	if (!sidSettings->EntryExist("General", "MOS8580"))
		sidSettings->WriteStringEntryValue("General", "MOS8580", "No");

	if (!sidSettings->EntryExist("General", "ForceSongSpeed"))
		sidSettings->WriteStringEntryValue("General", "ForceSongSpeed", "No");

	// Set default "MPU memory mode"
	if (!sidSettings->EntryExist("MPU", "Memory"))
		sidSettings->WriteIntEntryValue("MPU", "Memory", RADIO_MEMORY_TRANSPARENT);

	// Set default "C64 clock speed"
	if (!sidSettings->EntryExist("Speed", "ClockSpeed"))
		sidSettings->WriteIntEntryValue("Speed", "ClockSpeed", RADIO_SPEED_PAL);

	// Set default "Filter adjustment"
	if (!sidSettings->EntryExist("Filter", "FilterFs"))
		sidSettings->WriteIntEntryValue("Filter", "FilterFs", FILTER_PAR1_MIN - SIDEMU_DEFAULTFILTERFS + FILTER_PAR1_MAX);

	if (!sidSettings->EntryExist("Filter", "FilterFm"))
		sidSettings->WriteIntEntryValue("Filter", "FilterFm", SIDEMU_DEFAULTFILTERFM);

	if (!sidSettings->EntryExist("Filter", "FilterFt"))
		sidSettings->WriteIntEntryValue("Filter", "FilterFt", FILTER_PAR3_MIN * 100 - (SIDEMU_DEFAULTFILTERFT * 100) + FILTER_PAR3_MAX);

	// Set default "Panning"
	if (!sidSettings->EntryExist("Panning", "Channel1"))
		sidSettings->WriteIntEntryValue("Panning", "Channel1", 64);

	if (!sidSettings->EntryExist("Panning", "Channel2"))
		sidSettings->WriteIntEntryValue("Panning", "Channel2", 128);

	if (!sidSettings->EntryExist("Panning", "Channel3"))
		sidSettings->WriteIntEntryValue("Panning", "Channel3", 192);

	if (!sidSettings->EntryExist("Panning", "Channel4"))
		sidSettings->WriteIntEntryValue("Panning", "Channel4", 128);

	// Set default "Misc"
	if (!sidSettings->EntryExist("Misc", "DigiScan"))
		sidSettings->WriteIntEntryValue("Misc", "DigiScan", 10);

	if (!sidSettings->EntryExist("Misc", "StilPath"))
		sidSettings->WriteStringEntryValue("Misc", "StilPath", "");
}
