/******************************************************************************/
/* SpinSquare view class.                                                     */
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

// APlayerKit headers
#include "Layout.h"

// Agent headers
#include "SpinSquareWindow.h"
#include "SpinSquareView.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SpinSquareView::SpinSquareView() : BView("spinsquare", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE | B_PULSE_NEEDED)
{
	// Initialize member variables
	animate        = false;

	boxXCoords[0]  = 0;
	boxYCoords[0]  = 0;
	boxXCoords[1]  = 0;
	boxYCoords[1]  = 0;
	boxXCoords[2]  = 0;
	boxYCoords[2]  = 0;
	boxXCoords[3]  = 0;
	boxYCoords[3]  = 0;
	speed          = 0.5;
	angle          = 0.0;
	oldVolume      = 0;
	oldFrequency   = 0;

	drawCoords[0] = BPoint(0, 0);
	drawCoords[1] = BPoint(0, 0);
	drawCoords[2] = BPoint(0, 0);
	drawCoords[3] = BPoint(0, 0);

	// Set the background to transparent
	SetViewColor(B_TRANSPARENT_32_BIT);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SpinSquareView::~SpinSquareView(void)
{
}



/******************************************************************************/
/* Pulse() is called 50 times per second and will animate the square.         */
/******************************************************************************/
void SpinSquareView::Pulse(void)
{
	if (animate)
	{
		// Rotate the box
		double radAngle;
		double sinAngle, cosAngle;
		int16 i;

		// Calculate the new coordinates
		radAngle = angle * 2.0 * M_PI / 360.0;
		sinAngle = sin(radAngle);
		cosAngle = cos(radAngle);

		for (i = 0; i < 4; i++)
		{
			drawCoords[i].x = (boxXCoords[i] * cosAngle) - (boxYCoords[i] * sinAngle) + VIEW_WIDTH / 2.0f;
			drawCoords[i].y = (boxXCoords[i] * sinAngle) + (boxYCoords[i] * cosAngle) + VIEW_HEIGHT / 2.0f;
		}

		// Calculate the new angle
		angle += speed;

		while (angle >= 360.0)
			angle -= 360.0;

		while (angle < 0.0)
			angle += 360.0;

		Invalidate(Bounds());
	}
}



/******************************************************************************/
/* Draw() draw the curve and other stuff in the view.                         */
/*                                                                            */
/* Input:  "updateRect" is where the view need to be updated.                 */
/******************************************************************************/
void SpinSquareView::Draw(BRect updateRect)
{
	BRect frame;

	// Draw the frame
	frame = Bounds();
	SetHighColor(BeDarkShadow);
	StrokeLine(BPoint(frame.right - 1.0f, frame.top), BPoint(frame.left, frame.top));
	StrokeLine(BPoint(frame.left, frame.bottom - 1.0f));

	SetHighColor(White);
	StrokeLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom));
	StrokeLine(BPoint(frame.right, frame.top));

	// Then fill the box
	SetHighColor(BeButtonGrey);
	frame.InsetBy(1.0f, 1.0f);
	FillRect(frame);

	// Draw the box or disable mark
	if (animate)
	{
		SetHighColor(Blue);

		FillPolygon(drawCoords, 4);
	}
	else
	{
		SetHighColor(BeDarkShadow);
		SetDrawingMode(B_OP_OVER);
		FillRect(frame, B_MIXED_COLORS);
	}
}



/******************************************************************************/
/* ChannelChanged() will calculate the new box coordinates.                   */
/*                                                                            */
/* Input:  "flags" is the channel flags.                                      */
/*         "channel" is a pointer to the channel object.                      */
/******************************************************************************/
void SpinSquareView::ChannelChanged(uint32 flags, const APChannel *channel)
{
	int16 newVol;
	int32 newFreq, cachedFreq;
	double newSpeed, cachedSpeed;

	// Begin to rotate the box
	animate = true;

	// Has the volume changed?
	if (flags & NP_VOLUME)
	{
		newVol = channel->GetVolume();
		if (newVol != oldVolume)
		{
			oldVolume     = newVol;
			newVol       /= 8;
			boxYCoords[0] = newVol;
			boxYCoords[1] = newVol;
			boxYCoords[2] = -newVol;
			boxYCoords[3] = -newVol;
		}
	}

	// Has the frequency changed?
	cachedFreq = oldFrequency;

	if (flags & NP_FREQUENCY)
	{
		newFreq = channel->GetFrequency();
		if (newFreq != oldFrequency)
		{
			oldFrequency = newFreq;

			if (newFreq < 3547)
				newFreq = 3547;
			else
			{
				if (newFreq > 34104)
					newFreq = 34104;
			}

			newFreq      -= (34104 + 3547);
			newFreq       = -newFreq / 1066;
			boxXCoords[1] = newFreq;
			boxXCoords[2] = newFreq;
			boxXCoords[0] = -newFreq;
			boxXCoords[3] = -newFreq;
		}
	}

	// Calculate the rotation speed and duration
	cachedSpeed = speed;

	if (oldVolume != 0)
	{
		newSpeed = cachedFreq / oldVolume / 8;
		if (newSpeed != 0.0)
		{
			cachedFreq -= oldFrequency;
			if (cachedFreq < 0)
			{
				cachedFreq = -cachedFreq;
				newSpeed   = -newSpeed;
			}

			if (cachedFreq >= 10)
				speed = newSpeed;
		}
	}

	if (cachedSpeed == speed)
	{
		if (flags & NP_TRIGIT)
			speed = -speed;
	}
}



/******************************************************************************/
/* StopAnimation() will clear the view and stop the animation.                */
/******************************************************************************/
void SpinSquareView::StopAnimation(void)
{
	boxXCoords[0]  = 0;
	boxYCoords[0]  = 0;
	boxXCoords[1]  = 0;
	boxYCoords[1]  = 0;
	boxXCoords[2]  = 0;
	boxYCoords[2]  = 0;
	boxXCoords[3]  = 0;
	boxYCoords[3]  = 0;
	speed          = 0.5;
	angle          = 0.0;
	oldVolume      = 0;
	oldFrequency   = 0;

	animate        = false;
	Invalidate();
}
