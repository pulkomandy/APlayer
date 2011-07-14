/******************************************************************************/
/* APlayer add-on windows handler class.                                      */
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

// APlayerKit headers
#include "APList.h"

// Server headers
#include "APApplication.h"
#include "APWindowAddOnConfig.h"
#include "APAddOnWindows.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* APAddOnWindows class                                                       */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APAddOnWindows::APAddOnWindows(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnWindows::~APAddOnWindows(void)
{
}



/******************************************************************************/
/* FindWindowAddOns() will search all the add-ons in the list given to find   */
/*         which ones have a display and/or configuration window.             */
/*                                                                            */
/* Input : "infoList" is the list with the add-on information.                */
/******************************************************************************/
void APAddOnWindows::FindWindowAddOns(APMRSWList<AddOnInfo *> &infoList)
{
	int32 i, count;
	AddOnInfo *item;

	// Lock the list
	infoList.WaitToWrite();

	// Get the number of add-ons
	count = infoList.CountItems();

	// Search after view flags in the add-ons
	for (i = 0; i < count; i++)
	{
		// Get the information
		item = infoList.GetItem(i);

		// Initialize the add-on
		InitAddOnWindows(item);
	}

	// Unlock them again
	infoList.DoneWriting();
}



/******************************************************************************/
/* InitAddOnWindows() check the add-on to see if it has any windows. If so,   */
/*      it will update the info structure.                                    */
/*                                                                            */
/* Input:  "addOn" is the the add-on.                                         */
/******************************************************************************/
void APAddOnWindows::InitAddOnWindows(AddOnInfo *addOn)
{
	if (addOn->enabled)
	{
		APAddOnBase *addOnInstance;
		const APConfigInfo *cfgInfo;
		const APDisplayInfo *displayInfo;

		// Create an instance of the add-on
		addOnInstance = addOn->loader->CreateInstance();

		// See if there is a configuration window
		if ((cfgInfo = addOnInstance->GetConfigInfo()) != NULL)
		{
			addOn->settings = true;
			delete cfgInfo->view;
		}

		// See if there is a show window
		if ((displayInfo = addOnInstance->GetDisplayInfo()) != NULL)
		{
			addOn->display = true;
			delete displayInfo->window;
		}

		// Clean up again
		addOn->loader->DeleteInstance(addOnInstance);
	}
}



/******************************************************************************/
/* ShowAddOnSettings() is called when the user selects to show an add-on      */
/*      setting in the menu.                                                  */
/*                                                                            */
/* Input:  "info" is a pointer to the add-on information structure.           */
/*         "addOnInstance" is the instance of the add-on.                     */
/*         "config" is a pointer to the configuration information.            */
/******************************************************************************/
void APAddOnWindows::ShowAddOnSettings(AddOnInfo *info, APAddOnBase *addOnInstance, const APConfigInfo *config)
{
	int32 i, count;
	APOpenedConfigWindow *windowItem;
	PString title;
	bool found;

	// First check to see if the window is already opened
	configWindows.LockList();

	try
	{
		found = false;
		count = configWindows.CountItems();

		for (i = 0; i < count; i++)
		{
			windowItem = configWindows.GetItem(i);

			if (windowItem->name == info->addOnName)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			// We found the window in the list, but is it opened?
			if (windowItem->window != NULL)
			{
				// The window is already opened, so just activate it
				windowItem->window->Activate();

				// Delete the view given, because we don't need it anyway
				delete config->view;
			}
		}
		else
		{
			// Allocate new window item
			windowItem = new APOpenedConfigWindow;
			if (windowItem == NULL)
				throw PMemoryException();

			// Store the name of the add-on
			windowItem->name   = info->addOnName;
			windowItem->window = NULL;

			// Clone the original settings into the backup
			windowItem->firstBackupSettings.CloneSettings(config->settings);
			windowItem->firstBackupSettings.SetChangeFlag(false);

			// Add the item to the window list
			configWindows.AddTail(windowItem);
		}

		// Create the window title
		title.Format_S1(GetApp()->resource, IDS_TIT_ADDONSETTINGS, info->addOnName);

		// Create the window item and add it to the list
		windowItem->window = new APWindowAddOnConfig(title, info, addOnInstance, config, windowItem);
		if (windowItem->window == NULL)
			throw PMemoryException();

		// Show the window for the user
		windowItem->window->Show();
	}
	catch(...)
	{
		configWindows.UnlockList();
		throw;
	}

	// Unlock the list
	configWindows.UnlockList();
}



/******************************************************************************/
/* ShowAddOnDisplayWindow() is called when the user selects to show an add-on */
/*      display window in the menu.                                           */
/*                                                                            */
/* Input:  "info" is a pointer to the add-on information structure.           */
/*         "addOnInstance" is the instance of the add-on.                     */
/*         "display" is a pointer to the display information.                 */
/******************************************************************************/
void APAddOnWindows::ShowAddOnDisplayWindow(AddOnInfo *info, APAddOnBase *addOnInstance, const APDisplayInfo *display)
{
	int32 i, count;
	APOpenedDisplayWindow *windowItem;
	PString title;
	bool found;

	// First check to see if the window is already opened
	displayWindows.LockList();

	try
	{
		found = false;
		count = displayWindows.CountItems();

		for (i = 0; i < count; i++)
		{
			windowItem = displayWindows.GetItem(i);

			if (windowItem->name == info->addOnName)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			// We found the window in the list, but is it opened?
			if (windowItem->window != NULL)
			{
				// The window is already opened, so just activate it
				windowItem->window->Activate();

				// Delete the window given, because we don't need it anyway
				delete display->window;
			}
		}
		else
		{
			// Allocate new window item
			windowItem = new APOpenedDisplayWindow;
			if (windowItem == NULL)
				throw PMemoryException();

			// Store the name of the add-on
			windowItem->name   = info->addOnName;
			windowItem->window = display->window;
			windowItem->filter = new APAddOnWindowFilter(this);
			if (windowItem->filter == NULL)
			{
				delete windowItem;
				throw PMemoryException();
			}

			// Add the filter to the window
			windowItem->window->AddCommonFilter(windowItem->filter);

			// Show the window
			windowItem->window->Show();

			// Add the item to the window list
			displayWindows.AddTail(windowItem);
		}
	}
	catch(...)
	{
		displayWindows.UnlockList();
		throw;
	}

	// Unlock the list
	displayWindows.UnlockList();
}



