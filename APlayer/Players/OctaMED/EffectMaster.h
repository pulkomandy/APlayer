/******************************************************************************/
/* EffectMaster header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __EffectMaster_h
#define __EffectMaster_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"


/******************************************************************************/
/* EffectMaster class                                                         */
/******************************************************************************/
class EffectMaster
{
public:
	EffectMaster(void);
	virtual ~EffectMaster(void);

//XX	void FreeEffects(void);

	void AddGroup(void);
};

#endif
