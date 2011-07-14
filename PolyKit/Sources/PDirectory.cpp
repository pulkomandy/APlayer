/******************************************************************************/
/* PDirectory implementation file.                                            */
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
#include "PException.h"
#include "PDirectory.h"
#include "PList.h"
#include "PSystem.h"


/******************************************************************************/
/* PDirectory class                                                           */
/******************************************************************************/

/******************************************************************************/
/**	Default constructor for creating a new PDirectory instance that is
 *	uninitialized. That is, the new instance does not specify a directory name.
 *	In order to initialize the instance, call the SetDirectory() or
 *	FindDirectory() on the instance.
 *//***************************************************************************/
PDirectory::PDirectory(void)
{
	enumDir = NULL;
}



/******************************************************************************/
/**	Standard constructor for creating a new PDirectory object that is
 *	initialized to contain the specified directory name.
 *
 *	@param name directory name.
 *//***************************************************************************/
PDirectory::PDirectory(PString name)
{
	SetDirectory(name);
	enumDir = NULL;
}



/******************************************************************************/
/**	Destructor which destroys the object.
 *//***************************************************************************/
PDirectory::~PDirectory(void)
{
	EndEnum();
}



/******************************************************************************/
/**	Sets the directory name of this object.
 *
 *	@param name the new directory name that this object must contain.
 *//***************************************************************************/
void PDirectory::SetDirectory(PString name)
{
	// Make sure that the name ends with a slash
	if (name.Right(1) != P_DIRSLASH_STR)
		name += P_DIRSLASH_STR;

	dirName = name;
}



/******************************************************************************/
/**	Gets the current directory name of this object.
 *
 *	@return the current directory name.
 *//***************************************************************************/
PString PDirectory::GetDirectory(void) const
{
	return (dirName);
}



/******************************************************************************/
/**	Appends a directory name to the current directory name of this object.
 *
 *	@param name is the directory name to append to the current directory name
 *		of this object.
 *//***************************************************************************/
void PDirectory::Append(PString name)
{
	// If it's an empty name, don't do anything
	if (name.IsEmpty())
		return;

	// Make sure that the name ends with a slash
	if (name.Right(1) != P_DIRSLASH_STR)
		name += P_DIRSLASH_STR;

	// Remove a start slash if any
	if (name.Left(1) == P_DIRSLASH_STR)
		name.Delete(0);

	dirName += name;
}



/******************************************************************************/
/**	Finds and sets this object to contain the directory name of the specified
 *	directory type.
 *
 *	@param type the type of directory this object must be set to.
 *
 *	@exception PFileException
 *	@exception PSystemException
 *//***************************************************************************/
void PDirectory::FindDirectory(PFindType type)
{
	BPath path;
	status_t retVal;

	// Check to see if it's the launch directory, because
	// it's a special case
	if (type == pLaunch)
	{
		app_info info;
		BEntry appEntry;

		// Find the directory
		if ((retVal = be_app->GetAppInfo(&info)) != B_OK)
			throw PFileException(PSystem::ConvertOSError(retVal), "<< FindDirectory >>");

		if ((retVal = appEntry.SetTo(&info.ref)) != B_OK)
			throw PFileException(PSystem::ConvertOSError(retVal), "<< FindDirectory >>");

		if ((retVal = appEntry.GetPath(&path)) != B_OK)
			throw PFileException(PSystem::ConvertOSError(retVal), "<< FindDirectory >>");

		if ((retVal = path.GetParent(&path)) != B_OK)
			throw PFileException(PSystem::ConvertOSError(retVal), "<< FindDirectory >>");

		// And set it
		SetDirectory(path.Path());
	}
	else
	{
		// All other types
		directory_which which;

		// Convert the type to BeOS types
		switch (type)
		{
			case pSettings:
			{
				which = B_USER_SETTINGS_DIRECTORY;
				break;
			}

			case pLibs:
			{
				which = B_BEOS_LIB_DIRECTORY;
				break;
			}

			case pUser:
			{
				which = B_USER_DIRECTORY;
				break;
			}

			default:
			{
				// Illegal type
				ASSERT(false);
				which = B_USER_DIRECTORY;
				break;
			}
		}

		// Find the directory
		if ((retVal = find_directory(which, &path)) != B_OK)
			throw PFileException(PSystem::ConvertOSError(retVal), "<< FindDirectory >>");

		// And set it
		SetDirectory(path.Path());
	}
}



