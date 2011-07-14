/******************************************************************************/
/* APViewSamples header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewSamples_h
#define __APViewSamples_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "ColumnListView.h"

// Client headers
#include "MainWindowSystem.h"


/******************************************************************************/
/* Useful defines                                                             */
/******************************************************************************/
#define POLYPHONY_CHANNELS			3



/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
enum
{
	SAM_LIST_FORMAT				= '_for',
	SAM_BUTTON_SAVE				= '_sav'
};



/******************************************************************************/
/* SIViewSamples class                                                        */
/******************************************************************************/
class APListViewSample;

class APViewSamples : public BView
{
public:
	APViewSamples(APGlobalData *glob, MainWindowSystem *system, BRect frame, PString name);
	virtual ~APViewSamples(void);

	void SaveSettings(void);

	void HandleMessage(BMessage *msg);

	void AddItems(void);
	void RemoveItems(void);

	void ChangeOctave(uint16 startOctave);
	void ChangePolyphony(bool enable);

	void AttachedToWindow(void);
	void DetachedFromWindow(void);

protected:
	APGlobalData *global;
	MainWindowSystem *windowSystem;
	PResource *res;

	float itemFontHeight;
	float switchPos;

	BBox *sampleBox;
	CLVContainerView *containerView;
	APListViewSample *columnListView;
	BStringView *octaveText;
	BStringView *polyText;
	BPopUpMenu *formatPop;
	BMenuField *formatField;
	BButton *saveButton;

	BFilePanel *filePanel;
	BMessenger *messenger;

	PString saveFormat;
};

#endif
