/******************************************************************************/
/* APViewOptions header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewOptions_h
#define __APViewOptions_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// Client headers
#include "MainWindowSystem.h"
#include "APSlider.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define SET_OPT_CHECK_JUMP				'_jmp'
#define SET_OPT_CHECK_ADDLIST			'_als'
#define SET_OPT_CHECK_REMEMBERLIST		'_rml'
#define SET_OPT_CHECK_REMEMBERLISTPOS	'_rlp'
#define SET_OPT_CHECK_REMEMBERMODPOS	'_rmp'
#define SET_OPT_CHECK_SHOWLISTNUMBER	'_sln'
#define SET_OPT_CHECK_CHANGETYPE		'_cty'
#define SET_OPT_CHECK_BUTTONHELP		'_bhp'
#define SET_OPT_CHECK_SHOWNAME			'_snm'
#define SET_OPT_CHECK_SETLENGTH			'_slg'
#define SET_OPT_CHECK_SCANFILES			'_scf'
#define SET_OPT_CHECK_DOUBLEBUF			'_dbf'
#define SET_OPT_SLIDER_EARLYLOAD		'_eld'
#define SET_OPT_LIST_ERROR				'_err'
#define SET_OPT_CHECK_NEVERENDING		'_nve'
#define SET_OPT_EDIT_NEVERENDING		'_env'
#define SET_OPT_LIST_LISTEND			'_lie'



/******************************************************************************/
/* APViewOptions class                                                        */
/******************************************************************************/
class APViewOptions : public BView
{
public:
	APViewOptions(MainWindowSystem *system, BRect frame, PString name);
	virtual ~APViewOptions(void);

	void InitSettings(void);
	void RememberSettings(void);
	void CancelSettings(void);
	void SaveSettings(void);
	void SaveWindowSettings(void);
	void MakeBackup(void);
	void HandleMessage(BMessage *message);

protected:
	void SetPosAndSize(void);

	MainWindowSystem *windowSystem;
	PResource *res;

	BBox *generalBox;
	BCheckBox *jumpCheck;
	BCheckBox *addListCheck;
	BCheckBox *rememberListCheck;
	BCheckBox *rememberListPositionCheck;
	BCheckBox *rememberModulePositionCheck;
	BCheckBox *showListNumberCheck;
	BCheckBox *buttonHelpCheck;
	BCheckBox *showNameCheck;
	BCheckBox *setLengthCheck;
	BCheckBox *scanFilesCheck;

	BBox *loadingBox;
	BCheckBox *doubleBufCheck;
	APSlider *earlySlider;
	BPopUpMenu *errorPop;
	BMenuField *errorField;

	BBox *playingBox;
	BCheckBox *neverEndingCheck;
	BTextControl *secondsEdit;
	BStringView *secondsText;
	BPopUpMenu *listEndPop;
	BMenuField *listEndField;
};

#endif
