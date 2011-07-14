/******************************************************************************/
/* APlayer Settings Add-On list item class.                                   */
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

// APlayerKit headers
#include "CLVListItem.h"
#include "Layout.h"
#include "Colors.h"

// Client headers
#include "APGlobalData.h"
#include "APListItemAddOns.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Initialize static variables                                                */
/******************************************************************************/
int32 APListItemAddOns::imageCount = 0;
BBitmap *APListItemAddOns::imageCheck = NULL;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListItemAddOns::APListItemAddOns(PResource *res, float minHeight, APAddOnInformation *info, int32 num) : CLVListItem(0, false, false, max(PLAIN_FONT_HEIGHT, minHeight))
{
	// Initialize the member variables
	addOnInfo = info;
	listNum   = num;
	inUse     = false;

	version.Format("%*.*f", 3, 2, info->version);
	height = max(PLAIN_FONT_HEIGHT, minHeight);

	// Create the bitmap object
	if (imageCount == 0)
	{
		int32 len;
		uint8 *bitmap;
		BMessage msg;

		// Checkmark
		len    = res->GetItemLength(P_RES_BITMAP, IDB_CHECKMARK);
		bitmap = new uint8[len];
		if (bitmap == NULL)
			throw PMemoryException();

		res->GetItem(P_RES_BITMAP, IDB_CHECKMARK, bitmap, len);
		msg.Unflatten((char *)bitmap);
		imageCheck = new BBitmap(&msg);
		if (imageCheck == NULL)
			throw PMemoryException();

		delete[] bitmap;
	}

	imageCount++;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListItemAddOns::~APListItemAddOns(void)
{
	ASSERT(imageCount > 0);

	imageCount--;
	if (imageCount == 0)
	{
		// Checkmark
		delete imageCheck;
		imageCheck = NULL;
	}
}



/******************************************************************************/
/* GetItemInformation() returns the information about this item.              */
/*                                                                            */
/* Input:  "num" is a reference to store the list number.                     */
/*                                                                            */
/* Output: A pointer to the information.                                      */
/******************************************************************************/
APAddOnInformation *APListItemAddOns::GetItemInformation(int32 &num) const
{
	num = listNum;
	return (addOnInfo);
}



/******************************************************************************/
/* SetInUseFlag() change the "inUse" flag.                                    */
/*                                                                            */
/* Input:  "flag" is what to set the flag to.                                 */
/******************************************************************************/
void APListItemAddOns::SetInUseFlag(bool flag)
{
	inUse = flag;
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
int APListItemAddOns::SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn)
{
	APListItemAddOns *testItem1, *testItem2;
	int32 result;

	// Cast the items
	testItem1 = (APListItemAddOns *)item1;
	testItem2 = (APListItemAddOns *)item2;

	switch (keyColumn)
	{
		// Sort after active
		case 0:
			if (testItem1->addOnInfo->enabled == testItem2->addOnInfo->enabled)
				result = testItem1->addOnInfo->name.CompareNoCase(testItem2->addOnInfo->name);
			else
				result = testItem1->addOnInfo->enabled ? 1 : -1;
			break;

		// Sort after name
		case 1:
			result = testItem1->addOnInfo->name.CompareNoCase(testItem2->addOnInfo->name);
			break;

		// Sort after version
		case 2:
			if (testItem1->addOnInfo->version == testItem2->addOnInfo->version)
				result = testItem1->addOnInfo->name.CompareNoCase(testItem2->addOnInfo->name);
			else
				result = (testItem1->addOnInfo->version < testItem2->addOnInfo->version) ? -1 : 1;
			break;

		// Unknown sort order
		default:
			result = 0;
			break;
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
void APListItemAddOns::DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete)
{
	BRegion region;
	bool selected;
	rgb_color backCol, frontCol;
	float x;
	char *strPtr;

	// Get needed informations
	selected = IsSelected();

	// Find the background color
	if (selected)
		backCol = BeListSelectGrey;
	else
		backCol = White;

	// Find the front color
	if (inUse)
		frontCol = Blue;
	else
		frontCol = Black;

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
			if (addOnInfo->enabled)
			{
				if (Height() > height)
					itemColumnRect.top += ceil((Height() - height) / 2.0f);

				owner->SetDrawingMode(B_OP_OVER);
				owner->DrawBitmap(imageCheck, BPoint(itemColumnRect.left + 4.0f, itemColumnRect.top));
				owner->SetDrawingMode(B_OP_COPY);
			}
			break;
		}

		// Add-On name
		case 1:
		{
			owner->SetHighColor(frontCol);
			owner->DrawString((strPtr = addOnInfo->name.GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
			addOnInfo->name.FreeBuffer(strPtr);
			break;
		}

		// Version
		case 2:
		{
			strPtr = version.GetString();
			x = (itemColumnRect.Width() - 4.0f - owner->StringWidth(strPtr)) / 2.0f;

			owner->SetHighColor(frontCol);
			owner->DrawString(strPtr, BPoint(itemColumnRect.left + x, itemColumnRect.top + textOffset));

			version.FreeBuffer(strPtr);
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
void APListItemAddOns::Update(BView *owner, const BFont *font)
{
	font_height fontAttr;
	float fontHeight;

	CLVListItem::Update(owner, font);

	font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
	textOffset = ceil(fontAttr.ascent) + (Height() - fontHeight) / 2.0f;
}
