/******************************************************************************/
/* PAlert implementation file.                                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_POLYKIT_LIBRARY_

#include <assert.h>

#include <Alert.h>
#include <Button.h>

// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PSystem.h"
#include "PAlert.h"


/******************************************************************************/
/* PAlert class                                                               */
/******************************************************************************/

/******************************************************************************/
/**	Default constructor for creating a new PAlert object, i.e. a new modal
 *	alert box.
 *
 *	@param title title of the alert.
 *	@param message alert message.
 *	@param type type of alert.
 *	@param buttons a enumeration that indicate which buttons you want.
 *	@param defButton default button that is focused.
 *//***************************************************************************/
PAlert::PAlert(PString title, PString message, PAlertType type, PAlertButtons buttons, uint8 defButton)
{
	PString but1, but2, but3;

	assert(defButton <= 2);

	// Remember all the arguments
	alertTitle   = title;
	alertMessage = message;
	alertType    = type;
	alertButtons = buttons;
	alertDefault = defButton;

	// Find the button strings
	switch (buttons)
	{
		case pAbortRetryIgnore:
		{
			but1.LoadString(&PSystem::polyResource, IDS_PALERT_ABORT);
			but2.LoadString(&PSystem::polyResource, IDS_PALERT_RETRY);
			but3.LoadString(&PSystem::polyResource, IDS_PALERT_IGNORE);
			break;
		}

		case pOk:
		{
			but1.LoadString(&PSystem::polyResource, IDS_PALERT_OK);
			break;
		}

		case pOkCancel:
		{
			but1.LoadString(&PSystem::polyResource, IDS_PALERT_OK);
			but2.LoadString(&PSystem::polyResource, IDS_PALERT_CANCEL);
			break;
		}

		case pRetryCancel:
		{
			but1.LoadString(&PSystem::polyResource, IDS_PALERT_RETRY);
			but2.LoadString(&PSystem::polyResource, IDS_PALERT_CANCEL);
			break;
		}

		case pYesNo:
		{
			but1.LoadString(&PSystem::polyResource, IDS_PALERT_YES);
			but2.LoadString(&PSystem::polyResource, IDS_PALERT_NO);
			break;
		}

		case pYesNoCancel:
		{
			but1.LoadString(&PSystem::polyResource, IDS_PALERT_YES);
			but2.LoadString(&PSystem::polyResource, IDS_PALERT_NO);
			but3.LoadString(&PSystem::polyResource, IDS_PALERT_CANCEL);
			break;
		}

		default:
		{
			// The alert buttons aren't implemented
			assert(false);
			break;
		}
	}

	// Build the button string
	alertButtonString = but1;
	if (!but2.IsEmpty())
	{
		alertButtonString += "|" + but2;

		if (!but3.IsEmpty())
			alertButtonString += "|" + but3;
	}

	// Tell the show function that the standard constructor has been used
	usedStandard = true;
}



/******************************************************************************/
/**	Default constructor for creating a new PAlert object, i.e. a new modal
 *	alert box.
 *
 *	@param title title of the alert.
 *	@param message alert message.
 *	@param type type of alert.
 *	@param buttons buttons to include in the alert box. Each button is
 *		represented with a text separated by a '|'. In order to include an
 *		"Ok" and "Cancel" button, use "Ok|Cancel" as input parameter.
 *	@param defButton default button that is focused.
 *//***************************************************************************/
