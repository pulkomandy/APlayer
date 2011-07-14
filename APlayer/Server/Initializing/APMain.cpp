/******************************************************************************/
/* APlayer main function.                                                     */
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

// Program headers
#include "APApplication.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* main() is the first function which will be called. It will setup the       */
/*      application object and start it.                                      */
/******************************************************************************/
int main(void)
{
	APApplication *apApp = NULL;

	try
	{
		apApp = new APApplication("application/x-vnd.Polycode.APlayer");
		if (apApp == NULL)
			return (0);

		// Start APlayer
		apApp->Run();
	}
	catch(...)
	{
		APError::ShowError(IDS_ERR_EXCEPTION);
	}

	// Cleanup
	delete apApp;
	return (0);
}
