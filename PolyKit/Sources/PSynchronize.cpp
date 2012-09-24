/******************************************************************************/
/* PSynchronize implementation file.                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_POLYKIT_LIBRARY_

// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PSynchronize.h"
#include "PList.h"
#include "PSystem.h"


/******************************************************************************/
/* Structures used by the MultipleObjectWait() function.                      */
/******************************************************************************/
typedef struct PEventItem
{
	thread_id threadID;
	PEvent *event;
} PEventItem;


typedef struct PSyncItem
{
	thread_id threadID;
	PSync *syncObject;
	int32 listID;
	bool trigged;
} PSyncItem;



/******************************************************************************/
/* Global variables used by the MultipleObjectWait() function.                */
/******************************************************************************/
static PMutex syncListLock("MultipleObjectWait() list lock", false);
static PList<PEventItem> syncEventList;
static PList<PSyncItem> syncWaitList;



/******************************************************************************/
/* Global functions                                                           */
/******************************************************************************/

/******************************************************************************/
/* AtomicIncrement() will increment the variable you give by one and return   */
/*      the result. It makes sure that only one thread can access the         */
/*      variable at the time.                                                 */
/*                                                                            */
/* Input:  "variable" is a pointer to the variable you want to increment.     */
/*                                                                            */
/* Output: Is the result after the increment.                                 */
/******************************************************************************/
int32 AtomicIncrement(vint32 *variable)
{
	int32 result;

	result = atomic_add(variable, 1);
	return (result + 1);
}



/******************************************************************************/
/* AtomicDecrement() will decrement the variable you give by one and return   */
/*      the result. It makes sure that only one thread can access the         */
/*      variable at the time.                                                 */
/*                                                                            */
/* Input:  "variable" is a pointer to the variable you want to decrement.     */
/*                                                                            */
/* Output: Is the result after the decrement.                                 */
/******************************************************************************/
int32 AtomicDecrement(vint32 *variable)
{
	int32 result;

	result = atomic_add(variable, -1);
	return (result - 1);
}



