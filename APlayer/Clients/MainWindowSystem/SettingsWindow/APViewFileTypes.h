/******************************************************************************/
/* APViewFileTypes header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewFileTypes_h
#define __APViewFileTypes_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APGlobalData.h"

// Santa headers
#include <santa/ColumnListView.h>

// Client headers
#include "MainWindowSystem.h"
#include "APListFileTypes.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define SET_FIL_MSG_SELECTIONCHANGED	'_msc'
#define SET_FIL_BUT_SELECTED			'_sel'
#define SET_FIL_BUT_ALL					'_all'
#define SET_FIL_CHECK_CHANGETYPE		'_cty'
#define SET_FIL_CHECK_REGISTER			'_reg'



/******************************************************************************/
/* APViewFileTypes class                                                      */
/******************************************************************************/
class APViewFileTypes : public BView
{
public:
	APViewFileTypes(MainWindowSystem *system, APGlobalData *global, BRect frame, PString name);
	virtual ~APViewFileTypes(void);

	void InitSettings(void);
	void RememberSettings(void);
	void CancelSettings(void);
	void SaveSettings(void);
	void SaveWindowSettings(void);
	void MakeBackup(void);
	void HandleMessage(BMessage *message);

protected:
	void AddItems(void);
	void RemoveItems(void);

	MainWindowSystem *windowSystem;
	APGlobalData *globalData;
	PResource *res;

	BBox *fileTypesBox;
	APListFileTypes *fileTypesColList;
	CLVContainerView *fileTypesView;
	BButton *selectedBut;
	BButton *allBut;

	BBox *optionsBox;
	BCheckBox *typeCheck;
	BCheckBox *registerCheck;

	float fontHeight;
};

#endif
