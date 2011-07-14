/******************************************************************************/
/* APlayer Settings window class.                                             */
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
#include "PSettings.h"
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"
#include "APWindowSettings.h"
#include "APViewOptions.h"
#include "APViewPaths.h"
#include "APViewMixer.h"
#include "APViewNetwork.h"
#include "APViewFileTypes.h"
#include "APViewAddOns.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowSettings::APWindowSettings(MainWindowSystem *system, APGlobalData *global, BRect frame, PString title) : BWindow(frame, NULL, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	BRect rect;
	BPoint winSize;
	PString label;
	char *labelPtr;
	font_height fh;
	float height;
	BMessage *message;

	// Remember the arguments
	windowSystem = system;

	// Remember the resource
	res = windowSystem->res;

	// Set the window title
	SetTitle((labelPtr = title.GetString()));
	title.FreeBuffer(labelPtr);

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
	height     = max(PICVSIZE, fontHeight + VSPACE) + VSPACE;

	// Create the tab view
	tabView = new BTabView(rect, NULL, B_WIDTH_FROM_LABEL);

	// Create all the tabs
	label.LoadString(res, IDS_SET_TAB_OPTIONS);
	optionsTab = new APViewOptions(windowSystem, rect, label);
	tabView->AddTab(optionsTab);

	label.LoadString(res, IDS_SET_TAB_PATHS);
	pathsTab = new APViewPaths(windowSystem, rect, label);
	tabView->AddTab(pathsTab);

	label.LoadString(res, IDS_SET_TAB_MIXER);
	mixerTab = new APViewMixer(windowSystem, global, rect, label);
	tabView->AddTab(mixerTab);

	label.LoadString(res, IDS_SET_TAB_NETWORK);
	networkTab = new APViewNetwork(windowSystem, rect, label);
	tabView->AddTab(networkTab);

	label.LoadString(res, IDS_SET_TAB_FILETYPES);
	fileTypesTab = new APViewFileTypes(windowSystem, global, rect, label);
	tabView->AddTab(fileTypesTab);

	label.LoadString(res, IDS_SET_TAB_PLAYERS);
	playersTab = new APViewAddOns(windowSystem, global, rect, label, "Player", &windowSystem->playerAddOns);
	tabView->AddTab(playersTab);

	label.LoadString(res, IDS_SET_TAB_AGENTS);
	agentsTab = new APViewAddOns(windowSystem, global, rect, label, "Agent", &windowSystem->agentAddOns);
	tabView->AddTab(agentsTab);

	label.LoadString(res, IDS_SET_TAB_CLIENTS);
	clientsTab = new APViewAddOns(windowSystem, global, rect, label, "Client", &windowSystem->clientAddOns);
	tabView->AddTab(clientsTab);

	// Add the tab view to the window
	topView->AddChild(tabView);

	// Add the "save" button
	message = new BMessage(AP_SET_SAVE);
	label.LoadString(res, IDS_BUTTON_SAVE);
	saveButton = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	topView->AddChild(saveButton);
	label.FreeBuffer(labelPtr);

	// Add the "use" button
	message = new BMessage(AP_SET_USE);
	label.LoadString(res, IDS_BUTTON_USE);
	useButton = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_BOTTOM);
	topView->AddChild(useButton);
	label.FreeBuffer(labelPtr);

	// Add the "cancel" button
	message = new BMessage(AP_SET_CANCEL);
	label.LoadString(res, IDS_BUTTON_CANCEL);
	cancelButton = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	topView->AddChild(cancelButton);
	label.FreeBuffer(labelPtr);

	// Check to see if the given size is lesser than the minimum size
	winSize = CalcMinSize();
	if ((frame.Width() < winSize.x) || (frame.Height() < winSize.y))
		ResizeTo(winSize.x, winSize.y);

	// Set view positions and sizes
	SetPosAndSize();

	// Initialize the controls
	InitSettings();
	useThing = true;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowSettings::~APWindowSettings(void)
{
	windowSystem->settingsWin = NULL;

	// Delete the filter
	delete keyFilter;
}



