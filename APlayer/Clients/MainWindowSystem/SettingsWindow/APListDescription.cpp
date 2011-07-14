/******************************************************************************/
/* APlayer Settings description list class.                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "ColumnListView.h"

// Client headers
#include "APListDescription.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListDescription::APListDescription(BRect frame, CLVContainerView** containerView, uint32 resizingMode, uint32 flags, list_view_type type, bool hierarchical, bool horizontal, bool vertical, bool scrollViewCorner, border_style border, const BFont* labelFont) :
	ColumnListView(frame, containerView, NULL, resizingMode, flags, type, hierarchical, horizontal, vertical, scrollViewCorner, border, labelFont)
{
	font_height fontAttr;

	be_plain_font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListDescription::~APListDescription(void)
{
}



/******************************************************************************/
/* KeyDown() is called for every key the user presses when the list has the   */
/*      focus. It parse key down and up so it list scrolls.                   */
/*                                                                            */
/* Input:  "bytes" is a pointer to the UTF-8 character.                       */
/*         "numBytes" is the number of bytes the key has.                     */
/******************************************************************************/
void APListDescription::KeyDown(const char *bytes, int32 numBytes)
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
	}

	ColumnListView::KeyDown(bytes, numBytes);
}