/******************************************************************************/
/* MultipleObjectsWait() will wait on multiple synchronize objects. You can   */
/*      select between you want to wait on all the objects or only on one     */
/*      of them.                                                              */
/*                                                                            */
/* Input:  "objects" is a pointer to an array with pointers to the objects.   */
/*         "count" is the number of objects to wait on.                       */
/*         "waitAll" selects between a "wait on all" or "wait on one".        */
/*         "timeout" is the timeout value in milliseconds.                    */
/*                                                                            */
/* Output: Is the array number of the object you got, or one of the errors.   */
/******************************************************************************/
int32 MultipleObjectsWait(PSync **objects, int32 count, bool waitAll, bigtime_t timeout)
{
	PSyncError err;
	int32 i;
	bigtime_t time;

	// Check arguments
	ASSERT(objects != NULL);

	// Check to see if we got a 0 count to wait on
	if (count == 0)
	{
		// We do, so just sleep, but check the timeout value
		ASSERT(timeout != PSYNC_INFINITE);

		PSystem::Sleep(timeout);
		return (pSyncTimeout);
	}

	if (waitAll)
	{
		// We need to wait on all the objects
		PSync *waitObj;
		int32 waitNum;
		int32 j;
		bool gotAll;

		// We just start to wait on the first object
		waitObj = objects[0];
		waitNum = 0;

		do
		{
			// Wait on the object
			time = system_time();
			err = waitObj->Lock(timeout);
			if (err != pSyncOk)
				return (err);

			// We got the object, can we get the rest
			gotAll = true;
			for (i = 0; i < count; i++)
			{
				if (i != waitNum)
				{
					// Try with no timeout
					err = objects[i]->Lock(0);
					if (err == pSyncError)
					{
						// Unlock the objects we have locked
						for (j = i - 1; j >= 0; j--)
						{
							if (j != waitNum)
								objects[j]->Unlock();
						}

						// Unlock the wait object
						waitObj->Unlock();

						return (err);
					}

					if (err == pSyncTimeout)
					{
						// We didn't get it, so unlock the objects
						// we already have locked and wait on this
						// object
						for (j = i - 1; j >= 0; j--)
						{
							if (j != waitNum)
								objects[j]->Unlock();
						}

						// Unlock the wait object
						waitObj->Unlock();

						// Wait on new object
						waitObj = objects[i];
						waitNum = i;
						gotAll  = false;

						// Calculate rest timeout
						if (timeout != PSYNC_INFINITE)
							timeout -= ((system_time() - time) / 1000);

						break;
					}
				}
			}
		}
		while (!gotAll);

		return (0);		// Just return the first object index
	}
	else
	{
		// We only need to wait on one object
		PEventItem eventItem, workEventItem;
		PSyncItem syncItem;
		thread_id curThread;
		int32 listCount;
		int32 result = pSyncError;
		bool trigged = false;

		// Find the current thread ID
		curThread = find_thread(NULL);

		// Create trig event
		eventItem.threadID = curThread;
		eventItem.event    = new PEvent("MultipleObjectWait() trigger", true, false);
		if (eventItem.event == NULL)
			throw PMemoryException();

		// Lock the wait lists
		if (syncListLock.Lock() == pSyncError)
		{
			// Delete the trigger
			delete eventItem.event;

			return (pSyncError);
		}

		// Add the trigger event in the event list
		syncEventList.AddTail(eventItem);

		// Add all the events to wait on the wait list
		for (i = 0; i < count; i++)
		{
			// Fill out the sync item
			syncItem.threadID   = curThread;
			syncItem.syncObject = objects[i];
			syncItem.listID     = i;

			if (is_instance_of(objects[i], PEvent))
			{
				if (((PEvent *)objects[i])->state)
				{
					syncItem.trigged = true;
					trigged          = true;
				}
				else
					syncItem.trigged = false;
			}
			else
				syncItem.trigged = false;

			// Add the object in the list
			syncWaitList.AddTail(syncItem);
		}

		// Unlock the lists again
		syncListLock.Unlock();

		// Wait for the trigger
		if (!trigged)
			err = eventItem.event->Lock(timeout);
		else
			err = pSyncOk;

		// Lock the lists
		if (syncListLock.Lock() == pSyncError)
		{
			// We can't fix this problem, arghhhhh!!!!!
			ASSERT(false);
		}

		// It seems that we got an object, so find it
		if (err == pSyncOk)
		{
			// Get the number of items in the wait list
			listCount = syncWaitList.CountItems();

			// Find the object
			for (i = 0; i < listCount; i++)
			{
				// Get the list item
				syncItem = syncWaitList.GetItem(i);

				// Found it?
				if ((syncItem.threadID == curThread) && (syncItem.trigged))
				{
					// Yup, get the ID
					result = syncItem.listID;
					break;
				}
			}
		}

		// Get the number of items in the wait list
		listCount = syncWaitList.CountItems();

		// Remove all the items with the current thread ID
		for (i = 0; i < listCount; i++)
		{
			// Get the list item
			syncItem = syncWaitList.GetItem(i);

			// Found it?
			if (syncItem.threadID == curThread)
			{
				// We know that all the sync objects are stored
				// right after each other in the list, so we can
				// remove them all in one go...
				syncWaitList.RemoveItems(i, count);
				break;
			}
		}

		// Remove the trigger for the list
		listCount = syncEventList.CountItems();

		for (i = 0; i < listCount; i++)
		{
			// Get the list item
			workEventItem = syncEventList.GetItem(i);

			// Found it?
			if (workEventItem.threadID == curThread)
			{
				// Yup, remove it
				syncEventList.RemoveItem(i);
				break;
			}
		}

		// Destroy the trigger event
		delete eventItem.event;

		// Unlock the list
		syncListLock.Unlock();

		// Got an object?
		if (err == pSyncOk)
			return (result);

		return (err);
	}
}





