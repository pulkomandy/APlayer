/******************************************************************************/
/* APlayer About window class.                                                */
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
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"
#include "APWindowAbout.h"
#include "ResourceIDs.h"

// APlayer logo
#include "Logo/Logo.h"


/******************************************************************************/
/* Logo defines                                                               */
/******************************************************************************/
#define LOGO_WIDTH		270
#define LOGO_HEIGHT		130



/******************************************************************************/
/* SLZ unpacking defines                                                      */
/******************************************************************************/
/*
 * SHIFT_UPPER is amount to multiply upper in one byte to get into next
 * higher byte. (H=4096 -> 16, H=2048 -> 8)
 * LSR_upper is amount to shift codeword to get upper byte into lower byte.
 *   (H=4096 -> 4, H=2048 -> 3)
 * MAX_COMP_LENGTH = (2 ^ (8 - LSR_upper)) + 1
*/

#define HISTORY_SIZE		4096
#define MASK_history		(HISTORY_SIZE - 1)
#define MASK_upper			(0xF0)
#define MASK_lower			(0x0F)
#define SHIFT_UPPER			16
#define LSR_upper			4
#define MAX_COMP_LEN		17



/******************************************************************************/
/* APWindowAbout class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowAbout::APWindowAbout(MainWindowSystem *system, BRect frame, PString title) : BWindow(frame, NULL, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	char *titleStr;
	BRect rect;

	// Remember the arguments
	windowSystem = system;

	// Create the keyboard filter
	keyFilter = new APKeyFilter(this);
	keyFilter->AddFilterKey(B_ESCAPE, 0);

	// Create background view
	rect    = Bounds();
	topView = new APViewAboutBox(windowSystem, rect);

	// Add view to the window
	AddChild(topView);

	// Change the window title
	SetTitle((titleStr = title.GetString()));
	title.FreeBuffer(titleStr);

	for (int i = 0; i < (LOGO_HEIGHT + 60); i++)
		topView->Pulse();

	// Set scroll speed
	SetPulseRate(100000);

	// Set wait count
	topView->waitCount = 20;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowAbout::~APWindowAbout(void)
{
	windowSystem->aboutWin = NULL;

	// Delete the filter
	delete keyFilter;
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output:  Returns true if it's okay to quit, else false.                    */
/******************************************************************************/
bool APWindowAbout::QuitRequested(void)
{
	BRect winPos;
	int32 x, y, w, h;

	try
	{
		// Store the window position and size if changed
		winPos = Frame();

		x = (int32)winPos.left;
		y = (int32)winPos.top;
		w = winPos.IntegerWidth();
		h = winPos.IntegerHeight();

		// Check to see if they have changed
		if ((windowSystem->saveSettings->GetIntEntryValue("Window", "AboutX") != x) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "AboutY") != y) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "AboutWidth") != w) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "AboutHeight") != h))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", "AboutX", x);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "AboutY", y);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "AboutWidth", w);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "AboutHeight", h);
			windowSystem->useSettings->WriteIntEntryValue("Window", "AboutX", x);
			windowSystem->useSettings->WriteIntEntryValue("Window", "AboutY", y);
			windowSystem->useSettings->WriteIntEntryValue("Window", "AboutWidth", w);
			windowSystem->useSettings->WriteIntEntryValue("Window", "AboutHeight", h);
		}
	}
	catch(...)
	{
		;
	}

	return (true);
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowAbout::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////
		// Drag'n'drop handler
		////////////////////////////////////////////////////////////////////
		case B_SIMPLE_DATA:
		case APLIST_DRAG:
		{
			// Well, the user dropped an object in the window where
			// it's not supported, so we just change the mouse
			// cursor back to normal
			windowSystem->mainWin->SetNormalCursor();
			break;
		}

		////////////////////////////////////////////////////////////////////
		// Global key handler
		////////////////////////////////////////////////////////////////////
		case B_KEY_DOWN:
		{
			BMessage *curMsg;
			int32 key, modifiers;

			// Extract the key that the user has pressed
			curMsg = CurrentMessage();
			if (curMsg->FindInt32("raw_char", &key) == B_OK)
			{
				if (curMsg->FindInt32("modifiers", &modifiers) == B_OK)
				{
					// Mask out lock keys
					modifiers &= ~(B_CAPS_LOCK | B_NUM_LOCK | B_SCROLL_LOCK);

					if ((key == B_ESCAPE) && (modifiers == 0))
					{
						PostMessage(B_QUIT_REQUESTED);
						return;
					}
				}
			}
			break;
		}
	}

	// Call base class
	BWindow::MessageReceived(msg);
}





