/******************************************************************************/
/* APlayer Settings Path View class.                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/

#include <compat/sys/stat.h>

// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PSettings.h"
#include "PDirectory.h"
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APViewPaths.h"
#include "APDiskButton.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* APViewPathFilter class                                                     */
/******************************************************************************/

/******************************************************************************/
/* Filter() is called for each entry read from the file panel.                */
/*                                                                            */
/* Input:  Different types of the entry.                                      */
/*                                                                            */
/* Output: True if the entry is valid, else false.                            */
/******************************************************************************/
bool APViewPathFilter::Filter(const entry_ref *ref, BNode *node, stat_beos *st, const char *filetype)
{
	if (S_ISDIR(st->st_mode))
		return (true);

	return (false);
}





/******************************************************************************/
/* APViewPaths class                                                          */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewPaths::APViewPaths(MainWindowSystem *system, BRect frame, PString name) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	PString text1, text2, text3;
	char *strPtr;
	float maxTextWidth, textWidth1, textWidth2, textWidth3;
	BMessage *message;
	BTextView *textView;
	float x, y1, y2, w;
	float controlWidth, controlHeight;
	font_height fh;
	float fontHeight;

	// Remember the arguments
	windowSystem = system;
	res          = system->res;

	// Initialize member variables
	startScanPanel = NULL;
	modulePanel    = NULL;
	apmlPanel      = NULL;
	messenger      = NULL;

	// Set the view name
	SetName((strPtr = name.GetString()));
	name.FreeBuffer(strPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// Create the "Paths" box
	//
	pathsBox = new BBox(BRect(0.0f, fontHeight / 2.0f, SET_MIN_VIEW_WIDTH, SET_MIN_VIEW_HEIGHT), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	AddChild(pathsBox);

	// Find the longest text
	text1.LoadString(res, IDS_SET_PAT_STARTSCAN);
	textWidth1 = StringWidth((strPtr = text1.GetString()));
	text1.FreeBuffer(strPtr);

	text2.LoadString(res, IDS_SET_PAT_MODULE);
	textWidth2 = StringWidth((strPtr = text2.GetString()));
	text2.FreeBuffer(strPtr);

	text3.LoadString(res, IDS_SET_PAT_APML);
	textWidth3 = StringWidth((strPtr = text3.GetString()));
	text3.FreeBuffer(strPtr);

	maxTextWidth = max(textWidth1, textWidth2);
	maxTextWidth = max(maxTextWidth, textWidth3);

	x = HSPACE + maxTextWidth + HSPACE * 2.0f;
	w = SET_MIN_VIEW_WIDTH - (PICHSIZE + HSPACE) - HSPACE - x - HSPACE * 2.0f;

	// Create the "Start scan" edit box
	message = new BMessage(SET_PAT_EDIT_STARTSCAN);
	startScanEdit = new BTextControl(BRect(x, 0.0f, x + w, 0.0f), NULL, NULL, NULL, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	startScanEdit->GetPreferredSize(&controlWidth, &controlHeight);

	controlHeight = max(controlHeight, fontHeight + VSPACE);

	startScanEdit->ResizeTo(controlWidth, controlHeight);
	textView = startScanEdit->TextView();
	textView->SetMaxBytes(B_PATH_NAME_LENGTH);
	pathsBox->AddChild(startScanEdit);

	message = new BMessage(SET_PAT_BUT_STARTSCAN);
	startScanButton = new APDiskButton(res, IDI_SET_DISK_NORMAL, IDI_SET_DISK_PRESSED, BRect(0.0f, 0.0f, PICHSIZE + HSPACE, controlHeight), message);
	pathsBox->AddChild(startScanButton);

	// Calculate the first y position
	y2 = ((SET_MIN_VIEW_HEIGHT - fontHeight) - (controlHeight * 3.0f + VSPACE * 4.0f * 2.0f)) / 2.0f;
	y1 = y2 + ((controlHeight - fontHeight) / 2.0f);

	startScanEdit->MoveBy(0.0f, y2);
	startScanButton->MoveTo(SET_MIN_VIEW_WIDTH - HSPACE - (PICHSIZE + HSPACE), y2);

	startScanText = new BStringView(BRect(HSPACE * 2.0f, y1, HSPACE * 2.0f + textWidth1, y1 + fontHeight), NULL, (strPtr = text1.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	pathsBox->AddChild(startScanText);
	text1.FreeBuffer(strPtr);

	// Create the "Module" edit box
	y1 += (controlHeight + VSPACE * 4.0f);
	y2 += (controlHeight + VSPACE * 4.0f);

	moduleText = new BStringView(BRect(HSPACE * 2.0f, y1, HSPACE * 2.0f + textWidth2, y1 + fontHeight), NULL, (strPtr = text2.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	pathsBox->AddChild(moduleText);
	text2.FreeBuffer(strPtr);

	message = new BMessage(SET_PAT_EDIT_MODULE);
	moduleEdit = new BTextControl(BRect(x, y2, x + w, y2 + controlHeight), NULL, NULL, NULL, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER);
	textView = moduleEdit->TextView();
	textView->SetMaxBytes(B_PATH_NAME_LENGTH);
	pathsBox->AddChild(moduleEdit);

	message = new BMessage(SET_PAT_BUT_MODULE);
	moduleButton = new APDiskButton(res, IDI_SET_DISK_NORMAL, IDI_SET_DISK_PRESSED, BRect(SET_MIN_VIEW_WIDTH - HSPACE - (PICHSIZE + HSPACE), y2, (SET_MIN_VIEW_WIDTH - HSPACE - (PICHSIZE + HSPACE)) + PICHSIZE + HSPACE, y2 + controlHeight), message);
	pathsBox->AddChild(moduleButton);

	// Create the "APML" edit box
	y1 += (controlHeight + VSPACE * 4.0f);
	y2 += (controlHeight + VSPACE * 4.0f);

	apmlText = new BStringView(BRect(HSPACE * 2.0f, y1, HSPACE * 2.0f + textWidth3, y1 + fontHeight), NULL, (strPtr = text3.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	pathsBox->AddChild(apmlText);
	text3.FreeBuffer(strPtr);

	message = new BMessage(SET_PAT_EDIT_APML);
	apmlEdit = new BTextControl(BRect(x, y2, x + w, y2 + controlHeight), NULL, NULL, NULL, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER);
	textView = apmlEdit->TextView();
	textView->SetMaxBytes(B_PATH_NAME_LENGTH);
	pathsBox->AddChild(apmlEdit);

	message = new BMessage(SET_PAT_BUT_APML);
	apmlButton = new APDiskButton(res, IDI_SET_DISK_NORMAL, IDI_SET_DISK_PRESSED, BRect(SET_MIN_VIEW_WIDTH - HSPACE - (PICHSIZE + HSPACE), y2, (SET_MIN_VIEW_WIDTH - HSPACE - (PICHSIZE + HSPACE)) + PICHSIZE + HSPACE, y2 + controlHeight), message);
	pathsBox->AddChild(apmlButton);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewPaths::~APViewPaths(void)
{
	delete apmlPanel;
	delete modulePanel;
	delete startScanPanel;
	delete messenger;
}



/******************************************************************************/
/* InitSettings() will read the settings and set all the controls.            */
/******************************************************************************/
void APViewPaths::InitSettings(void)
{
	PString tempStr;
	char *strPtr;

	//
	// Paths box
	//
	tempStr = windowSystem->useSettings->GetStringEntryValue("Paths", "StartScan");
	startScanEdit->SetText((strPtr = tempStr.GetString()));
	tempStr.FreeBuffer(strPtr);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Paths", "Modules");
	moduleEdit->SetText((strPtr = tempStr.GetString()));
	tempStr.FreeBuffer(strPtr);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Paths", "APML");
	apmlEdit->SetText((strPtr = tempStr.GetString()));
	tempStr.FreeBuffer(strPtr);
}



/******************************************************************************/
/* RememberSettings() will read the data from controls which hasn't stored    */
/*      they values yet, eg. edit controls.                                   */
/******************************************************************************/
void APViewPaths::RememberSettings(void)
{
	windowSystem->useSettings->WriteStringEntryValue("Paths", "StartScan", startScanEdit->Text());
	windowSystem->useSettings->WriteStringEntryValue("Paths", "Modules", moduleEdit->Text());
	windowSystem->useSettings->WriteStringEntryValue("Paths", "APML", apmlEdit->Text());
}



/******************************************************************************/
/* CancelSettings() will restore real-time values.                            */
/******************************************************************************/
void APViewPaths::CancelSettings(void)
{
}



/******************************************************************************/
/* SaveSettings() will save some of the settings in other files.              */
/******************************************************************************/
void APViewPaths::SaveSettings(void)
{
}



/******************************************************************************/
/* SaveWindowSettings() will save the view specific settings entries.         */
/******************************************************************************/
void APViewPaths::SaveWindowSettings(void)
{
}



/******************************************************************************/
/* MakeBackup() will make a backup of special settings not stored in the      */
/*      "use" settings.                                                       */
/******************************************************************************/
void APViewPaths::MakeBackup(void)
{
}



/******************************************************************************/
/* HandleMessage() is called for each message the window don't know.          */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APViewPaths::HandleMessage(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Start Scan path editbox
		////////////////////////////////////////////////////////////////////////
		case SET_PAT_EDIT_STARTSCAN:
		{
			windowSystem->useSettings->WriteStringEntryValue("Paths", "StartScan", startScanEdit->Text());
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Start Scan path button
		////////////////////////////////////////////////////////////////////////
		case SET_PAT_BUT_STARTSCAN:
		{
			ShowFilePanel(startScanEdit->Text(), &startScanPanel, SET_PAT_BUT_STARTSCAN);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Module path editbox
		////////////////////////////////////////////////////////////////////////
		case SET_PAT_EDIT_MODULE:
		{
			windowSystem->useSettings->WriteStringEntryValue("Paths", "Modules", moduleEdit->Text());
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Module path button
		////////////////////////////////////////////////////////////////////////
		case SET_PAT_BUT_MODULE:
		{
			ShowFilePanel(moduleEdit->Text(), &modulePanel, SET_PAT_BUT_MODULE);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// APML path editbox
		////////////////////////////////////////////////////////////////////////
		case SET_PAT_EDIT_APML:
		{
			windowSystem->useSettings->WriteStringEntryValue("Paths", "APML", apmlEdit->Text());
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// APML path button
		////////////////////////////////////////////////////////////////////////
		case SET_PAT_BUT_APML:
		{
			ShowFilePanel(apmlEdit->Text(), &apmlPanel, SET_PAT_BUT_APML);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Message from the file panel
		////////////////////////////////////////////////////////////////////////
		case B_REFS_RECEIVED:
		{
			entry_ref entRef;
			BEntry entry;
			BPath path;
			int32 id;
			PString str;
			char *strPtr;

			if (msg->FindRef("refs", 0, &entRef) == B_OK)
			{
				if (msg->FindInt32("diskID", 0, &id) == B_OK)
				{
					entry.SetTo(&entRef);
					entry.GetPath(&path);

					// Append slash
					str    = path.Path();
					str   += P_DIRSLASH_STR;
					strPtr = str.GetString();

					switch (id)
					{
						case SET_PAT_BUT_STARTSCAN:
						{
							startScanEdit->SetText(strPtr);
							break;
						}

						case SET_PAT_BUT_MODULE:
						{
							moduleEdit->SetText(strPtr);
							break;
						}

						case SET_PAT_BUT_APML:
						{
							apmlEdit->SetText(strPtr);
							break;
						}
					}

					str.FreeBuffer(strPtr);
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Message from the file panel, which is always sent when the panel
		// hides
		////////////////////////////////////////////////////////////////////////
		case B_CANCEL:
		{
			int32 id;

			if (msg->FindInt32("diskID", 0, &id) == B_OK)
			{
				switch (id)
				{
					case SET_PAT_BUT_STARTSCAN:
						delete startScanPanel;
						startScanPanel = NULL;
						break;

					case SET_PAT_BUT_MODULE:
						delete modulePanel;
						modulePanel = NULL;
						break;

					case SET_PAT_BUT_APML:
						delete apmlPanel;
						apmlPanel = NULL;
						break;
				}
			}
			break;
		}
	}
}



/******************************************************************************/
/* ShowFilePanel() will show the file panel.                                  */
/*                                                                            */
/* Input:  "startPath" is where the file panel has to start.                  */
/*         "panel" is a pointer to store the pointer to the file panel.       */
/*         "id" is the id that is stored in the message sent.                 */
/******************************************************************************/
void APViewPaths::ShowFilePanel(PString startPath, BFilePanel **panel, int32 id)
{
	BMessage msg(B_REFS_RECEIVED);
	PString str;
	char *strPtr;

	// Prepare the message
	msg.AddInt32("diskID", id);

	// Start to allocate the messenger
	if (messenger == NULL)
	{
		messenger = new BMessenger(this);
		if (messenger == NULL)
			throw PMemoryException();
	}

	// Allocate the panel
	if (*panel == NULL)
	{
		*panel = new BFilePanel(B_OPEN_PANEL, messenger, NULL, B_DIRECTORY_NODE, false, &msg, &filter);
		if (*panel == NULL)
			throw PMemoryException();
	}

	// Change the start entry
	str = PDirectory::GetParentDirectory(startPath);
	(*panel)->SetPanelDirectory((strPtr = str.GetString()));
	str.FreeBuffer(strPtr);

	// Change the look of the panel
	str.LoadString(res, IDS_PANEL_SELECT);
	(*panel)->SetButtonLabel(B_DEFAULT_BUTTON, (strPtr = str.GetString()));
	str.FreeBuffer(strPtr);

	// Show the panel
	(*panel)->Show();
}
