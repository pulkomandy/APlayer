/******************************************************************************/
/* ReverbViewSlider header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ReverbViewSlider_h
#define __ReverbViewSlider_h

// PolyKit headers
#include "POS.h"
#include "PString.h"


/******************************************************************************/
/* ReverbViewSlider class                                                     */
/******************************************************************************/
typedef PString (*ValueFunc)(void *, int32);

class ReverbViewSlider : public BSlider
{
public:
	ReverbViewSlider(BRect frame, PString label, bool labelPos, BMessage *message, int32 minValue, int32 maxValue, float valWidth = 0.0, thumb_style thumbType = B_BLOCK_THUMB, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
	virtual ~ReverbViewSlider(void);

	void SetLabelWidth(float width);
	void SetValueFunction(ValueFunc func, void *user);

	virtual BRect BarFrame(void) const;
	virtual void DrawText(void);
	virtual void DrawBar(void);

protected:
	PString barLabel;
	bool labelPosition;
	float labelLength;
	float fontHeight;
	float baseLine;

	float thumbHalfWidth;

	BMessage *sendMsg;
	int32 oldValue;

	float valueWidth;
	ValueFunc valueFunc;
	void *userData;
};

#endif
