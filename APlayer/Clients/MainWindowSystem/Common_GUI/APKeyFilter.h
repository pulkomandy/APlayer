/******************************************************************************/
/* APKeyFilter header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APKeyFilter_h
#define __APKeyFilter_h

// PolyKit headers
#include "POS.h"
#include "PList.h"


/******************************************************************************/
/* APKeyFilter class                                                          */
/******************************************************************************/
class APKeyFilter : public BMessageFilter
{
public:
	APKeyFilter(BWindow *win);
	virtual ~APKeyFilter(void);

	void AddFilterKey(char key, int32 modifiers, int32 code = 0);

protected:
	typedef struct Key
	{
		char key;
		int32 modifiers;
		int32 code;
	} Key;

	virtual filter_result Filter(BMessage *message, BHandler **target);

	BWindow *window;
	PList<Key> filterKeys;
};

#endif
