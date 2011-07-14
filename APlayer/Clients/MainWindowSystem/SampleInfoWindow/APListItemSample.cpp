/******************************************************************************/
/* Sample Info list item class.                                               */
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
#include "CLVListItem.h"
#include "Layout.h"

// Client headers
#include "APListItemSample.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Initialize static variables                                                */
/******************************************************************************/
int32 APListItemSample::imageCount = 0;
BBitmap *APListItemSample::imageLoop = NULL;
BBitmap *APListItemSample::imagePingPong = NULL;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListItemSample::APListItemSample(PResource *resource, float minHeight, APSampleInfo *info, uint32 num) : CLVListItem(0, false, false, max(FIXED_FONT_HEIGHT, minHeight))
{
	// Remember the arguments
	res        = resource;
	sampleInfo = *info;
	sampleNum  = num;

	// Create the bitmap objects
	if (imageCount == 0)
	{
		int32 len;
		uint8 *bitmap;
		BMessage msg;

		// Loop
		len    = res->GetItemLength(P_RES_BITMAP, IDB_LOOP);
		bitmap = new uint8[len];
		if (bitmap == NULL)
			throw PMemoryException();

		res->GetItem(P_RES_BITMAP, IDB_LOOP, bitmap, len);
		msg.Unflatten((char *)bitmap);
		imageLoop = new BBitmap(&msg);
		if (imageLoop == NULL)
			throw PMemoryException();

		delete[] bitmap;

		// Ping-Pong
		len    = res->GetItemLength(P_RES_BITMAP, IDB_PINGPONG);
		bitmap = new uint8[len];
		if (bitmap == NULL)
			throw PMemoryException();

		res->GetItem(P_RES_BITMAP, IDB_PINGPONG, bitmap, len);
		msg.Unflatten((char *)bitmap);
		imagePingPong = new BBitmap(&msg);
		if (imagePingPong == NULL)
			throw PMemoryException();

		delete[] bitmap;
	}

	imageCount++;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListItemSample::~APListItemSample(void)
{
	ASSERT(imageCount > 0);

	imageCount--;
	if (imageCount == 0)
	{
		// Ping-Pong
		delete imagePingPong;
		imagePingPong = NULL;

		// Loop
		delete imageLoop;
		imageLoop = NULL;
	}
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
int APListItemSample::SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn)
{
	APListItemSample *testItem1, *testItem2;
	int result = 0;

	// Cast the items
	testItem1 = (APListItemSample *)item1;
	testItem2 = (APListItemSample *)item2;

	switch (keyColumn)
	{
		// Sort after sample number
		case 0:
		{
			result = (testItem1->sampleNum < testItem2->sampleNum) ? -1 : 1;
			break;
		}

		// Sort after sample name
		case 1:
		{
			result = testItem1->sampleInfo.name.CompareNoCase(testItem2->sampleInfo.name);
			break;
		}

		// Sort after length
		case 2:
		{
			result = (testItem1->sampleInfo.length < testItem2->sampleInfo.length) ? -1 : 1;
			break;
		}

		// Sort after loop start
		case 3:
		{
			result = (testItem1->sampleInfo.loopStart < testItem2->sampleInfo.loopStart) ? -1 : 1;
			break;
		}

		// Sort after loop length
		case 4:
		{
			result = (testItem1->sampleInfo.loopLength < testItem2->sampleInfo.loopLength) ? -1 : 1;
			break;
		}

		// Sort after bits
		case 5:
		{
			result = (testItem1->sampleInfo.bitSize < testItem2->sampleInfo.bitSize) ? -1 : 1;
			break;
		}

		// Sort after volume
		case 6:
		{
			result = (testItem1->sampleInfo.volume < testItem2->sampleInfo.volume) ? -1 : 1;
			break;
		}

		// Sort after panning
		case 7:
		{
			result = (testItem1->sampleInfo.panning < testItem2->sampleInfo.panning) ? -1 : 1;
			break;
		}

		// Sort after C-4
		case 8:
		{
			result = (testItem1->sampleInfo.middleC < testItem2->sampleInfo.middleC) ? -1 : 1;
			break;
		}

		// Sort after type
		case 10:
		{
			PString type1 = testItem1->GetTypeString();
			PString type2 = testItem2->GetTypeString();

			result = type1.CompareNoCase(type2);
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
/* GetSampleInfo() returns the sample information about the sample.           */
/*                                                                            */
/* Output:  The sample information.                                           */
/******************************************************************************/
const APSampleInfo *APListItemSample::GetSampleInfo(void) const
{
	return (&sampleInfo);
}



/******************************************************************************/
/* DrawItemColumn() is called every time a column needs to be drawn.          */
/*                                                                            */
/* Input :  "owner" is the view to draw into.                                 */
/*          "itemColumnRect" is the size of the column.                       */
/*          "columnIndex" is the column to draw.                              */
/*          "complete" indicates if it should draw every pixel.               */
/******************************************************************************/
void APListItemSample::DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete)
{
	BRegion region;
	bool selected;
	rgb_color backCol;
	PString temp;
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
	owner->SetFont(be_fixed_font);

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
		// Sample Number
		case 0:
		{
			temp.SetUNumber(sampleNum);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Sample name
		case 1:
		{
			owner->SetHighColor(Black);
			owner->DrawString((strPtr = sampleInfo.name.GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
			sampleInfo.name.FreeBuffer(strPtr);
			break;
		}

		// Length
		case 2:
		{
			temp.SetUNumber(sampleInfo.bitSize == 16 ? sampleInfo.length * 2 : sampleInfo.length);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Loop Start
		case 3:
		{
			temp.SetUNumber(sampleInfo.bitSize == 16 ? sampleInfo.loopStart * 2 : sampleInfo.loopStart);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			break;
		}

		// Loop Length
		case 4:
		{
			temp.SetUNumber(sampleInfo.bitSize == 16 ? sampleInfo.loopLength : sampleInfo.loopLength);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Bits
		case 5:
		{
			temp.SetUNumber(sampleInfo.bitSize);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Volume
		case 6:
		{
			temp.SetUNumber(sampleInfo.volume);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Panning
		case 7:
		{
			if (sampleInfo.panning == -1)
				temp = "-";
			else
				temp.SetUNumber(sampleInfo.panning);

			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Middle C
		case 8:
		{
			temp.SetUNumber(sampleInfo.middleC);
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.right - 2.0f - owner->StringWidth(strPtr), itemColumnRect.top + textOffset));
			temp.FreeBuffer(strPtr);
			break;
		}

		// Info
		case 9:
		{
			// Center the image
			if (Height() > 12.0f)
				itemColumnRect.top += ceil((Height() - 12.0f) / 2.0f);

			// Initialize drawing
			owner->SetDrawingMode(B_OP_OVER);

			// Sample is looping
			if (sampleInfo.flags & APSAMP_LOOP)
				owner->DrawBitmap(imageLoop, BPoint(itemColumnRect.left + 4.0f, itemColumnRect.top));

			// Sample have ping-pong loop
			if (sampleInfo.flags & APSAMP_PINGPONG)
				owner->DrawBitmap(imagePingPong, BPoint(itemColumnRect.left + 4.0f + 12.0f + 2.0f, itemColumnRect.top));

			// Sample is in stereo
//			if (sampleInfo.flags & APSAMP_STEREO)
//				owner->DrawBitmap(imageStereo, BPoint(itemColumnRect.left + 4.0 + (12.0 + 2.0) * 2.0, itemColumnRect.top));

			// End drawing
			owner->SetDrawingMode(B_OP_COPY);
			break;
		}

		// Type
		case 10:
		{
			temp   = GetTypeString();
			strPtr = temp.GetString();
			owner->SetHighColor(Black);
			owner->DrawString(strPtr, BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
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
void APListItemSample::Update(BView *owner, const BFont *font)
{
	font_height fontAttr;
	float fontHeight;

	CLVListItem::Update(owner, font);

	font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
	textOffset = ceil(fontAttr.ascent) + (Height() - fontHeight) / 2.0f;
}



/******************************************************************************/
/* GetTypeString() returns a string with the type of the sample.              */
/*                                                                            */
/* Output:  is the string containing the type.                                */
/******************************************************************************/
PString APListItemSample::GetTypeString(void)
{
	PString temp;
	int32 id;

	switch (sampleInfo.type)
	{
		case apSample:
		default:
		{
			id = IDS_SI_SAMP_SAMPLE;
			break;
		}

		case apSynth:
		{
			id = IDS_SI_SAMP_SYNTH;
			break;
		}

		case apHybrid:
		{
			id = IDS_SI_SAMP_HYBRID;
			break;
		}
	}

	temp.LoadString(res, id);
	return (temp);
}
