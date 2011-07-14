/******************************************************************************/
/* APWindowMainList header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowMainList_h
#define __APWindowMainList_h

// PolyKit headers
#include "POS.h"

// Client headers
#include "APFileScanner.h"
#include "APWindowMainListItem.h"


/******************************************************************************/
/* Module list messages                                                       */
/******************************************************************************/
#define APLIST_INVOKE				'l_iv'
#define APLIST_DRAG					'l_dg'



/******************************************************************************/
/* APWindowMainList class                                                     */
/******************************************************************************/
class MainWindowSystem;
class APWindowMain;

class APWindowMainList : public BListView
{
public:
	APWindowMainList(MainWindowSystem *system, BRect frame, uint32 resizingMode, uint32 flags, BScrollView **containerView);
	virtual ~APWindowMainList(void);

	void RemoveAllItems(bool lock = true);
	void RemoveSelectedItems(void);

	void MoveSelectedItemsUp(void);
	void MoveSelectedItemsDown(void);
	void MoveSelectedItemsToTop(void);
	void MoveSelectedItemsToBottom(void);

	void EnableListNumber(bool enable);
	void SetPlayItem(APWindowMainListItem *item);

	void SortList(bool sortOrder);
	void Shuffle(void);

	virtual bool AddList(BList *list);
	virtual bool AddList(BList *list, int32 index);

protected:
	virtual void AttachedToWindow(void);
	virtual void FrameResized(float width, float height);
	virtual void Draw(BRect updateRect);

	virtual void SelectionChanged(void);
	virtual void MouseDown(BPoint point);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);

	virtual bool InitiateDrag(BPoint point, int32 index, bool wasSelected);
	virtual void MessageReceived(BMessage *msg);

	void UpdateScrollBar(void);
	void DrawLine(bool erase);

	static int SortFuncAZ(const void *arg1, const void *arg2);
	static int SortFuncZA(const void *arg1, const void *arg2);

	MainWindowSystem *windowSystem;

	BScrollView *scrollView;

	BMessage *dragMsg;
	BBitmap *dragBitmap;
	BPoint dragPoint;
	int32 insertIndex;
	uint32 oldKeyMod;

	APWindowMainListItem *playItem;
	APFileScanner *fileScanner;

	float prevWidth;
	bool showListNumber;
};

#endif
