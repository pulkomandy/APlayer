/******************************************************************************/
/* APlayer Main Window Module List class.                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PSystem.h"
#include "Colors.h"

// Client headers
#include "MainWindowSystem.h"
#include "APFileScanner.h"
#include "APWindowMain.h"
#include "APWindowMainList.h"
#include "APWindowMainListItem.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowMainList::APWindowMainList(MainWindowSystem *system, BRect frame, uint32 resizingMode, uint32 flags, BScrollView **containerView) : BListView(frame, NULL, B_MULTIPLE_SELECTION_LIST, resizingMode, flags)
{
	// Initialize member variables
	windowSystem = system;

	dragMsg        = NULL;
	dragBitmap     = NULL;

	insertIndex    = -2;
	oldKeyMod      = 0;

	playItem       = NULL;

	prevWidth      = 0.0f;
	showListNumber = false;

	// Create the scroll view
	scrollView = new BScrollView(NULL, this, B_FOLLOW_ALL, 0, false, true, B_PLAIN_BORDER);
	*containerView = scrollView;

	// Update the scroll bars
	UpdateScrollBar();

	// Start the file scanner
	fileScanner = new APFileScanner(windowSystem);
	if (fileScanner == NULL)
		throw PMemoryException();

	fileScanner->Start();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowMainList::~APWindowMainList(void)
{
	// Stop the file scanner and delete itself
	fileScanner->Stop();

	// Remove all items in the list
	RemoveAllItems(false);

	// Delete any drag'n'drop messages
	delete dragMsg;
}



/******************************************************************************/
/* RemoveAllItems() will remove all the items from the list.                  */
/*                                                                            */
/* Input:  "lock" is true if you want the list to be locked while the items   */
/*         are removed. This is the common use.                               */
/******************************************************************************/
void APWindowMainList::RemoveAllItems(bool lock)
{
	int32 i, count;
	APWindowMainListItem *item;

	// Set the sleep cursor
	windowSystem->mainWin->SetSleepCursor();

	// Lock the list
	if (lock)
		LockLooper();

	// Get the number of items in the list
	count = CountItems();

	// Remove all the items
	for (i = 0; i < count; i++)
	{
		// Get the item and delete it
		item = (APWindowMainListItem *)RemoveItem((int32)0);	// This is done because of a bug in BListView
//		item = (APWindowMainListItem *)ItemAt(i);

		delete item;
	}

	// Empty the list
//	MakeEmpty();

	// Update the scroll bars
	UpdateScrollBar();

	// Unlock again
	if (lock)
		UnlockLooper();

	// Set cursor back to normal
	windowSystem->mainWin->SetNormalCursor();
}



/******************************************************************************/
/* RemoveSelectedItems() will remove all the selected items from the list.    */
/******************************************************************************/
void APWindowMainList::RemoveSelectedItems(void)
{
	int32 index, selected;
	APWindowMainListItem *item;

	// Set the sleep cursor
	windowSystem->mainWin->SetSleepCursor();

	// Remove all the selected items
	index = 0;
	while ((selected = CurrentSelection(index)) >= 0)
	{
		// Get the item
		item = (APWindowMainListItem *)RemoveItem(selected);

		// If the item is the one that is playing, stop
		// playing it
		if (item->IsPlaying())
		{
			playItem = NULL;
			windowSystem->mainWin->StopAndFreeModule();
			windowSystem->FreeAllModules();
		}

		// Subtract the item time from the list
		windowSystem->mainWin->RemoveItemTimeFromList(item);

		// Delete the item
		delete item;
	}

	// Update the scroll bars
	UpdateScrollBar();

	// Set cursor back to normal
	windowSystem->mainWin->SetNormalCursor();
}



