/******************************************************************************/
/* APlayer Volume Slider class.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Client headers
#include "APVolSlider.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APVolSlider::APVolSlider(float min, float max, orientation posture, uint32 resizingMode) :
			BScrollBar(BRect(0.0f, min, 0.0f, max), NULL, new BView(BRect(0.0f, min, 0.0f, max), NULL, resizingMode, 0), min, max, posture)
{
	SetResizingMode(resizingMode);

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
void APVolSlider::SetHookFunction(valueChangedFunc func, uint32 userData)
{
	hookFunc = func;
	userDat  = userData;
}



/******************************************************************************/
/* ValueChanged() is called when the slider value change.                     */
/*                                                                            */
/* Input:  "newValue" is the new value.                                       */
/******************************************************************************/
void APVolSlider::ValueChanged(float newValue)
{
	if (hookFunc != NULL)
		(*hookFunc)(userDat, newValue);

	BScrollBar::ValueChanged(newValue);
}
