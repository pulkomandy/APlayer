/******************************************************************************/
/* APlayer add-on loader class.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSystem.h"

// APlayerKit headers
#include "APGlobal.h"

// Server headers
#include "APError.h"
#include "APAddOnLoader.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
extern APGlobal *globalData;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APAddOnLoader::APAddOnLoader(PString addOnFile)
{
	// Initialize member variables
	fileName = addOnFile;
	loaded   = false;
	imageID  = -1;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APAddOnLoader::~APAddOnLoader(void)
{
	// Unload the add-on
	Unload();
}



/******************************************************************************/
/* Load() loads the add-on into memory and initialize it.                     */
/*                                                                            */
/* Output: True if it could be loaded, else false.                            */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
bool APAddOnLoader::Load(void)
{
	status_t retVal;
	LoadFunc loadFunc;
	PString err;
	char *errStr, *nameStr;

	// Load the image
	imageID = load_add_on((nameStr = fileName.GetString()));
	fileName.FreeBuffer(nameStr);

	if (imageID >= 0)
	{
		// Call the load function in the add-on, which will initialize the add-on
		if ((retVal = get_image_symbol(imageID, "Load", B_SYMBOL_TYPE_TEXT, (void **)&loadFunc)) != B_OK)
		{
			// Show the error
			retVal = PSystem::ConvertOSError(retVal);
			err    = PSystem::GetErrorString(retVal);
			APError::ShowError(IDS_DEVERR_IMAGEFUNC, "Load", (nameStr = fileName.GetString()), retVal, (errStr = err.GetString()));
			err.FreeBuffer(errStr);
			fileName.FreeBuffer(nameStr);

			// Unload the image again
			unload_add_on(imageID);
			return (false);
		}
		else
		{
			PString addOnName("add-ons");

			// Call the Load() function
			loadFunc(globalData, addOnName + P_DIRSLASH_STR + fileName);

			// Find the AllocateInstance() function
			if ((retVal = get_image_symbol(imageID, "AllocateInstance", B_SYMBOL_TYPE_TEXT, (void **)&allocateInstanceFunc)) != B_OK)
			{
				// Show the error
				retVal = PSystem::ConvertOSError(retVal);
				err    = PSystem::GetErrorString(retVal);
				APError::ShowError(IDS_DEVERR_IMAGEFUNC, "AllocateInstance", (nameStr = fileName.GetString()), retVal, (errStr = err.GetString()));
				err.FreeBuffer(errStr);
				fileName.FreeBuffer(nameStr);

				// Unload the image again
				unload_add_on(imageID);
				return (false);
			}
			else
			{
				// Find the DeleteInstance() function
				if ((retVal = get_image_symbol(imageID, "DeleteInstance", B_SYMBOL_TYPE_TEXT, (void **)&deleteInstanceFunc)) != B_OK)
				{
					// Show the error
					retVal = PSystem::ConvertOSError(retVal);
					err    = PSystem::GetErrorString(retVal);
					APError::ShowError(IDS_DEVERR_IMAGEFUNC, "DeleteInstance", (nameStr = fileName.GetString()), retVal, (errStr = err.GetString()));
					err.FreeBuffer(errStr);
					fileName.FreeBuffer(nameStr);

					// Unload the image again
					unload_add_on(imageID);
					return (false);
				}
				else
				{
					loaded = true;
					return (true);
				}
			}
		}
	}
	else
		printf("Failed to load add-on: %s - %s\n", fileName.GetString(), strerror(imageID));

	return (false);
}



/******************************************************************************/
/* Unload() unloads the add-on from memory.                                   */
/******************************************************************************/
void APAddOnLoader::Unload(void)
{
	UnloadFunc unloadFunc;

	if (loaded)
	{
		// Call the add-ons Unload function
		if (get_image_symbol(imageID, "Unload", B_SYMBOL_TYPE_TEXT, (void **)&unloadFunc) == B_OK)
			unloadFunc(globalData);

		// Unload the image
		unload_add_on(imageID);
		imageID = -1;
		loaded  = false;
	}
}



/******************************************************************************/
/* CreateInstance() will create an add-on instance and return a pointer to it.*/
/*                                                                            */
/* Output: A pointer to the add-on.                                           */
/******************************************************************************/
APAddOnBase *APAddOnLoader::CreateInstance(void) const
{
	PString addOnName("add-ons");

	return (allocateInstanceFunc(globalData, addOnName + P_DIRSLASH_STR + fileName));
}



/******************************************************************************/
/* DeleteInstance() will delete the add-on instance you got from the          */
/*      CreateInstance() function.                                            */
/*                                                                            */
/* Input:  "addOn" is a pointer to the add-on.                                */
/******************************************************************************/
void APAddOnLoader::DeleteInstance(APAddOnBase *addOn) const
{
	deleteInstanceFunc(globalData, addOn);
}



/******************************************************************************/
/* GetNameIndex() returns the index of the name in the add-on.                */
/*                                                                            */
/* Input:  "name" is the the name to search for.                              */
/*         "addOn" is the add-on instance to use or NULL if you don't have    */
/*         one.                                                               */
/*                                                                            */
/* Output: The index to the name or -1 if not found.                          */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
int32 APAddOnLoader::GetNameIndex(PString name, APAddOnBase *addOn)
{
	int32 count, i;
	bool found = false;
	bool allocated = false;

	// First create an instance of the add-on
	if (addOn == NULL)
	{
		addOn     = CreateInstance();
		allocated = true;
	}

	// Get the number of add-ons in the add-on
	count = addOn->GetCount();

	for (i = 0; i < count; i++)
	{
		if (addOn->GetName(i) == name)
		{
			// Found the name
			found = true;
			break;
		}
	}

	// Delete the instance again
	if (allocated)
		DeleteInstance(addOn);

	if (found)
		return (i);

	return (-1);
}



/******************************************************************************/
/* NameExists() checks to see if the name exists in the add-on.               */
/*                                                                            */
/* Input:  "name" is the the name to search for.                              */
/*         "addOn" is the add-on instance to use or NULL if you don't have    */
/*         one.                                                               */
/*                                                                            */
/* Output: True if the name exists, else false.                               */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
bool APAddOnLoader::NameExists(PString name, APAddOnBase *addOn)
{
	return (GetNameIndex(name, addOn) == -1 ? false : true);
}
