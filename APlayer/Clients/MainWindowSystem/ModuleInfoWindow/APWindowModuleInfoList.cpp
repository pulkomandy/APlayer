/******************************************************************************/
/* APlayer Module Info list class.                                            */
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
#include <santa/CLVColumnLabelView.h>

// Client headers
#include "APWindowModuleInfo.h"
#include "APWindowModuleInfoList.h"
#include "APWindowModuleInfoListItem.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowModuleInfoList::APWindowModuleInfoList(BRect frame, CLVContainerView** containerView, uint32 resizingMode, uint32 flags, list_view_type type, bool hierarchical, bool horizontal, bool vertical, bool scrollViewCorner, border_style border, const BFont* labelFont) :
	ColumnListView(frame, containerView, NULL, resizingMode, flags, type, hierarchical, horizontal, vertical, scrollViewCorner, border, labelFont)
{
	font_height fh;

	// Find the font height
	GetFontHeight(&fh);
	fontHeight = ceil(fh.ascent + fh.descent);

	// Initialize other member variables
	prevIndex = -1;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowModuleInfoList::~APWindowModuleInfoList(void)
{
	// Remove all the list items
	RemoveAllItems();
}



/******************************************************************************/
/* RemoveAllItems() will remove all the items in the list.                    */
/******************************************************************************/
void APWindowModuleInfoList::RemoveAllItems(void)
{
	uint32 i, count;
	BListItem *item;

	// Remove all the items from the list view
	count = CountItems();
	for (i = 0; i < count; i++)
	{
		item = RemoveItem((int32)0);
		delete item;
	}
}



/******************************************************************************/
/* KeyDown() is called for every key the user presses when the list has the   */
/*      focus. It parse key down and up so it list scrolls.                   */
/*                                                                            */
/* Input:  "bytes" is a pointer to the UTF-8 character.                       */
/*         "numBytes" is the number of bytes the key has.                     */
/******************************************************************************/
void APWindowModuleInfoList::KeyDown(const char *bytes, int32 numBytes)
{
	if (numBytes == 1)
	{
		BRect curRect;

		// Get the showing rect
		curRect = Bounds();

		// Arrow down
		if (*bytes == B_DOWN_ARROW)
		{
			uint32 i, count;
			float height = 0.0f;

			// Get the number of items in the list
			count = CountItems();

			// Calculate the total number of pixels to use
			for (i = 0; i < count; i++)
				height += ceil(ItemAt(i)->Height() + 1.0f);

			if (count > 0)
				height -= 1.0f;

			if (curRect.bottom < height)
				ScrollBy(0.0f, fontHeight);

			return;
		}

		// Arrow up
		if (*bytes == B_UP_ARROW)
		{
			if (curRect.top > 0.0f)
				ScrollBy(0.0f, -fontHeight);

			return;
		}

		// Tab
		if (*bytes == B_TAB)
			return;
	}

	// Call the base class
	ColumnListView::KeyDown(bytes, numBytes);
}



/******************************************************************************/
/* MouseMoved() is called when the mouse is moving inside the view.           */
/*                                                                            */
/* Input:  "point" is the where the mouse is.                                 */
/*         "transit" is a code telling if the mouse is enter, inside or       */
/*         leaving the view.                                                  */
/*         "message" is a pointer to the message or NULL.                     */
/******************************************************************************/
void APWindowModuleInfoList::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	int32 index;
	APWindowModuleInfoListItem *item;

	if (Window()->IsActive())
	{
		// Get the index the mouse is over
		index = IndexOf(point);

		// Did the mouse reach another item than the last?
		if (index != prevIndex)
		{
			if (prevIndex >= 0)
			{
				// Call the MouseOut() function
				item = (APWindowModuleInfoListItem *)ItemAt(prevIndex);
				if (item != NULL)
					item->MouseOut();
			}

			prevIndex = index;
		}

		// Now get the item and call the MouseMoved() function
		if (index >= 0)
		{
			item = (APWindowModuleInfoListItem *)ItemAt(index);
			item->MouseMoved(this, index, point);
		}
	}
}
