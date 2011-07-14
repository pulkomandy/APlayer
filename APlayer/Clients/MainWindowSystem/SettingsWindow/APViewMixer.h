/******************************************************************************/
/* APViewMixer header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewMixer_h
#define __APViewMixer_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PSettings.h"
#include "PSkipList.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "MainWindowSystem.h"
#include "APSlider.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define SET_MIX_SLIDER_MIXFREQ		'_mfq'
#define SET_MIX_SLIDER_STEREOSEP	'_ssp'
#define SET_MIX_CHECK_INTERP		'_inp'
#define SET_MIX_CHECK_SURROUND		'_sur'
#define SET_MIX_CHECK_FILTER		'_fil'
#define SET_MIX_LIST_AGENT			'_agn'
#define SET_MIX_BUTTON_0_15			'_015'
#define SET_MIX_BUTTON_16_31		'_131'
#define SET_MIX_BUTTON_32_47		'_347'
#define SET_MIX_BUTTON_48_63		'_463'
#define SET_MIX_CHECK_CHANNEL		'_chn'



/******************************************************************************/
/* APViewLine class                                                           */
/******************************************************************************/
class APViewLine : public BView
{
public:
	APViewLine(BRect frame);
	virtual ~APViewLine(void);

protected:
	virtual void Draw(BRect updateRect);
};



/******************************************************************************/
/* APViewNoSettings class                                                     */
/******************************************************************************/
class APViewNoSettings : public BView
{
public:
	APViewNoSettings(PResource *res);
	virtual ~APViewNoSettings(void);

	virtual void MessageReceived(BMessage *message) {};

protected:
	virtual void Draw(BRect updateRect);

protected:
	font_height fh;
	PString textString;
};



/******************************************************************************/
/* APViewMixer class                                                          */
/******************************************************************************/
class APViewMixer : public BView
{
public:
	APViewMixer(MainWindowSystem *system, APGlobalData *global, BRect frame, PString name);
	virtual ~APViewMixer(void);

	void InitSettings(void);
	void RememberSettings(void);
	void CancelSettings(void);
	void SaveSettings(void);
	void SaveWindowSettings(void);
	void MakeBackup(void);
	void HandleMessage(BMessage *message);

	void SetChannels(void);

protected:
	typedef struct APAgentSettings
	{
		APAddOnAgent *agent;
		PString agentName;
		PSettings *original;
		PSettings *backup;
	} APAgentSettings;

	virtual void AttachedToWindow(void);

	void ShowAgentSettings(void);

	static PString MixerFreqFunc(int32 value, void *userData);
	static PString StereoSepFunc(int32 value, void *userData);

	MainWindowSystem *windowSystem;
	APGlobalData *globalData;
	PResource *res;

	uint16 channelsInUse;
	PSkipList<PString, APAgentSettings *> agentSettings;

	BBox *generalBox;
	APSlider *mixFreqSlider;
	APSlider *stereoSepSlider;
	BCheckBox *interpCheck;
	BCheckBox *surroundCheck;
	BCheckBox *filterCheck;

	BBox *mixerOutputBox;
	BPopUpMenu *agentPop;
	BMenuField *agentField;
	PString selectedAgent;
	APViewLine *lineView;
	BView *agentConfig;

	BBox *channelsBox;
	BButton *chan1But;
	BButton *chan2But;
	BButton *chan3But;
	BButton *chan4But;
	BCheckBox *chanCheck[MAX_NUM_CHANNELS];
};

#endif
