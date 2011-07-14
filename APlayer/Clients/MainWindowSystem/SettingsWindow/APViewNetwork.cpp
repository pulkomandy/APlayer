/******************************************************************************/
/* APlayer Settings Network View class.                                       */
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
#include "PResource.h"
#include "PSettings.h"
#include "PDirectory.h"
#include "PAlert.h"
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APViewNetwork.h"
#include "APViewPaths.h"
#include "APDiskButton.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* APViewNetwork class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewNetwork::APViewNetwork(MainWindowSystem *system, BRect frame, PString name) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect rect;
	PString label;
	char *labelPtr;
	BMessage *message;
	BTextView *textView;
	float x, y, w, y1, w1;
	float textWidth;
	float controlWidth, controlHeight;
	font_height fh;
	float fontHeight;

	// Remember the arguments
	windowSystem = system;
	res          = system->res;

	// Initialize member variables
	downloadPanel = NULL;
	messenger     = NULL;

	// Set the view name
	SetName((labelPtr = name.GetString()));
	name.FreeBuffer(labelPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// Create the "Proxy Server" box
	//
	label.LoadString(res, IDS_SET_NET_PROXYSERVER);
	proxyBox = new BBox(BRect(0.0f, 0.0f, SET_MIN_VIEW_WIDTH, 0.0f), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	proxyBox->SetLabel((labelPtr = label.GetString()));
	AddChild(proxyBox);
	label.FreeBuffer(labelPtr);

	rect = proxyBox->Bounds();
	rect.InsetBy(HSPACE * 2.0f, 0.0f);

	x = HSPACE;
	y = rect.top + fontHeight + VSPACE;

	message = new BMessage(SET_NET_CHECK_USEPROXY);
	label.LoadString(res, IDS_SET_NET_USEPROXY);
	proxyCheck = new BCheckBox(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message);
	proxyCheck->GetPreferredSize(&controlWidth, &controlHeight);
	proxyCheck->ResizeTo(controlWidth, controlHeight);
	proxyBox->AddChild(proxyCheck);
	label.FreeBuffer(labelPtr);

	y += controlHeight + VSPACE * 2.0f;

	label.LoadString(res, IDS_SET_NET_PROXYADDRESS);
	textWidth = StringWidth((labelPtr = label.GetString()));

	x = HSPACE + textWidth + HSPACE * 2.0f;
	w = SET_MIN_VIEW_WIDTH - x - HSPACE * 2.0f;

	message = new BMessage(SET_NET_EDIT_PROXYADDRESS);
	proxyEdit = new BTextControl(BRect(x, y, x + w, y), NULL, NULL, NULL, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	proxyEdit->GetPreferredSize(&controlWidth, &controlHeight);

	controlHeight = max(controlHeight, fontHeight + VSPACE);

	proxyEdit->ResizeTo(controlWidth, controlHeight);
	proxyBox->AddChild(proxyEdit);

	y1 = y + ((controlHeight - fontHeight) / 2.0f);
	proxyText = new BStringView(BRect(HSPACE, y1, HSPACE + textWidth, y1 + fontHeight), NULL, labelPtr, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	proxyBox->AddChild(proxyText);
	label.FreeBuffer(labelPtr);

	y += controlHeight + VSPACE;
	proxyBox->ResizeTo(SET_MIN_VIEW_WIDTH, y);


	//
	// Create the "Updates" box
	//
	y += VSPACE * 5.0f;
	label.LoadString(res, IDS_SET_NET_UPDATES);
	updateBox = new BBox(BRect(0.0f, y, SET_MIN_VIEW_WIDTH, y), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	updateBox->SetLabel((labelPtr = label.GetString()));
	AddChild(updateBox);
	label.FreeBuffer(labelPtr);

	rect = updateBox->Bounds();
	rect.InsetBy(HSPACE * 2.0f, 0.0f);
	y = rect.top + fontHeight + VSPACE;

	checkPop = new BPopUpMenu("");

	message = new BMessage(SET_NET_LIST_CHECKUPDATES);
	label.LoadString(res, IDS_SET_NET_CHECKITEM0);
	textWidth = StringWidth((labelPtr = label.GetString()));
	checkPop->AddItem(new BMenuItem(label.GetString(), message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_NET_LIST_CHECKUPDATES);
	label.LoadString(res, IDS_SET_NET_CHECKITEM1);
	textWidth = max(StringWidth((labelPtr = label.GetString())), textWidth);
	checkPop->AddItem(new BMenuItem(label.GetString(), message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_NET_LIST_CHECKUPDATES);
	label.LoadString(res, IDS_SET_NET_CHECKITEM2);
	textWidth = max(StringWidth((labelPtr = label.GetString())), textWidth);
	checkPop->AddItem(new BMenuItem(label.GetString(), message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_NET_LIST_CHECKUPDATES);
	label.LoadString(res, IDS_SET_NET_CHECKITEM3);
	textWidth = max(StringWidth((labelPtr = label.GetString())), textWidth);
	checkPop->AddItem(new BMenuItem(label.GetString(), message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_NET_LIST_CHECKUPDATES);
	label.LoadString(res, IDS_SET_NET_CHECKITEM4);
	textWidth = max(StringWidth((labelPtr = label.GetString())), textWidth);
	checkPop->AddItem(new BMenuItem(label.GetString(), message));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SET_NET_CHECKUPDATES);
	w = StringWidth((labelPtr = label.GetString())) + HSPACE;

	w1 = w + textWidth + HSPACE * 8.0f;
	checkField = new BMenuField(BRect(HSPACE, y, HSPACE + w1, y), NULL, labelPtr, checkPop);
	checkField->SetDivider(w);
	updateBox->AddChild(checkField);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_NET_BUT_CHECKNOW);
	label.LoadString(res, IDS_SET_NET_CHECKNOW);
	checkNowButton = new BButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	checkNowButton->GetPreferredSize(&controlWidth, &controlHeight);

	checkNowButton->ResizeTo(controlWidth, controlHeight);
	checkNowButton->MoveTo(rect.right - controlWidth + HSPACE, y);
	updateBox->AddChild(checkNowButton);
	label.FreeBuffer(labelPtr);

	y += fontHeight + VSPACE * 3.0f + 1.0f + VSPACE;

	label.LoadString(res, IDS_SET_NET_DOWNLOAD);
	textWidth = StringWidth((labelPtr = label.GetString()));

	x = HSPACE + textWidth + HSPACE * 2.0f;
	w = SET_MIN_VIEW_WIDTH - (PICHSIZE + HSPACE) - HSPACE - x - HSPACE * 2.0f;

	message = new BMessage(SET_NET_EDIT_DOWNLOAD);
	downloadEdit = new BTextControl(BRect(x, y, x + w, y), NULL, NULL, NULL, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	downloadEdit->GetPreferredSize(&controlWidth, &controlHeight);

	controlHeight = max(controlHeight, fontHeight + VSPACE);

	downloadEdit->ResizeTo(controlWidth, controlHeight);
	textView = downloadEdit->TextView();
	textView->SetMaxBytes(B_PATH_NAME_LENGTH);
	updateBox->AddChild(downloadEdit);

	y1 = y + ((controlHeight - fontHeight) / 2.0f);
	downloadText = new BStringView(BRect(HSPACE, y1, HSPACE + textWidth, y1 + fontHeight), NULL, labelPtr, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	updateBox->AddChild(downloadText);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_NET_BUT_DOWNLOAD);
	downloadButton = new APDiskButton(res, IDI_SET_DISK_NORMAL, IDI_SET_DISK_PRESSED, BRect(SET_MIN_VIEW_WIDTH - HSPACE - (PICHSIZE + HSPACE), y, (SET_MIN_VIEW_WIDTH - HSPACE - (PICHSIZE + HSPACE)) + PICHSIZE + HSPACE, y + controlHeight), message);
	updateBox->AddChild(downloadButton);

	y += controlHeight + VSPACE;
	updateBox->ResizeTo(SET_MIN_VIEW_WIDTH, y);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewNetwork::~APViewNetwork(void)
{
	delete downloadPanel;
}



/******************************************************************************/
/* InitSettings() will read the settings and set all the controls.            */
/******************************************************************************/
void APViewNetwork::InitSettings(void)
{
	PString tempStr;
	char *strPtr;
	int32 tempNum;

	//
	// Proxy Server box
	//
	tempStr = windowSystem->useSettings->GetStringEntryValue("Network", "UseProxy");
	if (tempStr.CompareNoCase("Yes") == 0)
	{
		proxyCheck->SetValue(B_CONTROL_ON);
		proxyEdit->SetEnabled(true);
	}
	else
	{
		proxyCheck->SetValue(B_CONTROL_OFF);
		proxyEdit->SetEnabled(false);
	}

	tempStr = windowSystem->useSettings->GetStringEntryValue("Network", "ProxyAddress");
	proxyEdit->SetText((strPtr = tempStr.GetString()));
	tempStr.FreeBuffer(strPtr);

	//
	// Updates box
	//
	tempNum = windowSystem->useSettings->GetIntEntryValue("Network", "UpdateCheck");
	checkPop->ItemAt(tempNum)->SetMarked(true);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Network", "DownloadPath");
	downloadEdit->SetText((strPtr = tempStr.GetString()));
	tempStr.FreeBuffer(strPtr);
}



/******************************************************************************/
/* RememberSettings() will read the data from controls which hasn't stored    */
/*      they values yet, eg. edit controls.                                   */
/******************************************************************************/
void APViewNetwork::RememberSettings(void)
{
	windowSystem->useSettings->WriteStringEntryValue("Network", "ProxyAddress", proxyEdit->Text());
	windowSystem->useSettings->WriteStringEntryValue("Network", "DownloadPath", downloadEdit->Text());
}



/******************************************************************************/
/* CancelSettings() will restore real-time values.                            */
/******************************************************************************/
void APViewNetwork::CancelSettings(void)
{
}



/******************************************************************************/
/* SaveSettings() will save some of the settings in other files.              */
/******************************************************************************/
void APViewNetwork::SaveSettings(void)
{
}



/******************************************************************************/
/* SaveWindowSettings() will save the view specific settings entries.         */
/******************************************************************************/
void APViewNetwork::SaveWindowSettings(void)
{
}



/******************************************************************************/
/* MakeBackup() will make a backup of special settings not stored in the      */
/*      "use" settings.                                                       */
/******************************************************************************/
void APViewNetwork::MakeBackup(void)
{
}



/******************************************************************************/
/* HandleMessage() is called for each message the window don't know.          */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APViewNetwork::HandleMessage(BMessage *msg)
{
	int32 value;

	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Use proxy checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_NET_CHECK_USEPROXY:
		{
			value = (proxyCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Network", "UseProxy", value ? "Yes" : "No");
			proxyEdit->SetEnabled(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Proxy address editbox
		////////////////////////////////////////////////////////////////////////
		case SET_NET_EDIT_PROXYADDRESS:
		{
			windowSystem->useSettings->WriteStringEntryValue("Network", "ProxyAddress", proxyEdit->Text());
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Check for updates list menu
		////////////////////////////////////////////////////////////////////////
		case SET_NET_LIST_CHECKUPDATES:
		{
			if (msg->FindInt32("index", &value) == B_OK)
				windowSystem->useSettings->WriteIntEntryValue("Network", "UpdateCheck", value);

			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Check now button
		////////////////////////////////////////////////////////////////////////
		case SET_NET_BUT_CHECKNOW:
		{
			bool newVersion;

			windowSystem->mainWin->SetSleepCursor();
			newVersion = windowSystem->CheckForUpdates();
			windowSystem->mainWin->SetNormalCursor();

			if (!newVersion)
			{
				PString title, msg;

				// Show error
				title.LoadString(res, IDS_MAIN_TITLE);
				msg.LoadString(res, IDS_UPDATE_NO_UPDATES);

				PAlert alert(title, msg, PAlert::pInfo, PAlert::pOk);
				alert.Show();
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Download path editbox
		////////////////////////////////////////////////////////////////////////
		case SET_NET_EDIT_DOWNLOAD:
		{
			windowSystem->useSettings->WriteStringEntryValue("Network", "DownloadPath", downloadEdit->Text());
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Download path button
		////////////////////////////////////////////////////////////////////////
		case SET_NET_BUT_DOWNLOAD:
		{
			ShowFilePanel(downloadEdit->Text(), &downloadPanel, SET_NET_BUT_DOWNLOAD);
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
					str = path.Path();
					str += P_DIRSLASH_STR;

					switch (id)
					{
						case SET_NET_BUT_DOWNLOAD:
						{
							downloadEdit->SetText((strPtr = str.GetString()));
							str.FreeBuffer(strPtr);
							break;
						}
					}
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
					case SET_NET_BUT_DOWNLOAD:
					{
						delete downloadPanel;
						downloadPanel = NULL;
						break;
					}
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
void APViewNetwork::ShowFilePanel(PString startPath, BFilePanel **panel, int32 id)
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
