/******************************************************************************/
/* APlayer Module Info window class.                                          */
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
#include "PResource.h"
#include "PDirectory.h"
#include "Colors.h"

// APlayerKit headers
#include "ColumnListView.h"
#include "CLVColumn.h"
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"
#include "APWindowModuleInfo.h"
#include "APWindowModuleInfoList.h"
#include "APWindowModuleInfoListItem.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowModuleInfo::APWindowModuleInfo(MainWindowSystem *system, BRect frame, PString title) : BWindow(frame, NULL, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS)
{
	BRect rect;
	BPoint winSize;
	PString label;
	char *titleStr, *labelPtr;
	font_height fh;
	int32 colWidth;

	// Remember the arguments
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

	SetSizeLimits(winSize.x, 32768.0f, winSize.y, 32768.0f);

	// Add the column listview
	rect           = Bounds();
	rect.left     += HSPACE;
	rect.right    -= (HSPACE + B_V_SCROLL_BAR_WIDTH);
	rect.top      += VSPACE;
	rect.bottom   -= (VSPACE + B_H_SCROLL_BAR_HEIGHT);
	columnListView = new APWindowModuleInfoList(rect, &containerView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true, B_PLAIN_BORDER);

	// Add the columns
	label.LoadString(res, IDS_COLUMN_DESCRIPTION);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "InfoCol1W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_NOT_MOVABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_COLUMN_VALUE);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "InfoCol2W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_NOT_MOVABLE));
	label.FreeBuffer(labelPtr);

	// Attach the listview to the window
	topView->AddChild(containerView);

	// Add the items in the listview
	AddItems();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowModuleInfo::~APWindowModuleInfo(void)
{
	BRect winPos;
	int32 x, y, w, h;
	CLVColumn *column;
	PString tempStr;

	// Clear the window pointer
	windowSystem->infoWin = NULL;

	try
	{
		// Store the window position and size if changed
		winPos = Frame();

		x = (int32)winPos.left;
		y = (int32)winPos.top;
		w = winPos.IntegerWidth();
		h = winPos.IntegerHeight();

		// Check to see if they have changed
		if ((windowSystem->saveSettings->GetIntEntryValue("Window", "InfoX") != x) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "InfoY") != y) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "InfoWidth") != w) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "InfoHeight") != h))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", "InfoX", x);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "InfoY", y);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "InfoWidth", w);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "InfoHeight", h);
		}

		// Check to see if the open status has changed
		tempStr = wasOnScreen ? "Yes" : "No";
		if (windowSystem->saveSettings->GetStringEntryValue("Window", "InfoOpenWindow").CompareNoCase(tempStr) != 0)
			windowSystem->saveSettings->WriteStringEntryValue("Window", "InfoOpenWindow", tempStr);

		// Store the column sizes
		column = columnListView->ColumnAt(0);
		w      = (int32)column->Width();

		if (windowSystem->saveSettings->GetIntEntryValue("Window", "InfoCol1W") != w)
			windowSystem->saveSettings->WriteIntEntryValue("Window", "InfoCol1W", w);

		column = columnListView->ColumnAt(1);
		w      = (int32)column->Width();

		if (windowSystem->saveSettings->GetIntEntryValue("Window", "InfoCol2W") != w)
			windowSystem->saveSettings->WriteIntEntryValue("Window", "InfoCol2W", w);
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
void APWindowModuleInfo::OpenWindow(void)
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
void APWindowModuleInfo::CloseWindow(void)
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
void APWindowModuleInfo::RefreshWindow(void)
{
	// Lock the window
	Lock();

	// Remove all the items from the list view
	columnListView->RemoveAllItems();

	// Add the items
	AddItems();

	// Unlock it again
	Unlock();
}



