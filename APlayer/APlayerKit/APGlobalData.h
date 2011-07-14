/******************************************************************************/
/* APGlobalData header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APGlobalData_h
#define __APGlobalData_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PList.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APAddOns.h"
#include "APServerCommunication.h"
#include "APMultiFiles.h"
#include "APFileTypes.h"
#include "APList.h"


/******************************************************************************/
/* APAddOnInformation structure                                               */
/******************************************************************************/
enum APAddOnType { apPlayer, apAgent, apClient, apConverter };

typedef struct APAddOnInformation
{
	APAddOnType type;					// The type of the add-on
	PString name;						// The name of the add-on
	PString description;				// The description
	float version;						// The version
	bool enabled;						// True if the add-on is enabled
	bool settings;						// True if the add-on have a settings window
	bool display;						// True if the add-on have a display window
	uint32 pluginFlags;					// The flags returned by the GetSupportFlags() function
} APAddOnInformation;



/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
typedef void (*GetAddOnInfo)(APAddOnType type, APList<APAddOnInformation *> &list);
typedef void (*FreeAddOnInfo)(APList<APAddOnInformation *> &list);
typedef const PList<APInstInfo *> *(*GetInstInfo)(uint32 fileHandle);
typedef void (*UnlockInstInfo)(uint32 fileHandle);
typedef const PList<APSampleInfo *> *(*GetSampInfo)(uint32 fileHandle);
typedef void (*UnlockSampInfo)(uint32 fileHandle);
typedef APAddOnBase *(*GetAddOnInst)(PString name, APAddOnType type, int32 *index);
typedef void (*DeleteAddOnInst)(PString name, APAddOnType type, APAddOnBase *converter);



/******************************************************************************/
/* APGlobalData class                                                         */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_APKIT APGlobalData
{
public:
	APGlobalData(void);
	virtual ~APGlobalData(void);

	// Global functions you can use in your add-ons
	void GetAddOnList(APAddOnType type, APList<APAddOnInformation *> &list);
	void FreeAddOnList(APList<APAddOnInformation *> &list);

	const PList<APInstInfo *> *GetInstrumentInformationWithLock(uint32 fileHandle);
	void UnlockInstrumentInformation(uint32 fileHandle);

	const PList<APSampleInfo *> *GetSampleInformationWithLock(uint32 fileHandle);
	void UnlockSampleInformation(uint32 fileHandle);

	APAddOnBase *GetAddOnInstance(PString name, APAddOnType type, int32 *index);
	void DeleteAddOnInstance(PString name, APAddOnType type, APAddOnBase *addOn);

	// Here are some different objects which implements some useful
	// functionalities you can use in your add-ons
	APServerCommunication *communication;	// Use this to communicate with the APlayer server from a client add-on
	APMultiFiles *multiFiles;				// This class can handle files that contains multiple files, like archives and lists
	APFileTypes *fileTypes;					// This class handles all the different file types from e.g. the players

protected:
	// Function pointers to the server
	GetAddOnInfo getAddOnInfo;
	FreeAddOnInfo freeAddOnInfo;
	GetInstInfo getInstInfo;
	UnlockInstInfo unlockInstInfo;
	GetSampInfo getSampInfo;
	UnlockSampInfo unlockSampInfo;
	GetAddOnInst getAddOnInst;
	DeleteAddOnInst deleteAddOnInst;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
