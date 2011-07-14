/******************************************************************************/
/* APlayer module loader class.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PFile.h"
#include "PSystem.h"

// Server headers
#include "APApplication.h"
#include "APModuleLoader.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APModuleLoader::APModuleLoader(void)
{
	// Initialize member variables
	file       = NULL;
	usingFile  = NULL;
	playerInfo = NULL;
	player     = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APModuleLoader::~APModuleLoader(void)
{
}



/******************************************************************************/
/* LoadModule() will try to find a player that understand the file and then   */
/*      load the file into memory.                                            */
/*                                                                            */
/* Input:  "fileName" is the name of the file to load with full path.         */
/*         "changeType" indicates if you want to change the file type.        */
/*         "errorStr" is a reference to a string, where an error will be      */
/*         stored if the load failed.                                         */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APModuleLoader::LoadModule(PString fileName, bool changeType, PString &errorStr)
{
	APAgent_DecrunchFile decrunchInfo;
	APAgent_ConvertModule convInfo;
	APMRSWList<AddOnInfo *> *infoList;
	bool modConverted = false;
	bool foundType = false;
	bool result = false;
	ap_result apResult;

	// Allocate the file object
	file = new PCacheFile();
	if (file == NULL)
		throw PMemoryException();

	try
	{
		// Initialize the converter structure
		convInfo.moduleFile         = NULL;
		convInfo.newModuleFile      = NULL;

		// Initialize the decruncher structure
		decrunchInfo.file           = file;
		decrunchInfo.decrunchedFile = NULL;

		// Open the module
		decrunchInfo.file->Open(fileName, PFile::pModeReadWrite | PFile::pModeShareRead | PFile::pModeNoTruncate);

		// Get the original length of the file
		fileLength = decrunchInfo.file->GetLength();

		// Decrunch the file
		DecrunchFile(&decrunchInfo);

		// Copy the file used for decrunching into the converter structure
		convInfo.moduleFile = decrunchInfo.file;

		// Well, lock the player list
		infoList = &GetApp()->playerInfo;
		infoList->WaitToRead();

		try
		{
			// Check to see if we can find a player via
			// the file type
			if (!FindPlayerViaFileType(convInfo.moduleFile))
			{
				// No player could be found via the file type.
				// Now try to convert the module
				modConverted = ConvertModule(&convInfo);

				// Try all the players to see if we can find
				// one that understand the file format
				if (FindPlayer(convInfo.moduleFile))
					result = true;
			}
			else
			{
				foundType = true;
				result    = true;
			}

			// Did we found a player?
			if (result)
			{
				PString typeString;
				PString playerError;

				// Yap, change the file type
				if (changeType && !foundType)
				{
					try
					{
						// Change the module type on the file
						if (modConverted)
						{
							if (convInfo.fileType.IsEmpty())
								typeString = player->GetModTypeString(playerInfo->index);
							else
								typeString = convInfo.fileType;
						}
						else
							typeString = player->GetModTypeString(playerInfo->index);

						if (!typeString.IsEmpty())
							file->SetFileType(typeString);
					}
					catch(PFileException e)
					{
						// If we can't set the file type, we ignore the error.
						// An error will occure on e.g. CD-ROM disks
						;
					}
				}

				// Yap, load the module
				convInfo.moduleFile->SeekToBegin();
				apResult = player->LoadModule(playerInfo->index, convInfo.moduleFile, playerError);

				if (apResult != AP_OK)
				{
					// Well, something went wrong when loading the file
					//
					// Build the error string
					errorStr.Format_S3(GetApp()->resource, IDS_CMDERR_LOAD_MODULE, fileName, playerInfo->addOnName, playerError);

					// Delete the player
					playerInfo->loader->DeleteInstance(player);
					player     = NULL;
					playerInfo = NULL;

					result = false;
				}
				else
				{
					// Get module information
					playerName = playerInfo->addOnName;

					if (modConverted)
					{
						moduleFormat = convInfo.modKind;
						if (moduleFormat.IsEmpty())
							moduleFormat = playerInfo->addOnName;
					}
					else
						moduleFormat = playerInfo->addOnName;

					// Should the file still be open?
					if (player->GetSupportFlags(playerInfo->index) & appDontCloseFile)
					{
						// Yes, remember the file pointer
						if (modConverted)
						{
							usingFile = convInfo.newModuleFile;
							convInfo.newModuleFile = NULL;
						}
						else
						{
							usingFile = file;
							file      = NULL;
						}
					}
				}
			}
			else
			{
				// Nup, send an error back
				errorStr.Format_S1(GetApp()->resource, IDS_CMDERR_UNKNOWN_MODULE, fileName);
			}
		}
		catch(...)
		{
			infoList->DoneReading();
			throw;
		}

		// Done with the list
		infoList->DoneReading();
	}
	catch(PFileException e)
	{
		PString err;
		char *fileStr, *errStr;

		// Build error message
		err = PSystem::GetErrorString(e.errorNum);
		errorStr.Format(GetApp()->resource, IDS_CMDERR_FILE, (fileStr = fileName.GetString()), e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);
		fileName.FreeBuffer(fileStr);
		result = false;
	}
	catch(...)
	{
		;
	}

	// Close the files again
	delete convInfo.newModuleFile;
	delete file;
	file = NULL;

	return (result);
}



