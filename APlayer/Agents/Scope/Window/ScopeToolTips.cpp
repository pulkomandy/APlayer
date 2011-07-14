/******************************************************************************/
/* ScopeToolTips implementation file.                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "PList.h"
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Agent headers
#include "ScopeToolTips.h"


/******************************************************************************/
/* ScopeToolView class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
ScopeToolView::ScopeToolView(void) : BView(BRect(0.0f, 0.0f, 10.0f, 10.0f), NULL, B_FOLLOW_ALL, B_WILL_DRAW)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ScopeToolView::~ScopeToolView(void)
{
}



/******************************************************************************/
/* SetText() changes the tip text.                                            */
/*                                                                            */
/* Input:  "text" is the new text to be shown.                                */
/******************************************************************************/
void ScopeToolView::SetText(PString text)
{
	tipText = text;
}



/******************************************************************************/
/* Draw() draws the tool tip.                                                 */
/*                                                                            */
/* Input:  "updateRect" is the rect which needs to be updated.                */
/******************************************************************************/
void ScopeToolView::Draw(BRect updateRect)
{
	BRect rect;
	font_height fh;
	float pos, height;
	int32 index, oldIndex;
	PString drawStr;
	char *drawPtr;

	// Find the size of the view
	rect = Bounds();

	// Fill in the background
	SetHighColor(LightYellow);
	FillRect(rect);

	// Draw the box
	SetHighColor(Black);
	StrokeRect(rect);

	// Draw the text in the box
	GetFontHeight(&fh);
	pos = ceil(fh.ascent + fh.descent);
	height = pos;

	oldIndex = 0;

	while ((index = tipText.Find('\n', oldIndex)) != -1)
	{
		drawStr = tipText.Mid(oldIndex, index - oldIndex);
		DrawString((drawPtr = drawStr.GetString()), BPoint(HSPACE / 2.0f + 1.0f, pos));
		drawStr.FreeBuffer(drawPtr);
		oldIndex = index + 1;
		pos += height;
	}

	drawStr = tipText.Mid(oldIndex);
	DrawString((drawPtr = drawStr.GetString()), BPoint(HSPACE / 2.0f + 1.0f, pos));
	drawStr.FreeBuffer(drawPtr);
}





/******************************************************************************/
/* ScopeToolTips class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ScopeToolTips::ScopeToolTips(BRect frame) : BBox(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER)
{
	// Initialize member variables
	timeCounter = 0;
	showing     = false;
	notAgain    = false;

	// Create tool tip window
	tipWin = new BWindow(BRect(0.0f, 0.0f, 10.0f, 10.0f), "Scope: Tooltip window", B_NO_BORDER_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_AVOID_FRONT | B_AVOID_FOCUS | B_NO_WORKSPACE_ACTIVATION | B_ASYNCHRONOUS_CONTROLS);
	if (tipWin == NULL)
		throw PMemoryException();

	// Create the view used to draw the tip
	tipView = new ScopeToolView();
	if (tipView == NULL)
	{
		delete tipWin;
		throw PMemoryException();
	}

	// Add the view and start the window message thread (does not show the window)
	tipWin->AddChild(tipView);
	tipWin->Run();

	// Initialize tool tip thread
	threadEvent = new PEvent(true, false);
	if (threadEvent == NULL)
	{
		delete tipWin;
		throw PMemoryException();
	}

	tipThread.SetHookFunc(TipThread, this);
	tipThread.SetName("Scope: Tooltip handling");
	tipThread.StartThread();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ScopeToolTips::~ScopeToolTips(void)
{
	int32 count, i;
	ScopeViewData *data;

	// Exit the thread
	threadEvent->SetEvent();
	tipThread.WaitOnThread();

	// Quit the window
	tipWin->Lock();
	tipWin->Quit();

	// Lock the object
	listLock.WaitToWrite();

	// Clean up the list
	count = tipList.CountItems();

	for (i = 0; i < count; i++)
	{
		data = tipList.GetAndRemoveItem(0);
		delete data;
	}

	// Unlock again
	listLock.DoneWriting();
}



/******************************************************************************/
/* AddToolTip() adds a view with and the tip to the list of tips.             */
/*                                                                            */
/* Input:  "view" is a pointer to the view you want a tip to.                 */
/*         "text" is a string with the tip you want to be shown.              */
/*                                                                            */
/* Output: A pointer to the item added. This can be used to remove it later.  */
/******************************************************************************/
void *ScopeToolTips::AddToolTip(BView *view, PString text)
{
	ScopeViewData *data;

	// Start to allocate and fill in the structure
	data = new ScopeViewData;
	if (data == NULL)
		throw PMemoryException();

	data->view = view;
	data->text = text;

	// Lock the object
	listLock.WaitToWrite();

	// Add the new tool tip to the list
	tipList.AddTail(data);

	// Unlock again
	listLock.DoneWriting();

	return (data);
}



