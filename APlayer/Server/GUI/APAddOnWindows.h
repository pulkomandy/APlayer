/******************************************************************************/
/* APAddOnWindows header file.                                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APAddOnWindows_h
#define __APAddOnWindows_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSettings.h"

// APlayerKit headers
#include "APAddOns.h"
#include "APList.h"

// Server headers
#include "APApplication.h"
#include "APWindowAddOnConfig.h"


/******************************************************************************/
/* Add-On opened config window structure                                      */
/******************************************************************************/
typedef struct APOpenedConfigWindow
{
	PString name;					// The name of the add-on
	APWindowAddOnConfig *window;	// A pointer to the window
	PSettings firstBackupSettings;	// Backup of the first or last saved settings
} APOpenedConfigWindow;



/******************************************************************************/
/* Add-On opened display window structure                                     */
/******************************************************************************/
class APAddOnWindowFilter;

typedef struct APOpenedDisplayWindow
{
	PString name;					// The name of the add-on
	BWindow *window;				// A pointer to the window
	APAddOnWindowFilter *filter;	// Is used to get B_QUIT_REQUESTED messages
} APOpenedDisplayWindow;


class APAddOnWindows;

/******************************************************************************/
/* APAddOnDisplayWindowFilter class                                           */
/******************************************************************************/
class APAddOnWindowFilter : public BMessageFilter
{
public:
	APAddOnWindowFilter(APAddOnWindows *windowObject);
	virtual ~APAddOnWindowFilter(void);

	virtual filter_result Filter(BMessage *message, BHandler **target);

protected:
	APAddOnWindows *winObj;
};


/******************************************************************************/
/* APAddOnWindows class                                                       */
/******************************************************************************/
class APAddOnWindows
{
public:
	APAddOnWindows(void);
	virtual ~APAddOnWindows(void);

	void FindWindowAddOns(APMRSWList<AddOnInfo *> &infoList);
	void InitAddOnWindows(AddOnInfo *addOn);

	void ShowAddOnSettings(AddOnInfo *info, APAddOnBase *addOnInstance, const APConfigInfo *config);
	void ShowAddOnDisplayWindow(AddOnInfo *info, APAddOnBase *addOnInstance, const APDisplayInfo *display);
	void CloseAddOnWindows(AddOnInfo *addOn);
	void CloseWindows(void);

protected:
	friend class APAddOnWindowFilter;

	APList<APOpenedConfigWindow *> configWindows;
	APList<APOpenedDisplayWindow *> displayWindows;
};

#endif
