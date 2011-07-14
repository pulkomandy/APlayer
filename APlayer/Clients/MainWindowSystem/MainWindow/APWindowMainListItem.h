/******************************************************************************/
/* APWindowMainListItem header file.                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowMainListItem_h
#define __APWindowMainListItem_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PTime.h"


/******************************************************************************/
/* APWindowMainListItem class                                                 */
/******************************************************************************/
class APWindowMainListItem : public BListItem
{
public:
	enum ItemType { apNormal };

	APWindowMainListItem(PString text, PString fileName);
	virtual ~APWindowMainListItem(void);

	void SetItemNumber(int32 number);

//XX	void ChangeToArchive(PString archiveName, ItemType type);

	void SetPlayingFlag(bool flag);
	bool IsPlaying(void) const;

	void SetTime(PTimeSpan time);
	PTimeSpan GetTime(void) const;
	bool HaveTime(void) const;

	PString GetText(float *offset = NULL) const;
	ItemType GetItemType(void) const;
	PString GetFileName(void) const;
//XX	PString GetArchiveName(void);

protected:
	virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
	virtual void Update(BView *owner, const BFont *font);

	int32 itemNum;
	ItemType itemType;
	PString itemText;
	PString itemFileName;
	PString itemArchiveName;
	PTimeSpan itemTime;
	bool timeSet;
	bool playing;

	float textOffset;
};

#endif