/******************************************************************************/
/* FreeModule() will free the module from memory.                             */
/******************************************************************************/
void APModuleLoader::FreeModule(void)
{
	// Delete the player if any
	if (playerInfo != NULL)
	{
		playerInfo->loader->DeleteInstance(player);
		player     = NULL;
		playerInfo = NULL;
	}

	// Close the file if it isn't already closed
	delete usingFile;
	usingFile = NULL;

	// Clear the module information
	fileLength = 0;
	moduleFormat.MakeEmpty();
}



/******************************************************************************/
/* GetPlayer() returns the pointer to the current player.                     */
/*                                                                            */
/* Input:  "index" is a reference where the player index will be stored.      */
/*                                                                            */
/* Output: The player pointer.                                                */
/******************************************************************************/
APAddOnPlayer *APModuleLoader::GetPlayer(int32 &index) const
{
	// Store the index
	index = playerInfo->index;

	// And return the pointer
	return (player);
}



/******************************************************************************/
/* GetModuleFormat() returns the module format of the module loaded.          */
/*                                                                            */
/* Output: The module format.                                                 */
/******************************************************************************/
PString APModuleLoader::GetModuleFormat(void) const
{
	return (moduleFormat);
}



/******************************************************************************/
/* GetPlayerName() returns the name of the player.                            */
/*                                                                            */
/* Output: The player name.                                                   */
/******************************************************************************/
PString APModuleLoader::GetPlayerName(void) const
{
	return (playerName);
}



/******************************************************************************/
/* GetModuleSize() returns the size of the module loaded.                     */
/*                                                                            */
/* Output: The module size.                                                   */
/******************************************************************************/
uint32 APModuleLoader::GetModuleSize(void) const
{
	return (fileLength);
}



/******************************************************************************/
/* FindPlayerViaFileType() will get the file type and try to find a player    */
/*      that has associated with the type.                                    */
/*                                                                            */
/* Input:  "modFile" is a pointer to the file object holding the module.      */
/*                                                                            */
/* Output: True if a player was found, false if not.                          */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
bool APModuleLoader::FindPlayerViaFileType(PFile *modFile)
{
	APMRSWList<AddOnInfo *> *infoList;
	PString fileType, playerType;
	int32 i, count;
	ap_result apResult;

	try
	{
		// First try to get the file type
		fileType = modFile->GetFileType();
	}
	catch(PFileException e)
	{
		// Ignore any errors
		;
	}

	// Did the file have any file type?
	if (fileType.IsEmpty())
		return (false);

	// Get a pointer to the player list
	infoList = &GetApp()->playerInfo;

	// Found the file type, so now check the players for a match
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the player information
		playerInfo = infoList->GetItem(i);

		// Check to see if the player is enabled
		if (playerInfo->enabled)
		{
			// Create an instance of the player
			player = (APAddOnPlayer *)playerInfo->loader->CreateInstance();

			// Get the player type
			playerType = player->GetModTypeString(playerInfo->index);

			// Did we get a match?
			if (fileType == playerType)
			{
				// Found the file type in a player. Now call the
				// players check routine, just to make sure it's
				// the right player
				apResult = player->ModuleCheck(playerInfo->index, modFile);
				if (apResult == AP_OK)
				{
					// We found the right player
					return (true);
				}

				if (apResult != AP_UNKNOWN)
				{
					// Some error occurred
					playerInfo->loader->DeleteInstance(player);
					playerInfo = NULL;
					player     = NULL;

					throw PFileException(P_ERR_ANY);
				}
			}

			// Delete the player instance
			playerInfo->loader->DeleteInstance(player);
			player = NULL;
		}
	}

	// No player was found
	playerInfo = NULL;
	return (false);
}



/******************************************************************************/
/* FindPlayer() will call all the players check function to see if some of    */
/*      them understand the file format.                                      */
/*                                                                            */
/* Input:  "modFile" is a pointer to the file object holding the module.      */
/*                                                                            */
/* Output: True if a player was found, false if not.                          */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
bool APModuleLoader::FindPlayer(PFile *modFile)
{
	APMRSWList<AddOnInfo *> *infoList;
	int32 i, count;
	ap_result apResult;

	// Get a pointer to the player list
	infoList = &GetApp()->playerInfo;

	// Traverse all the players to see if we can find one
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the player information
		playerInfo = infoList->GetItem(i);

		// Is the player enabled?
		if (playerInfo->enabled)
		{
			// Create an instance of the player
			player = (APAddOnPlayer *)playerInfo->loader->CreateInstance();

			// Check the file
			apResult = player->ModuleCheck(playerInfo->index, modFile);
			if (apResult == AP_OK)
			{
				// We found the right player
				return (true);
			}

			if (apResult != AP_UNKNOWN)
			{
				// Some error occurred
				playerInfo->loader->DeleteInstance(player);
				playerInfo = NULL;
				player     = NULL;

				throw PFileException(P_ERR_ANY);
			}

			// Delete the player instance
			playerInfo->loader->DeleteInstance(player);
			player = NULL;
		}
	}

	// No player was found
	playerInfo = NULL;
	return (false);
}



