/******************************************************************************/
/* PThread implementation file.                                               */
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
#include "PThread.h"
#include "PSystem.h"


/******************************************************************************/
/* PThread class                                                              */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PThread::PThread(void)
{
	// Initialize member variables
	threadRunning = false;
	threadPri     = pNormal;
	threadName    = "Some PThread";
	threadFunc    = NULL;

	// Create start signal
	threadStart = new PEvent(true, false);
	if (threadStart == NULL)
		throw PMemoryException();

	// Create suspend signal
	suspendSignal = new PEvent(true, true);
	if (suspendSignal == NULL)
		throw PMemoryException();

	suspendCounter = 0;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PThread::~PThread(void)
{
	ASSERT(threadRunning == false);

	// Delete the suspend signal
	delete suspendSignal;
	delete threadStart;
}



/******************************************************************************/
/* StartThread() will start the thread.                                       */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PThread::StartThread(void)
{
	char *nameStr;

	ASSERT(threadRunning == false);
	ASSERT(threadFunc != NULL);

	// Spawn the thread
	tid = spawn_thread(ThreadFunc, (nameStr = threadName.GetString()), ConvertPriority(), this);
	threadName.FreeBuffer(nameStr);

	if (tid < 0)
		throw PSystemException(PSystem::ConvertOSError(tid));

	// Start the thread
	resume_thread(tid);

	// Wait until the thread have started running
	threadStart->Lock();
}



/******************************************************************************/
/* WaitOnThread() will wait until the thread exits.                           */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PThread::WaitOnThread(void) const
{
	status_t exitVal, retVal;

	// Exit if the thread is not running
	if (!threadRunning)
		return;

	// First wait on the suspend signal. If it is set, the thread is running
	if (suspendSignal->Lock() == pSyncError)
		throw PSystemException(P_ERR_ANY);

	// Wait on the thread
	retVal = wait_for_thread(tid, &exitVal);
	if (retVal != B_OK)
		throw PSystemException(PSystem::ConvertOSError(retVal));
}



/******************************************************************************/
/* GetExitCode() will return the code your thread returned.                   */
/*                                                                            */
/* Output: The exit code.                                                     */
/******************************************************************************/
int32 PThread::GetExitCode(void) const
{
	ASSERT(threadRunning == false);

	return (exitCode);
}



/******************************************************************************/
/* Resume() will resume the thread if it was suspended.                       */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PThread::Resume(void)
{
	status_t retVal;

	ASSERT(threadRunning == true);

	// Decrement the suspend counter
	if (AtomicDecrement(&suspendCounter) == 0)
	{
		// Set the suspend signal
		if (suspendSignal->SetEvent() == pSyncError)
			throw PSystemException(P_ERR_ANY);

		// Start the thread
		retVal = resume_thread(tid);
		if (retVal != B_OK)
			throw PSystemException(PSystem::ConvertOSError(retVal));
	}
}



/******************************************************************************/
/* Suspend() will suspend the thread.                                         */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PThread::Suspend(void)
{
	status_t retVal;

	ASSERT(threadRunning == true);

	// Increment the suspend counter
	if (AtomicIncrement(&suspendCounter) == 1)
	{
		// Reset the suspend signal
		if (suspendSignal->ResetEvent() == pSyncError)
			throw PSystemException(P_ERR_ANY);

		// Suspend the thread
		retVal = suspend_thread(tid);
		if (retVal != B_OK)
			throw PSystemException(PSystem::ConvertOSError(retVal));
	}
}



/******************************************************************************/
/* Kill() will try to kill the thread.                                        */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PThread::Kill(void)
{
	status_t retVal;

	ASSERT(threadRunning == true);

	// Kill the thread
	retVal = kill_thread(tid);
	if (retVal != B_OK)
		throw PSystemException(PSystem::ConvertOSError(retVal));

	// Clear the suspend counter
	suspendCounter = 0;
	suspendSignal->SetEvent();

	// The thread isn't running anymore
	threadRunning = false;
}



/******************************************************************************/
/* IsAlive() tells if the thread is still running.                            */
/******************************************************************************/
bool PThread::IsAlive(void) const
{
	return (threadRunning);
}



/******************************************************************************/
/* SetHookFunc() will tell the class what function to run.                    */
/*                                                                            */
/* Input:  "func" is a pointer to the function.                               */
/*         "userData" can be anything you desire. It will be given to your    */
/*         function as argument.                                              */
/******************************************************************************/
void PThread::SetHookFunc(PThreadFunc func, void *userData)
{
	ASSERT(func != NULL);

	// Remember the data
	threadFunc = func;
	threadData = userData;
}



/******************************************************************************/
/* SetName() will change the thread name.                                     */
/*                                                                            */
/* Input:  "name" is the new thread name.                                     */
/******************************************************************************/
void PThread::SetName(PString name)
{
	threadName = name;
}



/******************************************************************************/
/* SetPriority() will change the thread priority. If the thread is running,   */
/*      it will change the priority immediately.                              */
/*                                                                            */
/* Input:  "pri" is the new priority.                                         */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
void PThread::SetPriority(PPriority pri)
{
	// Remember the priority
	threadPri = pri;

	// Set the thread priority if the thread is running
	if (threadRunning)
	{
		status_t retVal;

		retVal = set_thread_priority(tid, ConvertPriority());
		if (retVal != B_OK)
			throw PSystemException(PSystem::ConvertOSError(retVal));
	}
}



/******************************************************************************/
/* GetPriority() will return the current priority of the thread.              */
/*                                                                            */
/* Output: Is the current priority.                                           */
/******************************************************************************/
PThread::PPriority PThread::GetPriority(void) const
{
	return (threadPri);
}



/******************************************************************************/
/* ThreadFunc() is the thread function that will be started. It will call the */
/*      user function and remember the exit code.                             */
/*                                                                            */
/* Input:  "object" is a pointer to the current PThread object.               */
/******************************************************************************/
int32 PThread::ThreadFunc(void *object)
{
	PThread *obj = (PThread *)object;

	// Call the user thread
	obj->threadRunning = true;
	obj->threadStart->SetEvent();	// Set the thread start event
	obj->exitCode      = obj->threadFunc(obj->threadData);
	obj->threadRunning = false;

	return (obj->exitCode);
}



/******************************************************************************/
/* ConvertPriority() will convert a PolyKit priority to an OS priority.       */
/*                                                                            */
/* Output: Is the OS priority.                                                */
/******************************************************************************/
int32 PThread::ConvertPriority(void) const
{
	int32 pri;

	switch (threadPri)
	{
		case pIdle:
		{
			pri = 1;
			break;
		}

		case pLow:
		{
			pri = B_LOW_PRIORITY;
			break;
		}

		case pBelowNormal:
		{
			pri = B_NORMAL_PRIORITY - ((B_NORMAL_PRIORITY - B_LOW_PRIORITY) / 2);
			break;
		}

		case pAboveNormal:
		{
			pri = B_NORMAL_PRIORITY + ((B_NORMAL_PRIORITY - B_LOW_PRIORITY) / 2);
			break;
		}

		case pHigh:
		{
			pri = B_DISPLAY_PRIORITY;
			break;
		}

		case pRealTime:
		{
			pri = B_REAL_TIME_PRIORITY;
			break;
		}

		default:
		{
			pri = B_NORMAL_PRIORITY;
			break;
		}
	}

	return (pri);
}
