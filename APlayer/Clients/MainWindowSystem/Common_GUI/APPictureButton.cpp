/******************************************************************************/
/* APPictureButton implementation file.                                       */
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
#include "PResource.h"
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Client headers
#include "APPictureButton.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APPictureButton::APPictureButton(PResource *res, int32 iconID, BRect frame, BMessage *message, uint32 behaviour, uint32 resizingMode, uint32 flags) : BPictureButton(frame, NULL, new BPicture(), new BPicture(), message, behaviour, resizingMode, flags)
{
	uint8 icon[256];

	// Get the icon from the resource
	res->GetItem(P_RES_SMALL_ICON, iconID, icon, sizeof(icon));

	// Set the image to the bitmap
	image = new BBitmap(BRect(0.0f, 0.0f, PICHSIZE - 1.0f, PICVSIZE - 1.0f), B_COLOR_8_BIT);
	if (image == NULL)
		throw PMemoryException();

	image->SetBits(icon, PICHSIZE * PICVSIZE, 0, B_COLOR_8_BIT);

	// Set the default width
	width = PICHSIZE + HSPACE;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APPictureButton::~APPictureButton(void)
{
	delete image;
}



/******************************************************************************/
/* SetWidth() will set a new width in the picture.                            */
/*                                                                            */
/* Input:  "newWidth" is the new width.                                       */
/******************************************************************************/
void APPictureButton::SetWidth(float newWidth)
{
	width = newWidth;
}



/******************************************************************************/
/* AttachedToWindow() is called when the button is attached to the window. It */
/*         will draw all the different states the button can be in.           */
/******************************************************************************/
void APPictureButton::AttachedToWindow(void)
{
	font_height fh;
	float fontHeight, height;
	float left, top;

	// Get view it's attached to
	view = Parent();

	// Get other needed information
	view->GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));
	height = max(PICVSIZE, fontHeight) + VSPACE;

	// Calculate left and top values
	left = HSPACE / 2.0f;
	top  = (height - PICVSIZE) / 2.0f;

	// Draw normal button
	view->BeginPicture(new BPicture());
	DrawFrame(width, height, false);
	view->SetDrawingMode(B_OP_OVER);
	view->DrawBitmap(image, BPoint(left, top));
	view->SetDrawingMode(B_OP_COPY);
	normalPic = view->EndPicture();

	// Draw selected button
	view->BeginPicture(new BPicture());
	DrawFrame(width, height, true);
	view->SetDrawingMode(B_OP_OVER);
	view->DrawBitmap(image, BPoint(left + 1.0f, top + 1.0f));
	view->SetDrawingMode(B_OP_COPY);
	selectedPic = view->EndPicture();

	// Draw normal disabled button
	view->BeginPicture(new BPicture());
	view->DrawPicture(normalPic, BPoint(0.0f, 0.0f));
	view->SetDrawingMode(B_OP_OVER);
	view->SetHighColor(BeButtonGrey);
	view->FillRect(BRect(0.0f, 0.0f, width, height), B_MIXED_COLORS);
	view->SetDrawingMode(B_OP_COPY);
	normalDisPic = view->EndPicture();

	// Draw selected disabled button
	view->BeginPicture(new BPicture());
	view->DrawPicture(selectedPic, BPoint(0.0f, 0.0f));
	view->SetDrawingMode(B_OP_OVER);
	view->SetHighColor(BeLightShadow);
	view->FillRect(BRect(0.0f, 0.0f, width, height), B_MIXED_COLORS);
	view->SetDrawingMode(B_OP_COPY);
	selectedDisPic = view->EndPicture();

	// Change the pictures
	SetEnabledOff(normalPic);
	SetEnabledOn(selectedPic);
	SetDisabledOff(normalDisPic);
	SetDisabledOn(selectedDisPic);

	// Now call inherited version
	BPictureButton::AttachedToWindow();
}



/******************************************************************************/
/* DrawFrame() will draw a single 3D frame.                                   */
/*                                                                            */
/* Input:  "width" is the width of the frame.                                 */
/*         "height" is the height of the frame.                               */
/*         "selected" set to true if it's a selected frame.                   */
/******************************************************************************/
void APPictureButton::DrawFrame(float width, float height, bool selected)
{
	// Draw the frame
	if (selected)
	{
		// Selected
		view->SetHighColor(BeHighlight);
		view->StrokeRect(BRect(0.0f, 0.0f, width - 2.0f, height - 2.0f));
		view->SetHighColor(BeDarkShadow);
		view->StrokeRect(BRect(1.0f, 1.0f, width - 1.0f, height - 1.0f));

		// Draw the background
		view->SetHighColor(BeListSelectGrey);
		view->FillRect(BRect(2.0f, 2.0f, width - 3.0f, height - 3.0f));
	}
	else
	{
		// Normal
		view->SetHighColor(BeHighlight);
		view->StrokeRect(BRect(1.0f, 1.0f, width - 1.0f, height - 1.0f));
		view->SetHighColor(BeDarkShadow);
		view->StrokeRect(BRect(0.0f, 0.0f, width - 2.0f, height - 2.0f));

		// Draw the background
		view->SetHighColor(BeBackgroundGrey);
		view->FillRect(BRect(2.0f, 2.0f, width - 3.0f, height - 3.0f));
	}
}