/******************************************************************************/
/* CloseAddOnWindows() will close all the windows the add-on given has open.  */
/*                                                                            */
/* Input:  "addOn" is the the add-on.                                         */
/******************************************************************************/
void APAddOnWindows::CloseAddOnWindows(AddOnInfo *addOn)
{
	APOpenedConfigWindow *configWindowItem;
	APOpenedDisplayWindow *displayWindowItem;
	int32 i, count;

	// First take the config windows
	//
	// Lock the list
	configWindows.LockList();

	// Try to find the add-on in the window list
	count = configWindows.CountItems();
	for (i = 0; i < count; i++)
	{
		configWindowItem = configWindows.GetItem(i);

		if (configWindowItem->name == addOn->addOnName)
		{
			// Found the window
			//
			// Remove the item from the list
			configWindows.RemoveItem(i);

			// Close the window
			if (configWindowItem->window != NULL)
			{
				configWindowItem->window->Lock();
				configWindowItem->window->Quit();
			}

			delete configWindowItem;
			break;
		}
	}

	// Unlock the list again
	configWindows.UnlockList();

	// Now take the display windows
	//
	// Lock the list
	displayWindows.LockList();

	// Try to find the add-on in the window list
	count = displayWindows.CountItems();
	for (i = 0; i < count; i++)
	{
		displayWindowItem = displayWindows.GetItem(i);

		if (displayWindowItem->name == addOn->addOnName)
		{
			// Found the window
			//
			// Remove the item from the list
			displayWindows.RemoveItem(i);

			// Close the window
			if (displayWindowItem->window != NULL)
			{
				displayWindowItem->window->Lock();
				displayWindowItem->window->RemoveCommonFilter(displayWindowItem->filter);
				displayWindowItem->window->Quit();
				delete displayWindowItem->filter;
			}

			delete displayWindowItem;
			break;
		}
	}

	// Unlock the list again
	displayWindows.UnlockList();
}



/******************************************************************************/
/* CloseWindows() will close all the windows opened from the server.          */
/******************************************************************************/
void APAddOnWindows::CloseWindows(void)
{
	APOpenedConfigWindow *configWindowItem;
	APOpenedDisplayWindow *displayWindowItem;
	int32 i, count;

	// Take the config list first
	//
	// Lock the list
	configWindows.LockList();

	// Tell all the config windows to quit
	count = configWindows.CountItems();
	for (i = 0; i < count; i++)
	{
		configWindowItem = configWindows.GetAndRemoveItem(0);
		if (configWindowItem->window != NULL)
		{
			configWindowItem->window->Lock();
			configWindowItem->window->Quit();
		}

		delete configWindowItem;
	}

	// Unlock the list again
	configWindows.UnlockList();

	// Then take the display list
	//
	// Lock the list
	displayWindows.LockList();

	// Tell all the config windows to quit
	count = displayWindows.CountItems();
	for (i = 0; i < count; i++)
	{
		displayWindowItem = displayWindows.GetAndRemoveItem(0);
		if (displayWindowItem->window != NULL)
		{
			displayWindowItem->window->Lock();
			displayWindowItem->window->RemoveCommonFilter(displayWindowItem->filter);
			displayWindowItem->window->Quit();
			delete displayWindowItem->filter;
		}

		delete displayWindowItem;
	}

	// Unlock the list again
	displayWindows.UnlockList();
}





/******************************************************************************/
/* APAddOnWindowFilter class                                                  */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APAddOnWindowFilter::APAddOnWindowFilter(APAddOnWindows *windowObject) : BMessageFilter(B_QUIT_REQUESTED)
{
	// Remember argumentes
	winObj = windowObject;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnWindowFilter::~APAddOnWindowFilter(void)
{
}



/******************************************************************************/
/* Filter() is called when a B_QUIT_REQUESTED is sent to a display window.    */
/******************************************************************************/
filter_result APAddOnWindowFilter::Filter(BMessage *message, BHandler **target)
{
	APOpenedDisplayWindow *displayWindowItem;
	int32 i, count;

	// The window is now closed, so find it in the list and
	// remove the item
	//
	// Lock the list
	winObj->displayWindows.LockList();

	// Tell all the config windows to quit
	count = winObj->displayWindows.CountItems();
	for (i = 0; i < count; i++)
	{
		displayWindowItem = winObj->displayWindows.GetItem(i);
		if (displayWindowItem->filter == this)
		{
			winObj->displayWindows.RemoveItem(i);
			delete displayWindowItem;
			i--;
			count--;
		}
	}

	// Unlock the list again
	winObj->displayWindows.UnlockList();

	return (B_DISPATCH_MESSAGE);
}
