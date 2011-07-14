/******************************************************************************/
/* APInfoView header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APInfoView_h
#define __APInfoView_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* APInfoView class                                                           */
/******************************************************************************/
class MainWindowSystem;

class APInfoView : public BStringView
{
public:
	APInfoView(MainWindowSystem *winSys, BRect frame, BMessage *message, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP);
	virtual ~APInfoView(void);

protected:
	virtual void MouseDown(BPoint point);

	MainWindowSystem *windowSystem;
	BMessage *invokeMsg;
};

#endif
