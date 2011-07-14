/******************************************************************************/
/* ModuleConverter header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ModuleConverter_h
#define __ModuleConverter_h

// PolyKit headers
#include "POS.h"
#include "PFile.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* ModuleConverter class                                                      */
/******************************************************************************/
class ModuleConverter
{
public:
	ModuleConverter(PResource *resource);
	virtual ~ModuleConverter(void);

	virtual bool CheckModule(APAgent_ConvertModule *convInfo) = 0;
	virtual ap_result ConvertModule(APAgent_ConvertModule *convInfo, PFile *destFile) = 0;

protected:
	void ShowError(uint32 id);
	void CopyData(PFile *source, PFile *dest, uint32 length);

	PResource *res;
};



/******************************************************************************/
/* ModConv_FC13 class                                                         */
/******************************************************************************/
class ModConv_FC13 : public ModuleConverter
{
public:
	ModConv_FC13(PResource *resource) : ModuleConverter(resource) {};
	virtual ~ModConv_FC13(void) {};

	virtual bool CheckModule(APAgent_ConvertModule *convInfo);
	virtual ap_result ConvertModule(APAgent_ConvertModule *convInfo, PFile *destFile);
};



/******************************************************************************/
/* ModConv_SFX13 class                                                        */
/******************************************************************************/
class ModConv_SFX13 : public ModuleConverter
{
public:
	ModConv_SFX13(PResource *resource) : ModuleConverter(resource) {};
	virtual ~ModConv_SFX13(void) {};

	virtual bool CheckModule(APAgent_ConvertModule *convInfo);
	virtual ap_result ConvertModule(APAgent_ConvertModule *convInfo, PFile *destFile);
};



/******************************************************************************/
/* ModConv_UMX class                                                          */
/******************************************************************************/
class ModConv_UMX : public ModuleConverter
{
public:
	ModConv_UMX(PResource *resource) : ModuleConverter(resource) {};
	virtual ~ModConv_UMX(void) {};

	virtual bool CheckModule(APAgent_ConvertModule *convInfo);
	virtual ap_result ConvertModule(APAgent_ConvertModule *convInfo, PFile *destFile);
};

#endif
