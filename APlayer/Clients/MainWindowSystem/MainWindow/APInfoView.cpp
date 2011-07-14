/******************************************************************************/
/* Information view Interface.                                                */
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
#include "MainWindowSystem.h"
#include "APInfoView.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APInfoView::APInfoView(MainWindowSystem *winSys, BRect frame, BMessage *message, uint32 resizingMode) : BStringView(frame, NULL, NULL, resizingMode)
{
	// Initialize member variables
	windowSystem = winSys;
	invokeMsg    = message;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APInfoView::~APInfoView(void)
{
	delete invokeMsg;
}



/******************************************************************************/
/* MouseDown() is called when the user presses the mouse button in the view.  */
/*                                                                            */
/* Input:  "point" is where in the view the user presses the button.          */
/******************************************************************************/
void APInfoView::MouseDown(BPoint point)
{
	BMessage *mouseMsg;
	int32 buttons;

	// Get the current mouse down message
	mouseMsg = windowSystem->mainWin->CurrentMessage();
	ASSERT(mouseMsg != NULL);

	// Get the mouse buttons
	if (mouseMsg->FindInt32("buttons", &buttons) == B_OK)
	{
		// Is left mouse button is pressed?
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
		{
			// Send the message
			if (invokeMsg != NULL)
				windowSystem->mainWin->PostMessage(invokeMsg);
		}
	}

	// Call the base class
	BStringView::MouseDown(point);
}
