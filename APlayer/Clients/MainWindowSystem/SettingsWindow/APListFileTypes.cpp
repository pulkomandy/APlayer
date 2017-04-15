/******************************************************************************/
/* APlayer Settings filetypes list class.                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"

// Santa headers
#include <santa/ColumnListView.h>
#include <santa/CLVColumnLabelView.h>

// Client headers
#include "APViewFileTypes.h"
#include "APListFileTypes.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListFileTypes::APListFileTypes(BRect frame, CLVContainerView** containerView, uint32 resizingMode, uint32 flags, list_view_type type, bool hierarchical, bool horizontal, bool vertical, bool scrollViewCorner, border_style border, const BFont* labelFont) :
	ColumnListView(frame, containerView, NULL, resizingMode, flags, type, hierarchical, horizontal, vertical, scrollViewCorner, border, labelFont)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListFileTypes::~APListFileTypes(void)
{
}



/******************************************************************************/
/* SelectionChanged() is called every a selection is made.                    */
/******************************************************************************/
void APListFileTypes::SelectionChanged(void)
{
	BMessage msg(SET_FIL_MSG_SELECTIONCHANGED);

	// Set an attribute telling if we have some selected or not
	msg.AddBool("selected", CurrentSelection() == -1 ? false : true);

	// Send the message
	Window()->PostMessage(&msg, Window());
	MakeFocus();
}
