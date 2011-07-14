/******************************************************************************/
/* APWindowAddOnConfig header file.                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APWindowAddOnConfig_h
#define __APWindowAddOnConfig_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSettings.h"

// APlayerKit headers
#include "APAddOns.h"

// Server headers
#include "APApplication.h"
#include "APFilter.h"


/******************************************************************************/
/* Internal messages                                                          */
/******************************************************************************/
#define APCFG_SAVE				'_sav'
#define APCFG_USE				'_use'
#define APCFG_CANCEL			'_can'



/******************************************************************************/
/* APWindowAddOnConfig class                                                  */
/******************************************************************************/
struct APOpenedConfigWindow;

class APWindowAddOnConfig : public BWindow
{
public:
	APWindowAddOnConfig(PString title, AddOnInfo *info, APAddOnBase *addOn, const APConfigInfo *config, APOpenedConfigWindow *windowItem);
	virtual ~APWindowAddOnConfig(void);

	virtual void Quit(void);

protected:
	virtual void MessageReceived(BMessage *msg);

	void CheckAndWriteWindowPositions(void);
	void RefreshSettings(BView *startView);

	APFilter *filter;

	AddOnInfo *addOnInfo;
	APAddOnBase *addOnInstance;
	const APConfigInfo *cfgInfo;
	APOpenedConfigWindow *windowInfo;

	PSettings backupSettings;
	bool closedByCancel;

	BBox *line;
	BButton *saveButton;
	BButton *useButton;
	BButton *cancelButton;
};

#endif
