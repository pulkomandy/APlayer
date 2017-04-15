/******************************************************************************/
/* APlayer Settings add-on list class.                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Santa headers
#include <santa/ColumnListView.h>

// Client headers
#include "APViewAddOns.h"
#include "APListAddOns.h"
#include "APListItemAddOns.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListAddOns::APListAddOns(BRect frame, CLVContainerView** containerView, uint32 resizingMode, uint32 flags, list_view_type type, bool hierarchical, bool horizontal, bool vertical, bool scrollViewCorner, border_style border, const BFont* labelFont) :
	ColumnListView(frame, containerView, NULL, resizingMode, flags, type, hierarchical, horizontal, vertical, scrollViewCorner, border, labelFont)
{
	// Initialize member variables
	prevSelected = -1;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListAddOns::~APListAddOns(void)
{
}



/******************************************************************************/
/* MouseDown() checks for double clicks.                                      */
/******************************************************************************/
void APListAddOns::MouseDown(BPoint point)
{
	int32 selected;

	// First call the base class
	ColumnListView::MouseDown(point);

	// Get the current selected item
	selected = CurrentSelection();

	// Only check for double-click if an item is selected
	if (selected != -1)
	{
		if (selected == prevSelected)
		{
			// Well, clicked on the same as the previous
			if ((system_time() - prevTime) <= testTime)
			{
				// Okay, we got a double-click, send a message
				APListItemAddOns *item;
				int32 listNum;
				BMessage msg(SET_ADD_MSG_DOUBLECLICK);

				// Get the item
				item = (APListItemAddOns *)ItemAt(selected);
				item->GetItemInformation(listNum);

				// Create the message
				msg.AddInt32("listNum", listNum);
				msg.AddInt32("itemNum", selected);

				// Send the message
				Window()->PostMessage(&msg, Window());

				// Reset the variables
				prevSelected = -1;
			}
			else
			{
				// Clicked on the same item, but too late, so reset the variables
				prevSelected = selected;
				prevTime     = system_time();
				get_click_speed(&testTime);
			}
		}
		else
		{
			// Clicked on a different item, so reset the variables
			prevSelected = selected;
			prevTime     = system_time();
			get_click_speed(&testTime);
		}
	}
	else
		prevSelected = -1;
}



/******************************************************************************/
/* SelectionChanged() is called every a selection is made.                    */
/******************************************************************************/
void APListAddOns::SelectionChanged(void)
{
	int32 newSelected;
	APListItemAddOns *item;
	APAddOnInformation *info;
	char *strPtr;
	int32 listNum;
	BMessage msg(SET_ADD_MSG_SELECTIONCHANGED);

	newSelected = CurrentSelection();

	if (newSelected != -1)
	{
		// Get the item
		item = (APListItemAddOns *)ItemAt(newSelected);
		info = item->GetItemInformation(listNum);

		// Create the message
		msg.AddString("description", (strPtr = info->description.GetString()));
		msg.AddBool("settings", info->settings);
		msg.AddBool("display", info->display);
		msg.AddInt32("listNum", listNum);
		info->description.FreeBuffer(strPtr);
	}
	else
	{
		// Create an empty message
		msg.AddString("description", "");
		msg.AddBool("settings", false);
		msg.AddBool("display", false);
		msg.AddInt32("listNum", -1);
	}

	Window()->PostMessage(&msg, Window());
	MakeFocus();
}
