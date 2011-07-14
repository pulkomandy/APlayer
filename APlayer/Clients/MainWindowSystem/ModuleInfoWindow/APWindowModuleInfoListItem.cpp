/******************************************************************************/
/* APlayer Module Info list item class.                                       */
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
#include "PDirectory.h"
#include "Colors.h"

// APlayerKit headers
#include "CLVListItem.h"
#include "Layout.h"

// Client headers
#include "APWindowModuleInfoList.h"
#include "APWindowModuleInfoListItem.h"


/******************************************************************************/
/* APWindowModuleInfoListItem class                                           */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowModuleInfoListItem::APWindowModuleInfoListItem(float minHeight, PString description, PString value) : CLVListItem(0, false, false, minHeight)
{
	// Set the column strings
	columnText[0] = description;
	columnText[1] = value;

	// Set the default text color
	columnColor[0] = Black;
	columnColor[1] = Black;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowModuleInfoListItem::~APWindowModuleInfoListItem(void)
{
}



/******************************************************************************/
/* ChangeColumn() will change a column string.                                */
/*                                                                            */
/* Input :  "string" is the string to want in the column.                     */
/*          "columnIndex" is the column to change.                            */
/*                                                                            */
/* Output:  True if the column was changed and false if not.                  */
/******************************************************************************/
bool APWindowModuleInfoListItem::ChangeColumn(PString string, int16 columnIndex)
{
	// Check for out of range
	ASSERT((columnIndex >= 0) && (columnIndex <= 1));

	// Is the new value the same as the previous?
	if (string == columnText[columnIndex])
		return (false);

	// Change the column
	columnText[columnIndex] = string;

	return (true);
}



/******************************************************************************/
/* MouseMoved() is called when the mouse is moving inside the item.           */
/*                                                                            */
/* Input:  "list" is a pointer to the list that the item is stored in.        */
/*         "index" is the index of the item.                                  */
/*         "point" is the where the mouse is.                                 */
/******************************************************************************/
void APWindowModuleInfoListItem::MouseMoved(APWindowModuleInfoList *list, int32 index, BPoint point)
{
}



/******************************************************************************/
/* MouseOut() is called when the mouse is moving outside the item.            */
/******************************************************************************/
void APWindowModuleInfoListItem::MouseOut(void)
{
}



/******************************************************************************/
/* DrawItemColumn() is called every time a column needs to be drawn.          */
/*                                                                            */
/* Input :  "owner" is the view to draw into.                                 */
/*          "itemColumnRect" is the size of the column.                       */
/*          "columnIndex" is the column to draw.                              */
/*          "complete" indicates if it should draw every pixel.               */
/******************************************************************************/
void APWindowModuleInfoListItem::DrawItemColumn(BView *owner, BRect itemColumnRect, int32 columnIndex, bool complete)
{
	BRegion region;
	char *strPtr;

	// Initialize the pen
	owner->SetLowColor(White);
	owner->SetDrawingMode(B_OP_COPY);

	if (complete)
	{
		// Draw the background color
		owner->SetHighColor(White);
		owner->FillRect(itemColumnRect);
	}

	// Draw the item
	region.Include(itemColumnRect);
	owner->ConstrainClippingRegion(&region);

	if (columnIndex >= 0)
	{
		owner->SetHighColor(columnColor[columnIndex]);
		owner->DrawString((strPtr = columnText[columnIndex].GetString()), BPoint(itemColumnRect.left + 2.0f, itemColumnRect.top + textOffset));
		columnText[columnIndex].FreeBuffer(strPtr);
	}

	owner->ConstrainClippingRegion(NULL);
}



/******************************************************************************/
/* Update() is called when the width or height has changed.                   */
/*                                                                            */
/* Input :  "owner" is the owner view.                                        */
/*          "font" is the font used in the column list view.                  */
/******************************************************************************/
void APWindowModuleInfoListItem::Update(BView *owner, const BFont *font)
{
	font_height fontAttr;
	float fontHeight;

	// Call the base class
	CLVListItem::Update(owner, font);

	font->GetHeight(&fontAttr);
	fontHeight = ceil(fontAttr.ascent) + ceil(fontAttr.descent);
	textOffset = ceil(fontAttr.ascent) + (Height() - fontHeight) / 2.0f;
}





/******************************************************************************/
/* APWindowModuleInfoListPathItem class                                       */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowModuleInfoListPathItem::APWindowModuleInfoListPathItem(float minHeight, PString description, PString value) : APWindowModuleInfoListItem(minHeight, description, value)
{
	// Does the string contains a valid path?
	if (PDirectory::DirectoryExists(PDirectory::GetDirectoryPart(value)))
	{
		// Change the color of the value
		columnColor[1] = Blue;
		validPath      = true;
	}
	else
	{
		// Set the value color back to normal
		columnColor[1] = Black;
		validPath      = false;
	}

	// Initialize member variables
	pathWindow = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowModuleInfoListPathItem::~APWindowModuleInfoListPathItem(void)
{
	// Close the path window
	if (pathWindow != NULL)
	{
		pathWindow->Lock();
		pathWindow->QuitRequested();
		pathWindow->Quit();
	}
}



/******************************************************************************/
/* CloseWindow() will close the path window if open.                          */
/******************************************************************************/
void APWindowModuleInfoListPathItem::CloseWindow(void)
{
	if (pathWindow != NULL)
	{
		// Tell the window to close
		pathWindow->PostMessage(B_QUIT_REQUESTED);
		pathWindow = NULL;
	}
}



/******************************************************************************/
/* MouseMoved() is called when the mouse is moving inside the item.           */
/*                                                                            */
/* Input:  "list" is a pointer to the list that the item is stored in.        */
/*         "index" is the index of the item.                                  */
/*         "point" is the where the mouse is.                                 */
/******************************************************************************/
void APWindowModuleInfoListPathItem::MouseMoved(APWindowModuleInfoList *list, int32 index, BPoint point)
{
	BRect listRect, winRect;
	float colWidth;
	char *strPtr;

	// Do only open the window if we have a valid path
	if (validPath)
	{
		// Find the size and position of the window
		listRect      = list->Frame();
		colWidth      = list->ColumnAt(0)->Width();

		winRect       = list->ItemFrame(index);
		winRect.left += colWidth;
		winRect.right = winRect.left + list->StringWidth((strPtr = columnText[1].GetString())) + HSPACE;
		columnText[1].FreeBuffer(strPtr);

		// Check to see if the mouse is over the value part of the item
		if ((point.x > winRect.left) && (point.x < winRect.right))
		{
			// The mouse is now over the value part of the file name item.
			// Open a window with the full path
			if (pathWindow == NULL)
			{
				BScreen scr(B_MAIN_SCREEN_ID);
				BRect scrRect;
				float width;

				// Convert the window rect to screen coordinates
				list->ConvertToScreen(&winRect);

				// Check for out of range
				scrRect = scr.Frame();
				width   = winRect.Width();

				if ((winRect.left + width) > scrRect.right)
					winRect.OffsetTo(scrRect.right - width, winRect.top);

				if (winRect.left < scrRect.left)
					winRect.OffsetTo(scrRect.left, winRect.top);

				// Create the window
				pathWindow = new BWindow(winRect, "MainWindowSystem: File path window", B_BORDERED_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_AVOID_FOCUS | B_NO_WORKSPACE_ACTIVATION | B_WILL_ACCEPT_FIRST_CLICK | B_ASYNCHRONOUS_CONTROLS);
				if (pathWindow != NULL)
				{
					// Create the view that show the path
					APWindowModuleInfoListPathView *view = new APWindowModuleInfoListPathView(pathWindow->Bounds(), columnText[1], this);
					if (view == NULL)
					{
						delete pathWindow;
						pathWindow = NULL;
					}
					else
					{
						// Add the view to the window
						pathWindow->AddChild(view);

						// Open the window and show it to the user
						pathWindow->Sync();
						pathWindow->Show();
					}
				}
			}
		}
	}
}



/******************************************************************************/
/* MouseOut() is called when the mouse is moving outside the item.            */
/******************************************************************************/
void APWindowModuleInfoListPathItem::MouseOut(void)
{
	// Just close the window
	CloseWindow();
}





/******************************************************************************/
/* APWindowModuleInfoListPathView class                                       */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowModuleInfoListPathView::APWindowModuleInfoListPathView(BRect frame, PString path, APWindowModuleInfoListPathItem *item) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// Initialize member variables
	text      = path;
	listItem  = item;
	mouseDown = false;
	inView    = false;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowModuleInfoListPathView::~APWindowModuleInfoListPathView(void)
{
}



/******************************************************************************/
/* Draw() draws the path.                                                     */
/*                                                                            */
/* Input:  "updateRect" is the rect which needs to be updated.                */
/******************************************************************************/
void APWindowModuleInfoListPathView::Draw(BRect updateRect)
{
	BRect rect;
	font_height fh;
	char *textPtr;

	// Find the size of the view
	rect = Bounds();

	// Fill in the background
	SetHighColor(ViewColor());
	FillRect(rect);

	// Draw the text
	GetFontHeight(&fh);
	SetHighColor(Blue);
	DrawString((textPtr = text.GetString()), BPoint(3.0f, Bounds().Height() - fh.descent));
	text.FreeBuffer(textPtr);
}



/******************************************************************************/
/* MouseMoved() is called when the mouse is moving inside the view. It's used */
/*         to detect and see if the window needs to be closed.                */
/*                                                                            */
/* Input:  "point" is the where the mouse is.                                 */
/*         "transit" is a code telling if the mouse is enter, inside or       */
/*         leaving the view.                                                  */
/*         "message" is a pointer to the message or NULL.                     */
/******************************************************************************/
void APWindowModuleInfoListPathView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	// Set the "inside view" flag
	inView = ((transit == B_ENTERED_VIEW) || (transit == B_INSIDE_VIEW)) ? true : false;

	// Is the mouse button pressed?
	if (mouseDown)
	{
		// Yes, should we select or deselect the view
		if ((transit == B_ENTERED_VIEW) || (transit == B_EXITED_VIEW))
			InvertRect(Bounds());
	}
	else
	{
		// Should we close the window?
		if (transit == B_EXITED_VIEW)
			listItem->CloseWindow();
	}

	// Call the base class
	BView::MouseMoved(point, transit, message);
}



/******************************************************************************/
/* MouseDown() is called when the mouse is pressed in the view. It's used to  */
/*         "select" the view.                                                 */
/*                                                                            */
/* Input:  "point" is the where the mouse is.                                 */
/******************************************************************************/
void APWindowModuleInfoListPathView::MouseDown(BPoint point)
{
	BMessage *mouseMsg;
	int32 buttons;

	// Get the current mouse down message
	mouseMsg = Window()->CurrentMessage();
	ASSERT(mouseMsg != NULL);

	// Get the mouse buttons
	if (mouseMsg->FindInt32("buttons", &buttons) == B_OK)
	{
		// If left mouse button is pressed, select the view
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
		{
			// Make the view as selected and make the
			// view to receive mouse events when the
			// mouse is out of the view
			InvertRect(Bounds());
			SetMouseEventMask(B_POINTER_EVENTS);

			mouseDown = true;
			inView    = true;
		}
	}

	// Call the base class
	BView::MouseDown(point);
}



/******************************************************************************/
/* MouseUp() is called when the mouse is released in the view. It's used to   */
/*         "deselect" the view.                                               */
/*                                                                            */
/* Input:  "point" is the where the mouse is.                                 */
/******************************************************************************/
void APWindowModuleInfoListPathView::MouseUp(BPoint point)
{
	if (inView)
	{
		BMessenger messenger("application/x-vnd.Be-TRAK");
		BMessage message;
		BEntry entry;
		entry_ref entryRef;
		node_ref nodeRef;
		char *textPtr;

		// Find the entry to the file
		if (entry.SetTo((textPtr = text.GetString())) == B_OK)
		{
			// Get the node
			entry.GetNodeRef(&nodeRef);

			// Find the entry to the directory
			if (entry.SetTo(PDirectory::GetDirectoryPart(text).GetString(), false) == B_OK)
			{
				entry.GetRef(&entryRef);

				// Tell the Tracker to open the window
				message.what = B_REFS_RECEIVED;
				message.AddRef("refs", &entryRef);
				message.AddData("nodeRefToSelect", B_RAW_TYPE, &nodeRef, sizeof(node_ref));
				messenger.SendMessage(&message);
			}
		}

		text.FreeBuffer(textPtr);
	}

	// Close the window
	listItem->CloseWindow();

	// Call the base class
	BView::MouseUp(point);
}
