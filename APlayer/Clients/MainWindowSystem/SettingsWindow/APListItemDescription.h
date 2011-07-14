/******************************************************************************/
/* APListItemDescription header file.                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APListItemDescription_h
#define __APListItemDescription_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// APlayerKit headers
#include "CLVListItem.h"


/******************************************************************************/
/* APListItemDescription class                                                */
/******************************************************************************/
class APListItemDescription : public CLVListItem
{
public:
	APListItemDescription(float minHeight, PString string);
	virtual ~APListItemDescription(void);

protected:
	virtual void DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete);
	virtual void Update(BView *owner, const BFont *font);

	PString columnText;
	float textOffset;
};

#endif
