/******************************************************************************/
/* APlayer File Types class.                                                  */
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

// APlayerKit headers
#include "APAddOns.h"
#include "APList.h"
#include "APFileTypes.h"


/******************************************************************************/
/* Add-on file type structure                                                 */
/******************************************************************************/
typedef struct APAddOnFileType
{
	PString type;					// The mime string
	PString extension;				// File extensions
	PString longDescription;		// Long description
	PString shortDescription;		// Short description
	uint8 smallIcon[16 * 16];		// Small icon
	uint8 largeIcon[32 * 32];		// Large icon
} APAddOnFileType;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APFileTypes::APFileTypes(void)
{
	// Register the APlayer play list file type
	BMimeType mime("text/x-aplayer-playlist");
	mime.SetPreferredApp("application/x-vnd.Polycode.APlayer");
	mime.Install();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APFileTypes::~APFileTypes(void)
{
	int32 count, i;

	// Lock the lists
	addOnFileTypeList.LockList();
	systemFileTypeList.LockList();

	// Delete the file type list
	DeleteFileTypeList();

	// Delete the file type list
	count = addOnFileTypeList.CountItems();
	for (i = 0; i < count; i++)
		delete addOnFileTypeList.GetItem(i);

	// Unlock again
	systemFileTypeList.UnlockList();
	addOnFileTypeList.UnlockList();
}



/******************************************************************************/
/* RegisterFileType() installs the file type with icons.                      */
/*                                                                            */
/* Input:  "resource" is a pointer to the resource object holding the file    */
/*         type icons.                                                        */
/*         "iconID" is the ID of the icons, both the small and large.         */
/*         "type" is the module type string.                                  */
/*         "extension" is the file extension.                                 */
/*         "longDescription" is the long description of the add-on.           */
/*         "shortDescription" is a short description of the mime type.        */
/******************************************************************************/
void APFileTypes::RegisterFileType(PResource *resource, int32 iconID, PString type, PString extension, PString longDescription, PString shortDescription)
{
	APAddOnFileType *typeItem;

	// Allocate some memory to hold the filetype item
	typeItem = new APAddOnFileType;
	if (typeItem == NULL)
		throw PMemoryException();

	// Fill out the mime item
	typeItem->type             = type;
	typeItem->extension        = extension;
	typeItem->longDescription  = longDescription;
	typeItem->shortDescription = shortDescription;

	// Make empty icons in case the icon doesn't exists
	memset(typeItem->smallIcon, 0xff, sizeof(typeItem->smallIcon));
	memset(typeItem->largeIcon, 0xff, sizeof(typeItem->largeIcon));

	// Get the icon data
	resource->LoadResource(P_RES_SMALL_ICON | P_RES_LARGE_ICON);
	resource->GetItem(P_RES_SMALL_ICON, iconID, typeItem->smallIcon, sizeof(typeItem->smallIcon));
	resource->GetItem(P_RES_LARGE_ICON, iconID, typeItem->largeIcon, sizeof(typeItem->largeIcon));

	// Lock the list
	addOnFileTypeList.LockList();

	// Add the information to the list
	addOnFileTypeList.AddTail(typeItem);

	// Unlock again
	addOnFileTypeList.UnlockList();
}



/******************************************************************************/
/* RegisterFileTypeInSystem() installs the file type with icons.              */
/*                                                                            */
/* Input:  "addOnNum" is the add-on number to register.                       */
/******************************************************************************/
void APFileTypes::RegisterFileTypeInSystem(int32 addOnNum)
{
	APAddOnFileType *fileType;

	// Lock the list
	addOnFileTypeList.LockList();

	// Get the item with the add-on information
	fileType = addOnFileTypeList.GetItem(addOnNum);

	// And register it
	RegisterFileTypeInSystem(fileType);

	// Unlock again
	addOnFileTypeList.UnlockList();
}



/******************************************************************************/
/* RegisterAPlayerFileTypesInSystem() will register all the file types in the */
/*      system that only APlayer is the preferred application or does not     */
/*      exists at all.                                                        */
/******************************************************************************/
void APFileTypes::RegisterAPlayerFileTypesInSystem(void)
{
	BMimeType mime;
	char strBuf[B_MIME_TYPE_LENGTH];
	APAddOnFileType *addOnType;
	char *typeStr;
	int32 count, i;
	bool registerIt;

	// Lock the list
	addOnFileTypeList.LockList();

	// Register all the file types
	count = addOnFileTypeList.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get a player file type
		addOnType = addOnFileTypeList.GetItem(i);

		// Initialize the file type
		if (mime.SetTo((typeStr = addOnType->type.GetString())) == B_OK)
		{
			registerIt = false;

			// Get the preferred application
			if (mime.GetPreferredApp(strBuf) == B_OK)
			{
				// If the preferred application is APlayer, register it
				if (((PString)strBuf).CompareNoCase("application/x-vnd.Polycode.APlayer") == 0)
					registerIt = true;
			}
			else
			{
				// No preferred application is assigned to the mime-type,
				// so register it
				registerIt = true;
			}

			if (registerIt)
				RegisterFileTypeInSystem(addOnType);
		}

		// Free the string buffer
		addOnType->type.FreeBuffer(typeStr);
	}

	// Unlock the lists again
	addOnFileTypeList.UnlockList();

	// Build the system file type list
	BuildFileTypeList();
}