/******************************************************************************/
/* MoveSelectedItemsUp() will move all the selected items one index up.       */
/******************************************************************************/
void APWindowMainList::MoveSelectedItemsUp(void)
{
	int32 index, selected, prevSelected;
	bool prevMoved;

	// Move all the selected items
	prevMoved    = false;
	prevSelected = -1;
	index        = 0;

	while ((selected = CurrentSelection(index++)) >= 0)
	{
		// Move the item
		if ((selected > 0) && (((selected - 1) != prevSelected) || (prevMoved)))
		{
			MoveItem(selected, selected - 1);
			prevMoved = true;
		}
		else
			prevMoved = false;

		// Remember the selected index
		prevSelected = selected;
	}
}



/******************************************************************************/
/* MoveSelectedItemsDown() will move all the selected items one index down.   */
/******************************************************************************/
void APWindowMainList::MoveSelectedItemsDown(void)
{
	int32 index, count, listCount;
	int32 selected, prevSelected;
	bool prevMoved;
	PList<int32> selList;

	// Remember all the selected items
	index = 0;
	while ((selected = CurrentSelection(index++)) >= 0)
		selList.AddHead(selected);

	// Move all the selected items
	prevMoved    = false;
	prevSelected = -1;
	listCount    = CountItems();
	count        = selList.CountItems();

	for (index = 0; index < count; index++)
	{
		// Get the selected item index
		selected = selList.GetItem(index);

		// Move the item
		if (((selected + 1) < listCount) && (((selected + 1) != prevSelected) || (prevMoved)))
		{
			MoveItem(selected, selected + 1);
			prevMoved = true;
		}
		else
			prevMoved = false;

		// Remember the selected index
		prevSelected = selected;
	}
}



/******************************************************************************/
/* MoveSelectedItemsToTop() will move all the selected items to the top of    */
/*      the list.                                                             */
/******************************************************************************/
void APWindowMainList::MoveSelectedItemsToTop(void)
{
	int32 index, selected;

	// Move all the selected items
	index = 0;
	while ((selected = CurrentSelection(index)) >= 0)
	{
		// Move the item
		MoveItem(selected, index);
		index++;
	}
}



/******************************************************************************/
/* MoveSelectedItemsToBottom() will move all the selected items to the bottom */
/*      of the list.                                                          */
/******************************************************************************/
void APWindowMainList::MoveSelectedItemsToBottom(void)
{
	int32 index, count, listCount, selected;
	PList<int32> selList;

	// Remember all the selected items
	index = 0;
	while ((selected = CurrentSelection(index++)) >= 0)
		selList.AddHead(selected);

	// Move all the selected items
	listCount = CountItems();
	count     = selList.CountItems();

	for (index = 0; index < count; index++)
	{
		// Get the selected item index
		selected = selList.GetItem(index);

		// Move the item
		MoveItem(selected, listCount - index - 1);
	}
}



/******************************************************************************/
/* EnableListNumber() enable or disable the list number showing.              */
/*                                                                            */
/* Input:  "enable" true to enable it or false to disable it.                 */
/******************************************************************************/
void APWindowMainList::EnableListNumber(bool enable)
{
	// Lock the list
	LockLooper();

	// Change the list status and redraw it
	showListNumber = enable;
	Invalidate();

	// Unlock the list again
	UnlockLooper();
}



/******************************************************************************/
/* SetPlayItem() will set the item given to the current playing item. It will */
/*      deselect any previous playing item.                                   */
/*                                                                            */
/* Input:  "item" is a pointer to the item to set to playing or NULL if you   */
/*         only want to deselect any previous playing item.                   */
/******************************************************************************/
void APWindowMainList::SetPlayItem(APWindowMainListItem *item)
{
	// Lock the list
	LockLooper();

	// First deselect any previous playing item
	if (playItem != NULL)
	{
		// Change the item and refresh it
		playItem->SetPlayingFlag(false);
		InvalidateItem(IndexOf(playItem));
		playItem = NULL;
	}

	if (item != NULL)
	{
		// Change the item and refresh it
		item->SetPlayingFlag(true);
		InvalidateItem(IndexOf(item));

		// Remember the item
		playItem = item;
	}

	// Unlock the list again
	UnlockLooper();
}



