/******************************************************************************/
/* ReverbView header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ReverbView_h
#define __ReverbView_h

// PolyKit headers
#include "POS.h"
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "ReverbViewSlider.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define REV_SLIDER_REVERB			'_rev'



/******************************************************************************/
/* ReverbView class                                                           */
/******************************************************************************/
class ReverbView : public BView
{
public:
	ReverbView(APGlobalData *global, PResource *resource);
	virtual ~ReverbView(void);

	virtual void MessageReceived(BMessage *message);

protected:
	static PString ReverbFunc(void *userData, int32 value);

	APGlobalData *globalData;
	PResource *res;

	ReverbViewSlider *reverbSlider;
};

#endif
