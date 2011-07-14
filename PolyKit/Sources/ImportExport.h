/******************************************************************************/
/* PolyKit shared library import/export functions.                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __POLYKIT_BUILD_h
#define __POLYKIT_BUILD_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* Define the macros needed                                                   */
/******************************************************************************/

// BeOS Intel
#ifdef __INTEL__

#ifdef _BUILDING_POLYKIT_LIBRARY_
#define _IMPEXP_PKLIB		__declspec(dllexport)
#else
#define _IMPEXP_PKLIB		__declspec(dllimport)
#endif

#else

// BeOS PowerPC
#define _IMPEXP_PKLIB
#endif

#endif
