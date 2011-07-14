/******************************************************************************/
/* Reverb View class.                                                         */
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
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Agent headers
#include "ReverbView.h"
#include "ReverbViewSlider.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *reverbSettings;



/******************************************************************************/
/* ReverbView class                                                           */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ReverbView::ReverbView(APGlobalData *global, PResource *resource) : BView(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BMessage *message;
	PString label;
	char *labelPtr;
	font_height fh;
	float fontHeight;
	float w, valWidth;
	float controlWidth, controlHeight;
	int32 num;

	// Initialize member variables
	globalData = global;
	res        = resource;

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Create the reverb slider
	message = new BMessage(REV_SLIDER_REVERB);
	label.LoadString(res, IDS_REVERB_REVERB);
	w        = StringWidth((labelPtr = label.GetString()));
	valWidth = StringWidth("88");
	label.FreeBuffer(labelPtr);

	reverbSlider = new ReverbViewSlider(BRect(HSPACE, VSPACE * 2.0f, HSPACE + 256.0f, VSPACE * 2.0f), label, true, message, 0, 15, valWidth, B_BLOCK_THUMB, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	reverbSlider->SetValueFunction(ReverbFunc, this);
	reverbSlider->SetHashMarks(B_HASH_MARKS_TOP);
	reverbSlider->SetHashMarkCount(16);
	reverbSlider->UseFillColor(true, &LightBlue);
	reverbSlider->GetPreferredSize(&controlWidth, &controlHeight);
	reverbSlider->ResizeTo(controlWidth, controlHeight);
	AddChild(reverbSlider);

	reverbSlider->SetLabelWidth(w);

	ResizeTo(HSPACE * 2.0f + 256.0f, VSPACE * 4.0f + controlHeight);

	// Set the slider value
	num = reverbSettings->GetIntEntryValue("General", "Reverb");
	reverbSlider->SetValue(num);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ReverbView::~ReverbView(void)
{
}



/******************************************************************************/
/* MessageReceived() handles all the messages we have defined.                */
/*                                                                            */
/* Input:  "message" is the message.                                          */
/******************************************************************************/
void ReverbView::MessageReceived(BMessage *message)
{
	int32 value;

	switch (message->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Reverb slider
		////////////////////////////////////////////////////////////////////////
		case REV_SLIDER_REVERB:
		{
			value = reverbSlider->Value();
			reverbSettings->WriteIntEntryValue("General", "Reverb", value);
			break;
		}
	}
}



/******************************************************************************/
/* ReverbFunc() will be called from the reverb slider.                        */
/*                                                                            */
/* Input:  "userData" is the pointer to gave the slider class.                */
/*         "value" is the current slider value.                               */
/*                                                                            */
/* Output: The value string to show next to the slider.                       */
/******************************************************************************/
PString ReverbView::ReverbFunc(void *userData, int32 value)
{
	PString retStr;

	// Create the value string
	retStr.SetNumber(value);

	// Change the value real-time
	reverbSettings->WriteIntEntryValue("General", "Reverb", value);

	return (retStr);
}
