/******************************************************************************/
/* PTimer header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PTimer_h
#define __PTimer_h

// PolyKit headers
#include "POS.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "ImportExport.h"


/******************************************************************************/
/* The message that will be sent                                              */
/******************************************************************************/
#define PM_TIMER			'Ptim'



/******************************************************************************/
/* PTimer class                                                               */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PTimer
{
public:
	PTimer(BLooper *looper, uint16 id);
	virtual ~PTimer(void);

	void SetElapseValue(uint32 value);
	void StartTimer(void);
	void StopTimer(void);

protected:
	BLooper *msgLooper;
	BMessage message;
	BMessageRunner *runner;

	uint32 waitTime;
	bool running;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
