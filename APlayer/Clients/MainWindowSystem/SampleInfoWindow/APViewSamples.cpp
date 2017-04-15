/******************************************************************************/
/* Sample Info Sample View class.                                             */
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
#include "PAlert.h"
#include "PSystem.h"
#include "Colors.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Santa headers
#include <santa/ColumnListView.h>

// Client headers
#include "MainWindowSystem.h"
#include "APViewSamples.h"
#include "APListViewSample.h"
#include "APListItemSample.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APViewSamples::APViewSamples(APGlobalData *glob, MainWindowSystem *system, BRect frame, PString name) : BView(frame, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BMessage *message;
	PString label, tempStr;
	char *labelPtr, *strPtr;
	font_height fh;
	float controlWidth, controlHeight, maxWidth;
	float x, y, w, h;
	int32 i, count;
	int32 order[11];
	int32 sortKey[1];
	CLVSortMode sortMode[1];
	uint32 colWidth;
	bool found;

	// Remember the arguments
	global       = glob;
	windowSystem = system;

	// Remember the resource
	res = windowSystem->res;

	// Initialize other members
	filePanel = NULL;
	messenger = NULL;

	// Set the view name
	SetName((strPtr = name.GetString()));
	name.FreeBuffer(strPtr);

	// Change the color to light grey
	SetViewColor(BeBackgroundGrey);

	// Find font height
	be_fixed_font->GetHeight(&fh);
	itemFontHeight = max(FIXED_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	//
	// Create the "Sample" box
	//
	frame.right  -= HSPACE * 2.0f;
	frame.bottom -= VSPACE * 2.0f;
	sampleBox = new BBox(frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	AddChild(sampleBox);

	// Make the view area a little bit smaller
	frame = sampleBox->Bounds();
	frame.InsetBy(HSPACE, VSPACE);

	// Create the save button
	message = new BMessage(SAM_BUTTON_SAVE);
	label.LoadString(res, IDS_SI_BUT_SAVE);
	saveButton = new BButton(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, (labelPtr = label.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	saveButton->GetPreferredSize(&controlWidth, &controlHeight);
	x = frame.right - controlWidth - 2.0f;
	y = frame.bottom - controlHeight - 1.0f;
	saveButton->ResizeTo(controlWidth, controlHeight);
	saveButton->MoveTo(x, y);
	label.FreeBuffer(labelPtr);

	// Create the format pop-up menu
	formatPop = new BPopUpMenu("");

	// Add the converter names into the format pop-up box
	maxWidth = 0.0f;

	count = windowSystem->converterAddOns.CountItems();
	for (i = 0; i < count; i++)
	{
		APAddOnInformation *info = windowSystem->converterAddOns.GetItem(i);

		if (info->pluginFlags & apcSaver)
		{
			message  = new BMessage(SAM_LIST_FORMAT);
			strPtr   = info->name.GetString();
			maxWidth = max(maxWidth, StringWidth(strPtr));
			formatPop->AddItem(new BMenuItem(strPtr, message));
			info->name.FreeBuffer(strPtr);
		}
	}

	label.LoadString(res, IDS_SI_POP_FORMAT);
	labelPtr = label.GetString();
	w = StringWidth(labelPtr) + HSPACE;

	x -= (maxWidth + w + HSPACE * 7.0f);
	formatField = new BMenuField(BRect(x, y, frame.right - HSPACE, y), NULL, labelPtr, formatPop, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP);
	formatField->SetDivider(w);
	label.FreeBuffer(labelPtr);

	// Add the column listview
	frame.left    += HSPACE;
	frame.right   -= (HSPACE + B_V_SCROLL_BAR_WIDTH);
	frame.bottom  -= (VSPACE + B_H_SCROLL_BAR_HEIGHT + (frame.Height() - y) + VSPACE + 1.0f);
	frame.top     += VSPACE;
	columnListView = new APListViewSample(frame, &containerView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE | B_NAVIGABLE_JUMP, B_SINGLE_SELECTION_LIST, false, true, true, true, B_PLAIN_BORDER);

	// Add the text boxes
	label.Format(res, IDS_SI_OCTAVE, 0, 1);
	octaveText = new BStringView(BRect(frame.left, y, frame.left, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	octaveText->GetPreferredSize(&controlWidth, &h);
	octaveText->ResizeTo(controlWidth, h);
	y += (controlHeight - h) / 2.0f;
	octaveText->MoveBy(0.0f, (controlHeight - h) / 2.0f);
	label.FreeBuffer(labelPtr);

	tempStr.LoadString(res, IDS_SI_OFF);
	label.Format_S1(res, IDS_SI_POLYPHONY, tempStr);
	x = frame.left + controlWidth + HSPACE * 5.0f;
	polyText = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	polyText->ResizeToPreferred();
	label.FreeBuffer(labelPtr);

	// Initialize the switch position
	BRect rect(columnListView->Bounds());
	switchPos = rect.top;

	// Get the save format
	saveFormat = windowSystem->useSettings->GetStringEntryValue("Window", "SampleSampSaveFormat");

	// See if the name exists
	found = false;
	count = formatPop->CountItems();
	for (i = 0; i < count; i++)
	{
		if (saveFormat.CompareNoCase(formatPop->ItemAt(i)->Label()) == 0)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		// Well, we didn't find the converter, so reset the setting
		// to the first one or an empty string if no converter are loaded
		if (count == 0)
			saveFormat.MakeEmpty();
		else
			saveFormat = formatPop->ItemAt(0)->Label();
	}

	// Select the item in the popup-list
	BMenuItem *item = formatPop->FindItem((strPtr = saveFormat.GetString()));
	if (item != NULL)
		item->SetMarked(true);

	saveFormat.FreeBuffer(strPtr);

	// Add the columns
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol1W");
	columnListView->AddColumn(new CLVColumn(NULL, colWidth, CLV_SORT_KEYABLE));

	label.LoadString(res, IDS_SI_SCOL_NAME);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol2W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_LENGTH);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol3W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_LOOPSTART);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol4W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_LOOPLENGTH);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol5W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_BITSIZE);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol6W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_VOLUME);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol7W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_PANNING);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol8W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_MIDDLEC);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol9W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_INFO);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol10W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	label.LoadString(res, IDS_SI_SCOL_TYPE);
	colWidth = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol11W");
	columnListView->AddColumn(new CLVColumn((labelPtr = label.GetString()), colWidth, CLV_SORT_KEYABLE));
	label.FreeBuffer(labelPtr);

	// Get the order
	order[0]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol1Pos");
	order[1]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol2Pos");
	order[2]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol3Pos");
	order[3]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol4Pos");
	order[4]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol5Pos");
	order[5]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol6Pos");
	order[6]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol7Pos");
	order[7]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol8Pos");
	order[8]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol9Pos");
	order[9]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol10Pos");
	order[10] = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampCol11Pos");
	columnListView->SetDisplayOrder(order);

	// Attach the views to the window
	sampleBox->AddChild(containerView);
	sampleBox->AddChild(octaveText);
	sampleBox->AddChild(polyText);
	sampleBox->AddChild(formatField);
	sampleBox->AddChild(saveButton);

	// Set the sort function and modes
	sortKey[0]  = windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampSortKey");
	sortMode[0] = (CLVSortMode)windowSystem->useSettings->GetIntEntryValue("Window", "SampleSampSortMode");

	columnListView->SetSortFunction(APListItemSample::SortFunction);
	columnListView->SetSorting(1, sortKey, sortMode);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APViewSamples::~APViewSamples(void)
{
	RemoveItems();

	delete filePanel;
	delete messenger;
}



