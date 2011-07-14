/******************************************************************************/
/* APViewPaths header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewPaths_h
#define __APViewPaths_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// Client headers
#include "MainWindowSystem.h"
#include "APDiskButton.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define SET_PAT_EDIT_STARTSCAN		'_scn'
#define SET_PAT_BUT_STARTSCAN		'_bsc'
#define SET_PAT_EDIT_MODULE			'_mod'
#define SET_PAT_BUT_MODULE			'_bmo'
#define SET_PAT_EDIT_APML			'_apm'
#define SET_PAT_BUT_APML			'_bap'



/******************************************************************************/
/* APViewPathFilter class                                                     */
/******************************************************************************/
class APViewPathFilter : public BRefFilter
{
protected:
	bool Filter(const entry_ref *ref, BNode *node, struct stat *st, const char *filetype);
};



/******************************************************************************/
/* APViewPaths class                                                          */
/******************************************************************************/
class APViewPaths : public BView
{
public:
	APViewPaths(MainWindowSystem *system, BRect frame, PString name);
	virtual ~APViewPaths(void);

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

	BBox *pathsBox;

	BStringView *startScanText;
	BTextControl *startScanEdit;
	APDiskButton *startScanButton;
	BFilePanel *startScanPanel;

	BStringView *moduleText;
	BTextControl *moduleEdit;
	APDiskButton *moduleButton;
	BFilePanel *modulePanel;

	BStringView *apmlText;
	BTextControl *apmlEdit;
	APDiskButton *apmlButton;
	BFilePanel *apmlPanel;

	BMessenger *messenger;
	APViewPathFilter filter;
};

#endif