PAlert::PAlert(PString title, PString message, PAlertType type, PString buttons, uint8 defButton)
{
	assert(defButton <= 2);

	// Remember all the arguments
	alertTitle        = title;
	alertMessage      = message;
	alertType         = type;
	alertButtonString = buttons;
	alertDefault      = defButton;

	// Tell the show function that the custom constructor has been used
	usedStandard = false;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PAlert::~PAlert(void)
{
}



/******************************************************************************/
/**	Attend the user by showing this alert box. The alert box only disappear
 *	when the user press one of the buttons of the alert box.
 *
 *	@return the ID of the button which the user pressed down.
 *//***************************************************************************/
PAlert::PAlertButton PAlert::Show(void)
{
	PAlertButton button;
	PString showMsg, but1, but2, but3;
	char *butAdr1, *butAdr2, *butAdr3;
	char *msgAdr;
	int32 index1, index2;
	alert_type type;
	int32 butIndex;
	BAlert *alert;
	BButton *but;

	// Create the message to be shown. This is done because BeOS alerts
	// doesn't have a title bar.
	showMsg = alertTitle + "\n\n" + alertMessage;

	// Find the type of the alert
	switch (alertType)
	{
		case pInfo:
		case pQuestion:
		{
			type = B_INFO_ALERT;
			break;
		}

		case pWarning:
		{
			type = B_WARNING_ALERT;
			break;
		}

		case pStop:
		{
			type = B_STOP_ALERT;
			break;
		}

		default:
		{
			// The alert type isn't implemented
			assert(false);
			type = B_STOP_ALERT;
			break;
		}
	}

	// Extract the button names from the button string
	index1 = alertButtonString.Find('|');
	if (index1 == -1)
		but1 = alertButtonString;
	else
	{
		but1   = alertButtonString.Left(index1);
		index2 = alertButtonString.Find('|', index1 + 1);
		if (index2 == -1)
			but2 = alertButtonString.Mid(index1 + 1);
		else
		{
			but2 = alertButtonString.Mid(index1 + 1, index2 - index1 - 1);
			but3 = alertButtonString.Mid(index2 + 1);
		}
	}

	// Convert the strings into pointers
	butAdr1 = but1.GetString();
	butAdr2 = (but2.IsEmpty() ? NULL : but2.GetString());
	butAdr3 = (but3.IsEmpty() ? NULL : but3.GetString());

	// Create the alert
	alert = new BAlert(NULL, (msgAdr = showMsg.GetString()), butAdr1, butAdr2, butAdr3, B_WIDTH_AS_USUAL, type);
	showMsg.FreeBuffer(msgAdr);
	but3.FreeBuffer(butAdr3);
	but2.FreeBuffer(butAdr2);
	but1.FreeBuffer(butAdr1);

	if (alert == NULL)
		throw PMemoryException();

	// Set the default button and keys
	if (butAdr3 != NULL)
	{
		// Three buttons dialog
		//
		// Remove the default button
		but = alert->ButtonAt(2);
		but->MakeDefault(false);

		// And set a new default button
		but = alert->ButtonAt(alertDefault);
		but->MakeDefault(true);

		// Escape can be pressed to indicate the last button
		alert->SetShortcut(2, B_ESCAPE);
	}
	else
	{
		if (butAdr2 != NULL)
		{
			// Two buttons dialog
			//
			// Make sure that the default is a valid button
			if (alertDefault > 1)
				alertDefault = 0;

			// Remove the default button
			but = alert->ButtonAt(1);
			but->MakeDefault(false);

			// And set a new default button
			but = alert->ButtonAt(alertDefault);
			but->MakeDefault(true);

			// Escape can be pressed to indicate cancel
			alert->SetShortcut(1, B_ESCAPE);
		}
	}

	// Show the alert
	butIndex = alert->Go();

	// Has the standard constructor been used?
	if (usedStandard)
	{
		// Find out which button that has been pressed
		switch (alertButtons)
		{
			case pAbortRetryIgnore:
			{
				button = (butIndex == 0 ? pIDAbort : (butIndex == 1 ? pIDRetry : pIDIgnore));
				break;
			}

			case pOk:
			{
				button = pIDOk;
				break;
			}

			case pOkCancel:
			{
				button = (butIndex == 0 ? pIDOk : pIDCancel);
				break;
			}

			case pRetryCancel:
			{
				button = (butIndex == 0 ? pIDRetry : pIDCancel);
				break;
			}

			case pYesNo:
			{
				button = (butIndex == 0 ? pIDYes : pIDNo);
				break;
			}

			case pYesNoCancel:
			{
				button = (butIndex == 0 ? pIDYes : (butIndex == 1 ? pIDNo : pIDCancel));
				break;
			}

			default:
			{
				// The alert buttons aren't implemented
				assert(false);
				button = pIDCancel;
				break;
			}
		}
	}
	else
	{
		// Custom constructor has been used, so convert the button index
		button = (butIndex == 0 ? pIDButton1 : (butIndex == 1 ? pIDButton2 : pIDButton3));
	}

	return (button);
}
