/******************************************************************************/
/* ScopeWindow header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ScopeWindow_h
#define __ScopeWindow_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// Agent headers
#include "ScopeAgent.h"
#include "ScopeView.h"
#include "ScopeToolTips.h"


/******************************************************************************/
/* ScopeWindow class                                                          */
/******************************************************************************/
class ScopeWindow : public BWindow
{
public:
	ScopeWindow(ScopeAgent *scopeAgent, PResource *resource, PString title);
	virtual ~ScopeWindow(void);

	virtual void Show(void);
	virtual void Quit(void);
	virtual bool QuitRequested(void);

	void SwitchScopeType(void);

	void DrawWindow(const int16 *buffer, int32 length, bool stereo);
	void ClearWindow(void);

protected:
	BPoint CalcMinSize(void);

	PResource *res;
	ScopeAgent *agent;

	bool isOnScreen;

	ScopeToolTips *box;
	ScopeView *leftChannel;
	ScopeView *rightChannel;
};

#endif
