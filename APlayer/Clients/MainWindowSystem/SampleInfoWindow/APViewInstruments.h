/******************************************************************************/
/* APViewInstruments header file.                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APViewInstruments_h
#define __APViewInstruments_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APGlobalData.h"

// Santa headers
#include <santa/ColumnListView.h>

// Client headers
#include "MainWindowSystem.h"
#include "APListViewInstrument.h"


/******************************************************************************/
/* APViewInstruments class                                                    */
/******************************************************************************/
class APViewInstruments : public BView
{
public:
	APViewInstruments(APGlobalData *glob, MainWindowSystem *system, BRect frame, PString name);
	virtual ~APViewInstruments(void);

	void SaveSettings(void);

	void AddItems(void);
	void RemoveItems(void);

protected:
	virtual void AttachedToWindow(void);
	virtual void DetachedFromWindow(void);

	APGlobalData *global;
	MainWindowSystem *windowSystem;
	PResource *res;

	float itemFontHeight;
	float switchPos;

	BBox *instrumentBox;
	CLVContainerView *containerView;
	APListViewInstrument *columnListView;
};

#endif