/******************************************************************************/
/* APViewAboutBox class                                                       */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewAboutBox::APViewAboutBox(MainWindowSystem *system, BRect frame) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED)
{
	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Initialize pointers
	windowSystem = system;
	scrollWin    = NULL;

	// Initialize pulse counter
	counter   = 0;
	waitCount = 0;

	// Initialize string counter
	currentStringNum = IDS_ABOUT_START;

	// Initialize start color
	color = Black;

	// Set mode to text mode
	showMode = apText;

	// Okay, allocate the memory to hold the APlayer logo
	logo = new uint8[((LOGO_WIDTH + 15) / 16) * 16 * LOGO_HEIGHT];

	// Now unpack the logo
	UnpackSLZ(APlayerLogo, logo);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewAboutBox::~APViewAboutBox(void)
{
	delete scrollWin;
	delete[] logo;
}



/******************************************************************************/
/* Pulse() scrolls the scroll text.                                           */
/******************************************************************************/
void APViewAboutBox::Pulse(void)
{
	BRect rect, source;
	uint8 *bitMem;
	int32 totalLen, rowLen;

	if (waitCount != 0)
	{
		waitCount--;
		return;
	}

	// Get bitmap information needed
	bitMem   = (uint8 *)scrollWin->Bits();
	totalLen = scrollWin->BitsLength();
	rowLen   = scrollWin->BytesPerRow();

	// Scroll the bitmap one line up
	memcpy(bitMem, bitMem + rowLen, totalLen - rowLen);

	counter++;
	if (counter == linesToScroll)
	{
		PString str;
		char *strPtr;
		uint32 x, y, w, h;
		bool moreCommands = true;
		bool append;

		counter = 0;

		// Lock the bitmap window
		scrollWin->Lock();

		if (showMode != apLogo)
		{
			// Clear the background
			rect     = scrollRect;
			rect.top = rect.bottom - extraHeight;

			writeView->SetHighColor(White);
			writeView->FillRect(rect);
		}

		// Calculate the x, y, width and height
		x = (uint32)(scrollRect.left + 5.0f);
		y = (uint32)(scrollRect.bottom - extraHeight);
		w = (uint32)(scrollRect.right - x - 5.0f);
		h = (uint32)(scrollRect.bottom - y);

		switch (showMode)
		{
			////////////////////////////////////////////////////////////////////////
			// Show logo
			////////////////////////////////////////////////////////////////////////
			case apLogo:
			{
				// Copy one line into the bitmap
				memcpy(bitMem + totalLen - rowLen + x + ((w - LOGO_WIDTH) / 2), logo + logoLine * LOGO_WIDTH, LOGO_WIDTH);

				logoLine++;
				if (logoLine == LOGO_HEIGHT)
				{
					linesToScroll = extraHeight;
					showMode      = apText;
				}
				break;
			}

			////////////////////////////////////////////////////////////////////////
			// Show addons
			////////////////////////////////////////////////////////////////////////
			case apAddOns:
			{
				BFont font;

				// Special mode: Show add-ons
				addOnList->LockList();

				try
				{
					if (addOnList->IsEmpty())
					{
						// No add-ons installed
						str.LoadString(windowSystem->res, IDS_NONE);
						showMode = apEmpty;
					}
					else
					{
						if (addOnIndex == addOnList->CountItems())
						{
							// No more add-ons to show
							str.MakeEmpty();
							showMode = apText;
						}
						else
						{
							// Okay, show the current add-ons
							str = addOnList->GetItem(addOnIndex)->name;
							addOnIndex++;
						}
					}
				}
				catch(...)
				{
					showMode = apText;
				}

				addOnList->UnlockList();

				// Center the string and draw it
				writeView->GetFont(&font);

				strPtr = str.GetString();
				x += (uint32)((w - font.StringWidth(strPtr)) / 2.0f);
				writeView->MovePenTo(x, y + baseLine);

				writeView->SetHighColor(color);
				writeView->DrawString(strPtr);
				str.FreeBuffer(strPtr);
				break;
			}

			////////////////////////////////////////////////////////////////////////
			// Text mode
			////////////////////////////////////////////////////////////////////////
			case apText:
			{
				// Get the next line to show
				str.LoadString(windowSystem->res, currentStringNum++);

				// Do only parse and write the string if is not empty
				if (!(str.IsEmpty()))
				{
					// Set the pen position
					writeView->MovePenTo(x, y + baseLine);

					do
					{
						append = false;

						if (str.GetAt(0) == PChar("¤", 2))
						{
							do
							{
								switch (str.GetAt(1).GetChar()[0])
								{
									// End of scroll text
									case '1':
									{
										currentStringNum = IDS_ABOUT_START;
										str.LoadString(windowSystem->res, currentStringNum++);
										break;
									}

									// Show logo
									case '2':
									{
										str = str.Mid(2);

										linesToScroll = 1;
										logoLine      = 0;
										showMode      = apLogo;
										moreCommands  = false;
										break;
									}

									// Center the line
									case '3':
									{
										BFont font;

										// Cut out the command character
										str = str.Mid(2);

										// Get the view font
										writeView->GetFont(&font);

										// Center the string
										x += (uint32)((w - font.StringWidth((strPtr = str.GetString()))) / 2.0f);
										writeView->MovePenTo(x, y + baseLine);
										str.FreeBuffer(strPtr);
										break;
									}

									// Take the next string and append it
									case '4':
									{
										str    = str.Mid(2);
										append = true;
										break;
									}

									// Change color to black
									case '5':
									{
										str   = str.Mid(2);
										color = Black;
										break;
									}

									// Change color to red
									case '6':
									{
										str   = str.Mid(2);
										color = Red;
										break;
									}

									// Change color to blue
									case '7':
									{
										str   = str.Mid(2);
										color = Blue;
										break;
									}

									// Show players
									case '8':
									{
										str = str.Mid(2);

										addOnList    = &windowSystem->playerAddOns;
										addOnIndex   = 0;
										showMode     = apAddOns;
										moreCommands = false;
										break;
									}

									// Show agents
									case '9':
									{
										str = str.Mid(2);

										addOnList    = &windowSystem->agentAddOns;
										addOnIndex   = 0;
										showMode     = apAddOns;
										moreCommands = false;
										break;
									}

									// Show clients
									case 'A':
									{
										str = str.Mid(2);

										addOnList    = &windowSystem->clientAddOns;
										addOnIndex   = 0;
										showMode     = apAddOns;
										moreCommands = false;
										break;
									}

									// Show APlayer version number
									case 'B':
									{
										PString tempStr;

										tempStr.Format("%d.%d.%d", windowSystem->versionMajor, windowSystem->versionMiddle, windowSystem->versionMinor);
										str = str.Mid(2) + tempStr;
										break;
									}

									default:
									{
										moreCommands = false;
										break;
									}
								}
							}
							while (moreCommands);
						}

						// Write the string
						writeView->SetHighColor(color);
						writeView->DrawString((strPtr = str.GetString()));
						str.FreeBuffer(strPtr);

						if (append)
						{
							str.LoadString(windowSystem->res, currentStringNum++);
							moreCommands = true;
						}
					}
					while (append);
				}
				break;
			}

			////////////////////////////////////////////////////////////////////////
			// Default to an empty line
			////////////////////////////////////////////////////////////////////////
			default:
			{
				showMode = apText;
				break;
			}
		}

		// Wait for the view to draw the images and unlock the bitmap window again
		writeView->Sync();
		scrollWin->Unlock();
	}

	// Draw the bitmap again
	rect         = Bounds();
	rect.left   += (HSPACE * 2.0f + 1.0f);
	rect.right  -= (HSPACE * 2.0f + 1.0f);
	rect.top    += (VSPACE * 2.0f + 1.0f);
	rect.bottom -= (VSPACE * 2.0f + 1.0f);

	source         = scrollRect;
	source.bottom -= extraHeight;

	DrawBitmap(scrollWin, source, rect);
}



/******************************************************************************/
/* AttachedToWindow() is called when the view is attached to the window. It   */
/*         will initialize the view.                                          */
/******************************************************************************/
void APViewAboutBox::AttachedToWindow(void)
{
	BFont font;
	font_height fh;

	// Initialize the font
	font.SetFamilyAndStyle("Swis721 BT", "Roman");
	font.SetSize(10);

	// Find the font height
	font.GetHeight(&fh);
	extraHeight   = (uint32)ceil(fh.ascent + fh.descent);
	linesToScroll = extraHeight;
	counter       = extraHeight - 1;

	// How many pixel to add to find the base line
	baseLine = (uint32)ceil(fh.ascent);

	// Create the bitmap where the scroll text will be shown in
	scrollRect         = Bounds();
	scrollRect.left    = 0.0f;
	scrollRect.top     = 0.0f;
	scrollRect.right  -= (HSPACE * 4.0f + 2.0f);
	scrollRect.bottom -= (VSPACE * 4.0f + 2.0f);
	scrollRect.bottom += extraHeight;

	scrollWin = new BBitmap(scrollRect, B_COLOR_8_BIT, true);

	// Create the view to write in and attach it to the bitmap
	writeView = new BView(scrollRect, NULL, B_FOLLOW_NONE, 0);
	writeView->SetFont(&font);
	scrollWin->AddChild(writeView);

	// Fill the bitmap with white
	scrollWin->Lock();
	writeView->SetHighColor(White);
	writeView->FillRect(scrollRect);
	writeView->Sync();
	scrollWin->Unlock();
}



/******************************************************************************/
/* Draw() is called when some of the view need to be drawn.                   */
/*                                                                            */
/* Input:  "updateRect" is the rectangle where there needs an update.         */
/******************************************************************************/
void APViewAboutBox::Draw(BRect updateRect)
{
	BRect rect, source;

	// Draw the frame
	rect         = Bounds();
	rect.left   += HSPACE * 2.0f;
	rect.right  -= (HSPACE * 2.0f);
	rect.top    += VSPACE * 2.0f;
	rect.bottom -= (VSPACE * 2.0f);

	SetHighColor(BeDarkShadow);
	StrokeRect(rect);

	// Draw the bitmap
	rect.left   += 1.0f;
	rect.right  -= 1.0f;
	rect.top    += 1.0f;
	rect.bottom -= 1.0f;

	source         = scrollRect;
	source.bottom -= extraHeight;

	DrawBitmap(scrollWin, source, rect);
}



/******************************************************************************/
/* UnpackSLZ() is the SLZ unpack routine. Make sure the "outBuffer" is big    */
/*         enough to hold the unpacked data, else ...                         */
/*                                                                            */
/* Input:  "inBuffer" is a pointer to the packed data.                        */
/*         "outBuffer" is a pointer to store the unpacked data.               */
/******************************************************************************/
void APViewAboutBox::UnpackSLZ(uint8 *inBuffer, uint8 *outBuffer)
{
	int16 myTAG, myCount, myOffset;
	int32 loop1;
	int16 lzhist_offset = 0;
	uint8 *LZ_history;

	// Allocate memory to history buffer
	LZ_history = new uint8[HISTORY_SIZE];

	for (;;)	// loop forever (until goto occurs to break out of loop)
	{
		myTAG = *inBuffer++;

		for (loop1 = 0; (loop1 != 8); loop1++)
		{
			if (myTAG & 0x80)
			{
				if ((myCount = *inBuffer++) == 0)	// Check EXIT
					goto skip2;		// goto's are gross but it's efficient :(
				else
				{
					myOffset = HISTORY_SIZE - (((MASK_upper & myCount) * SHIFT_UPPER) + (*inBuffer++));
					myCount &= MASK_lower;
					myCount += 2;

					while (myCount != 0)
					{
						*outBuffer = LZ_history[(lzhist_offset + myOffset) & MASK_history];
						LZ_history[lzhist_offset] = *outBuffer++;
						lzhist_offset = (lzhist_offset + 1) & MASK_history;

						myCount--;
					}
				}
			}
			else
			{
				*outBuffer = *inBuffer++;
				LZ_history[lzhist_offset] = *outBuffer++;
				lzhist_offset = (lzhist_offset + 1) & MASK_history;
			}

			myTAG += myTAG;
		}
	}

skip2:
	delete[] LZ_history;
}
