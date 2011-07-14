/******************************************************************************/
/* APListFileTypes header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APListFileTypes_h
#define __APListFileTypes_h

// Polykit headers
#include "POS.h"

// APlayerKit headers
#include "ColumnListView.h"


/******************************************************************************/
/* APListFileTypes class                                                      */
/******************************************************************************/
class APListFileTypes : public ColumnListView
{
public:
	APListFileTypes(BRect frame, CLVContainerView** containerView, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
		list_view_type type = B_SINGLE_SELECTION_LIST, bool hierarchical = false, bool horizontal = true,
		bool vertical = true, bool scrollViewCorner = true, border_style border = B_NO_BORDER, const BFont* labelFont = be_plain_font);
	virtual ~APListFileTypes(void);

protected:
	virtual void SelectionChanged(void);
};

#endif
