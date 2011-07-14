/******************************************************************************/
/* APlayer Sample Info window class.                                          */
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

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"
#include "APWindowSampleInfo.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowSampleInfo::APWindowSampleInfo(APGlobalData *glob, MainWindowSystem *system, BRect frame, PString title, int32 active) : BWindow(frame, NULL, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS), sampleLock(false)
{
	BRect rect;
	BPoint winSize;
	PString label;
	char *titleStr;
	font_height fh;

	// Remember the arguments
	global       = glob;
	windowSystem = system;

	// Remember the resource
	res = windowSystem->res;

	// Initialize member variables
	shutDown = false;

	// Set the window title
	SetTitle((titleStr = title.GetString()));
	title.FreeBuffer(titleStr);

	// Create the keyboard filter
	keyFilter = new APKeyFilter(this);
	keyFilter->AddFilterKey(B_ESCAPE, 0);

	// Create background view
	rect    = Bounds();
	topView = new BView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	// Change the color to light grey
	topView->SetViewColor(BeBackgroundGrey);

	// Add view to the window
	AddChild(topView);

	// Get other needed information
	topView->GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Check to see if the given size is lesser than the minimum size
	winSize = CalcMinSize();
	if ((frame.Width() < winSize.x) || (frame.Height() < winSize.y))
		ResizeTo(winSize.x, winSize.y);

	SetSizeLimits(winSize.x, 65536.0, winSize.y, 65536.0);

	// Create the tab view
	rect.InsetBy(HSPACE, VSPACE);
	tabView = new BTabView(rect, NULL);

	// Calculate the size and positions of the tab views
	rect.Set(HSPACE, VSPACE, rect.Width() - 2.0f - HSPACE, rect.Height() - 2.0f - fontHeight - VSPACE * 3.0f - 1.0f);

	// Create all the tabs
	label.LoadString(res, IDS_SI_TAB_INSTRUMENT);
	instTab = new APViewInstruments(global, windowSystem, rect, label);
	tabView->AddTab(instTab);

	label.LoadString(res, IDS_SI_TAB_SAMPLE);
	sampTab = new APViewSamples(global, windowSystem, rect, label);
	tabView->AddTab(sampTab);

	// Add the tab view to the window
	topView->AddChild(tabView);

	// Activate the right tab
	tabView->Select(active);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowSampleInfo::~APWindowSampleInfo(void)
{
	BRect winPos;
	int32 x, y, w, h, active;
	PString tempStr;

	// Clear the window pointer
	windowSystem->sampWin = NULL;

	try
	{
		// Store the window position and size if changed
		winPos = Frame();

		x = (int32)winPos.left;
		y = (int32)winPos.top;
		w = winPos.IntegerWidth();
		h = winPos.IntegerHeight();

		// Find active tab
		active = tabView->Selection();

		// Check to see if they have changed
		if ((windowSystem->saveSettings->GetIntEntryValue("Window", "SampleX") != x) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleY") != y) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleWidth") != w) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleHeight") != h))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleX", x);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleY", y);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleWidth", w);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleHeight", h);
		}

		// Check to see if the open status has changed
		tempStr = wasOnScreen ? "Yes" : "No";
		if (windowSystem->saveSettings->GetStringEntryValue("Window", "SampleOpenWindow").CompareNoCase(tempStr) != 0)
			windowSystem->saveSettings->WriteStringEntryValue("Window", "SampleOpenWindow", tempStr);

		// Check to see if the active tab has changed
		if (windowSystem->saveSettings->GetIntEntryValue("Window", "SampleActiveTab") != active)
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleActiveTab", active);

		// Save the tab settings
		instTab->SaveSettings();
		sampTab->SaveSettings();
	}
	catch(...)
	{
		;
	}

	// Delete the filter
	delete keyFilter;
}



/******************************************************************************/
/* OpenWindow() will open or activate the window.                             */
/******************************************************************************/
void APWindowSampleInfo::OpenWindow(void)
{
	// Lock the window
	Lock();

	// Do only show the window if it's hidden
	if (IsHidden())
		Show();

	// Activate it
	Activate();

	// Unlock the window
	Unlock();
}



/******************************************************************************/
/* CloseWindow() will close the window.                                       */
/******************************************************************************/
void APWindowSampleInfo::CloseWindow(void)
{
	// Lock the window
	Lock();

	// Set the shutdown flag and close the window
	shutDown = true;
	QuitRequested();
	Quit();

	// We don't need to unlock the window, because it's destroyed at this point
}



/******************************************************************************/
/* RefreshWindow() will clear the window and add all the items again.         */
/******************************************************************************/
void APWindowSampleInfo::RefreshWindow(void)
{
	// Lock the window
	Lock();

	// Start to empty any waiting samples to be played
	if (sampleLock.Lock() == pSyncOk)
	{
		// Empty the list
		sampleList.MakeEmpty();

		// Done with the writing
		sampleLock.Unlock();
	}

	// Remove all the items from the lists
	instTab->RemoveItems();
	sampTab->RemoveItems();

	// Now add the items
	instTab->AddItems();
	sampTab->AddItems();

	// Unlock it again
	Unlock();
}



