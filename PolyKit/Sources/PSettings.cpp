/******************************************************************************/
/* PSettings implementation file.                                             */
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
#include "PString.h"
#include "PException.h"
#include "PSynchronize.h"
#include "PDirectory.h"
#include "PFile.h"
#include "PList.h"
#include "PSettings.h"


/******************************************************************************/
/* PSettings class                                                            */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PSettings::PSettings(void)
{
	// Initialize member variables
	changed = false;

	// Allocate syncronize object
	listLock = new PMRSWLock();
	if (listLock == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PSettings::~PSettings(void)
{
	// Destroy the file lock
	delete listLock;

	// Empty the list of lines
	lines.MakeEmpty();
}



/******************************************************************************/
/* LoadFile() will load the .ini file into the memory.                        */
/*                                                                            */
/* Input:  "fileName" is the name of the file to load.                        */
/*         "company" is the company folder name.                              */
/*         "product" is the product folder name.                              */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void PSettings::LoadFile(PString fileName, PString company, PString product)
{
	PDirectory dir;
	PFile file;
	PFileLine line;

	// The settings is not changed, because we will load new ones :)
	changed = false;

	// Find the settings directory and create it
	dir.FindDirectory(PDirectory::pSettings);
	dir.Append(company);
	dir.Append(product);
	dir.CreateDirectory();

	// Append the file name to the directory to build the full file name
	fileName = dir.GetDirectory() + fileName;

	// Lock the list
	listLock->WaitToWrite();

	try
	{
		// Empty any previous file data
		lines.MakeEmpty();

		// Open the file
		if (file.FileExists(fileName))
		{
			file.Open(fileName, PFile::pModeRead | PFile::pModeShareRead);

			// Read one line at the time into the memory
			while (!file.IsEOF())
			{
				// Read the line
				line.line = file.ReadLine();

				// Skip empty lines
				if (line.line.IsEmpty())
					continue;

				// Find out the type of the line
				if (line.line.GetAt(0) == ';')
				{
					// Comment
					line.type = pComment;
				}
				else
				{
					if (line.line.GetAt(0) == '[')
					{
						// Section
						line.type = pSection;

						// Remove the section marks
						line.line = line.line.Mid(1, line.line.GetLength() - 2);
					}
					else
					{
						// Entry, but check to see if it's valid
						if (line.line.Find('=') == -1)
							continue;

						line.type = pEntry;
					}
				}

				// Add the line to the list
				lines.AddTail(line);
			}

			// Close the file
			file.Close();
		}
	}
	catch(...)
	{
		// Unlock again
		listLock->DoneWriting();
		throw;
	}

	// Unlock again
	listLock->DoneWriting();
}



/******************************************************************************/
/* SaveFile() will save the data in the memory back to the file given.        */
/*                                                                            */
/* Input:  "fileName" is the name of the file to write to.                    */
/*         "company" is the company folder name.                              */
/*         "product" is the product folder name.                              */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void PSettings::SaveFile(PString fileName, PString company, PString product)
{
	PDirectory dir;
	PFile file;
	PFileLine line;
	int32 i, count;
	bool firstSection = true;

	// Well, we do only need to save if the settings has been changed
	if (!changed)
		return;

	// Find the settings directory and create it
	dir.FindDirectory(PDirectory::pSettings);
	dir.Append(company);
	dir.Append(product);
	dir.CreateDirectory();

	// Append the file name to the directory to build the full file name
	fileName = dir.GetDirectory() + fileName;

	// Lock the list for reading
	listLock->WaitToRead();

	// Open the file
	file.Open(fileName, PFile::pModeWrite | PFile::pModeCreate);

	try
	{
		// Get number of lines to write
		count = lines.CountItems();

		for (i = 0; i < count; i++)
		{
			// Get the line
			line = lines.GetItem(i);

			// If the type is a section, we need to write an empty
			// line before it, except if it is the first section
			if (line.type == pSection)
			{
				if (!firstSection)
				{
					// Write an empty line
					file.WriteLine("");
				}
				else
					firstSection = false;

				// Write the section name
				file.WriteLine("[" + line.line + "]");
			}
			else
			{
				// Write the line itself
				file.WriteLine(line.line);
			}
		}

		// Close the file
		file.Close();
	}
	catch(...)
	{
		// Unlock again
		listLock->DoneReading();
		throw;
	}

	// Unlock again
	listLock->DoneReading();

	// The settings has been saved, reset the change variable
	changed = false;
}



/******************************************************************************/
/* CloneSettings() will copy the settings from the source given into the      */
/*      current object.                                                       */
/*                                                                            */
/* Input:  "source" is a pointer to the object the settings are copied from.  */
/******************************************************************************/
void PSettings::CloneSettings(const PSettings *source)
{
	// Wait for write access
	listLock->WaitToWrite();

	// Empty any previous file data
	lines.MakeEmpty();

	// Wait for read access
	source->listLock->WaitToRead();

	// Copy the list
	lines = source->lines;

	// The settings has been changed
	changed = true;

	// Unlock the settings again
	source->listLock->DoneReading();
	listLock->DoneWriting();
}



/******************************************************************************/
/* SetChangeFlag() will set the change flag to the value given.               */
/*                                                                            */
/* Input:  "flag" is what you want to set the change flag to.                 */
/******************************************************************************/
void PSettings::SetChangeFlag(bool flag)
{
	// Wait for write access before we change the flag
	listLock->WaitToWrite();
	changed = flag;
	listLock->DoneWriting();
}



/******************************************************************************/
/* HasChanged() will return the changed flag, indicating if the settings has  */
/*		been changed.                                                         */
/*                                                                            */
/* Output: The change flag.                                                   */
/******************************************************************************/
bool PSettings::HasChanged(void) const
{
	bool flag;

	// We need read access before we can return the flag
	listLock->WaitToRead();
	flag = changed;
	listLock->DoneReading();

	return (flag);
}



/******************************************************************************/
/* GetStringEntryValue() will try to read the entry in the setting file. If   */
/*      it couldn't be read, the default value is returned.                   */
/*                                                                            */
/* Input:  "section" is the section name where the entry is stored.           */
/*         "entry" is the name of the entry to read.                          */
/*         "defaultValue" is the value to return it the entry couldn't be     */
/*         read.                                                              */
/*                                                                            */
/* Output: The entry value or default value.                                  */
/******************************************************************************/
PString PSettings::GetStringEntryValue(PString section, PString entry, PString defaultValue) const
{
	PFileLine line;
	PString value(defaultValue);
	int32 index, insertPos;
	int32 valPos;

	// Start to lock the list
	listLock->WaitToRead();

	// Now find the section
	index = FindSection(section);
	if (index != -1)
	{
		// Found it, now find the entry
		index = FindEntry(index + 1, entry, insertPos);
		if (index != -1)
		{
			// Got it, now extract the value
			line   = lines.GetItem(index);
			valPos = line.line.Find('=');
			value  = line.line.Mid(valPos + 1);
		}
	}

	// Unlock again
	listLock->DoneReading();

	return (value);
}



/******************************************************************************/
/* GetStringEntryValue() will try to read the entry with a specific number in */
/*      the setting file. If it couldn't be read, the default value is        */
/*      returned.                                                             */
/*                                                                            */
/* Input:  "section" is the section name where the entry is stored.           */
/*         "entryNum" is the entry number in the section to read.             */
/*         "entryName" is a reference where the entry name will be stored.    */
/*         "defaultValue" is the value to return it the entry couldn't be     */
/*         read.                                                              */
/*                                                                            */
/* Output: The entry value or default value.                                  */
/******************************************************************************/
PString PSettings::GetStringEntryValue(PString section, int32 entryNum, PString &entryName, PString defaultValue) const
{
	PFileLine line;
	PString value(defaultValue);
	int32 index, insertPos;
	int32 valPos;

	// Start to lock the list
	listLock->WaitToRead();

	// Now find the section
	index = FindSection(section);
	if (index != -1)
	{
		// Found it, now find the entry
		index = FindEntryByNumber(index + 1, entryNum, insertPos);
		if (index != -1)
		{
			// Got it, now extract the value
			line      = lines.GetItem(index);
			valPos    = line.line.Find('=');
			entryName = line.line.Left(valPos);
			value     = line.line.Mid(valPos + 1);
		}
	}

	// Unlock again
	listLock->DoneReading();

	return (value);
}



/******************************************************************************/
/* GetIntEntryValue() will try to read the entry in the setting file. If it   */
/*      couldn't be read, the default value is returned.                      */
/*                                                                            */
/* Input:  "section" is the section name where the entry is stored.           */
/*         "entry" is the name of the entry to read.                          */
/*         "defaultValue" is the value to return it the entry couldn't be     */
/*         read.                                                              */
/*                                                                            */
/* Output: The entry or default value.                                        */
/******************************************************************************/
int32 PSettings::GetIntEntryValue(PString section, PString entry, int32 defaultValue) const
{
	PString value;

	// Use the string read function
	value = GetStringEntryValue(section, entry);
	if (value.IsEmpty())
		return (defaultValue);

	return (value.GetNumber());
}



/******************************************************************************/
/* GetIntEntryValue() will try to read the entry with a specific number in    */
/*      the setting file. If it couldn't be read, the default value is        */
/*      returned.                                                             */
/*                                                                            */
/* Input:  "section" is the section name where the entry is stored.           */
/*         "entryNum" is the entry number in the section to read.             */
/*         "entryName" is a reference where the entry name will be stored.    */
/*         "defaultValue" is the value to return it the entry couldn't be     */
/*         read.                                                              */
/*                                                                            */
/* Output: The entry value or default value.                                  */
/******************************************************************************/
int32 PSettings::GetIntEntryValue(PString section, int32 entryNum, PString &entryName, int32 defaultValue) const
{
	PString value;

	// Use the string read function
	value = GetStringEntryValue(section, entryNum, entryName);
	if (value.IsEmpty())
		return (defaultValue);

	return (value.GetNumber());
}



/******************************************************************************/
/* WriteStringEntryValue() will store the entry in the setting file. If it    */
/*      already exist, it will be overwritten.                                */
/*                                                                            */
/* Input:  "section" is the section name where the entry should be stored.    */
/*         "entry" is the name of the entry to write.                         */
/*         "value" is the entry value.                                        */
/******************************************************************************/
void PSettings::WriteStringEntryValue(PString section, PString entry, PString value)
{
	PFileLine line;
	int32 index, insertPos;

	// Start to lock the list
	listLock->WaitToWrite();

	// Now find the section
	index = FindSection(section);
	if (index != -1)
	{
		// Found the section. Now see if the entry exists in the section
		index = FindEntry(index + 1, entry, insertPos);
		if (index != -1)
		{
			// Got it, now overwrite the value
			line = lines.GetItem(index);
			line.line.Format_S2("%s=%s", entry, value);
			lines.SetItem(line, index);
		}
		else
		{
			// The entry couldn't be found, so add a new
			// one to the section
			line.line.Format_S2("%s=%s", entry, value);
			line.type = pEntry;
			lines.InsertItem(line, insertPos);
		}
	}
	else
	{
		// The section couldn't be found, so we create a new one
		line.line = section;
		line.type = pSection;
		lines.AddTail(line);

		// And then add the entry
		line.line.Format_S2("%s=%s", entry, value);
		line.type = pEntry;
		lines.AddTail(line);
	}

	// Settings has been changed
	changed = true;

	// Unlock again
	listLock->DoneWriting();
}



/******************************************************************************/
/* WriteIntEntryValue() will store the entry in the setting file. If it       */
/*      already exist, it will be overwritten.                                */
/*                                                                            */
/* Input:  "section" is the section name where the entry should be stored.    */
/*         "entry" is the name of the entry to write.                         */
/*         "value" is the entry value.                                        */
/******************************************************************************/
void PSettings::WriteIntEntryValue(PString section, PString entry, int32 value)
{
	PString val;

	// Store the number in a string
	val.SetNumber(value);

	// Use the string write function to write the number
	WriteStringEntryValue(section, entry, val);
}



/******************************************************************************/
/* EntryExist() will check to see if the entry exist.                         */
/*                                                                            */
/* Input:  "section" is the section name where the entry is stored.           */
/*         "entry" is the name of the entry to check for.                     */
/*                                                                            */
/* Output: True if the entry exists, false if not.                            */
/******************************************************************************/
bool PSettings::EntryExist(PString section, PString entry) const
{
	int32 index, insertPos;
	bool result = false;

	// Start to lock the list
	listLock->WaitToRead();

	// Start to find the section
	index = FindSection(section);
	if (index != -1)
	{
		// Found the section, now find the entry
		index = FindEntry(index + 1, entry, insertPos);
		if (index != -1)
			result = true;
	}

	// Unlock again
	listLock->DoneReading();

	return (result);
}



/******************************************************************************/
/* RemoveEntry() will remove an entry from the section given. If the entry    */
/*      couldn't be found, nothing is done.                                   */
/*                                                                            */
/* Input:  "section" is the section name where the entry is stored.           */
/*         "entry" is the name of the entry to remove.                        */
/*                                                                            */
/* Output: True if the entry was removed, false if not.                       */
/******************************************************************************/
bool PSettings::RemoveEntry(PString section, PString entry)
{
	int32 sectionIndex, entryIndex, insertPos;
	bool result = false;

	// Start to lock the list
	listLock->WaitToWrite();

	// Now find the section
	sectionIndex = FindSection(section);
	if (sectionIndex != -1)
	{
		// Found the section. Now see if the entry exists in the section
		entryIndex = FindEntry(sectionIndex + 1, entry, insertPos);
		if (entryIndex != -1)
		{
			// Got it, now remove it
			lines.RemoveItem(entryIndex);
			result = true;

			// Was it the last entry in the section?
			if (((sectionIndex + 1) == lines.CountItems()) ||
				((sectionIndex + 1) == entryIndex) && (lines.GetItem(entryIndex).type == pSection))
			{
				// Yes, remove the section
				lines.RemoveItem(sectionIndex);
			}

			// Settings has been changed
			changed = true;
		}
	}

	// Unlock again
	listLock->DoneWriting();

	return (result);
}



/******************************************************************************/
/* RenameEntry() will rename an entry in the section given. If the entry      */
/*      couldn't be found, nothing is done.                                   */
/*                                                                            */
/* Input:  "section" is the section name where the entry is stored.           */
/*         "entry" is the name of the entry to rename.                        */
/*         "newEntry" is the new name of the entry.                           */
/*                                                                            */
/* Output: True if the entry was renamed, false if not.                       */
/******************************************************************************/
bool PSettings::RenameEntry(PString section, PString entry, PString newEntry)
{
	PFileLine line;
	int32 index, insertPos;
	int32 valPos;
	bool result = false;

	// Start to lock the list
	listLock->WaitToWrite();

	// Now find the section
	index = FindSection(section);
	if (index != -1)
	{
		// Found the section. Now see if the entry exists in the section
		index = FindEntry(index + 1, entry, insertPos);
		if (index != -1)
		{
			// Got it, now get the data so we can rename the entry
			line = lines.GetItem(index);

			// Store the new name
			valPos    = line.line.Find('=');
			line.line = newEntry + line.line.Mid(valPos);

			// And store the line back in the list
			lines.SetItem(line, index);
			result = true;

			// Settings has been changed
			changed = true;
		}
	}

	// Unlock again
	listLock->DoneWriting();

	return (result);
}



/******************************************************************************/
/* FindSection() will search from the top after the section given. If found,  */
/*      the list index where the section is stored is returned, else -1 will  */
/*      be returned.                                                          */
/*                                                                            */
/* Input:  "section" is the section name to search for.                       */
/*                                                                            */
/* Output: The section index in the list or -1.                               */
/******************************************************************************/
int32 PSettings::FindSection(PString section) const
{
	PFileLine line;
	int32 i, count;

	// Find out how many lines in the list
	count = lines.CountItems();

	// Okay, start to find the section
	for (i = 0; i < count; i++)
	{
		// Get the line
		line = lines.GetItem(i);

		if (line.type == pSection)
		{
			// Found a section, but is it the one we search for
			if (line.line == section)
			{
				// Yup, return the index
				return (i);
			}
		}
	}

	// The section couldn't be found
	return (-1);
}



/******************************************************************************/
/* FindEntry() will search from the index given after the entry. If found,    */
/*      the list index where the entry is stored is returned, else -1 will    */
/*      be returned.                                                          */
/*                                                                            */
/* Input:  "startIndex" is the index to start the search at.                  */
/*         "entry" is the entry name to search for.                           */
/*         "insertPos" is a reference to where the insert position is stored. */
/*                                                                            */
/* Output: The entry index in the list or -1.                                 */
/******************************************************************************/
int32 PSettings::FindEntry(int32 startIndex, PString entry, int32 &insertPos) const
{
	PFileLine line;
	int32 i, count;
	int32 valPos;

	// Find out how many lines in the list
	count = lines.CountItems();

	// Check start index
	ASSERT(startIndex < count);

	// Okay, start to find the section
	for (i = startIndex; i < count; i++)
	{
		// Get the line
		line = lines.GetItem(i);

		if (line.type == pSection)
		{
			// A new section is found, which mean the entry is
			// not stored in the previous section
			insertPos = i;
			return (-1);
		}

		if (line.type == pEntry)
		{
			// Check the entry name
			valPos = line.line.Find('=');
			if (line.line.Left(valPos) == entry)
			{
				// Found the entry
				insertPos = i;
				return (i);
			}
		}
	}

	// The entry couldn't be found
	insertPos = count;
	return (-1);
}



/******************************************************************************/
/* FindEntryByNumber() will search from the index given after the entry. If   */
/*      found, the list index where the entry is stored is returned, else -1  */
/*      will be returned.                                                     */
/*                                                                            */
/* Input:  "startIndex" is the index to start the search at.                  */
/*         "entryNum" is the entry number to search for.                      */
/*         "insertPos" is a reference to where the insert position is stored. */
/*                                                                            */
/* Output: The entry index in the list or -1.                                 */
/******************************************************************************/
int32 PSettings::FindEntryByNumber(int32 startIndex, int32 entryNum, int32 &insertPos) const
{
	PFileLine line;
	int32 i, count;

	// Get number of lines in the file
	count = lines.CountItems();

	// Start to find the entry
	for (i = 0; i < entryNum; i++)
	{
		// Did we reach the end of the file
		if ((startIndex + i) >= count)
		{
			insertPos = count;
			return (-1);
		}

		// Get the line
		line = lines.GetItem(startIndex + i);

		if (line.type == pSection)
		{
			// A new section is found, which mean the entry is
			// not stored in the previous section
			insertPos = startIndex + i;
			return (-1);
		}

		if (line.type == pComment)
		{
			// Skip comments
			entryNum++;
		}
	}

	// Entry found or end of file reached
	if ((startIndex + i) >= count)
	{
		// EOF reached
		insertPos = count;
		return (-1);
	}

	insertPos = startIndex + i;
	return (insertPos);
}
