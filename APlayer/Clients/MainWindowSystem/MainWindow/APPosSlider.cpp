/******************************************************************************/
/* APlayer Position Slider class.                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "Colors.h"

// Client headers
#include "APWindowMain.h"
#include "APPosSlider.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APPosSlider::APPosSlider(BRect frame, BMessage *message, uint32 resizingMode, uint32 flags) : BSlider(frame, NULL, NULL, message, 0, 99, B_BLOCK_THUMB, resizingMode, flags)
{
	// Set fill color
	UseFillColor(true, &LightBlue);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APPosSlider::~APPosSlider(void)
{
}



/******************************************************************************/
/* MouseDown() is called when the mouse is pressed down on the slider.        */
/*                                                                            */
/* Input:  "point" is the position where the mouse was when pressed down.     */
/******************************************************************************/
void APPosSlider::MouseDown(BPoint point)
{
	// Forbid slider updates when printing information
	if (IsEnabled())
	{
		((APWindowMain *)Window())->posSliderUpdate = false;
		oldValue = Value();
	}

	// Call the base class
	BSlider::MouseDown(point);
}



/******************************************************************************/
/* MouseUp() is called when the mouse button has been released.               */
/*                                                                            */
/* Input:  "point" is the position where the mouse was released.              */
/******************************************************************************/
void APPosSlider::MouseUp(BPoint point)
{
	// Enable slider updates again if the slider hasn't changed value
	if (IsEnabled() && (Value() == oldValue))
		((APWindowMain *)Window())->posSliderUpdate = true;

	// Call the base class
	BSlider::MouseUp(point);
}



/******************************************************************************/
/* DrawThumb() drawn the slider thumb.                                        */
/******************************************************************************/
void APPosSlider::DrawThumb(void)
{
	BRect rect;
	BView *view;

	// Get the frame rectangle of the thumb
	// and the offscreen view
	rect = ThumbFrame();
	view = OffscreenView();

	// Draw the black shadow
	view->SetHighColor(Black);
	rect.top++;
	rect.left++;
	view->StrokeEllipse(rect);

	// Draw the dark grey edge
	view->SetHighColor(BeDarkShadow);
	rect.bottom--;
	rect.right--;
	view->StrokeEllipse(rect);

	// Fill the inside of the thumb
	view->SetHighColor(BeButtonGrey);
	rect.InsetBy(1.0f, 1.0f);
	view->FillEllipse(rect);
}



/******************************************************************************/
/* DrawFocusMark() drawn the focus mark in the slider thumb.                  */
/******************************************************************************/
void APPosSlider::DrawFocusMark(void)
{
	BRect rect;
	BView *view;
	rgb_color color;

	if (IsFocus())
	{
		// Get the frame rectangle of the thumb
		// and the offscreen view
		rect = ThumbFrame();
		view = OffscreenView();

		// Get the color to use
		color = keyboard_navigation_color();
		view->SetHighColor(color);

		// Draw a circle inside the thumb
		rect.InsetBy(2.0f, 2.0f);
		view->StrokeEllipse(rect);
	}
}
