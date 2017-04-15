/******************************************************************************/
/* Instrument Info list item class.                                           */
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
#include "Colors.h"

// APlayerKit headers
#include "APAddOns.h"
#include "Layout.h"

// Santa headers
#include <santa/CLVListItem.h>

// Client headers
#include "APListItemInstrument.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListItemInstrument::APListItemInstrument(float minHeight, APInstInfo *info, uint32 num) : CLVListItem(0, false, false, max(FIXED_FONT_HEIGHT, minHeight))
{
	bool *useTab;
	int32 i, sampNum = 0;

	// Remember the arguments
	instName = info->name;
	instNum  = num;

	// Initialize the temp table
	useTab = new bool[32768];
	memset(useTab, 0, 32768 * sizeof(bool));

	// Scan the sample number array to find all the samples used
	for (i = 0; i < 10 * 12; i++)
	{
		if (info->notes[i] >= 0)
			useTab[info->notes[i]] = true;
	}

	// Count the samples
	for (i = 0; i < 32768; i++)
	{
		if (useTab[i])
			sampNum++;
	}

	// Remember the number
	usedSamples = sampNum;

	// Clean up again
	delete[] useTab;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListItemInstrument::~APListItemInstrument(void)
{
}



/******************************************************************************/
/* SortFunction() is called when to sort a single item.                       */
/*                                                                            */
/* Input :  "item1" is the first item.                                        */
/*          "item2" is the second item.                                       */
/*          "keyColumn" is the column to sort after.                          */
/*                                                                            */
/* Output:  -1 if I1 < I2, 0 if I1 == I2 and +1 if I1 > I2.                   */
/******************************************************************************/
int APListItemInstrument::SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn)
{
	APListItemInstrument *testItem1, *testItem2;
	int result;

	// Cast the items
	testItem1 = (APListItemInstrument *)item1;
	testItem2 = (APListItemInstrument *)item2;

	switch (keyColumn)
	{
		// Sort after instrument number
		case 0:
		{
			result = (testItem1->instNum < testItem2->instNum) ? -1 : 1;
			break;
		}

		// Sort after instrument name
		case 1:
		{
			result = testItem1->instName.CompareNoCase(testItem2->instName);
			break;
		}

		// Sort after number of samples
		case 2:
		{
			result = (testItem1->usedSamples < testItem2->usedSamples) ? -1 : 1;
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
/* Input :  "owner" is the view to draw into.                                 */
/*          "itemColumnRect" is the size of the column.                       */
/*          "columnIndex" is the column to draw.                              */
/*          "complete" indicates if it should draw every pixel.               */
/******************************************************************************/
void APListItemInstrument::DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete)
{
	BRegion region;
	PString temp;
	char *strPtr;

	// Initialize drawing
	owner->SetLowColor(White);
	owner->SetDrawingMode(B_OP_COPY);
	owner->SetFont(be_fixed_font);

	// Draw the background color
	if (complete)
	{
		owner->SetHighColor(White);
		owner->FillRect(itemColumnRect);
	}

	// Draw the item
	region.Include(itemColumnRect);
	owner->ConstrainClippingRegion(&region);

	switch (columnIndex)
	{
		// Instrument Number
		case 0:
		{
			temp.SetUNumber(instNum);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Instrument name
		case 1:
		{
			owner->SetHighColor(Black);
			owner->DrawString((strPtr = instName.GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
			instName.FreeBuffer(strPtr);
			break;
		}

		// Samples used
		case 2:
		{
			temp.SetUNumber(usedSamples);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}
	}

	owner->ConstrainClippingRegion(NULL);
}



/******************************************************************************/
/* Update() is called when the width or height has changed.                   */
/*                                                                            */
/* Input :  "owner" is the owner view.                                        */
/*          "font" is the font used in the column list view.                  */
/******************************************************************************/
void APListItemInstrument::Update(BView *owner, const BFont *font)
{
	font_height fontAttr;
	float fontHeight;

	CLVListItem::Update(owner, font);

	font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
	textOffset = ceil(fontAttr.ascent) + (Height() - fontHeight) / 2.0f;
}
