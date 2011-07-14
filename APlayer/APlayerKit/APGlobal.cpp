/******************************************************************************/
/* APlayer global class.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_APLAYERKIT_

// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APGlobal.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APGlobal::APGlobal(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APGlobal::~APGlobal(void)
{
}



/******************************************************************************/
/* SetFunctionPointers() will initialize all the function pointers in the     */
/*      class.                                                                */
/*                                                                            */
/* Input:  Function pointers.                                                 */
/******************************************************************************/
void APGlobal::SetFunctionPointers(GetAddOnInfo getAddOnInfo, FreeAddOnInfo freeAddOnInfo, GetInstInfo getInstInfo, UnlockInstInfo unlockInstInfo, GetSampInfo getSampInfo, UnlockSampInfo unlockSampInfo, GetAddOnInst getAddOnInst, DeleteAddOnInst deleteAddOnInst)
{
	// Set the function pointers
	this->freeAddOnInfo   = freeAddOnInfo;
	this->getAddOnInfo    = getAddOnInfo;
	this->getInstInfo     = getInstInfo;
	this->unlockInstInfo  = unlockInstInfo;
	this->getSampInfo     = getSampInfo;
	this->unlockSampInfo  = unlockSampInfo;
	this->getAddOnInst    = getAddOnInst;
	this->deleteAddOnInst = deleteAddOnInst;
}
