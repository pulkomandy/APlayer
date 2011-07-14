/******************************************************************************/
/* APlayer Settings Option View class.                                        */
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
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APViewOptions.h"
#include "APSlider.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewOptions::APViewOptions(MainWindowSystem *system, BRect frame, PString name) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect rect(0.0f, 0.0f, 0.0f, 0.0f);
	PString label;
	char *labelPtr;
	BMessage *message;
	BTextView *textView;
	float w, w1;

	// Remember the arguments
	windowSystem = system;
	res          = system->res;

	// Set the view name
	SetName((labelPtr = name.GetString()));
	name.FreeBuffer(labelPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	//
	// Create the "General" box
	//
	label.LoadString(res, IDS_SET_OPT_GENERAL);
	generalBox = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	generalBox->SetLabel((labelPtr = label.GetString()));
	AddChild(generalBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_JUMP);
	label.LoadString(res, IDS_SET_OPT_JUMP);
	jumpCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(jumpCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_ADDLIST);
	label.LoadString(res, IDS_SET_OPT_ADDLIST);
	addListCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(addListCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_REMEMBERLIST);
	label.LoadString(res, IDS_SET_OPT_REMEMBERLIST);
	rememberListCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(rememberListCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_REMEMBERLISTPOS);
	label.LoadString(res, IDS_SET_OPT_REMEMBERLISTPOS);
	rememberListPositionCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(rememberListPositionCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_REMEMBERMODPOS);
	label.LoadString(res, IDS_SET_OPT_REMEMBERMODPOS);
	rememberModulePositionCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(rememberModulePositionCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_SHOWLISTNUMBER);
	label.LoadString(res, IDS_SET_OPT_SHOWLISTNUMBER);
	showListNumberCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(showListNumberCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_BUTTONHELP);
	label.LoadString(res, IDS_SET_OPT_BUTTONHELP);
	buttonHelpCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(buttonHelpCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_SHOWNAME);
	label.LoadString(res, IDS_SET_OPT_SHOWNAME);
	showNameCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(showNameCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_SETLENGTH);
	label.LoadString(res, IDS_SET_OPT_SETLENGTH);
	setLengthCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(setLengthCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_SCANFILES);
	label.LoadString(res, IDS_SET_OPT_SCANFILES);
	scanFilesCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(scanFilesCheck);
	label.FreeBuffer(labelPtr);

	//
	// Create the "Loading" box
	//
	label.LoadString(res, IDS_SET_OPT_LOADING);
	loadingBox = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	loadingBox->SetLabel((labelPtr = label.GetString()));
	AddChild(loadingBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_DOUBLEBUF);
	label.LoadString(res, IDS_SET_OPT_DOUBLEBUF);
	doubleBufCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	loadingBox->AddChild(doubleBufCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_SLIDER_EARLYLOAD);
	label.LoadString(res, IDS_SET_OPT_EARLYLOAD);
	earlySlider = new APSlider(rect, label, false, message, 1, 9, 0.0f, B_BLOCK_THUMB, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	earlySlider->SetHashMarks(B_HASH_MARKS_TOP);
	earlySlider->SetHashMarkCount(9);
	loadingBox->AddChild(earlySlider);

	errorPop = new BPopUpMenu("");

	message = new BMessage(SET_OPT_LIST_ERROR);
	label.LoadString(res, IDS_SET_OPT_ERRORITEM0);
	labelPtr = label.GetString();
	w = StringWidth(labelPtr);
	errorPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_LIST_ERROR);
	label.LoadString(res, IDS_SET_OPT_ERRORITEM1);
	labelPtr = label.GetString();
	w = max(StringWidth(labelPtr), w);
	errorPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_LIST_ERROR);
	label.LoadString(res, IDS_SET_OPT_ERRORITEM2);
	labelPtr = label.GetString();
	w = max(StringWidth(labelPtr), w);
	errorPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_LIST_ERROR);
	label.LoadString(res, IDS_SET_OPT_ERRORITEM3);
	labelPtr = label.GetString();
	w = max(StringWidth(labelPtr), w);
	errorPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SET_OPT_ERROR);
	labelPtr = label.GetString();
	w1 = StringWidth(labelPtr) + HSPACE;

	errorField = new BMenuField(BRect(0.0f, 0.0f, w + w1 + HSPACE * 8.0f, 1.0f), NULL, labelPtr, errorPop);
	errorField->SetDivider(w1);
	loadingBox->AddChild(errorField);
	label.FreeBuffer(labelPtr);

	//
	// Create the "Playing" box
	//
	label.LoadString(res, IDS_SET_OPT_PLAYING);
	playingBox = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	playingBox->SetLabel((labelPtr = label.GetString()));
	AddChild(playingBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_CHECK_NEVERENDING);
	label.LoadString(res, IDS_SET_OPT_NEVERENDING);
	neverEndingCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	playingBox->AddChild(neverEndingCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_EDIT_NEVERENDING);
	secondsEdit = new BTextControl(BRect(0.0f, 0.0f, StringWidth("8888") + 6.0f, 1.0f), NULL, NULL, NULL, message, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	textView = secondsEdit->TextView();
	textView->SetMaxBytes(3);

	for (int i = 0x20; i < 0x100; i++)
		textView->DisallowChar(i);

	for (int i = 0x30; i < 0x3a; i++)
		textView->AllowChar(i);

	playingBox->AddChild(secondsEdit);

	label.LoadString(res, IDS_SET_OPT_SECONDS);
	secondsText = new BStringView(rect, NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	playingBox->AddChild(secondsText);
	label.FreeBuffer(labelPtr);

	listEndPop = new BPopUpMenu("");

	message = new BMessage(SET_OPT_LIST_LISTEND);
	label.LoadString(res, IDS_SET_OPT_LISTENDITEM0);
	labelPtr = label.GetString();
	w = StringWidth(labelPtr);
	listEndPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_LIST_LISTEND);
	label.LoadString(res, IDS_SET_OPT_LISTENDITEM1);
	labelPtr = label.GetString();
	w = max(StringWidth(labelPtr), w);
	listEndPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_OPT_LIST_LISTEND);
	label.LoadString(res, IDS_SET_OPT_LISTENDITEM2);
	labelPtr = label.GetString();
	w = max(StringWidth(labelPtr), w);
	listEndPop->AddItem(new BMenuItem(labelPtr, message));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SET_OPT_LISTEND);
	labelPtr = label.GetString();
	w1 = StringWidth(labelPtr) + HSPACE;

	listEndField = new BMenuField(BRect(0.0f, 0.0f, w + w1 + HSPACE * 8.0f, 1.0f), NULL, labelPtr, listEndPop);
	listEndField->SetDivider(w1);
	playingBox->AddChild(listEndField);
	label.FreeBuffer(labelPtr);

	// Set and resize all the views
	SetPosAndSize();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewOptions::~APViewOptions(void)
{
}



