/******************************************************************************/
/* APListDescription header file.                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APListDescription_h
#define __APListDescription_h

// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "ColumnListView.h"


/******************************************************************************/
/* APListDescription class                                                    */
/******************************************************************************/
class APListDescription : public ColumnListView
{
public:
	APListDescription(BRect frame, CLVContainerView** containerView, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
		list_view_type type = B_SINGLE_SELECTION_LIST, bool hierarchical = false, bool horizontal = true,
		bool vertical = true, bool scrollViewCorner = true, border_style border = B_NO_BORDER, const BFont* labelFont = be_plain_font);
	virtual ~APListDescription(void);

protected:
	virtual void KeyDown(const char *bytes, int32 numBytes);

	float fontHeight;
};

#endif
