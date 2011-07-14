/******************************************************************************/
/* Scope view class.                                                          */
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
#include "ScopeWindow.h"
#include "ScopeView.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ScopeView::ScopeView(BRect frame) : BView(frame, NULL, B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
	BRect bitmapFrame;

	// Initialize member variables
	scopeType  = eFilled;
	sampleData = NULL;

	// Create a bitmap used to hold all the bits to show
	bitmapFrame.Set(0.0f, 0.0f, frame.Width(), frame.Height() + MAX_VIEW_HEIGHT);
	bitmap = new BBitmap(bitmapFrame, B_COLOR_8_BIT, true);

	// Create the render view, where we do all the drawing
	renderView = new BView(bitmapFrame, NULL, B_FOLLOW_ALL_SIDES, 0);
	bitmap->AddChild(renderView);

	// Set the background to transparent
	SetViewColor(B_TRANSPARENT_32_BIT);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ScopeView::~ScopeView(void)
{
	// Done with the bitmap
	delete bitmap;
}



/******************************************************************************/
/* Draw() draw the curve and other stuff in the view.                         */
/*                                                                            */
/* Input:  "updateRect" is where the view need to be updated.                 */
/******************************************************************************/
void ScopeView::Draw(BRect updateRect)
{
	BRect frame;
	float midPos;

	// Lock the bitmap
	bitmap->Lock();

	// Draw the frame
	frame = Bounds();
	renderView->SetHighColor(BeDarkShadow);
	renderView->StrokeLine(BPoint(frame.right - 1.0f, frame.top), BPoint(frame.left, frame.top));
	renderView->StrokeLine(BPoint(frame.left, frame.bottom - 1.0f));

	renderView->SetHighColor(White);
	renderView->StrokeLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom));
	renderView->StrokeLine(BPoint(frame.right, frame.top));

	// Then fill the box
	renderView->SetHighColor(Black);
	frame.InsetBy(1.0f, 1.0f);
	renderView->FillRect(frame);

	// Find the middle line
	midPos = frame.Height() / 2.0f;

	// Now draw the sample
	if (sampleData == NULL)
	{
		// No sample data assigned, so just draw a straigt line
		renderView->SetHighColor(Green);
		renderView->StrokeLine(BPoint(frame.left, midPos), BPoint(frame.right, midPos));
	}
	else
	{
		switch (scopeType)
		{
			case eFilled:
			{
				DrawFilled(midPos);
				break;
			}

			case eLines:
			{
				DrawLines(midPos);
				break;
			}

			case eDots:
			{
				DrawDots(midPos);
				break;
			}

			default:
			{
				ASSERT(false);
				break;
			}
		}
	}

	// Done with all the drawing
	renderView->Sync();

	// Blit the rendered image into the view that is shown to the user
	DrawBitmapAsync(bitmap, BPoint(0.0f, 0.0f));
	Flush();
	Sync();

	// Also done with the bitmap
	bitmap->Unlock();
}



/******************************************************************************/
/* MouseDown() is called when the user presses one of the mouse buttons.      */
/*                                                                            */
/* Input:  "point" is where in the view the mouse was when pressed.           */
/******************************************************************************/
void ScopeView::MouseDown(BPoint point)
{
	((ScopeWindow *)Window())->SwitchScopeType();
}



/******************************************************************************/
/* SetScopeType() will change the type of scope to show.                      */
/*                                                                            */
/* Input:  "type" is the type to show.                                        */
/******************************************************************************/
void ScopeView::SetScopeType(ScopeType type)
{
	scopeType = type;
}



/******************************************************************************/
/* RotateScopeType() will rotate the scope type.                              */
/*                                                                            */
/* Output: The new scope type.                                                */
/******************************************************************************/
ScopeView::ScopeType ScopeView::RotateScopeType(void)
{
	switch (scopeType)
	{
		case eFilled:
		{
			scopeType = eLines;
			break;
		}

		case eLines:
		{
			scopeType = eDots;
			break;
		}

		case eDots:
		{
			scopeType = eFilled;
			break;
		}

		default:
		{
			ASSERT(false);
			break;
		}
	}

	return (scopeType);
}



/******************************************************************************/
/* DrawSample() will tell the view to draw itself.                            */
/*                                                                            */
/* Input:  "buffer" is a pointer to the data.                                 */
/*         "len" is the length of the data in samples.                        */
/*         "stereo" indicate if the sample data is stored interleaved or not. */
/******************************************************************************/
void ScopeView::DrawSample(const int16 *buffer, int32 len, bool stereo)
{
	// Remember the arguments
	sampleData   = buffer;
	sampleLen    = len;
	sampleStereo = stereo;

	// Draw the view. Directly call is faster than invalide the view
	Draw(Bounds());
}



