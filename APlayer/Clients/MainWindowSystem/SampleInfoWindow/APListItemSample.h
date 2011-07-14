/******************************************************************************/
/* APListItemSample header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APListItemSample_h
#define __APListItemSample_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// APlayer headers
#include "APAddOns.h"
#include "CLVListItem.h"


/******************************************************************************/
/* APListItemSample class                                                     */
/******************************************************************************/
class APListItemSample : public CLVListItem
{
public:
	APListItemSample(PResource *resource, float minHeight, APSampleInfo *info, uint32 num);
	virtual ~APListItemSample(void);

	static int SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn);

	const APSampleInfo *GetSampleInfo(void) const;

protected:
	void DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete);
	void Update(BView *owner, const BFont *font);

	PString GetTypeString(void);

	PResource *res;

	float textOffset;

	uint32 sampleNum;
	APSampleInfo sampleInfo;

	static int32 imageCount;
	static BBitmap *imageLoop;
	static BBitmap *imagePingPong;
};

#endif
