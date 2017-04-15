/******************************************************************************/
/* APWindowModuleInfoListItem header file.                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowModuleInfoListItem_h
#define __APWindowModuleInfoListItem_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// Santa headers
#include <santa/CLVListItem.h>

// Client headers
#include "APWindowModuleInfoList.h"


/******************************************************************************/
/* APWindowModuleInfoListItem class                                           */
/******************************************************************************/
class APWindowModuleInfoListItem : public CLVListItem
{
public:
	APWindowModuleInfoListItem(float minHeight, PString description, PString value);
	virtual ~APWindowModuleInfoListItem(void);

	bool ChangeColumn(PString string, int16 columnIndex);

	virtual void MouseMoved(APWindowModuleInfoList *list, int32 index, BPoint point);
	virtual void MouseOut(void);

protected:
	virtual void DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete);
	virtual void Update(BView *owner, const BFont *font);

	PString columnText[2];
	rgb_color columnColor[2];
	float textOffset;
};



/******************************************************************************/
/* APWindowModuleInfoListPathItem class                                       */
/******************************************************************************/
class APWindowModuleInfoListPathItem : public APWindowModuleInfoListItem
{
public:
	APWindowModuleInfoListPathItem(float minHeight, PString description, PString value);
	virtual ~APWindowModuleInfoListPathItem(void);

	void CloseWindow(void);

protected:
	virtual void MouseMoved(APWindowModuleInfoList *list, int32 index, BPoint point);
	virtual void MouseOut(void);

	bool validPath;
	BWindow *pathWindow;
};



/******************************************************************************/
/* APWindowModuleInfoListPathView class                                       */
/******************************************************************************/
class APWindowModuleInfoListPathView : public BView
{
public:
	APWindowModuleInfoListPathView(BRect frame, PString path, APWindowModuleInfoListPathItem *item);
	virtual ~APWindowModuleInfoListPathView(void);

protected:
	virtual void Draw(BRect updateRect);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);

	PString text;
	APWindowModuleInfoListPathItem *listItem;
	bool mouseDown;
	bool inView;
};

#endif
