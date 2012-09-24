/******************************************************************************/
/* PSynchronize header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PSynchronize_h
#define __PSynchronize_h

#include <OS.h>

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PList.h"
#include "ImportExport.h"


/******************************************************************************/
/* Error values                                                               */
/******************************************************************************/
enum PSyncError
{
	pSyncOk      = 0,
	pSyncError   = -1,
	pSyncTimeout = -2
};



/******************************************************************************/
/* Timeout values                                                             */
/******************************************************************************/
#define PSYNC_INFINITE				0xffffffff



/******************************************************************************/
/* Prototypes of global functions                                             */
/******************************************************************************/
class PSync;

#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

_IMPEXP_PKLIB int32 AtomicIncrement(vint32 *variable);
_IMPEXP_PKLIB int32 AtomicDecrement(vint32 *variable);

_IMPEXP_PKLIB int32 MultipleObjectsWait(PSync **objects, int32 count, bool waitAll, bigtime_t timeout = PSYNC_INFINITE);



/******************************************************************************/
/* PSync class                                                                */
/******************************************************************************/
class _IMPEXP_PKLIB PSync
{
public:
	PSync(void);
	virtual ~PSync(void);

	virtual PSyncError Lock(uint32 timeout = PSYNC_INFINITE) = 0;
	virtual PSyncError Unlock(void) = 0;

protected:
	void TrigMultiWait(void);
};



/******************************************************************************/
/* PSemaphore class                                                           */
/******************************************************************************/
class _IMPEXP_PKLIB PSemaphore : public PSync
{
public:
	PSemaphore(uint32 count = 1, bool lockSameThread = false);
	PSemaphore(PString name, uint32 count = 1, bool lockSameThread = false);
	virtual ~PSemaphore(void);

	virtual PSyncError Lock(uint32 timeout = PSYNC_INFINITE);
	virtual PSyncError Unlock(void);

	PSyncError LockWithCount(uint32 count, uint32 timeout = PSYNC_INFINITE);
	PSyncError UnlockWithCount(uint32 count);

protected:
	void Initialize(PString name, uint32 count, bool lockSameThread);

	int32 ownerCount;
	bool lockThread;

	sem_id semID;
	thread_id owner;
};



/******************************************************************************/
/* PMutex class                                                               */
/******************************************************************************/
class _IMPEXP_PKLIB PMutex : public PSync
{
public:
	PMutex(bool lockSameThread);
	PMutex(PString name, bool lockSameThread);
	virtual ~PMutex(void);

	virtual PSyncError Lock(uint32 timeout = PSYNC_INFINITE);
	virtual PSyncError Unlock(void);

protected:
	void Initialize(PString name, bool lockSameThread);

	int32 ownerCount;
	bool lockThread;

	sem_id semID;
	thread_id owner;
};



/******************************************************************************/
/* PEvent class                                                               */
/******************************************************************************/
class _IMPEXP_PKLIB PEvent : public PSync
{
public:
	PEvent(bool manualReset, bool initialState);
	PEvent(PString name, bool manualReset, bool initialState);
	virtual ~PEvent(void);

	virtual PSyncError Lock(uint32 timeout = PSYNC_INFINITE);
	virtual PSyncError Unlock(void);

	PSyncError SetEvent(void);
	PSyncError ResetEvent(void);

protected:
	void Initialize(PString name, bool manualReset, bool initialState);

	// MultibleObjectsWait() needs to read the state variable
	friend int32 MultipleObjectsWait(PSync **objects, int32 count, bool waitAll, bigtime_t timeout);

	sem_id semID;
	bool state;
	bool manual;
	bool releaseThread;
};



/******************************************************************************/
/* PMRSWLock class                                                            */
/******************************************************************************/
class _IMPEXP_PKLIB PMRSWLock
{
public:
	PMRSWLock(void);
	PMRSWLock(PString name);
	virtual ~PMRSWLock(void);

	PSyncError WaitToWrite(uint32 timeout = PSYNC_INFINITE);
	PSyncError DoneWriting(void);

	PSyncError WaitToRead(uint32 timeout = PSYNC_INFINITE);
	PSyncError DoneReading(void);

protected:
	typedef struct PReadLockInfo
	{
		int32 thread;				// Holds the thread id that have locked
		int32 count;				// Holds the number of read locks the specific thread have
	} PReadLockInfo;

	void Initialize(PString name);
	int32 FindThreadID(void);

	PMutex *mainLock;				// Mutex to lock the counter variables

	PList<PReadLockInfo> readList;	// Holds all the read locks
	int32 readWaitCount;			// Number of read locks waiting to gain access
	PSemaphore *readSem;			// Will be trigged if some read locks waits

	int32 writeCount;				// Number of write locks. It's either 0 or 1
	int32 writeWaitCount;			// Number of write locks waiting on another write lock
	PSemaphore *writeSem;			// Will be trigged if a write lock wait for a read lock to finish
	PSemaphore *writeLockSem;		// Will be trigged if a write lock wait for another write lock to finish

	int32 writerNest;				// A counter used for nested calls by the same thread
	int32 writerThread;				// The thread id to the thread that holds the lock with write access
};



/******************************************************************************/
/* PLock class                                                                */
/******************************************************************************/
class _IMPEXP_PKLIB PLock
{
public:
	PLock(PSync *lock);
	virtual ~PLock(void);

protected:
	PSync *curLock;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
