/******************************************************************************/
/* APKeyFilter implementation file.                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PList.h"

// Client headers
#include "APKeyFilter.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APKeyFilter::APKeyFilter(BWindow *win) : BMessageFilter(B_PROGRAMMED_DELIVERY, B_ANY_SOURCE)
{
	// Remember the arguments
	window = win;

	// Add the filter to the window
	window->AddCommonFilter(this);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APKeyFilter::~APKeyFilter(void)
{
	// Remove the filter from the window
	window->RemoveCommonFilter(this);
}



/******************************************************************************/
/* AddFilterKey() will add a filter key to the list of keys to filter.        */
/*                                                                            */
/* Input:  "key" is the unmodified key to filter on.                          */
/*         "modifiers" is the modifiers.                                      */
/*         "code" is an optional key code.                                    */
/******************************************************************************/
void APKeyFilter::AddFilterKey(char key, int32 modifiers, int32 code)
{
	Key newKey;

	// Add it to the filter list
	newKey.key       = key;
	newKey.modifiers = modifiers;
	newKey.code      = code;
	filterKeys.AddTail(newKey);
}



/******************************************************************************/
/* Filter() is the main filter function.                                      */
/*                                                                            */
/* Input:  "message" is the message to filter on.                             */
/*         "target" is the target where the message is heading to.            */
/*                                                                            */
/* Output: Tells to dispatch or skip the message.                             */
/******************************************************************************/
filter_result APKeyFilter::Filter(BMessage *message, BHandler **target)
{
	// Did we get a key down message?
	if (message->what == B_KEY_DOWN)
	{
		// Yes, now get the key that the user pressed
		int32 rawChar, modifiers, code;

		if (message->FindInt32("raw_char", &rawChar) == B_OK)
		{
			if (message->FindInt32("modifiers", &modifiers) == B_OK)
			{
				if (message->FindInt32("key", &code) == B_OK)
				{
					// Got it, now see if it's one to filter on
					Key checkKey;
					int32 i, count;

					// Mask out general modifier keys
					modifiers &= (B_SHIFT_KEY | B_COMMAND_KEY | B_CONTROL_KEY | B_NUM_LOCK | B_OPTION_KEY | B_MENU_KEY);

					count = filterKeys.CountItems();
					for (i = 0; i < count; i++)
					{
						checkKey = filterKeys.GetItem(i);
						if ((checkKey.key == (char)rawChar) && (checkKey.modifiers == modifiers))
						{
							if ((checkKey.code == 0) || ((checkKey.code != 0) && (checkKey.code == code)))
							{
								// Change the target to the main window
								*target = window;
								break;
							}
						}
					}
				}
			}
		}
	}

	return (B_DISPATCH_MESSAGE);
}
