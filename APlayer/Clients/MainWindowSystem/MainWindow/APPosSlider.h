/******************************************************************************/
/* APPosSlider header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APPosSlider_h
#define __APPosSlider_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* APSlider class                                                             */
/******************************************************************************/
class APPosSlider : public BSlider
{
public:
	APPosSlider(BRect frame, BMessage *message, uint32 resizingMode, uint32 flags);
	virtual ~APPosSlider(void);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);

protected:
	virtual void DrawThumb(void);
	virtual void DrawFocusMark(void);

	int32 oldValue;
};

#endif