/******************************************************************************/
/* RefreshWindow() will update all the tabs that are dynamic changeable.      */
/******************************************************************************/
void APWindowSettings::RefreshWindow(void)
{
	PostMessage(AP_SET_REFRESH_TABS);
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output: Returns true if it's okay to quit, else false.                     */
/******************************************************************************/
bool APWindowSettings::QuitRequested(void)
{
	BRect winPos;
	int32 x, y, w, h;
	int32 tab;

	try
	{
		// First "use" the configuration
		if (useThing)
		{
			// Save the settings
			optionsTab->RememberSettings();
			pathsTab->RememberSettings();
			mixerTab->RememberSettings();
			networkTab->RememberSettings();
			fileTypesTab->RememberSettings();
			playersTab->RememberSettings();
			agentsTab->RememberSettings();
			clientsTab->RememberSettings();
		}

		// Store the window position and size if changed
		winPos = Frame();

		x = (int32)winPos.left;
		y = (int32)winPos.top;
		w = winPos.IntegerWidth();
		h = winPos.IntegerHeight();

		tab = tabView->Selection();

		// Check to see if they have changed
		if ((windowSystem->saveSettings->GetIntEntryValue("Window", "SettingsX") != x) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SettingsY") != y) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SettingsWidth") != w) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SettingsHeight") != h) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SettingsTab") != tab))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SettingsX", x);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SettingsY", y);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SettingsWidth", w);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SettingsHeight", h);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SettingsTab", tab);
			windowSystem->useSettings->WriteIntEntryValue("Window", "SettingsX", x);
			windowSystem->useSettings->WriteIntEntryValue("Window", "SettingsY", y);
			windowSystem->useSettings->WriteIntEntryValue("Window", "SettingsWidth", w);
			windowSystem->useSettings->WriteIntEntryValue("Window", "SettingsHeight", h);
			windowSystem->useSettings->WriteIntEntryValue("Window", "SettingsTab", tab);
		}
	}
	catch(...)
	{
		;
	}

	// Save the tab window entries
	optionsTab->SaveWindowSettings();
	pathsTab->SaveWindowSettings();
	mixerTab->SaveWindowSettings();
	networkTab->SaveWindowSettings();
	fileTypesTab->SaveWindowSettings();
	playersTab->SaveWindowSettings();
	agentsTab->SaveWindowSettings();
	clientsTab->SaveWindowSettings();

	return (true);
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowSettings::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
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
						// Cancel the settings
						PostMessage(AP_SET_CANCEL);
						return;
					}
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Refresh the tabs
		////////////////////////////////////////////////////////////////////////
		case AP_SET_REFRESH_TABS:
		{
			// Change all the tabs
			mixerTab->SetChannels();

			if (windowSystem->playerInfo->HaveInformation())
			{
				playersTab->UpdateAddOnItem(windowSystem->playerInfo->GetPlayerName());
				agentsTab->UpdateAddOnItem(windowSystem->playerInfo->GetOutputAgent());
			}
			else
			{
				playersTab->ClearAddOnItem();
				agentsTab->ClearAddOnItem();
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Save the settings on disk
		////////////////////////////////////////////////////////////////////////
		case AP_SET_SAVE:
		{
			// Save the settings
			optionsTab->RememberSettings();
			pathsTab->RememberSettings();
			mixerTab->RememberSettings();
			networkTab->RememberSettings();
			fileTypesTab->RememberSettings();
			playersTab->RememberSettings();
			agentsTab->RememberSettings();
			clientsTab->RememberSettings();

			// Save settings in other files
			optionsTab->SaveSettings();
			pathsTab->SaveSettings();
			mixerTab->SaveSettings();
			networkTab->SaveSettings();
			fileTypesTab->SaveSettings();
			playersTab->SaveSettings();
			agentsTab->SaveSettings();
			clientsTab->SaveSettings();

			// Save server settings
			windowSystem->SaveServerSettings();

			// Clone the settings into the save settings and save them
			windowSystem->saveSettings->CloneSettings(windowSystem->useSettings);
			windowSystem->saveSettings->SaveFile("MainWindowSystem.ini", "Polycode", "APlayer");

			// Make a new backup
			MakeBackup();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Just use the current settings
		////////////////////////////////////////////////////////////////////////
		case AP_SET_USE:
		{
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Copy back the backup
		////////////////////////////////////////////////////////////////////////
		case AP_SET_CANCEL:
		{
			// Copy the backup back
			windowSystem->useSettings->CloneSettings(&backupSettings);

			// Cancel the settings
			optionsTab->CancelSettings();
			pathsTab->CancelSettings();
			mixerTab->CancelSettings();
			networkTab->CancelSettings();
			fileTypesTab->CancelSettings();
			playersTab->CancelSettings();
			agentsTab->CancelSettings();
			clientsTab->CancelSettings();

			// Do not make the "use" thing in QuitRequested()
			useThing = false;
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Default handler
		////////////////////////////////////////////////////////////////////////
		default:
		{
			switch (tabView->Selection())
			{
				case SET_TAB_OPTIONS:
				{
					optionsTab->HandleMessage(msg);
					break;
				}

				case SET_TAB_PATHS:
				{
					pathsTab->HandleMessage(msg);
					break;
				}

				case SET_TAB_MIXER:
				{
					mixerTab->HandleMessage(msg);
					break;
				}

				case SET_TAB_NETWORK:
				{
					networkTab->HandleMessage(msg);
					break;
				}

				case SET_TAB_FILETYPES:
				{
					fileTypesTab->HandleMessage(msg);
					break;
				}

				case SET_TAB_PLAYERS:
				{
					playersTab->HandleMessage(msg);
					break;
				}

				case SET_TAB_AGENTS:
				{
					agentsTab->HandleMessage(msg);
					break;
				}

				case SET_TAB_CLIENTS:
				{
					clientsTab->HandleMessage(msg);
					break;
				}
			}

			BWindow::MessageReceived(msg);
			break;
		}
	}
}



/******************************************************************************/
/* CalcMinSize() will calculate the minimum size the window can have.         */
/*                                                                            */
/* Output: Is a BPoint where the x is the minimum width and y is the minimum  */
/*         height.                                                            */
/******************************************************************************/
BPoint APWindowSettings::CalcMinSize(void)
{
	BPoint size;
	float w, h;

	// Set minimum view size
	size.x = HSPACE * 5.0f + SET_MIN_VIEW_WIDTH - 1.0f;
	size.y = VSPACE * 5.0f + tabView->TabHeight() + SET_MIN_VIEW_HEIGHT;

	// Calculate size of the extra buttons
	saveButton->GetPreferredSize(&w, &h);
	size.y += (VSPACE * 2.0f + h - 1.0f);

	return (size);
}



/******************************************************************************/
/* SetPosAndSize() will calculate the positions and sizes for all views.      */
/******************************************************************************/
void APWindowSettings::SetPosAndSize(void)
{
	BRect winRect;
	float x, y, w, h;
	float temp, temp1;

	// Find height of the extra buttons
	saveButton->GetPreferredSize(&w, &h);

	// Calculate the windows size we can draw in
	winRect = topView->Bounds();
	winRect.InsetBy(HSPACE, VSPACE);
	winRect.bottom -= (VSPACE + h - 1.0f);

	// Place the main tab view
	tabView->MoveTo(winRect.left, winRect.top);
	tabView->ResizeTo(winRect.Width(), winRect.Height());

	// Set the size and position of the tab views
	winRect = tabView->Bounds();

	x = winRect.left + HSPACE;
	y = winRect.top + VSPACE;
	optionsTab->MoveTo(x, y);
	optionsTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);
	pathsTab->MoveTo(x, y);
	pathsTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);
	mixerTab->MoveTo(x, y);
	mixerTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);
	networkTab->MoveTo(x, y);
	networkTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);
	fileTypesTab->MoveTo(x, y);
	fileTypesTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);
	playersTab->MoveTo(x, y);
	playersTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);
	agentsTab->MoveTo(x, y);
	agentsTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);
	clientsTab->MoveTo(x, y);
	clientsTab->ResizeTo(SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT);

	// Find the width of the buttons
	winRect = topView->Bounds();
	winRect.InsetBy(HSPACE, VSPACE);

	y = winRect.bottom - h + 1.0f;

	saveButton->MoveTo(winRect.left, y);
	saveButton->ResizeTo(w, h);

	cancelButton->GetPreferredSize(&temp, &h);
	cancelButton->MoveTo(winRect.right - temp, y);
	cancelButton->ResizeTo(temp, h);

	useButton->GetPreferredSize(&temp1, &h);
	useButton->MoveTo((winRect.right - temp - (winRect.left + w) - temp1) / 2.0f + winRect.left + w, y);
	useButton->ResizeTo(temp1, h);
}