/******************************************************************************/
/* SortList() will sort the entire list in the order given.                   */
/*                                                                            */
/* Input:  "sortOrder" is false for ascending and true for descending.        */
/******************************************************************************/
void APWindowMainList::SortList(bool sortOrder)
{
	// Set the sleep cursor
	windowSystem->mainWin->SetSleepCursor();

	if (!sortOrder)
		SortItems(SortFuncAZ);
	else
		SortItems(SortFuncZA);

	// Tell the file scanner to scan the modules
	fileScanner->ScanItems(0, CountItems());

	// Set cursor back to normal
	windowSystem->mainWin->SetNormalCursor();
}



/******************************************************************************/
/* Shuffle() will shuffle the entire list.                                    */
/******************************************************************************/
void APWindowMainList::Shuffle(void)
{
	int32 total;

	// Set the sleep cursor
	windowSystem->mainWin->SetSleepCursor();

	// Get the number of items
	total = CountItems();

	// Do we have enough items in the list?
	if (total > 1)
	{
		BList tempList, newList;
		int32 i, index;

		// Copy all the items from the list view to the
		// temporary list
		for (i = 0; i < total; i++)
			tempList.AddItem(ItemAt(i));

		// Well, if a module is playing, we want to
		// place that module in the top of the list
		if (playItem != NULL)
		{
			// Find the item index
			index = IndexOf(playItem);
			newList.AddItem(tempList.RemoveItem(index));

			// One item less to shuffle
			total--;
		}

		// Ok, now it's time to shuffle
		for (; total > 0; total--)
		{
			// Find a random item and add it to the new list +
			// remove it from the old one
			index = PSystem::Random(total - 1);
			newList.AddItem(tempList.RemoveItem(index));
		}

		// Copy the new list into the list view
		MakeEmpty();
		AddList(&newList);

		// Clean up
		tempList.MakeEmpty();
		newList.MakeEmpty();
	}

	// Set cursor back to normal
	windowSystem->mainWin->SetNormalCursor();
}



/******************************************************************************/
/* AddList() extents the add function with a file scan.                       */
/*                                                                            */
/* Input:  "list" is a pointer to the items to add.                           */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APWindowMainList::AddList(BList *list)
{
	// Just call the other version
	return (AddList(list, CountItems()));
}



/******************************************************************************/
/* AddList() extents the add function with a file scan.                       */
/*                                                                            */
/* Input:  "list" is a pointer to the items to add.                           */
/*         "index" is where to insert the items.                              */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APWindowMainList::AddList(BList *list, int32 index)
{
	// First call the base class
	if (!BListView::AddList(list, index))
		return (false);

	// Now tell the file scanner to scan the new added modules
	fileScanner->ScanItems(index, list->CountItems());

	return (true);
}



/******************************************************************************/
/* AttachedToWindow() is called when the list has been attached to a window.  */
/******************************************************************************/
void APWindowMainList::AttachedToWindow(void)
{
	// Set the invoke message and change the target to the list itself
	SetInvocationMessage(new BMessage(APLIST_INVOKE));
	SetTarget(this);
}



/******************************************************************************/
/* FrameResized() is called when the view change size.                        */
/*                                                                            */
/* Input:  "width" is the new width.                                          */
/*         "height" is the new height.                                        */
/******************************************************************************/
void APWindowMainList::FrameResized(float width, float height)
{
	BRect showFrame, itemFrame;
	int32 count, i;

	// Update the scroll bar
	UpdateScrollBar();

	// Get the frame that is shown at the moment
	showFrame = Bounds();

	// It's only neccesary to invalidate the items
	// if the width of the list has changed. This
	// check is made to minimize flicker
	if (prevWidth != showFrame.Width())
	{
		prevWidth = showFrame.Width();

		count = CountItems();
		if (count > 0)
		{
			// Find the first visible item
			for (i = 0; i < count; i++)
			{
				itemFrame = ItemFrame(i);
				if (itemFrame.bottom >= showFrame.top)
					break;
			}

			// Got it, now invalidate all the items shown
			for (; i < count; i++)
			{
				InvalidateItem(i);

				// Check to see if we have gone out of the view
				itemFrame = ItemFrame(i);
				if (itemFrame.top > showFrame.bottom)
					break;
			}
		}
	}

	// Call the base class
	BListView::FrameResized(width, height);
}



