/******************************************************************************/
/* RIFFWAVESettingsWindow header file.                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __RIFFWAVESettingsWindow_h
#define __RIFFWAVESettingsWindow_h

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
/* RIFFWAVESettingsWindow class                                               */
/******************************************************************************/
class RIFFWAVESettingsWindow : public BWindow
{
public:
	RIFFWAVESettingsWindow(PResource *resource, PString title);
	virtual ~RIFFWAVESettingsWindow(void);

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
