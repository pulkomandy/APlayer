/******************************************************************************/
/* PThread header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PThread_h
#define __PThread_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSynchronize.h"
#include "ImportExport.h"


/******************************************************************************/
/* PThread class                                                              */
/******************************************************************************/
typedef int32 (*PThreadFunc)(void *);

#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PThread
{
public:
	enum PPriority { pIdle, pLow, pBelowNormal, pNormal, pAboveNormal, pHigh, pRealTime };

	PThread(void);
	virtual ~PThread(void);

	void StartThread(void);
	void WaitOnThread(void) const;
	int32 GetExitCode(void) const;

	void Resume(void);
	void Suspend(void);
	void Kill(void);

	bool IsAlive(void) const;

	void SetHookFunc(PThreadFunc func, void *userData = NULL);
	void SetName(PString name);
	void SetPriority(PPriority pri);
	PPriority GetPriority(void) const;

protected:
	static int32 ThreadFunc(void *object);
	int32 ConvertPriority(void) const;

	thread_id tid;
	PEvent *suspendSignal;
	int32 suspendCounter;

	bool threadRunning;
	PEvent *threadStart;
	PPriority threadPri;
	PString threadName;
	PThreadFunc threadFunc;
	void *threadData;
	int32 exitCode;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
