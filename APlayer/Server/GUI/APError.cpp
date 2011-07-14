/******************************************************************************/
/* APlayer error class.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PResource.h"
#include "PString.h"
#include "PAlert.h"

// Server headers
#include "APApplication.h"
#include "APError.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* ShowError() will show the error to the user.                               */
/*                                                                            */
/* Input:  "resourceID" is the string resource to show.                       */
/*         "..." is the optional arguments to the resource string.            */
/******************************************************************************/
void APError::ShowError(int32 resourceID, ...)
{
	PString title, msg;
	va_list argList;

	// Set argList to the first argument
	va_start(argList, resourceID);

	// Load the title used in errors
	title.LoadString(GetApp()->resource, IDS_APP_NAME);
	msg.FormatV(GetApp()->resource, resourceID, argList);

	PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
	alert.Show();

	// End argument list parsing
	va_end(argList);
}
