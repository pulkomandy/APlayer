/******************************************************************************/
/* APFilter implementation file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Server headers
#include "APWindowAddOnConfig.h"
#include "APFilter.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APFilter::APFilter(BWindow *win) : BMessageFilter(B_PROGRAMMED_DELIVERY, B_ANY_SOURCE)
{
	// Remember the arguments
	window = win;

	// Add the filter to the window
	window->AddCommonFilter(this);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APFilter::~APFilter(void)
{
	// Remove the filter from the window
	window->RemoveCommonFilter(this);
}



/******************************************************************************/
/* Filter() is the main filter function.                                      */
/*                                                                            */
/* Input:  "message" is the message to filter on.                             */
/*         "target" is the target where the message is heading to.            */
/*                                                                            */
/* Output: Tells to dispatch or skip the message.                             */
/******************************************************************************/
filter_result APFilter::Filter(BMessage *message, BHandler **target)
{
	// Did we get a key down message?
	if (message->what == B_KEY_DOWN)
	{
		// Yes, now get the key that the user pressed
		int32 rawChar, modifiers;

		if (message->FindInt32("raw_char", &rawChar) == B_OK)
		{
			if (message->FindInt32("modifiers", &modifiers) == B_OK)
			{
				// Got it, now see if it's the ESC key
				//
				// Mask out general modifier keys
				modifiers &= (B_SHIFT_KEY | B_COMMAND_KEY | B_CONTROL_KEY | B_OPTION_KEY | B_MENU_KEY);

				if ((rawChar == B_ESCAPE) && (modifiers == 0))
				{
					// Change the target to the main window and
					// change the message to a "cancel button"
					*target       = window;
					message->what = APCFG_CANCEL;
				}
			}
		}
	}

	return (B_DISPATCH_MESSAGE);
}
