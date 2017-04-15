/******************************************************************************/
/* APlayer Settings Add-On View class.                                        */
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
#include "PAlert.h"
#include "PList.h"
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APList.h"
#include "Layout.h"

// Santa headers
#include <santa/ColumnListView.h>

// Client headers
#include "MainWindowSystem.h"
#include "APViewAddOns.h"
#include "APListAddOns.h"
#include "APListDescription.h"
#include "APListItemAddOns.h"
#include "APListItemDescription.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* APViewAddOns class                                                         */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewAddOns::APViewAddOns(MainWindowSystem *system, APGlobalData *global, BRect frame, PString name, PString type, APList<APAddOnInformation *> *list) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect rect;
	PString label;
	char *labelPtr;
	BMessage *message;
	float y, w;
	float controlWidth, controlHeight;
	font_height fh;
	int32 order[3];
	int32 sortKey[1];
	CLVSortMode sortMode[1];

	// Remember the arguments
	windowSystem = system;
	res          = system->res;
	globalData   = global;
	addOnType    = type;
	infoList     = list;

	// Set the view name
	SetName((labelPtr = name.GetString()));
	name.FreeBuffer(labelPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// Create the "AddOns" box
	//
	addOnsBox = new BBox(BRect(0.0f, fontHeight / 2.0f, SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT), NULL, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	AddChild(addOnsBox);

	// Create the description text box
	rect = addOnsBox->Bounds();
	rect.InsetBy(HSPACE * 2.0f, VSPACE * 2.0f);
	rect.right = ((rect.Width() / 2.0f) - HSPACE - B_V_SCROLL_BAR_WIDTH) + rect.left;
	rect.OffsetBy(rect.Width() + B_V_SCROLL_BAR_WIDTH + HSPACE * 2.0f, 0.0f);

	descriptionColList = new APListDescription(rect, &descriptionView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, false, true, true, B_PLAIN_BORDER);

	label.LoadString(res, IDS_SET_ADO_DESCRIPTION);
	descriptionColList->AddColumn(new CLVColumn((labelPtr = label.GetString()), rect.Width(), CLV_NOT_RESIZABLE | CLV_NOT_MOVABLE));
	label.FreeBuffer(labelPtr);

	// Create the buttons
	rect.OffsetBy(-(rect.Width() + B_V_SCROLL_BAR_WIDTH + HSPACE * 2.0f), 0.0f);

	message = new BMessage(SET_ADD_BUT_SETTINGS);
	label.LoadString(res, IDS_SET_ADO_SETTINGS);
	settingBut = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	settingBut->GetPreferredSize(&controlWidth, &controlHeight);
	label.FreeBuffer(labelPtr);

	w = (rect.Width() + B_V_SCROLL_BAR_WIDTH - HSPACE) / 2.0f;
	y = rect.bottom - controlHeight + 2.0f;

	settingBut->ResizeTo(w, controlHeight);
	settingBut->MoveTo(rect.left - 2.0f, y);
	settingBut->SetEnabled(false);

	message = new BMessage(SET_ADD_BUT_DISPLAY);
	label.LoadString(res, IDS_SET_ADO_DISPLAY);
	displayBut = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	displayBut->ResizeTo(w, controlHeight);
	displayBut->MoveTo(rect.left + w + HSPACE + 2.0f, y);
	displayBut->SetEnabled(false);
	label.FreeBuffer(labelPtr);

	// Create the add-on list box
	rect.bottom -= (controlHeight + VSPACE);

	addOnsColList = new APListAddOns(rect, &addOnsView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE | B_NAVIGABLE_JUMP, B_SINGLE_SELECTION_LIST, false, false, true, true, B_PLAIN_BORDER);

	// Add the columns
	addOnsColList->AddColumn(new CLVColumn(NULL, 20.0f, CLV_NOT_RESIZABLE | CLV_SORT_KEYABLE, 20.0f));

	label.LoadString(res, IDS_SET_ADO_VERSION);
	w = StringWidth((labelPtr = label.GetString())) + HSPACE * 4.0f;
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SET_ADO_NAME);
	addOnsColList->AddColumn(new CLVColumn((labelPtr = label.GetString()), rect.Width() - w - 20.0f - 2.0f, CLV_NOT_RESIZABLE | CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SET_ADO_VERSION);
	addOnsColList->AddColumn(new CLVColumn((labelPtr = label.GetString()), w, CLV_NOT_RESIZABLE | CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	order[0] = windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "Col1Pos");
	order[1] = windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "Col2Pos");
	order[2] = windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "Col3Pos");
	addOnsColList->SetDisplayOrder(order);

	// Attach the view to the window
	addOnsBox->AddChild(addOnsView);

	// Add the items to the list view
	AddItems();

	// Set the sort function and modes
	sortKey[0]  = windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "SortKey");
	sortMode[0] = (CLVSortMode)windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "SortMode");

	addOnsColList->SetSortFunction(APListItemAddOns::SortFunction);
	addOnsColList->SetSorting(1, sortKey, sortMode);
	addOnsColList->SortItems();

	// Add the rest of the controls
	addOnsBox->AddChild(settingBut);
	addOnsBox->AddChild(displayBut);
	addOnsBox->AddChild(descriptionView);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewAddOns::~APViewAddOns(void)
{
	int32 i, count;

	// Remove all the items in the list
	count = addOnsColList->CountItems();
	for (i = 0; i < count; i++)
		delete addOnsColList->RemoveItem((int32)0);
}



