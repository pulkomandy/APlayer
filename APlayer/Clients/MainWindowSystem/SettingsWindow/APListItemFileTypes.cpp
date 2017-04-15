/******************************************************************************/
/* APlayer Settings FileTypes list item class.                                */
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

// APlayerKit headers
#include "APFileTypes.h"
#include "Colors.h"

// Santa
#include <santa/CLVListItem.h>

// Client headers
#include "APListItemFileTypes.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListItemFileTypes::APListItemFileTypes(float minHeight, APSystemFileType *info) : CLVListItem(0, false, false, max(17.0f, minHeight))
{
	// Remember the arguments
	fileType = info;

	height = max(17.0f, minHeight);

	// Create the bitmap object
	icon = new BBitmap(BRect(0.0f, 0.0f, 15.0f, 15.0f), B_CMAP8);
	if (icon == NULL)
		throw PMemoryException();

	icon->SetBits(info->icon, 16 * 16, 0, B_CMAP8);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListItemFileTypes::~APListItemFileTypes(void)
{
	delete icon;
}



/******************************************************************************/
/* SortFunction() is called when to sort a single item.                       */
/*                                                                            */
/* Input:  "item1" is the first item.                                         */
/*         "item2" is the second item.                                        */
/*         "keyColumn" is the column to sort after.                           */
/*                                                                            */
/* Output: -1 if I1 < I2, 0 if I1 == I2 and +1 if I1 > I2.                    */
/******************************************************************************/
int APListItemFileTypes::SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn)
{
	APListItemFileTypes *testItem1, *testItem2;
	int32 result;

	// Cast the items
	testItem1 = (APListItemFileTypes *)item1;
	testItem2 = (APListItemFileTypes *)item2;

	switch (keyColumn)
	{
		// Sort after description
		case 1:
		{
			result = testItem1->fileType->description.CompareNoCase(testItem2->fileType->description);
			break;
		}

		// Sort after preferred application
		case 2:
		{
			result = testItem1->fileType->preferredApp.CompareNoCase(testItem2->fileType->preferredApp);
			break;
		}

		// Sort after extensions
		case 3:
		{
			result = testItem1->fileType->extensions.CompareNoCase(testItem2->fileType->extensions);
			break;
		}

		// Unknown sort order
		default:
		{
			result = 0;
			break;
		}
	}

	return (result);
}



/******************************************************************************/
/* DrawItemColumn() is called every time a column needs to be drawn.          */
/*                                                                            */
/* Input:  "owner" is the view to draw into.                                  */
/*         "itemColumnRect" is the size of the column.                        */
/*         "columnIndex" is the column to draw.                               */
/*         "complete" indicates if it should draw every pixel.                */
/******************************************************************************/
void APListItemFileTypes::DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete)
{
	BRegion region;
	bool selected;
	rgb_color backCol;
	char *strPtr;

	// Get needed informations
	selected = IsSelected();

	// Find the background color
	if (selected)
		backCol = BeListSelectGrey;
	else
		backCol = White;

	// Initialize drawing
	owner->SetLowColor(backCol);
	owner->SetDrawingMode(B_OP_COPY);

	// Draw the background color
	if (selected || complete)
	{
		owner->SetHighColor(backCol);
		owner->FillRect(itemColumnRect);
	}

	// Draw the item
	region.Include(itemColumnRect);
	owner->ConstrainClippingRegion(&region);

	switch (columnIndex)
	{
		// Icon
		case 0:
		{
			if (Height() > height)
				itemColumnRect.top += ceil((Height() - height) / 2.0f);

			owner->SetDrawingMode(B_OP_OVER);
			owner->DrawBitmap(icon, BPoint(itemColumnRect.left + 4.0f, itemColumnRect.top));
			owner->SetDrawingMode(B_OP_COPY);
			break;
		}

		// Filetype
		case 1:
		{
			PString tempStr;

			if (!fileType->description.IsEmpty())
				tempStr = fileType->description;
			else
				tempStr = fileType->mime;

			owner->SetHighColor(Black);
			owner->DrawString((strPtr = tempStr.GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
			tempStr.FreeBuffer(strPtr);
			break;
		}

		// Preferred application
		case 2:
		{
			owner->SetHighColor(Black);
			owner->DrawString((strPtr = fileType->preferredApp.GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
			fileType->preferredApp.FreeBuffer(strPtr);
			break;
		}

		// File extensions
		case 3:
		{
			owner->SetHighColor(Black);
			owner->DrawString((strPtr = fileType->extensions.GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
			fileType->extensions.FreeBuffer(strPtr);
			break;
		}
	}

	owner->ConstrainClippingRegion(NULL);
}



/******************************************************************************/
/* Update() is called when the width or height has changed.                   */
/*                                                                            */
/* Input:  "owner" is the owner view.                                         */
/*         "font" is the font used in the column list view.                   */
/******************************************************************************/
void APListItemFileTypes::Update(BView *owner, const BFont *font)
{
	font_height fontAttr;
	float fontHeight;

	CLVListItem::Update(owner, font);

	font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
	textOffset = ceil(fontAttr.ascent) + (Height() - fontHeight) / 2.0f;
}
