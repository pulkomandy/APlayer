/******************************************************************************/
/* APlayer Settings Mixer View class.                                         */
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
#include "PSettings.h"
#include "PSkipList.h"
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APViewMixer.h"
#include "APSlider.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Global tables                                                              */
/******************************************************************************/
#define FREQNUM			6
static int32 freqTable[FREQNUM] = { 8268, 11025, 22050, 33075, 44100, 48000 };



/******************************************************************************/
/* APViewLine class                                                           */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewLine::APViewLine(BRect frame) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewLine::~APViewLine(void)
{
}



/******************************************************************************/
/* Draw() is called when the view needs to be drawn.                          */
/*                                                                            */
/* Input:  "updateRect" is the rect to update.                                */
/******************************************************************************/
void APViewLine::Draw(BRect updateRect)
{
	BRect rect;

	// Call base class
	BView::Draw(updateRect);

	// Draw the 3D line
	rect = Bounds();

	SetHighColor(BeInactiveGrey);
	SetDrawingMode(B_OP_COPY);
	StrokeLine(BPoint(rect.left, rect.top), BPoint(rect.right, rect.top));

	SetHighColor(White);
	StrokeLine(BPoint(rect.left, rect.top + 1.0f), BPoint(rect.right, rect.top + 1.0f));
}





