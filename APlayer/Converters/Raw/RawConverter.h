/******************************************************************************/
/* Raw Converter header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __RawConverter_h
#define __RawConverter_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"


/******************************************************************************/
/* RawConverter class                                                         */
/******************************************************************************/
class RawConverter : public APAddOnConverter
{
public:
	RawConverter(APGlobalData *global, PString fileName);
	virtual ~RawConverter(void);

	virtual float GetVersion(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual PString GetExtension(int32 index);

	// Saver functions
	virtual bool SaverInit(int32 index, const APConverter_SampleFormat *convInfo);
	virtual void SaverEnd(int32 index, const APConverter_SampleFormat *convInfo);

	virtual ap_result SaveData(PFile *file, const float *buffer, uint32 length, const APConverter_SampleFormat *convInfo);
	virtual ap_result SaveTail(PFile *file, const APConverter_SampleFormat *convInfo);

protected:
	PResource *res;

	// Saver variables
	int8 *saveBuffer;
	uint32 bufLength;
};

#endif
