/******************************************************************************/
/* APlayer Volume Slider class.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/

#include <iostream>

// PolyKit headers
#include "POS.h"

// Client headers
#include "APVolSlider.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APVolSlider::APVolSlider(float min, float max, orientation posture, uint32 resizingMode) :
	BSlider("volslide", NULL, new BMessage('SLID'), min, max, posture)
{
	SetModificationMessage(new BMessage('SLID'));
	hookFunc = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APVolSlider::~APVolSlider(void)
{
}



/******************************************************************************/
/* SetHookFunction() sets the function pointer to be called every time the    */
/*      slider value change.                                                  */
/*                                                                            */
/* Input:  "func" is a pointer to the function to call.                       */
/*         "userData" is some data on your own.                               */
/******************************************************************************/
void APVolSlider::SetHookFunction(valueChangedFunc func, uintptr_t userData)
{
	hookFunc = func;
	userDat  = userData;
}


void APVolSlider::AttachedToWindow()
{
	BSlider::AttachedToWindow();

	// BSlider will send messages to the window, which is not what we need here.
	SetTarget(this);
}


void APVolSlider::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case 'SLID':
			if (hookFunc != NULL) {
				int32 newValue = 0;
				message->FindInt32("be:value", 0, &newValue);
				(*hookFunc)(userDat, newValue);
			}
			break;
		default:
			BSlider::MessageReceived(message);
	}
}
