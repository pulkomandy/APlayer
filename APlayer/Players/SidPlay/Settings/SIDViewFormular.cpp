/******************************************************************************/
/* SID ViewFormular class.                                                    */
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
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Player headers
#include "SIDViewFormular.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Formular view size                                                         */
/******************************************************************************/
#define FORMULAR_CELL_WIDTH		32.0f
#define FORMULAR_CELL_HEIGHT	24.0f

#define FORMULAR_WIDTH			(FORMULAR_CELL_WIDTH * 4.0f)
#define FORMULAR_HEIGHT			(FORMULAR_CELL_HEIGHT * 4.0f)



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDViewFormular::SIDViewFormular(PResource *resource, float par1, float par2, float par3) : BView(BRect(0.0f, 0.0f, 128.0f, 128.0f), NULL, B_FOLLOW_NONE, B_WILL_DRAW)
{
	res = resource;

	// Remember the filter parameters
	filterFs = par1;
	filterFm = par2;
	filterFt = par3;

	// Set the background, so it won't be drawn. This removes the flashes
	// when the curve are updated
	SetViewColor(B_TRANSPARENT_32_BIT);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDViewFormular::~SIDViewFormular(void)
{
}



/******************************************************************************/
/* UpdateCurve() will recalculate the curve.                                  */
/*                                                                            */
/* Input:  "par1" is the new parameter 1.                                     */
/*         "par2" is the new parameter 2.                                     */
/*         "par3" is the new parameter 3.                                     */
/******************************************************************************/
void SIDViewFormular::UpdateCurve(float par1, float par2, float par3)
{
	if (par1 >= 0.0f)
		filterFs = par1;

	if (par2 >= 0.0f)
		filterFm = par2;

	if (par3 >= 0.0f)
		filterFt = par3;

	Invalidate();
}



/******************************************************************************/
/* GetPreferredSize() will calculate the preferred size the view can have.    */
/*                                                                            */
/* Input:  "width" is where to store the width.                               */
/*         "height" is where to store the height.                             */
/******************************************************************************/
void SIDViewFormular::GetPreferredSize(float *width, float *height)
{
	*width  = FORMULAR_WIDTH;
	*height = FORMULAR_HEIGHT;
}



/******************************************************************************/
/* Draw() draw the curve and other stuff in the view.                         */
/*                                                                            */
/* Input:  "updateRect" is where the view need to be updated.                 */
/******************************************************************************/
void SIDViewFormular::Draw(BRect updateRect)
{
//	BPicture *picture;
	BFont font;
	font_height fh;
	float fontHeight, baseLine;
	PString text;
	char *textStr;
	float x, y;
	float vx, vy;
	float oldvx, oldvy;

	// Initialize buffer view
//	BeginPicture(new BPicture());

	// Start to erase the view
	SetHighColor(White);
	FillRect(Bounds());

	// Draw the grid
	SetHighColor(LightGreen);

	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 0.0f, FORMULAR_CELL_HEIGHT * 0.0f, FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 1.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 0.0f, FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 1.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 0.0f, FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 1.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 0.0f, FORMULAR_CELL_WIDTH * 4.0f, FORMULAR_CELL_HEIGHT * 1.0f));

	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 0.0f, FORMULAR_CELL_HEIGHT * 1.0f, FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 2.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 1.0f, FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 2.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 1.0f, FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 2.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 1.0f, FORMULAR_CELL_WIDTH * 4.0f, FORMULAR_CELL_HEIGHT * 2.0f));

	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 0.0f, FORMULAR_CELL_HEIGHT * 2.0f, FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 3.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 2.0f, FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 3.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 2.0f, FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 3.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 2.0f, FORMULAR_CELL_WIDTH * 4.0f, FORMULAR_CELL_HEIGHT * 3.0f));

	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 0.0f, FORMULAR_CELL_HEIGHT * 3.0f, FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 4.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 1.0f, FORMULAR_CELL_HEIGHT * 3.0f, FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 4.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 2.0f, FORMULAR_CELL_HEIGHT * 3.0f, FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 4.0f));
	StrokeRect(BRect(FORMULAR_CELL_WIDTH * 3.0f, FORMULAR_CELL_HEIGHT * 3.0f, FORMULAR_CELL_WIDTH * 4.0f, FORMULAR_CELL_HEIGHT * 4.0f));

	// Draw axis text
	GetFont(&font);
	font.SetSize(9.0f);
	font.SetRotation(0.0f);
	SetFont(&font, B_FONT_SIZE | B_FONT_ROTATION);

	GetFontHeight(&fh);
	fontHeight = ceil(fh.ascent + fh.descent);
	baseLine   = fh.ascent;

	SetHighColor(BeShadow);

	// X-Axis
	text.LoadString(res, IDS_SID_CFG_XAXIS);
	x = FORMULAR_WIDTH - StringWidth((textStr = text.GetString())) - HSPACE;
	y = FORMULAR_HEIGHT - fontHeight + baseLine - VSPACE;
	DrawString(textStr, BPoint(x, y));
	text.FreeBuffer(textStr);

	// Y-Axis
	font.SetRotation(90.0f);
	SetFont(&font, B_FONT_ROTATION);

	text.LoadString(res, IDS_SID_CFG_YAXIS);
	x = HSPACE + baseLine;
	y = VSPACE + StringWidth((textStr = text.GetString()));
	DrawString(textStr, BPoint(x, y));
	text.FreeBuffer(textStr);

	// We want the curve to be blue
	SetHighColor(Blue);

	oldvx = -1.0f;
	oldvy = -1.0f;

	for (x = 0.0f, vx = 0.0f; x < 2048.0f; x += (2048.0f / FORMULAR_WIDTH), vx++)
	{
		// Calculate the y position
		y = -(exp(x / 2048.0f * log(filterFs)) / filterFm) - filterFt;

		if (y > 0.0f)
			y = 0.0f;

		if (y < -1.0f)
			y = -1.0f;

		// And transform it to the view y position
		vy = -y * FORMULAR_HEIGHT;

		if (oldvx < 0.0f)
		{
			oldvx = vx;
			oldvy = vy;
		}

		// Set the point in the view
		StrokeLine(BPoint(oldvx, oldvy), BPoint(vx, vy));

		oldvx = vx;
		oldvy = vy;
	}

	// Done with the picture
//	picture = EndPicture();

	// Now blit in the picture on the screen
//	DrawPicture(picture, BPoint(0, 0));
//	delete picture;
}