/******************************************************************************/
/* SaveSettings() will save the view specific setting entries.                */
/******************************************************************************/
void APViewSamples::SaveSettings(void)
{
	int32 sortKey[1];
	CLVSortMode sortMode[1];
	CLVColumn *column;
	int32 width[11];
	int32 order[11];

	// Get the display order
	columnListView->GetDisplayOrder(&order[0]);

	// Get the sort information
	columnListView->GetSorting(sortKey, sortMode);

	// Get the column widths
	for (int32 i = 0; i < 11; i++)
	{
		// Get the column sizes
		column   = columnListView->ColumnAt(i);
		width[i] = (uint32)column->Width();
	}

	try
	{
		// Do we need to store the positions
		if ((windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol1Pos") != order[0]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol2Pos") != order[1]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol3Pos") != order[2]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol4Pos") != order[3]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol5Pos") != order[4]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol6Pos") != order[5]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol7Pos") != order[6]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol8Pos") != order[7]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol9Pos") != order[8]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol10Pos") != order[9]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol11Pos") != order[10]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol1W") != width[0]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol2W") != width[1]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol3W") != width[2]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol4W") != width[3]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol5W") != width[4]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol6W") != width[5]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol7W") != width[6]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol8W") != width[7]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol9W") != width[8]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol10W") != width[9]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampCol11W") != width[10]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampSortKey") != sortKey[0]) ||
			(windowSystem->saveSettings->GetIntEntryValue("Window", "SampleSampSortMode") != sortMode[0]) ||
			(windowSystem->saveSettings->GetStringEntryValue("Window", "SampleSampSaveFormat") != saveFormat))
		{
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol1Pos", order[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol2Pos", order[1]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol3Pos", order[2]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol4Pos", order[3]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol5Pos", order[4]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol6Pos", order[5]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol7Pos", order[6]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol8Pos", order[7]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol9Pos", order[8]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol10Pos", order[9]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol11Pos", order[10]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol1W", width[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol2W", width[1]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol3W", width[2]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol4W", width[3]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol5W", width[4]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol6W", width[5]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol7W", width[6]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol8W", width[7]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol9W", width[8]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol10W", width[9]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampCol11W", width[10]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampSortKey", sortKey[0]);
			windowSystem->saveSettings->WriteIntEntryValue("Window", "SampleSampSortMode", sortMode[0]);
			windowSystem->saveSettings->WriteStringEntryValue("Window", "SampleSampSaveFormat", saveFormat);
		}
	}
	catch(...)
	{
		throw;
	}
}



