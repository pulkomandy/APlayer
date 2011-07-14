/******************************************************************************/
/* EffectMaster Interface.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "EffectGroup.h"
#include "EffectMaster.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
EffectMaster::EffectMaster(void)
{
//XX	AddGroup();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
EffectMaster::~EffectMaster(void)
{
//XX	FreeEffects();
}



/******************************************************************************/
/* FreeEffects()                                                              */
/******************************************************************************/
//XX
/*void EffectMaster::FreeEffects(void)
{
	uint32 cnt;

	for (cnt = 0; cnt < GetNumGroups(); cnt++)
	{
		EffectGroup &grp = GetGroup(cnt);
		grp.FreeEchoBuff();
		delete[] grp.deMixBuff;
		grp.deMixBuff = NULL;
	}
}
*/


/******************************************************************************/
/* AddGroup()                                                                 */
/******************************************************************************/
void EffectMaster::AddGroup(void)
{
//XX	EffectGroup *fg = new EffectGroup();
/*	if (fg != NULL)
	{
		fxGroup.Append(fg);

		SetCurrGroupNum(GetNumGroups() - 1);
	}
*/
}
