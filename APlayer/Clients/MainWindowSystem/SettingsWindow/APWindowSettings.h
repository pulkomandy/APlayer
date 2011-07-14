/******************************************************************************/
/* APWindowSettings header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowSettings_h
#define __APWindowSettings_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PSettings.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"


/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define SET_MIN_VIEW_WIDTH			540.0f
#define SET_MIN_VIEW_HEIGHT			360.0f

#define SET_TAB_OPTIONS				0
#define SET_TAB_PATHS				1
#define SET_TAB_MIXER				2
#define SET_TAB_NETWORK				3
#define SET_TAB_FILETYPES			4
#define SET_TAB_PLAYERS				5
#define SET_TAB_AGENTS				6
#define SET_TAB_CLIENTS				7



/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define AP_SET_REFRESH_TABS			'rtb_'
#define AP_SET_SAVE					'sav_'
#define AP_SET_USE					'use_'
#define AP_SET_CANCEL				'can_'



/******************************************************************************/
/* APWindowSettings class                                                     */
/******************************************************************************/
class APViewOptions;
class APViewPaths;
class APViewMixer;
class APViewNetwork;
class APViewFileTypes;
class APViewAddOns;

class APWindowSettings : public BWindow
{
public:
	APWindowSettings(MainWindowSystem *system, APGlobalData *global, BRect frame, PString title);
	virtual ~APWindowSettings(void);

	void RefreshWindow(void);

	virtual bool QuitRequested(void);

protected:
	void MessageReceived(BMessage *msg);

	BPoint CalcMinSize(void);
	void SetPosAndSize(void);

	void InitSettings(void);

	void MakeBackup(void);

	MainWindowSystem *windowSystem;
	PResource *res;
	APKeyFilter *keyFilter;

	PSettings backupSettings;

	BView *topView;
	float fontHeight;

	BTabView *tabView;
	APViewOptions *optionsTab;
	APViewPaths *pathsTab;
	APViewMixer *mixerTab;
	APViewNetwork *networkTab;
	APViewFileTypes *fileTypesTab;
	APViewAddOns *playersTab;
	APViewAddOns *agentsTab;
	APViewAddOns *clientsTab;

	BButton *saveButton;
	BButton *useButton;
	BButton *cancelButton;

	bool useThing;
};

#endif
