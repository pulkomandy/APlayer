/******************************************************************************/
/* SIDView header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDView_h
#define __SIDView_h

// PolyKit headers
#include "POS.h"
#include "PResource.h"

// Player headers
#include "SIDViewSlider.h"
#include "SIDViewFormular.h"
#include "SIDDiskButton.h"


/******************************************************************************/
/* Filter parameter minimum and maximum values                                */
/******************************************************************************/
#define FILTER_PAR1_MIN					700.0f
#define FILTER_PAR1_MAX					10.0f
#define FILTER_PAR2_MIN					2.0f
#define FILTER_PAR2_MAX					120.0f
#define FILTER_PAR3_MIN					0.5f
#define FILTER_PAR3_MAX					0.0f



/******************************************************************************/
/* Radio button values                                                        */
/******************************************************************************/
#define RADIO_MEMORY_FULLBANK			0
#define RADIO_MEMORY_TRANSPARENT		1
#define RADIO_MEMORY_PLAYSID			2

#define RADIO_SPEED_PAL					0
#define RADIO_SPEED_NTSC				1



/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define SID_STIL_REF_RECEIVED			'_srr'

#define SID_CHECK_FILTER				'_cfi'
#define SID_CHECK_8580					'_mos'
#define SID_CHECK_SPEED					'_spd'

#define SID_RADIO_FULLBANK				'_ful'
#define SID_RADIO_TRANSPARENT			'_tra'
#define SID_RADIO_PLAYSID				'_psd'

#define SID_RADIO_PAL					'_pal'
#define SID_RADIO_NTSC					'_nts'

#define SID_FILTER_PAR1					'_fp1'
#define SID_FILTER_PAR2					'_fp2'
#define SID_FILTER_PAR3					'_fp3'
#define SID_FILTER_DEFAULT				'_fdf'

#define SID_PANNING_CHAN1				'_pc1'
#define SID_PANNING_CHAN2				'_pc2'
#define SID_PANNING_CHAN3				'_pc3'
#define SID_PANNING_CHAN4				'_pc4'

#define SID_DIGI_SCAN					'_dig'
#define SID_STIL_PATH					'_stp'
#define SID_STIL_DISK					'_std'



/******************************************************************************/
/* SIDView class                                                              */
/******************************************************************************/
class SIDView : public BView
{
public:
	SIDView(PResource *resource);
	virtual ~SIDView(void);

	virtual void MessageReceived(BMessage *message);

protected:
	BPoint CalcMinSize(void);
	void SetPosAndSize(void);

	PResource *res;

	BMessenger *panelMessenger;
	BFilePanel *panel;

	float fontHeight;

	BBox *generalBox;
	BCheckBox *filterCheck;
	BCheckBox *mos8580Check;
	BCheckBox *forceSpeedCheck;

	BBox *memoryBox;
	BRadioButton *fullRadio;
	BRadioButton *transparentRadio;
	BRadioButton *playSidRadio;

	BBox *speedBox;
	BRadioButton *palRadio;
	BRadioButton *ntscRadio;

	BBox *filterBox;
	SIDViewSliderFilter *par1Slider;
	SIDViewSliderFilter *par2Slider;
	SIDViewSliderFilter *par3Slider;
	SIDViewFormular *formular;
	BButton *defaultButton;

	BBox *panningBox;
	SIDViewSliderPanning *chan1Slider;
	SIDViewSliderPanning *chan2Slider;
	SIDViewSliderPanning *chan3Slider;
	SIDViewSliderPanning *chan4Slider;

	BBox *miscBox;
	SIDViewSlider *digiSlider;
	BStringView *stilText;
	BTextControl *stilPath;
	SIDDiskButton *stilDisk;
};

#endif
