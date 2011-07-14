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
typedef void (*valueChangedFunc)(uint32 data, float newValue);



/******************************************************************************/
/* APVolSlider class                                                          */
/******************************************************************************/
class APVolSlider : public BScrollBar
{
public:
	APVolSlider(float min, float max, orientation posture, uint32 resizingMode);
	virtual ~APVolSlider(void);

	void SetHookFunction(valueChangedFunc func, uint32 userData);

protected:
	void ValueChanged(float newValue);

	valueChangedFunc hookFunc;
	uint32 userDat;
};

#endif
