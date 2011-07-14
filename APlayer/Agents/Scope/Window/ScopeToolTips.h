/******************************************************************************/
/* ScopeToolTips header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ScopeToolTips_h
#define __ScopeToolTips_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "PList.h"


/******************************************************************************/
/* ScopeToolView class                                                        */
/******************************************************************************/
class ScopeToolView : public BView
{
public:
	ScopeToolView(void);
	virtual ~ScopeToolView(void);

	void SetText(PString text);

protected:
	virtual void Draw(BRect updateRect);

	PString tipText;
};



/******************************************************************************/
/* ScopeToolTips class                                                        */
/******************************************************************************/
class ScopeToolTips : public BBox
{
public:
	ScopeToolTips(BRect frame);
	virtual ~ScopeToolTips(void);

	void *AddToolTip(BView *view, PString text);
	void *AddToolTip(BView *view, PResource *resource, int32 strNum);

protected:
	typedef struct ScopeViewData
	{
		BView *view;
		PString text;
	} ScopeViewData;

	static int32 TipThread(void *userData);
	void ShowTip(void);

	BWindow *tipWin;
	ScopeToolView *tipView;

	PMRSWLock listLock;
	PList<ScopeViewData *> tipList;

	PThread tipThread;
	PEvent *threadEvent;

	uint32 timeCounter;
	bool showing;
	bool notAgain;

	BPoint oldPos;
};

#endif
