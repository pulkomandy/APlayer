/******************************************************************************/
/* APBackView header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APBackView_h
#define __APBackView_h

// PolyKit headers
#include "POS.h"

// Client headers
#include "APToolTips.h"


/******************************************************************************/
/* APBackView class                                                           */
/******************************************************************************/
class APBackView : public APToolTips
{
public:
	APBackView(BRect frame);
	virtual ~APBackView(void);

protected:
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
};

#endif
