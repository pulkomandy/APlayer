/******************************************************************************/
/* Sample Info Instrument View class.                                         */
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
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "ColumnListView.h"
#include "CLVColumn.h"
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APViewInstruments.h"
#include "APListItemInstrument.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewInstruments::APViewInstruments(APGlobalData *glob, MainWindowSystem *system, BRect frame, PString name) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	PString label;
	char *labelPtr;
	font_height fh;
	int32 order[3];
	int32 sortKey[1];
	CLVSortMode sortMode[1];
	uint32 colWidth;

	// Remember the arguments
	global       = glob;
	windowSystem = system;

	// Remember the resource
	res = windowSystem->res;

	// Set the name of the view
	SetName((labelPtr = name.GetString()));
	name.FreeBuffer(labelPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	be_fixed_font->GetHeight(&fh);
	itemFontHeight = max(FIXED_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// Create the "Instrument" box
	//
	frame.right  -= (HSPACE * 2.0f);
	frame.bottom -= (VSPACE * 2.0f);
	instrumentBox = new BBox(frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	AddChild(instrumentBox);

	// Add the column listview
	frame           = instrumentBox->Bounds();
	frame.left     += (HSPACE * 2.0f);
	frame.right    -= (HSPACE * 2.0f + B_V_SCROLL_BAR_WIDTH);
	frame.bottom   -= (VSPACE * 2.0f + B_H_SCROLL_BAR_HEIGHT);
	frame.top      += (VSPACE * 2.0f);
	columnListView = new APListViewInstrument(frame, &containerView, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE | B_NAVIGABLE_JUMP, B_SINGLE_SELECTION_LIST, false, true, true, true, B_PLAIN_BORDER);

	// Initialize the switch position
	BRect rect(columnListView->Bounds());
	switchPos = rect.top;

	// Add the columns
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstCol1W");
	columnListView->AddColumn(new CLVColumn(NULL, colWidth, CLV_SORT_KEYABLE));

	label.LoadString(res, IDS_SI_ICOL_NAME);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstCol2W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_ICOL_SAMPLENUM);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstCol3W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	// Get the order
	order[0] = windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstCol1Pos");
	order[1] = windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstCol2Pos");
	order[2] = windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstCol3Pos");
	columnListView->SetDisplayOrder(order);

	// Attach the views to the window
	instrumentBox->AddChild(containerView);

	// Set the sort function and modes
	sortKey[0]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstSortKey");
	sortMode[0] = (CLVSortMode)windowSystem->useSettings->GetIntEntryValue("Window", "SampleInstSortMode");

	columnListView->SetSortFunction(APListItemInstrument::SortFunction);
	columnListView->SetSorting(1, sortKey, sortMode);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewInstruments::~APViewInstruments(void)
{
	RemoveItems();
}



/******************************************************************************/
/* SaveSettings() will save the view specific setting entries.                */
/******************************************************************************/
void APViewInstruments::SaveSettings(void)
{
	int32 *order;
	int32 sortKey[1];
	CLVSortMode sortMode[1];
	CLVColumn *column;
	int32 width[3];

	// Get the display order
	order = columnListView->DisplayOrder();

	// Get the sort information
	columnListView->Sorting(sortKey, sortMode);

	// Get the column widths
	for (int32 i = 0; i < 3; i++)
	{
		// Get the column sizes
		column   = columnListView->ColumnAt(i);
		width[i] = (uint32)column->Width();
	}

	try
	{
		// Do we need to store the positions
		if ((windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstCol1Pos") != order[0]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstCol2Pos") != order[1]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstCol3Pos") != order[2]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstCol1W") != width[0]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstCol2W") != width[1]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstCol3W") != width[2]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstSortKey") != sortKey[0]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleInstSortMode") != sortMode[0]))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstCol1Pos", order[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstCol2Pos", order[1]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstCol3Pos", order[2]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstCol1W", width[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstCol2W", width[1]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstCol3W", width[2]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstSortKey", sortKey[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleInstSortMode", sortMode[0]);
		}
	}
	catch(...)
	{
		delete[] order;
		throw;
	}

	// Clean up
	delete[] order;
}



/******************************************************************************/
/* AddItems() will add all the items in the column list view.                 */
/******************************************************************************/
void APViewInstruments::AddItems(void)
{
	uint32 handle;
	const PList<APInstInfo *> *list;
	int32 i, count;

	// Get the current file handle
	handle = windowSystem->loader->GetFileHandle();
	if (handle == 0)
		return;

	// Get the list pointer
	list = global->GetInstrumentInformationWithLock(handle);

	// Add the items
	count = list->CountItems();
	for (i = 0; i < count; i++)
		columnListView->AddItem(new APListItemInstrument(itemFontHeight, list->GetItem(i), i + 1));

	// Unlock it again
	global->UnlockInstrumentInformation(handle);

	// Sort the items
	columnListView->SortItems();
}



/******************************************************************************/
/* RemoveItems() will remove all the items in the column list view.           */
/******************************************************************************/
void APViewInstruments::RemoveItems(void)
{
	int32 i, count;
	BListItem *item;

	// Remove all the items from the list view
	count = columnListView->CountItems();
	for (i = 0; i < count; i++)
	{
		item = columnListView->RemoveItem((int32)0);
		delete item;
	}

	// Reset the listview position
	switchPos = 0.0f;
}



/******************************************************************************/
/* AttachedToWindow() is called everytime the view has to be shown.           */
/******************************************************************************/
void APViewInstruments::AttachedToWindow(void)
{
	BRect frame;

	// Call base class
	BView::AttachedToWindow();

	// Find the tab view size
	frame = Parent()->Bounds();
	frame.right  -= HSPACE * 2.0f;
	frame.bottom -= VSPACE * 2.0f;

	// Resize the view
	ResizeTo(frame.Width(), frame.Height());

	// Set the focus to the column list view
	columnListView->MakeFocus();

	// Scroll to the position the listview was in when switched to another view
	columnListView->ScrollTo(BPoint(0.0f, switchPos));
}



/******************************************************************************/
/* DetachedFromWindow() is called everytime the view has to be hidden.        */
/******************************************************************************/
void APViewInstruments::DetachedFromWindow(void)
{
	// Remember to position of the column view
	switchPos = columnListView->Bounds().top;

	// Call base class
	BView::DetachedFromWindow();
}