/******************************************************************************/
/* UpdateWindow() will be called everytime a new value has changed.           */
/*                                                                            */
/* Input:  "line" is the line starting from 0.                                */
/*         "newValue" is the new value.                                       */
/******************************************************************************/
void APWindowModuleInfo::UpdateWindow(uint32 line, PString newValue)
{
	APWindowModuleInfoListItem *item;
	bool changed;

	// Lock the window
	Lock();

	// Find the item to change
	item = (APWindowModuleInfoListItem *)columnListView->ItemAt(line + AP_INFO_FIRST_CUSTOM_ITEM);

	if (item != NULL)
	{
		// Now change the item
		changed = item->ChangeColumn(newValue, 1);

		// Invalidate the item so it will be redrawn
		if (changed)
			columnListView->InvalidateItem(line + AP_INFO_FIRST_CUSTOM_ITEM);
	}

	// Unlock the window
	Unlock();
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output: Returns true if it's okay to quit, else false.                     */
/******************************************************************************/
bool APWindowModuleInfo::QuitRequested(void)
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
/* WindowActivated() is called when the window becomes the active one. It     */
/*      will set the focus on the column list view.                           */
/*                                                                            */
/* Input:  "active" is true if the window get active, else false.             */
/******************************************************************************/
void APWindowModuleInfo::WindowActivated(bool active)
{
	if (active)
		columnListView->MakeFocus();
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowModuleInfo::MessageReceived(BMessage *msg)
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
BPoint APWindowModuleInfo::CalcMinSize(void)
{
	BPoint size;

	size.x = HSPACE * 2.0f + B_V_SCROLL_BAR_WIDTH + 50.0f;
	size.y = VSPACE * 2.0f + B_H_SCROLL_BAR_HEIGHT + 50.0f;

	return (size);
}



/******************************************************************************/
/* AddItems() will add the all items in the column list view.                 */
/******************************************************************************/
void APWindowModuleInfo::AddItems(void)
{
	PString desc, val;

	// Check to see if there is any module in memory
	if (!windowSystem->playerInfo->HaveInformation())
	{
		// No module in memory
		val.LoadString(res, IDS_MI_ITEM_NA);

		desc.LoadString(res, IDS_MI_ITEM_MODULENAME);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		desc.LoadString(res, IDS_MI_ITEM_AUTHOR);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		desc.LoadString(res, IDS_MI_ITEM_MODULEFORMAT);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		desc.LoadString(res, IDS_MI_ITEM_ACTIVEPLAYER);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		desc.LoadString(res, IDS_MI_ITEM_CHANNELS);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		desc.LoadString(res, IDS_MI_ITEM_TIME);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		desc.LoadString(res, IDS_MI_ITEM_MODULESIZE);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		desc.LoadString(res, IDS_MI_ITEM_FILE);
		columnListView->AddItem(new APWindowModuleInfoListPathItem(fontHeight, desc, val));
	}
	else
	{
		// Module in memory, add items
		PTimeSpan totalTime;
		int32 i;

		// Module Name
		desc.LoadString(res, IDS_MI_ITEM_MODULENAME);

		val = windowSystem->playerInfo->GetModuleName();
		if (val.IsEmpty())
			val = PDirectory::GetFilePart(windowSystem->playerInfo->GetFileName());

		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		// Author
		desc.LoadString(res, IDS_MI_ITEM_AUTHOR);

		val = windowSystem->playerInfo->GetAuthor();
		if (val.IsEmpty())
			val.LoadString(res, IDS_MI_ITEM_UNKNOWN);

		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		// Module format
		desc.LoadString(res, IDS_MI_ITEM_MODULEFORMAT);
		val = windowSystem->playerInfo->GetModuleFormat();
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		// Active player
		desc.LoadString(res, IDS_MI_ITEM_ACTIVEPLAYER);
		val = windowSystem->playerInfo->GetPlayerName();
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		// Used channels
		desc.LoadString(res, IDS_MI_ITEM_CHANNELS);
		val.SetUNumber(windowSystem->playerInfo->GetModuleChannels());
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		// Total time
		desc.LoadString(res, IDS_MI_ITEM_TIME);
		totalTime.SetTimeSpan((windowSystem->playerInfo->GetTotalTime().GetTotalMilliSeconds() + 500) / 1000 * 1000);

		if (totalTime.GetTotalMilliSeconds() == 0)
			val.LoadString(res, IDS_MI_ITEM_UNKNOWN);
		else
		{
			if (totalTime.GetTotalHours() > 0)
				val.Format("%Ld:%02d:%02d", totalTime.GetTotalHours(), totalTime.GetMinutes(), totalTime.GetSeconds());
			else
				val.Format("%d:%02d", totalTime.GetMinutes(), totalTime.GetSeconds());
		}

		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		// Module size
		desc.LoadString(res, IDS_MI_ITEM_MODULESIZE);
		val.SetUNumber(windowSystem->playerInfo->GetModuleSize(), true);
		columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));

		// File name
		desc.LoadString(res, IDS_MI_ITEM_FILE);
		val = windowSystem->playerInfo->GetFileName();
		columnListView->AddItem(new APWindowModuleInfoListPathItem(fontHeight, desc, val));

		// Add player specific items
		for (i = 0; windowSystem->playerInfo->GetModuleInformation(i, desc, val); i++)
		{
			if (i == 0)
			{
				// Add an empty line
				columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, "", ""));
			}

			// Add the information
			columnListView->AddItem(new APWindowModuleInfoListItem(fontHeight, desc, val));
		}
	}
}
