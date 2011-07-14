/******************************************************************************/
/* APViewNetwork header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewNetwork_h
#define __APViewNetwork_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// Client headers
#include "MainWindowSystem.h"
#include "APViewPaths.h"
#include "APSlider.h"
#include "APDiskButton.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define SET_NET_CHECK_USEPROXY		'_upx'
#define SET_NET_EDIT_PROXYADDRESS	'_pad'
#define SET_NET_LIST_CHECKUPDATES	'_chu'
#define SET_NET_BUT_CHECKNOW		'_cnw'
#define SET_NET_EDIT_DOWNLOAD		'_dwl'
#define SET_NET_BUT_DOWNLOAD		'_bdw'



/******************************************************************************/
/* APViewNetwork class                                                        */
/******************************************************************************/
class APViewNetwork : public BView
{
public:
	APViewNetwork(MainWindowSystem *system, BRect frame, PString name);
	virtual ~APViewNetwork(void);

	void InitSettings(void);
	void RememberSettings(void);
	void CancelSettings(void);
	void SaveSettings(void);
	void SaveWindowSettings(void);
	void MakeBackup(void);
	void HandleMessage(BMessage *message);

protected:
	void ShowFilePanel(PString startPath, BFilePanel **panel, int32 id);

	MainWindowSystem *windowSystem;
	PResource *res;

	BBox *proxyBox;
	BCheckBox *proxyCheck;
	BStringView *proxyText;
	BTextControl *proxyEdit;

	BBox *updateBox;
	BPopUpMenu *checkPop;
	BMenuField *checkField;
	BButton *checkNowButton;
	BStringView *downloadText;
	BTextControl *downloadEdit;
	APDiskButton *downloadButton;
	BFilePanel *downloadPanel;

	BMessenger *messenger;
	APViewPathFilter filter;
};

#endif
