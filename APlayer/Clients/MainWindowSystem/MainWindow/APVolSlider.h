/******************************************************************************/
/* APVolSlider header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APVolSlider_h
#define __APVolSlider_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* Hook function type defination                                              */
/******************************************************************************/
typedef void (*valueChangedFunc)(uintptr_t data, float newValue);



/******************************************************************************/
/* APVolSlider class                                                          */
/******************************************************************************/
class APVolSlider : public BSlider
{
public:
	APVolSlider(float min, float max, orientation posture, uint32 resizingMode);
	virtual ~APVolSlider(void);

	void SetHookFunction(valueChangedFunc func, uintptr_t userData);

protected:
	void MessageReceived(BMessage* message);
	void AttachedToWindow();

	valueChangedFunc hookFunc;
	uintptr_t userDat;
};

#endif
