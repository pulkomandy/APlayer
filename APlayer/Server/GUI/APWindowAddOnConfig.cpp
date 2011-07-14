/******************************************************************************/
/* APlayer AddOn Config window class.                                         */
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
#include "PSettings.h"
#include "Colors.h"

// APlayerKit headers
#include "APAddOns.h"
#include "Layout.h"

// Server headers
#include "APAddOnWindows.h"
#include "APWindowAddOnConfig.h"
#include "APFilter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* APWindowAddOnConfig class                                                  */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowAddOnConfig::APWindowAddOnConfig(PString title, AddOnInfo *info, APAddOnBase *addOn, const APConfigInfo *config, APOpenedConfigWindow *windowItem) : BWindow(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS)
{
	PString text;
	char *textStr;
	float x, y, w, h;
	float temp, temp1;
	BRect rect;
	BView *topView;
	BMessage *message;

	// Remember the config information
	addOnInfo     = info;
	addOnInstance = addOn;
	cfgInfo       = config;
	windowInfo    = windowItem;

	// Initialize member variables
	closedByCancel = false;

	// Calculate the position and size of the extra view with buttons
	rect     = cfgInfo->view->Bounds();
	rect.top = rect.bottom;

	topView = new BView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	// Change the color to light grey
	topView->SetViewColor(BeBackgroundGrey);

	// Add the 3D line
	line = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	topView->AddChild(line);

	// Add the "save" button
	message = new BMessage(APCFG_SAVE);
	text.LoadString(GetApp()->resource, IDS_BUT_SAVE);
	saveButton = new BButton(rect, NULL, (textStr = text.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	text.FreeBuffer(textStr);
	topView->AddChild(saveButton);

	// Add the "use" button
	message = new BMessage(APCFG_USE);
	text.LoadString(GetApp()->resource, IDS_BUT_USE);
	useButton = new BButton(rect, NULL, (textStr = text.GetString()), message, B_FOLLOW_BOTTOM);
	text.FreeBuffer(textStr);
	topView->AddChild(useButton);

	// Add the "cancel" button
	message = new BMessage(APCFG_CANCEL);
	text.LoadString(GetApp()->resource, IDS_BUT_CANCEL);
	cancelButton = new BButton(rect, NULL, (textStr = text.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	text.FreeBuffer(textStr);
	topView->AddChild(cancelButton);

	// Calculate position and size of the extra views
	x = rect.left + HSPACE;
	y = VSPACE * 2.0f;
	w = rect.right - HSPACE - x;

	line->MoveTo(x, y);
	line->ResizeTo(w, 2.0f);

	y += (2.0f + VSPACE * 2.0f);
	saveButton->GetPreferredSize(&w, &h);
	saveButton->MoveTo(x, y);
	saveButton->ResizeTo(w, h);

	cancelButton->GetPreferredSize(&temp, &h);
	cancelButton->MoveTo(rect.right - HSPACE - temp, y);
	cancelButton->ResizeTo(temp, h);

	useButton->GetPreferredSize(&temp1, &h);
	useButton->MoveTo((rect.right - HSPACE - temp - (x + w) - temp1) / 2.0f + x + w, y);
	useButton->ResizeTo(temp1, h);

	// Resize the window to the minimum size
	rect.bottom += (y + h + VSPACE);
	ResizeTo(rect.right, rect.bottom);

	// Add button view to the window
	topView->ResizeTo(rect.right - rect.left, rect.bottom - rect.top);
	AddChild(topView);

	// Add add-on view to the window
	AddChild(cfgInfo->view);

	// Set the window title
	SetTitle(textStr = title.GetString());
	title.FreeBuffer(textStr);

	// Add the message filter
	filter = new APFilter(this);

	// Clone the original settings into the local backup
	backupSettings.CloneSettings(cfgInfo->settings);

	// Does any window position exists in the settings
	if (windowInfo->firstBackupSettings.EntryExist("APlayerConfig", "WinX") &&
		windowInfo->firstBackupSettings.EntryExist("APlayerConfig", "WinY") &&
		windowInfo->firstBackupSettings.EntryExist("APlayerConfig", "WinW") &&
		windowInfo->firstBackupSettings.EntryExist("APlayerConfig", "WinH"))
	{
		// Yes, now set the new window positions
		x = windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinX");
		y = windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinY");
		w = windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinW");
		h = windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinH");

		rect = Frame();
		ResizeTo(rect.right, rect.bottom);
		MoveTo(x, y);
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowAddOnConfig::~APWindowAddOnConfig(void)
{
	// Delete the add-on instance if it was allocated
	if (addOnInfo->agent == NULL)
		addOnInfo->loader->DeleteInstance(addOnInstance);

	// Clear the window pointer in the window list item
	windowInfo->window = NULL;

	// Delete the filter
	delete filter;
}



/******************************************************************************/
/* Quit() is called when the window closes.                                   */
/******************************************************************************/
void APWindowAddOnConfig::Quit(void)
{
	if (!closedByCancel)
	{
		// Refresh the settings assigned to controls that can't update themself
		RefreshSettings(cfgInfo->view);
	}

	// Store the window positions if changed
	CheckAndWriteWindowPositions();

	// Call the base class
	BWindow::Quit();
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowAddOnConfig::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Save the settings on disk
		////////////////////////////////////////////////////////////////////////
		case APCFG_SAVE:
		{
			// Refresh the settings assigned to controls that can't update themself
			RefreshSettings(cfgInfo->view);

			// Clone the changed settings back to the backups
			backupSettings.CloneSettings(cfgInfo->settings);
			windowInfo->firstBackupSettings.CloneSettings(cfgInfo->settings);

			// Check the window positions and save the settings
			CheckAndWriteWindowPositions();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Just use the current settings
		////////////////////////////////////////////////////////////////////////
		case APCFG_USE:
		{
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Copy back the backup
		////////////////////////////////////////////////////////////////////////
		case APCFG_CANCEL:
		{
			// Clone back the backup to the add-on settings
			cfgInfo->settings->CloneSettings(&backupSettings);
			closedByCancel = true;

			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Default handler
		////////////////////////////////////////////////////////////////////////
		default:
		{
			cfgInfo->view->MessageReceived(msg);
			BWindow::MessageReceived(msg);
			break;
		}
	}
}



/******************************************************************************/
/* CheckAndWriteWindowPositions() will check the current window positions     */
/*      against the first backup copy and write the new settings if changed.  */
/******************************************************************************/
void APWindowAddOnConfig::CheckAndWriteWindowPositions(void)
{
	BRect winPos;
	int32 x, y, w, h;

	try
	{
		// Store the window position and size if changed
		winPos = Frame();

		x = (int32)winPos.left;
		y = (int32)winPos.top;
		w = winPos.IntegerWidth();
		h = winPos.IntegerHeight();

		// Write the window position in the settings if they have changed
		if ((windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinX") != x) ||
			(windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinY") != y) ||
			(windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinW") != w) ||
			(windowInfo->firstBackupSettings.GetIntEntryValue("APlayerConfig", "WinH") != h))
		{
			windowInfo->firstBackupSettings.WriteIntEntryValue("APlayerConfig", "WinX", x);
			windowInfo->firstBackupSettings.WriteIntEntryValue("APlayerConfig", "WinY", y);
			windowInfo->firstBackupSettings.WriteIntEntryValue("APlayerConfig", "WinW", w);
			windowInfo->firstBackupSettings.WriteIntEntryValue("APlayerConfig", "WinH", h);
		}

		// Write the settings back on disk
		windowInfo->firstBackupSettings.SaveFile(cfgInfo->fileName, "Polycode", "APlayer");
	}
	catch(PFileException e)
	{
		PString err;
		char *errStr, *nameStr;

		err = PSystem::GetErrorString(e.errorNum);
		APError::ShowError(IDS_ERR_FILE, (nameStr = e.fileName.GetString()), e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);
		e.fileName.FreeBuffer(nameStr);
	}
}



/******************************************************************************/
/* RefreshSettings() will try to find all controls that can't update themself */
/*      in the add-on view, and then invoke them.                             */
/*                                                                            */
/* Input:  "startView" is a pointer to the view to start from.                */
/******************************************************************************/
void APWindowAddOnConfig::RefreshSettings(BView *startView)
{
	BView *workView;
	BMessage *message;

	workView = startView->ChildAt(0);
	while (workView != NULL)
	{
		if (is_kind_of(workView, BTextControl))
		{
			// Get the BMessage assigned with the control
			message = ((BTextControl *)workView)->Message();
			if (message != NULL)
			{
				// Call the main view message receive function to parse the message
				cfgInfo->view->MessageReceived(message);
			}
		}

		// Traverse childs for this view
		RefreshSettings(workView);

		// Take the next view
		workView = workView->NextSibling();
	}
}
