/******************************************************************************/
/* PAlert header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PAlert_h
#define __PAlert_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "ImportExport.h"


/******************************************************************************/
/* String resource names                                                      */
/******************************************************************************/
/** @name String resource names */
//@{
/// Abort
#define IDS_PALERT_ABORT							1
/// Retry
#define IDS_PALERT_RETRY							2
/// Ignore
#define IDS_PALERT_IGNORE							3
/// Ok
#define IDS_PALERT_OK								4
/// Cancel
#define IDS_PALERT_CANCEL							5
/// Yes
#define IDS_PALERT_YES								6
/// No
#define IDS_PALERT_NO								7
//@}



/******************************************************************************/
/* PAlert class                                                               */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

/**
 *	This class is used for attending the user with a message using an alert
 *	box that need respons from the user. The alert box displays a message and
 *	wait for the user to press one of the buttons to continue. The user will
 *	have to respond to the message before the alert box disapear. That is, the
 *	alert box is modal. The alert box is useful for showing informative
 *	messages, warnings, errors, or questions.
 */
class _IMPEXP_PKLIB PAlert
{
public:
	/// Alert type
	enum PAlertType
	{
		/// Used for informing the user
		pInfo,
		/// Used for warning the user of an issue
		pWarning,
		/// Used for attending the user of a critical issue such as e.g. an error
		pStop,
		/// Used for giving the user a question
		pQuestion
	};

	/// Button combinations for the alert box
	enum PAlertButtons
	{
		/// Abort, Retry & Ignore
		pAbortRetryIgnore,
		/// Ok
		pOk,
		/// Ok & Cancel
		pOkCancel,
		/// Retry & Cancel
		pRetryCancel,
		/// Yes & No
		pYesNo,
		/// Yes, No & Cancel
		pYesNoCancel
	};

	/// Activation button
	enum PAlertButton
	{
		/// Abort
		pIDAbort,
		/// Retry
		pIDRetry,
		/// Ignore
		pIDIgnore,
		/// Ok
		pIDOk,
		/// Cancel
		pIDCancel,
		/// Yes
		pIDYes,
		/// No
		pIDNo,
		/// Button #1
		pIDButton1,
		/// Button #2
		pIDButton2,
		/// Button #3
		pIDButton3
	};

	PAlert(PString title, PString message, PAlertType type, PAlertButtons buttons, uint8 defButton = 0);
	PAlert(PString title, PString message, PAlertType type, PString buttons, uint8 defButton = 0);
	virtual ~PAlert(void);

	PAlertButton Show(void);

protected:
	PString alertTitle;				// Title of the alert
	PString alertMessage;			// Alert message
	PAlertType alertType;			// Type of alert
	PAlertButtons alertButtons;		// Buttons to include in the alert box
	PString alertButtonString;		// String that represents the buttons to include in the alert box
	uint8 alertDefault;				// Default button to receive the focus when the alert box is shown
	bool usedStandard;				// Flag that specifies if the standard constructor was used
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
