/******************************************************************************/
/* APlayer Sample Info Instrument list view class.                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// APlayer headers
#include "ColumnListView.h"
#include "CLVColumnLabelView.h"

// Agent headers
#include "APListViewInstrument.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListViewInstrument::APListViewInstrument(BRect frame, CLVContainerView** containerView, uint32 resizingMode, uint32 flags, list_view_type type, bool hierarchical, bool horizontal, bool vertical, bool scrollViewCorner, border_style border, const BFont* labelFont) :
	ColumnListView(frame, containerView, NULL, resizingMode, flags, type, hierarchical, horizontal, vertical, scrollViewCorner, border, labelFont)
{
	// Set the font
	SetFont(be_fixed_font);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListViewInstrument::~APListViewInstrument(void)
{
}



/******************************************************************************/
/* KeyDown() is called for every key the user presses when the list has the   */
/*      focus. It parse key down and up so the list scrolls.                  */
/*                                                                            */
/* Input:  "bytes" is a pointer to the UTF-8 character.                       */
/*         "numBytes" is the number of bytes the key has.                     */
/******************************************************************************/
void APListViewInstrument::KeyDown(const char *bytes, int32 numBytes)
{
	if (numBytes == 1)
	{
		BRect curRect;

		// Get the showing rect
		curRect = Bounds();

		// Arrow down
		if (*bytes == B_DOWN_ARROW)
		{
			uint32 count;
			float totalHeight, lineHeight;

			// Get the number of items in the list
			count = CountItems();

			// Calculate the total number of pixels to use
			if (count > 0)
			{
				lineHeight  = ceil(ItemAt(0)->Height() + 1.0f);
				totalHeight = count * lineHeight - 1.0f;
			}
			else
			{
				lineHeight  = 0.0f;
				totalHeight = 0.0f;
			}

			if (curRect.bottom < totalHeight)
				ScrollBy(0.0f, lineHeight);

			return;
		}

		// Arrow up
		if (*bytes == B_UP_ARROW)
		{
			if (curRect.top > 0.0f)
			{
				if (CountItems() > 0)
					ScrollBy(0.0f, -(ceil(ItemAt(0)->Height() + 1.0f)));
			}

			return;
		}

		// Tab
		if (*bytes == B_TAB)
			return;
	}

	// Call the base class
	ColumnListView::KeyDown(bytes, numBytes);
}
