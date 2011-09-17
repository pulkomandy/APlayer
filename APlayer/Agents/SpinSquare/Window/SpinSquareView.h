/******************************************************************************/
/* SpinSquareView header file.                                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SpinSquareView_h
#define __SpinSquareView_h

#include <Size.h>

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

/******************************************************************************/
/* SpinSquareView class                                                       */
/******************************************************************************/
class SpinSquareView : public BView
{
public:
	SpinSquareView();
	virtual ~SpinSquareView(void);

	virtual void Pulse(void);
	virtual void Draw(BRect updateRect);

	void ChannelChanged(uint32 flags, const APChannel *channel);
	void StopAnimation(void);
	
	BSize MinSize() {return BSize(64, 64);}

protected:
	bool animate;

	int16 boxXCoords[4];
	int16 boxYCoords[4];
	double speed;
	double angle;
	int16 oldVolume;
	int32 oldFrequency;

	BPoint drawCoords[4];
};

#endif
