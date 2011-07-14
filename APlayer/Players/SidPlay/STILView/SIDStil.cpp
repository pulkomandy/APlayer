/******************************************************************************/
/* SID STIL Interface.                                                        */
/*                                                                            */
/* This class was original written by LaLa (LaLa@C64.org), but modified to    */
/* use BeOS variable types and using PolyKit by Thomas Neumann.               */
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
#include "PFile.h"
#include "PDirectory.h"
#include "PException.h"
#include "PList.h"

// Player headers
#include "SIDStil.h"


/******************************************************************************/
/* STIL entry strings                                                         */
/******************************************************************************/
#define _NAME_STR					"   NAME: "
#define _AUTHOR_STR					" AUTHOR: "
#define _TITLE_STR					"  TITLE: "
#define _ARTIST_STR					" ARTIST: "
#define _COMMENT_STR				"COMMENT: "
#define _BUG_STR					"BUG: "



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDStil::SIDStil(void)
{
	// Initialize member variables
	stilVersion = 0.0f;
	stilFile    = NULL;
	bugFile     = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDStil::~SIDStil(void)
{
	DeleteDirList(stilDirs);
	DeleteDirList(bugDirs);
}



/******************************************************************************/
/* SetBaseDir() tells the object where the HVSC base directory is - it        */
/*      figures that the STIL should be in /DOCUMENTS/STIL.txt and that the   */
/*      BUGlist should be in /DOCUMENTS/BUGlist.txt.                          */
/*                                                                            */
/* Input:  "pathToHVSC" is the HVSC base directory.                           */
/*                                                                            */
/* Output: False - Problems opening or parsing STIL/BUGList.                  */
/*         True - All okay.                                                   */
/******************************************************************************/
bool SIDStil::SetBaseDir(PString pathToHVSC)
{
	PString tempName;
	PDirectory dir;
	PList<DirList *> tempStilDirs, tempBugDirs;
	float tempStilVersion;

	// Temporary placeholder for STIL.txt's version number
	tempStilVersion = stilVersion;

	// Sanity check the length
	if (pathToHVSC.IsEmpty())
		return (false);

	try
	{
		// Attempt to open STIL
		dir.SetDirectory(pathToHVSC);
		dir.Append("DOCUMENTS");

		stilFile = new PCacheFile(dir.GetDirectory() + "STIL.txt", PFile::pModeRead | PFile::pModeShareRead);
		if (stilFile == NULL)
			throw PMemoryException();

		// Attempt to open BUGList
		try
		{
			bugFile = new PCacheFile(dir.GetDirectory() + "BUGlist.txt", PFile::pModeRead | PFile::pModeShareRead);
			if (bugFile == NULL)
				throw PMemoryException();
		}
		catch(PFileException e)
		{
			// This is not a critical error - some earlier versions of HVSC did
			// not have a BUGlist.txt file at all
			bugFile = NULL;
		}

		// This is necessary so the version number gets scanned in from the
		// new file, too
		stilVersion = 0.0f;

		if (GetDirs(stilFile, tempStilDirs, true) != true)
			throw PUserException();

		if (bugFile != NULL)
		{
			if (GetDirs(bugFile, tempBugDirs, false) != true)
			{
				// This is not a critical error - it is possible that the
				// BUGlist.txt file has no entries in it at all (in fact, that's
				// good!)
				;
			}
		}
	}
	catch(...)
	{
		// Close the files
		delete bugFile;
		bugFile = NULL;

		delete stilFile;
		stilFile = NULL;

		// Delete the directory lists
		DeleteDirList(tempStilDirs);
		DeleteDirList(tempBugDirs);

		// Copy back the original version
		stilVersion = tempStilVersion;

		return (false);
	}

	// Close the files
	stilFile->Close();
	bugFile->Close();

	// Now we can copy the stuff into private data.
	// NOTE: At this point, stilVersion should contain the new info!
	//
	// Remember the base directory
	baseDir = pathToHVSC;

	// First, delete whatever may have been there previously
	DeleteDirList(stilDirs);
	DeleteDirList(bugDirs);

	// Now proceed with copy
	CopyDirList(stilDirs, tempStilDirs);
	CopyDirList(bugDirs, tempBugDirs);

	// Clear the buffers (caches)
	entryBuf.MakeEmpty();
	globalBuf.MakeEmpty();
	bugBuf.MakeEmpty();

	// Cleanup
	DeleteDirList(tempStilDirs);
	DeleteDirList(tempBugDirs);

	return (true);
}



/******************************************************************************/
/* GetBaseDir() returns the current base directory in use.                    */
/*                                                                            */
/* Output: The base directory in use at the moment.                           */
/******************************************************************************/
PString SIDStil::GetBaseDir(void) const
{
	return (baseDir);
}



/******************************************************************************/
/* GetGlobalComment() - Given an HVSC pathname it returns a formatted         */
/*      string that contains the section-global comment (if it exists). If it */
/*      doesn't exist, returns an empty string.                               */
/*                                                                            */
/* Input:  "relPathToEntry" is the file name relative to the HVSC base        */
/*         directory to the entry.                                            */
/*                                                                            */
/* Output: The section-global comment or an empty string.                     */
/******************************************************************************/
PString SIDStil::GetGlobalComment(PString relPathToEntry)
{
	PString dir;
	int32 lastSlash, pathLen;
	int32 temp;

	if (baseDir.IsEmpty())
		return ("");

	// Save the dirpath
	lastSlash = relPathToEntry.ReverseFind('/');
	if (lastSlash == -1)
		return ("");

	pathLen = lastSlash + 1;
	dir = relPathToEntry.Left(pathLen);

	// Find out whether we have this global comment in the buffer.
	// If the baseDir was changed, we'll have to read it in again,
	// even if it might be in the buffer already
	if ((globalBuf.Left(dir.GetLength()) != dir) || ((globalBuf.Find('\n') != pathLen) && (stilVersion > 2.59f)))
	{
		// The relative pathnames don't match or they're not the same length:
		// We don't have it in the buffer, so pull it in
		try
		{
			PDirectory tempDir;

			tempDir.SetDirectory(baseDir);
			tempDir.Append("DOCUMENTS");

			stilFile->Open(tempDir.GetDirectory() + "STIL.txt", PFile::pModeRead | PFile::pModeShareRead);

			if (PositionToEntry(dir, stilFile, stilDirs) == false)
			{
				// Copy the dirname to the buffer
				globalBuf = dir + "\n";
			}
			else
			{
				globalBuf.MakeEmpty();
				ReadEntry(stilFile, globalBuf);
			}

			stilFile->Close();
		}
		catch(PFileException e)
		{
			// Failed reading from the STIL.txt file
			stilFile->Close();
			return ("");
		}
	}

	// Position the index to the global comment field
	temp = globalBuf.Find('\n');
	temp++;

	// Check whether this is a NULL entry or not
	if (temp == globalBuf.GetLength())
		return ("");

	return (globalBuf.Mid(temp));
}



/******************************************************************************/
/* GetAbsGlobalComment() - Given an HVSC pathname it returns a formatted      */
/*      string that contains the section-global comment (if it exists). If it */
/*      doesn't exist, returns an empty string.                               */
/*                                                                            */
/* Input:  "absPathToEntry" is the file name with full path to the entry.     */
/*                                                                            */
/* Output: The section-global comment or an empty string.                     */
/******************************************************************************/
PString SIDStil::GetAbsGlobalComment(PString absPathToEntry)
{
	PString tempDir;

	if (baseDir.IsEmpty())
		return ("");

	// Determine if the baseDir is in the given pathname
	if (absPathToEntry.Left(baseDir.GetLength()) != baseDir)
		return ("");

	// Extract the file name, relative from the base dir,
	// from the entry given and convert the slashes
	tempDir = absPathToEntry.Mid(baseDir.GetLength());
	tempDir.Replace(P_DIRSLASH_CHR, '/');

	return (GetGlobalComment(tempDir));
}



/******************************************************************************/
/* GetEntry() - Given an HVSC pathname, a tune number and a field             */
/*      designation, it returns a formatted string that contains the STIL     */
/*      field for the tune number (if exists). If it doesn't exist, an empty  */
/*      string is returned.                                                   */
/*                                                                            */
/* Input:  "relPathToEntry" is the file name relative to the HVSC base        */
/*         directory to the entry.                                            */
/*         "tuneNo" is the song number within in the song.                    */
/*         "field" is the field to retrieve.                                  */
/*                                                                            */
/*         What the possible combinations of tuneNo and field represent:      */
/*                                                                            */
/*         - tuneNo = 0, field = all : All of the STIL entry is returned.     */
/*         - tuneNo = 0, field = comment : The file-global comment is         */
/*           returned. (For single-tune entries, this returns nothing!)       */
/*         - tuneNo = 0, field = <other> : INVALID! Empty string is returned. */
/*         - tuneNo != 0, field = all : All fields of the STIL entry for the  */
/*           given tune number are returned. (For single-tune entries, this   */
/*           is equivalent to saying tuneNo = 0, field = all.)                */
/*           However, the file-global comment is *NOT* returned with it any   */
/*           more! (Unlike in versions before v2.00.) It led to confusions:   */
/*           eg. when a comment was asked for tune #3, it returned the        */
/*           file-global comment even if there was no specific entry for tune */
/*           #3!                                                              */
/*         - tuneNo != 0, field = <other> : The specific field of the         */
/*           specific tune number is returned. If the tune number doesn't     */
/*           exist (eg. if tuneNo=2 for single-tune entries, or if tuneNo=2   */
/*           where there's no STIL entry for tune #2 in a multitune entry),   */
/*           returns an empty string.                                         */
/*                                                                            */
/*         NOTE: For older versions of STIL (older than v2.59) the tuneNo and */
/*         field parameters are ignored and are assumed to be tuneNo=0 and    */
/*         field=all to maintain backwards compatibility.                     */
/*                                                                            */
/* Output: The STIL entry or an empty string.                                 */
/******************************************************************************/
PString SIDStil::GetEntry(PString relPathToEntry, int32 tuneNo, STILField field)
{
	if (baseDir.IsEmpty())
		return ("");

	// Fail if a section-global comment was asked for
	if (relPathToEntry.GetAt(relPathToEntry.GetLength() - 1) == '/')
		return ("");

	if (stilVersion < 2.59f)
	{
		tuneNo = 0;
		field  = all;
	}

	// Find out whether we have this entry in the buffer
	if ((entryBuf.Left(relPathToEntry.GetLength()) != relPathToEntry) || ((entryBuf.Find('\n') != relPathToEntry.GetLength()) && (stilVersion > 2.59f)))
	{
		// The relative pathnames don't match or they're not the same length:
		// We don't have it in the buffer, so pull it in
		try
		{
			PDirectory tempDir;

			tempDir.SetDirectory(baseDir);
			tempDir.Append("DOCUMENTS");

			stilFile->Open(tempDir.GetDirectory() + "STIL.txt", PFile::pModeRead | PFile::pModeShareRead);

			if (PositionToEntry(relPathToEntry, stilFile, stilDirs) == false)
			{
				// Copy the entry's name to the buffer
				entryBuf = relPathToEntry + "\n";
			}
			else
			{
				entryBuf.MakeEmpty();
				ReadEntry(stilFile, entryBuf);
			}

			stilFile->Close();
		}
		catch(PFileException e)
		{
			// Failed reading from the STIL.txt file
			stilFile->Close();
			return ("");
		}
	}

	// Put the requested field into the result string
	if (GetField(resultEntry, entryBuf, tuneNo, field) != true)
		return ("");

	return (resultEntry);
}



/******************************************************************************/
/* GetAbsEntry() - Given an HVSC pathname, a tune number and a field          */
/*      designation, it returns a formatted string that contains the STIL     */
/*      field for the tune number (if exists). If it doesn't exist, an empty  */
/*      string is returned.                                                   */
/*                                                                            */
/* Input:  "absPathToEntry" is the file name with full path to the entry.     */
/*         "tuneNo" is the song number within in the song.                    */
/*         "field" is the field to retrieve.                                  */
/*                                                                            */
/* Output: The STIL entry or an empty string.                                 */
/******************************************************************************/
PString SIDStil::GetAbsEntry(PString absPathToEntry, int32 tuneNo, STILField field)
{
	PString tempDir;

	if (baseDir.IsEmpty())
		return ("");

	// Determine if the baseDir is in the given pathname
	if (absPathToEntry.Left(baseDir.GetLength()) != baseDir)
		return ("");

	// Extract the file name, relative from the base dir,
	// from the entry given and convert the slashes
	tempDir = absPathToEntry.Mid(baseDir.GetLength());
	tempDir.Replace(P_DIRSLASH_CHR, '/');

	return (GetEntry(tempDir, tuneNo, field));
}



/******************************************************************************/
/* GetBug() - Given an HVSC pathname and tune number it returns a formatted   */
/*      string that contains the BUG entry for the tune number (if exists).   */
/*      If it doesn't exist, an empty string is returned.                     */
/*                                                                            */
/* Input:  "relPathToEntry" is the file name relative to the HVSC base        */
/*         directory to the entry.                                            */
/*         "tuneNo" is the song number within in the song. If tuneNo=0,       */
/*         returns all of the BUG entry.                                      */
/*                                                                            */
/*         NOTE: For older versions of STIL (older than v2.59) tuneNo is      */
/*         ignored and is assumed to be 0 to maintain backwards               */
/*         compatibility.                                                     */
/*                                                                            */
/* Output: The BUG entry or an empty string.                                  */
/******************************************************************************/
PString SIDStil::GetBug(PString relPathToEntry, int32 tuneNo)
{
	if (baseDir.IsEmpty())
		return ("");

	// Older versions of STIL is detected
	if (stilVersion < 2.59f)
		tuneNo = 0;

	// Find out whether we have this bug entry in the buffer.
	// If the baseDir was changed, we'll have to read it in again,
	// even if it might be in the buffer already
	if ((bugBuf.Left(relPathToEntry.GetLength()) != relPathToEntry) || ((bugBuf.Find('\n') != relPathToEntry.GetLength()) && (stilVersion > 2.59f)))
	{
		// The relative pathnames don't match or they're not the same length:
		// We don't have it in the buffer, so pull it in
		try
		{
			PDirectory tempDir;

			tempDir.SetDirectory(baseDir);
			tempDir.Append("DOCUMENTS");

			bugFile->Open(tempDir.GetDirectory() + "BUGlist.txt", PFile::pModeRead | PFile::pModeShareRead);

			if (PositionToEntry(relPathToEntry, bugFile, bugDirs) == false)
			{
				// Copy the entry's name to the buffer
				bugBuf = relPathToEntry + "\n";
			}
			else
			{
				bugBuf.MakeEmpty();
				ReadEntry(bugFile, bugBuf);
			}

			bugFile->Close();
		}
		catch(PFileException e)
		{
			// Failed reading from the STIL.txt file
			bugFile->Close();
			return ("");
		}
	}

	// Put the requested field into the result string
	if (GetField(resultBug, bugBuf, tuneNo) != true)
		return ("");

	return (resultBug);
}



/******************************************************************************/
/* GetAbsBug() - Given an HVSC pathname and tune number it returns a          */
/*      formatted string that contains the BUG entry for the tune number (if  */
/*      exists). If it doesn't exist, an empty string is returned.            */
/*                                                                            */
/* Input:  "absPathToEntry" is the file name with full path to the entry.     */
/*         "tuneNo" is the song number within in the song. If tuneNo=0,       */
/*         returns all of the BUG entry.                                      */
/*                                                                            */
/*         NOTE: For older versions of STIL (older than v2.59) tuneNo is      */
/*         ignored and is assumed to be 0 to maintain backwards               */
/*         compatibility.                                                     */
/*                                                                            */
/* Output: The BUG entry or an empty string.                                  */
/******************************************************************************/
PString SIDStil::GetAbsBug(PString absPathToEntry, int32 tuneNo)
{
	PString tempDir;

	if (baseDir.IsEmpty())
		return ("");

	// Determine if the baseDir is in the given pathname
	if (absPathToEntry.Left(baseDir.GetLength()) != baseDir)
		return ("");

	// Extract the file name, relative from the base dir,
	// from the entry given and convert the slashes
	tempDir = absPathToEntry.Mid(baseDir.GetLength());
	tempDir.Replace(P_DIRSLASH_CHR, '/');

	return (GetBug(tempDir, tuneNo));
}



/******************************************************************************/
/* GetDirs() populates the given dirList array with the directories obtained  */
/*      from 'inFile' for faster positioning within 'inFile'.                 */
/*                                                                            */
/* Input:  "inFile" is where to read the directories from.                    */
/*         "dirs" is the list that should be populated with the directory     */
/*         list.                                                              */
/*         "isStilFile" indicates if the file is a STIL or BUGlist file.      */
/*                                                                            */
/* Output: False - No entries were found or otherwise failed to process       */
/*         inFile.                                                            */
/*         True - Everything is okay.                                         */
/******************************************************************************/
bool SIDStil::GetDirs(PFile *inFile, PList<DirList *> &dirs, bool isStilFile)
{
	PString line, temp;
	char *tempStr;
	int32 i = 0;
	int32 j;
	bool newDir;
	DirList *item;
	int32 startPos;
	PCharSet_MS_WIN_1252 charSet;

	try
	{
		if (isStilFile)
			newDir = false;
		else
			newDir = true;

		inFile->SeekToBegin();

		while (!inFile->IsEOF())
		{
			// Remember the start of line position
			startPos = inFile->GetPosition();

			// Read the next line
			line = inFile->ReadLine(&charSet);

			// Try to extract STIL's version number if it's not done, yet
			if (isStilFile && (stilVersion == 0.0f))
			{
				if (line.Left(9) == "#  STIL v")
				{
					// Get the version number
					temp        = line.Mid(9);
					tempStr     = temp.GetString();
					stilVersion = atof(tempStr);
					temp.FreeBuffer(tempStr);
					continue;
				}
			}

			// Search for the start of a dir separator first
			if (isStilFile && !newDir && (line.Left(4) == "### "))
			{
				newDir = true;
				continue;
			}

			// Is this the start of an entry immediately following a dir separator?
			if (newDir && !line.IsEmpty() && (line.GetAt(0) == '/'))
			{
				// Get the directory only
				j = line.ReverseFind('/') + 1;

				if (!isStilFile)
				{
					// Compare it to the last stored dirname
					if (i == 0)
						newDir = true;
					else
					{
						if (dirs.GetItem(i - 1)->dirName != line.Left(j))
							newDir = true;
						else
							newDir = false;
					}
				}

				// Store the info
				if (newDir)
				{
					item = new DirList;
					if (item == NULL)
						throw PMemoryException();

					item->dirName  = line.Left(j);
					item->position = startPos;

					dirs.AddTail(item);
					i++;
				}

				if (isStilFile)
					newDir = false;
				else
					newDir = true;
			}
		}

		if (i == 0)
		{
			// No entries found - something is wrong.
			// NOTE: It's perfectly valid to have a BUGlist.txt file with no
			// entries in it!
			return (false);
		}
	}
	catch(...)
	{
		return (false);
	}

	return (true);
}



/******************************************************************************/
/* DeleteDirList() deletes *all* of the list of dirnames.                     */
/*                                                                            */
/* Input:  "dirs" is a reference to the list to delete.                       */
/******************************************************************************/
void SIDStil::DeleteDirList(PList<DirList *> &dirs)
{
	DirList *item;
	int32 i, count;

	count = dirs.CountItems();
	for (i = 0; i < count; i++)
	{
		item = dirs.GetItem(i);
		delete item;
	}

	dirs.MakeEmpty();
}



/******************************************************************************/
/* CopyDirList() copies the list of dirnames from one list to another. It     */
/*      creates new dirlist entries in the destination list as needed.        */
/*                                                                            */
/* Input:  "toList" is a reference to the destination list.                   */
/*         "fromList" is a reference to the source list.                      */
/******************************************************************************/
void SIDStil::CopyDirList(PList<DirList *> &toList, PList<DirList *> &fromList)
{
	DirList *sourceItem, *destItem;
	int32 i, count;

	count = fromList.CountItems();
	for (i = 0; i < count; i++)
	{
		sourceItem = fromList.GetItem(i);

		destItem = new DirList;
		if (destItem == NULL)
			throw PMemoryException();

		destItem->dirName  = sourceItem->dirName;
		destItem->position = sourceItem->position;

		toList.AddTail(destItem);
	}
}



/******************************************************************************/
/* PositionToEntry() positions the file pointer to the given entry in         */
/*      'inFile' using the 'dirs' dirList for faster positioning.             */
/*                                                                            */
/* Input:  "entryStr" is the entry to position to.                            */
/*         "inFile" is a pointer to the file to change the position in.       */
/*         "dirs" is a reference to the list to use for faster positioning.   */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool SIDStil::PositionToEntry(PString entryStr, PFile *inFile, PList<DirList *> &dirs)
{
	DirList *item = NULL;
	int32 slashIndex;
	int32 startPos;
	int32 i, count;
	int32 pathLen, entryStrLen;
	PString line;
	bool foundIt = false;
	bool globComm = false;
	bool temp;
	PCharSet_MS_WIN_1252 charSet;

	try
	{
		// Seek to the start of the file
		inFile->SeekToBegin();

		// Get the dirpath
		slashIndex = entryStr.ReverseFind('/');

		// If no slash was found, something is screwed up in the entryStr
		if (slashIndex == -1)
			return (false);

		pathLen = slashIndex + 1;

		// Determine whether a section-global comment is asked for
		entryStrLen = entryStr.GetLength();
		if (pathLen == entryStrLen)
			globComm = true;

		// Find it in the table
		count = dirs.CountItems();
		for (i = 0; i < count; i++)
		{
			item = dirs.GetItem(i);
			if (entryStr.Left(pathLen) == item->dirName)
			{
				foundIt = true;
				break;
			}
		}

		if (!foundIt)
		{
			// The directory was not found
			return (false);
		}

		// Jump to the first entry of this section
		inFile->Seek(item->position, PFile::pSeekBegin);
		foundIt = false;

		// Now find the desired entry
		do
		{
			startPos = inFile->GetPosition();
			line     = inFile->ReadLine(&charSet);

			if (inFile->IsEOF())
				break;

			// Check if it is the start of an entry
			if (!line.IsEmpty() && (line.GetAt(0) == '/'))
			{
				if (line.Left(pathLen) != item->dirName)
				{
					// We are outside the section - get out of the loop,
					// which will fail the search
					break;
				}

				// Check whether we need to find a section-global comment or
				// a specific entry
				if (globComm || (stilVersion > 2.59f))
					temp = (line == entryStr);
				else
				{
					// To be compatible with older versions of STIL, which may have
					// the tune designation on the first line of a STIL entry
					// together with the pathname
					temp = (line.Left(entryStrLen) == entryStr);
				}

				if (temp)
				{
					// Found it!
					foundIt = true;
				}
			}
		}
		while (!foundIt);

		if (foundIt)
		{
			// Reposition the file pointer back to the start of the entry
			inFile->Seek(startPos, PFile::pSeekBegin);
			return (true);
		}
	}
	catch(PFileException e)
	{
		;
	}

	return (false);
}



/******************************************************************************/
/* ReadEntry() reads the entry from 'inFile' into 'buffer'. 'inFile' should   */
/*      already be positioned to the entry to be read.                        */
/*                                                                            */
/* Input:  "inFile" is a pointer to the file to read from.                    */
/*         "buffer" is a reference to the string where the result should be   */
/*         stored.                                                            */
/******************************************************************************/
void SIDStil::ReadEntry(PFile *inFile, PString &buffer)
{
	PString line;
	PCharSet_MS_WIN_1252 charSet;

	try
	{
		do
		{
			line = inFile->ReadLine(&charSet);
			buffer += line;
			if (!line.IsEmpty())
				buffer += "\n";
		}
		while(!line.IsEmpty());
	}
	catch(PFileException e)
	{
		;
	}
}



/******************************************************************************/
/* GetField() - Given a STIL formatted entry in 'buffer', a tune number, and  */
/*      a field designation, it returns the requested STIL field into         */
/*      'result'. If field=all, it also puts the file-global comment (if it   */
/*      exists) as the first field into 'result'.                             */
/*                                                                            */
/* Input:  "result" is a reference to put the resulting string into.          */
/*         "buffer" is the STIL formatted string to search in.                */
/*         "tuneNo" is the song number within the song.                       */
/*         "field" is the field to retrieve.                                  */
/*                                                                            */
/* Output: False if nothing was put into 'result', true if a result has been  */
/*         returned.                                                          */
/******************************************************************************/
bool SIDStil::GetField(PString &result, PString buffer, int32 tuneNo, STILField field)
{
	int32 start, firstTuneNo, temp, temp2 = -1;

	// Clean out the result buffer first
	result.MakeEmpty();

	// Position pointer to the first char beyond the file designation
	start = buffer.Find('\n');
	start++;

	// Check whether this is a NULL entry or not
	if (start == 0)
		return (false);

	// Is this a multitune entry?
	firstTuneNo = buffer.Find("(#", start);

	// This is a tune designation only if the previous char was
	// a newline (ie. if the "(#" is on the beginning of a line).
	if ((firstTuneNo >= 1) && (buffer.GetAt(firstTuneNo - 1) != '\n'))
		firstTuneNo = -1;

	if (firstTuneNo == -1)
	{
		//-------------------//
		// SINGLE TUNE ENTRY //
		//-------------------//
		//
		// Is the first thing in this STIL entry the COMMENT?
		temp = buffer.Find(_COMMENT_STR, start);

		// Search for other potential fields beyond the COMMENT
		if (temp == start)
		{
			temp2 = buffer.Find(_NAME_STR, start);
			if (temp2 == -1)
			{
				temp2 = buffer.Find(_AUTHOR_STR, start);
				if (temp2 == -1)
				{
					temp2 = buffer.Find(_TITLE_STR, start);
					if (temp2 == -1)
						temp2 = buffer.Find(_ARTIST_STR, start);
				}
			}
		}

		if (temp == start)
		{
			// Yes. So it's assumed to be a file-global comment
			if ((tuneNo == 0) && ((field == all) || ((field == comment) && (temp2 == -1))))
			{
				// Simply copy the stuff in
				result = buffer.Mid(start);
				return (true);
			}
			else if ((tuneNo == 0) && (field == comment))
			{
				// Just copy the comment
				result = buffer.Mid(start, temp2 - start);
				return (true);
			}
			else if ((tuneNo == 1) && (temp2 != -1))
			{
				// A specific field was asked for
				return (GetOneField(result, buffer.Mid(temp2), field));
			}
			else
			{
				// Anything else is invalid as of v2.00
				return (false);
			}
		}
		else
		{
			// No. Handle it as a regular entry
			if ((field == all) && ((tuneNo == 0) || (tuneNo == 1)))
			{
				// The complete entry was asked for. Simply copy the stuff in
				result = buffer.Mid(start);
				return (true);
			}
			else if (tuneNo == 1)
			{
				// A specific field was asked for
				return (GetOneField(result, buffer.Mid(start), field));
			}
			else
			{
				// Anything else is invalid as of v2.00
				return (false);
			}
		}
	}
	else
	{
		//-----------------//
		// MULTITUNE ENTRY //
		//-----------------//
		int32 myTuneNo, nextTuneNo;
		PString tuneNoStr;

		// Was the complete entry asked for?
		if (tuneNo == 0)
		{
			switch (field)
			{
				case all:
				{
					// Yes. Simply copy the stuff in
					result = buffer.Mid(start);
					return (true);
				}

				case comment:
				{
					// Only the file-global comment field was asked for
					if (firstTuneNo != start)
						return (GetOneField(result, buffer.Mid(start, firstTuneNo - start), comment));
					else
						return (false);
				}

				default:
				{
					// If a specific field other than a comment is
					// asked for tuneNo=0, this is illegal
					return (false);
				}
			}
		}

		// Search for the requested tune number
		tuneNoStr.Format("(#%d)", tuneNo);
		myTuneNo = buffer.Find(tuneNoStr, start);

		if (myTuneNo != -1)
		{
			// We found the requested tune number.
			// Set the pointer beyond it
			myTuneNo = buffer.Find('\n', myTuneNo) + 1;

			// Where is the next one?
			nextTuneNo = buffer.Find("\n(#", myTuneNo);
			if (nextTuneNo == -1)
			{
				// There is no next one - set pointer to the end of entry
				nextTuneNo = buffer.GetLength();
			}
			else
			{
				// The search included the \n - go beyond it
				nextTuneNo++;
			}

			// Put the desired fields into the result (which may be 'all')
			PString tempResult;
			bool retVal;

			retVal = GetOneField(tempResult, buffer.Mid(myTuneNo, nextTuneNo - myTuneNo), field);
			result += tempResult;
			return (retVal);
		}
		else
			return (false);
	}
}



/******************************************************************************/
/* GetOneField()                                                              */
/*                                                                            */
/* Input:  "result" is a reference to put the resulting string into.          */
/*         "buffer" is the STIL formatted string to search in.                */
/*         "field" is the field to retrieve.                                  */
/*                                                                            */
/* Output: False if nothing was put into 'result', true if a result has been  */
/*         returned.                                                          */
/******************************************************************************/
bool SIDStil::GetOneField(PString &result, PString buffer, STILField field)
{
	int32 temp = -1;

	// Sanity check
	if (buffer.GetAt(buffer.GetLength() - 1) != '\n')
	{
		result.MakeEmpty();
		return (false);
	}

	switch (field)
	{
		case all:
			result += buffer;
			return (true);

		case name:
			temp = buffer.Find(_NAME_STR);
			break;

		case author:
			temp = buffer.Find(_AUTHOR_STR);
			break;

		case title:
			temp = buffer.Find(_TITLE_STR);
			break;

		case artist:
			temp = buffer.Find(_ARTIST_STR);
			break;

		case comment:
			temp = buffer.Find(_COMMENT_STR);
			break;

		default:
			break;
	}

	// If the field was not found or it is not in between 'start'
	// and 'end', it is declared a failure
	if (temp == -1)
	{
		result.MakeEmpty();
		return (false);
	}

	// Search for the end of this field. This is done by finding
	// where the next field starts
	int32 nextName, nextAuthor, nextTitle, nextArtist, nextComment, nextField;

	nextName    = buffer.Find(_NAME_STR, temp + 1);
	nextAuthor  = buffer.Find(_AUTHOR_STR, temp + 1);
	nextTitle   = buffer.Find(_TITLE_STR, temp + 1);
	nextArtist  = buffer.Find(_ARTIST_STR, temp + 1);
	nextComment = buffer.Find(_COMMENT_STR, temp + 1);

	// Now determine which one is the closest to our field - that one
	// will mark the end of the required field
	nextField = nextName;

	if (nextField == -1)
		nextField = nextAuthor;
	else if ((nextAuthor != -1) && (nextAuthor < nextField))
		nextField = nextAuthor;

	if (nextField == -1)
		nextField = nextTitle;
	else if ((nextTitle != -1) && (nextTitle < nextField))
		nextField = nextTitle;

	if (nextField == -1)
		nextField = nextArtist;
	else if ((nextArtist != -1) && (nextArtist < nextField))
		nextField = nextArtist;

	if (nextField == -1)
		nextField = nextComment;
	else if ((nextComment != -1) && (nextComment < nextField))
		nextField = nextComment;

	if (nextField == -1)
		nextField = buffer.GetLength();

	// Now nextField points to the last+1 char that should be copied to
	// result. Do that
	result += buffer.Mid(temp, nextField - temp);
	return (true);
}
