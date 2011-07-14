/******************************************************************************/
/* APFilter header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APFilter_h
#define __APFilter_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* APFilter class                                                             */
/******************************************************************************/
class APFilter : public BMessageFilter
{
public:
	APFilter(BWindow *win);
	virtual ~APFilter(void);

protected:
	virtual filter_result Filter(BMessage *message, BHandler **target);

	BWindow *window;
};

#endif
