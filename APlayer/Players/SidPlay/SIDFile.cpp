/******************************************************************************/
/* SIDFile implementation file.                                               */
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
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"	

// Player headers
#include "SIDPlayer.h"
#include "SIDFile.h"
#include "SIDTune.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Filename extension supported                                               */
/******************************************************************************/
static const char *defaultExtensions[] =
{
	// Preferred default file extension for single-file sidtunes
	// or sidtune description files in SIDPLAY INFOFILE format.
	".sid",

	// Common file extension for single-file sidtunes due to SIDPLAY/DOS
	// displaying files *.DAT in its file selector by default.
	// Originally this was intended to be the extension of the raw data file
	// of two-file sidtunes in SIDPLAY INFOFILE format.
	".dat",

	// Extension of Amiga Workbench tooltype icon info files, which
	// have been cut to MS-DOS file name length (8.3).
	".inf",

	// No extension for the raw data file of two-file sidtunes in
	// PlaySID Amiga Workbench tooltype icon info format.
	"",

	// Common upper-case file extensions from MS-DOS (unconverted).
	".DAT", ".SID", ".INF",

	// File extensions used (and created) by various C64 emulators and
	// related utilities. These extensions are recommended to be used as
	// a replacement for ".dat" in conjunction with two-file sidtunes.
	".c64", ".prg", ".C64", ".PRG",

	// Uncut extensions from Amiga.
	".info", ".INFO", ".data", ".DATA",

	// End.
	NULL
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDFile::SIDFile(PFile *file, PResource *resource, SIDPlayer *play)
{
	player      = play;
	curFile     = file;
	descripFile = NULL;

	modAdr      = NULL;
	modLen      = 0;

	res         = resource;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDFile::~SIDFile(void)
{
	delete[] modAdr;
}



/******************************************************************************/
/* TestIt() will test the file to see if it's one of the known formats.       */
/*                                                                            */
/* Output: Is an ap_result return code.                                       */
/******************************************************************************/
ap_result SIDFile::TestIt(void)
{
	PString fileName, testFileName;
	PCacheFile *testFile = NULL;
	const char **tempExt;
	int32 pos;
	ap_result result;

	// Check for single file formats
	result = TestPSID(curFile);
	if (result == AP_UNKNOWN)
	{
		result = TestMUS(curFile);
		if (result == AP_UNKNOWN)
		{
			// Now check for multiple-files formats
			// We can't simply try to load additional files, if a description
			// file was specified. It would work, but is error-prone. Imagine
			// a filename mismatch or more than one description file (in
			// another) format. Any other file with an appropriate file name
			// can be the C64 data file.
			// First we see if the file already loaded could be a raw data
			// file. In that case we have to find the corresponding description
			// file.
			//
			// Make sure that the file in memory does not contain a description file
			if ((TestSID(curFile) == AP_UNKNOWN) && (TestINFO(curFile) == AP_UNKNOWN))
			{
				tempExt  = &defaultExtensions[0];
				fileName = curFile->GetFullPath();

				while (*tempExt != NULL)
				{
					// Create filename to check
					pos = fileName.ReverseFind('.');
					if (pos >= 0)
						testFileName = fileName.Left(pos) + *tempExt;
					else
						testFileName = fileName + *tempExt;

					// Do not load the file again if the names are equal
					if (testFileName != fileName)
					{
						try
						{
							testFile = new PCacheFile(testFileName, PFile::pModeRead);

							if ((TestSID(testFile) == AP_OK) || (TestINFO(testFile) == AP_OK))
							{
								// Found a module in SID or INFO format
								descripFile   = testFile;
								deleteDescrip = true;

								return (AP_OK);
							}

							// Well, close the file again
							testFile->Close();
							delete testFile;
							testFile = NULL;
						}
						catch(...)
						{
							delete testFile;
							testFile = NULL;
						}
					}

					// Go to the next extension
					tempExt++;
				}

				return (AP_UNKNOWN);
			}
			else
			{
				// Still unsuccessful? Probally the user have selected a
				// description file, so now we create the name of the data
				// file and swap both used memory buffers when calling
				// the test functions.
				// If it works, the second file is the data file! If it is
				// not, but does exists, we are out of luck, since we cannot
				// detect data files.
				if ((TestSID(curFile) == AP_OK) || (TestINFO(curFile) == AP_OK))
				{
					tempExt  = &defaultExtensions[0];
					fileName = curFile->GetFullPath();

					while (*tempExt != NULL)
					{
						// Create filename to check
						pos = fileName.ReverseFind('.');
						if (pos >= 0)
							testFileName = fileName.Left(pos) + *tempExt;
						else
							testFileName = fileName + *tempExt;

						// Do not load the file again if the names are equal
						if (testFileName != fileName)
						{
							try
							{
								testFile = new PCacheFile(testFileName, PFile::pModeRead);

								if ((TestSID(curFile) == AP_OK) || (TestINFO(curFile) == AP_OK))
								{
									// Found a module in SID or INFO format,
									// so load it into the memory
									descripFile   = curFile;
									curFile       = testFile;
									deleteDescrip = false;

									return (AP_OK);
								}

								// Well, close the file again
								testFile->Close();
								delete testFile;
								testFile = NULL;
							}
							catch(...)
							{
								delete testFile;
								testFile = NULL;
							}
						}

						// Go to the next extension
						tempExt++;
					}

					return (AP_UNKNOWN);
				}
			}
		}
	}

	return (result);
}



/******************************************************************************/
/* TestPSID() will test the file to see if it's a PSID file.                  */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*                                                                            */
/* Output: Is an ap_result return code.                                       */
/******************************************************************************/
ap_result SIDFile::TestPSID(PFile *file)
{
	uint32 id;
	uint16 version;

	// We need at least as long as version 2 plus C64 load address data.
	if (file->GetLength() < 126)
		return (AP_UNKNOWN);

	// Read the ID and version
	file->SeekToBegin();
	id      = file->Read_B_UINT32();
	version = file->Read_B_UINT16();

	// Check it
	if ((id == 'PSID') && ((version == 1) || (version == 2)))
	{
		type = apPSID;
		return (AP_OK);
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* TestMUS() will test the file to see if it's a MUS file.                    */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*                                                                            */
/* Output: Is an ap_result return code.                                       */
/******************************************************************************/
ap_result SIDFile::TestMUS(PFile *file)
{
	int32 voice1Index, voice2Index, voice3Index;

	// Skip load address and 3x length entry
	voice1Index = (2 + 3 * 2);

	// Add length of voice 1 data
	file->Seek(2, PFile::pSeekBegin);
	voice1Index += file->Read_L_UINT16();

	// Add length of voice 2 data
	voice2Index = voice1Index + file->Read_L_UINT16();

	// Add length of voice 3 data
	voice3Index = voice2Index + file->Read_L_UINT16();

	// Okay, check to see if it's a MUS file
	if (voice1Index < file->GetLength())
	{
		file->Seek(voice1Index - 2, PFile::pSeekBegin);
		if (file->Read_L_UINT16() == 0x4f01)
		{
			if (voice2Index < file->GetLength())
			{
				file->Seek(voice2Index - 2, PFile::pSeekBegin);
				if (file->Read_L_UINT16() == 0x4f01)
				{
					if (voice3Index < file->GetLength())
					{
						file->Seek(voice3Index - 2, PFile::pSeekBegin);
						if (file->Read_L_UINT16() == 0x4f01)
						{
							type = apMUS;
							return (AP_OK);
						}
					}
				}
			}
		}
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* TestSID() will test the file to see if it's a SID file.                    */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*                                                                            */
/* Output: Is an ap_result return code.                                       */
/******************************************************************************/
ap_result SIDFile::TestSID(PFile *file)
{
	bool hasAddress, hasName, hasAuthor, hasCopyright, hasSongs, hasSpeed;
	int32 pos;
	PString line, keyword;
	char buffer[16];

	// Check for minimum file size. If it's smaller, we will not proceed
	if (file->GetLength() < 17)
		return (AP_UNKNOWN);

	// The first line has to contain the exact identification string
	file->SeekToBegin();
	file->Read(buffer, 16);
	if (memcmp(buffer, "SIDPLAY INFOFILE", 16) != 0)
		return (AP_UNKNOWN);

	// Initialize variables
	hasAddress   = false;
	hasName      = false;
	hasAuthor    = false;
	hasCopyright = false;
	hasSongs     = false;
	hasSpeed     = false;

	// Read the first line again, so we skip it probably
	file->SeekToBegin();
	file->ReadLine();

	// Parse as long we have not collected all required entries
	while (!hasAddress || !hasName || !hasAuthor || !hasCopyright || !hasSongs || !hasSpeed)
	{
		// Skip to next line. Leave loop, if none
		line = file->ReadLine();
		if (line.IsEmpty())
			break;

		// Find equal sign
		pos = line.Find('=');
		if (pos < 0)
			break;

		// We found it, now clip out the keyword
		keyword = line.Left(pos);
		keyword.TrimLeft();
		keyword.TrimRight();

		// Okay, check for possible keywords
		if (keyword.CompareNoCase("ADDRESS") == 0)
			hasAddress = true;
		else if (keyword.CompareNoCase("NAME") == 0)
			hasName = true;
		else if (keyword.CompareNoCase("AUTHOR") == 0)
			hasAuthor = true;
		else if (keyword.CompareNoCase("COPYRIGHT") == 0)
			hasCopyright = true;
		else if (keyword.CompareNoCase("SONGS") == 0)
			hasSongs = true;
		else if (keyword.CompareNoCase("SPEED") == 0)
			hasSpeed = true;
	}

	// Again, check for the required values
	if (hasAddress && hasName && hasAuthor && hasCopyright && hasSongs && hasSpeed)
	{
		type = apSID;
		return (AP_OK);
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* TestINFO() will test the file to see if it's a INFO file.                  */
/*                                                                            */
/* Input:  "file" is a pointer to a file object with the file to check.       */
/*                                                                            */
/* Output: Is an ap_result return code.                                       */
/******************************************************************************/
ap_result SIDFile::TestINFO(PFile *file)
{
	int32 minSize;
	uint32 i;
	DiskObject diskObject;
	Border border;
	Image image;
	uint32 iconPos;
	uint32 imgSize;
	bool hasAddress, hasName, hasAuthor, hasCopyright, hasSongs, hasSpeed, hasUnknownChunk;
	int32 pos;
	char strBuf[256];
	PString cmpStr;
	PCharSet_Amiga charSet;

	// Require a first minimum safety size
	minSize = 1 + sizeof(DiskObject);
	if (file->GetLength() < minSize)
		return (AP_UNKNOWN);

	// Read the DiskObject structure into memory
	file->SeekToBegin();

	diskObject.magic                = file->Read_B_UINT16();
	diskObject.version              = file->Read_B_UINT16();
	diskObject.gadget.nextGadget    = file->Read_B_UINT32();
	diskObject.gadget.leftEdge      = file->Read_B_UINT16();
	diskObject.gadget.topEdge       = file->Read_B_UINT16();
	diskObject.gadget.width         = file->Read_B_UINT16();
	diskObject.gadget.height        = file->Read_B_UINT16();
	diskObject.gadget.flags         = file->Read_B_UINT16();
	diskObject.gadget.activation    = file->Read_B_UINT16();
	diskObject.gadget.gadgetType    = file->Read_B_UINT16();
	diskObject.gadget.gadgetRender  = file->Read_B_UINT32();
	diskObject.gadget.selectRender  = file->Read_B_UINT32();
	diskObject.gadget.gadgetText    = file->Read_B_UINT32();
	diskObject.gadget.mutualExclude = file->Read_B_UINT32();
	diskObject.gadget.specialInfo   = file->Read_B_UINT32();
	diskObject.gadget.gadgetID      = file->Read_B_UINT16();
	diskObject.gadget.userData      = file->Read_B_UINT32();
	diskObject.type                 = file->Read_UINT8();
	file->Seek(1, PFile::pSeekCurrent);		// Skip pad byte
	diskObject.defaultTool          = file->Read_B_UINT32();
	diskObject.toolTypes            = file->Read_B_UINT32();
	diskObject.currentX             = file->Read_B_UINT32();
	diskObject.currentY             = file->Read_B_UINT32();
	diskObject.drawerData           = file->Read_B_UINT32();
	diskObject.toolWindow           = file->Read_B_UINT32();
	diskObject.stackSize            = file->Read_B_UINT32();

	// Require Magic ID in the first two bytes of the file
	if (diskObject.magic != WB_DISKMAGIC)
		return (AP_UNKNOWN);

	// Only version 1.x supported
	if (diskObject.version != WB_DISKVERSION)
		return (AP_UNKNOWN);

	// A PlaySID icon must be of type project
	if (diskObject.type != WB_PROJECT)
		return (AP_UNKNOWN);

	// We want to skip a possible Gadget Image item
	iconPos = file->GetPosition();

	if ((diskObject.gadget.flags & GFLG_GADGIMAGE) == 0)
	{
		// Calculate size of gadget borders (vector image)
		if (diskObject.gadget.gadgetRender != 0)	// Border present?
		{
			// Require another minimum safety size
			minSize += sizeof(Border);
			if (file->GetLength() < minSize)
				return (AP_UNKNOWN);

			// Load the border structure
			file->Seek(iconPos, PFile::pSeekBegin);

			border.leftEdge   = file->Read_B_UINT16();
			border.topEdge    = file->Read_B_UINT16();
			border.frontPen   = file->Read_UINT8();
			border.backPen    = file->Read_UINT8();
			border.drawMode   = file->Read_UINT8();
			border.count      = file->Read_UINT8();
			border.xy         = file->Read_B_UINT32();
			border.nextBorder = file->Read_B_UINT32();

			iconPos += (2 * 2 + 1 * 4 + 2 * 4);		// sizeof(Border)
			iconPos += border.count * (2 + 2);		// Pair of uint16
		}

		if (diskObject.gadget.selectRender != 0)	// Alternate Border present?
		{
			// Require another minimum safety size
			minSize += sizeof(Border);
			if (file->GetLength() < minSize)
				return (AP_UNKNOWN);

			// Load the border structure
			file->Seek(iconPos, PFile::pSeekBegin);

			border.leftEdge   = file->Read_B_UINT16();
			border.topEdge    = file->Read_B_UINT16();
			border.frontPen   = file->Read_UINT8();
			border.backPen    = file->Read_UINT8();
			border.drawMode   = file->Read_UINT8();
			border.count      = file->Read_UINT8();
			border.xy         = file->Read_B_UINT32();
			border.nextBorder = file->Read_B_UINT32();

			iconPos += (2 * 2 + 1 * 4 + 2 * 4);		// sizeof(Border)
			iconPos += border.count * (2 + 2);		// Pair of uint16
		}
	}
	else
	{
		// Calculate size of gadget images (bitmap image)
		if (diskObject.gadget.gadgetRender != 0)	// Image present?
		{
			// Require another minimum safety size
			minSize += sizeof(Image);
			if (file->GetLength() < minSize)
				return (AP_UNKNOWN);

			// Read image structure
			file->Seek(iconPos, PFile::pSeekBegin);

			image.leftEdge   = file->Read_B_UINT16();
			image.topEdge    = file->Read_B_UINT16();
			image.width      = file->Read_B_UINT16();
			image.height     = file->Read_B_UINT16();
			image.depth      = file->Read_B_UINT16();
			image.imageData  = file->Read_B_UINT32();
			image.planePick  = file->Read_UINT8();
			image.planeOnOff = file->Read_UINT8();
			image.nextImage  = file->Read_B_UINT32();

			iconPos += (2 * 5 + 4 + 1 * 2 + 4);		// sizeof(Image)

			imgSize = 0;
			for (i = 0; i < image.depth; i++)
			{
				if ((image.planePick & (1 << i)) != 0)
				{
					// NOTE: Intuition relies on planePick to know how many planes
					// of data are found in imageData. There should be no more
					// '1'-bits in planePick than there are planes in imageData
					imgSize++;
				}
			}

			imgSize *= ((image.width + 15) / 16) * 2;	// Bytes per line
			imgSize *= image.height;					// Bytes per plane
			iconPos += imgSize;
		}

		if (diskObject.gadget.selectRender != 0)	// Alternate image present?
		{
			// Require another minimum safety size
			minSize += sizeof(Image);
			if (file->GetLength() < minSize)
				return (AP_UNKNOWN);

			// Read image structure
			file->Seek(iconPos, PFile::pSeekBegin);

			image.leftEdge   = file->Read_B_UINT16();
			image.topEdge    = file->Read_B_UINT16();
			image.width      = file->Read_B_UINT16();
			image.height     = file->Read_B_UINT16();
			image.depth      = file->Read_B_UINT16();
			image.imageData  = file->Read_B_UINT32();
			image.planePick  = file->Read_UINT8();
			image.planeOnOff = file->Read_UINT8();
			image.nextImage  = file->Read_B_UINT32();

			iconPos += (2 * 5 + 4 + 1 * 2 + 4);		// sizeof(Image)

			imgSize = 0;
			for (i = 0; i < image.depth; i++)
			{
				if ((image.planePick & (1 << i)) != 0)
				{
					// NOTE: Intuition relies on planePick to know how many planes
					// of data are found in imageData. There should be no more
					// '1'-bits in planePick than there are planes in imageData
					imgSize++;
				}
			}

			imgSize *= ((image.width + 15) / 16) * 2;	// Bytes per line
			imgSize *= image.height;					// Bytes per plane
			iconPos += imgSize;
		}
	}

	// Here we use a smart pointer to prevent access violation errors
	file->Seek(iconPos, PFile::pSeekBegin);

	// Skip default tool
	file->Seek(file->Read_B_UINT32(), PFile::pSeekCurrent);

	// Defaults
	speed   = 0;
	musPlay = false;

	// Initialize variables
	hasAddress      = false;
	hasName         = false;
	hasAuthor       = false;
	hasCopyright    = false;
	hasSongs        = false;
	hasSpeed        = false;
	hasUnknownChunk = false;

	// Calculate number of tooltype strings
	i = file->Read_B_UINT32();
	if (i != 0)
	{
		i = (i / 4) - 1;

		while (i-- > 0)
		{
			// Get length of this tool
			uint32 toolLen = file->Read_B_UINT32();

			// Copy item to a string
			file->ReadString(strBuf, toolLen);
			cmpStr.SetString(strBuf, &charSet);

			// Now check all possible keywords
			if (cmpStr.Left(8) == "ADDRESS=")
			{
				cmpStr     = cmpStr.Mid(8);
				loadAdr    = ReadHex(cmpStr, &pos);
				cmpStr     = cmpStr.Mid(pos);
				initAdr    = ReadHex(cmpStr, &pos);
				playAdr    = ReadHex(cmpStr.Mid(pos), &pos);
				hasAddress = true;
			}
			else if (cmpStr.Left(6) == "SONGS=")
			{
				cmpStr    = cmpStr.Mid(6);
				songs     = ReadDec(cmpStr, &pos);
				startSong = ReadDec(cmpStr.Mid(pos), &pos);
				hasSongs  = true;
			}
			else if (cmpStr.Left(6) == "SPEED=")
			{
				speed    = ReadHex(cmpStr.Mid(6), &pos);
				hasSpeed = true;
			}
			else if (cmpStr.Left(5) == "NAME=")
			{
				name    = cmpStr.Mid(5);
				hasName = true;
			}
			else if (cmpStr.Left(7) == "AUTHOR=")
			{
				author    = cmpStr.Mid(7);
				hasAuthor = true;
			}
			else if (cmpStr.Left(10) == "COPYRIGHT=")
			{
				copyright    = cmpStr.Mid(10);
				hasCopyright = true;
			}
			else if (cmpStr.Left(11) == "SIDSONG=YES")
			{
				musPlay = true;
			}
			else
				hasUnknownChunk = true;
		}
	}

	// Check to see if all tool types required are found
	if (hasAddress && hasName && hasAuthor && hasCopyright && hasSongs && hasSpeed)
	{
		type = apINFO;
		return (AP_OK);
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* FillInInfo() will take the information from the SID module and put it into */
/*         the public class variables.                                        */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void SIDFile::FillInInfo(void)
{
	uint32 offset = 0;
	uint16 i;

	// Start to clear the info strings
	name.MakeEmpty();
	author.MakeEmpty();
	copyright.MakeEmpty();

	for (i = 0; i < 5; i++)
		info[i].MakeEmpty();

	// Initialize the info depending on the file type
	switch (type)
	{
		////////////////////////////////////////////////////////////////////////
		// PSID
		////////////////////////////////////////////////////////////////////////
		case apPSID:
		{
			uint16 version, flags;
			char buffer[34];
			PCharSet_Amiga charSet;

			// Seek to start of header and skip the id
			curFile->Seek(4, PFile::pSeekBegin);
			version = curFile->Read_B_UINT16();
			offset  = curFile->Read_B_UINT16();

			// Load the needed information
			loadAdr   = curFile->Read_B_UINT16();
			initAdr   = curFile->Read_B_UINT16();
			playAdr   = curFile->Read_B_UINT16();
			songs     = curFile->Read_B_UINT16();
			startSong = curFile->Read_B_UINT16();
			speed     = curFile->Read_B_UINT32();

			// Get the strings
			curFile->ReadString(buffer, 32);
			name.SetString(buffer, &charSet);

			curFile->ReadString(buffer, 32);
			author.SetString(buffer, &charSet);

			curFile->ReadString(buffer, 32);
			copyright.SetString(buffer, &charSet);

			flags = curFile->Read_B_UINT16();

			musPlay      = false;
			psidSpecific = false;

			if (version >= 2)
			{
				if (flags & 0x01)
					musPlay = true;

				if (flags & 0x02)
					psidSpecific = true;

				clock          = (flags & 0x0c) >> 2;
				sidModel       = (flags & 0x30) >> 4;
				relocStartPage = curFile->Read_UINT8();
				relocPages     = curFile->Read_UINT8();
			}
			else
			{
				clock          = SIDTUNE_CLOCK_UNKNOWN;
				sidModel       = SIDTUNE_SIDMODEL_UNKNOWN;
				relocStartPage = 0;
				relocPages     = 0;
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// MUS
		////////////////////////////////////////////////////////////////////////
		case apMUS:
		{
			char buffer[34];
			char c;
			uint16 count;
			uint32 index;

			offset         = 2;
			loadAdr        = 0x1000;
			initAdr        = 0xc7b0;
			playAdr        = 0;
			songs          = 1;
			startSong      = 1;
			speed          = 0x00000001;
			musPlay        = true;
			psidSpecific   = false;
			clock          = SIDTUNE_CLOCK_UNKNOWN;
			sidModel       = SIDTUNE_SIDMODEL_UNKNOWN;
			relocStartPage = 0;
			relocPages     = 0;

			// Skip load address and 3x length entry
			index = (2 + 3 * 2);

			// Add length of voice 1 data
			curFile->Seek(2, PFile::pSeekBegin);
			index += curFile->Read_L_UINT16();

			// Add length of voice 2 data
			index += curFile->Read_L_UINT16();

			// Add length of voice 3 data
			index += curFile->Read_L_UINT16();

			// Seek to the start of the info strings
			curFile->Seek(index, PFile::pSeekBegin);

			// Convert the info strings from pet format
			for (i = 0; i < 5; i++)
			{
				count  = 0;

				do
				{
					c = curFile->Read_UINT8();

					// Do not forget to fix the Pet-filtering!
					if ((c >= 0x20) && (c < 96) && (count < 31))
						buffer[count++] = c;
				}
				while ((c != 0x0d) && (c != 0x00) && (!curFile->IsEOF()));

				buffer[count] = 0x00;
				info[i] = buffer;
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// SID
		////////////////////////////////////////////////////////////////////////
		case apSID:
		{
			int32 pos;
			PString line, keyword;

			// Initialize to default values
			offset         = 0;
			musPlay        = false;
			psidSpecific   = false;
			clock          = SIDTUNE_CLOCK_UNKNOWN;
			sidModel       = SIDTUNE_SIDMODEL_UNKNOWN;
			relocStartPage = 0;
			relocPages     = 0;

			descripFile->SeekToBegin();
			descripFile->ReadLine();

			// Parse as long we have not collected all required entries
			for (;;)
			{
				// Skip to next line. Leave loop, if none
				line = descripFile->ReadLine();
				if (line.IsEmpty())
					break;

				// Find equal sign
				pos = line.Find('=');
				if (pos == -1)
					break;

				// We found it, now clip out the keyword
				keyword = line.Left(pos);
				keyword.TrimLeft();
				keyword.TrimRight();

				// Okay, check for possible keywords
				line = line.Mid(pos + 1);

				if (keyword.CompareNoCase("ADDRESS") == 0)
				{
					loadAdr = ReadHex(line, &pos);
					line    = line.Mid(pos);
					initAdr = ReadHex(line, &pos);
					playAdr = ReadHex(line.Mid(pos), &pos);
				}
				else if (keyword.CompareNoCase("NAME") == 0)
				{
					name = line;
					name.TrimRight();
				}
				else if (keyword.CompareNoCase("AUTHOR") == 0)
				{
					author = line;
					author.TrimRight();
				}
				else if (keyword.CompareNoCase("COPYRIGHT") == 0)
				{
					copyright = line;
					copyright.TrimRight();
				}
				else if (keyword.CompareNoCase("SONGS") == 0)
				{
					songs     = ReadDec(line, &pos);
					startSong = ReadDec(line.Mid(pos), &pos);
				}
				else if (keyword.CompareNoCase("SPEED") == 0)
					speed = ReadHex(line, &pos);
				else if (keyword.CompareNoCase("SIDSONG") == 0)
				{
					if (line.CompareNoCase("YES") == 0)
						musPlay = true;
				}
			}
			break;
		}

		////////////////////////////////////////////////////////////////////////
		// INFO
		////////////////////////////////////////////////////////////////////////
		case apINFO:
		{
			// We don't need to do anything, because the whole stuff are done
			// in the test function
			TestINFO(descripFile);

			offset         = 0;
			psidSpecific   = false;
			clock          = SIDTUNE_CLOCK_UNKNOWN;
			sidModel       = SIDTUNE_SIDMODEL_UNKNOWN;
			relocStartPage = 0;
			relocPages     = 0;
			break;
		}
	}

	// Fix the information
	if (copyright.IsEmpty())
		copyright.LoadString(res, IDS_SID_NOTAVAILABLE);

	// Allocate memory to hold the C64 module
	modLen = curFile->GetLength() - offset;
	modAdr = new uint8[modLen];
	if (modAdr == NULL)
		throw PMemoryException();

	// Seek to the data part of the file
	curFile->Seek(offset, PFile::pSeekBegin);

	if (loadAdr == 0)
	{
		loadAdr = curFile->Read_L_UINT16();
		modLen -= 2;
	}

	if (initAdr == 0)
		initAdr = loadAdr;

	if (songs > sidClassMaxSongs)
		songs = sidClassMaxSongs;

	if (startSong == 0)
		startSong = 1;

	// Decrement the start song, because APlayer starts with 0, but
	// SidPlay starts with 1.
	startSong--;

	// Now adjust MUS songs
	if (musPlay)
	{
		loadAdr = 0x1000;
		initAdr = 0xc7b0;
		playAdr = 0x0000;
	}

	// Load the file into the memory
	curFile->Read(modAdr, modLen);

	// Do we need to delete the description file
	if (descripFile != NULL)
	{
		// This is done, just so the file size of the
		// description file will be added to the total size
		if (deleteDescrip)
		{
			PFile *tempFile = player->OpenExtraFile(descripFile->GetFullPath(), "");
			player->CloseExtraFile(tempFile);
			delete descripFile;
		}
		else
		{
			PFile *tempFile = player->OpenExtraFile(curFile->GetFullPath(), "");
			player->CloseExtraFile(tempFile);
			delete curFile;
		}
	}
}



/******************************************************************************/
/* ReadHex() will read a hex number from the string and return it.            */
/*                                                                            */
/* Input : "line" is the string to read the string from.                      */
/*         "nextPos" will have the position to the next number in the string. */
/*                                                                            */
/* Output: Is the number stored in the string.                                */
/******************************************************************************/
uint32 SIDFile::ReadHex(PString line, int32 *nextPos)
{
	uint32 num = 0;
	PChar c;

	// Loop until we got a space character
	*nextPos = 0;
	while ((*nextPos < line.GetLength()) && (!(line.GetAt(*nextPos).IsSpace())))
	{
		c = line.GetAt(*nextPos);
		*nextPos = *nextPos + 1;

		if ((c != ',') && (c != ':'))
		{
			char c1 = c.GetChar()[0] & 0xdf;
			(c1 < 0x3a) ? (c1 &= 0x0f) : (c1 -= (0x41 - 0x0a));
			num <<= 4;
			num  |= (uint32)c1;
		}
		else
			break;
	}

	return (num);
}



/******************************************************************************/
/* ReadDec() will read a decimal number from the string and return it.        */
/*                                                                            */
/* Input : "line" is the string to read the string from.                      */
/*         "nextPos" will have the position to the next number in the string. */
/*                                                                            */
/* Output: Is the number stored in the string.                                */
/******************************************************************************/
uint32 SIDFile::ReadDec(PString line, int32 *nextPos)
{
	uint32 num = 0;
	PChar c;

	// Loop until we got a space character
	*nextPos = 0;
	while ((*nextPos < line.GetLength()) && (!(line.GetAt(*nextPos).IsSpace())))
	{
		c = line.GetAt(*nextPos);
		*nextPos = *nextPos + 1;

		if ((c != ',') && (c != ':'))
		{
			char c1 = c.GetChar()[0] & 0x0f;
			num *= 10;
			num += (uint32)c1;
		}
		else
			break;
	}

	return (num);
}
