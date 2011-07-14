/******************************************************************************/
/* APWindowAbout header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowAbout_h
#define __APWindowAbout_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "MainWindowSystem.h"
#include "APKeyFilter.h"


/******************************************************************************/
/* APViewAboutBox class                                                       */
/******************************************************************************/
class APViewAboutBox : public BView
{
public:
	APViewAboutBox(MainWindowSystem *system, BRect frame);
	virtual ~APViewAboutBox(void);

	virtual void Pulse(void);

	uint32 waitCount;

protected:
	enum mode { apText, apLogo, apAddOns, apEmpty };

	virtual void AttachedToWindow(void);
	virtual void Draw(BRect updateRect);

	void UnpackSLZ(uint8 *inBuffer, uint8 *outBuffer);

	MainWindowSystem *windowSystem;

	BRect scrollRect;
	BBitmap *scrollWin;
	BView *writeView;

	uint32 extraHeight;
	uint32 baseLine;

	uint32 counter;
	uint32 currentStringNum;

	rgb_color color;
	uint32 linesToScroll;

	mode showMode;

	uint8 *logo;
	uint32 logoLine;

	APList<APAddOnInformation *> *addOnList;
	int32 addOnIndex;
};



/******************************************************************************/
/* APWindowAbout class                                                        */
/******************************************************************************/
class APWindowAbout : public BWindow
{
public:
	APWindowAbout(MainWindowSystem *system, BRect frame, PString title);
	virtual ~APWindowAbout(void);

	virtual bool QuitRequested(void);

protected:
	virtual void MessageReceived(BMessage *msg);

	MainWindowSystem *windowSystem;
	APKeyFilter *keyFilter;

	APViewAboutBox *topView;
};

#endif
