/******************************************************************************/
/* PResource header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PResource_h
#define __PResource_h

#include <Resources.h>
#include <SupportDefs.h>

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "ImportExport.h"


/******************************************************************************/
/* Resource types supported                                                   */
/******************************************************************************/
#define P_RES_STRING				0x00000001
#define P_RES_LARGE_ICON			0x00000002
#define P_RES_SMALL_ICON			0x00000004
#define P_RES_CURSOR				0x00000008
#define P_RES_BITMAP				0x00000010



/******************************************************************************/
/* PResource class                                                            */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PResource
{
public:
	PResource(PString fileName);
	virtual ~PResource(void);

	void LoadResource(uint32 type);

	int32 GetItemLength(uint32 type, int32 id);
	void GetItem(uint32 type, int32 id, void *buffer, int32 length);

protected:
	type_code ConvertType(uint32 type);

	BResources resource;

	PString fileStr;
	uint32 loadedTypes;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
