/******************************************************************************/
/* PResource implementation file.                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_POLYKIT_LIBRARY_

// PolyKit headers
#include "POS.h"
#include "PDirectory.h"
#include "PSkipList.h"
#include "PResource.h"


/******************************************************************************/
/* PResource class                                                            */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "fileName" is the name of the file where the resource is stored.   */
/*         The name is a relative path from the launch directory.             */
/******************************************************************************/
PResource::PResource(PString fileName)
{
	// Initialize member variables
	loadedTypes = 0;

	// Remember the file name
	fileStr = fileName;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PResource::~PResource(void)
{
}



/******************************************************************************/
/* LoadResource() will load all the resource types given into memory.         */
/*                                                                            */
/* Input:  "type" is a bitmask with all the resource types to load.           */
/******************************************************************************/
void PResource::LoadResource(uint32 type)
{
	PDirectory dir;
	PString filePath;
	BFile file;
	char *pathStr;

	// First check if we need to load any of the types
	if ((loadedTypes & type) == type)
		return;			// All types given has been loaded

	// Find the full path to the file
	dir.FindDirectory(PDirectory::pLaunch);
	dir.Append(dir.GetDirectoryPart(fileStr));
	filePath = dir.GetDirectory() + dir.GetFilePart(fileStr);

	// Initialize the file
	if (file.SetTo((pathStr = filePath.GetString()), B_READ_ONLY) != B_OK)
	{
		filePath.FreeBuffer(pathStr);
		return;
	}

	filePath.FreeBuffer(pathStr);

	// Initialize the resource object
	if (resource.SetTo(&file) != B_OK)
		return;

	// Check if the string type has to be loaded
	if ((type & P_RES_STRING) && (!(loadedTypes & P_RES_STRING)))
	{
		// Load the string resource
		if (resource.PreloadResourceType(ConvertType(P_RES_STRING)) == B_OK)
		{
			// Done with the loading of strings
			loadedTypes |= P_RES_STRING;
		}
	}

	// Check if the large icon type has to be loaded
	if ((type & P_RES_LARGE_ICON) && (!(loadedTypes & P_RES_LARGE_ICON)))
	{
		// Load the icon resource
		if (resource.PreloadResourceType(ConvertType(P_RES_LARGE_ICON)) == B_OK)
		{
			// Done with the loading of icons
			loadedTypes |= P_RES_LARGE_ICON;
		}
	}

	// Check if the small icon type has to be loaded
	if ((type & P_RES_SMALL_ICON) && (!(loadedTypes & P_RES_SMALL_ICON)))
	{
		// Load the icon resource
		if (resource.PreloadResourceType(ConvertType(P_RES_SMALL_ICON)) == B_OK)
		{
			// Done with the loading of icons
			loadedTypes |= P_RES_SMALL_ICON;
		}
	}

	// Check if the cursor type has to be loaded
	if ((type & P_RES_CURSOR) && (!(loadedTypes & P_RES_CURSOR)))
	{
		// Load the icon resource
		if (resource.PreloadResourceType(ConvertType(P_RES_CURSOR)) == B_OK)
		{
			// Done with the loading of cursors
			loadedTypes |= P_RES_CURSOR;
		}
	}

	// Check if the bitmap type has to be loaded
	if ((type & P_RES_BITMAP) && (!(loadedTypes & P_RES_BITMAP)))
	{
		// Load the icon resource
		if (resource.PreloadResourceType(ConvertType(P_RES_BITMAP)) == B_OK)
		{
			// Done with the loading of cursors
			loadedTypes |= P_RES_BITMAP;
		}
	}
}



/******************************************************************************/
/* GetItemLength() returns the length of the resource item given.             */
/*                                                                            */
/* Input:  "type" is the resource type the item have.                         */
/*         "id" is the resource item id.                                      */
/*                                                                            */
/* Output: The length of the item.                                            */
/******************************************************************************/
int32 PResource::GetItemLength(uint32 type, int32 id)
{
	type_code typeCode;
	const char *name;
	size_t length;

	// Is the resource type loaded?
	if (!(loadedTypes & type))
		return (0);

	// Convert the type to a type code
	typeCode = ConvertType(type);

	// Get the resource info
	if (!resource.GetResourceInfo(typeCode, id, &name, &length))
		return (0);

	// Return the length
	return (length);
}



/******************************************************************************/
/* GetItem() store the resource item in the buffer given.                     */
/*                                                                            */
/* Input:  "type" is the resource type the item have.                         */
/*         "id" is the resource item id.                                      */
/*         "buffer" is a pointer to the buffer where the item is stored.      */
/*         "length" is the length of the buffer.                              */
/******************************************************************************/
void PResource::GetItem(uint32 type, int32 id, void *buffer, int32 length)
{
	type_code typeCode;
	const void *resBuf;
	size_t outSize;

	// Is the resource type loaded?
	if (!(loadedTypes & type))
		return;

	// Convert the type to a type code
	typeCode = ConvertType(type);

	// Get the resource item
	resBuf = resource.LoadResource(typeCode, id, &outSize);
	if (resBuf == NULL)
		return;

	// Copy the resource item
	memcpy(buffer, resBuf, min((size_t)length, outSize));
}



/******************************************************************************/
/* ConvertType() converts the type given to a BeOS type code.                 */
/*                                                                            */
/* Input:  "type" is the resource type.                                       */
/*                                                                            */
/* Output: A BeOS type code.                                                  */
/******************************************************************************/
type_code PResource::ConvertType(uint32 type)
{
	switch (type)
	{
		case P_RES_STRING:
			return (B_STRING_TYPE);

		case P_RES_LARGE_ICON:
			return ('ICON');

		case P_RES_SMALL_ICON:
			return ('MICN');

		case P_RES_CURSOR:
			return ('CURS');

		case P_RES_BITMAP:
			return ('BBMP');
	}

	// Unknown type
	ASSERT(false);
	return (0);
}
