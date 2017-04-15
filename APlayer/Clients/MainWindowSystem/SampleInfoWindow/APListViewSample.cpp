/******************************************************************************/
/* APlayer Sample Info Sample list view class.                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include <santa/ColumnListView.h>
#include <santa/CLVColumnLabelView.h>

// Client headers
#include "APWindowSampleInfo.h"
#include "APListViewSample.h"
#include "APListItemSample.h"


/******************************************************************************/
/* Key table                                                                  */
/******************************************************************************/
static int32 keyTab[] =
{
	13, 15, -1, 18, 20, 22, -1, 25, 27, -1, 30, 32, -1, -1, -1, -1, -1, -1,
	-1, -1, 12, 14, 16, 17, 19, 21, 23, 24, 26, 28, 29, 31, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,  1,  3, -1,  6,  8, 10, -1, 13, 15, -1, 33, -1,
	-1, -1, -1,  0,  2,  4,  5,  7,  9, 11, 12, 14, 16
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APListViewSample::APListViewSample(BRect frame, CLVContainerView** containerView, uint32 resizingMode, uint32 flags, list_view_type type, bool hierarchical, bool horizontal, bool vertical, bool scrollViewCorner, border_style border, const BFont* labelFont) :
	ColumnListView(frame, containerView, NULL, resizingMode, flags, type, hierarchical, horizontal, vertical, scrollViewCorner, border, labelFont)
{
	// Set the font
	SetFont(be_fixed_font);

	// Initialize the member variables
	octave    = 48;
	polyphony = false;
	keyCount  = 1;

	for (int8 i = 0; i < POLYPHONY_CHANNELS; i++)
		keys[i] = -1;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APListViewSample::~APListViewSample(void)
{
}



/******************************************************************************/
/* KeyDown() is called for every key the user presses when the list has the   */
/*      focus. It parses the key and play the selected sample.                */
/*                                                                            */
/* Input:  "bytes" is a pointer to the UTF-8 character.                       */
/*         "numBytes" is the number of bytes the key has.                     */
/******************************************************************************/
void APListViewSample::KeyDown(const char *bytes, int32 numBytes)
{
	status_t result;
	BMessage *msg;
	int32 key, i;

	if (keyCount > 0)
	{
		// Get the current message
		msg = Window()->CurrentMessage();

		// Find the raw value of the key pressed
		result = msg->FindInt32("key", &key);
		if (result == B_OK)
		{
			switch (key)
			{
				// TAB and Delete
				case 0x26:
				case 0x34:
				{
					if (polyphony)
					{
						polyphony = false;
						keyCount  = 1;
					}
					else
					{
						polyphony = true;
						keyCount  = POLYPHONY_CHANNELS;
					}

					((APWindowSampleInfo *)Window())->ChangePolyphony(polyphony);
					return;
				}

				// ESC and SPACE
				case 0x01:
				case 0x5e:
				{
					// Add an empty sample
					((APWindowSampleInfo *)Window())->AddSampleToQuery(NULL, 0.0f);

					// Clear up the key variables
					if (polyphony)
					{
						keyCount = POLYPHONY_CHANNELS;
						for (i = 0; i < POLYPHONY_CHANNELS; i++)
							keys[i] = -1;
					}
					else
						keyCount = 1;

					break;
				}

				// F1
				case 0x02:
				{
					octave = 0;
					((APWindowSampleInfo *)Window())->ChangeOctave(0);
					break;
				}

				// F2
				case 0x03:
				{
					octave = 12;
					((APWindowSampleInfo *)Window())->ChangeOctave(1);
					break;
				}

				// F3
				case 0x04:
				{
					octave = 24;
					((APWindowSampleInfo *)Window())->ChangeOctave(2);
					break;
				}

				// F4
				case 0x05:
				{
					octave = 36;
					((APWindowSampleInfo *)Window())->ChangeOctave(3);
					break;
				}

				// F5
				case 0x06:
				{
					octave = 48;
					((APWindowSampleInfo *)Window())->ChangeOctave(4);
					break;
				}

				// F6
				case 0x07:
				{
					octave = 60;
					((APWindowSampleInfo *)Window())->ChangeOctave(5);
					break;
				}

				// F7
				case 0x08:
				{
					octave = 72;
					((APWindowSampleInfo *)Window())->ChangeOctave(6);
					break;
				}

				// F8
				case 0x09:
				{
					octave = 84;
					((APWindowSampleInfo *)Window())->ChangeOctave(7);
					break;
				}

				// The rest of the keys
				default:
				{
					// Get current selected item
					int32 selected = CurrentSelection();

					// Any item selected?
					if (selected != -1)
					{
						// Get the item
						APListItemSample *item = (APListItemSample *)ItemAt(selected);

						// Get the sample information
						const APSampleInfo *sampInfo = item->GetSampleInfo();

						// Set the loop flag
						if (sampInfo->flags & APSAMP_LOOP)
							loops = true;
						else
							loops = false;

						// Count down number of simulated keys
						keyCount--;

						// Find an empty space in the key array
						for (i = 0; i < POLYPHONY_CHANNELS; i++)
						{
							if (keys[i] == -1)
							{
								keys[i] = key;
								break;
							}
						}

						// Find the note out from the pressed key
						int32 note = FindNoteFromKey(key);
						if (note != -1)
						{
							// Calculate the frequency
							double hertz = sampInfo->middleC;

							if (note < 48)
							{
								for (i = note; i < 48; i++)
									hertz /= 1.059463094359;
							}
							else
							{
								for (i = 48; i < note; i++)
									hertz *= 1.059463094359;
							}

							// Add the sample info to the query
							((APWindowSampleInfo *)Window())->AddSampleToQuery(sampInfo, hertz);
						}
					}
				}
			}
		}
	}

	// Call base class
	ColumnListView::KeyDown(bytes, numBytes);
}



/******************************************************************************/
/* KeyUp() is called for every key the user releases when the list has the    */
/*      focus.                                                                */
/*                                                                            */
/* Input:  "bytes" is a pointer to the UTF-8 character.                       */
/*         "numBytes" is the number of bytes the key has.                     */
/******************************************************************************/
void APListViewSample::KeyUp(const char *bytes, int32 numBytes)
{
	status_t result;
	BMessage *msg;
	int32 key, i;

	// Get the current message
	msg = Window()->CurrentMessage();

	// Find the raw value of the key pressed
	result = msg->FindInt32("key", &key);
	if (result == B_OK)
	{
		// Try to find the key in the key array
		for (i = 0; i < POLYPHONY_CHANNELS; i++)
		{
			if (keys[i] == key)
			{
				// One more key can be pressed
				keys[i] = -1;
				keyCount++;

				// Add an empty sample if the sample loops
				if (loops)
					((APWindowSampleInfo *)Window())->AddSampleToQuery(NULL, 0.0f);

				break;
			}
		}
	}

	// Call base class
	ColumnListView::KeyUp(bytes, numBytes);
}



/******************************************************************************/
/* AttachedToWindow() is called everytime the view has to be shown.           */
/******************************************************************************/
void APListViewSample::AttachedToWindow(void)
{
	APWindowSampleInfo *win;

	// Call base class
	ColumnListView::AttachedToWindow();

	// Update the static texts
	win = (APWindowSampleInfo *)Window();
	win->ChangeOctave(octave / 12);
	win->ChangePolyphony(polyphony);
}



/******************************************************************************/
/* FindNoteFromKey() tries to find the key given as the argument in the key   */
/*      map and then return the corresponding note number.                    */
/*                                                                            */
/* Input:  "key" is the key pressed.                                          */
/*                                                                            */
/* Output: The note number or -1 if it couldn't be found.                     */
/******************************************************************************/
int32 APListViewSample::FindNoteFromKey(int32 key)
{
	int32 note;

	// Check to see if the key is in the valid range
	if ((key < 0x13) || (key > 0x55))
		return (-1);

	// Look up the key in the table
	note = keyTab[key - 0x13];
	if (note != -1)
		note += octave;

	return (note);
}
