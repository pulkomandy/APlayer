/******************************************************************************/
/* SpinSquare window class.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
//
// Copyright 2011, Adrien Destugues (pulkomandy@pulkomandy.ath.cx)

#include <GridLayout.h>

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
SpinSquareWindow::SpinSquareWindow(SpinSquareAgent *spinAgent, PResource *resource, PString title)
	: BWindow(BRect(0.0f, 0.0f, 100*8, 100*2), NULL, B_TITLED_WINDOW, B_NOT_ZOOMABLE)
	, itemCount(0)
{
	BRect rect;
	char *titleStr;
	float x, y, w, h;

	// Remember the arguments
	res   = resource;
	agent = spinAgent;

	// Set the window title
	SetTitle((titleStr = title.GetString()));
	title.FreeBuffer(titleStr);

	// Create background view
	rect = Bounds();
	
	layout = new BGridLayout(4, 4);
	layout->SetInsets(4, 4, 4, 4);
	SetLayout(layout);
	
	layout->TargetView()->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Get the window positions
	x = spinSettings->GetIntEntryValue("Window", "WinX");
	y = spinSettings->GetIntEntryValue("Window", "WinY");
	w = spinSettings->GetIntEntryValue("Window", "WinW");
	h = spinSettings->GetIntEntryValue("Window", "WinH");

	// Resize and set the window
	MoveTo(x, y);
	ResizeTo(w, h);

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
		
		x = (int)winPos.Width();
		y = (int)winPos.Height();
		if ((spinSettings->GetIntEntryValue("Window", "WinW") != x) ||
			(spinSettings->GetIntEntryValue("Window", "WinH") != y))
		{
			spinSettings->WriteIntEntryValue("Window", "WinW", x);
			spinSettings->WriteIntEntryValue("Window", "WinH", y);
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

		todo = channelInfo->channels;
		
		// TODO also do this when the window size changed
		if (todo != itemCount)
		{
			itemCount = todo;
			// remove all the existing squares
			BLayoutItem* item;
			while((item = layout->RemoveItem(0L)))
			{
				item->View()->Parent()->RemoveChild(item->View());
				delete item->View();
				delete item;
			}
			
			// Compute optimal line count
			float width = Bounds().Width();
			float height = Bounds().Height();
			float bestRatio = 999;
			int bestLines;
			int lines, col;
			for(lines = itemCount; lines > 0; --lines)
			{
				col = (int)ceilf(itemCount/(float)lines);
				int sqwidth = (int)(width / lines);
				int sqheight = (int)(height / col);
				
				float ratio = sqwidth / (float)sqheight;
				if (ratio < 1.f) ratio = 1.f / ratio;
				if (ratio < bestRatio) {
					bestLines = lines;
					bestRatio = ratio;
				}
			}
			
			lines = bestLines;
			col = (int)ceilf(itemCount/(float)lines);
			
			// create new ones
			for (i = 0; i < col; i++)
			for (int j = 0; j < lines; j++)
			{
				layout->AddView(new SpinSquareView(), j, i);
			}
		}
		
		for (i = 0; i < itemCount; i++) {
			SpinSquareView* square = static_cast<SpinSquareView*>(static_cast<BLayout*>(layout)->ItemAt(i)->View());
			square->ChannelChanged(channelInfo->channelFlags[i], channelInfo->channelInfo[i]);
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
		// Tell each view to clear itself
		for (int i = 0; i < itemCount; i++) {
			SpinSquareView* square = static_cast<SpinSquareView*>(static_cast<BLayout*>(layout)->ItemAt(i)->View());
			square->StopAnimation();
		}

		// Unlock the window again
		Unlock();
	}
}
