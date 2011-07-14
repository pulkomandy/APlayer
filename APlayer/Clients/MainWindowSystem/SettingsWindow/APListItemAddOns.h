/******************************************************************************/
/* APListItemAddOns header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APListItemAddOns_h
#define __APListItemAddOns_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "CLVListItem.h"


/******************************************************************************/
/* APListItemAddOns class                                                     */
/******************************************************************************/
class APListItemAddOns : public CLVListItem
{
public:
	APListItemAddOns(PResource *res, float minHeight, APAddOnInformation *info, int32 num);
	virtual ~APListItemAddOns(void);

	APAddOnInformation *GetItemInformation(int32 &num) const;
	void SetInUseFlag(bool flag);

	static int SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn);

protected:
	virtual void DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete);
	virtual void Update(BView *owner, const BFont *font);

	APAddOnInformation *addOnInfo;
	int32 listNum;
	bool inUse;

	PString version;
	float textOffset;
	float height;

	static int32 imageCount;
	static BBitmap *imageCheck;
};

#endif
