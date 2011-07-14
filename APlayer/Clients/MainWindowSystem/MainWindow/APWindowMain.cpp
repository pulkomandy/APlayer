/******************************************************************************/
/* Main Window Interface.                                                     */
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
#include "PDirectory.h"
#include "PFile.h"
#include "PTime.h"
#include "PTimer.h"
#include "PAlert.h"
#include "PSystem.h"
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APBackView.h"
#include "APToolTips.h"
#include "APPictureButton.h"
#include "APDiskButton.h"
#include "APVolSlider.h"
#include "APPosSlider.h"
#include "APKeyFilter.h"
#include "APWindowMainList.h"
#include "APWindowMainListItem.h"
#include "APWindowMain.h"
#include "APWindowSampleInfo.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowMain::APWindowMain(MainWindowSystem *system, APGlobalData *global, BRect frame) : BWindow(frame, "", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS)
{
	BRect rect(0.0f, 0.0f, 0.0f, 0.0f);
	BRect topRect;
	font_height fh;
	float height;
	PString label, tempStr;
	char *labelPtr;
	BPoint winSize;
	BMenu *menu;
	BMenuItem *menuItem;
	BMessage *message;

	// Remember the arguments
	globalData   = global;
	windowSystem = system;

	// Remember the resource
	res = windowSystem->res;

	// Initialize member variables
	rememberFilled    = false;
	playItem          = NULL;
	modMessenger      = NULL;
	modPanel          = NULL;
	apmlMessenger     = NULL;
	apmlPanel         = NULL;
	apmlSaveMessenger = NULL;
	apmlSavePanel     = NULL;
	neverEndingTimer  = NULL;
	posSliderUpdate   = true;
	neverEndingUpdate = false;

	// Set the time format
	tempStr = windowSystem->useSettings->GetStringEntryValue("General", "TimeFormat");
	if (tempStr.CompareNoCase("remaining") == 0)
		timeFormat = apRemaining;
	else
		timeFormat = apElapsed;

	// Set the master volume
	windowSystem->playerInfo->SetVolume(windowSystem->useSettings->GetIntEntryValue("General", "MasterVolume", 256));

	// Load the needed resources
	res->LoadResource(P_RES_SMALL_ICON | P_RES_CURSOR | P_RES_BITMAP);

	// Create mouse cursors
	forbiddenCursor = CreateCursor(IDC_FORBIDDEN, true);
	appendCursor    = CreateCursor(IDC_APPEND, true);
	insertCursor    = CreateCursor(IDC_INSERT, true);
	sleepCursor     = CreateCursor(IDC_SLEEP, false);

	// Create the menu
	menuBar = new BMenuBar(rect, NULL);

	// Create the file menu
	label.LoadString(res, IDS_MENU_FILE);
	menu = new BMenu((labelPtr = label.GetString()));
	label.FreeBuffer(labelPtr);

	message = new BMessage(AP_PLAY_BUTTON);
	label.LoadString(res, IDS_MENU_FILE_OPEN);
	menuItem = new BMenuItem((labelPtr = label.GetString()), message, 'O');
	menu->AddItem(menuItem);
	menu->AddSeparatorItem();
	label.FreeBuffer(labelPtr);

	message = new BMessage(B_QUIT_REQUESTED);
	label.LoadString(res, IDS_MENU_FILE_QUIT);
	menuItem = new BMenuItem((labelPtr = label.GetString()), message, 'Q');
	menu->AddItem(menuItem);
	menuBar->AddItem(menu);
	label.FreeBuffer(labelPtr);

	// Create the window menu
	label.LoadString(res, IDS_MENU_WINDOW);
	menu = new BMenu((labelPtr = label.GetString()));
	label.FreeBuffer(labelPtr);

	message = new BMessage(AP_MENU_SETTINGS);
	label.LoadString(res, IDS_MENU_WINDOW_SETTINGS);
	menuItem = new BMenuItem((labelPtr = label.GetString()), message);
	menu->AddItem(menuItem);
	menu->AddSeparatorItem();
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_MENU_WINDOW_PLAYERS);
	menuPlayers = new BMenu((labelPtr = label.GetString()));
	menu->AddItem(menuPlayers);
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_MENU_WINDOW_AGENTS);
	menuAgents = new BMenu((labelPtr = label.GetString()));
	menu->AddItem(menuAgents);
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_MENU_WINDOW_CLIENTS);
	menuClients = new BMenu((labelPtr = label.GetString()));
	menu->AddItem(menuClients);
	menuBar->AddItem(menu);
	label.FreeBuffer(labelPtr);

	// Create the help menu
	label.LoadString(res, IDS_MENU_HELP);
	menu = new BMenu((labelPtr = label.GetString()));
	label.FreeBuffer(labelPtr);

	message = new BMessage(AP_MENU_ONLINEHELP);
	label.LoadString(res, IDS_MENU_HELP_ONLINEHELP);
	menuItem = new BMenuItem((labelPtr = label.GetString()), message, 'H');
	menu->AddItem(menuItem);
	menu->AddSeparatorItem();
	label.FreeBuffer(labelPtr);

	message = new BMessage(B_ABOUT_REQUESTED);
	label.LoadString(res, IDS_MENU_HELP_ABOUT);
	menuItem = new BMenuItem((labelPtr = label.GetString()), message);
	menu->AddItem(menuItem);
	label.FreeBuffer(labelPtr);

	// Add the whole menu to the menu bar
	menuBar->AddItem(menu);

	// Add the menu to the window
	AddChild(menuBar);

	// Create top view
	height = menuBar->Bounds().bottom + 1.0f;
	topRect = Bounds();
	topRect.top += height;
	topView = new APBackView(topRect);

	// Add view to the window
	AddChild(topView);

	// Get other needed information
	topView->GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Check to see if the given size is lesser than the minimum size
	winSize = CalcMinSize();
	if ((frame.Width() < winSize.x) || (frame.Height() < winSize.y))
		ResizeTo(winSize.x, winSize.y);

	// Set the size limits for the window
	SetSizeLimits(winSize.x, 32768.0f, winSize.y, 32768.0f);

	// Create the info view with a border
	message  = new BMessage(AP_MODULEINFO_TEXT);
	infoView = new APInfoView(windowSystem, rect, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	topView->AddToolTip(infoView, res, IDS_TIP_MAIN_INFO_TEXT);
	topView->AddChild(infoView);

	infoBox = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	topView->AddChild(infoBox);

	// Create the module info button
	message = new BMessage(AP_MODULEINFO_BUTTON);
	infoBut = new APPictureButton(res, IDI_MAIN_INFO, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_TOP | B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	topView->AddToolTip(infoBut, res, IDS_TIP_MAIN_INFO);
	topView->AddChild(infoBut);

	// Create the volume slider
	message = new BMessage(AP_MUTE_BUTTON);
	muteBut = new APPictureButton(res, IDI_MAIN_VOLUME, rect, message, B_TWO_STATE_BUTTON, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	muteBut->SetWidth(B_V_SCROLL_BAR_WIDTH + 1.0f);
	topView->AddToolTip(muteBut, res, IDS_TIP_MAIN_MUTEBUTTON);
	topView->AddChild(muteBut);

	volSlider = new APVolSlider(-256.0f, 0.0f, B_VERTICAL, B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT);
	volSlider->SetHookFunction(SetMasterVolume, (uint32)this);
	volSlider->SetFlags(volSlider->Flags() | B_FULL_UPDATE_ON_RESIZE);
	volSlider->SetValue(-windowSystem->playerInfo->GetVolume());
	topView->AddToolTip(volSlider, res, IDS_TIP_MAIN_VOLUME);
	topView->AddChild(volSlider);

	// Create the module list
	modList = new APWindowMainList(windowSystem, BRect(0.0f, 0.0f, 0.0f, B_V_SCROLL_BAR_WIDTH), B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS, &modScrollView);
	topView->AddChild(modScrollView);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "ShowListNumber");
	modList->EnableListNumber(tempStr.CompareNoCase("Yes") == 0);

	// Create the list buttons
	message = new BMessage(AP_ADD_BUTTON);
	addBut = new APPictureButton(res, IDI_MAIN_ADD, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	topView->AddToolTip(addBut, res, IDS_TIP_MAIN_ADD);
	topView->AddChild(addBut);

	message = new BMessage(AP_REMOVE_BUTTON);
	removeBut = new APPictureButton(res, IDI_MAIN_REMOVE, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	removeBut->SetEnabled(false);
	topView->AddToolTip(removeBut, res, IDS_TIP_MAIN_REMOVE);
	topView->AddChild(removeBut);

	message = new BMessage(AP_SWAP_BUTTON);
	swapBut = new APPictureButton(res, IDI_MAIN_SWAP, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	swapBut->SetEnabled(false);
	topView->AddToolTip(swapBut, res, IDS_TIP_MAIN_SWAP);
	topView->AddChild(swapBut);

	message = new BMessage(AP_SORT_BUTTON);
	sortBut = new APPictureButton(res, IDI_MAIN_SORT, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(sortBut, res, IDS_TIP_MAIN_SORT);
	topView->AddChild(sortBut);

	message = new BMessage(AP_UP_BUTTON);
	upBut = new APPictureButton(res, IDI_MAIN_UP, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	upBut->SetEnabled(false);
	topView->AddToolTip(upBut, res, IDS_TIP_MAIN_UP);
	topView->AddChild(upBut);

	message = new BMessage(AP_DOWN_BUTTON);
	downBut = new APPictureButton(res, IDI_MAIN_DOWN, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	downBut->SetEnabled(false);
	topView->AddToolTip(downBut, res, IDS_TIP_MAIN_DOWN);
	topView->AddChild(downBut);

	message = new BMessage(AP_SELECT_BUTTON);
	selectBut = new APPictureButton(res, IDI_MAIN_SELECT, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(selectBut, res, IDS_TIP_MAIN_SELECT);
	topView->AddChild(selectBut);

	message = new BMessage(AP_DISK_BUTTON);
	diskBut = new APDiskButton(res, IDI_MAIN_DISK_NORMAL, IDI_MAIN_DISK_PRESSED, rect, message, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(diskBut, res, IDS_TIP_MAIN_DISK);
	topView->AddChild(diskBut);

	listBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddChild(listBox);

	// Create the time view with a border
	timeView = new BStringView(rect, NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	timeView->SetText("0:00/0:00");
	topView->AddToolTip(timeView, res, IDS_TIP_MAIN_TIME);
	topView->AddChild(timeView);

	totalView = new BStringView(rect, NULL, NULL, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	totalView->SetAlignment(B_ALIGN_RIGHT);
	totalView->SetText("0/0");
	topView->AddToolTip(totalView, res, IDS_TIP_MAIN_TOTAL);
	topView->AddChild(totalView);

	timeBox = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);
	topView->AddChild(timeBox);

	// Create the position slider
	message = new BMessage(AP_POS_SLIDER);
	posSlider = new APPosSlider(rect, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	topView->AddToolTip(posSlider, res, IDS_TIP_MAIN_POSITIONSLIDER);
	topView->AddChild(posSlider);

	// Create the tape deck
	message = new BMessage(AP_PREVIOUSMOD_BUTTON);
	prevModBut = new APPictureButton(res, IDI_MAIN_PREVIOUS_MODULE, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	topView->AddToolTip(prevModBut, res, IDS_TIP_MAIN_PREVMOD);
	topView->AddChild(prevModBut);

	message = new BMessage(AP_PREVIOUSSONG_BUTTON);
	prevSongBut = new APPictureButton(res, IDI_MAIN_PREVIOUS_SONG, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(prevSongBut, res, IDS_TIP_MAIN_PREVSONG);
	topView->AddChild(prevSongBut);

	message = new BMessage(AP_REWIND_BUTTON);
	rewBut = new APPictureButton(res, IDI_MAIN_REWIND, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(rewBut, res, IDS_TIP_MAIN_REWIND);
	topView->AddChild(rewBut);

	message = new BMessage(AP_PLAY_BUTTON);
	playBut = new APPictureButton(res, IDI_MAIN_PLAY, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(playBut, res, IDS_TIP_MAIN_PLAY);
	topView->AddChild(playBut);

	message = new BMessage(AP_FORWARD_BUTTON);
	fwdBut = new APPictureButton(res, IDI_MAIN_FORWARD, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(fwdBut, res, IDS_TIP_MAIN_FORWARD);
	topView->AddChild(fwdBut);

	message = new BMessage(AP_NEXTSONG_BUTTON);
	nextSongBut = new APPictureButton(res, IDI_MAIN_NEXT_SONG, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(nextSongBut, res, IDS_TIP_MAIN_NEXTSONG);
	topView->AddChild(nextSongBut);

	message = new BMessage(AP_NEXTMOD_BUTTON);
	nextModBut = new APPictureButton(res, IDI_MAIN_NEXT_MODULE, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(nextModBut, res, IDS_TIP_MAIN_NEXTMOD);
	topView->AddChild(nextModBut);

	message = new BMessage(AP_EJECT_BUTTON);
	ejectBut = new APPictureButton(res, IDI_MAIN_EJECT, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(ejectBut, res, IDS_TIP_MAIN_EJECT);
	topView->AddChild(ejectBut);

	message = new BMessage(AP_PAUSE_BUTTON);
	pauseBut = new APPictureButton(res, IDI_MAIN_PAUSE, rect, message, B_TWO_STATE_BUTTON, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(pauseBut, res, IDS_TIP_MAIN_PAUSE);
	topView->AddChild(pauseBut);

	tapeBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	topView->AddChild(tapeBox);

	// Create the module loop and sample buttons
	modLoopBut = new APPictureButton(res, IDI_MAIN_LOOP, rect, NULL, B_TWO_STATE_BUTTON, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	topView->AddToolTip(modLoopBut, res, IDS_TIP_MAIN_MODULELOOP);
	topView->AddChild(modLoopBut);

	message = new BMessage(AP_SAMPLE_BUTTON);
	sampleBut = new APPictureButton(res, IDI_MAIN_SAMPLE_INFO, rect, message, B_ONE_STATE_BUTTON, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	topView->AddToolTip(sampleBut, res, IDS_TIP_MAIN_SAMP);
	topView->AddChild(sampleBut);

	loopSampleBox = new BBox(rect, NULL, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	topView->AddChild(loopSampleBox);

	// Create the keyboard filter.
	//
	// Will redirect these keys to the main window MessageReceived()
	// function instead of the view in focus
	keyFilter = new APKeyFilter(this);

	keyFilter->AddFilterKey(B_FUNCTION_KEY, 0, B_F12_KEY);		// Play random module
	keyFilter->AddFilterKey(B_DELETE, 0);						// Eject
	keyFilter->AddFilterKey(B_INSERT, 0);						// Play
	keyFilter->AddFilterKey(B_SPACE, 0);						// Pause
	keyFilter->AddFilterKey(B_LEFT_ARROW, 0);					// Rewind
	keyFilter->AddFilterKey(B_RIGHT_ARROW, 0);					// Fast forward
	keyFilter->AddFilterKey(B_BACKSPACE, 0);					// Module loop

	keyFilter->AddFilterKey('1', 0);							// Sub-song selector
	keyFilter->AddFilterKey('2', 0);
	keyFilter->AddFilterKey('3', 0);
	keyFilter->AddFilterKey('4', 0);
	keyFilter->AddFilterKey('5', 0);
	keyFilter->AddFilterKey('6', 0);
	keyFilter->AddFilterKey('7', 0);
	keyFilter->AddFilterKey('8', 0);
	keyFilter->AddFilterKey('9', 0);
	keyFilter->AddFilterKey('0', 0);
	keyFilter->AddFilterKey('+', 0);
	keyFilter->AddFilterKey('-', 0);

	keyFilter->AddFilterKey(B_END, B_NUM_LOCK, 88);				// Sub-song selector on the numpad keyboard
	keyFilter->AddFilterKey(B_DOWN_ARROW, B_NUM_LOCK, 89);
	keyFilter->AddFilterKey(B_PAGE_DOWN, B_NUM_LOCK, 90);
	keyFilter->AddFilterKey(B_LEFT_ARROW, B_NUM_LOCK, 72);
	keyFilter->AddFilterKey(B_HOME, B_NUM_LOCK, 73);
	keyFilter->AddFilterKey(B_RIGHT_ARROW, B_NUM_LOCK, 74);
	keyFilter->AddFilterKey(B_HOME, B_NUM_LOCK, 55);
	keyFilter->AddFilterKey(B_UP_ARROW, B_NUM_LOCK, 56);
	keyFilter->AddFilterKey(B_PAGE_UP, B_NUM_LOCK, 57);
	keyFilter->AddFilterKey(B_INSERT, B_NUM_LOCK, 100);

	keyFilter->AddFilterKey(B_SPACE, B_SHIFT_KEY);				// Mute
	keyFilter->AddFilterKey(B_LEFT_ARROW, B_SHIFT_KEY);			// Load previous module
	keyFilter->AddFilterKey(B_RIGHT_ARROW, B_SHIFT_KEY);		// Load next module

	// Enable or disable window options
	EnableToolTips(windowSystem->useSettings->GetStringEntryValue("Options", "ToolTips").CompareNoCase("Yes") == 0);
	EnableShowNameInTitlebar(windowSystem->useSettings->GetStringEntryValue("Options", "ShowNameInTitle").CompareNoCase("Yes") == 0);

	// Set view positions and sizes
	SetPosAndSize();

	// Update the tape deck buttons
	UpdateTapeDeck();

	// Print module info
	PrintInfo();

	// Create the timer object
	timer = new PTimer(this, AP_TIME_CLOCK);
	if (timer == NULL)
		throw PMemoryException();

	timer->SetElapseValue(1000);		// One second
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowMain::~APWindowMain(void)
{
	// Delete the timer objects
	delete timer;

	// Delete the file panels
	delete modPanel;
	delete apmlPanel;
	delete apmlSavePanel;

	// Delete the messenger objects
	delete modMessenger;
	delete apmlMessenger;
	delete apmlSaveMessenger;

	// Delete the cursor objects
	delete sleepCursor;
	delete insertCursor;
	delete appendCursor;
	delete forbiddenCursor;

	// Delete the filter
	delete keyFilter;
}



/******************************************************************************/
/* AddAddOnsToMenu() will add all the add-ons that have either a              */
/*      configuration or display window to the window menu.                   */
/******************************************************************************/
void APWindowMain::AddAddOnsToMenu(void)
{
	// Start to lock the window
	Lock();

	// Now add the players
	AddAddOnListToMenu(windowSystem->playerAddOns, menuPlayers, AP_MENU_ADDONDISPLAYVIEW);
	AddAddOnListToMenu(windowSystem->playerAddOns, menuPlayers, AP_MENU_ADDONCONFIGVIEW);

	// The agents
	AddAddOnListToMenu(windowSystem->agentAddOns, menuAgents, AP_MENU_ADDONDISPLAYVIEW);
	AddAddOnListToMenu(windowSystem->agentAddOns, menuAgents, AP_MENU_ADDONCONFIGVIEW);

	// And at last the clients
	AddAddOnListToMenu(windowSystem->clientAddOns, menuClients, AP_MENU_ADDONDISPLAYVIEW);
	AddAddOnListToMenu(windowSystem->clientAddOns, menuClients, AP_MENU_ADDONCONFIGVIEW);

	// Unlock the window again
	Unlock();
}



/******************************************************************************/
/* AddAddOnToMenu() will add the menu with the add-on name if any.            */
/*                                                                            */
/* Input:  "addOn" is the the add-on.                                         */
/******************************************************************************/
void APWindowMain::AddAddOnToMenu(APAddOnInformation *addOn)
{
	BMenu *workMenu;
	BMenuItem *item;
	int32 i, count;
	bool insert = true;

	// Start to lock the window
	Lock();

	// Find the menu pointer to use
	switch (addOn->type)
	{
		case apPlayer:
		{
			workMenu = menuPlayers;
			break;
		}

		case apAgent:
		{
			workMenu = menuAgents;
			break;
		}

		case apClient:
		{
			workMenu = menuClients;
			break;
		}

		default:
		{
			// Type not implemented
			ASSERT(false);
			Unlock();
			return;
		}
	}

	// Search through the menu to find the right place to insert the new
	// menu at
	count = workMenu->CountItems();
	for (i = 0; i < count; i++)
	{
		item = workMenu->ItemAt(i);
		if (item != NULL)
		{
			if (addOn->name == item->Label())
			{
				insert = false;
				break;
			}

			if (addOn->name < item->Label())
				break;
		}
	}

	if (insert)
	{
		PString config, display;
		char *nameStr, *configStr, *displayStr;
		BMenu *subMenu = NULL;
		BMenuItem *menuItem;
		BMessage *message;

		// Initialize the strings
		config.LoadString(res, IDS_MENU_SETTINGS);
		display.LoadString(res, IDS_MENU_DISPLAY);
		nameStr = addOn->name.GetString();

		// Found the right place, create the menu item
		if (addOn->display)
		{
			// Add the item as a submenu
			subMenu = new BMenu(nameStr);
			workMenu->AddItem(subMenu, (i == count ? count : i == 0 ? 0 : i - 1));

			// Create message to send
			message = new BMessage(AP_MENU_ADDONDISPLAYVIEW);
			message->AddString("addOn", nameStr);

			// And add the display item
			menuItem = new BMenuItem((displayStr = display.GetString()), message);
			subMenu->AddItem(menuItem);
			display.FreeBuffer(displayStr);
		}

		if (addOn->settings)
		{
			// Check if we have already created a submenu with a display item
			if (subMenu == NULL)
			{
				// No, create the submenu
				subMenu = new BMenu(nameStr);
				workMenu->AddItem(subMenu, (i == count ? count : i == 0 ? 0 : i - 1));
			}

			// Create the message to send
			message = new BMessage(AP_MENU_ADDONCONFIGVIEW);
			message->AddString("addOn", nameStr);

			// And add the config item
			menuItem = new BMenuItem((configStr = config.GetString()), message);
			subMenu->AddItem(menuItem);
			config.FreeBuffer(configStr);
		}

		// Free name buffer
		addOn->name.FreeBuffer(nameStr);
	}

	// Unlock the window
	Unlock();
}



/******************************************************************************/
/* RemoveAddOnFromMenu() will remove the menu with the add-on name if any.    */
/*                                                                            */
/* Input:  "addOn" is the the add-on.                                         */
/******************************************************************************/
void APWindowMain::RemoveAddOnFromMenu(APAddOnInformation *addOn)
{
	BMenu *workMenu;
	BMenuItem *item;
	char *nameStr;

	// Start to lock the window
	Lock();

	// Find the menu pointer to use
	switch (addOn->type)
	{
		case apPlayer:
		{
			workMenu = menuPlayers;
			break;
		}

		case apAgent:
		{
			workMenu = menuAgents;
			break;
		}

		case apClient:
		{
			workMenu = menuClients;
			break;
		}

		default:
		{
			// Type not implemented
			ASSERT(false);
			Unlock();
			return;
		}
	}

	// See if the add-on have a menu
	item = workMenu->FindItem((nameStr = addOn->name.GetString()));
	if (item != NULL)
	{
		// We found one, now remove it
		VERIFY(workMenu->RemoveItem(item));
		delete item;
	}

	addOn->name.FreeBuffer(nameStr);

	// Unlock the window
	Unlock();
}



/******************************************************************************/
/* AddFilesToList() will parse a refs message and add all the files to the    */
/*      module list.                                                          */
/*                                                                            */
/* Input:  "message" is a pointer to refs message holding all the files.      */
/*         "skipList" indicates if you want to skip list files.               */
/*         "startIndex" is where to insert the files or -1 to add them in the */
/*         end of the list.                                                   */
/******************************************************************************/
void APWindowMain::AddFilesToList(BMessage *message, bool skipList, int32 startIndex)
{
	entry_ref entRef;
	BEntry entry, entryExpanded;
	BPath path;
	int32 i;
	BList itemList(50);

	// Set the sleep cursor
	SetSleepCursor();

	for (i = 0; (message->FindRef("refs", i, &entRef) == B_OK); i++)
	{
		// Make sure we have the full path to the file
		entry.SetTo(&entRef);
		entry.GetPath(&path);
		entryExpanded.SetTo(&entRef, true);

		if (entryExpanded.IsDirectory())
			AddDirectoryToList(path.Path(), itemList);
		else
		{
			// Add the file to the list
			AppendMultiFile(path.Path(), itemList);
		}
	}

	// Add the items to the list
	if (startIndex == -1)
		modList->AddList(&itemList);
	else
		modList->AddList(&itemList, startIndex);

	// Update the controls
	UpdateListCount();
	UpdateTimes();
	UpdateTapeDeck();

	// Set the cursor back to normal
	SetNormalCursor();
}



/******************************************************************************/
/* AddFileToList() will add a single file to the module list.                 */
/*                                                                            */
/* Input:  "fileName" is the full path to the file to add.                    */
/*         "updateWindow" tells the function to update the window or not.     */
/******************************************************************************/
void APWindowMain::AddFileToList(PString fileName, bool updateWindow)
{
	// Lock the window and add the item to the list
	Lock();
	AppendMultiFile(fileName);

	// Update the window?
	if (updateWindow)
	{
		UpdateListCount();
		UpdateTimes();
		UpdateTapeDeck();
	}

	// Unlock the window
	Unlock();
}



/******************************************************************************/
/* UpdateList() will update all the list controls.                            */
/******************************************************************************/
void APWindowMain::UpdateList(void)
{
	// Lock the window
	Lock();

	// Update the window
	UpdateListCount();
	UpdateTimes();
	UpdateTapeDeck();

	// Unlock the window
	Unlock();
}



/******************************************************************************/
/* SetNormalCursor() will change the mouse cursor to the normal (hand) image. */
/******************************************************************************/
void APWindowMain::SetNormalCursor(void)
{
	be_app->SetCursor(B_CURSOR_SYSTEM_DEFAULT);
}



/******************************************************************************/
/* SetForbiddenCursor() will change the mouse cursor to the forbidden image.  */
/******************************************************************************/
void APWindowMain::SetForbiddenCursor(void)
{
	be_app->SetCursor(forbiddenCursor);
}



/******************************************************************************/
/* SetAppendCursor() will change the mouse cursor to the append image.        */
/******************************************************************************/
void APWindowMain::SetAppendCursor(void)
{
	be_app->SetCursor(appendCursor);
}



/******************************************************************************/
/* SetInsertCursor() will change the mouse cursor to the insert image.        */
/******************************************************************************/
void APWindowMain::SetInsertCursor(void)
{
	be_app->SetCursor(insertCursor);
}



/******************************************************************************/
/* SetSleepCursor() will change the mouse cursor to the sleep image.          */
/******************************************************************************/
void APWindowMain::SetSleepCursor(void)
{
	be_app->SetCursor(sleepCursor);
}



/******************************************************************************/
/* EnableListNumber() will enable or disable the list number showing.         */
/*                                                                            */
/* Input:  "enable" is true if you want to enable it, false if not.           */
/******************************************************************************/
void APWindowMain::EnableListNumber(bool enable)
{
	modList->EnableListNumber(enable);
}



/******************************************************************************/
/* EnableToolTips() will enable or disable the tool tips.                     */
/*                                                                            */
/* Input:  "enable" is true if you want to enable it, false if not.           */
/******************************************************************************/
void APWindowMain::EnableToolTips(bool enable)
{
	topView->SetEnabled(enable);
}



/******************************************************************************/
/* EnableShowNameInTitlebar() will enable or disable the "module name in      */
/*      titlebar" function.                                                   */
/*                                                                            */
/* Input:  "enable" is true if you want to enable it, false if not.           */
/******************************************************************************/
void APWindowMain::EnableShowNameInTitlebar(bool enable)
{
	// Lock the window
	Lock();

	// Remember the setting and change the title
	showName = enable;

	if (enable)
		ShowModuleName();
	else
		ShowNormalTitle();

	// Unlock the window again
	Unlock();
}



/******************************************************************************/
/* GetListItem() will return the item at the index given.                     */
/*                                                                            */
/* Input:   "index" is the list index to the item to return.                  */
/*                                                                            */
/* Output:  The item or NULL if the index is out of range.                    */
/******************************************************************************/
APWindowMainListItem *APWindowMain::GetListItem(int32 index)
{
	return ((APWindowMainListItem *)modList->ItemAt(index));
}



/******************************************************************************/
/* GetPlayItem() returns the current playing module index.                    */
/*                                                                            */
/* Output:  The index to the current playing module or -1 if none is playing. */
/******************************************************************************/
int32 APWindowMain::GetPlayItem(void)
{
	return (modList->IndexOf(playItem));
}



/******************************************************************************/
/* GetListCount() returns the number of items in the module list.             */
/*                                                                            */
/* Output:  The number of modules in the module list.                         */
/******************************************************************************/
int32 APWindowMain::GetListCount(void)
{
	return (modList->CountItems());
}



/******************************************************************************/
/* SetTimeOnItem() will change the time on the item given.                    */
/*                                                                            */
/* Input:  "item" is a pointer to the item.                                   */
/*         "time" is the new time.                                            */
/******************************************************************************/
void APWindowMain::SetTimeOnItem(APWindowMainListItem *item, PTimeSpan time)
{
	if (item != NULL)
	{
		PTimeSpan prevTime;

		// Get the previous item time
		prevTime = item->GetTime();

		// Change the time on the item
		item->SetTime(time);

		// And update it in the list
		modList->InvalidateItem(modList->IndexOf(item));

		// Now calculate the new list time
		listTime = listTime - prevTime + time;

		// And show it
		UpdateTimes();
	}
}



/******************************************************************************/
/* RemoveItemTimeFromList() will subtrack the item given time from the list   */
/*      time.                                                                 */
/*                                                                            */
/* Input:  "item" is a pointer to the item.                                   */
/******************************************************************************/
void APWindowMain::RemoveItemTimeFromList(APWindowMainListItem *item)
{
	// Subtract the item time
	listTime -= item->GetTime();
}



/******************************************************************************/
/* RemoveListItem() removes the item at the index given in the module list.   */
/*                                                                            */
/* Input:   "index" is the index to the item to remove.                       */
/******************************************************************************/
void APWindowMain::RemoveListItem(int32 index)
{
	APWindowMainListItem *item;

	// Delete the item
	item = (APWindowMainListItem *)modList->RemoveItem(index);
	RemoveItemTimeFromList(item);
	delete item;

	// Update the controls
	UpdateListCount();
	UpdateTimes();
	UpdateTapeDeck();
}



/******************************************************************************/
/* EmptyList() will remove all the items from the module list.                */
/*                                                                            */
/* Input:  "lock" is true if you want to lock the window.                     */
/******************************************************************************/
void APWindowMain::EmptyList(bool lock)
{
	// Clear the module list
	modList->RemoveAllItems(lock);

	// Lock the window?
	if (lock)
		Lock();

	// Clear the time variables
	listTime.SetTimeSpan(0);
	selectedTime.SetTimeSpan(0);

	// And update the window
	UpdateTimes();

	// Update the "number of files" view
	totalView->SetText("0/0");

	// Update the tape deck
	UpdateTapeDeck();

	// Clear the random list
	randomList.MakeEmpty();

	// Unlock again
	if (lock)
		Unlock();
}



/******************************************************************************/
/* InitWindowWhenPlayStarts() initialize all the buttons etc. when a new      */
/*      module starts playing.                                                */
/******************************************************************************/
void APWindowMain::InitWindowWhenPlayStarts(void)
{
	// Post a message telling the window to update it's buttons
	PostMessage(AP_INIT_CONTROLS);
}



/******************************************************************************/
/* StopAndFreeModule() will stop the current playing module and free it.      */
/******************************************************************************/
void APWindowMain::StopAndFreeModule(void)
{
	// Stop playing the module
	windowSystem->StopAndFreeModule();

	// Stop the timers
	StopTimers();

	// Lock the window
	Lock();

	// Reset the position slider
	posSlider->SetValue(0);

	// Set the window title back to normal
	ShowNormalTitle();

	// Unlock again
	Unlock();
}



/******************************************************************************/
/* InitSubSongs() initialize the sub-songs variables.                         */
/******************************************************************************/
void APWindowMain::InitSubSongs(void)
{
	subSongMultiply = 0;
}



/******************************************************************************/
/* IsLoopOn() checks to see if the loop button is pressed.                    */
/*                                                                            */
/* Output:  True if loop is on, false if not.                                 */
/******************************************************************************/
bool APWindowMain::IsLoopOn(void)
{
	return (modLoopBut->Value() == B_CONTROL_ON);
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output:  Returns true if it's okay to quit, else false.                    */
/******************************************************************************/
bool APWindowMain::QuitRequested(void)
{
	BRect winPos;
	int32 x, y, w, h;
	PString tempStr;
	uint16 volume;
	TimeFormat format;

	// Make sure the window is locked
	Lock();

	// Remember the current playing module
	if (!rememberFilled)
	{
		if (playItem != NULL)
		{
			rememberSelected = modList->IndexOf(playItem);
			rememberPosition = windowSystem->playerInfo->GetSongPosition();
			rememberSong     = windowSystem->playerInfo->GetCurrentSong();
		}
		else
		{
			rememberSelected = -1;
			rememberPosition = -1;
			rememberSong     = -1;
		}

		rememberFilled = true;
	}

	// Unlock the window again
	Unlock();

	// Free any playing modules
	StopAndFreeModule();

	// Close the connection to the APlayer server
	globalData->communication->DisconnectFromServer(windowSystem->serverHandle);

	// Do not close the window if the user pressed the "close window"
	// button. The main class will then make sure the window will
	// be closed
	if (!windowSystem->closeWindow)
		return (false);

	// And lock the window one more time
	Lock();

	// Save the module list
	RememberModuleList();

	// Store the window position and size if changed
	winPos = Frame();

	x = (int32)winPos.left;
	y = (int32)winPos.top;
	w = winPos.IntegerWidth();
	h = winPos.IntegerHeight();

	// Check to see if they have changed
	if ((windowSystem->saveSettings->GetIntEntryValue("Window", "MainX") != x) ||
		(windowSystem->saveSettings->GetIntEntryValue("Window", "MainY") != y) ||
		(windowSystem->saveSettings->GetIntEntryValue("Window", "MainWidth") != w) ||
		(windowSystem->saveSettings->GetIntEntryValue("Window", "MainHeight") != h))
	{
		windowSystem->saveSettings->WriteIntEntryValue("Window", "MainX", x);
		windowSystem->saveSettings->WriteIntEntryValue("Window", "MainY", y);
		windowSystem->saveSettings->WriteIntEntryValue("Window", "MainWidth", w);
		windowSystem->saveSettings->WriteIntEntryValue("Window", "MainHeight", h);
	}

	// Remember the master volume?
	volume = windowSystem->playerInfo->GetVolume();
	if (windowSystem->saveSettings->GetIntEntryValue("General", "MasterVolume") != volume)
		windowSystem->saveSettings->WriteIntEntryValue("General", "MasterVolume", volume);

	// Remember the time format?
	tempStr = windowSystem->saveSettings->GetStringEntryValue("General", "TimeFormat");
	if (tempStr.CompareNoCase("remaining") == 0)
		format = apRemaining;
	else
		format = apElapsed;

	if (format != timeFormat)
		windowSystem->saveSettings->WriteStringEntryValue("General", "TimeFormat", timeFormat == apElapsed ? "Elapsed" : "Remaining");

	// Unlock the window again
	Unlock();

	return (true);
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowMain::MessageReceived(BMessage *msg)
{
	try
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
				SetNormalCursor();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Show the about window
			////////////////////////////////////////////////////////////////////
			case B_ABOUT_REQUESTED:
			{
				windowSystem->ShowAboutWindow();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Keyboard events
			////////////////////////////////////////////////////////////////////
			case B_KEY_DOWN:
			{
				BMessage *curMsg;
				int32 key, modifiers, code;
				bool callBase = true;

				// Extract the key that the user has pressed
				curMsg = CurrentMessage();
				if (curMsg->FindInt32("raw_char", &key) == B_OK)
				{
					if (curMsg->FindInt32("modifiers", &modifiers) == B_OK)
					{
						if (curMsg->FindInt32("key", &code) == B_OK)
							callBase = !ParseKey((char)key, modifiers, code);
					}
				}

				if (callBase)
					BWindow::MessageReceived(msg);

				break;
			}

			////////////////////////////////////////////////////////////////////
			// Timer messages
			////////////////////////////////////////////////////////////////////
			case PM_TIMER:
			{
				if (playItem != NULL)
				{
					// Get the timer id
					int16 timerID;

					if (msg->FindInt16("ID", &timerID) == B_OK)
					{
						switch (timerID)
						{
							//
							// The normal clock in the window
							//
							case AP_TIME_CLOCK:
							{
								// Do only print the information if the module is playing
								if (windowSystem->playerInfo->IsPlaying())
								{
									// Calculate time offset
									timeOccurred = PTime::GetNow() - timeStart;

									// Print the module information
									PrintInfo();
								}
								break;
							}

							//
							// Never ending timer, jump to the next module
							//
							case AP_TIME_NEVERENDING:
							{
								// Emulate a module end
								windowSystem->EndModule();
								break;
							}
						}
					}
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Got "play button" file information from the file panel
			////////////////////////////////////////////////////////////////////
			case AP_MOD_PLAY_REFS_RECEIVED:
			{
				// Stop playing any modules
				StopAndFreeModule();

				// Clear the module list
				EmptyList(false);

				// Add all the files in the module list
				AddFilesToList(msg, true);

				// Start to load and play the first module
				windowSystem->LoadAndPlayModule(0);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Got "add button" file information from the file panel
			////////////////////////////////////////////////////////////////////
			case AP_MOD_ADD_REFS_RECEIVED:
			{
				int32 selected, jumpNum;

				// Get the selected item
				selected = modList->CurrentSelection();
				if (selected < 0)
				{
					selected = -1;
					jumpNum  = modList->CountItems();
				}
				else
					jumpNum = selected;

				// Add all the files in the module list
				AddFilesToList(msg, true, selected);

				// Free any double buffer loaded modules
				windowSystem->FreeExtraModules();

				// Should we load the first added module?
				if (windowSystem->useSettings->GetStringEntryValue("Options", "AddJump").CompareNoCase("Yes") == 0)
				{
					// Stop playing any modules and load the first added one
					StopAndFreeModule();
					windowSystem->LoadAndPlayModule(jumpNum);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Got "APML load" file information from the file panel
			////////////////////////////////////////////////////////////////////
			case AP_APML_LOAD_REFS_RECEIVED:
			{
				entry_ref entRef;
				BEntry entry;
				BPath path;

				// Extract the directory and file name
				if (msg->FindRef("refs", &entRef) == B_OK)
				{
					entry.SetTo(&entRef);
					entry.GetPath(&path);

					// Remember the directory
					apmlPanel->GetPanelDirectory(&apmlLastEntry);

					// Stop playing any modules
					StopAndFreeModule();

					// Clear the module list
					EmptyList(false);

					// Load the file into the module list
					AppendListFile(path.Path(), -1);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Got "APML append" file information from the file panel
			////////////////////////////////////////////////////////////////////
			case AP_APML_APPEND_REFS_RECEIVED:
			{
				int32 selected, jumpNum;
				entry_ref entRef;
				BEntry entry;
				BPath path;

				// Extract the directory and file name
				if (msg->FindRef("refs", &entRef) == B_OK)
				{
					entry.SetTo(&entRef);
					entry.GetPath(&path);

					// Remember the directory
					apmlPanel->GetPanelDirectory(&apmlLastEntry);

					// Get the selected item
					selected = modList->CurrentSelection();
					if (selected < 0)
					{
						selected = -1;
						jumpNum  = modList->CountItems();
					}
					else
						jumpNum = selected;

					// Load the file into the module list
					AppendListFile(path.Path(), selected);

					// Free any double buffer loaded modules
					windowSystem->FreeExtraModules();

					// Should we load the first added module
					if (windowSystem->useSettings->GetStringEntryValue("Options", "AddJump").CompareNoCase("Yes") == 0)
					{
						// Stop playing any modules and load the first added one
						StopAndFreeModule();
						windowSystem->LoadAndPlayModule(jumpNum);
					}
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Got "APML save" file information from the file panel
			////////////////////////////////////////////////////////////////////
			case AP_APML_SAVE_REFS_RECEIVED:
			{
				entry_ref entRef;
				BEntry entry;
				BPath path;
				const char *fileName;

				// Extract the directory and file name
				if (msg->FindRef("directory", &entRef) == B_OK)
				{
					entry.SetTo(&entRef);
					entry.GetPath(&path);

					if (msg->FindString("name", &fileName) == B_OK)
					{
						// Well, now it's time to save the module list
						path.Append(fileName);

						// Remember the directory
						apmlSavePanel->GetPanelDirectory(&apmlLastEntry);

						// Save the module list
						SaveAPMLFile(path.Path());
					}
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Initialize window controls
			////////////////////////////////////////////////////////////////////
			case AP_INIT_CONTROLS:
			{
				BRect listRect, itemRect;
				int32 index;

				prevPosition = -1;

				// Start the timers
				if (playItem != NULL)
					StartTimers();

				// Make sure the pause button isn't pressed
				pauseBut->SetValue(B_CONTROL_OFF);

				// Set the position slider on the first position
				posSlider->SetValue(0);

				// Update the tape deck buttons
				UpdateTapeDeck();

				// Get the total time of the playing song
				songTotalTime = windowSystem->playerInfo->GetTotalTime();

				// Change the time to the position time
				SetPositionTime(windowSystem->playerInfo->GetSongPosition());

				// Print the module information
				PrintInfo();

				// Change the window title
				ShowModuleName();

				// Update the list item
				SetTimeOnItem(playItem, songTotalTime);

				if (playItem != NULL)
				{
					// Check to see if the playing item can be seen
					listRect = modList->Bounds();
					itemRect = modList->ItemFrame(modList->IndexOf(playItem));

					if (!listRect.Contains(itemRect))
					{
						// Make sure the item can be seen
						modList->ScrollTo(0.0f, itemRect.top - listRect.Height() / 2.0f);
					}

					// Add the new index to the random list
					index = modList->IndexOf(playItem);
					randomList.AddTail(index);

					// Do we need to remove any previous items
					if (randomList.CountItems() > (modList->CountItems() / 3))
						randomList.RemoveItem(0);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Update the information control
			////////////////////////////////////////////////////////////////////
			case AP_UPDATE_INFORMATION:
			{
				int16 position;

				// Get the position
				if (msg->FindInt16("position", &position) == B_OK)
				{
					// Change the time to the position time
					if (position < prevPosition)
						SetPositionTime(position);

					prevPosition = position;

					// Set the new position
					windowSystem->playerInfo->SetSongPosition(position);

					// Print the information
					PrintInfo();
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Update the window when a list selection has changed
			////////////////////////////////////////////////////////////////////
			case AP_UPDATE_SELECTION:
			{
				// Update the time and count controls
				UpdateTimes();
				UpdateListCount();

				// Update the list controls
				UpdateListControls();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Reset the play time
			////////////////////////////////////////////////////////////////////
			case AP_RESET_TIME:
			{
				int16 position;

				// Get the current position
				position = windowSystem->playerInfo->GetSongPosition();

				// Set the time to the position time
				SetPositionTime(position);

				// Print the information
				PrintInfo();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Change the play item
			////////////////////////////////////////////////////////////////////
			case AP_CHANGE_PLAY_ITEM:
			{
				int32 index;

				// Get the index of the item to select or deselect
				if (msg->FindInt32("index", &index) == B_OK)
				{
					// Remember the item
					playItem = (APWindowMainListItem *)modList->ItemAt(index);

					// Change the item
					modList->SetPlayItem(playItem);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User selected to show the settings
			////////////////////////////////////////////////////////////////////
			case AP_MENU_SETTINGS:
			{
				windowSystem->ShowSettingsWindow();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User selected to show the display window in an add-on
			////////////////////////////////////////////////////////////////////
			case AP_MENU_ADDONDISPLAYVIEW:
			{
				const char *addOnName;

				// Get the name of the add-on
				if (msg->FindString("addOn", &addOnName) == B_OK)
				{
					// Tell the server to open the configuration window
					windowSystem->OpenDisplayWindow(addOnName);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User selected to show the configuration window in an add-on
			////////////////////////////////////////////////////////////////////
			case AP_MENU_ADDONCONFIGVIEW:
			{
				const char *addOnName;

				// Get the name of the add-on
				if (msg->FindString("addOn", &addOnName) == B_OK)
				{
					// Tell the server to open the configuration window
					windowSystem->OpenConfigWindow(addOnName);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User selected the On Line Help menu item
			////////////////////////////////////////////////////////////////////
			case AP_MENU_ONLINEHELP:
			{
				BPath path;
				BFile file;
				PDirectory dir;
				PString dirName, fileName;
				char *nameStr;
				BMimeType htmlType("text/html");
				char browserSignature[B_MIME_TYPE_LENGTH];
				status_t retVal;

				// Find the "URL"
				dir.FindDirectory(PDirectory::pLaunch);
				dirName = dir.GetDirectory();
				retVal  = path.SetTo((nameStr = dirName.GetString()));
				dirName.FreeBuffer(nameStr);

				if (retVal != B_OK)
					throw PFileException(PSystem::ConvertOSError(retVal), path.Path());

				path.Append("Documentation/index.html");
				fileName.Format("file://%s", path.Path());

				// Check to see if the file exists
				retVal = file.SetTo(path.Path(), B_READ_ONLY);
				if (retVal != B_OK)
					throw PFileException(PSystem::ConvertOSError(retVal), path.Path());

				// Close the file again
				file.Unset();

				// Find default browser signature
				htmlType.GetPreferredApp(browserSignature);

				// Create message
				BMessage msg(B_ARGV_RECEIVED);

				msg.AddString("argv", "foo");
				msg.AddString("argv", (nameStr = fileName.GetString()));
				msg.AddInt32("argc", 2);
				fileName.FreeBuffer(nameStr);

				// And send the message to the browser
				BMessenger messenger(browserSignature, -1, NULL);

				if (messenger.IsValid())
					messenger.SendMessage(&msg);
				else
					be_roster->Launch(browserSignature, &msg);

				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the position slider
			////////////////////////////////////////////////////////////////////
			case AP_POS_SLIDER:
			{
				int16 newSongPos;

				// Calculate the song position
				newSongPos = posSlider->Value() * windowSystem->playerInfo->GetSongLength() / 100;

				// Set the new song position
				SetPosition(newSongPos);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the info bar
			////////////////////////////////////////////////////////////////////
			case AP_MODULEINFO_TEXT:
			{
				// Switch between the time formats
				if (timeFormat == apElapsed)
					timeFormat = apRemaining;
				else
					timeFormat = apElapsed;

				// Show it to the user
				PrintInfo();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the module info button
			////////////////////////////////////////////////////////////////////
			case AP_MODULEINFO_BUTTON:
			{
				// Open the window
				windowSystem->infoWin->OpenWindow();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the mute button
			////////////////////////////////////////////////////////////////////
			case AP_MUTE_BUTTON:
			{
				if (muteBut->Value() == B_CONTROL_OFF)
				{
					windowSystem->SetVolume(windowSystem->playerInfo->GetVolume());
					windowSystem->playerInfo->SetMuteFlag(false);
				}
				else
				{
					windowSystem->SetVolume(0);
					windowSystem->playerInfo->SetMuteFlag(true);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the add button
			////////////////////////////////////////////////////////////////////
			case AP_ADD_BUTTON:
			{
				ShowModuleFilePanel(AP_MOD_ADD_REFS_RECEIVED, IDS_PANEL_ADD);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the remove button
			////////////////////////////////////////////////////////////////////
			case AP_REMOVE_BUTTON:
			{
				// Remove all the selected module items
				modList->RemoveSelectedItems();

				// Update the controls
				UpdateListCount();
				UpdateTimes();
				UpdateTapeDeck();

				// Free any double buffer loaded modules
				windowSystem->FreeExtraModules();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the swap button
			////////////////////////////////////////////////////////////////////
			case AP_SWAP_BUTTON:
			{
				int32 index1, index2;

				// Get the two items to swap
				index1 = modList->CurrentSelection(0);
				index2 = modList->CurrentSelection(1);

				// Swap the items
				modList->SwapItems(index1, index2);

				// Free any double buffer loaded modules
				windowSystem->FreeExtraModules();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the sort button
			////////////////////////////////////////////////////////////////////
			case AP_SORT_BUTTON:
			{
				BPoint menuPoint;
				BMessage *message;
				BMenuItem *menuItem;
				BPopUpMenu *menu;
				PString label;
				char *labelPtr;

				// Build the pop-up menu to show
				menu = new BPopUpMenu(NULL, false);

				// Add the menu items
				message = new BMessage(AP_SORTMENU_SORT_AZ);
				label.LoadString(res, IDS_SORTMENU_SORT_AZ);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				message = new BMessage(AP_SORTMENU_SORT_ZA);
				label.LoadString(res, IDS_SORTMENU_SORT_ZA);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				message = new BMessage(AP_SORTMENU_SHUFFLE);
				label.LoadString(res, IDS_SORTMENU_SHUFFLE);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				// Set the window as the target for item messages
				menu->SetTargetForItems(this);

				// We want the menu to destroy itself when done
				menu->SetAsyncAutoDestruct(true);

				// Calculate where to show the menu
				menuPoint = sortBut->LeftTop();
				sortBut->ConvertToScreen(&menuPoint);
				menuPoint.x += HSPACE;
				menuPoint.y += VSPACE;

				// Show the menu
				menu->Go(menuPoint, true, true, true);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the up button
			////////////////////////////////////////////////////////////////////
			case AP_UP_BUTTON:
			{
				uint32 keyMod;

				// Get the keyboard modifiers
				keyMod = modifiers();

				// Move the items
				if (keyMod & B_SHIFT_KEY)
					modList->MoveSelectedItemsToTop();
				else
					modList->MoveSelectedItemsUp();

				// Make sure the selected item can be seen
				modList->ScrollToSelection();

				// Free any double buffer loaded modules
				windowSystem->FreeExtraModules();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the down button
			////////////////////////////////////////////////////////////////////
			case AP_DOWN_BUTTON:
			{
				uint32 keyMod;

				// Get the keyboard modifiers
				keyMod = modifiers();

				// Move the items
				if (keyMod & B_SHIFT_KEY)
					modList->MoveSelectedItemsToBottom();
				else
					modList->MoveSelectedItemsDown();

				// Make sure the selected item can be seen
				modList->ScrollToSelection();

				// Free any double buffer loaded modules
				windowSystem->FreeExtraModules();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the select button
			////////////////////////////////////////////////////////////////////
			case AP_SELECT_BUTTON:
			{
				BPoint menuPoint;
				BMessage *message;
				BMenuItem *menuItem;
				BPopUpMenu *menu;
				PString label;
				char *labelPtr;

				// Build the pop-up menu to show
				menu = new BPopUpMenu(NULL, false);

				// Add the menu items
				message = new BMessage(AP_SELECTMENU_ALL);
				label.LoadString(res, IDS_SELECTMENU_ALL);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				message = new BMessage(AP_SELECTMENU_NONE);
				label.LoadString(res, IDS_SELECTMENU_NONE);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				// Set the window as the target for item messages
				menu->SetTargetForItems(this);

				// We want the menu to destroy itself when done
				menu->SetAsyncAutoDestruct(true);

				// Calculate where to show the menu
				menuPoint = selectBut->LeftTop();
				selectBut->ConvertToScreen(&menuPoint);
				menuPoint.x += HSPACE;
				menuPoint.y += VSPACE;

				// Show the menu
				menu->Go(menuPoint, true, true, true);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the disk button
			////////////////////////////////////////////////////////////////////
			case AP_DISK_BUTTON:
			{
				BPoint menuPoint;
				BMessage *message;
				BMenuItem *menuItem;
				BPopUpMenu *menu;
				PString label;
				char *labelPtr;

				// Build the pop-up menu to show
				menu = new BPopUpMenu(NULL, false);

				// Add the menu items
				message = new BMessage(AP_DISKMENU_LOAD);
				label.LoadString(res, IDS_DISKMENU_LOAD);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				message = new BMessage(AP_DISKMENU_APPEND);
				label.LoadString(res, IDS_DISKMENU_APPEND);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				message = new BMessage(AP_DISKMENU_SAVE);
				label.LoadString(res, IDS_DISKMENU_SAVE);
				menuItem = new BMenuItem((labelPtr = label.GetString()), message);
				menu->AddItem(menuItem);
				label.FreeBuffer(labelPtr);

				// Set the window as the target for item messages
				menu->SetTargetForItems(this);

				// We want the menu to destroy itself when done
				menu->SetAsyncAutoDestruct(true);

				// Calculate where to show the menu
				menuPoint = diskBut->LeftTop();
				diskBut->ConvertToScreen(&menuPoint);
				menuPoint.x += HSPACE;
				menuPoint.y += VSPACE;

				// Show the menu
				menu->Go(menuPoint, true, true, true);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Ascending sort has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_SORTMENU_SORT_AZ:
			{
				// Sort the list
				modList->SortList(false);

				// Free any double buffer loaded modules
				windowSystem->FreeExtraModules();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Descending sort has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_SORTMENU_SORT_ZA:
			{
				// Sort the list
				modList->SortList(true);

				// Free any double buffer loaded modules
				windowSystem->FreeExtraModules();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Shuffle has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_SORTMENU_SHUFFLE:
			{
				// Shuffle the list
				modList->Shuffle();

				// If no module is playing, load the first one
				if ((playItem == NULL) && (modList->CountItems() > 0))
					windowSystem->LoadAndPlayModule(0);
				else
					windowSystem->FreeExtraModules();

				break;
			}

			////////////////////////////////////////////////////////////////////
			// Select all has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_SELECTMENU_ALL:
			{
				modList->Select(0, modList->CountItems() - 1, true);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Deselect all has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_SELECTMENU_NONE:
			{
				modList->DeselectAll();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Load list has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_DISKMENU_LOAD:
			{
				ShowAPMLFilePanel(AP_APML_LOAD_REFS_RECEIVED);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Append list has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_DISKMENU_APPEND:
			{
				ShowAPMLFilePanel(AP_APML_APPEND_REFS_RECEIVED);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Save list has been choosen from the popup menu
			////////////////////////////////////////////////////////////////////
			case AP_DISKMENU_SAVE:
			{
				// Show the APML file panel
				ShowSaveAPMLFilePanel();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the play button
			////////////////////////////////////////////////////////////////////
			case AP_PLAY_BUTTON:
			{
				ShowModuleFilePanel(AP_MOD_PLAY_REFS_RECEIVED, IDS_PANEL_OPEN);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the previous module button
			////////////////////////////////////////////////////////////////////
			case AP_PREVIOUSMOD_BUTTON:
			{
				uint16 curSong;
				int32 newIndex, count;

				if (playItem != NULL)
				{
					// Did the song played for more than 2 seconds?
					if (timeOccurred.GetTotalSeconds() > 2)
					{
						// Yup, start the module over again
						curSong = windowSystem->playerInfo->GetCurrentSong();
						StartSong(curSong);
					}
					else
					{
						// Load previous module
						newIndex = modList->IndexOf(playItem) - 1;
						count    = modList->CountItems();

						if (newIndex < 0)
						{
							newIndex = count - 1;
							if (newIndex == 0)
							{
								uint16 curSong;

								curSong = windowSystem->playerInfo->GetCurrentSong();
								StartSong(curSong);
								break;
							}
						}

						// Stop playing the module
						StopAndFreeModule();

						// Load and play the new module
						windowSystem->LoadAndPlayModule(newIndex);
					}
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the next module button
			////////////////////////////////////////////////////////////////////
			case AP_NEXTMOD_BUTTON:
			{
				int32 newIndex, count;

				if (playItem != NULL)
				{
					// Load next module
					newIndex = modList->IndexOf(playItem) + 1;
					count    = modList->CountItems();

					if (newIndex == count)
						newIndex = 0;

					// Stop playing the module
					StopAndFreeModule();

					// Load and play the new module
					windowSystem->LoadAndPlayModule(newIndex);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the previous song button
			////////////////////////////////////////////////////////////////////
			case AP_PREVIOUSSONG_BUTTON:
			{
				uint16 curSong;

				ASSERT(playItem != NULL);

				// Get current playing song
				curSong = windowSystem->playerInfo->GetCurrentSong();
				ASSERT(curSong != 0);

				// Start the new song
				StartSong(curSong - 1);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the next song button
			////////////////////////////////////////////////////////////////////
			case AP_NEXTSONG_BUTTON:
			{
				uint16 curSong, maxSong;

				ASSERT(playItem != NULL);

				// Get current playing song and maximum song number
				curSong = windowSystem->playerInfo->GetCurrentSong();
				maxSong = windowSystem->playerInfo->GetMaxSongNumber();
				ASSERT(curSong < maxSong - 1);

				// Start the new song
				StartSong(curSong + 1);
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the rewind button
			////////////////////////////////////////////////////////////////////
			case AP_REWIND_BUTTON:
			{
				int16 songPos;

				if (playItem != NULL)
				{
					songPos = windowSystem->playerInfo->GetSongPosition();

					// Only load the previous module if song position is 0 and
					// the module loop isn't on
					if (songPos == 0)
					{
						if (!IsLoopOn())
						{
							BMessage newMsg(AP_PREVIOUSMOD_BUTTON);
							PostMessage(&newMsg, this);
						}

						songPos = 1;
					}

					// Change the position
					SetPosition(songPos - 1);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the forward button
			////////////////////////////////////////////////////////////////////
			case AP_FORWARD_BUTTON:
			{
				int16 songPos, songLength;

				if (playItem != NULL)
				{
					songLength = windowSystem->playerInfo->GetSongLength();
					songPos    = windowSystem->playerInfo->GetSongPosition();

					// Only load the next module if song position is the last and
					// the module loop isn't on
					if (songPos == (songLength - 1))
					{
						if (!IsLoopOn())
						{
							BMessage newMsg(AP_NEXTMOD_BUTTON);
							PostMessage(&newMsg, this);
						}

						songPos = -1;
					}

					// Change the position
					SetPosition(songPos + 1);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the eject button
			////////////////////////////////////////////////////////////////////
			case AP_EJECT_BUTTON:
			{
				if (playItem != NULL)
				{
					// Stop and free the playing module
					StopAndFreeModule();
					windowSystem->FreeAllModules();
				}
				else
				{
					// Clear the module list
					EmptyList(false);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the pause button
			////////////////////////////////////////////////////////////////////
			case AP_PAUSE_BUTTON:
			{
				ASSERT(playItem != NULL);

				if (pauseBut->Value() == B_CONTROL_ON)
				{
					windowSystem->PausePlaying();

					// Stop the timers
					timer->StopTimer();

					if (neverEndingTimer != NULL)
						neverEndingTimer->StopTimer();
				}
				else
				{
					// Start to play again
					windowSystem->ResumePlaying();

					// We add the time to the start time, so there won't be added
					// a lot of seconds to the played time
					timeStart += (PTime::GetNow() - timeStart - timeOccurred);

					// Start the timers again
					timer->StartTimer();

					if (neverEndingTimer != NULL)
					{
						neverEndingTimer->SetElapseValue((neverEndingTimeout - timeOccurred).GetTotalMilliSeconds());
						neverEndingTimer->StartTimer();
					}
				}
				break;
			}

			////////////////////////////////////////////////////////////////////
			// User pressed on the sample button
			////////////////////////////////////////////////////////////////////
			case AP_SAMPLE_BUTTON:
			{
				// Open the window
				windowSystem->sampWin->OpenWindow();
				break;
			}

			////////////////////////////////////////////////////////////////////
			// Default handler
			////////////////////////////////////////////////////////////////////
			default:
			{
				BWindow::MessageReceived(msg);
				break;
			}
		}
	}
	catch(...)
	{
		;
	}
}



/******************************************************************************/
/* CalcMinSize() will calculate the minimum size the window can have.         */
/*                                                                            */
/* Output: Is a BPoint where the x is the minimum width and y is the minimum  */
/*         height.                                                            */
/******************************************************************************/
BPoint APWindowMain::CalcMinSize(void)
{
	BPoint size;
	float width1, width2;
	float textWidth1, textWidth2;
	float menuHeight;

	// Calculate height of the menu bar
	menuHeight = menuBar->Bounds().bottom + 1.0f;

	textWidth1 = topView->StringWidth("88:88:88/888:88:88");
	textWidth2 = topView->StringWidth("88888/88888");
	width1 = (PICHSIZE + HSPACE - 1.0f) * 11.0f + HSPACE * 14.0f;
	width2 = (PICHSIZE + HSPACE - 1.0f) * 8.0f + HSPACE * 14.0f + textWidth1 + textWidth2;
	size.x = max(width1, width2) + HSPACE * 2.0f;
	size.y = menuHeight + (fontHeight + VSPACE * 4.0f) * 4.0f + VSPACE * 2.0f + (fontHeight + 1.0f) * 6.0f - 1.0f;

	return (size);
}



/******************************************************************************/
/* SetPosAndSize() will calculate the positions and sizes for all views.      */
/******************************************************************************/
void APWindowMain::SetPosAndSize(void)
{
	BRect winRect;
	float x, y, w, h;
	float listButStart;

	// Calculate the windows size we can draw in
	winRect = topView->Bounds();
	winRect.InsetBy(HSPACE, VSPACE);

	// Change the size of the information box
	w = winRect.right - winRect.left - (HSPACE * 2.0f + PICHSIZE - 1.0f);
	h = fontHeight + VSPACE * 2.0f - 1.0f;

	// Place the information box and string
	infoBox->MoveTo(winRect.left, winRect.top);
	infoBox->ResizeTo(w, h);

	infoView->MoveTo(winRect.left + HSPACE, winRect.top + VSPACE);
	infoView->ResizeTo(w - HSPACE * 2.0f, fontHeight);

	// Place the module info button
	infoBut->MoveTo(winRect.left + w + HSPACE, winRect.top);
	infoBut->ResizeTo(PICHSIZE + HSPACE - 1.0f, h);

	// Place the module list
	listButStart = winRect.bottom - h * 3.0f - VSPACE * 4.0f - VSPACE / 2.0f * 2.0f - 1.0f;
	y = winRect.top + h + VSPACE;
	x = winRect.left + B_V_SCROLL_BAR_WIDTH + HSPACE;
	modScrollView->MoveTo(x, y);
	modScrollView->ResizeTo(winRect.right - x, listButStart - VSPACE - y - 1.0f);

	// Place the volume slider and button
	muteBut->MoveTo(winRect.left, y);
	muteBut->ResizeTo(B_V_SCROLL_BAR_WIDTH - 1.0f, h);

	y += (h + VSPACE);
	volSlider->MoveTo(winRect.left, y);
	volSlider->ResizeTo(B_V_SCROLL_BAR_WIDTH - 1.0f, listButStart - VSPACE - y - 1.0f);

	// Place the list buttons
	w = (PICHSIZE + HSPACE - 1.0f) * 8.0f + HSPACE * 9.0f;
	listBox->MoveTo(winRect.left, listButStart);
	listBox->ResizeTo(w, h + VSPACE * 2.0f);

	x = winRect.left + HSPACE;
	y = listButStart + VSPACE;
	w = PICHSIZE + HSPACE - 1.0f;
	addBut->MoveTo(x, y);
	addBut->ResizeTo(w, h);

	removeBut->MoveTo(x + w + HSPACE, y);
	removeBut->ResizeTo(w, h);

	swapBut->MoveTo(x + (w + HSPACE) * 2.0f, y);
	swapBut->ResizeTo(w, h);

	sortBut->MoveTo(x + (w + HSPACE) * 3.0f, y);
	sortBut->ResizeTo(w, h);

	upBut->MoveTo(x + (w + HSPACE) * 4.0f, y);
	upBut->ResizeTo(w, h);

	downBut->MoveTo(x + (w + HSPACE) * 5.0f, y);
	downBut->ResizeTo(w, h);

	selectBut->MoveTo(x + (w + HSPACE) * 6.0f, y);
	selectBut->ResizeTo(w, h);

	diskBut->MoveTo(x + (w + HSPACE) * 7.0f, y);
	diskBut->ResizeTo(w, h);

	// Place the time box and strings
	x  = winRect.left + (PICHSIZE + HSPACE - 1.0f) * 8.0f + HSPACE * 9.0f + HSPACE;
	y -= VSPACE;
	timeBox->MoveTo(x, y);
	timeBox->ResizeTo(winRect.right - x, h + VSPACE * 2.0f);

	y = (h + VSPACE * 2.0f - fontHeight) / 2.0f + y + 1.0f;
	w = timeView->StringWidth("88:88:88/888:88:88");
	timeView->MoveTo(x + HSPACE, y);
	timeView->ResizeTo(w, fontHeight);

	totalView->MoveTo(x + w + HSPACE * 2.0f + 1.0f, y);
	totalView->ResizeTo(winRect.right - x - w - HSPACE * 3.0f + 1.0f, fontHeight);

	// Place the position slider
	posSlider->MoveTo(winRect.left, listButStart + h + VSPACE * 2.0f + VSPACE / 2.0f * 2.0f - 1.0f);
	posSlider->ResizeTo(winRect.right - winRect.left, h);

	// Place the tape deck box and buttons
	w = (PICHSIZE + HSPACE - 1.0f) * 9.0f + HSPACE * 10.0f;
	y = winRect.bottom - h - VSPACE * 2.0;
	tapeBox->MoveTo(winRect.left, y);
	tapeBox->ResizeTo(w, h + VSPACE * 2.0f);

	x  = winRect.left + HSPACE;
	y += VSPACE;
	w  = PICHSIZE + HSPACE - 1.0f;
	prevModBut->MoveTo(x, y);
	prevModBut->ResizeTo(w, h);

	prevSongBut->MoveTo(x + w + HSPACE, y);
	prevSongBut->ResizeTo(w, h);

	rewBut->MoveTo(x + (w + HSPACE) * 2.0f, y);
	rewBut->ResizeTo(w, h);

	playBut->MoveTo(x + (w + HSPACE) * 3.0f, y);
	playBut->ResizeTo(w, h);

	fwdBut->MoveTo(x + (w + HSPACE) * 4.0f, y);
	fwdBut->ResizeTo(w, h);

	nextSongBut->MoveTo(x + (w + HSPACE) * 5.0f, y);
	nextSongBut->ResizeTo(w, h);

	nextModBut->MoveTo(x + (w + HSPACE) * 6.0f, y);
	nextModBut->ResizeTo(w, h);

	ejectBut->MoveTo(x + (w + HSPACE) * 7.0f, y);
	ejectBut->ResizeTo(w, h);

	pauseBut->MoveTo(x + (w + HSPACE) * 8.0f, y);
	pauseBut->ResizeTo(w, h);

	// Place the module loop and sample buttons
	w = (PICHSIZE + HSPACE - 1.0f) * 2.0f + HSPACE * 3.0f;
	x = winRect.right - w;

	loopSampleBox->MoveTo(x, y - VSPACE);
	loopSampleBox->ResizeTo(w, h + VSPACE * 2.0f);

	x += HSPACE;
	w  = PICHSIZE + HSPACE - 1.0f;
	modLoopBut->MoveTo(x, y);
	modLoopBut->ResizeTo(w, h);

	sampleBut->MoveTo(x + w + HSPACE, y);
	sampleBut->ResizeTo(w, h);
}



/******************************************************************************/
/* ParseKey() will parse the key given and react on it.                       */
/*                                                                            */
/* Input:  "key" is the key the user has pressed in the window.               */
/*         "modifiers" is the modifiers that was down.                        */
/*         "code" is the key code.                                            */
/*                                                                            */
/* Output: True if the key was parsed, false if not.                          */
/******************************************************************************/
bool APWindowMain::ParseKey(char key, int32 modifiers, int32 code)
{
	// Mask out lock keys
	modifiers &= ~(B_CAPS_LOCK | B_SCROLL_LOCK);

	if (modifiers & B_NUM_LOCK)
	{
		// Keys on the numpad keyboard and numlock are down
		switch (code)
		{
			//
			// Sub-song 1
			//
			case 88:
			{
				SwitchSubSong('1');
				return (true);
			}

			//
			// Sub-song 2
			//
			case 89:
			{
				SwitchSubSong('2');
				return (true);
			}

			//
			// Sub-song 3
			//
			case 90:
			{
				SwitchSubSong('3');
				return (true);
			}

			//
			// Sub-song 4
			//
			case 72:
			{
				SwitchSubSong('4');
				return (true);
			}

			//
			// Sub-song 5
			//
			case 73:
			{
				SwitchSubSong('5');
				return (true);
			}

			//
			// Sub-song 6
			//
			case 74:
			{
				SwitchSubSong('6');
				return (true);
			}

			//
			// Sub-song 7
			//
			case 55:
			{
				SwitchSubSong('7');
				return (true);
			}

			//
			// Sub-song 8
			//
			case 56:
			{
				SwitchSubSong('8');
				return (true);
			}

			//
			// Sub-song 9
			//
			case 57:
			{
				SwitchSubSong('9');
				return (true);
			}

			//
			// Sub-song 10
			//
			case 100:
			{
				SwitchSubSong('0');
				return (true);
			}
		}
	}

	if ((modifiers == 0) || (modifiers == B_NUM_LOCK))
	{
		// Keys without any modifiers
		switch (key)
		{
			//
			// Function keys
			//
			case B_FUNCTION_KEY:
			{
				// F12 - Play a random module
				if (code == B_F12_KEY)
				{
					int32 total, index;

					// Get the number of items in the list
					total = modList->CountItems();

					// Do only continue if we have more than one
					// module in the list
					if (total > 1)
					{
						// Find a random module until we found
						// one that is not the playing one
						do
						{
							index = PSystem::Random(total - 1);
						}
						while ((index == GetPlayItem()) || (randomList.HasItem(index)));

						// Free any playing modules
						StopAndFreeModule();

						// Now load the module
						windowSystem->LoadAndPlayModule(index);
					}

					return (true);
				}
			}

			//
			// Space - Pause
			//
			case B_SPACE:
			{
				if (pauseBut->IsEnabled())
				{
					// Invert the state of the button
					pauseBut->SetValue(pauseBut->Value() == B_CONTROL_OFF ? B_CONTROL_ON : B_CONTROL_OFF);
					pauseBut->Invoke();
				}

				return (true);
			}

			//
			// Backspace - Module loop
			//
			case B_BACKSPACE:
			{
				// Invert the state of the button
				modLoopBut->SetValue(modLoopBut->Value() == B_CONTROL_OFF ? B_CONTROL_ON : B_CONTROL_OFF);
				modLoopBut->Invoke();
				return (true);
			}

			//
			// Numbers - Play new sub-song
			//
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0':
			{
				SwitchSubSong(key);
				return (true);
			}

			//
			// + - Add 10 to the sub-song selector
			//
			case '+':
			{
				uint16 maxSong;

				// Get the maximum song number available
				maxSong = windowSystem->playerInfo->GetMaxSongNumber();

				// Check for out of bounds
				if ((subSongMultiply + 10) < maxSong)
					subSongMultiply += 10;

				return (true);
			}

			//
			// - - Subtract 10 from the sub-song selector
			//
			case '-':
			{
				// Check for out of bounds
				if (subSongMultiply != 0)
					subSongMultiply -= 10;

				return (true);
			}
		}

		switch (code)
		{
			//
			// Delete - Eject
			//
			case 52:
			case 101:
			{
				ejectBut->Invoke();
				return (true);
			}

			//
			// Insert - Play
			//
			case 31:
			case 100:
			{
				playBut->Invoke();
				return (true);
			}

			//
			// Left arrow - Rewind
			//
			case 97:
			case 72:
			{
				rewBut->Invoke();
				return (true);
			}

			//
			// Right arrow - Fast forward
			//
			case 99:
			case 74:
			{
				fwdBut->Invoke();
				return (true);
			}
		}
	}
	else if ((modifiers & ~(B_LEFT_SHIFT_KEY | B_RIGHT_SHIFT_KEY | B_NUM_LOCK)) == B_SHIFT_KEY)
	{
		// Keys with shift pressed down
		switch (key)
		{
			//
			// Space - Mute
			//
			case B_SPACE:
			{
				// Invert the state of the button
				muteBut->SetValue(muteBut->Value() == B_CONTROL_OFF ? B_CONTROL_ON : B_CONTROL_OFF);
				muteBut->Invoke();
				return (true);
			}

			//
			// Left arrow - Load previous module
			//
			case B_LEFT_ARROW:
			{
				if (modList->CountItems() > 1)
					prevModBut->Invoke();

				return (true);
			}

			//
			// Right arrow - Load next module
			//
			case B_RIGHT_ARROW:
			{
				if (modList->CountItems() > 1)
					nextModBut->Invoke();

				return (true);
			}
		}
	}

	return (false);
}



/******************************************************************************/
/* CreateCursor() create a single mouse cursor.                               */
/*                                                                            */
/* Input:  "cursor" is the resource ID to the cursor.                         */
/*         "merge" is true, if you want to merge with the hand cursor.        */
/*                                                                            */
/* Output: A pointer to a BCursor object.                                     */
/******************************************************************************/
BCursor *APWindowMain::CreateCursor(int32 id, bool merge)
{
	BCursor *object;
	uint8 cursor[4 + 2 * 2 * 16];
	uint8 resCursor[4 + 2 * 2 * 16];
	int32 i;

	if (merge)
	{
		// First copy the default hand image
		memcpy(cursor, B_HAND_CURSOR, 4 + 2 * 2 * 16);

		// Get the cursor to merge with
		res->GetItem(P_RES_CURSOR, id, resCursor, sizeof(resCursor));

		// Merge the cursor with the hand
		for (i = 0; i < (2 * 16); i++)
		{
			cursor[4 + i] = (cursor[4 + i] & ~resCursor[4 + 2 * 16 + i]) | resCursor[4 + i];
			cursor[4 + 2 * 16 + i] |= resCursor[4 + 2 * 16 + i];
		}
	}
	else
	{
		// Just get the cursor from the resources
		res->GetItem(P_RES_CURSOR, id, cursor, sizeof(cursor));
	}

	// Create the BCursor object
	object = new BCursor(cursor);
	if (object == NULL)
		throw PMemoryException();

	return (object);
}



/******************************************************************************/
/* AddAddOnListToMenu() will add all the add-ons in the list given under a    */
/*      specifive menu.                                                       */
/*                                                                            */
/* Input:  "list" is a reference to the list with the add-ons to add.         */
/*         "menu" is a pointer to the menu you want to add the add-ons in.    */
/*         "menuMessage" is the message you want to be sent when the menu     */
/*         items are selected.                                                */
/******************************************************************************/
void APWindowMain::AddAddOnListToMenu(APList<APAddOnInformation *> &list, BMenu *menu, uint32 menuMessage)
{
	PString menuStr;
	APAddOnInformation *addOnInfo;
	char *nameStr;
	int32 i, count;
	int32 j, menuCount;
	BMenu *subMenu;
	BMenuItem *item;
	BMessage *message;
	bool createSubMenu;

	// Get the menu string
	if (menuMessage == AP_MENU_ADDONDISPLAYVIEW)
		menuStr.LoadString(res, IDS_MENU_DISPLAY);
	else
		menuStr.LoadString(res, IDS_MENU_SETTINGS);

	// Lock the list
	list.LockList();

	try
	{
		// Traverse the list of add-ons
		count = list.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get the add-on information
			addOnInfo = list.GetItem(i);

			// Do this add-on got any display or settings window?
			if ((menuMessage == AP_MENU_ADDONDISPLAYVIEW) && !addOnInfo->display)
				continue;

			if ((menuMessage == AP_MENU_ADDONCONFIGVIEW) && !addOnInfo->settings)
				continue;

			createSubMenu = true;

			// Search through the menu to find the right place to insert the new
			// menu at
			menuCount = menu->CountItems();

			for (j = 0; j < menuCount; j++)
			{
				item = menu->ItemAt(j);
				if (item != NULL)
				{
					if (addOnInfo->name == item->Label())
					{
						createSubMenu = false;
						break;
					}

					if (addOnInfo->name < item->Label())
						break;
				}
			}

			// Get name pointer
			nameStr = addOnInfo->name.GetString();

			// Should we create the sub-menu?
			if (createSubMenu)
			{
				// Yap, do it
				subMenu = new BMenu(nameStr);
				if (subMenu == NULL)
				{
					addOnInfo->name.FreeBuffer(nameStr);
					throw PMemoryException();
				}

				menu->AddItem(subMenu, (j == menuCount ? menuCount : j == 0 ? 0 : j - 1));
			}
			else
			{
				// The sub-menu does already exists, so get the pointer to it
				subMenu = menu->SubmenuAt(j);
			}

			// Now create the menu message to be sent
			message = new BMessage(menuMessage);
			if (message == NULL)
			{
				addOnInfo->name.FreeBuffer(nameStr);
				throw PMemoryException();
			}

			message->AddString("addOn", nameStr);
			addOnInfo->name.FreeBuffer(nameStr);

			// And add the menu item to the sub-menu
			item = new BMenuItem((nameStr = menuStr.GetString()), message);
			menuStr.FreeBuffer(nameStr);

			if (item == NULL)
				throw PMemoryException();

			subMenu->AddItem(item);
		}
	}
	catch(...)
	{
		list.UnlockList();
		throw;
	}

	// Unlock list again
	list.UnlockList();
}



/******************************************************************************/
/* StartTimers() will start all the timers.                                   */
/******************************************************************************/
void APWindowMain::StartTimers(void)
{
	// Start the playing timer
	timeStart.SetToNow();
	timeOccurred.SetTimeSpan(0);
	timer->StartTimer();

	// Start the never ending timer if needed
	if (!(windowSystem->playerInfo->CanChangePosition()))
	{
		if (windowSystem->useSettings->GetStringEntryValue("Options", "NeverEnding").CompareNoCase("Yes") == 0)
		{
			neverEndingTimeout.SetTimeSpan(windowSystem->useSettings->GetIntEntryValue("Options", "NeverEndingTimeout") * 1000);
			neverEndingUpdate = true;

			neverEndingTimer = new PTimer(this, AP_TIME_NEVERENDING);
			neverEndingTimer->SetElapseValue(neverEndingTimeout.GetTotalMilliSeconds());
			neverEndingTimer->StartTimer();
		}
	}
}



/******************************************************************************/
/* StopTimers() will stop all the timers.                                     */
/******************************************************************************/
void APWindowMain::StopTimers(void)
{
	// Stop the timers
	timer->StopTimer();

	if (neverEndingTimer != NULL)
	{
		neverEndingTimer->StopTimer();
		delete neverEndingTimer;
		neverEndingTimer  = NULL;
		neverEndingUpdate = false;
	}
}



/******************************************************************************/
/* StartSong() will start to play the song given.                             */
/*                                                                            */
/* Input:  "newSong" is the song number to play.                              */
/******************************************************************************/
void APWindowMain::StartSong(uint16 newSong)
{
	// First stop any timers
	StopTimers();

	// Now tell the loader to switch the song
	windowSystem->StartSong(newSong);
}



/******************************************************************************/
/* SwitchSubSong() will switch to another sub-song based on the key given.    */
/*                                                                            */
/* Input:  "key" is the song number to play in ascii ('0'-'9').               */
/******************************************************************************/
void APWindowMain::SwitchSubSong(char key)
{
	if (playItem != NULL)
	{
		uint16 maxSong, song;

		// Calculate the sub-song number to play
		if (key == '0')
			key += 10;

		song = key - '1' + subSongMultiply;

		// Get the maximum song number available
		maxSong = windowSystem->playerInfo->GetMaxSongNumber();

		// Can we play the sub-song selected?
		if (song < maxSong)
		{
			// Yep, play the new sub-song
			StartSong(song);
		}
	}
}



/******************************************************************************/
/* UpdateTapeDeck() will update all the tape deck buttons.                    */
/******************************************************************************/
void APWindowMain::UpdateTapeDeck(void)
{
	int32 count;
	uint16 curSong;

	// Get the number of items in the list
	count = modList->CountItems();

	// If the module list is empty, disable the eject button
	if (count == 0)
		ejectBut->SetEnabled(false);
	else
		ejectBut->SetEnabled(true);

	// If no module is playing, disable the pause button
	if (playItem == NULL)
		pauseBut->SetEnabled(false);
	else
		pauseBut->SetEnabled(true);

	// If only one item is in the list or none if playing, disable
	// the previous and next module buttons
	if ((playItem == NULL) || (count <= 1))
	{
		prevModBut->SetEnabled(false);
		nextModBut->SetEnabled(false);
	}
	else
	{
		prevModBut->SetEnabled(true);
		nextModBut->SetEnabled(true);
	}

	// If playing subsong 1, disable the previous song button
	curSong = windowSystem->playerInfo->GetCurrentSong();
	if (curSong == 0)
		prevSongBut->SetEnabled(false);
	else
		prevSongBut->SetEnabled(true);

	// If playing the last subsong, disable the next song button
	if ((playItem != NULL) && (curSong != windowSystem->playerInfo->GetMaxSongNumber() - 1))
		nextSongBut->SetEnabled(true);
	else
		nextSongBut->SetEnabled(false);

	// If no module is playing or the player doesn't support position change,
	// disable the rewind and forward buttons + the position slider
	if ((playItem == NULL) || (!windowSystem->playerInfo->CanChangePosition()))
	{
		rewBut->SetEnabled(false);
		fwdBut->SetEnabled(false);
		posSlider->SetEnabled(false);
	}
	else
	{
		rewBut->SetEnabled(true);
		fwdBut->SetEnabled(true);
		posSlider->SetEnabled(true);
	}
}



/******************************************************************************/
/* UpdateTimes() will update the list times.                                  */
/******************************************************************************/
void APWindowMain::UpdateTimes(void)
{
	PTimeSpan tempTime;
	PString selStr, listStr, winStr;
	char *winPtr;
	int32 listIndex, index;
	APWindowMainListItem *item;

	// Calculate the total time for all the selected items
	selectedTime.SetTimeSpan(0);
	index = 0;
	while ((listIndex = modList->CurrentSelection(index++)) >= 0)
	{
		// Get the selected item
		item = (APWindowMainListItem *)modList->ItemAt(listIndex);

		// Add the time to the total
		selectedTime += item->GetTime();
	}

	// Build the selected time string
	tempTime.SetTimeSpan((selectedTime.GetTotalMilliSeconds() + 500) / 1000 * 1000);
	if (tempTime.GetTotalHours() > 0)
		selStr.Format("%Ld:%02d:%02d", tempTime.GetTotalHours(), tempTime.GetMinutes(), tempTime.GetSeconds());
	else
		selStr.Format("%d:%02d", tempTime.GetMinutes(), tempTime.GetSeconds());

	// And build the list time string
	tempTime.SetTimeSpan((listTime.GetTotalMilliSeconds() + 500) / 1000 * 1000);
	if (tempTime.GetTotalHours() > 0)
		listStr.Format("%Ld:%02d:%02d", tempTime.GetTotalHours(), tempTime.GetMinutes(), tempTime.GetSeconds());
	else
		listStr.Format("%d:%02d", tempTime.GetMinutes(), tempTime.GetSeconds());

	// And update the text control
	winStr = selStr + "/" + listStr;
	timeView->SetText((winPtr = winStr.GetString()));
	winStr.FreeBuffer(winPtr);
}



/******************************************************************************/
/* UpdateListCount() will update the list count control.                      */
/******************************************************************************/
void APWindowMain::UpdateListCount(void)
{
	PString numStr;
	char *numPtr;
	int32 selected;

	// Count number of selected items
	selected = 0;
	while (modList->CurrentSelection(selected++) >= 0);

	// Update the "number of files" view
	numStr.Format("%d/%d", selected - 1, modList->CountItems());
	totalView->SetText((numPtr = numStr.GetString()));
	numStr.FreeBuffer(numPtr);
}



/******************************************************************************/
/* UpdateListControls() will update the list controls.                        */
/******************************************************************************/
void APWindowMain::UpdateListControls(void)
{
	if (modList->CurrentSelection() == -1)
	{
		// No items are selected
		removeBut->SetEnabled(false);
		swapBut->SetEnabled(false);
		upBut->SetEnabled(false);
		downBut->SetEnabled(false);
	}
	else
	{
		// Some items are selected
		removeBut->SetEnabled(true);
		upBut->SetEnabled(true);
		downBut->SetEnabled(true);

		// Is two and only two items selected?
		if ((modList->CurrentSelection(1) != -1) && (modList->CurrentSelection(2) == -1))
		{
			// Enable the swap button
			swapBut->SetEnabled(true);
		}
		else
		{
			// Disable the swap button
			swapBut->SetEnabled(false);
		}
	}
}



/******************************************************************************/
/* PrintInfo() print the current information in the info bar.                 */
/******************************************************************************/
void APWindowMain::PrintInfo(void)
{
	PString posStr, subStr, timeStr, infoStr, tempStr;
	char *infoPtr;
	int16 songLength, songPos, percent;
	uint16 currentSong, maxSong;

	if (playItem != NULL)
	{
		// Create song position string
		songLength = windowSystem->playerInfo->GetSongLength();
		songPos    = windowSystem->playerInfo->GetSongPosition();

		if (songLength > 0)
		{
			// We got a length, so we can create the position string
			percent = songPos * 100 / songLength;
			posStr.Format(res, IDS_POSITION, songPos, songLength, percent);

			// Set the position slider
			if ((posSliderUpdate) && (posSlider->Value() != percent))
				posSlider->SetValue(percent);
		}
		else
		{
			// Set the position string to n/a
			posStr.LoadString(res, IDS_NOPOSITION);

			// If never ending timeout is on, we need to update the position slider
			if (neverEndingUpdate)
			{
				// Calculate the percent
				percent = min(timeOccurred.GetTotalMilliSeconds() * 100 / neverEndingTimeout.GetTotalMilliSeconds(), 100);

				// Set the position slider
				if (posSlider->Value() != percent)
					posSlider->SetValue(percent);
			}
		}

		// Create subsongs string
		currentSong = windowSystem->playerInfo->GetCurrentSong() + 1;
		maxSong     = windowSystem->playerInfo->GetMaxSongNumber();

		if (maxSong == 0)
		{
			subStr.LoadString(res, IDS_NOSUBSONGS);
			timeStr.LoadString(res, timeFormat == apElapsed ? IDS_NOTIME : IDS_NEGATIVE_NOTIME);
		}
		else
		{
			subStr.Format(res, IDS_SUBSONGS, currentSong, maxSong);

			// Format the time string
			if (timeFormat == apElapsed)
			{
				tempStr.LoadString(res, IDS_TIME);
				timeStr = timeOccurred.Format(tempStr);
			}
			else
			{
				PTimeSpan tempTime;

				// Calculate the remaining time
				tempTime = songTotalTime - timeOccurred;

				// Check to see if we got a negative number
				if (tempTime.GetTotalMilliSeconds() < 0)
					tempTime.SetTimeSpan(0);

				// Format the time string
				tempStr.LoadString(res, IDS_NEGATIVE_TIME);
				timeStr = tempTime.Format(tempStr);
			}
		}
	}
	else
	{
		posStr.LoadString(res, IDS_NOPOSITION);
		subStr.LoadString(res, IDS_NOSUBSONGS);
		timeStr.LoadString(res, timeFormat == apElapsed ? IDS_NOTIME : IDS_NEGATIVE_NOTIME);
	}

	infoStr.Format_S3("%s %s %s", posStr, subStr, timeStr);
	infoView->SetText((infoPtr = infoStr.GetString()));
	infoStr.FreeBuffer(infoPtr);
}



/******************************************************************************/
/* ShowNormalTitle() will set the window titlebar back to normal.             */
/******************************************************************************/
void APWindowMain::ShowNormalTitle(void)
{
	PString title;
	char *titleStr;

	// Set the window title back to normal
	title.LoadString(res, IDS_MAIN_TITLE);
	SetTitle((titleStr = title.GetString()));
	title.FreeBuffer(titleStr);
}



/******************************************************************************/
/* ShowModuleName() will set the window titlebar to hold the module name.     */
/******************************************************************************/
void APWindowMain::ShowModuleName(void)
{
	PString title, modName;
	char *titleStr;

	// Get window title
	title.LoadString(res, IDS_MAIN_TITLE);

	// Should we change the window title
	if (showName && (playItem != NULL))
	{
		// Find the name of the module
		modName = windowSystem->playerInfo->GetModuleName();

		// Remove any white spaces
		modName.TrimRight();
		modName.TrimLeft();

		if (modName.IsEmpty())
			modName = PDirectory::GetFilePart(windowSystem->playerInfo->GetFileName());

		// Change the window title so it contains the module name
		title = modName + " / " + title;
		SetTitle((titleStr = title.GetString()));
	}
	else
	{
		// Just set the normal title
		SetTitle((titleStr = title.GetString()));
	}

	title.FreeBuffer(titleStr);
}



/******************************************************************************/
/* SetPosition() will change the module position.                             */
/*                                                                            */
/* Input:  "position" is the new position.                                    */
/******************************************************************************/
void APWindowMain::SetPosition(int16 position)
{
	if (playItem != NULL)
	{
		if (position != windowSystem->playerInfo->GetSongPosition())
		{
			// Change the time to the position time
			SetPositionTime(position);

			// Change the position
			windowSystem->SetSongPosition(position);
			prevPosition = position;

			// Show it to the user
			PrintInfo();
		}
	}

	// Slider updates is permitted again
	posSliderUpdate = true;
}



/******************************************************************************/
/* SetPositionTime() will change the time depending on the position given.    */
/*                                                                            */
/* Input:  "position" is the new position.                                    */
/******************************************************************************/
void APWindowMain::SetPositionTime(int16 position)
{
	PTimeSpan newTime;

	// Set the new time
	newTime = windowSystem->playerInfo->GetPositionTime(position);
	if (newTime.GetTotalMilliSeconds() != -1)
	{
		timeStart   -= (newTime - timeOccurred);
		timeOccurred = newTime;
	}
}



/******************************************************************************/
/* ShowModuleFilePanel() will show the file panel where the user can select   */
/*      the modules to put in the module list.                                */
/*                                                                            */
/* Input:  "messageID" is what to put in the what member in the notify        */
/*         message.                                                           */
/*         "buttonID" is the resource ID to the button label.                 */
/******************************************************************************/
void APWindowMain::ShowModuleFilePanel(uint32 messageID, int32 buttonID)
{
	BMessage extraMsg(messageID);
	PString buttonLabel;
	char *pathStr, *labelStr;

	if (modPanel != NULL)
	{
		entry_ref ref;

		// Get the current path
		modPanel->GetPanelDirectory(&ref);

		// Check to see if the current path still exists
		BEntry entry(&ref);
		BPath path(&entry);

		if (!PDirectory::DirectoryExists(path.Path()))
		{
			// It doesn't, delete the panel
			delete modPanel;
			modPanel = NULL;

			delete modMessenger;
			modMessenger = NULL;
		}
	}

	// Allocate the panel if not already allocated
	if (modPanel == NULL)
	{
		// Create the window messenger object
		modMessenger = new BMessenger(this);

		// Create the file panel object
		modPanel = new BFilePanel(B_OPEN_PANEL, modMessenger, NULL, B_FILE_NODE | B_DIRECTORY_NODE);

		PString modPath(windowSystem->useSettings->GetStringEntryValue("Paths", "Modules"));
		if (!modPath.IsEmpty())
		{
			BEntry entry((pathStr = modPath.GetString()));

			if (entry.InitCheck() == B_OK)
				modPanel->SetPanelDirectory(&entry);

			modPath.FreeBuffer(pathStr);
		}
	}

	// Set the message to send
	modPanel->SetMessage(&extraMsg);

	// Change the default button
	buttonLabel.LoadString(res, buttonID);
	modPanel->SetButtonLabel(B_DEFAULT_BUTTON, (labelStr = buttonLabel.GetString()));
	buttonLabel.FreeBuffer(labelStr);

	// Open the file panel
	modPanel->Show();
}



/******************************************************************************/
/* ShowAPMLFilePanel() will show the file panel where the user can select     */
/*      where to load the APML list.                                          */
/*                                                                            */
/* Input:  "messageID" is what to put in the what member in the notify        */
/*         message.                                                           */
/******************************************************************************/
void APWindowMain::ShowAPMLFilePanel(uint32 messageID)
{
	BMessage extraMsg(messageID);
	char *pathStr;

	if (apmlPanel != NULL)
	{
		entry_ref ref;

		// Get the current path
		apmlPanel->GetPanelDirectory(&ref);

		// Check to see if the current path still exists
		BEntry entry(&ref);
		BPath path(&entry);

		if (!PDirectory::DirectoryExists(path.Path()))
		{
			// It doesn't, delete all the panels
			delete apmlPanel;
			apmlPanel = NULL;

			delete apmlMessenger;
			apmlMessenger = NULL;

			delete apmlSavePanel;
			apmlSavePanel = NULL;

			delete apmlSaveMessenger;
			apmlSaveMessenger = NULL;
		}
	}

	// Allocate the panel if not already allocated
	if (apmlPanel == NULL)
	{
		// Create the window messenger object
		apmlMessenger = new BMessenger(this);

		// Create the file panel object
		apmlPanel = new BFilePanel(B_OPEN_PANEL, apmlMessenger, NULL, B_FILE_NODE, false);

		if (apmlSavePanel == NULL)
		{
			PString apmlPath(windowSystem->useSettings->GetStringEntryValue("Paths", "APML"));
			if (!apmlPath.IsEmpty())
			{
				BEntry entry((pathStr = apmlPath.GetString()));

				if (entry.InitCheck() == B_OK)
					apmlPanel->SetPanelDirectory(&entry);

				apmlPath.FreeBuffer(pathStr);
			}

			// Remember the directory
			apmlPanel->GetPanelDirectory(&apmlLastEntry);
		}
		else
		{
			// Change the start directory
			apmlPanel->SetPanelDirectory(&apmlLastEntry);
		}
	}
	else
	{
		// Change the start directory
		apmlPanel->SetPanelDirectory(&apmlLastEntry);
	}

	// Set the message to send
	apmlPanel->SetMessage(&extraMsg);

	// Open the file panel
	apmlPanel->Show();
}



/******************************************************************************/
/* ShowSaveAPMLFilePanel() will show the file panel where the user can select */
/*      where to save the APML list.                                          */
/******************************************************************************/
void APWindowMain::ShowSaveAPMLFilePanel(void)
{
	char *pathStr;

	if (apmlSavePanel != NULL)
	{
		entry_ref ref;

		// Get the current path
		apmlSavePanel->GetPanelDirectory(&ref);

		// Check to see if the current path still exists
		BEntry entry(&ref);
		BPath path(&entry);

		if (!PDirectory::DirectoryExists(path.Path()))
		{
			// It doesn't, delete all the panels
			delete apmlPanel;
			apmlPanel = NULL;

			delete apmlMessenger;
			apmlMessenger = NULL;

			delete apmlSavePanel;
			apmlSavePanel = NULL;

			delete apmlSaveMessenger;
			apmlSaveMessenger = NULL;
		}
	}

	// Allocate the panel if not already allocated
	if (apmlSavePanel == NULL)
	{
		BMessage msg(AP_APML_SAVE_REFS_RECEIVED);

		// Create the window messenger object
		apmlSaveMessenger = new BMessenger(this);

		// Create the file panel object
		apmlSavePanel = new BFilePanel(B_SAVE_PANEL, apmlSaveMessenger, NULL, B_FILE_NODE, false, &msg);

		if (apmlPanel == NULL)
		{
			PString apmlPath(windowSystem->useSettings->GetStringEntryValue("Paths", "APML"));
			if (!apmlPath.IsEmpty())
			{
				BEntry entry((pathStr = apmlPath.GetString()));

				if (entry.InitCheck() == B_OK)
					apmlSavePanel->SetPanelDirectory(&entry);

				apmlPath.FreeBuffer(pathStr);
			}

			// Remember the directory
			apmlSavePanel->GetPanelDirectory(&apmlLastEntry);
		}
		else
		{
			// Change the start directory
			apmlSavePanel->SetPanelDirectory(&apmlLastEntry);
		}
	}
	else
	{
		// Change the start directory
		apmlSavePanel->SetPanelDirectory(&apmlLastEntry);
	}

	// Open the file panel
	apmlSavePanel->Show();
}



/******************************************************************************/
/* AddDirectoryToList() will make a recursive scan of the directory given and */
/*      add all the files in it to the module list.                           */
/*                                                                            */
/* Input:  "directory" is the directory to scan.                              */
/*         "list" is a reference to the list where to store the files.        */
/******************************************************************************/
void APWindowMain::AddDirectoryToList(PString directory, BList &list)
{
	PDirectory dir(directory);
	PDirectory::PEntryType type;
	PString name;

	// Initialize the enumeration
	dir.InitEnum(PDirectory::pAny);

	// Go through all the files
	while (dir.GetNextEntry(name, type))
	{
		// Got an entry, if it's a directory, skip it
		if (type == PDirectory::pDirectory)
			continue;

		// Add the item
		AppendMultiFile(dir.GetDirectory() + name, list);
	}

	dir.EndEnum();

	// Go through all the directories
	dir.InitEnum(PDirectory::pDirectory);

	while (dir.GetNextEntry(name, type))
		AddDirectoryToList(dir.GetDirectory() + name, list);

	dir.EndEnum();
}



/******************************************************************************/
/* AppendListFile() will load and append the list file into the module list.  */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to load.               */
/*         "index" is where in the current list you want to insert the file   */
/*         or -1 to append it at the bottom.                                  */
/******************************************************************************/
void APWindowMain::AppendListFile(PString fileName, int32 index)
{
	try
	{
		APMultiFiles::MultiFileType type;

		// Start to check the file
		if ((type = globalData->multiFiles->CheckForMultiFile(fileName)) != APMultiFiles::apList)
		{
			PString title, msg;

			// Show error
			title.LoadString(res, IDS_ERR_TITLE);
			msg.Format_S1(res, IDS_ERR_UNKNOWN_LIST_FILE, fileName);
			PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
			alert.Show();
			return;
		}

		// Set the sleep cursor
		SetSleepCursor();

		// Load the list
		AppendMultiFile(fileName, index);

		// Update the count control
		UpdateListCount();

		// Update the time controls
		UpdateTimes();

		// Update the tape deck controls
		UpdateTapeDeck();

		// Set cursor back to normal
		SetNormalCursor();
	}
	catch(PFileException e)
	{
		PString title, msg, err;
		char *errStr;

		// Set cursor back to normal
		SetNormalCursor();

		// Show error
		title.LoadString(res, IDS_ERR_TITLE);
		err = PSystem::GetErrorString(e.errorNum);
		msg.Format(res, IDS_ERR_LOAD_APML, e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);
		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();
	}
}



/******************************************************************************/
/* SaveAPMLFile() will save the whole module list to an APML file.            */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to save the list into. */
/******************************************************************************/
void APWindowMain::SaveAPMLFile(PString fileName)
{
	try
	{
		APMultiFileSave saveStruct;

		// Fill out the save structure
		saveStruct.window = this;
		saveStruct.index  = 0;
		saveStruct.count  = modList->CountItems();

		// Save the module list
		globalData->multiFiles->SaveAPMLFile(fileName, MultiFileSave, &saveStruct);
	}
	catch(PFileException e)
	{
		PString title, msg, err;
		char *errStr;

		// Show error
		title.LoadString(res, IDS_ERR_TITLE);
		err = PSystem::GetErrorString(e.errorNum);
		msg.Format(res, IDS_ERR_SAVE_APML, e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);
		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();
	}
}



/******************************************************************************/
/* RememberModuleList() will save the whole module list as a remember list.   */
/******************************************************************************/
void APWindowMain::RememberModuleList(void)
{
	try
	{
		PDirectory dir;
		PString fileName;
		char *nameStr;

		// Generate the filename with path
		dir.FindDirectory(PDirectory::pSettings);
		dir.Append("Polycode");
		dir.Append("APlayer");
		fileName = dir.GetDirectory() + "___apml___RememberList.apml";

		// Delete the file
		if (PFile::FileExists(fileName))
			PFile::Remove(fileName);

		// Check to see if we should write the module list
		if ((windowSystem->useSettings->GetStringEntryValue("Options", "RememberList").CompareNoCase("Yes") == 0) && (modList->CountItems() > 0))
		{
			// Save the module list
			SaveAPMLFile(fileName);

			if (windowSystem->useSettings->GetStringEntryValue("Options", "RememberListPosition").CompareNoCase("Yes") == 0)
			{
				// Set the attributes on the file
				BNode node((nameStr = fileName.GetString()));
				fileName.FreeBuffer(nameStr);

				node.WriteAttr("Selected", B_INT32_TYPE, 0, &rememberSelected, sizeof(int32));

				if (windowSystem->useSettings->GetStringEntryValue("Options", "RememberModulePosition").CompareNoCase("Yes") == 0)
				{
					node.WriteAttr("Position", B_INT32_TYPE, 0, &rememberPosition, sizeof(int32));
					node.WriteAttr("SubSong", B_INT32_TYPE, 0, &rememberSong, sizeof(int32));
				}
			}
		}
	}
	catch(...)
	{
		;
	}
}



/******************************************************************************/
/* AppendMultiFile() will load and append a single or multi file into the     */
/*      module list.                                                          */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to add.                */
/*         "index" is where in the current list you want to insert the file   */
/*         or -1 to append it at the bottom.                                  */
/******************************************************************************/
void APWindowMain::AppendMultiFile(PString fileName, int32 index)
{
	try
	{
		APMultiFileAdd addStruct;

		// Fill out the add file structure
		addStruct.window = this;

		// Add the file
		globalData->multiFiles->GetMultiFiles(fileName, MultiFileAdd, &addStruct);

		// Add the loaded entries into the module list
		if (index == -1)
			modList->AddList(&addStruct.tempList);
		else
			modList->AddList(&addStruct.tempList, index);
	}
	catch(PFileException e)
	{
		PString title, msg, err;
		char *errStr, *nameStr;

		// Show error
		title.LoadString(res, IDS_ERR_TITLE);
		err = PSystem::GetErrorString(e.errorNum);
		msg.Format(res, IDS_ERR_ADD_FILE, (nameStr = e.fileName.GetString()), e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);
		e.fileName.FreeBuffer(nameStr);
		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();
	}
}



/******************************************************************************/
/* AppendMultiFile() will load and append a single or multi file into the     */
/*      list given.                                                           */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to add.                */
/*         "list" is a reference to the list where you want the files added.  */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APWindowMain::AppendMultiFile(PString fileName, BList &list)
{
	try
	{
		APMultiFileAdd addStruct;

		// Fill out the add file structure
		addStruct.window = this;

		// Add the file
		globalData->multiFiles->GetMultiFiles(fileName, MultiFileAdd, &addStruct);

		// Add the loaded entries into the list
		list.AddList(&addStruct.tempList);
	}
	catch(PFileException e)
	{
		PString title, msg, err;
		char *errStr, *nameStr;

		// Show error
		title.LoadString(res, IDS_ERR_TITLE);
		err = PSystem::GetErrorString(e.errorNum);
		msg.Format(res, IDS_ERR_ADD_FILE, (nameStr = e.fileName.GetString()), e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);
		e.fileName.FreeBuffer(nameStr);
		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();
	}
}



/******************************************************************************/
/* MultiFileAdd() is a callback function that will be called for every file   */
/*      in a multi file. It will then add the file in the module list.        */
/*                                                                            */
/* Input:  "type" is a pointer to the file type structure.                    */
/*         "userData" is a pointer to the add structure needed to add the     */
/*         file to the list.                                                  */
/*                                                                            */
/* Output: True to continue, false to stop.                                   */
/******************************************************************************/
bool APWindowMain::MultiFileAdd(APMultiFiles::APMultiFileType *type, void *userData)
{
	APWindowMainListItem *item;
	APMultiFileAdd *addStruct = (APMultiFileAdd *)userData;

	// Create a new module list item
	item = new APWindowMainListItem(PDirectory::GetFilePart(type->fileName), type->fileName);
	if (item == NULL)
		return (false);

	// Set the time if any
	if (type->timeAvailable)
		addStruct->window->SetTimeOnItem(item, type->time);

	// Add the item to the list
	addStruct->tempList.AddItem(item);

	return (true);
}



/******************************************************************************/
/* MultiFileSave() is a callback function that will be called for every file  */
/*      to save in an list file.                                              */
/*                                                                            */
/* Input:  "type" is a pointer to the file type structure to fill out.        */
/*         "userData" is a pointer to the add structure needed to add the     */
/*         file to the list.                                                  */
/*                                                                            */
/* Output: True means a file has been returned, false indicates no more       */
/*         entries.                                                           */
/******************************************************************************/
bool APWindowMain::MultiFileSave(APMultiFiles::APMultiFileType *type, void *userData)
{
	APWindowMainListItem *item;
	APMultiFileSave *saveStruct = (APMultiFileSave *)userData;

	// Check the list count
	if (saveStruct->index == saveStruct->count)
		return (false);

	// Get the item to work on
	item = (APWindowMainListItem *)saveStruct->window->modList->ItemAt(saveStruct->index++);

	// Find the type of the item
	switch (item->GetItemType())
	{
		//
		// Normal file
		//
		case APWindowMainListItem::apNormal:
		{
			type->type     = APMultiFiles::apPlain;
			type->fileName = item->GetFileName();
			type->time     = item->GetTime();
			if (type->time.GetTotalMilliSeconds() != 0)
				type->timeAvailable = true;
			else
				type->timeAvailable = false;

			break;
		}

		//
		// Stop debugger. We haven't made support for a new type
		//
		default:
		{
			ASSERT(false);
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* SetMasterVolume() is called every time the master volume change.           */
/*                                                                            */
/* Input:  "object" is a pointer to the this object.                          */
/*         "vol" is the new volume as a negative number.                      */
/******************************************************************************/
void APWindowMain::SetMasterVolume(uint32 object, float vol)
{
	APWindowMain *win = (APWindowMain *)object;
	uint16 newVol = -(uint16)vol;

	// Remember the volume
	win->windowSystem->playerInfo->SetVolume(newVol);

	if ((win->playItem != NULL) && (win->muteBut->Value() == B_CONTROL_OFF))
		win->windowSystem->SetVolume(newVol);
}