/******************************************************************************/
/* Draw() is called everytime some part of the list need to be drawn.         */
/*                                                                            */
/* Input:  "updateRect" is the part of the view that needs an update.         */
/******************************************************************************/
void APWindowMainList::Draw(BRect updateRect)
{
	int32 startIndex, endIndex;
	int32 i;

	// Find the start and end index of the items that needs to be redrawn
	startIndex = IndexOf(BPoint(0.0f, updateRect.top));
	endIndex   = IndexOf(BPoint(0.0f, updateRect.bottom));

	// Do only traverse the items if there is anyone to update
	if (startIndex >= 0)
	{
		if (endIndex < 0)
			endIndex = CountItems() - 1;

		// Now tell the items about the number they have
		if (showListNumber)
		{
			for (i = startIndex; i <= endIndex; i++)
				((APWindowMainListItem *)ItemAt(i))->SetItemNumber(i + 1);
		}
		else
		{
			for (i = startIndex; i <= endIndex; i++)
				((APWindowMainListItem *)ItemAt(i))->SetItemNumber(0);
		}
	}

	// Call the base class
	BListView::Draw(updateRect);
}



/******************************************************************************/
/* SelectionChanged() is called when a new selection is made in the list view.*/
/******************************************************************************/
void APWindowMainList::SelectionChanged(void)
{
	// Just post a message to the window telling it to update
	// whatever it need to update
	windowSystem->mainWin->PostMessage(AP_UPDATE_SELECTION);
}



/******************************************************************************/
/* MouseDown() is called when the user presses one of the mouse buttons.      */
/*                                                                            */
/* Input:  "point" is where in the view the mouse is.                         */
/******************************************************************************/
void APWindowMainList::MouseDown(BPoint point)
{
	BMessage *mouseMsg;
	int32 buttons;
	int32 index;

	// Get the current mouse down message
	mouseMsg = windowSystem->mainWin->CurrentMessage();
	ASSERT(mouseMsg != NULL);

	// Get the mouse buttons
	if (mouseMsg->FindInt32("buttons", &buttons) == B_OK)
	{
		// If right mouse button is pressed, load the module under the mouse
		if (buttons & B_SECONDARY_MOUSE_BUTTON)
		{
			// Get the index to the item the mouse is over
			index = IndexOf(point);
			if (index >= 0)
			{
				// Select the item and invoke it
				Select(index);
				Invoke();
			}
		}
	}

	// Call the base class
	BListView::MouseDown(point);
}



