/******************************************************************************/
/* APError header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APError_h
#define __APError_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* APError class                                                              */
/******************************************************************************/
class APError
{
public:
	static void ShowError(int32 resourceID, ...);
};

#endif
