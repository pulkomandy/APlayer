/******************************************************************************/
/* APWindowSampleInfo header file.                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowSampleInfo_h
#define __APWindowSampleInfo_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "APKeyFilter.h"
#include "APViewSamples.h"
#include "APViewInstruments.h"
#include "APViewSamples.h"


/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define TAB_INSTRUMENTS				0
#define TAB_SAMPLES					1



/******************************************************************************/
/* PlaySample structure                                                       */
/******************************************************************************/
typedef struct PlaySample
{
	const APSampleInfo *sampInfo;
	float playFrequency;
} PlaySample;



/******************************************************************************/
/* APWindowSampleInfo class                                                   */
/******************************************************************************/
class MainWindowSystem;

class APWindowSampleInfo : public BWindow
{
public:
	APWindowSampleInfo(APGlobalData *glob, MainWindowSystem *system, BRect frame, PString title, int32 active);
	virtual ~APWindowSampleInfo(void);

	void OpenWindow(void);
	void CloseWindow(void);

	void RefreshWindow(void);

	void AddSampleToQuery(const APSampleInfo *sampInfo, float frequency);
	bool GetNextSampleFromQuery(PlaySample *playSamp);

	void ChangeOctave(uint16 startOctave);
	void ChangePolyphony(bool enable);
	bool PolyphonyEnabled(void) const;

protected:
	virtual bool QuitRequested(void);
	virtual void MessageReceived(BMessage *msg);

	BPoint CalcMinSize(void);

	APGlobalData *global;
	MainWindowSystem *windowSystem;
	PResource *res;
	APKeyFilter *keyFilter;

	bool shutDown;
	bool wasOnScreen;

	BView *topView;
	BTabView *tabView;
	APViewInstruments *instTab;
	APViewSamples *sampTab;

	float fontHeight;

	PMutex sampleLock;
	PList<PlaySample> sampleList;
	bool poly;
};

#endif
