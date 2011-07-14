/******************************************************************************/
/* APlayer SpinSquare Agent class.                                            */
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
#include "SpinSquareAgent.h"
#include "SpinSquareWindow.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.00f



/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *spinSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SpinSquareAgent::SpinSquareAgent(APGlobalData *global, PString fileName) : APAddOnAgent(global), infoLock(false)
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
SpinSquareAgent::~SpinSquareAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float SpinSquareAgent::GetVersion(void)
{
	return (AgentVersion);
}



/******************************************************************************/
/* GetDisplayInfo() will return a pointer to a display structure.             */
/*                                                                            */
/* Output: Is a pointer to a display structure.                               */
/******************************************************************************/
const APDisplayInfo *SpinSquareAgent::GetDisplayInfo(void)
{
	PString title;

	title.LoadString(res, IDS_SPIN_TITLE);
	displayInfo.window = new SpinSquareWindow(this, res, title);
	displayInfo.openIt = spinSettings->GetStringEntryValue("Window", "OpenWindow").CompareNoCase("Yes") == 0;

	return (&displayInfo);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 SpinSquareAgent::GetSupportFlags(int32 index)
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
PString SpinSquareAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_SPIN_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString SpinSquareAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_SPIN_DESCRIPTION);
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
ap_result SpinSquareAgent::Run(int32 index, uint32 command, void *args)
{
	switch (command)
	{
		// Something has changed on a channel
		case APVA_CHANNEL_CHANGE:
		{
			ChannelChange((APAgent_ChannelChange *)args);
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
void SpinSquareAgent::WindowClosed(void)
{
	infoLock.Lock();
	displayInfo.window = NULL;
	infoLock.Unlock();
}



/******************************************************************************/
/* ChannelChange() is called when at least one channel has changed its        */
/*      information.                                                          */
/*                                                                            */
/* Input:  "channelInfo" is a pointer to a structure holding the channel      */
/*         information.                                                       */
/******************************************************************************/
void SpinSquareAgent::ChannelChange(APAgent_ChannelChange *channelInfo)
{
	// Tell the window about the new data
	infoLock.Lock();

	if (displayInfo.window != NULL)
		((SpinSquareWindow *)displayInfo.window)->DrawWindow(channelInfo);

	infoLock.Unlock();
}



/******************************************************************************/
/* StopShowing() is called when the module is ejected.                        */
/******************************************************************************/
void SpinSquareAgent::StopShowing(void)
{
	infoLock.Lock();

	if (displayInfo.window != NULL)
		((SpinSquareWindow *)displayInfo.window)->ClearWindow();

	infoLock.Unlock();
}