/******************************************************************************/
/* MouseMoved() is called when the mouse is moving inside the view. It's used */
/*         to show the user where he/she can drop a drag'n'drop object.       */
/*                                                                            */
/* Input:  "point" is the where the mouse is.                                 */
/*         "transit" is a code telling if the mouse is enter, inside or       */
/*         leaving the view.                                                  */
/*         "message" is a pointer to the message or NULL.                     */
/******************************************************************************/
void APWindowMainList::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	// Check to see if we have to tell the system to drag
	if (dragMsg != NULL)
	{
		// Tell the system to drag the item
		DragMessage(dragMsg, dragBitmap, B_OP_ALPHA, dragPoint);

		// It's safe to delete the message. The bitmap will
		// automatic be deleted by the system
		delete dragMsg;
		dragMsg    = NULL;
		dragBitmap = NULL;
	}

	// Check to see if it's a drag'n'drop message
	if (message != NULL)
	{
		bool validMsg = false;
		bool drawLine = false;

		// Now check to see if it's a message we understand
		switch (message->what)
		{
			case B_SIMPLE_DATA:
			{
				if (message->HasRef("refs"))
					validMsg = true;

				break;
			}

			case APLIST_DRAG:
			{
				if (message->HasInt32("index"))
					validMsg = true;

				break;
			}
		}

		if (validMsg)
		{
			// Check where the mouse has moved in the view
			switch (transit)
			{
				case B_ENTERED_VIEW:
				{
					insertIndex = -2;

					// Falling throu
				}

				case B_INSIDE_VIEW:
				{
					uint32 keyMod;
					int32 index;

					// Get the keyboard modifiers
					keyMod = modifiers();

					// If it's our own drag message, show the insert
					// cursor and draw the insert line
					if (message->what == APLIST_DRAG)
					{
						windowSystem->mainWin->SetInsertCursor();
						drawLine = true;
					}
					else
					{
						// Find out which cursor image to use
						if (keyMod & B_CONTROL_KEY)
						{
							if (keyMod & B_SHIFT_KEY)
								windowSystem->mainWin->SetAppendCursor();
							else
							{
								windowSystem->mainWin->SetInsertCursor();
								drawLine = true;
							}
						}
						else
							windowSystem->mainWin->SetNormalCursor();
					}

					// Check to see if it's the same index, and if so, don't draw anything
					index = IndexOf(point);
					if ((index != insertIndex) || (keyMod != oldKeyMod))
					{
						// Erase the old line
						DrawLine(true);

						// Remember the new values
						insertIndex = index;
						oldKeyMod   = keyMod;

						// Draw a new insert line
						if (drawLine)
							DrawLine(false);
					}
					break;
				}

				case B_EXITED_VIEW:
				{
					// Set the cursor to forbidden
					windowSystem->mainWin->SetForbiddenCursor();

					DrawLine(true);
					insertIndex = -2;
					break;
				}
			}
		}
	}
	else
	{
		// Set the mouse cursor to normal
		if (transit == B_ENTERED_VIEW)
			windowSystem->mainWin->SetNormalCursor();
	}

	// Call the base class
	BListView::MouseMoved(point, transit, message);
}



