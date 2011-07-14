/******************************************************************************/
/* DiskSaver View class.                                                      */
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
#include "PAlert.h"
#include "PSettings.h"
#include "PDirectory.h"
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "Layout.h"

// Agent headers
#include "DiskSaverView.h"
#include "DiskSaverDiskButton.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *diskSaverSettings;



/******************************************************************************/
/* DiskSaverFilter class                                                      */
/******************************************************************************/

/******************************************************************************/
/* Filter() is called for each entry read from the file panel.                */
/*                                                                            */
/* Input:  Different types of the entry.                                      */
/*                                                                            */
/* Output: True if the entry is valid, else false.                            */
/******************************************************************************/
bool DiskSaverFilter::Filter(const entry_ref *ref, BNode *node, struct stat *st, const char *filetype)
{
	if (S_ISDIR(st->st_mode))
		return (true);

	return (false);
}





/******************************************************************************/
/* DiskSaverViewGrey class                                                    */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
DiskSaverViewGrey::DiskSaverViewGrey(BRect frame) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);
}





/******************************************************************************/
/* DiskSaverView class                                                        */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
DiskSaverView::DiskSaverView(APGlobalData *global, PResource *resource) : BView(BRect(0.0f, 0.0f, OUTPUTAGENT_MAX_VIEW_WIDTH, OUTPUTAGENT_MAX_VIEW_HEIGHT - VSPACE), NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect rect;
	BMessage *message;
	BTextView *textView;
	BMenuItem *item;
	PString label;
	char *labelPtr, *nameStr, *tempPtr;
	font_height fh;
	float fontHeight;
	float controlWidth, controlHeight;
	float x, y, w, w1, h, y1;
	float maxWidth;
	int32 i, count;
	APList<APAddOnInformation *> formatAddOnList;
	APList<APAddOnInformation *> passAddOnList;
	APAddOnInformation *info;
	PString tempStr;
	int32 num;

	// Initialize member variables
	globalData      = global;
	res             = resource;
	converter       = NULL;
	converterWindow = NULL;
	messenger       = NULL;
	panel           = NULL;

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Create all the buttons etc.
	rect.left   = HSPACE;
	rect.right  = OUTPUTAGENT_MAX_VIEW_WIDTH - HSPACE;
	rect.top    = VSPACE;
	rect.bottom = OUTPUTAGENT_MAX_VIEW_HEIGHT - VSPACE;

	y = VSPACE * 3.0f;
	label.LoadString(res, IDS_DISKSAVER_PATH);
	labelPtr = label.GetString();
	x = rect.left + StringWidth(labelPtr);
	w = rect.Width() - (PICHSIZE + HSPACE) - x;

	message  = new BMessage(DS_EDIT_PATH);
	pathEdit = new BTextControl(BRect(x, y, x + w, y), NULL, NULL, NULL, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER, B_WILL_DRAW | B_NAVIGABLE);
	pathEdit->GetPreferredSize(&controlWidth, &controlHeight);

	controlHeight = max(controlHeight, fontHeight + VSPACE);

	pathEdit->ResizeTo(controlWidth, controlHeight);
	textView = pathEdit->TextView();
	textView->SetMaxBytes(B_PATH_NAME_LENGTH);
	AddChild(pathEdit);

	x  = rect.Width() - PICHSIZE;
	y1 = y + ((controlHeight - fontHeight) / 2.0f);

	message    = new BMessage(DS_BUT_PATH);
	pathButton = new DiskSaverDiskButton(res, IDI_DISKSAVER_DISK_NORMAL, IDI_DISKSAVER_DISK_PRESSED, BRect(x, y, x + PICHSIZE + HSPACE, y + controlHeight), message);
	AddChild(pathButton);

	pathText = new BStringView(BRect(rect.left, y1, rect.left + StringWidth(labelPtr), y1 + fontHeight), NULL, labelPtr, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	AddChild(pathText);
	label.FreeBuffer(labelPtr);

	y += controlHeight + VSPACE * 2.0f;

	grey1View = new DiskSaverViewGrey(BRect(rect.left, y, rect.left, y));
	AddChild(grey1View);

	message = new BMessage(DS_RADIO_8BIT);
	label.LoadString(res, IDS_DISKSAVER_8BIT);
	bit8Radio = new BRadioButton(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, (labelPtr = label.GetString()), message);
	bit8Radio->GetPreferredSize(&controlWidth, &controlHeight);
	bit8Radio->ResizeTo(controlWidth, controlHeight);
	maxWidth = controlWidth;
	grey1View->AddChild(bit8Radio);
	label.FreeBuffer(labelPtr);

	message = new BMessage(DS_RADIO_16BIT);
	label.LoadString(res, IDS_DISKSAVER_16BIT);
	bit16Radio = new BRadioButton(BRect(0.0f, controlHeight + VSPACE, 0.0f, controlHeight + VSPACE), NULL, (labelPtr = label.GetString()), message);
	bit16Radio->GetPreferredSize(&controlWidth, &controlHeight);
	bit16Radio->ResizeTo(controlWidth, controlHeight);
	maxWidth = max(maxWidth, controlWidth);
	grey1View->AddChild(bit16Radio);
	label.FreeBuffer(labelPtr);

	h = controlHeight * 2.0f + VSPACE;
	grey1View->ResizeTo(maxWidth, h);

	x = rect.left + maxWidth + HSPACE * 8.0f;
	grey2View = new DiskSaverViewGrey(BRect(x, y, x, y));
	AddChild(grey2View);

	message = new BMessage(DS_RADIO_MONO);
	label.LoadString(res, IDS_DISKSAVER_MONO);
	monoRadio = new BRadioButton(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, (labelPtr = label.GetString()), message);
	monoRadio->GetPreferredSize(&controlWidth, &controlHeight);
	monoRadio->ResizeTo(controlWidth, controlHeight);
	maxWidth = controlWidth;
	grey2View->AddChild(monoRadio);
	label.FreeBuffer(labelPtr);

	message = new BMessage(DS_RADIO_STEREO);
	label.LoadString(res, IDS_DISKSAVER_STEREO);
	stereoRadio = new BRadioButton(BRect(0.0f, controlHeight + VSPACE, 0.0f, controlHeight + VSPACE), NULL, (labelPtr = label.GetString()), message);
	stereoRadio->GetPreferredSize(&controlWidth, &controlHeight);
	stereoRadio->ResizeTo(controlWidth, controlHeight);
	w1 = max(maxWidth, controlWidth);
	grey2View->AddChild(stereoRadio);
	label.FreeBuffer(labelPtr);

	grey2View->ResizeTo(w1, h);

	formatPop = new BPopUpMenu("");

	// Add the converter names into the format pop-up box
	globalData->GetAddOnList(apConverter, formatAddOnList);
	maxWidth = 0.0f;

	count = formatAddOnList.CountItems();
	for (i = 0; i < count; i++)
	{
		info = formatAddOnList.GetItem(i);

		if (info->pluginFlags & apcSaver)
		{
			message = new BMessage(DS_LIST_FORMAT);

			if (info->pluginFlags & apcSaverSettings)
				message->AddBool("settings", true);

			// Check for support of the different output formats
			if (info->pluginFlags & apcSupport8Bit)
				message->AddBool("8bit", true);

			if (info->pluginFlags & apcSupport16Bit)
				message->AddBool("16bit", true);

			if (info->pluginFlags & apcSupportMono)
				message->AddBool("mono", true);

			if (info->pluginFlags & apcSupportStereo)
				message->AddBool("stereo", true);

			nameStr = info->name.GetString();
			maxWidth = max(maxWidth, StringWidth(nameStr));
			formatPop->AddItem(new BMenuItem(nameStr, message));
			info->name.FreeBuffer(nameStr);
		}
	}

	label.LoadString(res, IDS_DISKSAVER_FORMAT);
	labelPtr = label.GetString();
	w = StringWidth(labelPtr) + HSPACE;

	x += w1 + HSPACE * 8.0f;
	formatField = new BMenuField(BRect(x, y, x + w + maxWidth + HSPACE * 8.0f, y), NULL, labelPtr, formatPop);
	formatField->SetDivider(w);
	AddChild(formatField);
	label.FreeBuffer(labelPtr);
	w += maxWidth;

	// Create the settings button
	label.LoadString(res, IDS_DISKSAVER_SETTINGS);
	labelPtr = label.GetString();
	message = new BMessage(DS_BUT_SETTINGS);
	formatSettings = new BButton(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, labelPtr, message, B_FOLLOW_BOTTOM);
	formatSettings->ResizeToPreferred();
	formatSettings->MoveTo(x, y + fontHeight + VSPACE * 3.0f);
	AddChild(formatSettings);
	label.FreeBuffer(labelPtr);

	// Add the sound output agents into the "pass through" pop-up box
	passPop = new BPopUpMenu("");

	// First add the "none" string
	label.LoadString(res, IDS_DISKSAVER_NONE);
	labelPtr = label.GetString();
	message = new BMessage(DS_LIST_PASS);
	maxWidth = max(maxWidth, StringWidth(labelPtr));
	passPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	// Now add all the output agents
	tempStr.LoadString(res, IDS_DISKSAVER_NAME);
	globalData->GetAddOnList(apAgent, passAddOnList);
	maxWidth = 0.0f;

	count = passAddOnList.CountItems();
	for (i = 0; i < count; i++)
	{
		info = passAddOnList.GetItem(i);

		if (info->pluginFlags & apaSoundOutput)
		{
			// Do not add ourselves
			if (info->name != tempStr)
			{
				message = new BMessage(DS_LIST_PASS);
				nameStr = info->name.GetString();
				maxWidth = max(maxWidth, StringWidth(nameStr));
				passPop->AddItem(new BMenuItem(nameStr, message));
				info->name.FreeBuffer(nameStr);
			}
		}
	}

	// Free the list again
	globalData->FreeAddOnList(passAddOnList);

	label.LoadString(res, IDS_DISKSAVER_PASSTHROUGH);
	labelPtr = label.GetString();
	w1 = StringWidth(labelPtr) + HSPACE;

	x += w + HSPACE * 8.0f;
	passField = new BMenuField(BRect(x, y, x + w1 + maxWidth + HSPACE * 8.0f, y), NULL, labelPtr, passPop);
	passField->SetDivider(w1);
	AddChild(passField);
	label.FreeBuffer(labelPtr);

	// Set the controls to the settings
	//
	// Disk path
	tempStr = diskSaverSettings->GetStringEntryValue("General", "DiskPath");
	pathEdit->SetText((tempPtr = tempStr.GetString()));
	tempStr.FreeBuffer(tempPtr);

	// Output size
	num = diskSaverSettings->GetIntEntryValue("General", "OutputSize");
	if (num == CV_OUTPUTSIZE_8BIT)
		bit8Radio->SetValue(B_CONTROL_ON);
	else
		bit16Radio->SetValue(B_CONTROL_ON);

	// Output type
	num = diskSaverSettings->GetIntEntryValue("General", "OutputType");
	if (num == CV_OUTPUTTYPE_MONO)
		monoRadio->SetValue(B_CONTROL_ON);
	else
		stereoRadio->SetValue(B_CONTROL_ON);

	// Output format
	tempStr = diskSaverSettings->GetStringEntryValue("General", "OutputFormat");
	item = formatPop->FindItem((tempPtr = tempStr.GetString()));
	if (item != NULL)
	{
		item->SetMarked(true);

		// Find the add-on and enable/disable the settings button
		count = formatAddOnList.CountItems();
		for (i = 0; i < count; i++)
		{
			info = formatAddOnList.GetItem(i);

			if (info->name == tempStr)
			{
				if (info->pluginFlags & apcSaverSettings)
					formatSettings->SetEnabled(true);
				else
					formatSettings->SetEnabled(false);

				if (info->pluginFlags & apcSupport8Bit)
					bit8Radio->SetEnabled(true);
				else
					bit8Radio->SetEnabled(false);

				if (info->pluginFlags & apcSupport16Bit)
					bit16Radio->SetEnabled(true);
				else
					bit16Radio->SetEnabled(false);

				if (info->pluginFlags & apcSupportMono)
					monoRadio->SetEnabled(true);
				else
					monoRadio->SetEnabled(false);

				if (info->pluginFlags & apcSupportStereo)
					stereoRadio->SetEnabled(true);
				else
					stereoRadio->SetEnabled(false);

				if ((bit16Radio->Value() == B_CONTROL_ON) && (!bit16Radio->IsEnabled()))
					bit8Radio->SetValue(B_CONTROL_ON);

				if ((bit8Radio->Value() == B_CONTROL_ON) && (!bit8Radio->IsEnabled()))
					bit16Radio->SetValue(B_CONTROL_ON);

				if ((stereoRadio->Value() == B_CONTROL_ON) && (!stereoRadio->IsEnabled()))
					monoRadio->SetValue(B_CONTROL_ON);

				if ((monoRadio->Value() == B_CONTROL_ON) && (!monoRadio->IsEnabled()))
					stereoRadio->SetValue(B_CONTROL_ON);

				break;
			}
		}
	}
	else
		formatSettings->SetEnabled(false);

	tempStr.FreeBuffer(tempPtr);

	// Pass through
	tempStr = diskSaverSettings->GetStringEntryValue("General", "OutputAgent");
	item = passPop->FindItem((tempPtr = tempStr.GetString()));
	if (item != NULL)
	{
		// Found the agent, select it
		item->SetMarked(true);

		// Disable the radio buttons
		monoRadio->SetEnabled(false);
		stereoRadio->SetEnabled(false);
	}
	else
	{
		// Couldn't find the agent, select none
		item = passPop->ItemAt(0);
		item->SetMarked(true);
	}

	tempStr.FreeBuffer(tempPtr);

	// Free the format add-on list again
	globalData->FreeAddOnList(formatAddOnList);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
DiskSaverView::~DiskSaverView(void)
{
	// Have we previously opened a settings window?
	if (converterWindow != NULL)
	{
		thread_info info;

		// Yes, is it still valid?
		if (get_thread_info(converterThread, &info) == B_OK)
		{
			// It is, tell the window to quit
			converterWindow->Lock();
			converterWindow->Quit();
		}

		// Now delete the converter instance
		globalData->DeleteAddOnInstance(converterName, apConverter, converter);
	}

	delete panel;
	delete messenger;
}



/******************************************************************************/
/* MessageReceived() handles all the messages we have defined.                */
/*                                                                            */
/* Input:  "message" is the message.                                          */
/******************************************************************************/
void DiskSaverView::MessageReceived(BMessage *message)
{
	int32 value;

	switch (message->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Path editbox
		////////////////////////////////////////////////////////////////////////
		case DS_EDIT_PATH:
		{
			diskSaverSettings->WriteStringEntryValue("General", "DiskPath", pathEdit->Text());
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Path disk button
		////////////////////////////////////////////////////////////////////////
		case DS_BUT_PATH:
		{
			PString str;
			char *strPtr;

			// Start to allocate the messenger
			if (messenger == NULL)
			{
				messenger = new BMessenger(this);
				if (messenger == NULL)
					throw PMemoryException();
			}

			// Allocate the panel
			if (panel == NULL)
			{
				panel = new BFilePanel(B_OPEN_PANEL, messenger, NULL, B_DIRECTORY_NODE, false, NULL, &filter);
				if (panel == NULL)
					throw PMemoryException();
			}

			// Change the start entry
			str = PDirectory::GetDirectoryPart(pathEdit->Text());
			panel->SetPanelDirectory((strPtr = str.GetString()));
			str.FreeBuffer(strPtr);

			// Change the look of the panel
			str.LoadString(res, IDS_DISKSAVER_SELECT);
			panel->SetButtonLabel(B_DEFAULT_BUTTON, (strPtr = str.GetString()));
			str.FreeBuffer(strPtr);

			// Show the panel
			panel->Show();
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
			PString str;
			char *strPtr;

			if (message->FindRef("refs", 0, &entRef) == B_OK)
			{
				entry.SetTo(&entRef);
				entry.GetPath(&path);

				// Append slash
				str  = path.Path();
				str += P_DIRSLASH_STR;

				// Store the path in the edit box
				pathEdit->SetText((strPtr = str.GetString()));
				str.FreeBuffer(strPtr);

				// Write the path to the settings
				diskSaverSettings->WriteStringEntryValue("General", "DiskPath", str);
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// 8-bit radio button
		////////////////////////////////////////////////////////////////////////
		case DS_RADIO_8BIT:
		{
			diskSaverSettings->WriteIntEntryValue("General", "OutputSize", CV_OUTPUTSIZE_8BIT);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// 16-bit radio button
		////////////////////////////////////////////////////////////////////////
		case DS_RADIO_16BIT:
		{
			diskSaverSettings->WriteIntEntryValue("General", "OutputSize", CV_OUTPUTSIZE_16BIT);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Mono radio button
		////////////////////////////////////////////////////////////////////////
		case DS_RADIO_MONO:
		{
			diskSaverSettings->WriteIntEntryValue("General", "OutputType", CV_OUTPUTTYPE_MONO);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Stereo radio button
		////////////////////////////////////////////////////////////////////////
		case DS_RADIO_STEREO:
		{
			diskSaverSettings->WriteIntEntryValue("General", "OutputType", CV_OUTPUTTYPE_STEREO);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Output format list menu
		////////////////////////////////////////////////////////////////////////
		case DS_LIST_FORMAT:
		{
			BMenuItem *item;

			value = message->FindInt32("index");
			item  = formatPop->ItemAt(value);
			diskSaverSettings->WriteStringEntryValue("General", "OutputFormat", item->Label());

			if (message->FindBool("settings"))
				formatSettings->SetEnabled(true);
			else
				formatSettings->SetEnabled(false);

			if (message->FindBool("8bit"))
				bit8Radio->SetEnabled(true);
			else
				bit8Radio->SetEnabled(false);

			if (message->FindBool("16bit"))
				bit16Radio->SetEnabled(true);
			else
				bit16Radio->SetEnabled(false);

			if (message->FindBool("mono"))
				monoRadio->SetEnabled(true);
			else
				monoRadio->SetEnabled(false);

			if (message->FindBool("stereo"))
				stereoRadio->SetEnabled(true);
			else
				stereoRadio->SetEnabled(false);

			if ((bit16Radio->Value() == B_CONTROL_ON) && (!bit16Radio->IsEnabled()))
				bit8Radio->SetValue(B_CONTROL_ON);

			if ((bit8Radio->Value() == B_CONTROL_ON) && (!bit8Radio->IsEnabled()))
				bit16Radio->SetValue(B_CONTROL_ON);

			if ((stereoRadio->Value() == B_CONTROL_ON) && (!stereoRadio->IsEnabled()))
				monoRadio->SetValue(B_CONTROL_ON);

			if ((monoRadio->Value() == B_CONTROL_ON) && (!monoRadio->IsEnabled()))
				stereoRadio->SetValue(B_CONTROL_ON);

			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Converter settings button
		////////////////////////////////////////////////////////////////////////
		case DS_BUT_SETTINGS:
		{
			BMenuItem *item;

			// Have we previously opened a settings window?
			if (converterWindow != NULL)
			{
				thread_info info;

				// Yes, is it still valid?
				if (get_thread_info(converterThread, &info) == B_OK)
				{
					// It is, so activate the window
					converterWindow->Activate();
					break;
				}

				// The window has been closed, so delete the converter instance
				globalData->DeleteAddOnInstance(converterName, apConverter, converter);
				converter       = NULL;
				converterWindow = NULL;
				converterName.MakeEmpty();
			}

			// Get the selected converter
			item = formatPop->FindMarked();
			ASSERT(item != NULL);

			// Now create an instance of the converter add-on
			converterName = item->Label();
			converter     = (APAddOnConverter *)globalData->GetAddOnInstance(converterName, apConverter, &converterIndex);
			if (converter == NULL)
			{
				PString title, msg;

				title.LoadString(res, IDS_DISKSAVER_WIN_TITLE);
				msg.Format(res, IDS_DISKSAVER_ERR_NOCONVERTER, item->Label());

				PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
				alert.Show();
				break;
			}

			// And open the settings window
			converterWindow = converter->ShowSaverSettings();
			converterThread = converterWindow->Thread();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Output agent list menu
		////////////////////////////////////////////////////////////////////////
		case DS_LIST_PASS:
		{
			BMenuItem *item;

			value = message->FindInt32("index");
			if (value == 0)
			{
				diskSaverSettings->WriteStringEntryValue("General", "OutputAgent", "");

				// Enable the radio buttons
				monoRadio->SetEnabled(true);
				stereoRadio->SetEnabled(true);
			}
			else
			{
				item = passPop->ItemAt(value);
				diskSaverSettings->WriteStringEntryValue("General", "OutputAgent", item->Label());

				// Disable the radio buttons
				monoRadio->SetEnabled(false);
				stereoRadio->SetEnabled(false);
			}
			break;
		}
	}
}
