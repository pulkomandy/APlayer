/******************************************************************************/
/* APWindowModuleInfo header file.                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowModuleInfo_h
#define __APWindowModuleInfo_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "ColumnListView.h"

// Client headers
#include "APWindowModuleInfoList.h"
#include "APKeyFilter.h"


/******************************************************************************/
/* Item index numbers                                                         */
/******************************************************************************/
#define AP_INFO_ITEM_MODULE_NAME			0
#define AP_INFO_ITEM_AUTHOR					1
#define AP_INFO_ITEM_MODULE_FORMAT			2
#define AP_INFO_ITEM_ACTIVE_PLAYER			3
#define AP_INFO_ITEM_CHANNELS				4
#define AP_INFO_ITEM_TIME					5
#define AP_INFO_ITEM_MODULE_SIZE			6
#define AP_INFO_ITEM_FILE					7
#define AP_INFO_FIRST_CUSTOM_ITEM			9



/******************************************************************************/
/* APWindowModuleInfo class                                                   */
/******************************************************************************/
class MainWindowSystem;
class APWindowModuleInfoList;

class APWindowModuleInfo : public BWindow
{
public:
	APWindowModuleInfo(MainWindowSystem *system, BRect frame, PString title);
	virtual ~APWindowModuleInfo(void);

	void OpenWindow(void);
	void CloseWindow(void);

	void RefreshWindow(void);
	void UpdateWindow(uint32 line, PString newValue);

protected:
	virtual bool QuitRequested(void);
	virtual void WindowActivated(bool active);
	virtual void MessageReceived(BMessage *msg);

	BPoint CalcMinSize(void);
	void AddItems(void);

	MainWindowSystem *windowSystem;
	PResource *res;
	APKeyFilter *keyFilter;

	bool shutDown;
	bool wasOnScreen;

	BView *topView;
	CLVContainerView *containerView;
	APWindowModuleInfoList *columnListView;

	float fontHeight;
};

#endif
