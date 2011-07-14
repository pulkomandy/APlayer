/******************************************************************************/
/* ScopeView header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ScopeView_h
#define __ScopeView_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"


/******************************************************************************/
/* Some useful macros                                                         */
/******************************************************************************/
#define CHANNEL_SIZE				256
#define MAX_VIEW_HEIGHT				256.0f



/******************************************************************************/
/* ScopeView class                                                            */
/******************************************************************************/
class ScopeView : public BView
{
public:
	enum ScopeType { eFilled, eLines, eDots };

	ScopeView(BRect frame);
	virtual ~ScopeView(void);

	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint point);

	void SetScopeType(ScopeType type);
	ScopeType RotateScopeType(void);
	void DrawSample(const int16 *buffer, int32 len, bool stereo);

protected:
	void DrawFilled(float midPos);
	void DrawLines(float midPos);
	void DrawDots(float midPos);

	BBitmap *bitmap;
	BView *renderView;

	ScopeType scopeType;

	const int16 *sampleData;
	int32 sampleLen;
	bool sampleStereo;
};

#endif
