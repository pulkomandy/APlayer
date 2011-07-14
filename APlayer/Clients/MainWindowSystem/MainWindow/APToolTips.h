/******************************************************************************/
/* APToolTips header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APToolTips_h
#define __APToolTips_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "PList.h"


/******************************************************************************/
/* APToolView class                                                           */
/******************************************************************************/
class APToolView : public BView
{
public:
	APToolView(void);
	virtual ~APToolView(void);

	void SetText(PString text);

protected:
	virtual void Draw(BRect updateRect);

	PString tipText;
};



/******************************************************************************/
/* APToolTips class                                                           */
/******************************************************************************/
class APToolTips : public BView
{
public:
	APToolTips(BRect frame);
	virtual ~APToolTips(void);

	void *AddToolTip(BView *view, PString text);
	void *AddToolTip(BView *view, PResource *resource, int32 strNum);

	void SetEnabled(bool enable);

protected:
	typedef struct APViewData
	{
		BView *view;
		PString text;
	} APViewData;

	static int32 TipThread(void *userData);
	void ShowTip(void);

	BWindow *tipWin;
	APToolView *tipView;

	PMRSWLock listLock;
	PList<APViewData *> tipList;

	PThread tipThread;
	PEvent *threadEvent;

	uint32 timeCounter;
	bool showing;
	bool notAgain;
	bool enabled;

	BPoint oldPos;
};

#endif
