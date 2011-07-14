/******************************************************************************/
/* PTimer implementation file.                                                */
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
#include "PString.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "PTimer.h"


/******************************************************************************/
/* PTimer class                                                               */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "looper" is a pointer to the looper to send the message to.        */
/*         "id" is the timer id.                                              */
/******************************************************************************/
PTimer::PTimer(BLooper *looper, uint16 id)
{
	// Initialize member variables
	msgLooper  = looper;
	runner     = NULL;
	running    = false;
	waitTime   = 1;

	// Initialize the message
	message.what = PM_TIMER;
	message.AddInt16("ID", id);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PTimer::~PTimer(void)
{
	// Stop the timer
	StopTimer();
}



/******************************************************************************/
/* SetElapseValue() will change the time to elapse before a message is sent.  */
/*                                                                            */
/* Input:  "value" is the number to wait in milliseconds.                     */
/******************************************************************************/
void PTimer::SetElapseValue(uint32 value)
{
	waitTime = value;
}



/******************************************************************************/
/* StartTimer() will start the timer.                                         */
/******************************************************************************/
void PTimer::StartTimer(void)
{
	// Start the timer
	if (!running)
	{
		runner = new BMessageRunner(BMessenger(msgLooper), &message, (bigtime_t)waitTime * 1000);
		if (runner == NULL)
			throw PMemoryException();

		running = true;
	}
}



/******************************************************************************/
/* StopTimer() will stop the timer.                                           */
/******************************************************************************/
void PTimer::StopTimer(void)
{
	// Stop the timer
	if (running)
	{
		delete runner;
		runner  = NULL;
		running = false;
	}
}
