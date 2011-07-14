/******************************************************************************/
/* SIDDiskButton implementation file.                                         */
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

// Player headers
#include "SIDDiskButton.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDDiskButton::SIDDiskButton(PResource *res, int32 normalIconID, int32 pressedIconID, BRect frame, BMessage *message, uint32 resizingMode, uint32 flags) : BPictureButton(frame, NULL, new BPicture(), new BPicture(), message, B_ONE_STATE_BUTTON, resizingMode, flags)
{
	uint8 icon[256];

	// Get the normal icon from the resource
	res->GetItem(P_RES_SMALL_ICON, normalIconID, icon, sizeof(icon));

	// Set the images to the bitmaps
	imageNormal = new BBitmap(BRect(0.0f, 0.0f, PICHSIZE - 1.0f, PICVSIZE - 1.0f), B_COLOR_8_BIT);
	if (imageNormal == NULL)
		throw PMemoryException();

	imageNormal->SetBits(icon, PICHSIZE * PICVSIZE, 0, B_COLOR_8_BIT);

	// Get the pressed icon from the resource
	res->GetItem(P_RES_SMALL_ICON, pressedIconID, icon, sizeof(icon));

	imagePressed = new BBitmap(BRect(0.0f, 0.0f, PICHSIZE - 1.0f, PICVSIZE - 1.0f), B_COLOR_8_BIT);
	if (imagePressed == NULL)
		throw PMemoryException();

	imagePressed->SetBits(icon, PICHSIZE * PICVSIZE, 0, B_COLOR_8_BIT);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDDiskButton::~SIDDiskButton(void)
{
	delete imagePressed;
	delete imageNormal;
}



/******************************************************************************/
/* AttachedToWindow() is called when the button is attached to the window. It */
/*         will draw all the different states the button can be in.           */
/******************************************************************************/
void SIDDiskButton::AttachedToWindow(void)
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
	DrawFrame(PICHSIZE + HSPACE, height, false);
	view->SetDrawingMode(B_OP_OVER);
	view->DrawBitmap(imageNormal, BPoint(left, top));
	view->SetDrawingMode(B_OP_COPY);
	normalPic = view->EndPicture();

	// Draw selected button
	view->BeginPicture(new BPicture());
	DrawFrame(PICHSIZE + HSPACE, height, true);
	view->SetDrawingMode(B_OP_OVER);
	view->DrawBitmap(imagePressed, BPoint(left + 1.0f, top + 1.0f));
	view->SetDrawingMode(B_OP_COPY);
	selectedPic = view->EndPicture();

	// Draw normal disabled button
	view->BeginPicture(new BPicture());
	view->DrawPicture(normalPic, BPoint(0.0f, 0.0f));
	view->SetDrawingMode(B_OP_OVER);
	view->SetHighColor(BeButtonGrey);
	view->FillRect(BRect(0.0f, 0.0f, PICHSIZE + HSPACE, height), B_MIXED_COLORS);
	view->SetDrawingMode(B_OP_COPY);
	view->SetHighColor(Black);
	normalDisPic = view->EndPicture();

	// Draw selected disabled button
	view->BeginPicture(new BPicture());
	view->DrawPicture(selectedPic, BPoint(0.0f, 0.0f));
	view->SetDrawingMode(B_OP_OVER);
	view->SetHighColor(BeLightShadow);
	view->FillRect(BRect(0.0f, 0.0f, PICHSIZE + HSPACE, height), B_MIXED_COLORS);
	view->SetDrawingMode(B_OP_COPY);
	view->SetHighColor(Black);
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
void SIDDiskButton::DrawFrame(float width, float height, bool selected)
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