/******************************************************************************/
/* DrawFilled() will show the sample data as a filled block.                  */
/*                                                                            */
/* Input:  "midPos" is the y coordinate to the middle line.                   */
/******************************************************************************/
void ScopeView::DrawFilled(float midPos)
{
	BPoint start, end;
	float multiply, y;
	float pos, step;
	int32 i;

	// Find out what to multiply the sample value with
	// to get the right position in the view
	multiply = midPos / 128.0f;

	// Now calculate which samples to take from the buffer
	step = sampleLen / CHANNEL_SIZE;

	// Begin to draw
	renderView->BeginLineArray(CHANNEL_SIZE);

	if (sampleStereo)
	{
		for (i = 0, pos = 0; i < CHANNEL_SIZE; i++, pos += step)
		{
			// Find the position in the view where the sample is
			y = (sampleData[(int32)(pos * 2.0f)] >> 8) * multiply + midPos;

			// Draw the sample
			start.x = i;
			start.y = y;
			end.x   = i;
			end.y   = midPos;
			renderView->AddLine(start, end, Green);
		}
	}
	else
	{
		for (i = 0, pos = 0; i < CHANNEL_SIZE; i++, pos += step)
		{
			// Find the position in the view where the sample is
			y = (sampleData[(int32)pos] >> 8) * multiply + midPos;

			// Draw the sample
			start.x = i;
			start.y = y;
			end.x   = i;
			end.y   = midPos;
			renderView->AddLine(start, end, Green);
		}
	}

	// Draw all the lines
	renderView->EndLineArray();
}



/******************************************************************************/
/* DrawLines() will show the sample data as lines.                            */
/*                                                                            */
/* Input:  "midPos" is the y coordinate to the middle line.                   */
/******************************************************************************/
void ScopeView::DrawLines(float midPos)
{
	BPoint start, end;
	float multiply, y;
	float pos, step;
	int32 i;

	// Find out what to multiply the sample value with
	// to get the right position in the view
	multiply = midPos / 128.0f;

	// Now calculate which samples to take from the buffer
	step = sampleLen / CHANNEL_SIZE;

	// Initialize the start point
	end.x = 0;
	end.y = midPos;

	// Begin to draw
	renderView->BeginLineArray(CHANNEL_SIZE);

	if (sampleStereo)
	{
		for (i = 0, pos = 0; i < CHANNEL_SIZE; i++, pos += step)
		{
			// Find the position in the view where the sample is
			y = (sampleData[(int32)(pos * 2.0f)] >> 8) * multiply + midPos;

			// Draw the sample
			start.x = end.x;
			start.y = end.y;
			end.x   = i;
			end.y   = y;
			renderView->AddLine(start, end, Green);
		}
	}
	else
	{
		for (i = 0, pos = 0; i < CHANNEL_SIZE; i++, pos += step)
		{
			// Find the position in the view where the sample is
			y = (sampleData[(int32)pos] >> 8) * multiply + midPos;

			// Draw the sample
			start.x = end.x;
			start.y = end.y;
			end.x   = i;
			end.y   = y;
			renderView->AddLine(start, end, Green);
		}
	}

	// Draw all the lines
	renderView->EndLineArray();
}



/******************************************************************************/
/* DrawDots() will show the sample data as dots.                              */
/*                                                                            */
/* Input:  "midPos" is the y coordinate to the middle line.                   */
/******************************************************************************/
void ScopeView::DrawDots(float midPos)
{
	BPoint point;
	float multiply, y;
	float pos, step;
	int32 i;

	// Find out what to multiply the sample value with
	// to get the right position in the view
	multiply = midPos / 128.0f;

	// Now calculate which samples to take from the buffer
	step = sampleLen / CHANNEL_SIZE;

	// Begin to draw
	renderView->BeginLineArray(CHANNEL_SIZE);

	if (sampleStereo)
	{
		for (i = 0, pos = 0; i < CHANNEL_SIZE; i++, pos += step)
		{
			// Find the position in the view where the sample is
			y = (sampleData[(int32)(pos * 2.0f)] >> 8) * multiply + midPos;

			// Draw the sample
			point.x = i;
			point.y = y;
			renderView->AddLine(point, point, Green);
		}
	}
	else
	{
		for (i = 0, pos = 0; i < CHANNEL_SIZE; i++, pos += step)
		{
			// Find the position in the view where the sample is
			y = (sampleData[(int32)pos] >> 8) * multiply + midPos;

			// Draw the sample
			point.x = i;
			point.y = y;
			renderView->AddLine(point, point, Green);
		}
	}

	// Draw all the lines
	renderView->EndLineArray();
}