/******************************************************************************/
/* InitSettings() will read the settings and set all the controls.            */
/******************************************************************************/
void APViewAddOns::InitSettings(void)
{
}



/******************************************************************************/
/* RememberSettings() will read the data from controls which hasn't stored    */
/*      they values yet, eg. edit controls.                                   */
/******************************************************************************/
void APViewAddOns::RememberSettings(void)
{
}



/******************************************************************************/
/* CancelSettings() will restore real-time values.                            */
/******************************************************************************/
void APViewAddOns::CancelSettings(void)
{
	int32 i, count;
	APAddOnInformation *item;

	// Enable or disable previous enabled or disabled add-ons
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		item = infoList->GetItem(i);
		if (item->enabled != backupList.GetItem(i))
		{
			// Well, something has changed here
			if (backupList.GetItem(i))
				EnableAddOn(item);
			else
				DisableAddOn(item);
		}
	}
}



/******************************************************************************/
/* SaveSettings() will save some of the settings in other files.              */
/******************************************************************************/
void APViewAddOns::SaveSettings(void)
{
}



/******************************************************************************/
/* SaveWindowSettings() will save the view specific settings entries.         */
/******************************************************************************/
void APViewAddOns::SaveWindowSettings(void)
{
	int32 order[3];
	int32 sortKey[1];
	CLVSortMode sortMode[1];

	// Get the display order
	addOnsColList->GetDisplayOrder(&order[0]);

	// Get the sort information
	addOnsColList->GetSorting(sortKey, sortMode);

	try
	{
		// Do we need to store the positions
		if ((windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "Col1Pos") != order[0]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "Col2Pos") != order[1]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "Col3Pos") != order[2]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "SortKey") != sortKey[0]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", addOnType + "SortMode") != sortMode[0]))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", addOnType + "Col1Pos", order[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", addOnType + "Col2Pos", order[1]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", addOnType + "Col3Pos", order[2]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", addOnType + "SortKey", sortKey[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", addOnType + "SortMode", sortMode[0]);
			windowSystem->useSettings->WriteIntEntryValue("Window", addOnType + "Col1Pos", order[0]);
			windowSystem->useSettings->WriteIntEntryValue("Window", addOnType + "Col2Pos", order[1]);
			windowSystem->useSettings->WriteIntEntryValue("Window", addOnType + "Col3Pos", order[2]);
			windowSystem->useSettings->WriteIntEntryValue("Window", addOnType + "SortKey", sortKey[0]);
			windowSystem->useSettings->WriteIntEntryValue("Window", addOnType + "SortMode", sortMode[0]);
		}
	}
	catch(...)
	{
		throw;
	}
}



/******************************************************************************/
/* MakeBackup() will make a backup of special settings not stored in the      */
/*      "use" settings.                                                       */
/******************************************************************************/
void APViewAddOns::MakeBackup(void)
{
	int32 i, count;
	APAddOnInformation *item;

	// Remove any old backup
	backupList.MakeEmpty();

	// Now create a backup of the enabled add-ons
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the enable state and add it to the list
		item = infoList->GetItem(i);
		backupList.AddTail(item->enabled);
	}
}