/******************************************************************************/
/* APViewNoSettings class                                                     */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewNoSettings::APViewNoSettings(PResource *res) : BView(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_FOLLOW_NONE, B_WILL_DRAW)
{
	float fontHeight;
	char *strPtr;

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Get the text string
	textString.LoadString(res, IDS_SET_MIX_NOSETTINGS);

	// Resize the view
	ResizeTo(StringWidth((strPtr = textString.GetString())), fontHeight);
	textString.FreeBuffer(strPtr);

	// Change the background color
	SetViewColor(BeBackgroundGrey);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewNoSettings::~APViewNoSettings(void)
{
}



/******************************************************************************/
/* Draw() is called when the view needs to be drawn.                          */
/*                                                                            */
/* Input:  "updateRect" is the rect to update.                                */
/******************************************************************************/
void APViewNoSettings::Draw(BRect updateRect)
{
	char *strPtr;

	// Call base class
	BView::Draw(updateRect);

	// Draw the text string
	SetHighColor(Black);
	DrawString((strPtr = textString.GetString()), BPoint(0.0f, fh.ascent));
	textString.FreeBuffer(strPtr);
}





/******************************************************************************/
/* APViewMixer class                                                          */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewMixer::APViewMixer(MainWindowSystem *system, APGlobalData *global, BRect frame, PString name) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect rect;
	PString label;
	char *labelPtr;
	BMessage *message;
	float x, y, y1, w, h, tempX, outputY;
	float boxY, valWidth, maxWidth;
	float controlWidth, controlHeight;
	font_height fh;
	float fontHeight;
	int32 i, j;

	// Remember the arguments
	windowSystem = system;
	globalData   = global;
	res          = system->res;

	// Set the view name
	SetName((labelPtr = name.GetString()));
	name.FreeBuffer(labelPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// Create the "General" box
	//
	label.LoadString(res, IDS_SET_MIX_GENERAL);
	generalBox = new BBox(BRect(0.0f, 0.0f, SET_MIN_VIEW_WIDTH, 0.0f), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	generalBox->SetLabel((labelPtr = label.GetString()));
	AddChild(generalBox);
	label.FreeBuffer(labelPtr);

	rect = generalBox->Bounds();
	rect.InsetBy(HSPACE * 2.0f, 0.0f);

	valWidth = StringWidth("88888");
	y = rect.top + fontHeight + VSPACE;

	message = new BMessage(SET_MIX_SLIDER_MIXFREQ);
	label.LoadString(res, IDS_SET_MIX_MIXFREQ);
	w = StringWidth((labelPtr = label.GetString()));

	mixFreqSlider = new APSlider(BRect(rect.left, y, rect.right, y), label, true, message, 0, FREQNUM - 1, valWidth, B_BLOCK_THUMB, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_FRAME_EVENTS | B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	mixFreqSlider->SetValueFunction(MixerFreqFunc, NULL);
	mixFreqSlider->SetHashMarks(B_HASH_MARKS_TOP);
	mixFreqSlider->SetHashMarkCount(FREQNUM);
	mixFreqSlider->GetPreferredSize(&controlWidth, &controlHeight);
	mixFreqSlider->ResizeTo(controlWidth, controlHeight);
	generalBox->AddChild(mixFreqSlider);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_MIX_SLIDER_STEREOSEP);
	label.LoadString(res, IDS_SET_MIX_STEREOSEP);
	w  = max(w, StringWidth((labelPtr = label.GetString())));
	y += controlHeight + VSPACE;

	stereoSepSlider = new APSlider(BRect(rect.left, y, rect.right, y), label, true, message, 0, 100, valWidth, B_BLOCK_THUMB, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	stereoSepSlider->SetValueFunction(StereoSepFunc, this);
	stereoSepSlider->SetHashMarks(B_HASH_MARKS_TOP);
	stereoSepSlider->SetHashMarkCount(11);
	stereoSepSlider->ResizeToPreferred();
	generalBox->AddChild(stereoSepSlider);
	label.FreeBuffer(labelPtr);

	mixFreqSlider->SetLabelWidth(w);
	stereoSepSlider->SetLabelWidth(w);

	x  = HSPACE;
	y += controlHeight + VSPACE;

	message = new BMessage(SET_MIX_CHECK_INTERP);
	label.LoadString(res, IDS_SET_MIX_INTERPOLATION);
	interpCheck = new BCheckBox(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message);
	interpCheck->ResizeToPreferred();
	generalBox->AddChild(interpCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_MIX_CHECK_SURROUND);
	label.LoadString(res, IDS_SET_MIX_SURROUND);
	surroundCheck = new BCheckBox(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message);
	surroundCheck->GetPreferredSize(&controlWidth, &controlHeight);
	surroundCheck->ResizeTo(controlWidth, controlHeight);
	surroundCheck->MoveTo((SET_MIN_VIEW_WIDTH - controlWidth) / 2.0f, y);
	generalBox->AddChild(surroundCheck);
	label.FreeBuffer(labelPtr);

	message = new BMessage(SET_MIX_CHECK_FILTER);
	label.LoadString(res, IDS_SET_MIX_FILTER);
	filterCheck = new BCheckBox(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message);
	filterCheck->GetPreferredSize(&controlWidth, &controlHeight);
	filterCheck->ResizeTo(controlWidth, controlHeight);
	filterCheck->MoveTo(SET_MIN_VIEW_WIDTH - controlWidth - HSPACE, y);
	generalBox->AddChild(filterCheck);
	label.FreeBuffer(labelPtr);

	y += controlHeight + VSPACE;
	generalBox->ResizeTo(SET_MIN_VIEW_WIDTH, y);


	//
	// Create the "Mixer Output" box
	//
	boxY    = y + VSPACE * 2.0f;
	outputY = boxY;
	label.LoadString(res, IDS_SET_MIX_MIXEROUTPUT);
	mixerOutputBox = new BBox(BRect(0.0f, boxY, SET_MIN_VIEW_WIDTH, boxY), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	mixerOutputBox->SetLabel((labelPtr = label.GetString()));
	AddChild(mixerOutputBox);
	label.FreeBuffer(labelPtr);

	rect = mixerOutputBox->Bounds();
	rect.InsetBy(HSPACE * 2.0f, 0.0f);
	y = rect.top + fontHeight + VSPACE;

	agentPop = new BPopUpMenu("");

	label.LoadString(res, IDS_SET_MIX_AGENT);
	w = StringWidth((labelPtr = label.GetString())) + HSPACE;

	agentField = new BMenuField(BRect(HSPACE, y, HSPACE + w, y), NULL, labelPtr, agentPop);
	agentField->SetDivider(w);
	mixerOutputBox->AddChild(agentField);
	label.FreeBuffer(labelPtr);

	y += fontHeight + VSPACE * 3.0f + 1.0f;
	lineView = new APViewLine(BRect(rect.left, y, rect.right, y + 1.0f));
	mixerOutputBox->AddChild(lineView);

	agentConfig = NULL;

	//
	// Create the "Channels" box
	//
	boxY += y;
	label.LoadString(res, IDS_SET_MIX_CHANNELS);
	channelsBox = new BBox(BRect(0.0f, boxY, SET_MIN_VIEW_WIDTH, boxY + 150.0f), NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS);
	channelsBox->SetLabel((labelPtr = label.GetString()));
	AddChild(channelsBox);
	label.FreeBuffer(labelPtr);

	rect = channelsBox->Bounds();
	rect.InsetBy(HSPACE * 2.0f, 0.0f);

	y = rect.top + fontHeight + VSPACE;

	message = new BMessage(SET_MIX_BUTTON_0_15);
	label.LoadString(res, IDS_SET_MIX_0_15);
	chan1But = new BButton(BRect(rect.left, y, rect.left, y), NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	chan1But->GetPreferredSize(&controlWidth, &controlHeight);
	channelsBox->AddChild(chan1But);
	maxWidth = controlWidth;
	label.FreeBuffer(labelPtr);

	y1 = y + controlHeight + VSPACE / 2.0f;
	message = new BMessage(SET_MIX_BUTTON_16_31);
	label.LoadString(res, IDS_SET_MIX_16_31);
	chan2But = new BButton(BRect(rect.left, y1, rect.left, y1), NULL, (labelPtr = label.GetString()), message);
	chan2But->GetPreferredSize(&controlWidth, &controlHeight);
	channelsBox->AddChild(chan2But);
	maxWidth = max(maxWidth, controlWidth);
	label.FreeBuffer(labelPtr);

	y1 += controlHeight + VSPACE / 2.0f;
	message = new BMessage(SET_MIX_BUTTON_32_47);
	label.LoadString(res, IDS_SET_MIX_32_47);
	chan3But = new BButton(BRect(rect.left, y1, rect.left, y1), NULL, (labelPtr = label.GetString()), message);
	chan3But->GetPreferredSize(&controlWidth, &controlHeight);
	channelsBox->AddChild(chan3But);
	maxWidth = max(maxWidth, controlWidth);
	label.FreeBuffer(labelPtr);

	y1 += controlHeight + VSPACE / 2.0f;
	message = new BMessage(SET_MIX_BUTTON_48_63);
	label.LoadString(res, IDS_SET_MIX_48_63);
	chan4But = new BButton(BRect(rect.left, y1, rect.left, y1), NULL, (labelPtr = label.GetString()), message);
	chan4But->GetPreferredSize(&controlWidth, &controlHeight);
	channelsBox->AddChild(chan4But);
	maxWidth = max(maxWidth, controlWidth);
	label.FreeBuffer(labelPtr);

	chan1But->ResizeTo(maxWidth, controlHeight);
	chan2But->ResizeTo(maxWidth, controlHeight);
	chan3But->ResizeTo(maxWidth, controlHeight);
	chan4But->ResizeTo(maxWidth, controlHeight);

	x   = rect.left + maxWidth + HSPACE * 8.0f;
	y1 += controlHeight + VSPACE + VSPACE / 2.0f;

	y += (controlHeight - y) / 2.0f;

	for (i = 0; i < 4; i++)
	{
		tempX = x;

		for (j = 0; j < 16; j++)
		{
			message = new BMessage(SET_MIX_CHECK_CHANNEL);
			message->AddInt16("chan", i * 16 + j);
			chanCheck[i * 16 + j] = new BCheckBox(BRect(tempX, y, tempX, y), NULL, NULL, message);
			chanCheck[i * 16 + j]->GetPreferredSize(&controlWidth, &h);
			chanCheck[i * 16 + j]->ResizeTo(controlWidth, h);
			channelsBox->AddChild(chanCheck[i * 16 + j]);

			tempX += controlWidth + (HSPACE - (controlWidth - 22.0f));
		}

		y += controlHeight + VSPACE / 2.0f;
	}

	// Calculate the position of the channels box
	y = SET_MIN_VIEW_HEIGHT - y1;

	// Move and resize the boxes
	channelsBox->ResizeTo(SET_MIN_VIEW_WIDTH, y1);
	channelsBox->MoveTo(0.0f, y);
	mixerOutputBox->ResizeTo(SET_MIN_VIEW_WIDTH, y - outputY - VSPACE * 2.0f);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewMixer::~APViewMixer(void)
{
	APAgentSettings *settings;
	int32 i, count;

	// Delete all backup settings
	count = agentSettings.CountItems();
	for (i = 0; i < count; i++)
	{
		agentSettings.GetItem(i, settings);

		globalData->DeleteAddOnInstance(settings->agentName, apAgent, settings->agent);
		delete settings->backup;
		delete settings;
	}

	agentSettings.MakeEmpty();
}



/******************************************************************************/
/* InitSettings() will read the settings and set all the controls.            */
/******************************************************************************/
void APViewMixer::InitSettings(void)
{
	PString tempStr;
	int32 tempNum;
	int16 i;

	//
	// General box
	//
	// Mixer frequency
	tempNum = windowSystem->useSettings->GetIntEntryValue("Mixer", "Frequency");

	for (i = 0; i < FREQNUM; i++)
	{
		if (freqTable[i] > tempNum)
			break;
	}

	// Only decrement the index value if it isn't the first one
	if (i != 0)
		i--;

	mixFreqSlider->SetValue(i);

	// Stereo separation
	tempNum = windowSystem->useSettings->GetIntEntryValue("Mixer", "StereoSep");
	stereoSepSlider->SetValue(tempNum);

	// Interpolation
	tempStr = windowSystem->useSettings->GetStringEntryValue("Mixer", "Interpolation");
	interpCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	// Dolby Prologic Surround
	tempStr = windowSystem->useSettings->GetStringEntryValue("Mixer", "DolbyPrologic");
	surroundCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	// Amiga filter
	tempStr = windowSystem->useSettings->GetStringEntryValue("Mixer", "AmigaFilter");
	filterCheck->SetValue(tempStr.CompareNoCase("Yes") == 0 ? B_CONTROL_ON : B_CONTROL_OFF);

	//
	// Mixer Output box
	//
	// Output agent
	selectedAgent = windowSystem->useSettings->GetStringEntryValue("Mixer", "OutputAgent");

	//
	// Channels box
	//
	for (i = 0; i < MAX_NUM_CHANNELS; i++)
		chanCheck[i]->SetValue(windowSystem->channelsEnabled[i] ? B_CONTROL_ON : B_CONTROL_OFF);
}



/******************************************************************************/
/* RememberSettings() will read the data from controls which hasn't stored    */
/*      they values yet, eg. edit controls.                                   */
/******************************************************************************/
void APViewMixer::RememberSettings(void)
{
	BView *workView;
	BTextControl *controlView;
	BMessage *message;

	if (agentConfig != NULL)
	{
		// Get the first child view
		workView = agentConfig->ChildAt(0);

		// Traverse all child views
		while (workView != NULL)
		{
			// Is the view an edit box?
			if (is_kind_of(workView, BTextControl))
			{
				// We found an edit box, now cast it
				controlView = (BTextControl *)workView;

				// Get the BMessage assigned with the control
				message = controlView->Message();
				if (message != NULL)
				{
					// The control got a message, so parse it
					controlView->MessageReceived(message);
				}
			}

			// Go to the next view
			workView = workView->NextSibling();
		}
	}
}



/******************************************************************************/
/* CancelSettings() will restore real-time values.                            */
/******************************************************************************/
void APViewMixer::CancelSettings(void)
{
	int32 stereoSep, interp, prologic, filter;
	APAgentSettings *settings;
	int32 i, count;

	// Restore stereo separation
	stereoSep = windowSystem->useSettings->GetIntEntryValue("Mixer", "StereoSep");
	interp    = windowSystem->useSettings->GetStringEntryValue("Mixer", "Interpolation").CompareNoCase("Yes") == 0;
	prologic  = windowSystem->useSettings->GetStringEntryValue("Mixer", "DolbyPrologic").CompareNoCase("Yes") == 0;
	filter    = windowSystem->useSettings->GetStringEntryValue("Mixer", "AmigaFilter").CompareNoCase("Yes") == 0;
	windowSystem->SetMixerSettings(interp, prologic, stereoSep, filter);

	// Restore all the output agent settings
	count = agentSettings.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the settings information
		agentSettings.GetItem(i, settings);

		// Copy the settings back to the original
		settings->original->CloneSettings(settings->backup);
	}
}



/******************************************************************************/
/* SaveSettings() will save some of the settings in other files.              */
/******************************************************************************/
void APViewMixer::SaveSettings(void)
{
	PString fileName;
	APAgentSettings *settings;
	int32 i, count;

	// Traverse all the agent settings and save them
	count = agentSettings.CountItems();
	for (i = 0; i < count; i++)
	{
		agentSettings.GetKeyAndItem(i, fileName, settings);
		settings->original->SaveFile(fileName, "Polycode", "APlayer");
		settings->backup->CloneSettings(settings->original);
	}
}



/******************************************************************************/
/* SaveWindowSettings() will save the view specific settings entries.         */
/******************************************************************************/
void APViewMixer::SaveWindowSettings(void)
{
}



/******************************************************************************/
/* MakeBackup() will make a backup of special settings not stored in the      */
/*      "use" settings.                                                       */
/******************************************************************************/
void APViewMixer::MakeBackup(void)
{
}



/******************************************************************************/
/* HandleMessage() is called for each message the window don't know.          */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APViewMixer::HandleMessage(BMessage *msg)
{
	int32 value;

	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Mixer frequency slider
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_SLIDER_MIXFREQ:
		{
			value = mixFreqSlider->Value();
			windowSystem->useSettings->WriteIntEntryValue("Mixer", "Frequency", freqTable[value]);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Stereo separation slider
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_SLIDER_STEREOSEP:
		{
			value = stereoSepSlider->Value();
			windowSystem->useSettings->WriteIntEntryValue("Mixer", "StereoSep", value);
			windowSystem->SetMixerSettings(-1, -1, value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Interpolation checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_CHECK_INTERP:
		{
			value = (interpCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Mixer", "Interpolation", value ? "Yes" : "No");
			windowSystem->SetMixerSettings(value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Dolby Prologic Surround checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_CHECK_SURROUND:
		{
			value = (surroundCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Mixer", "DolbyPrologic", value ? "Yes" : "No");
			windowSystem->SetMixerSettings(-1, value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Amiga filter checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_CHECK_FILTER:
		{
			value = (filterCheck->Value() == B_CONTROL_ON ? true : false);
			windowSystem->useSettings->WriteStringEntryValue("Mixer", "AmigaFilter", value ? "Yes" : "No");
			windowSystem->SetMixerSettings(-1, -1, -1, value);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Output agent list menu
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_LIST_AGENT:
		{
			BMenuItem *item;

			value         = msg->FindInt32("index");
			item          = agentPop->ItemAt(value);
			selectedAgent = item->Label();

			windowSystem->useSettings->WriteStringEntryValue("Mixer", "OutputAgent", selectedAgent);
			ShowAgentSettings();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel button 0-15
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_BUTTON_0_15:
		{
			int16 i, count;
			bool enable = false;

			// Find the maximum channels to check
			count = min(channelsInUse, 16);

			// Find out if we need to enable or disable all the channels
			for (i = 0; i < count; i++)
			{
				if (!windowSystem->channelsEnabled[i])
				{
					enable = true;
					break;
				}
			}

			// Enable or disable the channels
			for (i = 0; i < count; i++)
				chanCheck[i]->SetValue(enable);

			// Change the channels
			windowSystem->EnableChannels(enable, 0, count - 1);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel button 16-31
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_BUTTON_16_31:
		{
			int16 i, count;
			bool enable = false;

			// Find the maximum channels to check
			count = min(channelsInUse, 32);

			// Find out if we need to enable or disable all the channels
			for (i = 16; i < count; i++)
			{
				if (!windowSystem->channelsEnabled[i])
				{
					enable = true;
					break;
				}
			}

			// Enable or disable the channels
			for (i = 16; i < count; i++)
				chanCheck[i]->SetValue(enable);

			// Change the channels
			windowSystem->EnableChannels(enable, 16, count - 1);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel button 32-47
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_BUTTON_32_47:
		{
			int16 i, count;
			bool enable = false;

			// Find the maximum channels to check
			count = min(channelsInUse, 48);

			// Find out if we need to enable or disable all the channels
			for (i = 32; i < count; i++)
			{
				if (!windowSystem->channelsEnabled[i])
				{
					enable = true;
					break;
				}
			}

			// Enable or disable the channels
			for (i = 32; i < count; i++)
				chanCheck[i]->SetValue(enable);

			// Change the channels
			windowSystem->EnableChannels(enable, 32, count - 1);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel button 48-63
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_BUTTON_48_63:
		{
			int16 i, count;
			bool enable = false;

			// Find the maximum channels to check
			count = min(channelsInUse, 64);

			// Find out if we need to enable or disable all the channels
			for (i = 48; i < count; i++)
			{
				if (!windowSystem->channelsEnabled[i])
				{
					enable = true;
					break;
				}
			}

			// Enable or disable the channels
			for (i = 48; i < count; i++)
				chanCheck[i]->SetValue(enable);

			// Change the channels
			windowSystem->EnableChannels(enable, 48, count - 1);
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Channel checkbox
		////////////////////////////////////////////////////////////////////////
		case SET_MIX_CHECK_CHANNEL:
		{
			int16 chan;
			bool enable;

			// Get the channel number from the message
			if (msg->FindInt16("chan", &chan) == B_OK)
			{
				// Get the enable state
				enable = (chanCheck[chan]->Value() == B_CONTROL_ON ? true : false);

				// Change the channel
				windowSystem->EnableChannels(enable, chan);
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Call the sound output agent settings view
		////////////////////////////////////////////////////////////////////////
		default:
		{
			if (agentConfig != NULL)
				agentConfig->MessageReceived(msg);

			break;
		}
	}
}



/******************************************************************************/
/* SetChannels() enable and disable the channel checkboxes.                   */
/******************************************************************************/
void APViewMixer::SetChannels(void)
{
	uint16 i;

	// Get the number of channels in use right now
	if (windowSystem->playerInfo->HaveInformation())
		channelsInUse = windowSystem->playerInfo->GetModuleChannels();
	else
		channelsInUse = 0;

	// First enable the used channels
	for (i = 0; i < channelsInUse; i++)
		chanCheck[i]->SetEnabled(true);

	// Now disable the rest
	for (i = channelsInUse; i < MAX_NUM_CHANNELS; i++)
		chanCheck[i]->SetEnabled(false);

	// Enable and disable the channel buttons
	if (channelsInUse > 0)
		chan1But->SetEnabled(true);
	else
		chan1But->SetEnabled(false);

	if (channelsInUse > 16)
		chan2But->SetEnabled(true);
	else
		chan2But->SetEnabled(false);

	if (channelsInUse > 32)
		chan3But->SetEnabled(true);
	else
		chan3But->SetEnabled(false);

	if (channelsInUse > 48)
		chan4But->SetEnabled(true);
	else
		chan4But->SetEnabled(false);
}



/******************************************************************************/
/* AttachedToWindow() is called everytime the view is shown.                  */
/******************************************************************************/
void APViewMixer::AttachedToWindow(void)
{
	int32 i, count;
	float maxWidth;
	BMessage *message;
	APAddOnInformation *addOnInfo;
	char *nameStr;

	// Find all the active output agents and add them to the popup box
	//
	// First empty the popup box
	count = agentPop->CountItems();
	for (i = 0; i < count; i++)
		delete agentPop->RemoveItem((int32)0);

	// Add the agent names into the popup box
	maxWidth = 0.0f;

	windowSystem->agentAddOns.LockList();

	count = windowSystem->agentAddOns.CountItems();
	for (i = 0; i < count; i++)
	{
		addOnInfo = windowSystem->agentAddOns.GetItem(i);

		// Is the agent a sound output agent?
		if (addOnInfo->pluginFlags & apaSoundOutput)
		{
			nameStr = addOnInfo->name.GetString();
			message = new BMessage(SET_MIX_LIST_AGENT);
			maxWidth = max(maxWidth, StringWidth(nameStr));
			agentPop->AddItem(new BMenuItem(nameStr, message));
			addOnInfo->name.FreeBuffer(nameStr);
		}
	}

	windowSystem->agentAddOns.UnlockList();

	// Select the right agent
	BMenuItem *item = agentPop->FindItem((nameStr = selectedAgent.GetString()));
	if (item != NULL)
	{
		item->SetMarked(true);
		ShowAgentSettings();
	}

	selectedAgent.FreeBuffer(nameStr);
}



/******************************************************************************/
/* ShowAgentSettings() show the selected output agent settings view.          */
/******************************************************************************/
void APViewMixer::ShowAgentSettings(void)
{
	const APConfigInfo *cfgInfo;
	APAddOnAgent *agent;
	int32 agentIndex;
	BRect lineRect, boxRect, viewRect;
	float x, y;

	// Get different rect needed
	lineRect = lineView->Frame();
	boxRect  = mixerOutputBox->Frame();

	// Now detach the old config view
	if (agentConfig != NULL)
	{
		// Make sure all controls are remembered before we delete it
		RememberSettings();

		// Now remove the view
		mixerOutputBox->RemoveChild(agentConfig);
		delete agentConfig;
		agentConfig = NULL;

		// Refresh the config part of the box
		BRect tempRect;

		tempRect.top    = lineRect.bottom + 1.0f;
		tempRect.bottom = boxRect.Height() - VSPACE;
		tempRect.left   = lineRect.left;
		tempRect.right  = lineRect.right;
		mixerOutputBox->Invalidate(tempRect);
	}

	// Make an instance of the agent
	agent = (APAddOnAgent *)globalData->GetAddOnInstance(selectedAgent, apAgent, &agentIndex);
	if (agent != NULL)
	{
		// Get the configuration view
		cfgInfo = agent->GetConfigInfo();
	}
	else
	{
		// Couldn't create the agent instance, so show no configuration
		cfgInfo = NULL;
	}

	// Add the view into the window
	if (cfgInfo == NULL)
	{
		// This agent doesn't have any config view, so show the
		// default one
		agentConfig = new APViewNoSettings(res);

		// Delete the agent instance, because we don't need it anymore
		if (agent != NULL)
			globalData->DeleteAddOnInstance(selectedAgent, apAgent, agent);
	}
	else
	{
		APAgentSettings *settings;

		// Get the agents config view
		agentConfig = cfgInfo->view;

		// Check to see if we already have made a backup
		if (!agentSettings.HasKey(cfgInfo->fileName))
		{
			// No, we haven't made the backup, so do it
			settings = new APAgentSettings;
			if (settings == NULL)
				throw PMemoryException();

			settings->backup = new PSettings();
			if (settings->backup == NULL)
			{
				delete settings;
				throw PMemoryException();
			}

			// Remember the agent instance
			settings->agent     = agent;
			settings->agentName = selectedAgent;
			settings->original  = cfgInfo->settings;

			// Clone the settings and insert them into the list
			settings->backup->CloneSettings(cfgInfo->settings);
			agentSettings.InsertItem(cfgInfo->fileName, settings);
		}
	}

	// Calculate the position of the view
	viewRect = agentConfig->Bounds();

	x  = lineRect.left;
	x += ((lineRect.Width() - viewRect.Width()) / 2.0f);
	y  = lineRect.bottom;
	y += ((boxRect.Height() - y - viewRect.Height()) / 2.0f);

	mixerOutputBox->AddChild(agentConfig);
	agentConfig->MoveTo(x, y);
}



/******************************************************************************/
/* MixerFreqFunc() will be called from the mixer frequency slider.            */
/*                                                                            */
/* Input:  "value" is the current slider value.                               */
/*         "userData" is not used in this function.                           */
/*                                                                            */
/* Output: The value string to show next to the slider.                       */
/******************************************************************************/
PString APViewMixer::MixerFreqFunc(int32 value, void *userData)
{
	PString retStr;

	retStr.SetNumber(freqTable[value]);
	return (retStr);
}



/******************************************************************************/
/* StereoSepFunc() will be called from the stereo separation slider.          */
/*                                                                            */
/* Input:  "value" is the current slider value.                               */
/*         "userData" is a pointer to the current object.                     */
/*                                                                            */
/* Output: The value string to show next to the slider.                       */
/******************************************************************************/
PString APViewMixer::StereoSepFunc(int32 value, void *userData)
{
	PString retStr;

	// Create the percent string
	retStr.Format("%d%%", value);

	// Change the value real-time
	((APViewMixer *)userData)->windowSystem->SetMixerSettings(-1, -1, value);

	return (retStr);
}
