/******************************************************************************/
/* APDiskButton header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APDiskButton_h
#define __APDiskButton_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* APDiskButton class                                                         */
/******************************************************************************/
class APDiskButton : public BPictureButton
{
public:
	APDiskButton(PResource *res, int32 normalIconID, int32 pressedIconID, BRect frame, BMessage *message, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
	virtual ~APDiskButton(void);

protected:
	virtual void AttachedToWindow(void);

	void DrawFrame(float width, float height, bool selected);

	BBitmap *imageNormal;
	BBitmap *imagePressed;

	BView *view;
	BPicture *normalPic;
	BPicture *selectedPic;
	BPicture *normalDisPic;
	BPicture *selectedDisPic;
};

#endif
