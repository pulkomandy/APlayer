/******************************************************************************/
/* Background view Interface.                                                 */
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
#include "APToolTips.h"
#include "APWindowMain.h"
#include "APBackView.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APBackView::APBackView(BRect frame) : APToolTips(frame)
{
	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APBackView::~APBackView(void)
{
}



/******************************************************************************/
/* MouseMoved() is called when the mouse is moving inside the view. It's used */
/*         to change the mouse cursor on drag'n'drop messages.                */
/*                                                                            */
/* Input:  "point" is the where the mouse is.                                 */
/*         "transit" is a code telling if the mouse is enter, inside or       */
/*         leaving the view.                                                  */
/*         "message" is a pointer to the message or NULL.                     */
/******************************************************************************/
void APBackView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	// Did we enter the view?
	if (transit == B_ENTERED_VIEW)
	{
		// Is the message a drag'n'drop message?
		if (message != NULL)
		{
			// Yap, change the mouse cursor to forbid
			((APWindowMain *)Window())->SetForbiddenCursor();
		}
		else
		{
			// Nip, change the mouse cursor to normal
			((APWindowMain *)Window())->SetNormalCursor();
		}
	}

	// Call the base class
	APToolTips::MouseMoved(point, transit, message);
}
