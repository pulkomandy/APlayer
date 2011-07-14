/******************************************************************************/
/* File Scanner Interface.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PDebug.h"
#include "PException.h"
#include "PSynchronize.h"
#include "PSystem.h"
#include "PTime.h"

// Client headers
#include "MainWindowSystem.h"
#include "APFileScanner.h"


/******************************************************************************/
/* Messages                                                                   */
/******************************************************************************/
#define APMSG_SCAN				'_scn'



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APFileScanner::APFileScanner(MainWindowSystem *system) : BLooper("MainWindowSystem: File Scanner", B_LOW_PRIORITY)
{
	// Initialize member variables
	windowSystem = system;
	stopEvent    = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APFileScanner::~APFileScanner(void)
{
	// Delete the stop event
	delete stopEvent;
}



/******************************************************************************/
/* Start() will initialize and start the file scanner.                        */
/******************************************************************************/
void APFileScanner::Start(void)
{
	// Create the stop event
	stopEvent = new PEvent("File Scanner Stop Event", true, false);
	if (stopEvent == NULL)
		throw PMemoryException();

	// Start the looper
	Run();
}



/******************************************************************************/
/* Stop() will stop the looper and delete itself.                             */
/******************************************************************************/
void APFileScanner::Stop(void)
{
	BMessenger messenger(NULL, this);
	BMessage message(B_QUIT_REQUESTED);

	// Set the stop event, so the scanner will stop parsing
	// any messages and ignore the rest
	stopEvent->SetEvent();

	// Tell the BLooper to quit and delete itself
	// This call is synchronous, so we are sure the
	// object has been deleted when the function
	// call returns
	messenger.SendMessage(&message, &message);
}



/******************************************************************************/
/* ScanItems() will start to scan the items and the indexes given.            */
/*                                                                            */
/* Input:  "startIndex" is the start index to start the scanning.             */
/*         "count" is the number of items to scan.                            */
/******************************************************************************/
void APFileScanner::ScanItems(int32 startIndex, int32 count)
{
	// Do only send the message if the option is enabled
	if (windowSystem->useSettings->GetStringEntryValue("Options", "ScanFiles").CompareNoCase("Yes") == 0)
	{
		BMessage msg(APMSG_SCAN);

		msg.AddInt32("startIndex", startIndex);
		msg.AddInt32("count", count);
		PostMessage(&msg);
	}
}



/******************************************************************************/
/* MessageReceived() is called for each message sent to the looper.           */
/*                                                                            */
/* Input:  "message" is a pointer to the message that has been sent.          */
/******************************************************************************/
void APFileScanner::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
		//
		// Start a new scanning
		//
		case APMSG_SCAN:
		{
			int32 startIndex, count;

			// Get the arguments
			if (message->FindInt32("startIndex", &startIndex) == B_OK)
			{
				if (message->FindInt32("count", &count) == B_OK)
				{
					// Scan the files
					ScanFiles(startIndex, count);
				}
			}
			break;
		}

		//
		// Unknown message. Call the base class
		//
		default:
		{
			BLooper::MessageReceived(message);
			break;
		}
	}
}



/******************************************************************************/
/* ScanFiles() will do the scanning of the files.                             */
/*                                                                            */
/* Input:  "index" is the start index to start the scanning.                  */
/*         "count" is the number of items to scan.                            */
/******************************************************************************/
void APFileScanner::ScanFiles(int32 index, int32 count)
{
	APWindowMainListItem *workItem;
	PString fileName;
	PTimeSpan totalTime;
	bool haveTime, setTime;

	for (; count > 0; index++, count--)
	{
		// Lock the window
		if (!LockWindow())
			return;

		// Get the item to work on and remember the file name
		workItem = windowSystem->mainWin->GetListItem(index);
		if (workItem == NULL)
		{
			windowSystem->mainWin->Unlock();
			continue;
		}

		fileName = workItem->GetFileName();
		haveTime = workItem->HaveTime();
		setTime  = workItem->GetItemType() == APWindowMainListItem::apNormal;

		// Unlock again
		windowSystem->mainWin->Unlock();

		// If the item already have a time, skip it
		if (haveTime)
			continue;

		// Now check the file to see if it already have a time attribute
		totalTime = GetAttrTime(fileName);

		// Did we get the total time?
		if (totalTime.GetTotalMilliSeconds() == 0)
		{
			// Nup, try to load the file and let the player returns the total time
			totalTime = GetPlayerTime(fileName, setTime);
		}

		// Update the list item
		//
		// Lock the window
		if (!LockWindow())
			return;

		// Get the item to work on
		workItem = windowSystem->mainWin->GetListItem(index);
		if (workItem == NULL)
		{
			windowSystem->mainWin->Unlock();
			continue;
		}

		// Is the item we got the one we got a total time on?
		if (workItem->GetFileName() == fileName)
		{
			// Yip, set the total time
			windowSystem->mainWin->SetTimeOnItem(workItem, totalTime);
		}

		// Unlock the window
		windowSystem->mainWin->Unlock();
	}
}



/******************************************************************************/
/* LockWindow() will try to lock the window.                                  */
/*                                                                            */
/* Output: True if the window is locked, false if you have to stop.           */
/******************************************************************************/
bool APFileScanner::LockWindow(void)
{
	do
	{
		// Check the stop event
		if (stopEvent->Lock(0) == pSyncOk)
			return (false);
	}
	while (windowSystem->mainWin->LockWithTimeout(1000000) != B_OK);

	return (true);
}



/******************************************************************************/
/* GetAttrTime() will try to get the total time from the file attribute.      */
/*                                                                            */
/* Input:  "fileName" is the file name to check.                              */
/*                                                                            */
/* Output: The attribute time or 0 if not found.                              */
/******************************************************************************/
PTimeSpan APFileScanner::GetAttrTime(PString fileName)
{
	BNode node;
	int32 minutes, seconds, charIndex;
	ssize_t readBytes;
	char buffer[20];
	PString tempStr;
	char *nameStr;
	PTimeSpan time;

	if (node.SetTo((nameStr = fileName.GetString())) == B_OK)
	{
		// Read the attribute and parse it
		readBytes = node.ReadAttr("Audio:Length", B_STRING_TYPE, 0, buffer, 20);
		if (readBytes > 0)
		{
			buffer[19] = 0x00;
			tempStr = buffer;
			tempStr.TrimLeft();

			// Split the string into hours and minutes
			charIndex = tempStr.Find(':');
			if (charIndex != -1)
			{
				minutes = tempStr.GetNumber();
				seconds = tempStr.GetNumber(charIndex + 1);

				time.SetTimeSpan(0, 0, minutes, seconds);
			}
		}
	}

	fileName.FreeBuffer(nameStr);

	return (time);
}



/******************************************************************************/
/* GetPlayerTime() will try to get the total time from the player.            */
/*                                                                            */
/* Input:  "fileName" is the file name to check.                              */
/*         "setTime" indicates if you want to write back the time as          */
/*         attributes in the file.                                            */
/*                                                                            */
/* Output: The attribute time or 0 if not found.                              */
/******************************************************************************/
PTimeSpan APFileScanner::GetPlayerTime(PString fileName, bool setTime)
{
	PTimeSpan time;

	time = windowSystem->loader->GetTotalTimeFromFile(fileName);

	// Set the total time attribute
	if (setTime)
		windowSystem->loader->SetTotalTimeOnFile(fileName, time);

	return (time);
}
