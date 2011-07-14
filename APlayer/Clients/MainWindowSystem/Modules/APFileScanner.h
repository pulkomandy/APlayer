/******************************************************************************/
/* APFileScanner header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APFileScanner_h
#define __APFileScanner_h

// PolyKit headers
#include "POS.h"
#include "PSynchronize.h"
#include "PTime.h"


/******************************************************************************/
/* APFileScanner class                                                        */
/******************************************************************************/
class MainWindowSystem;

class APFileScanner : public BLooper
{
public:
	APFileScanner(MainWindowSystem *system);
	virtual ~APFileScanner(void);

	void Start(void);
	void Stop(void);

	void ScanItems(int32 startIndex, int32 count);

protected:
	virtual void MessageReceived(BMessage *message);

	void ScanFiles(int32 index, int32 count);
	bool LockWindow(void);
	PTimeSpan GetAttrTime(PString fileName);
	PTimeSpan GetPlayerTime(PString fileName, bool setTime);

	MainWindowSystem *windowSystem;

	PEvent *stopEvent;
};

#endif
