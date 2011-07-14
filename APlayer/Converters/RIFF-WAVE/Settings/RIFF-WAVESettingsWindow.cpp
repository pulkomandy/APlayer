/******************************************************************************/
/* RIFFWAVESettingsWindow class.                                              */
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

// Server headers
#include "RIFF-WAVESettingsWindow.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Extern global functions and variables                                      */
/******************************************************************************/
extern PSettings *useSettings;
extern PSettings *saveSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
RIFFWAVESettingsWindow::RIFFWAVESettingsWindow(PResource *resource, PString title) : BWindow(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS)
{
	BRect rect;
	PString label;
	char *labelPtr, *titlePtr;
	float titWidth, winWidth;
	float controlWidth, controlHeight;
	float w, w1, x, y;
	BMessage *message;
	font_height fh;
	float fontHeight;

	// Remember the config information
	res = resource;

	// Set the window title
	SetTitle((titlePtr = title.GetString()));

	// Create background view
	rect    = Bounds();
	topView = new BView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	// Change the color to light grey
	topView->SetViewColor(BeBackgroundGrey);

	// Add view to the window
	AddChild(topView);

	// Find font height
	topView->GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Create the listbox with the different formats
	formatPop = new BPopUpMenu("");

	label.LoadString(res, IDS_RIFFWAVE_FORMAT_MSADPCM);
	labelPtr = label.GetString();
	w = topView->StringWidth(labelPtr);
	formatPop->AddItem(new BMenuItem(labelPtr, NULL));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_RIFFWAVE_FORMAT_PCM);
	labelPtr = label.GetString();
	w = max(topView->StringWidth(labelPtr), w);
	formatPop->AddItem(new BMenuItem(labelPtr, NULL));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_RIFFWAVE_FORMAT);
	labelPtr = label.GetString();
	w1 = topView->StringWidth(labelPtr) + HSPACE;

	formatField = new BMenuField(BRect(HSPACE, VSPACE, HSPACE + w + w1 + HSPACE * 8.0f, VSPACE + 1.0f), NULL, labelPtr, formatPop);
	formatField->SetDivider(w1);
	topView->AddChild(formatField);
	label.FreeBuffer(labelPtr);

	// Find the width of the window
	titWidth = topView->StringWidth(titlePtr);
	winWidth = max(titWidth + HSPACE * 20.0f, w + w1 + HSPACE * 8.0f);
	title.FreeBuffer(titlePtr);

	// Add the "save" button
	y = fontHeight + VSPACE * 5.0f;
	x = winWidth - HSPACE;
	message = new BMessage(APCFG_SAVE);
	label.LoadString(res, IDS_RIFFWAVE_SAVE);
	saveButton = new BButton(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	label.FreeBuffer(labelPtr);
	saveButton->GetPreferredSize(&controlWidth, &controlHeight);
	x -= controlWidth;
	ResizeTo(winWidth, y + controlHeight + VSPACE);

	saveButton->MoveTo(x, y);
	saveButton->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(saveButton);

	// Add the "Use" button
	message = new BMessage(APCFG_USE);
	label.LoadString(res, IDS_RIFFWAVE_USE);
	useButton = new BButton(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	label.FreeBuffer(labelPtr);
	useButton->GetPreferredSize(&controlWidth, &controlHeight);
	x -= (controlWidth + HSPACE);
	useButton->MoveTo(x, y);
	useButton->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(useButton);

	// Set the settings in the controls
	GetSettings();

	// Does any window position exists in the settings
	if (useSettings->EntryExist("Window", "WinX") && useSettings->EntryExist("Window", "WinY"))
	{
		// Yes, now set the new window positions
		x = useSettings->GetIntEntryValue("Window", "WinX");
		y = useSettings->GetIntEntryValue("Window", "WinY");
		MoveTo(x, y);
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
RIFFWAVESettingsWindow::~RIFFWAVESettingsWindow(void)
{
}



/******************************************************************************/
/* Quit() is called when the window closes.                                   */
/******************************************************************************/
void RIFFWAVESettingsWindow::Quit(void)
{
	// Remember the settings
	RememberSettings(useSettings);

	// Store the window positions if changed
	CheckAndWriteWindowPositions();

	// Call the base class
	BWindow::Quit();
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void RIFFWAVESettingsWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Save the settings on disk
		////////////////////////////////////////////////////////////////////////
		case APCFG_SAVE:
		{
			// Copy the settings into the save object
			RememberSettings(saveSettings);

			// Check the window positions
			CheckAndWriteWindowPositions();

			// And now save the file
			saveSettings->SaveFile("RIFF-WAVE.ini", "Polycode", "APlayer");
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Just use the current settings
		////////////////////////////////////////////////////////////////////////
		case APCFG_USE:
		{
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Default handler
		////////////////////////////////////////////////////////////////////////
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}



/******************************************************************************/
/* GetSettings() will get all the settings and initialize the controls.       */
/******************************************************************************/
void RIFFWAVESettingsWindow::GetSettings(void)
{
	PString tempStr;
	char *tempPoi;
	BMenuItem *item;

	// Set the output format
	tempStr = useSettings->GetStringEntryValue("General", "OutputFormat");
	tempPoi = tempStr.GetString();
	item    = formatPop->FindItem(tempPoi);
	if (item != NULL)
		item->SetMarked(true);

	tempStr.FreeBuffer(tempPoi);
}



/******************************************************************************/
/* CheckAndWriteWindowPositions() will check the current window positions     */
/*      against the save settings, and if changed, it will write the new      */
/*      positions in the settings.                                            */
/******************************************************************************/
void RIFFWAVESettingsWindow::CheckAndWriteWindowPositions(void)
{
	BRect winPos;
	int32 x, y;

	// Store the window position and size if changed
	winPos = Frame();

	x = (int32)winPos.left;
	y = (int32)winPos.top;

	// Write the window position in the settings if they have changed
	if ((saveSettings->GetIntEntryValue("Window", "WinX") != x) || (saveSettings->GetIntEntryValue("Window", "WinY") != y))
	{
		saveSettings->WriteIntEntryValue("Window", "WinX", x);
		saveSettings->WriteIntEntryValue("Window", "WinY", y);
	}
}



/******************************************************************************/
/* RememberSettings() will store all the settings in the settings object      */
/*      given as the argument.                                                */
/*                                                                            */
/* Input:  "settings" is where to store the settings.                         */
/******************************************************************************/
void RIFFWAVESettingsWindow::RememberSettings(PSettings *settings)
{
	BMenuItem *item;

	item = formatPop->FindMarked();
	if (item != NULL)
		settings->WriteStringEntryValue("General", "OutputFormat", item->Label());
}