/******************************************************************************/
/* InitSettings() will read the settings and initialize all the controls.     */
/******************************************************************************/
void APViewOptions::InitSettings(void)
{
	PString tempStr;
	char *strPtr;
	int32 tempNum;

	//
	// Options box
	//
	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "AddJump");
	jumpCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "AddToList");
	addListCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "RememberList");
	if (tempStr.CompareNoCase("Yes") == 0)
	{
		rememberListCheck->SetValue(B_CONTROL_ON);
		rememberListPositionCheck->SetEnabled(true);
		rememberModulePositionCheck->SetEnabled(true);
	}
	else
	{
		rememberListCheck->SetValue(B_CONTROL_OFF);
		rememberListPositionCheck->SetEnabled(false);
		rememberModulePositionCheck->SetEnabled(false);
	}

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "RememberListPosition");
	if (tempStr.CompareNoCase("Yes") == 0)
	{
		rememberListPositionCheck->SetValue(B_CONTROL_ON);

		if (rememberListCheck->Value() == B_CONTROL_ON)
			rememberModulePositionCheck->SetEnabled(true);
		else
			rememberModulePositionCheck->SetEnabled(false);
	}
	else
	{
		rememberListPositionCheck->SetValue(B_CONTROL_OFF);
		rememberModulePositionCheck->SetEnabled(false);
	}

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "RememberModulePosition");
	rememberModulePositionCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "ShowListNumber");
	showListNumberCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "ToolTips");
	buttonHelpCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "ShowNameInTitle");
	showNameCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "SetLength");
	setLengthCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "ScanFiles");
	scanFilesCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	//
	// Loading box
	//
	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "DoubleBuffering");
	if (tempStr.CompareNoCase("Yes") == 0)
	{
		doubleBufCheck->SetValue(B_CONTROL_ON);
		earlySlider->SetEnabled(true);
	}
	else
	{
		doubleBufCheck->SetValue(B_CONTROL_OFF);
		earlySlider->SetEnabled(false);
	}

	tempNum = windowSystem->useSettings->GetIntEntryValue("Options", "EarlyLoad");
	earlySlider->SetValue(tempNum);

	tempNum = windowSystem->useSettings->GetIntEntryValue("Options", "ModuleError");
	errorPop->ItemAt(tempNum)->SetMarked(true);

	//
	// Playing box
	//
	tempStr = windowSystem->useSettings->GetStringEntryValue("Options", "NeverEnding");
	if (tempStr.CompareNoCase("Yes") == 0)
	{
		neverEndingCheck->SetValue(B_CONTROL_ON);
		secondsEdit->SetEnabled(true);
	}
	else
	{
		neverEndingCheck->SetValue(B_CONTROL_OFF);
		secondsEdit->SetEnabled(false);
	}

	tempNum = windowSystem->useSettings->GetIntEntryValue("Options", "NeverEndingTimeout");
	tempStr = PString::CreateNumber(tempNum);
	secondsEdit->SetText((strPtr = tempStr.GetString()));
	tempStr.FreeBuffer(strPtr);

	tempNum = windowSystem->useSettings->GetIntEntryValue("Options", "ModuleListEnd");
	listEndPop->ItemAt(tempNum)->SetMarked(true);
}