/******************************************************************************/
/* BuildFileTypeList() will scan the filetype list. For each filetype, it     */
/*      will look it up in the system database and get the information there  */
/*      and store them in a list which can then be retrieved later.           */
/******************************************************************************/
void APFileTypes::BuildFileTypeList(void)
{
	BMimeType mime;
	BMessage ext;
	BBitmap bitmap(BRect(0.0f, 0.0f, 15.0f, 15.0f), B_CMAP8);
	char strBuf[B_MIME_TYPE_LENGTH];
	APAddOnFileType *addOnType;
	APSystemFileType *systemType;
	char *typeStr;
	int32 count, i;

	// Lock the lists
	addOnFileTypeList.LockList();
	systemFileTypeList.LockList();

	try
	{
		// Clear the file type list first
		DeleteFileTypeList();

		// Traverse all the file types
		count = addOnFileTypeList.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get a player file type
			addOnType = addOnFileTypeList.GetItem(i);

			// Initialize the file type
			if (mime.SetTo((typeStr = addOnType->type.GetString())) == B_OK)
			{
				// Allocate a new file type
				systemType = new APSystemFileType;
				if (systemType == NULL)
				{
					addOnType->type.FreeBuffer(typeStr);
					throw PMemoryException();
				}

				// Set the add-on number
				systemType->addonNum = i;

				// Set the mime string
				systemType->mime = addOnType->type;

				// Get the description
				if (mime.GetShortDescription(strBuf) == B_OK)
					systemType->description = strBuf;

				// Get preferred application
				if (mime.GetPreferredApp(strBuf) == B_OK)
				{
					BMimeType appMime(strBuf);
					entry_ref entry;

					// Get the filename to the preferred application
					if (appMime.GetAppHint(&entry) == B_OK)
						systemType->preferredApp = entry.name;
				}

				// Get the file extensions
				ext.MakeEmpty();
				if (mime.GetFileExtensions(&ext) == B_OK)
				{
					const char *extStr;
					int32 num = 0;

					while (ext.FindString("extensions", num++, &extStr) == B_OK)
					{
						systemType->extensions += "|.";
						systemType->extensions += extStr;
					}

					// Remove the first pipe character
					systemType->extensions.Delete(0);
				}

				// Get the small icon
				if (mime.GetIcon(&bitmap, B_MINI_ICON) == B_OK)
					memcpy(systemType->icon, bitmap.Bits(), 16 * 16);
				else
					memset(systemType->icon, 0xff, 16 * 16);

				// Add the filetype in the list
				systemFileTypeList.AddTail(systemType);
			}

			// Free the string buffer
			addOnType->type.FreeBuffer(typeStr);
		}
	}
	catch(...)
	{
		// Unlock the lists again
		systemFileTypeList.UnlockList();
		addOnFileTypeList.UnlockList();
		throw;
	}

	// Unlock the lists again
	systemFileTypeList.UnlockList();
	addOnFileTypeList.UnlockList();
}



/******************************************************************************/
/* GetFileTypeList() will return a pointer to the file type list.             */
/*                                                                            */
/* Output: A pointer to the list.                                             */
/******************************************************************************/
APList<APSystemFileType *> *APFileTypes::GetFileTypeList(void)
{
	return (&systemFileTypeList);
}



/******************************************************************************/
/* DeleteFileTypeList() will remove all items in the system list.             */
/******************************************************************************/
void APFileTypes::DeleteFileTypeList(void)
{
	int32 i, count;

	// Empty the list
	count = systemFileTypeList.CountItems();
	for (i = 0; i < count; i++)
		delete systemFileTypeList.GetAndRemoveItem(0);
}



/******************************************************************************/
/* RegisterFileTypeInSystem() will register a single mime type given as the   */
/*      argument.                                                             */
/*                                                                            */
/* Input:  "fileType" is a pointer to a file type structure with the          */
/*         information about the file type to register.                       */
/******************************************************************************/
void APFileTypes::RegisterFileTypeInSystem(APAddOnFileType *fileType)
{
	PString ext;
	char *typeStr, *extStr, *longStr, *shortStr;
	BMimeType mime((typeStr = fileType->type.GetString()));
	BBitmap small(BRect(0.0f, 0.0f, 15.0f, 15.0f), B_CMAP8);
	BBitmap large(BRect(0.0f, 0.0f, 31.0f, 31.0f), B_CMAP8);
	BMessage msg;
	int32 startPos, pos;

	// Free the string buffer
	fileType->type.FreeBuffer(typeStr);

	// Set the icons
	small.SetBits(fileType->smallIcon, 16 * 16, 0, B_CMAP8);
	large.SetBits(fileType->largeIcon, 32 * 32, 0, B_CMAP8);

	// Set the file extensions
	startPos = 0;
	while ((pos = fileType->extension.Find('|', startPos)) != -1)
	{
		ext = fileType->extension.Mid(startPos, pos - startPos);
		msg.AddString("extensions", (extStr = ext.GetString()));
		ext.FreeBuffer(extStr);
		startPos = pos + 1;
	}

	ext = fileType->extension.Mid(startPos);
	msg.AddString("extensions", (extStr = ext.GetString()));
	ext.FreeBuffer(extStr);

	// Initialize the filetype
	mime.SetIcon(&small, B_MINI_ICON);
	mime.SetIcon(&large, B_LARGE_ICON);
	mime.SetFileExtensions(&msg);
	mime.SetLongDescription((longStr = fileType->longDescription.GetString()));
	mime.SetShortDescription((shortStr = fileType->shortDescription.GetString()));
	mime.SetPreferredApp("application/x-vnd.Polycode.APlayer");
	mime.Install();

	// Free the string buffers
	fileType->shortDescription.FreeBuffer(shortStr);
	fileType->longDescription.FreeBuffer(longStr);
}
