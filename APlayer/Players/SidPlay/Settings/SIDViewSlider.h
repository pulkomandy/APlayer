/******************************************************************************/
/* SIDViewSlider header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDViewSlider_h
#define __SIDViewSlider_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// Player headers
#include "SIDViewFormular.h"


/******************************************************************************/
/* SIDViewSlider class                                                        */
/******************************************************************************/
class SIDViewSlider : public BSlider
{
public:
	SIDViewSlider(BRect frame, PString label, BMessage *message, int32 minValue, int32 maxValue, thumb_style thumbType = B_BLOCK_THUMB, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
	virtual ~SIDViewSlider(void);

	const char *Label(void) const;
	void SetLabelWidth(float width);

protected:
	virtual BRect BarFrame(void) const;
	virtual void DrawText(void);

	PString barLabel;
	char *barLabelStr;
	float labelLength;
	float fontHeight;
	float baseLine;

	float thumbHalfWidth;
};



/******************************************************************************/
/* SIDViewSliderFilter class                                                  */
/******************************************************************************/
class SIDViewSliderFilter : public SIDViewSlider
{
public:
	SIDViewSliderFilter(SIDViewFormular *view, uint8 num, BRect frame, PString label, BMessage *message, int32 minValue, int32 maxValue, thumb_style thumbType = B_BLOCK_THUMB, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
	virtual ~SIDViewSliderFilter(void);

protected:
	virtual void DrawThumb(void);

	SIDViewFormular *formular;
	uint8 parNum;

	float oldPar1;
	float oldPar2;
	float oldPar3;
};



/******************************************************************************/
/* SIDViewSliderPanning class                                                 */
/******************************************************************************/
class SIDViewSliderPanning : public SIDViewSlider
{
public:
	SIDViewSliderPanning(uint8 channel, BRect frame, PString label, BMessage *message, int32 minValue, int32 maxValue, thumb_style thumbType = B_BLOCK_THUMB, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE);
	virtual ~SIDViewSliderPanning(void);

protected:
	virtual void DrawBar(void);

	uint8 chanNum;
};

#endif
