/******************************************************************************/
/* DiskSaverView header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __DiskSaverView_h
#define __DiskSaverView_h

#include <sys/stat.h>

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "DiskSaverDiskButton.h"


/******************************************************************************/
/* Configuration values                                                       */
/******************************************************************************/
#define CV_OUTPUTTYPE_MONO							0
#define CV_OUTPUTTYPE_STEREO						1

#define CV_OUTPUTSIZE_8BIT							8
#define CV_OUTPUTSIZE_16BIT							16



/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define DS_EDIT_PATH			'pat_'
#define DS_BUT_PATH				'pbu_'
#define DS_RADIO_8BIT			'8bt_'
#define DS_RADIO_16BIT			'16b_'
#define DS_RADIO_MONO			'mon_'
#define DS_RADIO_STEREO			'ste_'
#define DS_LIST_FORMAT			'for_'
#define DS_BUT_SETTINGS			'set_'
#define DS_LIST_PASS			'pas_'



/******************************************************************************/
/* DiskSaverFilter class                                                      */
/******************************************************************************/
class DiskSaverFilter : public BRefFilter
{
protected:
	bool Filter(const entry_ref *ref, BNode *node, stat_beos *st, const char *filetype);
};



/******************************************************************************/
/* DiskSaverViewGrey class                                                    */
/******************************************************************************/
class DiskSaverViewGrey : public BView
{
public:
	DiskSaverViewGrey(BRect frame);
};



/******************************************************************************/
/* DiskSaverView class                                                        */
/******************************************************************************/
class DiskSaverView : public BView
{
public:
	DiskSaverView(APGlobalData *global, PResource *resource);
	virtual ~DiskSaverView(void);

	virtual void MessageReceived(BMessage *message);

protected:
	APGlobalData *globalData;
	PResource *res;

	APAddOnConverter *converter;
	int32 converterIndex;
	PString converterName;
	BWindow *converterWindow;
	thread_id converterThread;

	BStringView *pathText;
	BTextControl *pathEdit;
	DiskSaverDiskButton *pathButton;
	DiskSaverViewGrey *grey1View;
	BRadioButton *bit8Radio;
	BRadioButton *bit16Radio;
	DiskSaverViewGrey *grey2View;
	BRadioButton *monoRadio;
	BRadioButton *stereoRadio;
	BPopUpMenu *formatPop;
	BMenuField *formatField;
	BButton *formatSettings;
	BPopUpMenu *passPop;
	BMenuField *passField;

	BMessenger *messenger;
	BFilePanel *panel;
	DiskSaverFilter filter;
};

#endif