/******************************************************************************/
/* HandleMessage() is called for each message the window don't know.          */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APViewAddOns::HandleMessage(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Add-On selection
		////////////////////////////////////////////////////////////////////////
		case SET_ADD_MSG_SELECTIONCHANGED:
		{
			const char *descr;
			bool settings, display;

			// Find all the message attributes
			if (msg->FindString("description", &descr) == B_OK)
			{
				if (msg->FindBool("settings", &settings) == B_OK)
				{
					if (msg->FindBool("display", &display) == B_OK)
					{
						if (msg->FindInt32("listNum", &listNum) == B_OK)
						{
							// Show the description
							ShowDescription(descr);

							// Enable or disable the extra buttons
							settingBut->SetEnabled(settings);
							displayBut->SetEnabled(display);
						}
					}
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Double-click on an add-on
		////////////////////////////////////////////////////////////////////////
		case SET_ADD_MSG_DOUBLECLICK:
		{
			int32 listNum, itemNum;
			APAddOnInformation *item;

			// Get the item number
			if (msg->FindInt32("listNum", &listNum) == B_OK)
			{
				if (msg->FindInt32("itemNum", &itemNum) == B_OK)
				{
					// Lock the list
					infoList->LockList();

					// Get the item
					item = infoList->GetItem(listNum);

					if (item->enabled)
					{
						// Check to see if it's ourselve
						if (item->name == windowSystem->GetName(0))
						{
							// It is, warn the user
							PString title, msg;

							title.LoadString(res, IDS_ERR_TITLE);
							msg.LoadString(res, IDS_ERR_CANT_DISABLE);
							PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
							alert.Show();
						}
						else
							DisableAddOn(item);
					}
					else
						EnableAddOn(item);

					// Unlock the list again
					infoList->UnlockList();

					// Invalidate the list view
					addOnsColList->InvalidateItem(itemNum);
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Settings button
		////////////////////////////////////////////////////////////////////////
		case SET_ADD_BUT_SETTINGS:
		{
			APAddOnInformation *info;

			// Get the add-on information
			infoList->LockList();
			info = infoList->GetItem(listNum);

			if (info->enabled)
			{
				// Tell the server to open the configuration window
				windowSystem->OpenConfigWindow(info->name);
			}
			else
			{
				PString title, msg;

				// Show error
				title.LoadString(res, IDS_ERR_TITLE);
				msg.LoadString(res, IDS_ERR_SETTINGS_DISABLED);
				PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
				alert.Show();
			}

			infoList->UnlockList();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Display button
		////////////////////////////////////////////////////////////////////////
		case SET_ADD_BUT_DISPLAY:
		{
			APAddOnInformation *info;

			// Get the add-on information
			infoList->LockList();
			info = infoList->GetItem(listNum);

			if (info->enabled)
			{
				// Tell the server to open the display window
				windowSystem->OpenDisplayWindow(info->name);
			}
			else
			{
				PString title, msg;

				// Show error
				title.LoadString(res, IDS_ERR_TITLE);
				msg.LoadString(res, IDS_ERR_DISPLAY_DISABLED);
				PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
				alert.Show();
			}

			infoList->UnlockList();
			break;
		}
	}
}



/******************************************************************************/
/* EnableAddOn() will enable the add-on.                                      */
/*                                                                            */
/* Input:  "item" is the add-on to enable.                                    */
/******************************************************************************/
void APViewAddOns::EnableAddOn(APAddOnInformation *item)
{
	PString result;
	int index;

	// Tell the server to enable the add-on
	result = windowSystem->EnableAddOn(item->name);

	// Update the item
	item->settings = result.GetUNumber();

	index = result.Find(',');
	if (index == -1)
		item->display = false;
	else
		item->display = result.GetUNumber(index + 1);

	item->enabled = true;

	// Add the menu items
	windowSystem->mainWin->AddAddOnToMenu(item);

	// Enable or disable the extra buttons
	settingBut->SetEnabled(item->settings);
	displayBut->SetEnabled(item->display);
}



/******************************************************************************/
/* DisableAddOn() will disable the add-on.                                    */
/*                                                                            */
/* Input:  "item" is the add-on to disable.                                   */
/******************************************************************************/
void APViewAddOns::DisableAddOn(APAddOnInformation *item)
{
	// If the item is the one in-use, free the playing module
	if (inUseName == item->name)
		windowSystem->mainWin->StopAndFreeModule();

	// Remove the menu items
	windowSystem->mainWin->RemoveAddOnFromMenu(item);

	// Tell the server to disable the add-on
	windowSystem->DisableAddOn(item->name);

	// Update the item
	item->enabled = false;
}



/******************************************************************************/
/* UpdateAddOnItem() will set a single add-on item.                           */
/*                                                                            */
/* Input:  "name" is the add-on name to update.                               */
/******************************************************************************/
void APViewAddOns::UpdateAddOnItem(PString name)
{
	int32 i, count, temp;
	APListItemAddOns *item;
	APAddOnInformation *info;

	// Remember the name
	inUseName = name;

	// Get the number of items in the list
	count = addOnsColList->CountItems();

	// See if we can find the right item to invalidate
	for (i = 0; i < count; i++)
	{
		item = (APListItemAddOns *)addOnsColList->ItemAt(i);
		info = item->GetItemInformation(temp);

		// Did we found the item
		if (info->name == name)
		{
			// Yep, invalidate it
			item->SetInUseFlag(true);
			addOnsColList->InvalidateItem(i);
			break;
		}
	}
}



/******************************************************************************/
/* ClearAddOnItem() will clear a single add-on item.                          */
/******************************************************************************/
void APViewAddOns::ClearAddOnItem(void)
{
	int32 i, count, temp;
	APListItemAddOns *item;
	APAddOnInformation *info;

	// Get the number of items in the list
	count = addOnsColList->CountItems();

	// See if we can find the right item to invalidate
	for (i = 0; i < count; i++)
	{
		item = (APListItemAddOns *)addOnsColList->ItemAt(i);
		info = item->GetItemInformation(temp);

		// Did we found the item
		if (info->name == inUseName)
		{
			// Yep, invalidate it
			item->SetInUseFlag(false);
			addOnsColList->InvalidateItem(i);
			break;
		}
	}

	// Clear the "inUse" name
	inUseName.MakeEmpty();
}



/******************************************************************************/
/* AddItems() will add the all items in the column list view.                 */
/******************************************************************************/
void APViewAddOns::AddItems(void)
{
	APAddOnInformation *info;
	int32 i, count;

	// Start to lock the list
	infoList->LockList();

	// Add all the items in the list view
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the add-on information
		info = infoList->GetItem(i);

		// Add the list item
		addOnsColList->AddItem(new APListItemAddOns(res, fontHeight, info, i));
	}

	// Unlock the list
	infoList->UnlockList();
}



/******************************************************************************/
/* ShowDescription() will show the description in the description view.       */
/*                                                                            */
/* Input:  "description" is the description text.                             */
/******************************************************************************/
void APViewAddOns::ShowDescription(PString description)
{
	float listWidth, lineWidth;
	int32 index;
	PString tempStr, tempStr1;
	char *tempPtr;

	// Start the clear the description view
	descriptionColList->MakeEmpty();

	// Get needed informations before start
	listWidth = descriptionColList->ColumnAt(0)->Width();

	// Split the description
	while (!description.IsEmpty())
	{
		// See if there is any newlines
		index = description.Find('\n');
		if (index != -1)
		{
			// There is, get the line
			tempStr = description.Left(index);
			description.Delete(0, index + 1);
		}
		else
		{
			tempStr = description;
			description.MakeEmpty();
		}

		// Adjust the description line
		tempStr.TrimRight();
		tempStr.TrimLeft();

		// We got an empty line, so add it
		if (tempStr.IsEmpty())
			descriptionColList->AddItem(new APListItemDescription(fontHeight, ""));
		else
		{
			do
			{
				// See if the line can be showed
				lineWidth = descriptionColList->StringWidth((tempPtr = tempStr.GetString()));
				tempStr.FreeBuffer(tempPtr);
				tempStr1.MakeEmpty();

				while (lineWidth >= listWidth)
				{
					// We need to split the line
					index = tempStr.ReverseFind(' ');
					if (index != -1)
					{
						// Found a space, check if the line can be showed now
						tempStr1  = tempStr.Mid(index) + tempStr1;
						tempStr   = tempStr.Left(index);
						lineWidth = descriptionColList->StringWidth((tempPtr = tempStr.GetString()));
						tempStr.FreeBuffer(tempPtr);
					}
					else
						break;		// Well, the line can't be showed and we can't split it :(
				}

				// Adjust the description line
				tempStr.TrimRight();
				tempStr.TrimLeft();

				// Add the line in the list view
				descriptionColList->AddItem(new APListItemDescription(fontHeight, tempStr));
				tempStr = tempStr1;
			}
			while (!tempStr.IsEmpty());
		}
	}
}
