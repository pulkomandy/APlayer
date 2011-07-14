/******************************************************************************/
/* APListItemFileTypes header file.                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APListItemFileTypes_h
#define __APListItemFileTypes_h

// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "APFileTypes.h"
#include "CLVListItem.h"


/******************************************************************************/
/* APListItemFileTypes class                                                  */
/******************************************************************************/
class APListItemFileTypes : public CLVListItem
{
public:
	APListItemFileTypes(float minHeight, APSystemFileType *info);
	virtual ~APListItemFileTypes(void);

	static int SortFunction(const CLVListItem *item1, const CLVListItem *item2, int32 keyColumn);

	APSystemFileType *fileType;

protected:
	void DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete);
	void Update(BView *owner, const BFont *font);

	BBitmap *icon;

	float textOffset;
	float height;
};

#endif