/******************************************************************************/
/* RememberSettings() will read the data from controls which hasn't stored    */
/*      they values yet, eg. edit controls.                                   */
/******************************************************************************/
void APViewOptions::RememberSettings(void)
{
	int32 value;

	// Never ending timeout value
	value = atoi(secondsEdit->Text());
	windowSystem->useSettings->WriteIntEntryValue("Options", "NeverEndingTimeout", value);
}



/******************************************************************************/
/* CancelSettings() will restore real-time values.                            */
/******************************************************************************/
void APViewOptions::CancelSettings(void)
{
	bool temp;

	// Restore list number showing
	temp = windowSystem->useSettings->GetStringEntryValue("Options", "ShowListNumber").CompareNoCase("Yes") == 0;
	windowSystem->mainWin->EnableListNumber(temp);

	// Restore tool tips
	temp = windowSystem->useSettings->GetStringEntryValue("Options", "ToolTips").CompareNoCase("Yes") == 0;
	windowSystem->mainWin->EnableToolTips(temp);

	// Restore the module name in title
	temp = windowSystem->useSettings->GetStringEntryValue("Options", "ShowNameInTitle").CompareNoCase("Yes") == 0;
	windowSystem->mainWin->EnableShowNameInTitlebar(temp);
}



/******************************************************************************/
/* SaveSettings() will save some of the settings in other files.              */
/******************************************************************************/
void APViewOptions::SaveSettings(void)
{
}



/******************************************************************************/
/* SaveWindowSettings() will save the view specific settings entries.         */
/******************************************************************************/
void APViewOptions::SaveWindowSettings(void)
{
}



/******************************************************************************/
/* MakeBackup() will make a backup of special settings not stored in the      */
/*      "use" settings.                                                       */
/******************************************************************************/
void APViewOptions::MakeBackup(void)
{
}



