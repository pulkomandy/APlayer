/******************************************************************************/
/* Scope window class.                                                        */
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
#include "Layout.h"

// Agent headers
#include "ScopeAgent.h"
#include "ScopeView.h"
#include "ScopeWindow.h"
#include "ScopeToolTips.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *scopeSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ScopeWindow::ScopeWindow(ScopeAgent *scopeAgent, PResource *resource, PString title) : BWindow(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_H_RESIZABLE)
{
	BRect rect;
	BPoint winSize;
	char *titleStr;
	float x, y, w, h;
	ScopeView::ScopeType type;

	// Remember the arguments
	res   = resource;
	agent = scopeAgent;

	// Set the window title
	SetTitle((titleStr = title.GetString()));
	title.FreeBuffer(titleStr);

	// Create background view
	rect = Bounds();
	box  = new ScopeToolTips(rect);
	AddChild(box);

	// Get the window positions
	x = scopeSettings->GetIntEntryValue("Window", "WinX");
	y = scopeSettings->GetIntEntryValue("Window", "WinY");
	w = scopeSettings->GetIntEntryValue("Window", "WinW");
	h = scopeSettings->GetIntEntryValue("Window", "WinH");

	// Check to see if the given size is lesser than the minimum size
	winSize = CalcMinSize();
	if ((w < winSize.x) || (h < winSize.y))
	{
		w = winSize.x;
		h = winSize.y;
	}

	SetSizeLimits(winSize.x, winSize.x, winSize.y, winSize.y + MAX_VIEW_HEIGHT);

	// Resize and set the window
	ResizeTo(w, h);
	MoveTo(x, y);

	// Create the left channel view
	rect = box->Bounds();
	rect.left    = HSPACE + 1.0f;
	rect.top     = VSPACE + 1.0f;
	rect.right   = rect.left + CHANNEL_SIZE + 1.0f;
	rect.bottom -= (VSPACE + 1.0f);
	leftChannel = new ScopeView(rect);
	box->AddToolTip(leftChannel, res, IDS_SCOPE_TOOLTIP);
	box->AddChild(leftChannel);

	// Create the right channel view
	rect.left  = rect.right + HSPACE + 1.0f;
	rect.right = rect.left + CHANNEL_SIZE + 1.0f;
	rightChannel = new ScopeView(rect);
	box->AddToolTip(rightChannel, res, IDS_SCOPE_TOOLTIP);
	box->AddChild(rightChannel);

	// Get the scope type
	type = (ScopeView::ScopeType)scopeSettings->GetIntEntryValue("General", "Type");
	leftChannel->SetScopeType(type);
	rightChannel->SetScopeType(type);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ScopeWindow::~ScopeWindow(void)
{
	BRect winPos;
	int32 x, y, w, h;

	// Tell the agent that the window is closed
	agent->WindowClosed();

	try
	{
		// Store the window position and size if changed
		winPos = Frame();

		x = (int32)winPos.left;
		y = (int32)winPos.top;
		w = winPos.IntegerWidth();
		h = winPos.IntegerHeight();

		// Check to see if they have changed
		if ((scopeSettings->GetIntEntryValue("Window", "WinX") != x) ||
			(scopeSettings->GetIntEntryValue("Window", "WinY") != y) ||
			(scopeSettings->GetIntEntryValue("Window", "WinW") != w) ||
			(scopeSettings->GetIntEntryValue("Window", "WinH") != h))
		{
			scopeSettings->WriteIntEntryValue("Window", "WinX", x);
			scopeSettings->WriteIntEntryValue("Window", "WinY", y);
			scopeSettings->WriteIntEntryValue("Window", "WinW", w);
			scopeSettings->WriteIntEntryValue("Window", "WinH", h);
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
void ScopeWindow::Show(void)
{
	// Call the base class to open the window for real
	BWindow::Show();

	// The window is now on the screen
	isOnScreen = true;
}



/******************************************************************************/
/* Quit() is called when the window closes.                                   */
/******************************************************************************/
void ScopeWindow::Quit(void)
{
	PString tempStr;

	// Check to see if the open status has changed
	tempStr = isOnScreen ? "Yes" : "No";
	if (scopeSettings->GetStringEntryValue("Window", "OpenWindow").CompareNoCase(tempStr) != 0)
		scopeSettings->WriteStringEntryValue("Window", "OpenWindow", tempStr);

	// Call the base class
	BWindow::Quit();
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output: Returns true if it's okay to quit, else false.                     */
/******************************************************************************/
bool ScopeWindow::QuitRequested(void)
{
	// The window is no longer on the screen
	isOnScreen = false;

	return (true);
}



/******************************************************************************/
/* SwitchScopeType() will change the scope type on both views.                */
/******************************************************************************/
void ScopeWindow::SwitchScopeType(void)
{
	ScopeView::ScopeType type;

	leftChannel->RotateScopeType();
	type = rightChannel->RotateScopeType();

	// Write the new type in the settings
	scopeSettings->WriteIntEntryValue("General", "Type", type);
}



/******************************************************************************/
/* DrawWindow() will tell each channel to draw itself.                        */
/*                                                                            */
/* Input:  "buffer" is a pointer to the data.                                 */
/*         "len" is the length of the data in samples.                        */
/*         "stereo" indicate if the sample data is stored interleaved or not. */
/******************************************************************************/
void ScopeWindow::DrawWindow(const int16 *buffer, int32 length, bool stereo)
{
	// Lock the window
	if (Lock())
	{
		// Tell each view to draw
		leftChannel->DrawSample(buffer, length, stereo);

		if (stereo)
			rightChannel->DrawSample(&buffer[1], length, stereo);

		// Unlock the window again
		Unlock();
	}
}



/******************************************************************************/
/* ClearWindow() will clear the window.                                       */
/******************************************************************************/
void ScopeWindow::ClearWindow(void)
{
	// Lock the window
	if (Lock())
	{
		// Tell each view to clear itself
		leftChannel->DrawSample(NULL, 0, false);
		rightChannel->DrawSample(NULL, 0, false);

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
BPoint ScopeWindow::CalcMinSize(void)
{
	BPoint size;

	size.x = HSPACE * 3.0f + (2.0f + CHANNEL_SIZE) * 2.0f + 1.0f;
	size.y = VSPACE * 2.0f + (2.0f + 64.0f) * 2.0f + 1.0f;

	return (size);
}
