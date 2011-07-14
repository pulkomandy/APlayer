/******************************************************************************/
/* APFileTypes header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APFileTypes_h
#define __APFileTypes_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APAddOns.h"
#include "APList.h"


/******************************************************************************/
/* APSystemFileType structure                                                 */
/******************************************************************************/
typedef struct APSystemFileType
{
	int32 addonNum;					// The add-on number. Use this if you want to register it
	PString mime;					// The mime string
	PString description;			// The description
	PString preferredApp;			// The preferred application this file type has
	PString extensions;				// The file extensions separated with a | character
	uint8 icon[16 * 16];			// The small icon
} APSystemFileType;



/******************************************************************************/
/* APFileTypes class                                                          */
/******************************************************************************/
struct APAddOnFileType;

#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_APKIT APFileTypes
{
public:
	APFileTypes(void);
	virtual ~APFileTypes(void);

	void RegisterFileType(PResource *resource, int32 iconID, PString type, PString extension, PString longDescription, PString shortDescription);

	void RegisterFileTypeInSystem(int32 addOnNum);
	void RegisterAPlayerFileTypesInSystem(void);

	void BuildFileTypeList(void);
	APList<APSystemFileType *> *GetFileTypeList(void);

protected:
	void DeleteFileTypeList(void);

	void RegisterFileTypeInSystem(APAddOnFileType *fileType);

	APList<APAddOnFileType *> addOnFileTypeList;
	APList<APSystemFileType *> systemFileTypeList;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