/******************************************************************************/
/**	Creates a directory with the directory name of this object. If the path of
 *	the directory name consists of directory names, which are not already
 *	created, these directories are also created.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PDirectory::CreateDirectory(void) const
{
	char *dirStr;
	status_t retVal;

	if ((retVal = create_directory((dirStr = dirName.GetString()), 0755)) != B_OK)
	{
		dirName.FreeBuffer(dirStr);
		throw PFileException(PSystem::ConvertOSError(retVal), dirName);
	}

	dirName.FreeBuffer(dirStr);
}



/******************************************************************************/
/**	Checks if the directory name of this object contains the specified entry
 *	type and name.
 *
 *	@param name the name of the entry.
 *	@param type the entry type to check for.
 *
 *	@return true if the entry exists; false otherwise.
 *
 *	@exception PFileException
 *//***************************************************************************/
bool PDirectory::Contains(PString name, PEntryType type) const
{
	BDirectory dir;
	char *dirStr, *nameStr;
	int32 nodeFlags;
	status_t retVal;
	bool result;

	retVal = dir.SetTo((dirStr = dirName.GetString()));
	dirName.FreeBuffer(dirStr);

	if (retVal != B_OK)
		throw PFileException(PSystem::ConvertOSError(retVal), dirName);

	// Convert the type to BeOS types
	switch (type)
	{
		case pFile:
		{
			nodeFlags = B_FILE_NODE;
			break;
		}

		case pDirectory:
		{
			nodeFlags = B_DIRECTORY_NODE;
			break;
		}

		case pLink:
		{
			nodeFlags = B_SYMLINK_NODE;
			break;
		}

		default:
		{
			nodeFlags = B_ANY_NODE;
			break;
		}
	}

	result = dir.Contains((nameStr = name.GetString()), nodeFlags);
	name.FreeBuffer(nameStr);

	return (result);
}



/******************************************************************************/
/**	Initializes enumeration of the directory corresponding to the specified
 *	entry type.
 *
 *	@param type the entry type to enumerate.
 *
 *	@exception PFileException
 *
 *	@see GetNextEntry, EndEnum
 *//***************************************************************************/
void PDirectory::InitEnum(PEntryType type)
{
	char *dirStr;
	status_t retVal;

	// Forgot to call EndEnum()?
	ASSERT(enumDir == NULL);

	// Initialize the class variables
	enumType = type;
	enumDir  = new BDirectory;
	if (enumDir == NULL)
		throw PMemoryException();

	// Set the work directory
	retVal = enumDir->SetTo((dirStr = dirName.GetString()));
	dirName.FreeBuffer(dirStr);

	if (retVal != B_OK)
		throw PFileException(PSystem::ConvertOSError(retVal), dirName);

	// Initialize the directory pointer
	if ((retVal = enumDir->Rewind()) != B_OK)
		throw PFileException(PSystem::ConvertOSError(retVal), dirName);
}



/******************************************************************************/
/**	Gets the next entry in an enumeration.
 *
 *	@param name a reference to where the entry name must be stored.
 *	@param type a reference to where the entry type must be stored.
 *
 *	@return true if an entry was returned in the enumeration; false otherwise.
 *
 *	@exception PFileException
 *
 *	@see InitEnum, EndEnum
 *//***************************************************************************/
