/******************************************************************************/
/* APWindowUpdate header file.                                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowUpdate_h
#define __APWindowUpdate_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PTime.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "PFile.h"
#include "PSocket.h"
#include "PList.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define UPDATE_RADIO			'_rad'
#define UPDATE_DOWNLOAD			'_dwl'
#define UPDATE_CANCEL			'_can'
#define UPDATE_CHANGEBUTTON		'_chg'



/******************************************************************************/
/* APWindowUpdate class                                                       */
/******************************************************************************/
class APWindowUpdate : public BWindow
{
public:
	APWindowUpdate(MainWindowSystem *system, PString title, uint32 major, uint32 minor, uint32 revision, uint32 beta, uint32 fileSize, PTime release, PList<PString> &ftpUrls, PList<PString> &wwwUrls);
	virtual ~APWindowUpdate(void);

	virtual bool QuitRequested(void);

protected:
	virtual void MessageReceived(BMessage *msg);

	static int32 DownloadThread(void *userData);

	MainWindowSystem *windowSystem;
	PResource *res;
	APKeyFilter *keyFilter;

	PString downloadUrl;

	PEvent *stopEvent;
	PThread downloadThread;

	BView *topView;
	BStringView *updateText[3];
	BStringView *versionText;
	BStringView *fileNameText;
	BStringView *fileSizeText;
	BStringView *releaseText;
	BStringView *downloadText;
	PList<BRadioButton *> urlsRadio;
	BStatusBar *downloadBar;
	BButton *downloadBut;
	BButton *cancelBut;
};

#endif
