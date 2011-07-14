/******************************************************************************/
/* SID View class.                                                            */
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
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Player headers
#include "SIDEmuEngine.h"
#include "SIDViewSlider.h"
#include "SIDViewFormular.h"
#include "SIDDiskButton.h"
#include "SIDView.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Extern global functions and variables                                      */
/******************************************************************************/
extern PSettings *sidSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDView::SIDView(PResource *resource) : BView(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect rect(0.0f, 0.0f, 0.0f, 0.0f);
	BPoint size;
	PString label;
	char *labelPtr;
	font_height fh;
	BMessage *message;
	int32 value;

	// Initialize member variables
	res            = resource;
	panelMessenger = NULL;
	panel          = NULL;

	// Make sure that icons has been loaded
	res->LoadResource(P_RES_SMALL_ICON);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Set the background color to gray
	SetViewColor(BeBackgroundGrey);

	//
	// Create "General Settings" box
	//
	label.LoadString(res, IDS_SID_CFG_GENERALSETTINGS);
	generalBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	generalBox->SetLabel((labelPtr = label.GetString()));
	AddChild(generalBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_CHECK_FILTER);
	label.LoadString(res, IDS_SID_CFG_FILTER);
	filterCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	generalBox->AddChild(filterCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_CHECK_8580);
	label.LoadString(res, IDS_SID_CFG_8580WAVEFORMS);
	mos8580Check = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(mos8580Check);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_CHECK_SPEED);
	label.LoadString(res, IDS_SID_CFG_FORCESPEED);
	forceSpeedCheck = new BCheckBox(rect, NULL, (labelPtr = label.GetString()), message);
	generalBox->AddChild(forceSpeedCheck);
	label.FreeBuffer(labelPtr);

	//
	// Create "MPU Memory Mode" box
	//
	label.LoadString(res, IDS_SID_CFG_MEMORYMODE);
	memoryBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	memoryBox->SetLabel((labelPtr = label.GetString()));
	AddChild(memoryBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_RADIO_FULLBANK);
	label.LoadString(res, IDS_SID_CFG_FULL);
	fullRadio = new BRadioButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	memoryBox->AddChild(fullRadio);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_RADIO_TRANSPARENT);
	label.LoadString(res, IDS_SID_CFG_TRANSPARENT);
	transparentRadio = new BRadioButton(rect, NULL, (labelPtr = label.GetString()), message);
	memoryBox->AddChild(transparentRadio);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_RADIO_PLAYSID);
	label.LoadString(res, IDS_SID_CFG_PLAYSID);
	playSidRadio = new BRadioButton(rect, NULL, (labelPtr = label.GetString()), message);
	memoryBox->AddChild(playSidRadio);
	label.FreeBuffer(labelPtr);

	//
	// Create "C64 Clock Speed" box
	//
	label.LoadString(res, IDS_SID_CFG_SPEED);
	speedBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	speedBox->SetLabel((labelPtr = label.GetString()));
	AddChild(speedBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_RADIO_PAL);
	label.LoadString(res, IDS_SID_CFG_PAL);
	palRadio = new BRadioButton(rect, NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	speedBox->AddChild(palRadio);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_RADIO_NTSC);
	label.LoadString(res, IDS_SID_CFG_NTSC);
	ntscRadio = new BRadioButton(rect, NULL, (labelPtr = label.GetString()), message);
	speedBox->AddChild(ntscRadio);
	label.FreeBuffer(labelPtr);

	//
	// Create "Filter Adjustment" box
	//
	label.LoadString(res, IDS_SID_CFG_FILTERADJUSTMENT);
	filterBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	filterBox->SetLabel((labelPtr = label.GetString()));
	AddChild(filterBox);
	label.FreeBuffer(labelPtr);

	formular = new SIDViewFormular(res, 400.0f, 60.0f, 0.05f);
	filterBox->AddChild(formular);

	message = new BMessage(SID_FILTER_PAR1);
	label.LoadString(res, IDS_SID_CFG_PARAMETER1);
	par1Slider = new SIDViewSliderFilter(formular, 1, rect, label, message, FILTER_PAR1_MAX, FILTER_PAR1_MIN, B_BLOCK_THUMB, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	par1Slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	par1Slider->SetHashMarkCount(9);
	filterBox->AddChild(par1Slider);

	message = new BMessage(SID_FILTER_PAR2);
	label.LoadString(res, IDS_SID_CFG_PARAMETER2);
	par2Slider = new SIDViewSliderFilter(formular, 2, rect, label, message, FILTER_PAR2_MIN, FILTER_PAR2_MAX);
	par2Slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	par2Slider->SetHashMarkCount(9);
	filterBox->AddChild(par2Slider);

	message = new BMessage(SID_FILTER_PAR3);
	label.LoadString(res, IDS_SID_CFG_PARAMETER3);
	par3Slider = new SIDViewSliderFilter(formular, 3, rect, label, message, FILTER_PAR3_MAX * 100, FILTER_PAR3_MIN * 100);
	par3Slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	par3Slider->SetHashMarkCount(9);
	filterBox->AddChild(par3Slider);

	message = new BMessage(SID_FILTER_DEFAULT);
	label.LoadString(res, IDS_SID_CFG_DEFAULT);
	defaultButton = new BButton(rect, NULL, (labelPtr = label.GetString()), message);
	filterBox->AddChild(defaultButton);
	label.FreeBuffer(labelPtr);

	//
	// Create "Panning" box
	//
	label.LoadString(res, IDS_SID_CFG_PANNING);
	panningBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	panningBox->SetLabel((labelPtr = label.GetString()));
	AddChild(panningBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_PANNING_CHAN1);
	label.LoadString(res, IDS_SID_CFG_CHANNEL1);
	chan1Slider = new SIDViewSliderPanning(1, rect, label, message, 0, 255, B_BLOCK_THUMB, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	chan1Slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	chan1Slider->SetHashMarkCount(11);
	panningBox->AddChild(chan1Slider);

	message = new BMessage(SID_PANNING_CHAN2);
	label.LoadString(res, IDS_SID_CFG_CHANNEL2);
	chan2Slider = new SIDViewSliderPanning(2, rect, label, message, 0, 255);
	chan2Slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	chan2Slider->SetHashMarkCount(11);
	panningBox->AddChild(chan2Slider);

	message = new BMessage(SID_PANNING_CHAN3);
	label.LoadString(res, IDS_SID_CFG_CHANNEL3);
	chan3Slider = new SIDViewSliderPanning(3, rect, label, message, 0, 255);
	chan3Slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	chan3Slider->SetHashMarkCount(11);
	panningBox->AddChild(chan3Slider);

	message = new BMessage(SID_PANNING_CHAN4);
	label.LoadString(res, IDS_SID_CFG_CHANNEL4);
	chan4Slider = new SIDViewSliderPanning(4, rect, label, message, 0, 255);
	chan4Slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	chan4Slider->SetHashMarkCount(11);
	panningBox->AddChild(chan4Slider);

	//
	// Create "Misc" box
	//
	label.LoadString(res, IDS_SID_CFG_MISC);
	miscBox = new BBox(rect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	miscBox->SetLabel((labelPtr = label.GetString()));
	AddChild(miscBox);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_DIGI_SCAN);
	label.LoadString(res, IDS_SID_CFG_DIGISCAN);
	digiSlider = new SIDViewSlider(rect, label, message, 0, 20, B_BLOCK_THUMB, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	digiSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	digiSlider->SetHashMarkCount(11);
	digiSlider->UseFillColor(true, &LightBlue);
	miscBox->AddChild(digiSlider);

	label.LoadString(res, IDS_SID_CFG_STILPATH);
	stilText = new BStringView(rect, NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	miscBox->AddChild(stilText);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SID_STIL_PATH);
	stilPath = new BTextControl(rect, NULL, NULL, NULL, message, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);
	miscBox->AddChild(stilPath);

	message = new BMessage(SID_STIL_DISK);
	stilDisk = new SIDDiskButton(res, IDI_SID_DISK_NORMAL, IDI_SID_DISK_PRESSED, rect, message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	miscBox->AddChild(stilDisk);

	// Calculate minimum size
	size = CalcMinSize();

	// Resize to the minimum size
	ResizeTo(size.x, size.y);

	// Set and resize all the views
	SetPosAndSize();

	// Setup "General settings"
	if (sidSettings->GetStringEntryValue("General", "Filter").CompareNoCase("Yes") == 0)
		value = B_CONTROL_ON;
	else
		value = B_CONTROL_OFF;

	filterCheck->SetValue(value);

	if (sidSettings->GetStringEntryValue("General", "MOS8580").CompareNoCase("Yes") == 0)
		value = B_CONTROL_ON;
	else
		value = B_CONTROL_OFF;

	mos8580Check->SetValue(value);

	if (sidSettings->GetStringEntryValue("General", "ForceSongSpeed").CompareNoCase("Yes") == 0)
		value = B_CONTROL_ON;
	else
		value = B_CONTROL_OFF;

	forceSpeedCheck->SetValue(value);

	// Setup "MPU memory mode"
	switch (sidSettings->GetIntEntryValue("MPU", "Memory"))
	{
		case RADIO_MEMORY_FULLBANK:
		{
			fullRadio->SetValue(B_CONTROL_ON);
			break;
		}

		case RADIO_MEMORY_TRANSPARENT:
		default:
		{
			transparentRadio->SetValue(B_CONTROL_ON);
			break;
		}

		case RADIO_MEMORY_PLAYSID:
		{
			playSidRadio->SetValue(B_CONTROL_ON);
			break;
		}
	}

	// Setup "C64 clock speed"
	switch (sidSettings->GetIntEntryValue("Speed", "ClockSpeed"))
	{
		case RADIO_SPEED_PAL:
		default:
		{
			palRadio->SetValue(B_CONTROL_ON);
			break;
		}

		case RADIO_SPEED_NTSC:
		{
			ntscRadio->SetValue(B_CONTROL_ON);
			break;
		}
	}

	// Setup "Filter adjustment"
	value = sidSettings->GetIntEntryValue("Filter", "FilterFs");
	par1Slider->SetValue(value);

	value = sidSettings->GetIntEntryValue("Filter", "FilterFm");
	par2Slider->SetValue(value);

	value = sidSettings->GetIntEntryValue("Filter", "FilterFt");
	par3Slider->SetValue(value);

	// Setup "Panning"
	value = sidSettings->GetIntEntryValue("Panning", "Channel1");
	chan1Slider->SetValue(value);

	value = sidSettings->GetIntEntryValue("Panning", "Channel2");
	chan2Slider->SetValue(value);

	value = sidSettings->GetIntEntryValue("Panning", "Channel3");
	chan3Slider->SetValue(value);

	value = sidSettings->GetIntEntryValue("Panning", "Channel4");
	chan4Slider->SetValue(value);

	// Setup "Misc"
	value = sidSettings->GetIntEntryValue("Misc", "DigiScan");
	digiSlider->SetValue(value);

	label = sidSettings->GetStringEntryValue("Misc", "StilPath");
	stilPath->SetText((labelPtr = label.GetString()));
	label.FreeBuffer(labelPtr);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDView::~SIDView(void)
{
	// Delete the panel
	delete panel;
	delete panelMessenger;
}



/******************************************************************************/
/* MessageReceived() handles all the messages we have defined.                */
/*                                                                            */
/* Input:  "message" is the message.                                          */
/******************************************************************************/
void SIDView::MessageReceived(BMessage *message)
{
	PString strValue;
	int32 value;

	switch (message->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Filter checkbox
		////////////////////////////////////////////////////////////////////////
		case SID_CHECK_FILTER:
		{
			if (filterCheck->Value() == B_CONTROL_ON)
				strValue = "Yes";
			else
				strValue = "No";

			sidSettings->WriteStringEntryValue("General", "Filter", strValue);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// MOS 8580 checkbox
		////////////////////////////////////////////////////////////////////////
		case SID_CHECK_8580:
		{
			if (mos8580Check->Value() == B_CONTROL_ON)
				strValue = "Yes";
			else
				strValue = "No";

			sidSettings->WriteStringEntryValue("General", "MOS8580", strValue);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Force speed checkbox
		////////////////////////////////////////////////////////////////////////
		case SID_CHECK_SPEED:
		{
			if (forceSpeedCheck->Value() == B_CONTROL_ON)
				strValue = "Yes";
			else
				strValue = "No";

			sidSettings->WriteStringEntryValue("General", "ForceSongSpeed", strValue);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Full bank-switching radio button
		////////////////////////////////////////////////////////////////////////
		case SID_RADIO_FULLBANK:
		{
			sidSettings->WriteIntEntryValue("MPU", "Memory", RADIO_MEMORY_FULLBANK);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Transparent ROM radio button
		////////////////////////////////////////////////////////////////////////
		case SID_RADIO_TRANSPARENT:
		{
			sidSettings->WriteIntEntryValue("MPU", "Memory", RADIO_MEMORY_TRANSPARENT);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// PlaySID environment radio button
		////////////////////////////////////////////////////////////////////////
		case SID_RADIO_PLAYSID:
		{
			sidSettings->WriteIntEntryValue("MPU", "Memory", RADIO_MEMORY_PLAYSID);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// PAL radio button
		////////////////////////////////////////////////////////////////////////
		case SID_RADIO_PAL:
		{
			sidSettings->WriteIntEntryValue("Speed", "ClockSpeed", RADIO_SPEED_PAL);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// NTSC radio button
		////////////////////////////////////////////////////////////////////////
		case SID_RADIO_NTSC:
		{
			sidSettings->WriteIntEntryValue("Speed", "ClockSpeed", RADIO_SPEED_NTSC);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Filter parameters
		////////////////////////////////////////////////////////////////////////
		case SID_FILTER_PAR1:
		case SID_FILTER_PAR2:
		case SID_FILTER_PAR3:
		{
			float par1, par2, par3;

			par1 = FILTER_PAR1_MIN - par1Slider->Value() + FILTER_PAR1_MAX;
			par2 = par2Slider->Value();
			par3 = FILTER_PAR3_MIN - ((float)par3Slider->Value() / 100.0f) + FILTER_PAR3_MAX;

			formular->UpdateCurve(par1, par2, par3);

			sidSettings->WriteIntEntryValue("Filter", "FilterFs", par1Slider->Value());
			sidSettings->WriteIntEntryValue("Filter", "FilterFm", par2Slider->Value());
			sidSettings->WriteIntEntryValue("Filter", "FilterFt", par3Slider->Value());
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Filter default button
		////////////////////////////////////////////////////////////////////////
		case SID_FILTER_DEFAULT:
		{
			float par1, par2, par3;

			par1 = FILTER_PAR1_MIN - SIDEMU_DEFAULTFILTERFS + FILTER_PAR1_MAX;
			par2 = SIDEMU_DEFAULTFILTERFM;
			par3 = FILTER_PAR3_MIN * 100.0f - (SIDEMU_DEFAULTFILTERFT * 100.0f) + FILTER_PAR3_MAX;

			formular->UpdateCurve(SIDEMU_DEFAULTFILTERFS, SIDEMU_DEFAULTFILTERFM, SIDEMU_DEFAULTFILTERFT);

			sidSettings->WriteIntEntryValue("Filter", "FilterFs", par1);
			sidSettings->WriteIntEntryValue("Filter", "FilterFm", par2);
			sidSettings->WriteIntEntryValue("Filter", "FilterFt", par3);

			par1Slider->SetValue(par1);
			par2Slider->SetValue(par2);
			par3Slider->SetValue(par3);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel 1 panning
		////////////////////////////////////////////////////////////////////////
		case SID_PANNING_CHAN1:
		{
			value = chan1Slider->Value();
			sidSettings->WriteIntEntryValue("Panning", "Channel1", value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel 2 panning
		////////////////////////////////////////////////////////////////////////
		case SID_PANNING_CHAN2:
		{
			value = chan2Slider->Value();
			sidSettings->WriteIntEntryValue("Panning", "Channel2", value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel 3 panning
		////////////////////////////////////////////////////////////////////////
		case SID_PANNING_CHAN3:
		{
			value = chan3Slider->Value();
			sidSettings->WriteIntEntryValue("Panning", "Channel3", value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel 4 panning
		////////////////////////////////////////////////////////////////////////
		case SID_PANNING_CHAN4:
		{
			value = chan4Slider->Value();
			sidSettings->WriteIntEntryValue("Panning", "Channel4", value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Digi scan
		////////////////////////////////////////////////////////////////////////
		case SID_DIGI_SCAN:
		{
			value = digiSlider->Value();
			sidSettings->WriteIntEntryValue("Misc", "DigiScan", value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Stil path
		////////////////////////////////////////////////////////////////////////
		case SID_STIL_PATH:
		{
			strValue = stilPath->Text();
			sidSettings->WriteStringEntryValue("Misc", "StilPath", strValue);

			// Delete the panel
			delete panel;
			delete panelMessenger;

			panel          = NULL;
			panelMessenger = NULL;
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Stil disk
		////////////////////////////////////////////////////////////////////////
		case SID_STIL_DISK:
		{
			BMessage panelMsg(SID_STIL_REF_RECEIVED);
			PString buttonLabel;
			char *buttonStr;

			// Check if the path has been changed
			if (sidSettings->GetStringEntryValue("Misc", "StilPath") != stilPath->Text())
			{
				// Delete the panel
				delete panel;
				delete panelMessenger;

				panel          = NULL;
				panelMessenger = NULL;
			}

			if (panel != NULL)
			{
				entry_ref ref;

				// Get the current path
				panel->GetPanelDirectory(&ref);

				// Check to see if the current path still exists
				BEntry entry(&ref);
				BPath path(&entry);

				if (!PDirectory::DirectoryExists(path.Path()))
				{
					// It doesn't, delete the panel
					delete panel;
					panel = NULL;

					delete panelMessenger;
					panelMessenger = NULL;
				}
			}

			// Allocate the panel if not already allocated
			if (panel == NULL)
			{
				// Create the window messenger object
				panelMessenger = new BMessenger(Window());

				// Create the file panel object
				panel = new BFilePanel(B_OPEN_PANEL, panelMessenger, NULL, B_DIRECTORY_NODE);

				PString path(PDirectory::GetParentDirectory(stilPath->Text()));
				if (!path.IsEmpty())
				{
					char *pathStr;
					BEntry entry((pathStr = path.GetString()));
					if (entry.InitCheck() == B_OK)
						panel->SetPanelDirectory(&entry);
	
					path.FreeBuffer(pathStr);
				}
			}

			// Set the message to send
			panel->SetMessage(&panelMsg);

			// Change the default button
			buttonLabel.LoadString(res, IDS_SID_SELECT);
			panel->SetButtonLabel(B_DEFAULT_BUTTON, (buttonStr = buttonLabel.GetString()));
			buttonLabel.FreeBuffer(buttonStr);

			// Open the file panel
			panel->Show();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Stil file panel
		////////////////////////////////////////////////////////////////////////
		case SID_STIL_REF_RECEIVED:
		{
			entry_ref entRef;
			BEntry entry;
			BPath path;

			// Extract the directory name
			if (message->FindRef("refs", &entRef) == B_OK)
			{
				entry.SetTo(&entRef);
				entry.GetPath(&path);

				// Change the text control to hold the new path
				stilPath->SetText(path.Path());

				// And write it in the settings
				sidSettings->WriteStringEntryValue("Misc", "StilPath", path.Path());
			}
			break;
		}
	}
}



/******************************************************************************/
/* CalcMinSize() will calculate the minimum size the view can have.           */
/*                                                                            */
/* Output: Is a BPoint where the x is the minimum width and y is the minimum  */
/*         height.                                                            */
/******************************************************************************/
BPoint SIDView::CalcMinSize(void)
{
	float sw, sh;
	float bw, bh;
	float tw, th;
	float w;
	BPoint size;

	// Calculate height
	par1Slider->GetPreferredSize(&sw, &sh);
	defaultButton->GetPreferredSize(&bw, &bh);
	stilPath->GetPreferredSize(&tw, &th);

	size.y = VSPACE * 9.0f + (fontHeight + VSPACE * 2.0f) * 3.0f + (sh + VSPACE) * 8.0f + bh + 2.0f + th + (VSPACE * 2.0f);

	// Calculate width
	filterCheck->GetPreferredSize(&w, &sh);

	mos8580Check->GetPreferredSize(&sw, &sh);
	w = max(sw, w);

	forceSpeedCheck->GetPreferredSize(&sw, &sh);
	w = max(sw, w);

	fullRadio->GetPreferredSize(&sw, &sh);
	w = max(sw, w);

	transparentRadio->GetPreferredSize(&sw, &sh);
	w = max(sw, w);

	playSidRadio->GetPreferredSize(&sw, &sh);
	w = max(sw, w);

	palRadio->GetPreferredSize(&sw, &sh);
	w = max(sw, w);

	ntscRadio->GetPreferredSize(&sw, &sh);
	w = max(sw, w);

	w = max(w, B_DEFAULT_MITER_LIMIT * 2.0f + generalBox->StringWidth(generalBox->Label()));
	w = max(w, B_DEFAULT_MITER_LIMIT * 2.0f + memoryBox->StringWidth(memoryBox->Label()));
	w = max(w, B_DEFAULT_MITER_LIMIT * 2.0f + speedBox->StringWidth(speedBox->Label()));

	size.x = w + 4.0f * HSPACE;

	par1Slider->GetPreferredSize(&sw, &sh);

	defaultButton->GetPreferredSize(&bw, &bh);
	sw = max(sw, w);
	sw = max(sw, 144.0f);

	w = HSPACE * 3.0f + sw;
	formular->GetPreferredSize(&sw, &sh);

	w = max(w + sw + HSPACE, B_DEFAULT_MITER_LIMIT * 2.0f + filterBox->StringWidth(filterBox->Label()) + 2.0f * HSPACE);
	size.x += (w + 2.0f * HSPACE - 1.0f);

	return (size);
}



/******************************************************************************/
/* SetPosAndSize() will calculate the positions and sizes for all views.      */
/******************************************************************************/
void SIDView::SetPosAndSize(void)
{
	float x, y, w, h;
	float controlWidth, controlHeight;
	float temp, temp1, temp2, temp3, temp4;
	BRect rect;

	// Find maximum width of all check boxes and radio buttons in all boxes
	filterCheck->GetPreferredSize(&controlWidth, &controlHeight);

	mos8580Check->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	forceSpeedCheck->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	fullRadio->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	transparentRadio->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	playSidRadio->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	palRadio->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	ntscRadio->GetPreferredSize(&w, &controlHeight);
	controlWidth = max(controlWidth, w);

	// Set the checkboxes to the preferred sizes and move them to the right positions
	y = fontHeight + VSPACE;
	filterCheck->MoveTo(HSPACE, y);
	filterCheck->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	mos8580Check->MoveTo(HSPACE, y);
	mos8580Check->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	forceSpeedCheck->MoveTo(HSPACE, y);
	forceSpeedCheck->ResizeTo(controlWidth, controlHeight);

	w = max(controlWidth, B_DEFAULT_MITER_LIMIT * 2.0f + generalBox->StringWidth(generalBox->Label()));
	w = max(controlWidth, B_DEFAULT_MITER_LIMIT * 2.0f + memoryBox->StringWidth(memoryBox->Label()));
	w = max(controlWidth, B_DEFAULT_MITER_LIMIT * 2.0f + speedBox->StringWidth(speedBox->Label()));
	w += (2.0f * HSPACE);

	h = fontHeight + 3.0f * VSPACE + (y + controlHeight - (fontHeight + VSPACE));
	generalBox->MoveTo(HSPACE, VSPACE);
	generalBox->ResizeTo(w, h);

	//
	// MPU Memory Mode
	//
	y = fontHeight + VSPACE;
	fullRadio->MoveTo(HSPACE, y);
	fullRadio->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	transparentRadio->MoveTo(HSPACE, y);
	transparentRadio->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	playSidRadio->MoveTo(HSPACE, y);
	playSidRadio->ResizeTo(controlWidth, controlHeight);

	memoryBox->MoveTo(HSPACE, h + VSPACE * 4.0f);
	h = fontHeight + 3.0f * VSPACE + (y + controlHeight - (fontHeight + VSPACE));
	memoryBox->ResizeTo(w, h);

	//
	// C64 Clock Speed
	//
	y = fontHeight + VSPACE;
	palRadio->MoveTo(HSPACE, y);
	palRadio->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	ntscRadio->MoveTo(HSPACE, y);
	ntscRadio->ResizeTo(controlWidth, controlHeight);

	speedBox->MoveTo(HSPACE, VSPACE + (h + VSPACE * 3.0f) * 2.0f);
	h = fontHeight + 3.0f * VSPACE + (y + controlHeight - (fontHeight + VSPACE));
	speedBox->ResizeTo(w, h);

	//
	// Filter Adjustment
	//
	x = w + HSPACE * 3.0f;

	// Find the maximum width and height of all sliders
	par1Slider->GetPreferredSize(&controlWidth, &controlHeight);

	temp = par1Slider->StringWidth(par1Slider->Label());
	temp = max(temp, par2Slider->StringWidth(par2Slider->Label()));
	temp = max(temp, par3Slider->StringWidth(par3Slider->Label()));

	par1Slider->SetLabelWidth(temp);
	par2Slider->SetLabelWidth(temp);
	par3Slider->SetLabelWidth(temp);

	defaultButton->GetPreferredSize(&w, &temp);
	controlWidth = max(controlWidth, w);
	controlWidth = max(controlWidth, 144.0f);

	y = fontHeight + VSPACE;
	par1Slider->MoveTo(HSPACE, y);
	par1Slider->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	par2Slider->MoveTo(HSPACE, y);
	par2Slider->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	par3Slider->MoveTo(HSPACE, y);
	par3Slider->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE * 2.0f);
	defaultButton->MoveTo(HSPACE, y);
	defaultButton->ResizeTo(w, temp);
	h = temp;

	w = HSPACE * 2.0f + controlWidth;
	formular->MoveTo(w, fontHeight + VSPACE);
	formular->GetPreferredSize(&temp, &temp1);
	formular->ResizeTo(temp, temp1);

	w = max(w + temp + HSPACE, B_DEFAULT_MITER_LIMIT * 2.0f + filterBox->StringWidth(filterBox->Label()) + 2.0f * HSPACE);
	h = y + h + VSPACE;
	h = max(h, fontHeight + VSPACE * 2.0f + temp1);
	filterBox->MoveTo(x, VSPACE);
	filterBox->ResizeTo(w + HSPACE, h);

	temp = min(h, temp1);
	y = (h - fontHeight - temp) / 2.0f - 1.0f;
	formular->MoveBy(0.0f, -y);

	//
	// Panning
	//
	// Find the maximum width and height of all sliders
	w += HSPACE;

	chan1Slider->GetPreferredSize(&controlWidth, &controlHeight);

	temp = chan1Slider->StringWidth(chan1Slider->Label());
	temp = max(temp, chan2Slider->StringWidth(chan2Slider->Label()));
	temp = max(temp, chan3Slider->StringWidth(chan3Slider->Label()));
	temp = max(temp, chan4Slider->StringWidth(chan4Slider->Label()));

	chan1Slider->SetLabelWidth(temp);
	chan2Slider->SetLabelWidth(temp);
	chan3Slider->SetLabelWidth(temp);
	chan4Slider->SetLabelWidth(temp);

	controlWidth = w - HSPACE * 2.0f;

	y = fontHeight + VSPACE;
	chan1Slider->MoveTo(HSPACE, y);
	chan1Slider->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	chan2Slider->MoveTo(HSPACE, y);
	chan2Slider->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	chan3Slider->MoveTo(HSPACE, y);
	chan3Slider->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE);
	chan4Slider->MoveTo(HSPACE, y);
	chan4Slider->ResizeTo(controlWidth, controlHeight);

	temp1 = VSPACE + (h + VSPACE * 3.0f);
	panningBox->MoveTo(x, temp1);
	h = fontHeight + 4.0f * VSPACE + (y + controlHeight - (fontHeight + VSPACE));
	panningBox->ResizeTo(w, h);

	//
	// Misc
	//
	digiSlider->GetPreferredSize(&controlWidth, &controlHeight);

	temp = digiSlider->StringWidth(digiSlider->Label());

	digiSlider->SetLabelWidth(temp);

	controlWidth = w - HSPACE * 2.0f;

	y = fontHeight + VSPACE;
	digiSlider->MoveTo(HSPACE, y);
	digiSlider->ResizeTo(controlWidth, controlHeight);

	y += (controlHeight + VSPACE * 2.0f);
	controlWidth -= HSPACE;
	stilText->GetPreferredSize(&temp2, &temp3);
	stilText->ResizeTo(temp2, temp3);

	stilPath->MoveTo(HSPACE * 2.0f + temp2, y);
	stilPath->GetPreferredSize(&temp, &temp3);
	stilPath->ResizeTo(controlWidth - temp2 - HSPACE * 3.0f - PICHSIZE, temp3);

	stilDisk->MoveTo(controlWidth - PICHSIZE, y);
	stilDisk->ResizeTo(PICHSIZE + HSPACE - 1.0f, temp3);

	stilPath->TextView()->GetPreferredSize(&temp4, &temp);
	stilPath->TextView()->ResizeTo(controlWidth - temp2 - HSPACE * 5.0f - PICHSIZE - 1.0f, temp);

	stilText->MoveTo(HSPACE, y + ((temp - fontHeight) / 2.0f));

	y = temp1 + VSPACE * 2.0f + h;
	miscBox->MoveTo(x, y);
	h = fontHeight + VSPACE * 4.0f + controlHeight + temp3;
	miscBox->ResizeTo(w, h);

	//
	// Okay, fine tune the position of the boxes left
	//
	rect = speedBox->Frame();
	temp = (y + h - rect.bottom) / 2.0f;

	speedBox->MoveTo(rect.left, y + h - (rect.bottom - rect.top));

	rect = memoryBox->Frame();
	memoryBox->MoveTo(rect.left, rect.top + temp);
}
