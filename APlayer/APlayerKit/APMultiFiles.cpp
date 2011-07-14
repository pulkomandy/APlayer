/******************************************************************************/
/* APlayer Multi Files class.                                                 */
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
#include "PFile.h"
#include "PDirectory.h"
#include "PTime.h"

// APlayerKit headers
#include "APMultiFiles.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APMultiFiles::APMultiFiles(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APMultiFiles::~APMultiFiles(void)
{
}



/******************************************************************************/
/* CheckForMultiFile() will check the file given to see if it's one of the    */
/*      supported multiple file types.                                        */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to check.              */
/*                                                                            */
/* Output: The type of the file.                                              */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
APMultiFiles::MultiFileType APMultiFiles::CheckForMultiFile(PString fileName)
{
	MultiFileType type;

	// Start to check the file for a list type
	if (CheckForListFile(fileName) == apUnknown)
	{
		// Not a list file
		type = apPlain;
	}
	else
	{
		// The file is a list file, so tell the caller
		type = apList;
	}

	return (type);
}



/******************************************************************************/
/* GetMultiFiles() will read the file given and call the callback function    */
/*      for each file in the multi file.                                      */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to load.               */
/*         "func" is the function to call for each file retrieved.            */
/*         "userData" is a pointer to some data you want your function to     */
/*         retrieve. It can be anything and won't be modified.                */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APMultiFiles::GetMultiFiles(PString fileName, FileFunc func, void *userData)
{
	// Start to check the file for a multi file
	switch (CheckForMultiFile(fileName))
	{
		//
		// Plain file
		//
		case apPlain:
		default:
		{
			// Not a multi file, so just call the call back function with
			// the original file
			APMultiFileType fileType;

			// Fill out the type structure
			fileType.type          = apPlain;
			fileType.fileName      = fileName;
			fileType.timeAvailable = false;

			// Call the callback function
			func(&fileType, userData);
			break;
		}

		//
		// List file
		//
		case apList:
		{
			LoadListFile(fileName, func, userData);
			break;
		}
	}
}



/******************************************************************************/
/* SaveAPMLFile() will save the list in APML format to the file given. It     */
/*      will call the callback function to get each list entry.               */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to save.               */
/*         "func" is the function to call for each entry.                     */
/*         "userData" is a pointer to some data you want your function to     */
/*         retrieve. It can be anything and won't be modified.                */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APMultiFiles::SaveAPMLFile(PString fileName, FileFunc func, void *userData)
{
	APMultiFileType typeStruct;
	PCacheFile file;
	PString path, oldPath;
	PString line;

	// Append extension if not already there
	if (fileName.Right(5).CompareNoCase(".apml") != 0)
		fileName += ".apml";

	// Open the file
	file.Open(fileName, PFile::pModeWrite | PFile::pModeCreate);

	// Start to write the header
	file.WriteLine("@*ApML*@");

	// Write all the items to the file
	while (func(&typeStruct, userData))
	{
		// Find the type of the item
		switch (typeStruct.type)
		{
			//
			// Plain file
			//
			case apPlain:
			{
				// Check to see if the path is the same as the previous one
				path = PDirectory::GetDirectoryPart(typeStruct.fileName);

				if (path != oldPath)
				{
					// Store a new path
					file.WriteLine("");
					file.WriteLine("@*Path*@");
					file.WriteLine(path.Left(path.GetLength() - 1));

					// Write name command
					file.WriteLine("");
					file.WriteLine("@*Names*@");

					// Remember the new path
					oldPath = path;
				}
				break;
			}

			//
			// Stop debugger. We haven't made support for a new type
			//
			default:
				ASSERT(false);
				break;
		}

		// Build the line to write
		line = PDirectory::GetFilePart(typeStruct.fileName);
		if (typeStruct.timeAvailable)
			line += ":" + PString::CreateNumber64(typeStruct.time.GetTotalMilliSeconds());

		// And then write it
		file.WriteLine(line);
	}

	// Done with the file, now set the file type
	file.SetFileType("text/x-aplayer-playlist");
}



/******************************************************************************/
/* CheckForListFile() will check the file given to see if it's one of the     */
/*      supported file list types.                                            */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to check.              */
/*                                                                            */
/* Output: The type of the file.                                              */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
APMultiFiles::ListFileType APMultiFiles::CheckForListFile(PString fileName)
{
	PFile file;
	PString fileType;
	char buffer[16];

	// Open the file
	file.Open(fileName, PFile::pModeRead);

	// Read the first line
	file.Read(buffer, 16);

	if (memcmp(buffer, "@*ApML*@", 8) == 0)
		return (apAPlayer);

	if (memcmp(buffer, "#EXTM3U", 7) == 0)
		return (apWinAmp);

	try
	{
		// Check the file type
		fileType = file.GetFileType();

		if (fileType.CompareNoCase("text/x-cl-amp-playlist") == 0)
			return (apCL_Amp);

		if ((fileType.CompareNoCase("text/x-soundplay-playlist") == 0) ||
			(fileType.CompareNoCase("text/x-playlist") == 0))
			return (apSoundPlay);

		// Check file name
		if (file.GetFileExtension(fileName).CompareNoCase(".m3u") == 0)
			return (apWinAmp);
	}
	catch(PFileException e)
	{
		// Ignore any errors
		;
	}

	return (apUnknown);
}



