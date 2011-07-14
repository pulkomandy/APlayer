/******************************************************************************/
/* APGlobal header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APGlobal_h
#define __APGlobal_h

// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APGlobalData.h"


/******************************************************************************/
/* APGlobal class                                                             */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_APKIT APGlobal : public APGlobalData
{
public:
	APGlobal(void);
	virtual ~APGlobal(void);

	void SetFunctionPointers(GetAddOnInfo getAddOnInfo, FreeAddOnInfo freeAddOnInfo, GetInstInfo getInstInfo, UnlockInstInfo unlockInstInfo, GetSampInfo getSampInfo, UnlockSampInfo unlockSampInfo, GetAddOnInst getAddOnInst, DeleteAddOnInst deleteAddOnInst);
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