/******************************************************************************/
/* InitiateDrag() initialize drag'n'drop of an item in the list.              */
/*                                                                            */
/* Input:  "point" is the point where the mouse was pressed.                  */
/*         "index" is the index to the item which will be dragged.            */
/*         "wasSelected" is true if the item is selected, false if not.       */
/*                                                                            */
/* Output: True if it's okay to drag the item, else false.                    */
/******************************************************************************/
bool APWindowMainList::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	// Do only start drag'n'drop if the item already was selected
	if (wasSelected)
	{
		APWindowMainListItem *item;
		BRect dragRect, drawRect;
		BRegion clipRegion;
		BView *dragView;
		int32 selected, count, showCount, total, i;
		float width, height, y, offset;
		PString itemText;
		char *itemStr;

		// Delete any previous created message
		delete dragMsg;
		delete dragBitmap;
		dragBitmap = NULL;

		// Create the new message to send
		dragMsg = new BMessage(APLIST_DRAG);

		// Add all selected indexes to the message
		total = 0;
		while ((selected = CurrentSelection(total++)) >= 0)
			dragMsg->AddInt32("index", selected);

		count     = min(total - 1, 6);
		showCount = min(total - 1, 7);

		// Calculate the rect size
		item   = (APWindowMainListItem *)ItemAt(index);
		width  = Bounds().Width();
		height = ceil(item->Height() + 1.0f);
		y      = index * height;
		dragRect.Set(4.0f, y, 4.0f + width, y + height * showCount);

		// Find the drag point
		dragPoint = point - dragRect.LeftTop();

		// Create the bitmap to show
		dragBitmap = new BBitmap(dragRect, B_RGBA32, true);
		dragBitmap->Lock();

		drawRect = dragBitmap->Bounds();
		drawRect.OffsetTo(B_ORIGIN);
		dragView = new BView(drawRect, NULL, B_FOLLOW_NONE, 0);
		dragBitmap->AddChild(dragView);

		dragView->SetOrigin(0.0f, 0.0f);

		// Make sure we don't draw outside the view
		clipRegion.Set(drawRect);
		dragView->ConstrainClippingRegion(&clipRegion);

		// Transparent magic
		dragView->SetHighColor(0, 0, 0, 0);
		dragView->FillRect(drawRect);
		dragView->SetDrawingMode(B_OP_ALPHA);
		dragView->SetHighColor(0, 0, 0, 128);
		dragView->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);

		// Draw the item texts in the view
		y = 0.0f;
		for (i = 0; i < count; i++)
		{
			// Get the item index
			if (dragMsg->FindInt32("index", i, &selected) != B_OK)
				break;

			// Get the item
			item     = (APWindowMainListItem *)ItemAt(selected);
			itemText = item->GetText(&offset);

			// Draw the string
			dragView->MovePenTo(0.0f, y + offset);
			dragView->DrawString((itemStr = itemText.GetString()));
			itemText.FreeBuffer(itemStr);

			// Find the next y position
			y += item->Height();
		}

		// If there are selected more than 6 items, draw an ellipsis
		if (total > 7)
		{
			dragView->MovePenTo(0.0f, y + offset);
			dragView->DrawString(B_UTF8_ELLIPSIS);
		}

		// Make sure all drawing are done
		dragView->Sync();
		dragBitmap->Unlock();

		return (true);
	}

	return (false);
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowMainList::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// An item is invoked in the list
		////////////////////////////////////////////////////////////////////////
		case APLIST_INVOKE:
		{
			int32 index;

			// Get the first selected item
			index = CurrentSelection();
			if (index >= 0)
			{
				// Free the previous playing module
				windowSystem->mainWin->StopAndFreeModule();

				// Now load the selected one
				windowSystem->LoadAndPlayModule(index);
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Drag'n'drop handler for moving items
		////////////////////////////////////////////////////////////////////////
		case APLIST_DRAG:
		{
			int32 oldIndex, newIndex;
			int32 oldOffset, newOffset;
			int32 i;
			BPoint where;
			BListItem *item;

			if ((msg->FindInt32("index", &oldIndex)) == B_OK)
			{
				if (msg->WasDropped())
				{
					// Erase the line
					DrawLine(true);

					// Set the cursor back to normal
					windowSystem->mainWin->SetNormalCursor();

					// Calculate the insert index
					where = msg->DropPoint();
					ConvertFromScreen(&where);
					newIndex = IndexOf(where);

					if (newIndex < 0)
						newIndex = CountItems();

					// Move the items
					oldOffset = 0;
					newOffset = 0;

					for (i = 0; ; i++)
					{
						// Get the next index
						if (msg->FindInt32("index", i, &oldIndex) != B_OK)
							break;

						// Calculate the real index
						if (oldIndex < newIndex)
							oldIndex -= oldOffset;

						// Move the item
						item = RemoveItem(oldIndex);

						if (oldIndex < newIndex)
						{
							AddItem(item, newIndex - 1);
							oldOffset++;
						}
						else
						{
							AddItem(item, newIndex + newOffset);
							newOffset++;
						}
					}

					// Deselect all items
					DeselectAll();

					// Free any extra loaded modules
					windowSystem->FreeExtraModules();
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Drag'n'drop handler for Tracker files
		////////////////////////////////////////////////////////////////////////
		case B_SIMPLE_DATA:
		{
			entry_ref ref;

			// Check to see if it's a file which was dropped in the list
			if (((msg->FindRef("refs", &ref)) == B_OK) && (msg->WasDropped()))
			{
				BPoint where;
				int32 index, jumpIndex = -1;
				int32 i, count;
				uint32 keyMod;
				bool clearList;
				entry_ref entRef;
				BMessage newMsg;

				// Get the keyboard modifiers
				keyMod = modifiers();

				// Find out the action to perform
				if (keyMod & B_CONTROL_KEY)
				{
					clearList = false;

					if (keyMod & B_SHIFT_KEY)
					{
						index     = -1;		// Insert at the end of the list
						jumpIndex = CountItems();
					}
					else
					{
						// Find out where in the list to insert
						where = msg->DropPoint();
						ConvertFromScreen(&where);
						index = IndexOf(where);

						if (index == -1)
							jumpIndex = CountItems();
						else
							jumpIndex = index;
					}
				}
				else
				{
					clearList = true;
					index     = -1;
				}

				// Set the hand back to normal
				windowSystem->mainWin->SetNormalCursor();

				// Erase the line
				DrawLine(true);
				insertIndex = -2;

				// Should we remove all items from the list?
				if (clearList)
				{
					// Stop playing any modules
					windowSystem->mainWin->StopAndFreeModule();

					// Clear the list
					windowSystem->mainWin->EmptyList(false);
				}

				// Invert the files in the message, because
				// drag'n'drop give the files in reversed order!
				for (count = 0; (msg->FindRef("refs", count, &entRef) == B_OK); count++);

				for (i = count - 1; i >= 0; i--)
				{
					msg->FindRef("refs", i, &entRef);
					newMsg.AddRef("refs", &entRef);
				}

				// Insert the files in the list
				windowSystem->mainWin->AddFilesToList(&newMsg, false, index);

				// Start to load and play the first module if it's
				// a fresh list
				if (clearList)
					windowSystem->LoadAndPlayModule(0);
				else
				{
					windowSystem->FreeExtraModules();

					// Should we load the first added module?
					if (windowSystem->useSettings->GetStringEntryValue("Options", "AddJump").CompareNoCase("Yes") == 0)
					{
						// Stop playing any modules and load the first added one
						windowSystem->mainWin->StopAndFreeModule();
						windowSystem->LoadAndPlayModule(jumpIndex);
					}
				}
			}
			else
				BListView::MessageReceived(msg);

			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Default handler
		////////////////////////////////////////////////////////////////////////
		default:
		{
			BListView::MessageReceived(msg);
			break;
		}
	}
}



/******************************************************************************/
/* UpdateScrollBar() calculates the size and position of the scroll bar.      */
/******************************************************************************/
void APWindowMainList::UpdateScrollBar(void)
{
	BRect viewBounds;
	int32 numItems, i;
	float deltaY, heightProp;
	BScrollBar *vScrollBar;
	float dataHeight, pageHeight;

	// Figure out the height
	dataHeight = 0.0f;
	numItems   = CountItems();

	for (i = 0; i < numItems; i++)
		dataHeight += ceil(ItemAt(i)->Height() + 1.0f);

	// Figure out the bounds and scroll if necessary
	do
	{
		viewBounds = Bounds();

		// Figure out the height of the page rectangle
		pageHeight = dataHeight;

		// If view runs past the end, make more visible at the beginning
		deltaY = 0.0f;
		if ((viewBounds.bottom > dataHeight) && (viewBounds.top > 0.0f))
		{
			deltaY = viewBounds.bottom - dataHeight;
			if (deltaY > viewBounds.top)
				deltaY = viewBounds.top;
		}

		if (deltaY != 0.0f)
		{
			ScrollTo(BPoint(0.0f, viewBounds.top - deltaY));
			viewBounds = Bounds();
		}

		if ((viewBounds.bottom - viewBounds.top) > dataHeight)
			pageHeight = viewBounds.bottom;
	}
	while (deltaY != 0.0f);

	// Figure out the ratio of the bounds rectangle height to the page rectangle height
	heightProp = (viewBounds.bottom - viewBounds.top) / pageHeight;

	vScrollBar = scrollView->ScrollBar(B_VERTICAL);

	// Set the scroll bar ranges and proportions.
	// If the whole document is visible, inactivate the slider
	if (vScrollBar)
	{
		if ((heightProp >= 1.0f) && (viewBounds.top == 0.0f))
			vScrollBar->SetRange(0.0f, 0.0f);
		else
			vScrollBar->SetRange(0.0f, pageHeight - (viewBounds.bottom - viewBounds.top));

		vScrollBar->SetProportion(heightProp);
	}
}



/******************************************************************************/
/* DrawLine() will draw a line above the insert item.                         */
/*                                                                            */
/* Input:  "erase" is true to erase the line, false to draw it.               */
/******************************************************************************/
void APWindowMainList::DrawLine(bool erase)
{
	BRect rect;
	rgb_color color;
	float height, pos;
	int32 count, index;

	// Find the position where to draw the line
	rect  = Bounds();
	count = CountItems();

	if ((count == 0) || (insertIndex == 0))
		pos = 0.0f;
	else
	{
		height = ceil(ItemAt(0)->Height() + 1.0f);
		pos    = insertIndex * height - 1.0f;

		// Do we point at any item?
		if (insertIndex == -1)
		{
			if ((count * height) <= rect.bottom)
				pos = count * height - 1.0f;
		}
	}

	// Find the color to draw with
	if (erase)
	{
		switch (insertIndex)
		{
			case -2:
			case -1:
			{
				index = count - 1;
				break;
			}

			case 0:
			{
				index = 0;
				break;
			}

			default:
			{
				index = insertIndex - 1;
				break;
			}
		}

		// Erase the line
		color = index >= 0 ? ItemAt(index)->IsSelected() ? BeListSelectGrey : ViewColor() : ViewColor();
	}
	else
		color = LightBlue;

	// Draw the line
	SetHighColor(color);
	StrokeLine(BPoint(rect.left, pos), BPoint(rect.right, pos));
}



/******************************************************************************/
/* SortFuncAZ() is called for each item to check when using ascending         */
/*      sorting.                                                              */
/*                                                                            */
/* Input:  "arg1" is a pointer to the first item.                             */
/*         "arg2" is a pointer to the second item.                            */
/*                                                                            */
/* Output: < 0 if arg1 < arg2, = 0 if arg1 == arg2, > 0 if arg1 > arg2.       */
/******************************************************************************/
int APWindowMainList::SortFuncAZ(const void *arg1, const void *arg2)
{
	PString label1, label2;
	register APWindowMainListItem *item1 = *(APWindowMainListItem **)arg1;
	register APWindowMainListItem *item2 = *(APWindowMainListItem **)arg2;

	// Get the item labels
	label1 = item1->GetText();
	label2 = item2->GetText();

	// Compare the strings
	if (label1 < label2)
		return (-1);

	if (label1 > label2)
		return (1);

	return (0);
}



/******************************************************************************/
/* SortFuncZA() is called for each item to check when using descending        */
/*      sorting.                                                              */
/*                                                                            */
/* Input:  "arg1" is a pointer to the first item.                             */
/*         "arg2" is a pointer to the second item.                            */
/*                                                                            */
/* Output: < 0 if arg1 > arg2, = 0 if arg1 == arg2, > 0 if arg1 < arg2.       */
/******************************************************************************/
int APWindowMainList::SortFuncZA(const void *arg1, const void *arg2)
{
	PString label1, label2;
	register APWindowMainListItem *item1 = *(APWindowMainListItem **)arg1;
	register APWindowMainListItem *item2 = *(APWindowMainListItem **)arg2;

	// Get the item labels
	label1 = item1->GetText();
	label2 = item2->GetText();

	// Compare the strings
	if (label1 > label2)
		return (-1);

	if (label1 < label2)
		return (1);

	return (0);
}
