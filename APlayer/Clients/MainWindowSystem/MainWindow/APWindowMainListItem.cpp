/******************************************************************************/
/* APWindowMainListItem implementation file.                                  */
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
#include "PTime.h"
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Client headers
#include "APWindowMainListItem.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowMainListItem::APWindowMainListItem(PString text, PString fileName)
{
	// Initialize the member variables
	itemNum      = 0;
	itemType     = apNormal;
	itemText     = text;
	itemFileName = fileName;
	timeSet      = false;
	playing      = false;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowMainListItem::~APWindowMainListItem(void)
{
}



/******************************************************************************/
/* SetItemNumber() sets the item to the number it have in the list.           */
/*                                                                            */
/* Input:  "number" is the list number the item have.                         */
/******************************************************************************/
void APWindowMainListItem::SetItemNumber(int32 number)
{
	itemNum = number;
}



/******************************************************************************/
/* SetPlayingFlag() sets/clears the playing flag in the item.                 */
/*                                                                            */
/* Input:  "flag" is what the playing flag has to be set to.                  */
/******************************************************************************/
void APWindowMainListItem::SetPlayingFlag(bool flag)
{
	playing = flag;
}



/******************************************************************************/
/* IsPlaying() returns the playing flag.                                      */
/*                                                                            */
/* Output: Returns true if the item is the playing item, else false.          */
/******************************************************************************/
bool APWindowMainListItem::IsPlaying(void) const
{
	return (playing);
}



/******************************************************************************/
/* SetTime() sets the time for the item.                                      */
/*                                                                            */
/* Input:  "time" is the new time.                                            */
/******************************************************************************/
void APWindowMainListItem::SetTime(PTimeSpan time)
{
	itemTime = time;
	timeSet  = true;
}



/******************************************************************************/
/* GetTime() returns the items current time.                                  */
/*                                                                            */
/* Output: The items time.                                                    */
/******************************************************************************/
PTimeSpan APWindowMainListItem::GetTime(void) const
{
	return (itemTime);
}



/******************************************************************************/
/* HaveTime() returns the time flag.                                          */
/*                                                                            */
/* Output: Returns true if the item have a time associated, false if not.     */
/******************************************************************************/
bool APWindowMainListItem::HaveTime(void) const
{
	return (timeSet);
}



/******************************************************************************/
/* GetText() returns the text shown to the user.                              */
/*                                                                            */
/* Input:  "offset" is a pointer to where to store the text offset or NULL if */
/*         you don't want it.                                                 */
/*                                                                            */
/* Output: The item text.                                                     */
/******************************************************************************/
PString APWindowMainListItem::GetText(float *offset) const
{
	if (offset != NULL)
		*offset = textOffset;

	return (itemText);
}



/******************************************************************************/
/* GetItemType() returns the type of the item.                                */
/*                                                                            */
/* Output: The item type.                                                     */
/******************************************************************************/
APWindowMainListItem::ItemType APWindowMainListItem::GetItemType(void) const
{
	return (itemType);
}



/******************************************************************************/
/* GetFileName() returns the filename in the item.                            */
/*                                                                            */
/* Output: The item filename.                                                 */
/******************************************************************************/
PString APWindowMainListItem::GetFileName(void) const
{
	return (itemFileName);
}



/******************************************************************************/
/* DrawItem() is called every time a item needs to be drawn.                  */
/*                                                                            */
/* Input :  "owner" is the view to draw into.                                 */
/*          "frame" is the size of the item.                                  */
/*          "complete" indicates if it should draw every pixel.               */
/******************************************************************************/
void APWindowMainListItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	BRegion region;
	bool selected;
	rgb_color backCol, itemCol;
	PString tempStr;
	char *timeStr, *nameStr;
	float timeLen;

	// Get needed informations
	selected = IsSelected();

	// Check to see if the item is selected
	if (selected)
		backCol = BeListSelectGrey;
	else
		backCol = owner->ViewColor();

	// Check to see if the item is the one that playing
	if (playing)
		itemCol = Blue;
	else
		itemCol = Black;

	// Initialize the pen
	owner->SetLowColor(backCol);
	owner->SetDrawingMode(B_OP_COPY);

	if (selected || complete)
	{
		// Draw the background color
		owner->SetHighColor(backCol);
		owner->FillRect(frame);
	}

	// Set the pen to the right color
	owner->SetHighColor(itemCol);

	// Draw the module time
	if (timeSet)
	{
		if (itemTime.GetTotalMilliSeconds() == 0)
			tempStr = " ?:??";
		else
		{
			PTimeSpan tempTime((itemTime.GetTotalMilliSeconds() + 500) / 1000 * 1000);
			tempStr.Format(" %Ld:%02d", tempTime.GetTotalMinutes(), tempTime.GetSeconds());
		}

		// Get and calculate the length of the time string
		timeStr = tempStr.GetString();
		timeLen = owner->StringWidth(timeStr);

		// Draw the time
		owner->MovePenTo(frame.right - 2.0f - timeLen, frame.top + textOffset);
		owner->DrawString(timeStr);
		tempStr.FreeBuffer(timeStr);

		// Subtract the length of the time string to the frame to draw in
		frame.right -= (timeLen + HSPACE);
	}

	// Tell the view about the new valid rect to draw in
	region.Include(frame);
	owner->ConstrainClippingRegion(&region);

	// Draw the file name
	if (itemNum == 0)
		tempStr = itemText;
	else
		tempStr = PString::CreateNumber(itemNum) + ". " + itemText;

	owner->MovePenTo(frame.left + 2.0f, frame.top + textOffset);
	owner->DrawString((nameStr = tempStr.GetString()));
	tempStr.FreeBuffer(nameStr);

	// Remove the temporary draw rect
	owner->ConstrainClippingRegion(NULL);
}



/******************************************************************************/
/* Update() is called when the width or height has changed.                   */
/*                                                                            */
/* Input :  "owner" is the owner view.                                        */
/*          "font" is the font used in the column list view.                  */
/******************************************************************************/
void APWindowMainListItem::Update(BView *owner, const BFont *font)
{
	font_height fontAttr;
	float fontHeight;

	// Call the base class
	BListItem::Update(owner, font);

	// Set the new text offset
	font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
	textOffset = ceil(fontAttr.ascent) + (Height() - fontHeight) / 2.0f;
}