/******************************************************************************/
/* ConvertModule() will call all the converter agents to try to convert the   */
/*      module.                                                               */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the converter structure to use.         */
/*                                                                            */
/* Output: True if the module was converted, false if not.                    */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
bool APModuleLoader::ConvertModule(APAgent_ConvertModule *convInfo)
{
	int32 i, count;
	AddOnInfo *info;
	ap_result apResult;
	PString convKind, convType;
	PFile *convFile = NULL;
	bool converted = false;

	// Wait to read access to the plug-in list
	GetApp()->pluginLock.WaitToRead();

	try
	{
		// Get number of plug-ins
		count = GetApp()->converterAgents.CountItems();

		// Call all the plug-ins
		for (i = 0; i < count; i++)
		{
			// Get agent information
			info = GetApp()->converterAgents.GetItem(i);

			// Try to convert the module
			apResult = info->agent->Run(info->index, APCA_CONVERT_MODULE, convInfo);
			if (apResult == AP_ERROR)
				break;

			if (apResult == AP_OK)
			{
				// The module is converted
				if (convInfo->newModuleFile != NULL)
				{
					// Copy the filename to the new file instance
					// if it's a memory file
					if (is_kind_of(convInfo->newModuleFile, PMemFile))
						convInfo->newModuleFile->SetFilePath(convInfo->moduleFile->GetFullPath());

					// Now delete the old file handle.
					// This check is made if the module has been
					// converted multiple times
					if (convInfo->moduleFile != file)
						delete convInfo->moduleFile;

					// Remember the converter strings
					if (convKind.IsEmpty())
					{
						convKind = convInfo->modKind;
						convType = convInfo->fileType;
					}

					// Use the new file
					convFile                = convInfo->newModuleFile;
					convInfo->moduleFile    = convInfo->newModuleFile;
					convInfo->newModuleFile = NULL;
					converted               = true;
				}
			}
		}
	}
	catch(...)
	{
		// Unlock plug-in list
		GetApp()->pluginLock.DoneReading();
		throw;
	}

	// Done with the plug-ins
	GetApp()->pluginLock.DoneReading();

	// Store the original converter strings back
	convInfo->modKind       = convKind;
	convInfo->fileType      = convType;
	convInfo->newModuleFile = convFile;

	return (converted);
}



/******************************************************************************/
/* DecrunchFile() will call all the decruncher agents to try to decrunch the  */
/*      file.                                                                 */
/*                                                                            */
/* Input:  "decrunchInfo" is a pointer to the decruncher structure to use.    */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APModuleLoader::DecrunchFile(APAgent_DecrunchFile *decrunchInfo)
{
	int32 i, count;
	AddOnInfo *info;
	ap_result apResult;

	// Wait to read access to the plug-in list
	GetApp()->pluginLock.WaitToRead();

	try
	{
		// Get number of plug-ins
		count = GetApp()->decruncherAgents.CountItems();

		// Call all the plug-ins
		for (i = 0; i < count; i++)
		{
			// Get agent information
			info = GetApp()->decruncherAgents.GetItem(i);

			// Try to decrunch the file
			apResult = info->agent->Run(info->index, APDA_DECRUNCH_FILE, decrunchInfo);
			if (apResult == AP_ERROR)
				break;

			if (apResult == AP_OK)
			{
				// The file is decrunched
				if (decrunchInfo->decrunchedFile != NULL)
				{
					// Copy the filename to the new file instance
					// if it's a memory file
					if (is_kind_of(decrunchInfo->decrunchedFile, PMemFile))
					{
						try
						{
							decrunchInfo->decrunchedFile->SetFilePath(decrunchInfo->file->GetFullPath());
							decrunchInfo->decrunchedFile->SetFileType(decrunchInfo->file->GetFileType());
						}
						catch(...)
						{
							;
						}
					}

					// Now delete the old file handle.
					// This check is made if the file has been
					// decrunched multiple times
					if (decrunchInfo->file != file)
						delete decrunchInfo->file;

					// Use the new file
					decrunchInfo->file           = decrunchInfo->decrunchedFile;
					decrunchInfo->decrunchedFile = NULL;

					// Take all the decruncher agents one more time,
					// so we can handle recursive packed modules
					i = -1;
				}
			}
		}
	}
	catch(...)
	{
		// Unlock plug-in list
		GetApp()->pluginLock.DoneReading();
		throw;
	}

	// Done with the plug-ins
	GetApp()->pluginLock.DoneReading();
}
