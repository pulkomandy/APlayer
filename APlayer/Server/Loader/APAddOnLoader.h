/******************************************************************************/
/* APAddOnLoader header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APAddOnLoader_h
#define __APAddOnLoader_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"


/******************************************************************************/
/* Add-on functions                                                           */
/******************************************************************************/
typedef void (*LoadFunc)(APGlobalData *, PString fileName);
typedef void (*UnloadFunc)(APGlobalData *);
typedef APAddOnBase *(*AllocateInstanceFunc)(APGlobalData *, PString fileName);
typedef void (*DeleteInstanceFunc)(APGlobalData *, APAddOnBase *addOn);



/******************************************************************************/
/* APAddOnLoader class                                                        */
/******************************************************************************/
class APAddOnLoader
{
public:
	APAddOnLoader(PString addOnFile);
	virtual ~APAddOnLoader(void);

	bool Load(void);
	void Unload(void);

	APAddOnBase *CreateInstance(void) const;
	void DeleteInstance(APAddOnBase *addOn) const;

	int32 GetNameIndex(PString name, APAddOnBase *addOn = NULL);
	bool NameExists(PString name, APAddOnBase *addOn = NULL);

protected:
	PString fileName;

	bool loaded;
	image_id imageID;

	AllocateInstanceFunc allocateInstanceFunc;
	DeleteInstanceFunc deleteInstanceFunc;
};

#endif