/******************************************************************************/
/* AddSampleToQuery() adds the sample information to the play query. The      */
/*      sample will then be played later on.                                  */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to remember.  */
/*         "frequency" is the frequency the sample has to be played with.     */
/******************************************************************************/
void APWindowSampleInfo::AddSampleToQuery(const APSampleInfo *sampInfo, float frequency)
{
	PlaySample playSamp;

	// Only add the sample if it is of the sample type
	// NULL means to stop the current playing sample
	if ((sampInfo == NULL) || (sampInfo->type == apSample) || (sampInfo->type == apHybrid))
	{
		// Create the item to put in the list
		playSamp.sampInfo      = sampInfo;
		playSamp.playFrequency = frequency;

		// Wait to get write access
		if (sampleLock.Lock() == pSyncOk)
		{
			// Add the item to the list
			sampleList.AddTail(playSamp);

			// Done with the writing
			sampleLock.Unlock();
		}
	}
}



/******************************************************************************/
/* GetNextSampleFromQuery() checks the query to see if there is any samples   */
/*      waiting to be played and if so, it removes it from the query and      */
/*      return it.                                                            */
/*                                                                            */
/* Input:  "playSamp" is a pointer to the sample structure to be filled.      */
/*                                                                            */
/* Output: True if there is returned a sample, false if not.                  */
/******************************************************************************/
bool APWindowSampleInfo::GetNextSampleFromQuery(PlaySample *playSamp)
{
	PlaySample playSample;
	bool retVal = false;

	// Wait to get read access
	if (sampleLock.Lock() == pSyncOk)
	{
		// Check to see if there is any items in the list
		if (sampleList.CountItems() > 0)
		{
			// Get the first item
			playSample = sampleList.GetAndRemoveItem(0);

			// Fill out the structure
			playSamp->sampInfo      = playSample.sampInfo;
			playSamp->playFrequency = playSample.playFrequency;

			// We have returned a sample
			retVal = true;
		}

		// Done with the writing
		sampleLock.Unlock();
	}

	return (retVal);
}



/******************************************************************************/
/* ChangeOctave() changes the octave text string.                             */
/*                                                                            */
/* Input:  "startOctave" is the start octave number.                          */
/******************************************************************************/
void APWindowSampleInfo::ChangeOctave(uint16 startOctave)
{
	sampTab->ChangeOctave(startOctave);
}



/******************************************************************************/
/* ChangePolyphony() changes the polyphony text string.                       */
/*                                                                            */
/* Input:  "enable" tells if it has to enable or disable it.                  */
/******************************************************************************/
void APWindowSampleInfo::ChangePolyphony(bool enable)
{
	sampTab->ChangePolyphony(enable);
	poly = enable;
}



/******************************************************************************/
/* PolyphonyEnabled() returns if the polyphony is enabled or not.             */
/*                                                                            */
/* Output: True or false.                                                     */
/******************************************************************************/
bool APWindowSampleInfo::PolyphonyEnabled(void) const
{
	return (poly);
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output: Returns true if it's okay to quit, else false.                     */
/******************************************************************************/
bool APWindowSampleInfo::QuitRequested(void)
{
	// Remember the hidden status
	wasOnScreen = !IsHidden();

	// Hide the window if open
	if (wasOnScreen)
		Hide();

	// Return the shut down status. True means ok to destroy the window,
	// false means its not
	return (shutDown);
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowSampleInfo::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////
		// Drag'n'drop handler
		////////////////////////////////////////////////////////////////////
		case B_SIMPLE_DATA:
		case APLIST_DRAG:
		{
			// Well, the user dropped an object in the window where
			// it's not supported, so we just change the mouse
			// cursor back to normal
			windowSystem->mainWin->SetNormalCursor();
			break;
		}

		////////////////////////////////////////////////////////////////////
		// Global key handler
		////////////////////////////////////////////////////////////////////
		case B_KEY_DOWN:
		{
			BMessage *curMsg;
			int32 key, modifiers;

			// Extract the key that the user has pressed
			curMsg = CurrentMessage();
			if (curMsg->FindInt32("raw_char", &key) == B_OK)
			{
				if (curMsg->FindInt32("modifiers", &modifiers) == B_OK)
				{
					// Mask out lock keys
					modifiers &= ~(B_CAPS_LOCK | B_NUM_LOCK | B_SCROLL_LOCK);

					if ((key == B_ESCAPE) && (modifiers == 0))
					{
						PostMessage(B_QUIT_REQUESTED);
						return;
					}
				}
			}
			break;
		}
		
		////////////////////////////////////////////////////////////////////
		// View message handler
		////////////////////////////////////////////////////////////////////
		default:
		{
			switch (tabView->Selection())
			{
				case TAB_INSTRUMENTS:
				{
//					instTab->HandleMessage(msg);
					break;
				}

				case TAB_SAMPLES:
				{
					sampTab->HandleMessage(msg);
					break;
				}
			}
		}
	}

	// Call base class
	BWindow::MessageReceived(msg);
}



/******************************************************************************/
/* CalcMinSize() will calculate the minimum size the window can have.         */
/*                                                                            */
/* Output: Is a BPoint where the x is the minimum width and y is the minimum  */
/*         height.                                                            */
/******************************************************************************/
BPoint APWindowSampleInfo::CalcMinSize(void)
{
	BPoint size;

	size.x = HSPACE * 5.0f + B_V_SCROLL_BAR_WIDTH + 150.0f;
	size.y = VSPACE * 7.0f + fontHeight * 3.0f + B_H_SCROLL_BAR_HEIGHT + 100.0f;

	return (size);
}
