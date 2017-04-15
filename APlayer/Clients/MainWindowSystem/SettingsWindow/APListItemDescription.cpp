/******************************************************************************/
/* APlayer Settings description list item class.                              */
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

// APlayerKit headers
#include "Colors.h"

// Santa headers
#include <santa/CLVListItem.h>

// Client headers
#include "APListItemDescription.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListItemDescription::APListItemDescription(float minHeight, PString string) : CLVListItem(0, false, false, minHeight)
{
	columnText = string;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListItemDescription::~APListItemDescription(void)
{
}



/******************************************************************************/
/* DrawItemColumn() is called every time a column needs to be drawn.          */
/*                                                                            */
/* Input:  "owner" is the view to draw into.                                  */
/*         "itemColumnRect" is the size of the column.                        */
/*         "columnIndex" is the column to draw.                               */
/*         "complete" indicates if it should draw every pixel.                */
/******************************************************************************/
void APListItemDescription::DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete)
{
	BRegion region;
	char *strPtr;

	// Initialize drawing
	owner->SetLowColor(White);
	owner->SetDrawingMode(B_OP_COPY);

	// Draw the background color
	if (complete)
	{
		owner->SetHighColor(White);
		owner->FillRect(itemColumnRect);
	}

	// Draw the item
	region.Include(itemColumnRect);
	owner->ConstrainClippingRegion(&region);

	owner->SetHighColor(Black);
	owner->DrawString((strPtr = columnText.GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
	columnText.FreeBuffer(strPtr);

	owner->ConstrainClippingRegion(NULL);
}



/******************************************************************************/
/* Update() is called when the width or height has changed.                   */
/*                                                                            */
/* Input:  "owner" is the owner view.                                         */
/*         "font" is the font used in the column list view.                   */
/******************************************************************************/
void APListItemDescription::Update(BView *owner, const BFont *font)
{
	font_height fontAttr;
	float fontHeight;

	CLVListItem::Update(owner, font);

	font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
	textOffset = ceil(fontAttr.ascent) + (Height() - fontHeight) / 2.0f;
}