/******************************************************************************/
/* LoadListFile() will read the list file given and call the callback         */
/*      function for each file in the list.                                   */
/*                                                                            */
/* Input:  "fileName" is the name of the file you want to load.               */
/*         "func" is the function to call for each file retrieved.            */
/*         "userData" is a pointer to some data you want your function to     */
/*         retrieve. It can be anything and won't be modified.                */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APMultiFiles::LoadListFile(PString fileName, FileFunc func, void *userData)
{
	APMultiFileType typeStruct;
	PCacheFile file;
	ListFileType type;
	PString line;

	// Get the list type
	type = CheckForListFile(fileName);

	// Open the file for reading
	file.Open(fileName, PFile::pModeRead);

	switch (type)
	{
		//
		// APML
		//
		case apAPlayer:
		{
			PString path;
			int32 timePos;

			// Skip the header
			file.ReadLine();

			// At the moment, only plain files are supported in the APML list
			typeStruct.type = apPlain;

			// Start to "compile" the file
			while (!file.IsEOF())
			{
				line = file.ReadLine();

				if (!line.IsEmpty())
				{
					// "Path" command
					if (line == "@*Path*@")
						path = file.ReadLine();
					else
					{
						// If not the "Names" command, it's a file name
						if (line != "@*Names*@")
						{
							// See if there is stored a module time
							timePos = line.ReverseFind(':');
							if (timePos != -1)
							{
								// Set the time
								typeStruct.time.SetTimeSpan(line.Mid(timePos + 1).GetNumber64());
								typeStruct.timeAvailable = true;
								line = line.Left(timePos);
							}
							else
								typeStruct.timeAvailable = false;

							// Check to see if there is loaded any path
							if (path.IsEmpty())
							{
								// Set the file name using the load path
								typeStruct.fileName = PDirectory::GetDirectoryPart(fileName) + line;
							}
							else
							{
								// Set the file name
								typeStruct.fileName = PDirectory::EnsureDirectoryName(path) + line;
							}

							// Call the callback function
							if (!func(&typeStruct, userData))
								return;
						}
					}
				}
			}
			break;
		}

		//
		// CL-Amp or SoundPlay
		//
		case apCL_Amp:
		case apSoundPlay:
		{
			// Only plain files are supported and no time support
			typeStruct.type          = apPlain;
			typeStruct.timeAvailable = false;

			while (!file.IsEOF())
			{
				// Read one item
				line = file.ReadLine();

				// Remove any white spaces
				line.TrimRight();
				line.TrimLeft();

				if (!line.IsEmpty())
				{
					// Check to see if there is any directory in the name
					if (PDirectory::GetDirectoryPart(line).IsEmpty())
					{
						// Set the file name, using the load path
						typeStruct.fileName = PDirectory::GetDirectoryPart(fileName) + line;
					}
					else
					{
						// Set the file name
						typeStruct.fileName = line;
					}

					// Call the callback function
					if (!func(&typeStruct, userData))
						return;
				}
			}
			break;
		}

		//
		// WinAmp
		//
		case apWinAmp:
		{
			int32 timePos = -1;
			PCharSet_MS_WIN_1252 charSet;

			// Only plain files are supported
			typeStruct.type = apPlain;

			// Reset the time indicator
			typeStruct.timeAvailable = false;

			while (!file.IsEOF())
			{
				// Read one item
				line = file.ReadLine(&charSet);

				// Remove any white spaces
				line.TrimRight();
				line.TrimLeft();

				if (!line.IsEmpty())
				{
					// First line?
					if (line.Left(7) == "#EXTM3U")
						continue;		// Skip it

					// Do we have some extra information?
					if (line.Left(8) == "#EXTINF:")
					{
						// Extract the time
						timePos = line.Find(',');
						if (timePos != -1)
						{
							typeStruct.time.SetTimeSpan(line.Mid(8, timePos - 8).GetNumber64() * 1000);
							typeStruct.timeAvailable = true;
						}
						continue;		// Read the next line
					}

					// Check to see if there is any directory in the name
					if (PDirectory::GetDirectoryPart(line).IsEmpty())
					{
						// Set the file name, using the load path
						typeStruct.fileName = PDirectory::GetDirectoryPart(fileName) + line;
					}
					else
					{
						// Set the file name
						typeStruct.fileName = line;
					}

					// Call the callback function
					func(&typeStruct, userData);

					// Reset the time indicator
					typeStruct.timeAvailable = false;
				}
			}
			break;
		}

		//
		// Stop debugger. We haven't made support for a new type
		//
		default:
			ASSERT(false);
			break;
	}
}