/******************************************************************************/
/* HandleMessage() is called for each message the window don't know.          */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APViewSamples::HandleMessage(BMessage *msg)
{
	int32 value;

	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////////
		// Save format list menu
		////////////////////////////////////////////////////////////////////////
		case SAM_LIST_FORMAT:
		{
			BMenuItem *item;

			value      = msg->FindInt32("index");
			item       = formatPop->ItemAt(value);
			saveFormat = item->Label();
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Save button
		////////////////////////////////////////////////////////////////////////
		case SAM_BUTTON_SAVE:
		{
			PString title, msg;

			// Check to see if any samples are selected
			int32 selected = columnListView->CurrentSelection();
			if (selected == -1)
			{
				// Show a message
				title.LoadString(res, IDS_SAMPLE_INFO_TITLE);
				msg.LoadString(res, IDS_ERR_SAMPLE_NOSAMPLE);

				PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
				alert.Show();
			}
			else
			{
				// Is there selected a converter?
				if (saveFormat.IsEmpty())
				{
					// Show a message
					title.LoadString(res, IDS_SAMPLE_INFO_TITLE);
					msg.LoadString(res, IDS_ERR_SAMPLE_NOCONVERTER);

					PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
					alert.Show();
				}
				else
				{
					BMessage message(B_SAVE_REQUESTED);
					APListItemSample *item;
					const APSampleInfo *sampInfo;
					PString name;
					char *nameStr;

					// Get the sample information
					item     = (APListItemSample *)columnListView->ItemAt(selected);
					sampInfo = item->GetSampleInfo();

					// Check the sample length
					if (sampInfo->length == 0)
					{
						title.LoadString(res, IDS_SAMPLE_INFO_TITLE);
						msg.LoadString(res, IDS_ERR_SAMPLE_NOLENGTH);

						PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
						alert.Show();
					}
					else
					{
						if (sampInfo->type != apSample)
						{
							title.LoadString(res, IDS_SAMPLE_INFO_TITLE);
							msg.LoadString(res, IDS_ERR_SAMPLE_NOTASAMPLE);

							PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
							alert.Show();
						}
						else
						{
							// Prepare the message
							message.AddInt32("SampleNum", selected);

							// Allocate the messenger
							if (messenger == NULL)
							{
								messenger = new BMessenger(this);
								if (messenger == NULL)
									break;
							}

							// Allocate the panel
							if (filePanel == NULL)
							{
								filePanel = new BFilePanel(B_SAVE_PANEL, messenger, NULL, 0, false);
								if (filePanel == NULL)
									break;
							}

							// Set the save message
							filePanel->SetMessage(&message);

							// Put in the sample name as the filename
							name = sampInfo->name;
							name.TrimRight();
							name.TrimLeft();
							filePanel->SetSaveText((nameStr = name.GetString()));
							name.FreeBuffer(nameStr);

							// Show the panel
							filePanel->Show();
						}
					}
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// Message from the file panel
		////////////////////////////////////////////////////////////////////////
		case B_SAVE_REQUESTED:
		{
			entry_ref entRef;
			BEntry entry;
			BPath path;
			const char *fileName;
			APAddOnConverter *converter = NULL;
			float *buffer = NULL;

			// Extract the directory and file name
			if (msg->FindRef("directory", &entRef) == B_OK)
			{
				entry.SetTo(&entRef);
				entry.GetPath(&path);

				if (msg->FindString("name", &fileName) == B_OK)
				{
					// Well, now it's time to save the module list
					path.Append(fileName);

					try
					{
						PFile file;
						PString fileName;
						APListItemSample *item;
						const APSampleInfo *sampInfo;
						APConverter_SampleFormat convInfo;
						int32 selected;
						int32 convIndex, index;

						// Check to see if the current selected item
						// is still available
						if (msg->FindInt32("SampleNum", &selected) == B_OK)
						{
							if (selected >= columnListView->CountItems())
							{
								PString title, msg;

								// Show error
								title.LoadString(res, IDS_SAMPLE_INFO_TITLE);
								msg.LoadString(res, IDS_ERR_SAMPLE_NOTAVAILABLE);

								PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
								alert.Show();
								break;
							}

							// Get the sample information
							item     = (APListItemSample *)columnListView->ItemAt(selected);
							sampInfo = item->GetSampleInfo();

							// Create the converter structure
							convInfo.name       = sampInfo->name;
							convInfo.author.MakeEmpty();
							convInfo.flags      = sampInfo->flags;
							convInfo.bitSize    = sampInfo->bitSize;
							convInfo.frequency  = sampInfo->middleC;
							convInfo.channels   = 1;
							convInfo.volume     = sampInfo->volume;
							convInfo.panning    = sampInfo->panning;
							convInfo.loopStart  = sampInfo->loopStart;
							convInfo.loopLength = sampInfo->loopLength;

							// Allocate converter buffer
							buffer = new float[sampInfo->length];
							if (buffer == NULL)
								throw PMemoryException();

							// Convert the sample to float format
							if (sampInfo->bitSize == 8)
							{
								for (uint32 i = 0; i < sampInfo->length; i++)
									buffer[i] = ((float)((const int8 *)sampInfo->address)[i]) / 128.0f;
							}
							else
							{
								for (uint32 i = 0; i < sampInfo->length; i++)
									buffer[i] = ((float)((const int16 *)sampInfo->address)[i]) / 32768.0f;
							}

							// Allocate the converter
							converter = (APAddOnConverter *)global->GetAddOnInstance(saveFormat, apConverter, &convIndex);
							if (converter == NULL)
								throw PMemoryException();

							// Create the filename
							fileName = path.Path();

							if ((index = fileName.ReverseFind('.')) >= 0)
								fileName = fileName.Left(index);

							// Add extension
							fileName += converter->GetExtension(convIndex);

							// Open the file
							file.Open(fileName, PFile::pModeCreate | PFile::pModeReadWrite);

							// Initialize the saver
							if (!converter->SaverInit(convIndex, &convInfo))
								throw PFileException(P_FILE_ERR_FILE, path.Path());

							try
							{
								// Write the header
								converter->SaveHeader(&file, &convInfo);

								// Write the sample data
								converter->SaveData(&file, buffer, sampInfo->length, &convInfo);

								// Write the tail and what-ever missing
								converter->SaveTail(&file, &convInfo);
							}
							catch(...)
							{
								converter->SaverEnd(convIndex, &convInfo);
								throw;
							}

							// Clean up the saver
							converter->SaverEnd(convIndex, &convInfo);
						}
					}
					catch(PMemoryException e)
					{
						PString title, msg;

						// Show error
						title.LoadString(res, IDS_SAMPLE_INFO_TITLE);
						msg.LoadString(res, IDS_ERR_NO_MEMORY);

						PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
						alert.Show();
					}
					catch(PFileException e)
					{
						PString title, msg, err;
						char *errStr, *nameStr;

						// Show error
						title.LoadString(res, IDS_SAMPLE_INFO_TITLE);
						err = PSystem::GetErrorString(e.errorNum);
						msg.Format(res, IDS_ERR_FILE, (nameStr = e.fileName.GetString()), e.errorNum, (errStr = err.GetString()));
						err.FreeBuffer(errStr);
						e.fileName.FreeBuffer(nameStr);

						PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
						alert.Show();
					}

					// Delete the converter
					global->DeleteAddOnInstance(saveFormat, apConverter, converter);

					// Delete the buffer
					delete[] buffer;
				}
			}
			break;
		}
	}
}



/******************************************************************************/
/* AddItems() will add all the items in the column list view.                 */
/******************************************************************************/
void APViewSamples::AddItems(void)
{
	uint32 handle;
	const PList<APSampleInfo *> *list;
	int32 i, count;

	// Get the current file handle
	handle = windowSystem->loader->GetFileHandle();
	if (handle == 0)
		return;

	// Get the list pointer
	list = global->GetSampleInformationWithLock(handle);

	// Add the items
	count = list->CountItems();
	for (i = 0; i < count; i++)
		columnListView->AddItem(new APListItemSample(res, itemFontHeight, list->GetItem(i), i + 1));

	// Unlock it again
	global->UnlockSampleInformation(handle);

	// Sort the items
	columnListView->SortItems();
}



/******************************************************************************/
/* RemoveItems() will remove all the items in the column list view.           */
/******************************************************************************/
void APViewSamples::RemoveItems(void)
{
	int32 i, count;
	BListItem *item;

	// Remove all the items from the list view
	count = columnListView->CountItems();
	for (i = 0; i < count; i++)
	{
		item = columnListView->RemoveItem((int32)0);
		delete item;
	}

	// Reset the listview position
	switchPos = 0.0f;
}



/******************************************************************************/
/* ChangeOctave() changes the octave text string.                             */
/*                                                                            */
/* Input:  "startOctave" is the start octave number.                          */
/******************************************************************************/
void APViewSamples::ChangeOctave(uint16 startOctave)
{
	PString str;
	char *strPtr;

	str.Format(res, IDS_SI_OCTAVE, startOctave, startOctave + 1);
	octaveText->SetText((strPtr = str.GetString()));
	str.FreeBuffer(strPtr);
}



/******************************************************************************/
/* ChangePolyphony() changes the polyphony text string.                       */
/*                                                                            */
/* Input:  "enable" tells if it has to enable or disable it.                  */
/******************************************************************************/
void APViewSamples::ChangePolyphony(bool enable)
{
	PString str, str1;
	char *strPtr;

	if (enable)
		str1.LoadString(res, IDS_SI_ON);
	else
		str1.LoadString(res, IDS_SI_OFF);

	str.Format_S1(res, IDS_SI_POLYPHONY, str1);
	polyText->SetText((strPtr = str.GetString()));
	str.FreeBuffer(strPtr);
}



/******************************************************************************/
/* AttachedToWindow() is called everytime the view has to be shown.           */
/******************************************************************************/
void APViewSamples::AttachedToWindow(void)
{
	BRect frame;

	// Call base class
	BView::AttachedToWindow();

	// Find the tab view size
	frame = Parent()->Bounds();
	frame.right  -= HSPACE * 2.0f;
	frame.bottom -= VSPACE * 2.0f;

	// Resize the view
	ResizeTo(frame.Width(), frame.Height());

	// Set the focus to the column list view
	columnListView->MakeFocus();

	// Scroll to the position the listview was in when switched to another view
	columnListView->ScrollTo(BPoint(0.0f, switchPos));
}



/******************************************************************************/
/* DetachedFromWindow() is called everytime the view has to be hidden.        */
/******************************************************************************/
void APViewSamples::DetachedFromWindow(void)
{
	// Remember to position of the column view
	switchPos = columnListView->Bounds().top;

	// Call base class
	BView::DetachedFromWindow();
}
