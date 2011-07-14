/******************************************************************************/
/* APPictureButton header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APPictureButton_h
#define __APPictureButton_h

// PolyKit headers
#include "POS.h"
#include "PResource.h"


/******************************************************************************/
/* APPictureButton class                                                      */
/******************************************************************************/
class APPictureButton : public BPictureButton
{
public:
	APPictureButton(PResource *res, int32 iconID, BRect frame, BMessage *message, uint32 behaviour = B_ONE_STATE_BUTTON, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
	virtual ~APPictureButton(void);

	void SetWidth(float newWidth);

protected:
	virtual void AttachedToWindow(void);

	void DrawFrame(float width, float height, bool selected);

	BBitmap *image;
	float width;

	BView *view;
	BPicture *normalPic;
	BPicture *selectedPic;
	BPicture *normalDisPic;
	BPicture *selectedDisPic;
};

#endif
