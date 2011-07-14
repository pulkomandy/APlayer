/******************************************************************************/
/* Protected lists header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APList_h
#define __APList_h

// PolyKit headers
#include "POS.h"
#include "PSynchronize.h"
#include "PList.h"


/******************************************************************************/
/* APList class                                                               */
/******************************************************************************/
template<class TYPE>
class APList : public PList<TYPE>
{
public:
	APList(void) : lock(false) {};

	inline PSyncError LockList(uint32 timeout = PSYNC_INFINITE) { return (lock.Lock(timeout)); };
	inline void UnlockList(void) { lock.Unlock(); };

protected:
	PMutex lock;
};



/******************************************************************************/
/* APSWMRList class                                                           */
/******************************************************************************/
template<class TYPE>
class APMRSWList : public PList<TYPE>
{
public:
	inline PSyncError WaitToWrite(uint32 timeout = PSYNC_INFINITE) { return (lock.WaitToWrite(timeout)); };
	inline void DoneWriting(void) { lock.DoneWriting(); };

	inline PSyncError WaitToRead(uint32 timeout = PSYNC_INFINITE) { return (lock.WaitToRead(timeout)); };
	inline void DoneReading(void) { lock.DoneReading(); };

protected:
	PMRSWLock lock;
};

#endif
