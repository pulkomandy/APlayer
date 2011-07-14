/******************************************************************************/
/* APlayer global data class.                                                 */
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
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PList.h"

// APlayerKit headers
#include "APAddOns.h"
#include "APServerCommunication.h"
#include "APMultiFiles.h"
#include "APList.h"
#include "APGlobal.h"
#include "APGlobalData.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APGlobalData::APGlobalData(void)
{
	// Allocate the server communication object
	communication = new APServerCommunication();
	if (communication == NULL)
		throw PMemoryException();

	// Allocate the multi files object
	multiFiles = new APMultiFiles();
	if (multiFiles == NULL)
		throw PMemoryException();

	// Allocate the file types object
	fileTypes = new APFileTypes();
	if (fileTypes == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APGlobalData::~APGlobalData(void)
{
	// Delete all objects again
	delete fileTypes;
	delete multiFiles;
	delete communication;
}



/******************************************************************************/
/* GetAddOnList() returns a list with all the add-ons with the type given.    */
/*                                                                            */
/* Input:  "type" is the type of add-ons you want returned.                   */
/*         "list" is a reference to the list you want the information stored. */
/******************************************************************************/
void APGlobalData::GetAddOnList(APAddOnType type, APList<APAddOnInformation *> &list)
{
	getAddOnInfo(type, list);
}



/******************************************************************************/
/* FreeAddOnList() frees all the items in the list given.                     */
/*                                                                            */
/* Input:  "list" is a reference to the list you want to free.                */
/******************************************************************************/
void APGlobalData::FreeAddOnList(APList<APAddOnInformation *> &list)
{
	freeAddOnInfo(list);
}



/******************************************************************************/
/* GetInstrumentInformationWithLock() returns a list with all the instruments */
/*      from the module loaded. The list is locked, so remember to call the   */
/*      UnlockInstrumentInformation() function when done.                     */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module you want the         */
/*         instruments from.                                                  */
/*                                                                            */
/* Output: A pointer to the list with all the instruments or NULL for an      */
/*         error. Do not remember the pointers in the list. Make a copy of    */
/*         the items instead.                                                 */
/******************************************************************************/
const PList<APInstInfo *> *APGlobalData::GetInstrumentInformationWithLock(uint32 fileHandle)
{
	// Get the list
	return (getInstInfo(fileHandle));
}



/******************************************************************************/
/* UnlockInstrumentInformation() unlocks the instrument list.                 */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module you have locked.     */
/******************************************************************************/
void APGlobalData::UnlockInstrumentInformation(uint32 fileHandle)
{
	// Unlock the list
	unlockInstInfo(fileHandle);
}



/******************************************************************************/
/* GetSampleInformationWithLock() returns a list with all the samples from    */
/*      the module loaded. The list is locked, so remember to call the        */
/*      UnlockSampleInformation() function when done.                         */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module you want the         */
/*         samples from.                                                      */
/*                                                                            */
/* Output: A pointer to the list with all the samples or NULL for an          */
/*         error. Do not remember the pointers in the list. Make a copy of    */
/*         the items instead.                                                 */
/******************************************************************************/
const PList<APSampleInfo *> *APGlobalData::GetSampleInformationWithLock(uint32 fileHandle)
{
	// Get the list
	return (getSampInfo(fileHandle));
}



/******************************************************************************/
/* UnlockSampleInformation() unlocks the sample list.                         */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module you have locked.     */
/******************************************************************************/
void APGlobalData::UnlockSampleInformation(uint32 fileHandle)
{
	// Unlock the list
	unlockSampInfo(fileHandle);
}



/******************************************************************************/
/* GetAddOnInstance() will allocate an add-on instance and return the         */
/*      pointer.                                                              */
/*                                                                            */
/* Input:  "name" is the name of the add-on to allocate.                      */
/*         "type" is the type of the add-on.                                  */
/*         "index" is a pointer to store the converter index in the add-on.   */
/*                                                                            */
/* Output: A pointer to the add-on or NULL if it couldn't be found.           */
/******************************************************************************/
APAddOnBase *APGlobalData::GetAddOnInstance(PString name, APAddOnType type, int32 *index)
{
	// Allocate the converter
	return (getAddOnInst(name, type, index));
}



/******************************************************************************/
/* DeleteAddOnInstance() will delete the add-on instance you got from the     */
/*      GetAddOnInstance() function.                                          */
/*                                                                            */
/* Input:  "name" is the name of the add-on.                                  */
/*         "type" is the type of the add-on.                                  */
/*         "addOn" is a pointer to the object to delete.                      */
/******************************************************************************/
void APGlobalData::DeleteAddOnInstance(PString name, APAddOnType type, APAddOnBase *addOn)
{
	// Delete the instance
	deleteAddOnInst(name, type, addOn);
}