/******************************************************************************/
/* PSync class                                                                */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PSync::PSync(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PSync::~PSync(void)
{
}



/******************************************************************************/
/* TrigMultiWait() will tell any threads that wait for the object, that the   */
/*      object has been released.                                             */
/******************************************************************************/
void PSync::TrigMultiWait(void)
{
	int32 syncCount, i;
	int32 eventCount, j;
	PSyncItem syncItem;
	PEventItem eventItem;

	// If it's called from the sync list lock, just exit
	if (this == &syncListLock)
		return;

	// Lock the wait list
	syncListLock.Lock();

	// Get the counts on the 2 lists
	syncCount  = syncWaitList.CountItems();
	eventCount = syncEventList.CountItems();

	// Traverse the wait list and see if the current object is stored in it
	for (i = 0; i < syncCount; i++)
	{
		// Get the current item
		syncItem = syncWaitList.GetItem(i);

		// Did we found ourselves?
		if (syncItem.syncObject == this)
		{
			// Found it, now set the object as trigged and tell the thread
			// that wait for it
			syncItem.trigged = true;
			syncWaitList.SetItem(syncItem, i);

			// Search the event list to find the trigger event
			for (j = 0; j < eventCount; j++)
			{
				// Get the event item
				eventItem = syncEventList.GetItem(j);

				// Found the thread?
				if (eventItem.threadID == syncItem.threadID)
				{
					// Yup, trig it
					eventItem.event->SetEvent();
					break;
				}
			}
		}
	}

	// Unlock the list again
	syncListLock.Unlock();
}





/******************************************************************************/
/* PSemaphore class                                                           */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "count" is the start count of the semaphore.                       */
/*         "lockSameThread" set this if you want the thread that already got  */
/*         the semaphore to be locked when it calls this function again.      */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PSemaphore::PSemaphore(uint32 count, bool lockSameThread)
{
	Initialize("Some PSemaphore", count, lockSameThread);
}



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "name" is the name of the semaphore.                               */
/*         "count" is the start count of the semaphore.                       */
/*         "lockSameThread" set this if you want the thread that already got  */
/*         the semaphore to be locked when it calls this function again.      */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PSemaphore::PSemaphore(PString name, uint32 count, bool lockSameThread)
{
	Initialize(name, count, lockSameThread);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PSemaphore::~PSemaphore(void)
{
	// Delete the semaphore
	VERIFY(delete_sem(semID) == B_NO_ERROR);
}



/******************************************************************************/
/* Initialize() initializes the semaphore.                                    */
/*                                                                            */
/* Input:  "name" is the name of the semaphore.                               */
/*         "count" is the start count of the semaphore.                       */
/*         "lockSameThread" set this if you want the thread that already got  */
/*         the semaphore to be locked when it calls this function again.      */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PSemaphore::Initialize(PString name, uint32 count, bool lockSameThread)
{
	char *nameStr;

	// Create the semaphore
	semID = create_sem(count, (nameStr = name.GetString()));
	name.FreeBuffer(nameStr);

	if (semID < B_NO_ERROR)
		throw PSystemException(PSystem::ConvertOSError(semID));

	// Initialize owner variables
	owner      = 0;
	ownerCount = 0;
	lockThread = lockSameThread;
}



/******************************************************************************/
/* Lock() will wait on the object and first return when it got it or a        */
/*      timeout has occured.                                                  */
/*                                                                            */
/* Input:  "timeout" is the timeout value in milliseconds.                    */
/*                                                                            */
/* Output: pSyncOk if you got the object, else one of the PSyncError codes.   */
/******************************************************************************/
PSyncError PSemaphore::Lock(uint32 timeout)
{
	status_t err;
	thread_id callThread;

	// Get the current thread id
	callThread = find_thread(NULL);

	// Check to see if the calling thread already have the semaphore
	if (!lockThread && (callThread == owner))
	{
		ownerCount++;
		return (pSyncOk);
	}

	// Try to acquire the semaphore
	if (timeout == PSYNC_INFINITE)
		err = acquire_sem(semID);
	else
		err = acquire_sem_etc(semID, 1, B_TIMEOUT, timeout * 1000);

	if (err == B_NO_ERROR)
	{
		owner      = callThread;
		ownerCount = 1;

		return (pSyncOk);
	}

	if ((err == B_TIMED_OUT) || ((err == B_WOULD_BLOCK) && (timeout == 0)))
		return (pSyncTimeout);

	return (pSyncError);
}



/******************************************************************************/
/* Unlock() will release the object.                                          */
/*                                                                            */
/* Output: pSyncOk if you the object got released else pSyncError.            */
/******************************************************************************/
PSyncError PSemaphore::Unlock(void)
{
	// Release the semaphore
	status_t err;

	// Decrement the owner counter
	ownerCount--;

	if ((ownerCount <= 0) || (lockThread))
	{
		owner      = 0;
		ownerCount = 0;

		err = release_sem_etc(semID, 1, B_DO_NOT_RESCHEDULE);
		if (err != B_NO_ERROR)
			return (pSyncError);

		// Tell any multi waiting thread about the unlock
		TrigMultiWait();
	}

	return (pSyncOk);
}



/******************************************************************************/
/* LockWithCount() will wait on the object and first return when it got       */
/*      "count" locks or a timeout has occured.                               */
/*                                                                            */
/* Input:  "timeout" is the timeout value in milliseconds.                    */
/*                                                                            */
/* Output: pSyncOk if you got the object, else one of the PSyncError codes.   */
/******************************************************************************/
PSyncError PSemaphore::LockWithCount(uint32 count, uint32 timeout)
{
	status_t err;
	thread_id callThread;

	// Get the current thread id
	callThread = find_thread(NULL);

	// Check to see if the calling thread already have the semaphore
	if (!lockThread && (callThread == owner))
	{
		ownerCount += count;
		return (pSyncOk);
	}

	// Try to acquire the semaphore
	if (timeout == PSYNC_INFINITE)
		err = acquire_sem_etc(semID, count, 0, 0);
	else
		err = acquire_sem_etc(semID, count, B_TIMEOUT, timeout * 1000);

	if (err == B_NO_ERROR)
	{
		owner      = callThread;
		ownerCount = count;

		return (pSyncOk);
	}

	if ((err == B_TIMED_OUT) || ((err == B_WOULD_BLOCK) && (timeout == 0)))
		return (pSyncTimeout);

	return (pSyncError);
}



/******************************************************************************/
/* UnlockWithCount() will release the object "count" times.                   */
/*                                                                            */
/* Output: pSyncOk if you the object got released else pSyncError.            */
/******************************************************************************/
PSyncError PSemaphore::UnlockWithCount(uint32 count)
{
	// Release the semaphore
	status_t err;

	// Decrement the owner counter
	ownerCount -= count;

	if ((ownerCount <= 0) || (lockThread))
	{
		owner      = 0;
		ownerCount = 0;

		err = release_sem_etc(semID, count, B_DO_NOT_RESCHEDULE);
		if (err != B_NO_ERROR)
			return (pSyncError);

		// Tell any multi waiting thread about the unlock
		TrigMultiWait();
	}

	return (pSyncOk);
}





/******************************************************************************/
/* PMutex class                                                               */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "lockSameThread" set this if you want the thread that already got  */
/*         the semaphore to be locked when it calls this function again.      */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PMutex::PMutex(bool lockSameThread)
{
	Initialize("Some PMutex", lockSameThread);
}



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "name" is the name of the mutex.                                   */
/*         "lockSameThread" set this if you want the thread that already got  */
/*         the semaphore to be locked when it calls this function again.      */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PMutex::PMutex(PString name, bool lockSameThread)
{
	Initialize(name, lockSameThread);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PMutex::~PMutex(void)
{
	// Delete the semaphore
	VERIFY(delete_sem(semID) == B_NO_ERROR);
}



/******************************************************************************/
/* Initialize() initializes the mutex.                                        */
/*                                                                            */
/* Input:  "name" is the name of the mutex.                                   */
/*         "lockSameThread" set this if you want the thread that already got  */
/*         the mutex to be locked when it calls this function again.          */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PMutex::Initialize(PString name, bool lockSameThread)
{
	char *nameStr;

	// Create the semaphore
	semID = create_sem(1, (nameStr = name.GetString()));
	name.FreeBuffer(nameStr);

	if (semID < B_NO_ERROR)
		throw PSystemException(PSystem::ConvertOSError(semID));

	// Initialize owner variables
	owner      = 0;
	ownerCount = 0;
	lockThread = lockSameThread;
}



/******************************************************************************/
/* Lock() will wait on the object and first return when it got it or a        */
/*      timeout has occured.                                                  */
/*                                                                            */
/* Input:  "timeout" is the timeout value in milliseconds.                    */
/*                                                                            */
/* Output: pSyncOk if you got the object, else one of the PSyncError codes.   */
/******************************************************************************/
PSyncError PMutex::Lock(uint32 timeout)
{
	status_t err;
	thread_id callThread;

	// Get the current thread id
	callThread = find_thread(NULL);

	// Check to see if the calling thread already have the semaphore
	if (!lockThread && (callThread == owner))
	{
		ownerCount++;
		return (pSyncOk);
	}

	// Try to acquire the semaphore
	if (timeout == PSYNC_INFINITE)
		err = acquire_sem(semID);
	else
		err = acquire_sem_etc(semID, 1, B_TIMEOUT, timeout * 1000);

	if (err == B_NO_ERROR)
	{
		owner      = callThread;
		ownerCount = 1;

		return (pSyncOk);
	}

	if ((err == B_TIMED_OUT) || ((err == B_WOULD_BLOCK) && (timeout == 0)))
		return (pSyncTimeout);

	return (pSyncError);
}



/******************************************************************************/
/* Unlock() will release the object.                                          */
/*                                                                            */
/* Output: pSyncOk if you the object got released else pSyncError.            */
/******************************************************************************/
PSyncError PMutex::Unlock(void)
{
	status_t err;

	// Decrement the owner counter
	ownerCount--;

	if ((ownerCount <= 0) || (lockThread))
	{
		owner      = 0;
		ownerCount = 0;

		// Release the semaphore
		err = release_sem_etc(semID, 1, B_DO_NOT_RESCHEDULE);
		if (err != B_NO_ERROR)
			return (pSyncError);

		// Tell any multi waiting thread about the unlock
		TrigMultiWait();
	}

	return (pSyncOk);
}





/******************************************************************************/
/* PEvent class                                                               */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "manualReset" indicates if you want to create a manual or          */
/*         automatic reset event.                                             */
/*         "initialState" indicates what state you want the event to be       */
/*         created in.                                                        */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PEvent::PEvent(bool manualReset, bool initialState)
{
	Initialize("Some PEvent", manualReset, initialState);
}



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "name" is the name of the event.                                   */
/*         "manualReset" indicates if you want to create a manual or          */
/*         automatic reset event.                                             */
/*         "initialState" indicates what state you want the event to be       */
/*         created in.                                                        */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PEvent::PEvent(PString name, bool manualReset, bool initialState)
{
	Initialize(name, manualReset, initialState);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PEvent::~PEvent(void)
{
	// Delete the semaphore
	VERIFY(delete_sem(semID) == B_NO_ERROR);
}



/******************************************************************************/
/* Initialize() initializes the event.                                        */
/*                                                                            */
/* Input:  "name" is the name of the event.                                   */
/*         "manualReset" indicates if you want to create a manual or          */
/*         automatic reset event.                                             */
/*         "initialState" indicates what state you want the event to be       */
/*         created in.                                                        */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PEvent::Initialize(PString name, bool manualReset, bool initialState)
{
	char *nameStr;

	// Remember the arguments
	state         = initialState;
	manual        = manualReset;
	releaseThread = false;

	// Create the semaphore
	if (state)
		semID = create_sem(1, (nameStr = name.GetString()));
	else
		semID = create_sem(0, (nameStr = name.GetString()));

	name.FreeBuffer(nameStr);

	if (semID < B_NO_ERROR)
		throw PSystemException(PSystem::ConvertOSError(semID));
}



/******************************************************************************/
/* Lock() will wait on the object and first return when it got it or a        */
/*      timeout has occured.                                                  */
/*                                                                            */
/* Input:  "timeout" is the timeout value in milliseconds.                    */
/*                                                                            */
/* Output: pSyncOk if you got the object, else one of the PSyncError codes.   */
/******************************************************************************/
PSyncError PEvent::Lock(uint32 timeout)
{
	status_t err;
	bool releaseMe;

	// First check to see if the event has been set
	if (state)
		return (pSyncOk);		// It has, so just return

	do
	{
		// Try to acquire the semaphore
		if (timeout == PSYNC_INFINITE)
			err = acquire_sem(semID);
		else
			err = acquire_sem_etc(semID, 1, B_TIMEOUT, timeout * 1000);

		if ((err == B_TIMED_OUT) || ((err == B_WOULD_BLOCK) && (timeout == 0)))
			return (pSyncTimeout);

		if (err != B_NO_ERROR)
			return (pSyncError);

		releaseMe     = releaseThread;
		releaseThread = false;

		// Release the semaphore again so other waiting threads will
		// be signaled too
		err = release_sem_etc(semID, 1, B_DO_NOT_RESCHEDULE);
		if (err != B_NO_ERROR)
			return (pSyncError);
	}
	while ((!state) && (!releaseMe));

	return (pSyncOk);
}



/******************************************************************************/
/* Unlock() will release the object.                                          */
/*                                                                            */
/* Output: pSyncOk if you the object got released else pSyncError.            */
/******************************************************************************/
PSyncError PEvent::Unlock(void)
{
	return (pSyncError);
}



/******************************************************************************/
/* SetEvent() will set the event.                                             */
/*                                                                            */
/* Output: pSyncOk for success, pSyncError for an error.                      */
/******************************************************************************/
PSyncError PEvent::SetEvent(void)
{
	status_t err;

	// First check to see if the event already is set
	if (state)
		return (pSyncOk);		// It is, so just return

	// Set the event
	if (manual)
		state = true;
	else
		releaseThread = true;

	err = release_sem_etc(semID, 1, B_DO_NOT_RESCHEDULE);
	if (err == B_NO_ERROR)
	{
		if (!manual)
		{
			// Automatic event, wait a little bit so we get a task-switch
			// and then reset the event again
			snooze(1000);
			acquire_sem(semID);
		}

		// Tell any multi waiting thread about the unlock
		TrigMultiWait();

		return (pSyncOk);
	}

	return (pSyncError);
}



/******************************************************************************/
/* ResetEvent() will clear the event.                                         */
/*                                                                            */
/* Output: pSyncOk for success, pSyncError for an error.                      */
/******************************************************************************/
PSyncError PEvent::ResetEvent(void)
{
	status_t err;

	// First check to see if the event already is cleared
	if (!state)
		return (pSyncOk);		// It is, so just return

	// Clear the event
	err = acquire_sem(semID);
	if (err == B_NO_ERROR)
	{
		state = false;
		return (pSyncOk);
	}

	return (pSyncError);
}





/******************************************************************************/
/* PMRSWLock class                                                            */
/*                                                                            */
/* Note: The same thread can't first lock with read access and then with      */
/*       write access. This will make a deadlock. On the other hand, you can  */
/*       do it the other way around.                                          */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PMRSWLock::PMRSWLock(void)
{
	Initialize("Some PMRSWLock");
}



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "name" is the name of the lock.                                    */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PMRSWLock::PMRSWLock(PString name)
{
	Initialize(name);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PMRSWLock::~PMRSWLock(void)
{
	delete writeLockSem;
	delete writeSem;
	delete readSem;
	delete mainLock;
}



/******************************************************************************/
/* WaitToWrite() will wait on the object until it's ready to gain write       */
/*      access. It will first return when it's safe to write or a timeout has */
/*      occured.                                                              */
/*                                                                            */
/* Input:  "timeout" is the timeout value in milliseconds.                    */
/*                                                                            */
/* Output: pSyncOk if you got the object, else one of the PSyncError codes.   */
/******************************************************************************/
PSyncError PMRSWLock::WaitToWrite(uint32 timeout)
{
	PSyncError locked = pSyncError;

	// Does the current thread already hold the lock for writing?
	if (FindThreadID() == writerThread)
	{
		// Yup, just increment the nesting count
		writerNest++;
		locked = pSyncOk;
	}
	else
	{
		// Wait for the main lock
		locked = mainLock->Lock(timeout);

		if (locked == pSyncOk)
		{
			// New writer want the lock, but does any other have the write lock?
			if ((writeCount > 0) || (writeWaitCount > 0))
			{
				// Yap, put our thread behind the queue
				writeWaitCount++;

				// Done with the main lock
				mainLock->Unlock();

				// Wait until its our turn
				locked = writeLockSem->Lock(timeout);

				if (locked == pSyncOk)
				{
					// Wait for the main lock
					locked = mainLock->Lock(timeout);
				}

				if (locked == pSyncOk)
					writeWaitCount--;
			}

			if (locked == pSyncOk)
			{
				// Increment the write count
				writeCount++;
				ASSERT(writeCount == 1);

				// Does any thread have a read lock?
				if (readList.CountItems() > 0)
				{
					// Done with the main lock
					mainLock->Unlock();

					// Yip, wait until they are done
					locked = writeSem->Lock(timeout);

					if (locked == pSyncOk)
					{
						// Remember the thread id
						writerThread = FindThreadID();
						return (pSyncOk);
					}
				}

				if (locked == pSyncOk)
				{
					// Got the lock, remember the thread id
					writerThread = FindThreadID();

					// Done with the main lock
					mainLock->Unlock();
				}
			}
		}
	}

	return (locked);
}



/******************************************************************************/
/* DoneWriting() will release the write access, so other threads can read or  */
/*      write.                                                                */
/*                                                                            */
/* Output: pSyncOk for success, pSyncError for an error.                      */
/******************************************************************************/
PSyncError PMRSWLock::DoneWriting(void)
{
	PSyncError unlocked = pSyncError;
	int32 readersWaiting;

	// It has to be the current thread that hold the lock for writing
	if (FindThreadID() == writerThread)
	{
		// If this is a nested lock, just decrement the nest count
		if (writerNest > 0)
		{
			writerNest--;
			unlocked = pSyncOk;
		}
		else
		{
			// Writer finally unlocking
			//
			// Clear thread id
			writerThread = -1;

			// Wait for the main lock
			unlocked = mainLock->Lock();

			if (unlocked == pSyncOk)
			{
				// Decrement the lock counter
				writeCount--;
				ASSERT(writeCount >= 0);		// Did you unlock one time too much?

				// Any threads waiting for write access
				if (writeWaitCount > 0)
				{
					// Yes, tell the thread to continue
					unlocked = writeLockSem->Unlock();
				}
				else
				{
					// Nap, but does any thread then wait for read access
					if (readWaitCount > 0)
					{
						// Yup, tell the thread to continue
						readersWaiting = readWaitCount;
						readWaitCount  = 0;

						unlocked = readSem->UnlockWithCount(readersWaiting);
					}
					else
						unlocked = pSyncOk;
				}

				// Done with the main lock
				mainLock->Unlock();
			}
		}
	}

	return (unlocked);
}



/******************************************************************************/
/* WaitToRead() will wait on the object until it's ready to gain read access. */
/*      It will first return when it's safe to read or a timeout has occured. */
/*                                                                            */
/* Input:  "timeout" is the timeout value in milliseconds.                    */
/*                                                                            */
/* Output: pSyncOk if you got the object, else one of the PSyncError codes.   */
/******************************************************************************/
PSyncError PMRSWLock::WaitToRead(uint32 timeout)
{
	int32 thread;
	PSyncError locked = pSyncError;

	// Get the calling thread id
	thread = FindThreadID();

	// Does the current thread hold the lock for writing?
	if (thread == writerThread)
	{
		// We just increment the nesting
		writerNest++;
		locked = pSyncOk;
	}
	else
	{
		// Wait for the main lock
		locked = mainLock->Lock(timeout);

		if (locked == pSyncOk)
		{
			PReadLockInfo info;
			int32 i, count;
			bool found = false;

			// Now check to see if the calling thread already has a read lock
			count = readList.CountItems();
			for (i = 0; i < count; i++)
			{
				info = readList.GetItem(i);
				if (info.thread == thread)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				// Do a write thread have the lock or any writer threads waiting?
				if ((writeCount > 0) || (writeWaitCount > 0))
				{
					// Yes, put our thread behind the queue
					readWaitCount++;

					// Done with the main lock
					mainLock->Unlock();

					// Wait until its our turn
					locked = readSem->Lock(timeout);

					if (locked == pSyncOk)
					{
						// Wait for the main lock
						locked = mainLock->Lock(timeout);
					}
				}
			}

			if (locked == pSyncOk)
			{
				// Does the thread already exist in the list
				if (found)
				{
					// Yes, just increment the counter
					info.count++;
					readList.SetItem(info, i);
				}
				else
				{
					// No, create a new element and add it to the list
					info.thread = thread;
					info.count  = 1;
					readList.AddTail(info);
				}

				// Done with the main lock
				mainLock->Unlock();
			}
		}
	}

	return (locked);
}



/******************************************************************************/
/* DoneReading() will release the read access, so other threads can write.    */
/*                                                                            */
/* Output: pSyncOk for success, pSyncError for an error.                      */
/******************************************************************************/
PSyncError PMRSWLock::DoneReading(void)
{
	int32 thread;
	PSyncError unlocked = pSyncError;

	// Get the calling thread id
	thread = FindThreadID();

	// Does the current thread hold the lock for writing?
	if (thread == writerThread)
	{
		// Decrement the nesting count
		writerNest--;
		unlocked = pSyncOk;
	}
	else
	{
		// Wait for the main lock
		unlocked = mainLock->Lock();

		if (unlocked == pSyncOk)
		{
			PReadLockInfo info;
			int32 i, count;

			// Done with the read lock, so find the thread in the list
			count = readList.CountItems();
			for (i = 0; i < count; i++)
			{
				info = readList.GetItem(i);
				if (info.thread == thread)
				{
					// Decrement the counter
					info.count--;

					// Should we remove the element from the list?
					if (info.count == 0)
						readList.RemoveItem(i);
					else
						readList.SetItem(info, i);

					break;
				}
			}

			// Is this the last read lock and a writer lock is waiting?
			if ((readList.CountItems() == 0) && (writeCount > 0))
			{
				// Tell the write lock its ok to lock
				unlocked = writeSem->Unlock();
			}
			else
				unlocked = pSyncOk;

			// Done with the main lock
			mainLock->Unlock();
		}
	}

	return (unlocked);
}



/******************************************************************************/
/* Initialize() initializes the locker.                                       */
/*                                                                            */
/* Input:  "name" is the name of the locker.                                  */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PMRSWLock::Initialize(PString name)
{
	// Initialize member variables
	mainLock       = NULL;
	readWaitCount  = 0;
	readSem        = NULL;
	writeCount     = 0;
	writeWaitCount = 0;
	writeSem       = NULL;
	writeLockSem   = NULL;
	writerNest     = 0;
	writerThread   = -1;

	try
	{
		// Create the main guard mutex
		mainLock = new PMutex(name + " (main lock)", false);
		if (mainLock == NULL)
			throw PMemoryException();

		// Create the semaphore read locks wait for to gain access
		// when a writer have the lock
		readSem = new PSemaphore(name + " (read sem)", 0);
		if (readSem == NULL)
			throw PMemoryException();

		// Create the semaphore write locks wait for to gain access
		// when a reader have the lock
		writeSem = new PSemaphore(name + " (write sem)", 0);
		if (writeSem == NULL)
			throw PMemoryException();

		// Create the semaphore write locks wait for to gain access
		// when another writer have the lock
		writeLockSem = new PSemaphore(name + " (writeLock sem)", 0);
		if (writeLockSem == NULL)
			throw PMemoryException();
	}
	catch(PException *)
	{
		delete writeLockSem;
		delete writeSem;
		delete readSem;
		delete mainLock;
		throw;
	}
}



/******************************************************************************/
/* FindThreadID() returns the current thread ID.                              */
/*                                                                            */
/* Output: The thread ID.                                                     */
/******************************************************************************/
int32 PMRSWLock::FindThreadID(void)
{
	return (find_thread(NULL));
}





/******************************************************************************/
/* PLock class                                                                */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "lock" is a pointer to the lock to use.                            */
/******************************************************************************/
PLock::PLock(PSync *lock)
{
	// Remember the lock
	curLock = lock;

	// And lock it
	curLock->Lock();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PLock::~PLock(void)
{
	// Unlock the lock
	curLock->Unlock();
}
