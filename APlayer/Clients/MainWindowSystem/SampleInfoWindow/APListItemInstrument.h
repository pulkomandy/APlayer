/******************************************************************************/
/* APListItemInstrument header file.                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APListItemInstrument_h
#define __APListItemInstrument_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// APlayerKit headers
#include "APAddOns.h"
#include "CLVListItem.h"


/******************************************************************************/
/* APListItemInstrument class                                                 */
/******************************************************************************/
class APListItemInstrument : public CLVListItem
{
public:
	APListItemInstrument(float minHeight, APInstInfo *info, uint32 num);
	virtual ~APListItemInstrument(void);

	static int SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn);

protected:
	virtual void DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete);
	virtual void Update(BView *owner, const BFont *font);

	float textOffset;

	uint32 instNum;
	PString instName;
	uint32 usedSamples;
};

#endif
