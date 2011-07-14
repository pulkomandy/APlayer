/******************************************************************************/
/* SID ViewSlider classes.                                                    */
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
#include "PSettings.h"
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Player headers
#include "SIDViewFormular.h"
#include "SIDViewSlider.h"
#include "SIDView.h"


/******************************************************************************/
/* Extern global functions and variables                                      */
/******************************************************************************/
extern PSettings *sidSettings;



/******************************************************************************/
/* SIDViewSlider class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDViewSlider::SIDViewSlider(BRect frame, PString label, BMessage *message, int32 minValue, int32 maxValue, thumb_style thumbType, uint32 resizingMode, uint32 flags)
 : BSlider(frame, NULL, NULL, message, minValue, maxValue, thumbType, resizingMode, flags)
{
	char *labelPtr;
	font_height fh;
	BRect rect;

	// Remember the label
	barLabel    = label;
	barLabelStr = NULL;

	// Calculate the pixel length of the label
	labelLength = StringWidth((labelPtr = label.GetString()));
	label.FreeBuffer(labelPtr);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = ceil(fh.ascent + fh.descent);
	baseLine   = fh.ascent;

	// Find thumb width
	thumbHalfWidth = 0.0f;
	rect           = ThumbFrame();
	thumbHalfWidth = (rect.right - rect.left) / 2.0f;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDViewSlider::~SIDViewSlider(void)
{
	barLabel.FreeBuffer(barLabelStr);
}



/******************************************************************************/
/* Label() returns the label text.                                            */
/*                                                                            */
/* Output:  A pointer to the label.                                           */
/******************************************************************************/
const char *SIDViewSlider::Label(void) const
{
	barLabel.FreeBuffer(barLabelStr);
	((SIDViewSlider *)this)->barLabelStr = barLabel.GetString();

	return (barLabelStr);
}



/******************************************************************************/
/* SetLabelWidth() will set the width of the label to a fixed size.           */
/*                                                                            */
/* Input:  "width" is the width of the label.                                 */
/******************************************************************************/
void SIDViewSlider::SetLabelWidth(float width)
{
	labelLength = width;
}



/******************************************************************************/
/* BarFrame() returns the frame size of the bar.                              */
/*                                                                            */
/* Output: Is the size of the bar.                                            */
/******************************************************************************/
BRect SIDViewSlider::BarFrame(void) const
{
	BRect rect;

	rect = BSlider::BarFrame();

	// Make space for the label right of the bar
	rect.right -= (labelLength + thumbHalfWidth);

	return (rect);
}



/******************************************************************************/
/* DrawText() draws the label to the right of the slider.                     */
/******************************************************************************/
void SIDViewSlider::DrawText(void)
{
	BRect rect;
	BView *view;
	float x, y;
	float num1, num2;
	char *labelPtr;

	// Get the offscreen view to draw into
	view = OffscreenView();

	// Calculate the position of the label
	rect = ThumbFrame();
	x    = rect.Width() / 2.0f;

	num1 = rect.Height();
	num2 = max(fontHeight, num1);
	num1 = min(fontHeight, num1);
	y    = rect.top + ((num2 - num1) / 2.0f) + baseLine;

	if (y > rect.Height())
		y = rect.bottom;

	rect = BarFrame();
	x   += (rect.right + HSPACE);

	view->SetHighColor(Black);
	view->DrawString((labelPtr = barLabel.GetString()), BPoint(x, y));
	barLabel.FreeBuffer(labelPtr);
}





/******************************************************************************/
/* SIDViewSliderFilter class                                                  */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDViewSliderFilter::SIDViewSliderFilter(SIDViewFormular *view, uint8 num, BRect frame, PString label, BMessage *message, int32 minValue, int32 maxValue, thumb_style thumbType, uint32 resizingMode, uint32 flags)
 : SIDViewSlider(frame, label, message, minValue, maxValue, thumbType, resizingMode, flags)
{
	// Initialize member variables
	formular = view;
	parNum   = num;

	oldPar1 = -1.0f;
	oldPar2 = -1.0f;
	oldPar3 = -1.0f;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDViewSliderFilter::~SIDViewSliderFilter(void)
{
}



/******************************************************************************/
/* DrawThumb() draws the slider thumb.                                        */
/******************************************************************************/
void SIDViewSliderFilter::DrawThumb(void)
{
	bool update = false;
	float par1 = -1.0f;
	float par2 = -1.0f;
	float par3 = -1.0f;

	// Call base class
	SIDViewSlider::DrawThumb();

	switch (parNum)
	{
		case 1:
			par1 = FILTER_PAR1_MIN - Value() + FILTER_PAR1_MAX;
			if (par1 != oldPar1)
			{
				oldPar1 = par1;
				update  = true;
			}
			break;

		case 2:
			par2 = Value();
			if (par2 != oldPar2)
			{
				oldPar2 = par2;
				update  = true;
			}
			break;

		case 3:
			par3 = FILTER_PAR3_MIN - ((float)Value() / 100.0f) + FILTER_PAR3_MAX;
			if (par3 != oldPar3)
			{
				oldPar3 = par3;
				update  = true;
			}
			break;
	}

	if (update)
	{
		formular->UpdateCurve(par1, par2, par3);

		if (par1 >= 0.0f)
			sidSettings->WriteIntEntryValue("Filter", "FilterFs", Value());

		if (par2 >= 0.0f)
			sidSettings->WriteIntEntryValue("Filter", "FilterFm", Value());

		if (par3 >= 0.0f)
			sidSettings->WriteIntEntryValue("Filter", "FilterFt", Value());
	}
}





/******************************************************************************/
/* SIDViewSliderPanning class                                                 */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDViewSliderPanning::SIDViewSliderPanning(uint8 channel, BRect frame, PString label, BMessage *message, int32 minValue, int32 maxValue, thumb_style thumbType, uint32 resizingMode, uint32 flags)
 : SIDViewSlider(frame, label, message, minValue, maxValue, thumbType, resizingMode, flags)
{
	chanNum = channel;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDViewSliderPanning::~SIDViewSliderPanning(void)
{
}



/******************************************************************************/
/* DrawBar() draws the slider bar.                                            */
/******************************************************************************/
void SIDViewSliderPanning::DrawBar(void)
{
	rgb_color newColor;
	int32 value, colIndex;
	int32 red, green, blue;

	// Calculate the color index
	value = Value();

	colIndex = value % 128;
	if (value < 128)
		colIndex = 128 - colIndex;

	if (colIndex > 0)
		colIndex = colIndex * 2 - 1;

	// Calculate the color
	red   = colIndex * 100 / 100;
	green = colIndex * 75 / 100;
	blue  = colIndex * 50 / 100;

	newColor.red   = red;
	newColor.green = green;
	newColor.blue  = blue;
	newColor.alpha = 255;

	// And set it
	SetBarColor(newColor);

	SIDViewSlider::DrawBar();

	// Update the registry
	sidSettings->WriteIntEntryValue("Panning", "Channel" + PString::CreateNumber(chanNum), value);
}
