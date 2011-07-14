/******************************************************************************/
/* APlayer Update window class.                                               */
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
#include "PTime.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "PDirectory.h"
#include "PFile.h"
#include "PSocket.h"
#include "PAlert.h"
#include "PList.h"
#include "Colors.h"

// APlayerKit headers
#include "Layout.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"
#include "APWindowUpdate.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* APWindowUpdate class                                                       */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APWindowUpdate::APWindowUpdate(MainWindowSystem *system, PString title, uint32 major, uint32 minor, uint32 revision, uint32 beta, uint32 fileSize, PTime release, PList<PString> &ftpUrls, PList<PString> &wwwUrls) : BWindow(BRect(0.0f, 0.0f, 0.0f, 0.0f), NULL, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	BScreen screen;
	BRect scrRect, winRect;
	PString label, url, tempStr;
	char *labelPtr, *tempPtr;
	float x, y, w, h, maxW;
	float controlWidth, controlHeight;
	font_height fh;
	float fontHeight;
	int32 i, count;
	int32 index;
	BMessage *message;
	BRadioButton *radio;

	// Remember the arguments
	windowSystem = system;

	// Remember the resource
	res = windowSystem->res;

	// Initialize member variables
	stopEvent = NULL;

	// Set the window title
	SetTitle((tempPtr = title.GetString()));
	title.FreeBuffer(tempPtr);

	// Create the keyboard filter
	keyFilter = new APKeyFilter(this);
	keyFilter->AddFilterKey(B_ESCAPE, 0);

	// Create background view
	winRect = Bounds();
	topView = new BView(winRect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	// Change the color to white
	topView->SetViewColor(White);

	// Add view to the window
	AddChild(topView);

	// Get other needed information
	topView->GetFontHeight(&fh);
	fontHeight = max(PLAIN_FONT_HEIGHT, ceil(fh.ascent + fh.descent));

	// Create the information text controls
	x = winRect.left + HSPACE;
	y = winRect.top + VSPACE;

	label.LoadString(res, IDS_UPDATE_LINE1);
	updateText[0] = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	updateText[0]->GetPreferredSize(&maxW, &controlHeight);
	updateText[0]->ResizeTo(maxW, controlHeight);
	topView->AddChild(updateText[0]);
	label.FreeBuffer(labelPtr);
	y += controlHeight;

	label.LoadString(res, IDS_UPDATE_LINE2);
	updateText[1] = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	updateText[1]->GetPreferredSize(&controlWidth, &controlHeight);
	updateText[1]->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(updateText[1]);
	label.FreeBuffer(labelPtr);
	maxW = max(maxW, controlWidth);
	y += controlHeight;

	label.LoadString(res, IDS_UPDATE_LINE3);
	updateText[2] = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	updateText[2]->GetPreferredSize(&controlWidth, &controlHeight);
	updateText[2]->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(updateText[2]);
	label.FreeBuffer(labelPtr);
	maxW = max(maxW, controlWidth);
	y += controlHeight * 2.0f;

	// Build the version information
	x += HSPACE * 2.0f;

	if (beta == 65535)
		label.Format(res, IDS_UPDATE_VERSION, major, minor, revision);
	else
		label.Format(res, IDS_UPDATE_VERSION_BETA, major, minor, revision, beta);

	versionText = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	versionText->GetPreferredSize(&controlWidth, &controlHeight);
	versionText->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(versionText);
	label.FreeBuffer(labelPtr);
	maxW = max(maxW, controlWidth);
	y += controlHeight;

	if (ftpUrls.CountItems() != 0)
		url = ftpUrls.GetHead();
	else
		url = wwwUrls.GetHead();

	index   = url.ReverseFind('/');
	tempStr = url.Mid(index + 1);
	label.Format_S1(res, IDS_UPDATE_FILENAME, tempStr);
	fileNameText = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	fileNameText->GetPreferredSize(&controlWidth, &controlHeight);
	fileNameText->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(fileNameText);
	label.FreeBuffer(labelPtr);
	maxW = max(maxW, controlWidth);
	y += controlHeight;

	tempStr = PString::CreateUNumber(fileSize, true);
	label.Format_S1(res, IDS_UPDATE_FILESIZE, tempStr);
	fileSizeText = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	fileSizeText->GetPreferredSize(&controlWidth, &controlHeight);
	fileSizeText->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(fileSizeText);
	label.FreeBuffer(labelPtr);
	maxW = max(maxW, controlWidth);
	y += controlHeight;

	tempStr = release.GetUserDate(true);
	label.Format_S1(res, IDS_UPDATE_RELEASE, tempStr);
	releaseText = new BStringView(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	releaseText->GetPreferredSize(&controlWidth, &controlHeight);
	releaseText->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(releaseText);
	label.FreeBuffer(labelPtr);
	maxW = max(maxW, controlWidth);
	y += controlHeight * 2.0f;

	label.LoadString(res, IDS_UPDATE_DOWNLOAD);
	downloadText = new BStringView(BRect(x - HSPACE * 2.0f, y, x - HSPACE * 2.0f, y), NULL, (labelPtr = label.GetString()), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	downloadText->GetPreferredSize(&controlWidth, &controlHeight);
	downloadText->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(downloadText);
	label.FreeBuffer(labelPtr);
	maxW = max(maxW, controlWidth);
	y += controlHeight * 2.0f;

	// Add all the FTP sites
	count = ftpUrls.CountItems();
	for (i = 0; i < count; i++)
	{
		url = ftpUrls.GetItem(i);

		index = url.Find('/', 6);
		if (index == -1)
			continue;

		tempStr = url.Mid(6, index - 6);
		label.Format_S1(res, IDS_UPDATE_FTP, tempStr);

		message = new BMessage(UPDATE_RADIO);
		message->AddString("url", (tempPtr = url.GetString()));
		url.FreeBuffer(tempPtr);

		radio = new BRadioButton(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message);
		radio->GetPreferredSize(&controlWidth, &controlHeight);
		radio->ResizeTo(controlWidth, controlHeight);
		topView->AddChild(radio);
		label.FreeBuffer(labelPtr);
		maxW = max(maxW, controlWidth);
		y += controlHeight + VSPACE;

		urlsRadio.AddTail(radio);
	}

	// Add all the WWW sites
	count = wwwUrls.CountItems();
	for (i = 0; i < count; i++)
	{
		url = wwwUrls.GetItem(i);

		index = url.Find('/', 7);
		if (index == -1)
			continue;

		tempStr = url.Mid(7, index - 7);
		label.Format_S1(res, IDS_UPDATE_WWW, tempStr);

		message = new BMessage(UPDATE_RADIO);
		message->AddString("url", (tempPtr = url.GetString()));
		url.FreeBuffer(tempPtr);

		radio = new BRadioButton(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message);
		radio->GetPreferredSize(&controlWidth, &controlHeight);
		radio->ResizeTo(controlWidth, controlHeight);
		topView->AddChild(radio);
		label.FreeBuffer(labelPtr);
		maxW = max(maxW, controlWidth);
		y += controlHeight + VSPACE;

		urlsRadio.AddTail(radio);
	}

	// Select the first radio button
	urlsRadio.GetHead()->SetValue(B_CONTROL_ON);
	urlsRadio.GetHead()->Invoke();

	// Add the download bar
	downloadBar = new BStatusBar(BRect(x, y, x - HSPACE * 2.0f + maxW - HSPACE * 2.0f, y), NULL);
	downloadBar->GetPreferredSize(&controlWidth, &controlHeight);
	downloadBar->SetMaxValue(fileSize);
	topView->AddChild(downloadBar);
	y += controlHeight;

	// Add the buttons
	x -= HSPACE * 2.0f;
	y += VSPACE * 4.0f;

	message = new BMessage(UPDATE_DOWNLOAD);
	label.LoadString(res, IDS_BUTTON_DOWNLOAD);
	downloadBut = new BButton(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message, B_FOLLOW_LEFT | B_FOLLOW_TOP);
	downloadBut->GetPreferredSize(&controlWidth, &controlHeight);
	downloadBut->ResizeTo(controlWidth, controlHeight);
	topView->AddChild(downloadBut);
	label.FreeBuffer(labelPtr);

	message = new BMessage(UPDATE_CANCEL);
	label.LoadString(res, IDS_BUTTON_CANCEL);
	cancelBut = new BButton(BRect(x, y, x, y), NULL, (labelPtr = label.GetString()), message, B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	cancelBut->GetPreferredSize(&controlWidth, &controlHeight);
	cancelBut->ResizeTo(controlWidth, controlHeight);
	cancelBut->MoveTo(winRect.right - HSPACE - controlWidth, y);
	topView->AddChild(cancelBut);
	label.FreeBuffer(labelPtr);
	y += controlHeight;

	// Center the window on the screen
	scrRect = screen.Frame();
	w       = maxW + HSPACE * 2.0f;
	h       = y + VSPACE * 2.0f;
	x       = (scrRect.Width() - w) / 2.0f;
	y       = (scrRect.Height() - h) / 2.0f;
	MoveTo(x, y);
	ResizeTo(w, h);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APWindowUpdate::~APWindowUpdate(void)
{
	windowSystem->updateWin = NULL;

	// Delete the filter
	delete keyFilter;
}



/******************************************************************************/
/* QuitRequested() is called when the user presses the close button.          */
/*                                                                            */
/* Output:  Returns true if it's okay to quit, else false.                    */
/******************************************************************************/
bool APWindowUpdate::QuitRequested(void)
{
	// Stop any downloading and delete the downloaded file
	if (stopEvent != NULL)
	{
		stopEvent->SetEvent();
		downloadThread.WaitOnThread();
		delete stopEvent;
		stopEvent = NULL;
	}

	return (true);
}



/******************************************************************************/
/* MessageReceived() is called for each message the window doesn't know.      */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APWindowUpdate::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		////////////////////////////////////////////////////////////////////
		// Drag'n'drop handler
		////////////////////////////////////////////////////////////////////
		case B_SIMPLE_DATA:
		case APLIST_DRAG:
		{
			// Well, the user dropped an object in the window where
			// it's not supported, so we just change the mouse
			// cursor back to normal
			windowSystem->mainWin->SetNormalCursor();
			break;
		}

		////////////////////////////////////////////////////////////////////
		// Global key handler
		////////////////////////////////////////////////////////////////////
		case B_KEY_DOWN:
		{
			BMessage *curMsg;
			int32 key, modifiers;

			// Extract the key that the user has pressed
			curMsg = CurrentMessage();
			if (curMsg->FindInt32("raw_char", &key) == B_OK)
			{
				if (curMsg->FindInt32("modifiers", &modifiers) == B_OK)
				{
					// Mask out lock keys
					modifiers &= ~(B_CAPS_LOCK | B_NUM_LOCK | B_SCROLL_LOCK);

					if ((key == B_ESCAPE) && (modifiers == 0))
					{
						PostMessage(UPDATE_CANCEL);
						return;
					}
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////
		// A radio button
		////////////////////////////////////////////////////////////////////
		case UPDATE_RADIO:
		{
			const char *url;

			// Find the download url
			if (msg->FindString("url", &url) == B_OK)
				downloadUrl = url;

			break;
		}

		////////////////////////////////////////////////////////////////////
		// Download button
		////////////////////////////////////////////////////////////////////
		case UPDATE_DOWNLOAD:
		{
			// Create and initialize the download thread
			stopEvent = new PEvent("Downloader Stop Event", true, false);

			downloadThread.SetHookFunc(DownloadThread, this);
			downloadThread.SetName("MainWindowSystem: Downloader");
			downloadThread.StartThread();

			// Disable the download button
			downloadBut->SetEnabled(false);
			break;
		}

		////////////////////////////////////////////////////////////////////
		// Cancel button
		////////////////////////////////////////////////////////////////////
		case UPDATE_CANCEL:
		{
			// Close the window
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		////////////////////////////////////////////////////////////////////
		// Change the cancel button
		////////////////////////////////////////////////////////////////////
		case UPDATE_CHANGEBUTTON:
		{
			PString label;
			char *labelPtr;

			label.LoadString(res, IDS_BUTTON_OK);
			cancelBut->SetLabel((labelPtr = label.GetString()));
			label.FreeBuffer(labelPtr);
			break;
		}
	}

	// Call base class
	BWindow::MessageReceived(msg);
}



/******************************************************************************/
/* DownloadThread() will download the updated file.                           */
/*                                                                            */
/* Input:  "userData" is a pointer to the current object.                     */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 APWindowUpdate::DownloadThread(void *userData)
{
	APWindowUpdate *object = (APWindowUpdate *)userData;
	PString title, msg;
	PString url(object->downloadUrl);
	PString destName;
	PProxySocket socket;
	PFile *sourceFile = NULL;
	PFile destFile;
	uint8 buffer[1024];
	int32 readLen;
	int32 index;

	try
	{
		// Should we use proxy server?
		if (object->windowSystem->useSettings->GetStringEntryValue("Network", "UseProxy").CompareNoCase("Yes") != 0)
			socket.EnableProxy(false);
		else
			socket.SetProxyServer(PProxySocket::pHTTP, object->windowSystem->useSettings->GetStringEntryValue("Network", "ProxyAddress"));

		// Find the right file type to download
		if (url.Left(6).CompareNoCase("FTP://") == 0)
		{
			sourceFile = new PFTPFile(&socket, 2);
			if (sourceFile == NULL)
				return (0);

			// Initialize the protocol specific data
			((PFTPFile *)sourceFile)->SetLogonName(NETWORK_FTP_LOGON_NAME, NETWORK_FTP_LOGON_PASSWORD);
		}
		else
		{
			if (url.Left(7).CompareNoCase("HTTP://") == 0)
			{
				PString agent;
				uint32 versionMajor = object->windowSystem->versionMajor;
				uint32 versionMiddle = object->windowSystem->versionMiddle;
				uint32 versionMinor = object->windowSystem->versionMinor;
				uint32 versionBeta = object->windowSystem->versionBeta;

				sourceFile = new PHTTPFile(&socket, 2);
				if (sourceFile == NULL)
					return (0);

				// Initialize the protocol specific data
				NETWORK_AGENT_NAME(agent);
				((PHTTPFile *)sourceFile)->SetAgentName(agent);
			}
		}

		// Open the remote file
		sourceFile->Open(url, PFile::pModeRead);

		// Build the download file name
		index    = url.ReverseFind('/');
		destName = object->windowSystem->useSettings->GetStringEntryValue("Network", "DownloadPath");
		destName = PDirectory::EnsureDirectoryName(destName) + url.Mid(index + 1);

		// Open the destination file
		destFile.Open(destName, PFile::pModeWrite | PFile::pModeCreate);

		// Now download the file and write it back
		for (;;)
		{
			// Check the stop event
			if (object->stopEvent->Lock(0) == pSyncOk)
				throw PUserException();

			// Read a bit from the file
			readLen = sourceFile->Read(buffer, sizeof(buffer));
			if (readLen == 0)
				break;

			// Write it to disk
			destFile.Write(buffer, readLen);

			// Update the download bar
			BMessage updateMsg(B_UPDATE_STATUS_BAR);
			updateMsg.AddFloat("delta", readLen);
			object->PostMessage(&updateMsg, object->downloadBar);
		}
	}
	catch(PUserException e)
	{
		// Delete the downloaded file
		destFile.Close();
		if (destFile.FileExists(destName))
			destFile.Remove(destName);

		// Close the file
		if (sourceFile != NULL)
		{
			sourceFile->Close();
			delete sourceFile;
		}

		return (0);
	}
	catch(...)
	{
		// Delete the downloaded file
		destFile.Close();
		if (destFile.FileExists(destName))
			destFile.Remove(destName);

		// Close the file
		if (sourceFile != NULL)
		{
			sourceFile->Close();
			delete sourceFile;
		}

		// Show failure alert
		title.LoadString(object->res, IDS_ERR_TITLE);
		msg.LoadString(object->res, IDS_ERR_DOWNLOAD_FAILED);

		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();

		return (0);
	}

	// Close the files
	destFile.Close();
	sourceFile->Close();
	delete sourceFile;

	// Send a "change button" message
	object->PostMessage(UPDATE_CHANGEBUTTON);

	// Show download done alert
	title.LoadString(object->res, IDS_MAIN_TITLE);
	msg.LoadString(object->res, IDS_UPDATE_DOWNLOAD_DONE);

	PAlert alert(title, msg, PAlert::pInfo, PAlert::pOk);
	alert.Show();

	return (0);
}
