/******************************************************************************/
/* ReverbViewSlider classes.                                                  */
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
#include "Layout.h"

// Agent headers
#include "ReverbViewSlider.h"


/******************************************************************************/
/* ReverbViewSlider class                                                     */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ReverbViewSlider::ReverbViewSlider(BRect frame, PString label, bool labelPos, BMessage *message, int32 minValue, int32 maxValue, float valWidth, thumb_style thumbType, uint32 resizingMode, uint32 flags)
 : BSlider(frame, NULL, NULL, message, minValue, maxValue, thumbType, resizingMode, flags)
{
	char *labelPtr;
	font_height fh;
	BRect rect;

	// Remember the label
	barLabel      = label;
	labelPosition = labelPos;

	// Calculate the pixel length of the label
	labelLength = StringWidth((labelPtr = label.GetString()));
	label.FreeBuffer(labelPtr);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));
	baseLine   = fh.ascent;

	// Find thumb width
	thumbHalfWidth = 0.0f;
	rect           = ThumbFrame();
	thumbHalfWidth = rect.Width() / 2.0f;

	// Remember the message
	sendMsg  = message;
	oldValue = -1;

	// Remember the value width
	valueWidth = valWidth;
	valueFunc  = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ReverbViewSlider::~ReverbViewSlider(void)
{
}



/******************************************************************************/
/* SetLabelWidth() will set the width of the label to a fixed size.           */
/*                                                                            */
/* Input:  "width" is the width of the label.                                 */
/******************************************************************************/
void ReverbViewSlider::SetLabelWidth(float width)
{
	labelLength = width;
}



/******************************************************************************/
/* SetValueFunction() will set the function to be called when the value       */
/*      change.                                                               */
/*                                                                            */
/* Input:  "func" is a pointer to the function.                               */
/*         "user" is a pointer to anything you want.                          */
/******************************************************************************/
void ReverbViewSlider::SetValueFunction(ValueFunc func, void *user)
{
	valueFunc = func;
	userData  = user;
}



/******************************************************************************/
/* BarFrame() returns the frame size of the bar.                              */
/*                                                                            */
/* Output: Is the size of the bar.                                            */
/******************************************************************************/
BRect ReverbViewSlider::BarFrame(void) const
{
	BRect rect;

	rect = BSlider::BarFrame();

	// Make space for the label
	if (labelPosition)
	{
		// On the left of the bar
		rect.left += (labelLength + thumbHalfWidth);

		if (valueWidth != 0.0f)
			rect.right -= (valueWidth + thumbHalfWidth);
	}
	else
	{
		// On the right of the bar
		rect.right -= (labelLength + thumbHalfWidth);
	}

	return (rect);
}



/******************************************************************************/
/* DrawText() draws the label to the right of the slider.                     */
/******************************************************************************/
void ReverbViewSlider::DrawText(void)
{
	BRect rect;
	BView *view;
	float x, y;
	float num1, num2;
	char *str;

	// Get the offscreen view to draw into
	view = OffscreenView();

	// Calculate the position of the label
	rect = ThumbFrame();

	num1 = rect.Height();
	num2 = max(fontHeight, num1);
	num1 = min(fontHeight, num1);
	y    = rect.top + ((num2 - num1) / 2.0f) + baseLine;

	if (y > rect.Height())
		y = rect.bottom;

	rect = BarFrame();

	if (labelPosition == false)
		x = rect.right + thumbHalfWidth + HSPACE;
	else
		x = 0.0f;

	view->SetHighColor(Black);
	view->DrawString((str = barLabel.GetString()), BPoint(x, y));
	barLabel.FreeBuffer(str);

	// Draw the value
	if (valueWidth != 0.0f)
	{
		PString val;

		if (valueFunc != NULL)
			val = valueFunc(userData, oldValue);
		else
			val.SetNumber(oldValue);

		// Draw the value string
		rect = BarFrame();
		view->DrawString((str = val.GetString()), BPoint(rect.right + thumbHalfWidth + HSPACE, y));
		val.FreeBuffer(str);
	}
}



/******************************************************************************/
/* DrawBar() draws the slider bar.                                            */
/******************************************************************************/
void ReverbViewSlider::DrawBar(void)
{
	BWindow *attachedWindow;
	int32 newValue;

	// Call inherited class
	BSlider::DrawBar();

	// Send the message
	attachedWindow = Window();
	if (attachedWindow != NULL)
	{
		newValue = Value();
		if (newValue != oldValue)
		{
			oldValue = newValue;
			attachedWindow->PostMessage(sendMsg);
		}
	}
}
