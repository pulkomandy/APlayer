/******************************************************************************/
/* SpinSquareWindow header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
//
// Copyright 2011, Adrien Destugues (pulkomandy@pulkomandy.ath.cx)


#ifndef __SpinSquareWindow_h
#define __SpinSquareWindow_h

class BGridLayout;

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "SpinSquareAgent.h"
#include "SpinSquareView.h"


/******************************************************************************/
/* SpinSquareWindow class                                                     */
/******************************************************************************/
class SpinSquareWindow : public BWindow
{
public:
	SpinSquareWindow(SpinSquareAgent *spinAgent, PResource *resource, PString title);
	virtual ~SpinSquareWindow(void);

	virtual void Show(void);
	virtual void Quit(void);
	virtual bool QuitRequested(void);

	void DrawWindow(APAgent_ChannelChange *channelInfo);
	void ClearWindow(void);

protected:
	PResource *res;
	SpinSquareAgent *agent;

	bool isOnScreen;
	
	int itemCount;
	
	BGridLayout* layout;
};

#endif
