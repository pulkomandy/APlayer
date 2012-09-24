/******************************************************************************/
/* Import / Export header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __IMPORT_EXPORT_h
#define __IMPORT_EXPORT_h

// PolyKit headers
#include "POS.h"
#include "PString.h"


/******************************************************************************/
/* Define the library import / export macros                                  */
/******************************************************************************/

// BeOS
#if __p_os == __p_beos

// BeOS Intel version
#if defined(__BeOS) && defined(__INTEL__)

#ifdef _BUILDING_APLAYERKIT_
#define _IMPEXP_APKIT		__declspec(dllexport)
#else
#define _IMPEXP_APKIT		__declspec(dllimport)
#endif

#ifdef _BUILDING_APLAYER_ADDON_
#define _IMPEXP_APADDON		__declspec(dllexport)
#else
#define _IMPEXP_APADDON		__declspec(dllimport)
#endif

#else

// BeOS PowerPC version
#define _IMPEXP_APKIT
#define _IMPEXP_APADDON
#endif

// Windows
#elif __p_os == __p_windows

#ifdef _BUILDING_APLAYERKIT_
#define _IMPEXP_APKIT		__declspec(dllexport)
#else
#define _IMPEXP_APKIT		__declspec(dllimport)
#endif

#ifdef _BUILDING_APLAYER_ADDON_
#define _IMPEXP_APADDON		__declspec(dllexport)
#else
#define _IMPEXP_APADDON		__declspec(dllimport)
#endif

#endif



/******************************************************************************/
/* Add-On functions                                                           */
/******************************************************************************/
class APGlobalData;
class APAddOnBase;

#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

extern "C"
{
_IMPEXP_APADDON	void Load(APGlobalData *global, PString fileName);
_IMPEXP_APADDON	void Unload(APGlobalData *global);
_IMPEXP_APADDON	APAddOnBase *AllocateInstance(APGlobalData *global, PString fileName);
_IMPEXP_APADDON	void DeleteInstance(APGlobalData *global, APAddOnBase *addOn);
}
#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
