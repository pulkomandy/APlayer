/******************************************************************************/
/* APlayer Settings FileTypes View class.                                     */
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
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Santa headers
#include <santa/ColumnListView.h>

// Client headers
#include "MainWindowSystem.h"
#include "APViewFileTypes.h"
#include "APListFileTypes.h"
#include "APListItemFileTypes.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewFileTypes::APViewFileTypes(MainWindowSystem *system, APGlobalData *global, BRect frame, PString name) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect rect;
	PString label;
	char *labelPtr;
	BMessage *message;
	float x, y, w;
	float controlWidth, controlHeight;
	font_height fh;
	int32 order[4];
	int32 sortKey[1];
	CLVSortMode sortMode[1];

	// Remember the arguments
	windowSystem = system;
	globalData   = global;
	res          = system->res;

	// Set the view name
	SetName((labelPtr = name.GetString()));
	name.FreeBuffer(labelPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// Create the "Filetypes" box
	//
	w = (SET_MIN_VIEW_WIDTH - HSPACE * 2.0f) / 2.0f;
	label.LoadString(res, IDS_SET_FIL_FILETYPES);
	fileTypesBox = new BBox(BRect(0.0f, 0.0f, w, SET_MIN_VIEW_HEIGHT), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	fileTypesBox->SetLabel((labelPtr = label.GetString()));
	AddChild(fileTypesBox);
	label.FreeBuffer(labelPtr);

	//
	// Create the "Options" box
	//
	x = SET_MIN_VIEW_WIDTH / 2.0f + HSPACE;
	label.LoadString(res, IDS_SET_FIL_OPTIONS);
	optionsBox = new BBox(BRect(x, 0.0f, x + w, SET_MIN_VIEW_HEIGHT), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	optionsBox->SetLabel((labelPtr = label.GetString()));
	AddChild(optionsBox);
	label.FreeBuffer(labelPtr);

	//
	// Create the buttons in the filetypes box
	//
	rect = fileTypesBox->Bounds();
	rect.InsetBy(HSPACE * 2.0f, VSPACE * 2.0f);

	message = new BMessage(SET_FIL_BUT_SELECTED);
	label.LoadString(res, IDS_SET_FIL_REGISTER_SELECTED);
	selectedBut = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	selectedBut->GetPreferredSize(&controlWidth, &controlHeight);
	label.FreeBuffer(labelPtr);

	w = controlWidth;
	y = rect.bottom - controlHeight + 2.0f;

	selectedBut->ResizeTo(controlWidth, controlHeight);
	selectedBut->MoveTo(rect.left - 2.0f, y);
	selectedBut->SetEnabled(false);

	message = new BMessage(SET_FIL_BUT_ALL);
	label.LoadString(res, IDS_SET_FIL_REGISTER_ALL);
	allBut = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	allBut->GetPreferredSize(&controlWidth, &controlHeight);
	label.FreeBuffer(labelPtr);

	x = rect.left + w + HSPACE + 2.0f;
	allBut->ResizeTo(max(controlWidth, rect.right - x + 1.0f), controlHeight);
	allBut->MoveTo(x, y);

	//
	// Create the filetype list box
	//
	rect.top    += fontHeight;
	rect.bottom -= (controlHeight + VSPACE + B_H_SCROLL_BAR_HEIGHT + 1.0f);
	rect.right  -= (B_V_SCROLL_BAR_WIDTH + 1.0f);

	fileTypesColList = new APListFileTypes(rect, &fileTypesView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE | B_NAVIGABLE_JUMP, B_MULTIPLE_SELECTION_LIST, false, true, true, true, B_PLAIN_BORDER);

	// Add the columns
	w = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol1Width");
	fileTypesColList->AddColumn(new CLVColumn(NULL, w, CLV_NOT_RESIZABLE, 20.0f));

	label.LoadString(res, IDS_SET_FIL_TYPE);
	w = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol2Width");
	fileTypesColList->AddColumn(new CLVColumn((labelPtr = label.GetString()), w, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SET_FIL_APPLICATION);
	w = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol3Width");
	fileTypesColList->AddColumn(new CLVColumn((labelPtr = label.GetString()), w, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SET_FIL_EXTENSIONS);
	w = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol4Width");
	fileTypesColList->AddColumn(new CLVColumn((labelPtr = label.GetString()), w, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	order[0] = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol1Pos");
	order[1] = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol2Pos");
	order[2] = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol3Pos");
	order[3] = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol4Pos");
	fileTypesColList->SetDisplayOrder(order);

	// Attach the view to the window
	fileTypesBox->AddChild(fileTypesView);

	// Set the sort function and modes
	sortKey[0]  = windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesSortKey");
	sortMode[0] = (CLVSortMode)windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesSortMode");

	fileTypesColList->SetSortFunction(APListItemFileTypes::SortFunction);
	fileTypesColList->SetSorting(1, sortKey, sortMode);

	// Add the items to the list view
	AddItems();

	// Add the rest of the controls
	fileTypesBox->AddChild(selectedBut);
	fileTypesBox->AddChild(allBut);

	//
	// Create the checkboxes in the option box
	//
	rect = optionsBox->Bounds();

	x = rect.left + HSPACE * 2.0f;
	y = rect.top + fontHeight + VSPACE;

	message = new BMessage(SET_FIL_CHECK_CHANGETYPE);
	label.LoadString(res, IDS_SET_FIL_CHANGETYPE);
	typeCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	typeCheck->GetPreferredSize(&controlWidth, &controlHeight);
	typeCheck->ResizeTo(controlWidth, controlHeight);
	typeCheck->MoveTo(x, y);
	optionsBox->AddChild(typeCheck);
	label.FreeBuffer(labelPtr);

	y += (controlHeight + VSPACE);
	message = new BMessage(SET_FIL_CHECK_REGISTER);
	label.LoadString(res, IDS_SET_FIL_REGISTER);
	registerCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	registerCheck->ResizeToPreferred();
	registerCheck->MoveTo(x, y);
	optionsBox->AddChild(registerCheck);
	label.FreeBuffer(labelPtr);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewFileTypes::~APViewFileTypes(void)
{
	// Remove list items
	RemoveItems();
}



/******************************************************************************/
/* InitSettings() will read the settings and set all the controls.            */
/******************************************************************************/
void APViewFileTypes::InitSettings(void)
{
	PString tempStr;

	//
	// Options box
	//
	tempStr = windowSystem->useSettings->GetStringEntryValue("FileTypes", "ChangeModuleType");
	typeCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("FileTypes", "RegisterFileTypes");
	registerCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);
}



/******************************************************************************/
/* RememberSettings() will read the data from controls which hasn't stored    */
/*      they values yet, eg. edit controls.                                   */
/******************************************************************************/
void APViewFileTypes::RememberSettings(void)
{
}



/******************************************************************************/
/* CancelSettings() will restore real-time values.                            */
/******************************************************************************/
void APViewFileTypes::CancelSettings(void)
{
}



/******************************************************************************/
/* SaveSettings() will save some of the settings in other files.              */
/******************************************************************************/
void APViewFileTypes::SaveSettings(void)
{
}



/******************************************************************************/
/* SaveWindowSettings() will save the view specific settings entries.         */
/******************************************************************************/
void APViewFileTypes::SaveWindowSettings(void)
{
	int32 width[4];
	int32 order[4];
	int32 sortKey[1];
	CLVSortMode sortMode[1];
	CLVColumn *column;

	// Get the display order
	fileTypesColList->GetDisplayOrder(&order[0]);

	// Get the sort information
	fileTypesColList->GetSorting(sortKey, sortMode);

	// Get the width of the columns
	column   = fileTypesColList->ColumnAt(0);
	width[0] = (int32)column->Width();

	column   = fileTypesColList->ColumnAt(1);
	width[1] = (int32)column->Width();

	column   = fileTypesColList->ColumnAt(2);
	width[2] = (int32)column->Width();

	column   = fileTypesColList->ColumnAt(3);
	width[3] = (int32)column->Width();

	try
	{
		// Do we need to store the positions
		if ((windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol1Pos") != order[0]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol2Pos") != order[1]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol3Pos") != order[2]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol4Pos") != order[3]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol1Width") != width[0]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol2Width") != width[1]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol3Width") != width[2]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesCol4Width") != width[3]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesSortKey") != sortKey[0]) ||
			(windowSystem->useSettings->GetIntEntryValue("Window", "FileTypesSortMode") != sortMode[0]))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol1Pos", order[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol2Pos", order[1]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol3Pos", order[2]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol4Pos", order[3]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol1Width", width[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol2Width", width[1]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol3Width", width[2]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesCol4Width", width[3]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesSortKey", sortKey[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "FileTypesSortMode", sortMode[0]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol1Pos", order[0]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol2Pos", order[1]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol3Pos", order[2]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol4Pos", order[3]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol1Width", width[0]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol2Width", width[1]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol3Width", width[2]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesCol4Width", width[3]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesSortKey", sortKey[0]);
			windowSystem->useSettings->WriteIntEntryValue("Window", "FileTypesSortMode", sortMode[0]);
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
void APViewFileTypes::MakeBackup(void)
{
}



/******************************************************************************/
/* HandleMessage() is called for each message the window don't know.          */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APViewFileTypes::HandleMessage(BMessage *msg)
{
	int32 value;

	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Selection changed in the file type list
		////////////////////////////////////////////////////////////////////////
		case SET_FIL_MSG_SELECTIONCHANGED:
		{
			selectedBut->SetEnabled(msg->FindBool("selected"));
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Register selected filetypes button
		////////////////////////////////////////////////////////////////////////
		case SET_FIL_BUT_SELECTED:
		{
			APList<APSystemFileType *> *systemList;
			APListItemFileTypes *item;
			int32 selected, i;

			// Get the pointer to the system file type list
			systemList = globalData->fileTypes->GetFileTypeList();

			// Lock the list
			systemList->LockList();

			i = 0;
			while ((selected = fileTypesColList->CurrentSelection(i)) >= 0)
			{
				// Get selected item
				item = (APListItemFileTypes *)fileTypesColList->ItemAt(selected);

				// Register the file type
				globalData->fileTypes->RegisterFileTypeInSystem(item->fileType->addonNum);

				// Go to next selection
				i++;
			}

			// Unlock the list again
			systemList->UnlockList();

			// Rebuild the file type list
			RemoveItems();
			globalData->fileTypes->BuildFileTypeList();
			AddItems();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Register all filetypes button
		////////////////////////////////////////////////////////////////////////
		case SET_FIL_BUT_ALL:
		{
			APList<APSystemFileType *> *systemList;
			APListItemFileTypes *item;
			int32 count, i;

			// Get the pointer to the system file type list
			systemList = globalData->fileTypes->GetFileTypeList();

			// Lock the list
			systemList->LockList();

			count = fileTypesColList->CountItems();
			for (i = 0; i < count; i++)
			{
				// Get selected item
				item = (APListItemFileTypes *)fileTypesColList->ItemAt(i);

				// Register the file type
				globalData->fileTypes->RegisterFileTypeInSystem(item->fileType->addonNum);
			}

			// Unlock the list again
			systemList->UnlockList();

			// Rebuild the file type list
			RemoveItems();
			globalData->fileTypes->BuildFileTypeList();
			AddItems();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Set file type checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_FIL_CHECK_CHANGETYPE:
		{
			value = (typeCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("FileTypes", "ChangeModuleType", value ? "Yes" : "No");
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Register new filetypes checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_FIL_CHECK_REGISTER:
		{
			value = (registerCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("FileTypes", "RegisterFileTypes", value ? "Yes" : "No");
			break;
		}
	}
}



/******************************************************************************/
/* AddItems() will add the all items in the filetype list view.               */
/******************************************************************************/
void APViewFileTypes::AddItems(void)
{
	APList<APSystemFileType *> *fileList;
	APSystemFileType *fileType;
	int32 i, count;

	// Get the file type list
	fileList = globalData->fileTypes->GetFileTypeList();

	// Lock the list
	fileList->LockList();

	// Get the number of items in the list
	count = fileList->CountItems();

	for (i = 0; i < count; i++)
	{
		// Get the filetype information
		fileType = fileList->GetItem(i);

		// Add the list item
		fileTypesColList->AddItem(new APListItemFileTypes(fontHeight, fileType));
	}

	// Unlock the list
	fileList->UnlockList();

	// Sort the list
	fileTypesColList->SortItems();
}



/******************************************************************************/
/* RemoveItems() will remove all items in the filetype list view.             */
/******************************************************************************/
void APViewFileTypes::RemoveItems(void)
{
	int32 i, count;

	// Remove all the items in the list
	count = fileTypesColList->CountItems();
	for (i = 0; i < count; i++)
		delete fileTypesColList->RemoveItem((int32)0);
}
