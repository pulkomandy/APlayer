/******************************************************************************/
/* SpinSquare window class.                                                   */
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
#include "PString.h"
#include "PResource.h"
#include "PSettings.h"
#include "Colors.h"

// APlayerKit headers
#include "APAddOns.h"
#include "Layout.h"

// Agent headers
#include "SpinSquareAgent.h"
#include "SpinSquareView.h"
#include "SpinSquareWindow.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *spinSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SpinSquareWindow::SpinSquareWindow(SpinSquareAgent *spinAgent, PResource *resource, PString title) : BWindow(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
	BRect rect;
	BPoint winSize;
	char *titleStr;
	float x, y, w, h;
	int32 i, j;

	// Remember the arguments
	res   = resource;
	agent = spinAgent;

	// Set the window title
	SetTitle((titleStr = title.GetString()));
	title.FreeBuffer(titleStr);

	// Create background view
	rect = Bounds();
	box  = new BBox(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER);
	AddChild(box);

	// Get the window positions
	x = spinSettings->GetIntEntryValue("Window", "WinX");
	y = spinSettings->GetIntEntryValue("Window", "WinY");

	// Check to see if the given size is lesser than the minimum size
	winSize = CalcMinSize();
	SetSizeLimits(winSize.x, winSize.x, winSize.y, winSize.y);

	// Resize and set the window
	ResizeTo(winSize.x, winSize.y);
	MoveTo(x, y);

	// Create all the square views
	rect.top = VSPACE + 1.0f;

	for (i = 0; i < 2; i++)
	{
		rect.left = HSPACE + 1.0f;

		for (j = 0; j < 8; j++)
		{
			rect.right  = rect.left + VIEW_WIDTH + 1.0f;
			rect.bottom = rect.top + VIEW_HEIGHT + 1.0f;

			squares[j][i] = new SpinSquareView(rect);
			box->AddChild(squares[j][i]);

			// Move to the next position
			rect.left = rect.right + HSPACE + 1.0f;
		}

		rect.top = rect.bottom + VSPACE + 1.0f;
	}

	// Set the pulse rate for the views to 50 times per second
	SetPulseRate(1 * 1000 * 1000 / 50);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SpinSquareWindow::~SpinSquareWindow(void)
{
	BRect winPos;
	int32 x, y;

	// Tell the agent that the window is closed
	agent->WindowClosed();

	try
	{
		// Store the window position and size if changed
		winPos = Frame();

		x = (int32)winPos.left;
		y = (int32)winPos.top;

		// Check to see if they have changed
		if ((spinSettings->GetIntEntryValue("Window", "WinX") != x) ||
			(spinSettings->GetIntEntryValue("Window", "WinY") != y))
		{
			spinSettings->WriteIntEntryValue("Window", "WinX", x);
			spinSettings->WriteIntEntryValue("Window", "WinY", y);
		}
	}
	catch(...)
	{
		;
	}
}



/******************************************************************************/
/* Show() is called when the window is opened for the first time.             */
/******************************************************************************/
void SpinSquareWindow::Show(void)
{
	// Call the base class to open the window for real
	BWindow::Show();

	// The window is now on the screen
	isOnScreen = true;
}



/******************************************************************************/
/* Quit() is called when the window closes.                                   */
/******************************************************************************/
void SpinSquareWindow::Quit(void)
{
	PString tempStr;

	// Check to see if the open status has changed
	tempStr = isOnScreen ? "Yes" : "No";
	if (spinSettings->GetStringEntryValue("Window", "OpenWindow").CompareNoCase(tempStr) != 0)
		spinSettings->WriteStringEntryValue("Window", "OpenWindow", tempStr);

	// Call the base class
	BWindow::Quit();
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output: Returns true if it's okay to quit, else false.                     */
/******************************************************************************/
bool SpinSquareWindow::QuitRequested(void)
{
	// The window is no longer on the screen
	isOnScreen = false;

	return (true);
}



/******************************************************************************/
/* DrawWindow() will tell each channel to draw itself.                        */
/*                                                                            */
/* Input:  "channelInfo" is a pointer to the channel information.             */
/******************************************************************************/
void SpinSquareWindow::DrawWindow(APAgent_ChannelChange *channelInfo)
{
	// Lock the window
	if (Lock())
	{
		uint16 i, todo;

		todo = min(channelInfo->channels, 8);
		for (i = 0; i < todo; i++)
			squares[i][0]->ChannelChanged(channelInfo->channelFlags[i], channelInfo->channelInfo[i]);

		if (channelInfo->channels > 8)
		{
			todo = min(channelInfo->channels - 8, 8);
			for (i = 0; i < todo; i++)
				squares[i][1]->ChannelChanged(channelInfo->channelFlags[i + 8], channelInfo->channelInfo[i + 8]);
		}

		// Unlock the window again
		Unlock();
	}
}



/******************************************************************************/
/* ClearWindow() will clear the window.                                       */
/******************************************************************************/
void SpinSquareWindow::ClearWindow(void)
{
	// Lock the window
	if (Lock())
	{
		int32 i, j;

		// Tell each view to clear itself
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 8; j++)
				squares[j][i]->StopAnimation();
		}

		// Unlock the window again
		Unlock();
	}
}



/******************************************************************************/
/* CalcMinSize() will calculate the minimum size the window can have.         */
/*                                                                            */
/* Output: Is a BPoint where the x is the minimum width and y is the minimum  */
/*         height.                                                            */
/******************************************************************************/
BPoint SpinSquareWindow::CalcMinSize(void)
{
	BPoint size;

	size.x = HSPACE * 9.0f + (2.0f + VIEW_WIDTH) * 8.0f + 1.0f;
	size.y = VSPACE * 3.0f + (2.0f + VIEW_HEIGHT) * 2.0f + 1.0f;

	return (size);
}
