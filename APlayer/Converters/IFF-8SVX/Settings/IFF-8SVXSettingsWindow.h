/******************************************************************************/
/* IFF8SVXSettingsWindow header file.                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __IFF8SVXSettingsWindow_h
#define __IFF8SVXSettingsWindow_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PSettings.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define APCFG_USE				'_use'
#define APCFG_SAVE				'_sav'



/******************************************************************************/
/* IFF8SVXSettingsWindow class                                                */
/******************************************************************************/
class IFF8SVXSettingsWindow : public BWindow
{
public:
	IFF8SVXSettingsWindow(PResource *resource, PString title);
	virtual ~IFF8SVXSettingsWindow(void);

	virtual void Quit(void);

protected:
	virtual void MessageReceived(BMessage *msg);

	void GetSettings(void);
	void RememberSettings(PSettings *settings);
	void CheckAndWriteWindowPositions(void);

	PResource *res;

	BView *topView;
	BPopUpMenu *formatPop;
	BMenuField *formatField;
	BButton *useButton;
	BButton *saveButton;
};

#endif