bool PDirectory::GetNextEntry(PString &name, PEntryType &type)
{
	BEntry entry;
	BPath path;
	status_t retVal;
	bool gotIt = false;

	do
	{
		// Get the next entry
		if ((retVal = enumDir->GetNextEntry(&entry)) != B_OK)
		{
			if (retVal == B_ENTRY_NOT_FOUND)
				return (false);

			throw PFileException(PSystem::ConvertOSError(retVal), dirName);
		}

		// Check the type to see if we want to return it
		switch (enumType)
		{
			case pFile:
			{
				if (entry.IsFile())
				{
					type  = pFile;
					gotIt = true;
				}
				break;
			}

			case pDirectory:
			{
				if (entry.IsDirectory())
				{
					type  = pDirectory;
					gotIt = true;
				}
				break;
			}

			case pLink:
			{
				if (entry.IsSymLink())
				{
					type  = pLink;
					gotIt = true;
				}
				break;
			}

			default:
			{
				if (entry.IsFile())
					type = pFile;
				else
				{
					if (entry.IsDirectory())
						type = pDirectory;
					else
					{
						if (entry.IsSymLink())
							type = pLink;
						else
							type = pAny;
					}
				}

				gotIt = true;
				break;
			}
		}
	}
	while (!gotIt);

	// Okay, now get the name
	if ((retVal = entry.GetPath(&path)) != B_OK)
		throw PFileException(PSystem::ConvertOSError(retVal), dirName);

	name = path.Leaf();
	return (true);
}



/******************************************************************************/
/**	Ends and cleanup the enumeration.
 *
 *	@see InitEnum, GetNextEntry
 *//***************************************************************************/
void PDirectory::EndEnum(void)
{
	// Delete the BDirectory object
	delete enumDir;
	enumDir = NULL;
}



/******************************************************************************/
/**	Returns a valid directory path out from a specified directory name.
 *
 *	@param path the directory path that must be transformed into a valid
 *		directory path.
 *
 *	@return a valid directory path out from the specified directory path.
 *//***************************************************************************/
PString PDirectory::EnsureDirectoryName(PString path)
{
	// Make sure we have a directory slash at the end
	if (path.Right(1) != P_DIRSLASH_STR)
		path += P_DIRSLASH_STR;

	return (path);
}



/******************************************************************************/
/**	Returns the directory part of the specified path.
 *
 *	@param path the path to extract the directory part from.
 *
 *	@return the directory part of the specified path.
 *//***************************************************************************/
PString PDirectory::GetDirectoryPart(PString path)
{
	int32 index;

	// Find the last slash
	index = path.ReverseFind(P_DIRSLASH_CHR);
	if (index == -1)
		return ("");

	return (path.Left(index + 1));
}



/******************************************************************************/
/**	Returns the file part of the specified path.
 *
 *	@param the path to extract the file part from.
 *
 *	@return the file part of the specified path.
 *//***************************************************************************/
PString PDirectory::GetFilePart(PString path)
{
	int32 index;

	// Check if the path is a directory only
	if (path.Right(1) == P_DIRSLASH_STR)
		return ("");

	// Find the last slash
	index = path.ReverseFind(P_DIRSLASH_CHR);
	if (index == -1)
		return (path);

	return (path.Mid(index + 1));
}



/******************************************************************************/
/**	Returns the parent directory of the specified path.
 *
 *	@param path the path to extract the parent directory from.
 *
 *	@return the parent directory of the specified path.
 *//***************************************************************************/
PString PDirectory::GetParentDirectory(PString path)
{
	int32 index;
	PString tempStr(path);

	// If the path is empty, just return an empty string
	if (path.IsEmpty())
		return ("");

	// Remove the last slash if any
	if (tempStr.Right(1) == P_DIRSLASH_STR)
		tempStr = path.Left(path.GetLength() - 1);

	// Find the last slash
	index = tempStr.ReverseFind(P_DIRSLASH_CHR);
	if (index == -1)
		return (path);

	return (path.Left(index + 1));
}



/******************************************************************************/
/**	Returns the root directory of the specified path.
 *
 *	@param path the path to extract the root directory from.
 *
 *	@return the root directory of the specified path.
 *//***************************************************************************/
PString PDirectory::GetRootDirectory(PString path)
{
	// If the path is empty, just return an empty string
	if (path.IsEmpty())
		return ("");

	return (P_DIRSLASH_STR);
}



/******************************************************************************/
/**	Checks if the directory exists which the object represents.
 *
 *	@param path the directory path of the directory to check.
 *
 *	@return true if the directory exists; false otherwise.
 *//***************************************************************************/
bool PDirectory::DirectoryExists(PString path)
{
	BDirectory dir;
	char *pathStr;
	status_t error;

	// Try to open the file
	error = dir.SetTo((pathStr = path.GetString()));
	path.FreeBuffer(pathStr);

	if (error == B_OK)
		return (true);

	return (false);
}