/******************************************************************************/
/* AddToolTip() adds a view with and the tip to the list of tips.             */
/*                                                                            */
/* Input:  "view" is a pointer to the view you want a tip to.                 */
/*         "resource" is the resources to take the string from.               */
/*         "strNum" is the string number in the string resource.              */
/*                                                                            */
/* Output: A pointer to the item added. This can be used to remove it later.  */
/******************************************************************************/
void *ScopeToolTips::AddToolTip(BView *view, PResource *resource, int32 strNum)
{
	PString text;

	text.LoadString(resource, strNum);
	return (AddToolTip(view, text));
}



/******************************************************************************/
/* TipThread() is the timer function.                                         */
/*                                                                            */
/* Input:  "userData" is a pointer to the current object.                     */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 ScopeToolTips::TipThread(void *userData)
{
	ScopeToolTips *object = (ScopeToolTips *)userData;
	BWindow *win;

	while (object->threadEvent->Lock(200) == pSyncTimeout)
	{
		win = object->Window();
		if (win != NULL)
		{
			if (win->LockWithTimeout(0) == B_NO_ERROR)
			{
				object->ShowTip();
				win->Unlock();
			}
		}
	}

	return (0);
}



/******************************************************************************/
/* ShowTip() is the main tool tip function. It will count the timer counter   */
/*         up and when it reach a certain value, it will show the tool tip.   */
/******************************************************************************/
void ScopeToolTips::ShowTip(void)
{
	BPoint position;
	uint32 buttons;

	// Count the timer
	timeCounter++;

	// It is time to show the tool tip?
	if (timeCounter == 4)
	{
		if ((!showing) && (!notAgain) && (Window()->IsActive()))
		{
			BScreen scr(B_MAIN_SCREEN_ID);
			BRect scrRect;
			PString tempStr;
			char *tempPtr;
			ScopeViewData *viewData = NULL;
			font_height fh;
			float width, height, lineHeight;
			int32 i, count;
			int32 index, oldIndex;
			bool found = false;

			// Get the mouse position
			GetMouse(&position, &buttons);

			// Find the right tool tip view
			listLock.WaitToRead();

			count = tipList.CountItems();
			for (i = 0; i < count; i++)
			{
				viewData = tipList.GetItem(i);
				if (viewData->view->Frame().Contains(position))
				{
					found = true;
					break;
				}
			}

			listLock.DoneReading();

			// Convert the mouse position to screen coordinates
			ConvertToScreen(&position);

			// Remember the position
			oldPos = position;

			if (found)
			{
				// We are showing the tool tip
				showing = true;

				// Calculate the size of the tip
				viewData->view->GetFontHeight(&fh);
				lineHeight = ceil(fh.ascent + fh.descent);

				height   = 0.0f;
				width    = 0.0f;
				oldIndex = 0;

				while ((index = viewData->text.Find('\n', oldIndex)) != -1)
				{
					tempStr  = viewData->text.Mid(oldIndex, index - oldIndex);
					width    = max(width, viewData->view->StringWidth((tempPtr = tempStr.GetString())));
					tempStr.FreeBuffer(tempPtr);
					oldIndex = index + 1;

					height += lineHeight;
				}

				tempStr = viewData->text.Mid(oldIndex);
				width   = max(width, viewData->view->StringWidth((tempPtr = tempStr.GetString()))) + HSPACE;
				tempStr.FreeBuffer(tempPtr);
				height += lineHeight + VSPACE;

				// Tell the view about the tip text
				tipView->SetText(viewData->text);

				// Calculate the show position
				position.y += 16.0f;

				// Check for out of range
				scrRect = scr.Frame();

				if ((position.x + width) > scrRect.right)
					position.x = scrRect.right - width;

				if ((position.y + height) > scrRect.bottom)
					position.y = scrRect.bottom - height;

				// Place the tool tip on the right location
				tipWin->MoveTo(position);
				tipWin->ResizeTo(width, height);
				tipWin->Sync();
				tipWin->Show();
			}
		}
	}
	else
	{
		// Get the mouse position
		GetMouse(&position, &buttons);
		ConvertToScreen(&position);

		// Check to see if we have moved the mouse
		if ((position != oldPos) || /*(buttons != 0) ||*/ (timeCounter == 24) || (!Window()->IsActive()))
		{
			notAgain = (timeCounter == 24) ? true : false;

			// Clear the time counter, because the mouse is moving
			timeCounter = 0;

			// Store the current position
			oldPos = position;

			// If a tool tip is showed, hide it again
			if (showing)
			{
				showing = false;
				tipWin->Hide();
			}
		}
	}
}
