/******************************************************************************/
/* APViewAddOns header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewAddOns_h
#define __APViewAddOns_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PList.h"

// APlayerKit headers
#include "ColumnListView.h"
#include "APGlobalData.h"
#include "APList.h"

// Client headers
#include "MainWindowSystem.h"
#include "APListAddOns.h"
#include "APListDescription.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define SET_ADD_MSG_SELECTIONCHANGED	'_msc'
#define SET_ADD_MSG_DOUBLECLICK			'_dou'
#define SET_ADD_BUT_SETTINGS			'_set'
#define SET_ADD_BUT_DISPLAY				'_dis'



/******************************************************************************/
/* APViewAddOns class                                                         */
/******************************************************************************/
class APViewAddOns : public BView
{
public:
	APViewAddOns(MainWindowSystem *system, APGlobalData *global, BRect frame, PString name, PString type, APList<APAddOnInformation *> *list);
	virtual ~APViewAddOns(void);

	void InitSettings(void);
	void RememberSettings(void);
	void CancelSettings(void);
	void SaveSettings(void);
	void SaveWindowSettings(void);
	void MakeBackup(void);
	void HandleMessage(BMessage *message);

	void EnableAddOn(APAddOnInformation *item);
	void DisableAddOn(APAddOnInformation *item);

	void UpdateAddOnItem(PString name);
	void ClearAddOnItem(void);

protected:
	void AddItems(void);
	void ShowDescription(PString description);

	MainWindowSystem *windowSystem;
	APGlobalData *globalData;
	PResource *res;

	PString addOnType;
	APList<APAddOnInformation *> *infoList;
	PString inUseName;

	PList<bool> backupList;

	// Selection variables
	int32 listNum;

	// Visual variables
	BBox *addOnsBox;

	APListAddOns *addOnsColList;
	CLVContainerView *addOnsView;

	APListDescription *descriptionColList;
	CLVContainerView *descriptionView;

	BButton *settingBut;
	BButton *displayBut;

	float fontHeight;
};

#endif
