/******************************************************************************/
/* APlayer Scope Agent class.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PSettings.h"
#include "PSynchronize.h"

// APlayerKit headers
#include "APGlobalData.h"

// Agent headers
#include "ScopeAgent.h"
#include "ScopeWindow.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.00f



/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *scopeSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ScopeAgent::ScopeAgent(APGlobalData *global, PString fileName) : APAddOnAgent(global), infoLock(false)
{
	// Fill out the version variable
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	displayInfo.window = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ScopeAgent::~ScopeAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float ScopeAgent::GetVersion(void)
{
	return (AgentVersion);
}



/******************************************************************************/
/* GetDisplayInfo() will return a pointer to a display structure.             */
/*                                                                            */
/* Output: Is a pointer to a display structure.                               */
/******************************************************************************/
const APDisplayInfo *ScopeAgent::GetDisplayInfo(void)
{
	PString title;

	title.LoadString(res, IDS_SCOPE_TITLE);
	displayInfo.window = new ScopeWindow(this, res, title);
	displayInfo.openIt = scopeSettings->GetStringEntryValue("Window", "OpenWindow").CompareNoCase("Yes") == 0;

	return (&displayInfo);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 ScopeAgent::GetSupportFlags(int32 index)
{
	return (apaVisual);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString ScopeAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_SCOPE_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString ScopeAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_SCOPE_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* Run() will run one command in the agent.                                   */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*         "command" is the command to run.                                   */
/*         "args" is a pointer to a structure containing the arguments. The   */
/*         structure is different for each command.                           */
/*                                                                            */
/* Output: The result from the command.                                       */
/******************************************************************************/
ap_result ScopeAgent::Run(int32 index, uint32 command, void *args)
{
	switch (command)
	{
		// Show the output data in the window
		case APVA_MIXED_DATA:
		{
			MixedData((APAgent_MixedData *)args);
			return (AP_OK);
		}

		// The module has been ejected, so clean up our window
		case APVA_STOP_SHOWING:
		{
			StopShowing();
			return (AP_OK);
		}
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* WindowClosed() will clear the window pointer.                              */
/******************************************************************************/
void ScopeAgent::WindowClosed(void)
{
	infoLock.Lock();
	displayInfo.window = NULL;
	infoLock.Unlock();
}



/******************************************************************************/
/* MixedData() is called for every buffer with mixed data.                    */
/*                                                                            */
/* Input:  "mixedData" is a pointer to a structure holding the data           */
/*         information.                                                       */
/******************************************************************************/
void ScopeAgent::MixedData(APAgent_MixedData *mixedData)
{
	// Tell the window about the new data
	infoLock.Lock();

	if (displayInfo.window != NULL)
		((ScopeWindow *)displayInfo.window)->DrawWindow(mixedData->buffer, mixedData->length, mixedData->stereo);

	infoLock.Unlock();
}



/******************************************************************************/
/* StopShowing() is called when the module is ejected.                        */
/******************************************************************************/
void ScopeAgent::StopShowing(void)
{
	infoLock.Lock();

	if (displayInfo.window != NULL)
		((ScopeWindow *)displayInfo.window)->ClearWindow();

	infoLock.Unlock();
}