/******************************************************************************/
/* InitSettings() will make a backup of the settings and initialize all the   */
/*         views so they show the current settings.                           */
/******************************************************************************/
void APWindowSettings::InitSettings(void)
{
	// Make the backup of the settings
	MakeBackup();

	// Initialize the tab views
	optionsTab->InitSettings();
	pathsTab->InitSettings();
	mixerTab->InitSettings();
	networkTab->InitSettings();
	fileTypesTab->InitSettings();
	playersTab->InitSettings();
	agentsTab->InitSettings();
	clientsTab->InitSettings();

	// Refresh the tabs
	RefreshWindow();

	// Select the last selected tab view
	tabView->Select(windowSystem->useSettings->GetIntEntryValue("Window", "SettingsTab"));
}



/******************************************************************************/
/* MakeBackup() will make a backup of the settings.                           */
/******************************************************************************/
void APWindowSettings::MakeBackup(void)
{
	// Make backup of the settings
	backupSettings.CloneSettings(windowSystem->useSettings);

	// Let the tab views make a backup
	optionsTab->MakeBackup();
	pathsTab->MakeBackup();
	mixerTab->MakeBackup();
	networkTab->MakeBackup();
	fileTypesTab->MakeBackup();
	playersTab->MakeBackup();
	agentsTab->MakeBackup();
	clientsTab->MakeBackup();
}