/******************************************************************************/
/* HandleMessage() is called for each message the window don't know.          */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APViewOptions::HandleMessage(BMessage *msg)
{
	int32 value;

	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Jump To Added Module checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_JUMP:
		{
			value = (jumpCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "AddJump", value ? "Yes" : "No");
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Add to list as default checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_ADDLIST:
		{
			value = (addListCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "AddToList", value ? "Yes" : "No");
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Remember list checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_REMEMBERLIST:
		{
			value = (rememberListCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "RememberList", value ? "Yes" : "No");
			rememberListPositionCheck->SetEnabled(value);

			if (value && (rememberListPositionCheck->Value() == B_CONTROL_ON))
				rememberModulePositionCheck->SetEnabled(true);
			else
				rememberModulePositionCheck->SetEnabled(false);

			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Remember list position checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_REMEMBERLISTPOS:
		{
			value = (rememberListPositionCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "RememberListPosition", value ? "Yes" : "No");
			rememberModulePositionCheck->SetEnabled(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Remember module position checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_REMEMBERMODPOS:
		{
			value = (rememberModulePositionCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "RememberModulePosition", value ? "Yes" : "No");
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Show list number checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_SHOWLISTNUMBER:
		{
			value = (showListNumberCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "ShowListNumber", value ? "Yes" : "No");
			windowSystem->mainWin->EnableListNumber(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Tool tips checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_BUTTONHELP:
		{
			value = (buttonHelpCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "ToolTips", value ? "Yes" : "No");
			windowSystem->mainWin->EnableToolTips(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Show name checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_SHOWNAME:
		{
			value = (showNameCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "ShowNameInTitle", value ? "Yes" : "No");
			windowSystem->mainWin->EnableShowNameInTitlebar(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Set length checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_SETLENGTH:
		{
			value = (setLengthCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "SetLength", value ? "Yes" : "No");
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Scan files checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_SCANFILES:
		{
			value = (scanFilesCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "ScanFiles", value ? "Yes" : "No");
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Set double buffering checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_DOUBLEBUF:
		{
			value = (doubleBufCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "DoubleBuffering", value ? "Yes" : "No");
			earlySlider->SetEnabled(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Set early load slider
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_SLIDER_EARLYLOAD:
		{
			value = earlySlider->Value();
			windowSystem->useSettings->WriteIntEntryValue("Options", "EarlyLoad", value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Module error list menu
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_LIST_ERROR:
		{
			if (msg->FindInt32("index", &value) == B_OK)
				windowSystem->useSettings->WriteIntEntryValue("Options", "ModuleError", value);

			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Set never ending checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_CHECK_NEVERENDING:
		{
			value = (neverEndingCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Options", "NeverEnding", value ? "Yes" : "No");
			secondsEdit->SetEnabled(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Set never ending timeout editbox
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_EDIT_NEVERENDING:
		{
			value = atoi(secondsEdit->Text());
			windowSystem->useSettings->WriteIntEntryValue("Options", "NeverEndingTimeout", value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// End of module list menu
		////////////////////////////////////////////////////////////////////////
		case SET_OPT_LIST_LISTEND:
		{
			if (msg->FindInt32("index", &value) == B_OK)
				windowSystem->useSettings->WriteIntEntryValue("Options", "ModuleListEnd", value);

			break;
		}
	}
}



/******************************************************************************/
/* SetPosAndSize() will calculate the positions and sizes for all views.      */
/******************************************************************************/
void APViewOptions::SetPosAndSize(void)
{
	float x, y, w, h;
	float controlWidth, controlHeight;
	float temp, boxY;
	BRect rect;
	font_height fh;
	float fontHeight;

	// Find view bounds
	rect = Bounds();

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// General box
	//
	// Column 1
	jumpCheck->GetPreferredSize(&controlWidth, &controlHeight);

	addListCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	rememberListCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	rememberListPositionCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w + HSPACE * 2.0f);

	rememberModulePositionCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w + HSPACE * 2.0f);

	showListNumberCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	y = fontHeight + VSPACE;
	jumpCheck->MoveTo(HSPACE, y);
	jumpCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	addListCheck->MoveTo(HSPACE, y);
	addListCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	rememberListCheck->MoveTo(HSPACE, y);
	rememberListCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	rememberListPositionCheck->MoveTo(HSPACE * 3.0f, y);
	rememberListPositionCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	rememberModulePositionCheck->MoveTo(HSPACE * 3.0f, y);
	rememberModulePositionCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	showListNumberCheck->MoveTo(HSPACE, y);
	showListNumberCheck->ResizeTo(controlWidth, controlHeight);

	// Column 2
	x = controlWidth + HSPACE * 4.0f;
	y = fontHeight + VSPACE;

	buttonHelpCheck->GetPreferredSize(&controlWidth, &controlHeight);

	showNameCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	buttonHelpCheck->MoveTo(x, y);
	buttonHelpCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	showNameCheck->MoveTo(x, y);
	showNameCheck->ResizeTo(controlWidth, controlHeight);

	// Column 3
	x += controlWidth + HSPACE * 4.0f;
	y  = fontHeight + VSPACE;

	setLengthCheck->GetPreferredSize(&controlWidth, &controlHeight);

	scanFilesCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	setLengthCheck->MoveTo(x, y);
	setLengthCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	scanFilesCheck->MoveTo(x, y);
	scanFilesCheck->ResizeTo(controlWidth, controlHeight);

	// The box
	h = fontHeight + controlHeight * 6.0f + VSPACE * 7.0f;
	generalBox->MoveTo(0.0f, 0.0f);
	generalBox->ResizeTo(rect.Width(), h);

	//
	// Loading box
	//
	y = fontHeight + VSPACE;
	doubleBufCheck->GetPreferredSize(&controlWidth, &controlHeight);
	doubleBufCheck->MoveTo(HSPACE, y);
	doubleBufCheck->ResizeTo(controlWidth, controlHeight);

	earlySlider->GetPreferredSize(&w, &temp);
	y += (controlHeight + VSPACE);
	earlySlider->MoveTo(HSPACE, y);
	earlySlider->ResizeTo(rect.Width() - HSPACE * 2.0f, temp);

	y += (controlHeight + VSPACE);
	errorField->MoveTo(HSPACE, y);

	boxY = h + VSPACE * 2.0f;
	h = fontHeight + controlHeight * 2.2f + temp + 2.0f + VSPACE * 4.0f;
	loadingBox->MoveTo(0.0f, boxY);
	loadingBox->ResizeTo(rect.Width(), h);

	//
	// Playing box
	//
	y = fontHeight + VSPACE;
	neverEndingCheck->GetPreferredSize(&controlWidth, &controlHeight);
	neverEndingCheck->MoveTo(HSPACE, y);
	neverEndingCheck->ResizeTo(controlWidth, controlHeight);

	x = controlWidth + HSPACE * 2.0f;
	secondsEdit->GetPreferredSize(&controlWidth, &controlHeight);
	secondsEdit->MoveTo(x, y);
	secondsEdit->ResizeTo(controlWidth, controlHeight);

	x += (controlWidth + HSPACE);
	secondsText->MoveTo(x, y + ceil(fh.descent + fh.leading) + 1.0f);
	controlWidth = StringWidth(secondsText->Text());
	secondsText->ResizeTo(controlWidth, fontHeight);

	y += (controlHeight + VSPACE);
	listEndField->MoveTo(HSPACE, y);

	boxY += h + VSPACE * 2.0f;
	h = fontHeight + controlHeight * 2.0f + 2.0f + VSPACE * 3.0f;
	playingBox->MoveTo(0.0f, boxY);
	playingBox->ResizeTo(rect.Width(), h);

	// Spread out the boxes
	boxY += h;
	temp = ceil((SET_MIN_VIEW_HEIGHT - boxY) / 2.0f);

	loadingBox->MoveBy(0.0f, temp);
	playingBox->MoveTo(0.0f, SET_MIN_VIEW_HEIGHT - h);
}
