/******************************************************************************/
/* Block Interface.                                                           */
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

// Player headers
#include "MEDTypes.h"
#include "Block.h"


/******************************************************************************/
/*                                                                            */
/* MED_Cmd class                                                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* SetCmdData() changes the command and arguments.                            */
/*                                                                            */
/* Input:  "cmd" is the new command.                                          */
/*         "data" is argument 1.                                              */
/*         "data2" is argument 2.                                             */
/******************************************************************************/
void MED_Cmd::SetCmdData(uint8 cmd, uint8 data, uint8 data2)
{
	cmdNum = cmd;
	data0  = data;
	data1  = data2;
}



/******************************************************************************/
/* SetData() changes both arguments.                                          */
/*                                                                            */
/* Input:  "data" is the first argument.                                      */
/*         "data2" is the second argument.                                    */
/******************************************************************************/
void MED_Cmd::SetData(uint8 data, uint8 data2)
{
	data0 = data;
	data1 = data2;
}



/******************************************************************************/
/* SetData() changes the first argument.                                      */
/*                                                                            */
/* Input:  "data" is the argument.                                            */
/******************************************************************************/
void MED_Cmd::SetData(uint8 data)
{
	data0 = data;
}



/******************************************************************************/
/* SetData2() changes the second argument.                                    */
/*                                                                            */
/* Input:  "data" is the argument.                                            */
/******************************************************************************/
void MED_Cmd::SetData2(uint8 data)
{
	data1 = data;
}



/******************************************************************************/
/* GetCmd() returns the command.                                              */
/*                                                                            */
/* Output: The command.                                                       */
/******************************************************************************/
uint8 MED_Cmd::GetCmd(void) const
{
	return (cmdNum);
}



/******************************************************************************/
/* GetData() returns all the arguments.                                       */
/*                                                                            */
/* Output: The arguments.                                                     */
/******************************************************************************/
uint16 MED_Cmd::GetData(void) const
{
	return (data0 * 256 + data1);
}



/******************************************************************************/
/* GetDataB() returns the command argument 1.                                 */
/*                                                                            */
/* Output: The argument.                                                      */
/******************************************************************************/
uint8 MED_Cmd::GetDataB(void) const
{
	return (data0);
}



/******************************************************************************/
/* GetData2() returns the command argument 2.                                 */
/*                                                                            */
/* Output: The argument.                                                      */
/******************************************************************************/
uint8 MED_Cmd::GetData2(void) const
{
	return (data1);
}





/******************************************************************************/
/*                                                                            */
/* MED_Block class                                                            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "lines" is the number of lines this block have.                    */
/*         "tracks" is the number of tracks this block have.                  */
/*         "pages" is the number of effect commands each line have.           */
/******************************************************************************/
MED_Block::MED_Block(LINE_NUM lines, TRACK_NUM tracks, PAGE_NUM pages)
{
	PAGE_NUM cnt;

	// Remember the arguments
	numLines    = lines;
	numTracks   = tracks;
	numCmdPages = pages;

	grid     = NULL;
	cmdPages = NULL;

	try
	{
		grid = new MED_Note[lines * tracks];
		if (grid == NULL)
			throw PMemoryException();

		cmdPages = new MED_Cmd *[pages];
		if (cmdPages == NULL)
			throw PMemoryException();

		memset(grid, 0, sizeof(MED_Note) * lines * tracks);
		for (cnt = 0; cnt < pages; cnt++)
			cmdPages[cnt] = NULL;

		for (cnt = 0; cnt < pages; cnt++)
		{
			cmdPages[cnt] = new MED_Cmd[lines * tracks];
			if (cmdPages[cnt] == NULL)
				throw PMemoryException();

			memset(cmdPages[cnt], 0, sizeof(MED_Cmd) * lines * tracks);
		}
	}
	catch(...)
	{
		if (cmdPages != NULL)
		{
			for (cnt = 0; cnt < pages; cnt++)
				delete[] cmdPages[cnt];

			delete[] cmdPages;
		}

		delete[] grid;
		throw;
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MED_Block::~MED_Block(void)
{
	PAGE_NUM cnt;

	delete[] grid;

	for (cnt = 0; cnt < numCmdPages; cnt++)
		delete[] cmdPages[cnt];

	delete[] cmdPages;
}



/******************************************************************************/
/* SetName() set the block name.                                              */
/*                                                                            */
/* Input:  "newName" is the name.                                             */
/******************************************************************************/
void MED_Block::SetName(const char *newName)
{
	PCharSet_Amiga charSet;

	name.SetString(newName, &charSet);
}



/******************************************************************************/
/* SetCmdPages() sets the number of pages in the block.                       */
/*                                                                            */
/* Input:  "numOfPages" is the number of pages to set.                        */
/******************************************************************************/
void MED_Block::SetCmdPages(PAGE_NUM numOfPages)
{
	if ((Pages() != numOfPages) && (numOfPages > 0))
	{
		MED_Cmd **newCmdPages = NULL;
		PAGE_NUM cnt;

		try
		{
			newCmdPages = new MED_Cmd *[numOfPages];
			if (newCmdPages == NULL)
				throw PMemoryException();

			for (cnt = 0; cnt < numOfPages; cnt++)
				newCmdPages[cnt] = NULL;

			for (cnt = 0; cnt < min(numOfPages, Pages()); cnt++)
				newCmdPages[cnt] = cmdPages[cnt];

			if (numOfPages > Pages())
			{
				for (cnt = Pages(); cnt < numOfPages; cnt++)
				{
					newCmdPages[cnt] = new MED_Cmd[Lines() * Tracks()];
					if (newCmdPages[cnt] == NULL)
						throw PMemoryException();

					memset(newCmdPages[cnt], 0, sizeof(MED_Cmd) * Lines() * Tracks());
				}
			}
			else
			{
				for (cnt = numOfPages; cnt < Pages(); cnt++)
					delete[] cmdPages[cnt];
			}

			delete[] cmdPages;
			cmdPages    = newCmdPages;
			numCmdPages = numOfPages;
		}
		catch(...)
		{
			if (newCmdPages != NULL)
			{
				for (cnt = 0; cnt < numOfPages; cnt++)
					delete[] newCmdPages[cnt];

				delete[] newCmdPages;
				throw;
			}
		}
	}
}



/******************************************************************************/
/* Note() returns the note at the position given.                             */
/*                                                                            */
/* Input:  "line" is the line you want the note from.                         */
/*         "track" is the track you want the note from.                       */
/*                                                                            */
/* Output: A reference to the note at the position given.                     */
/******************************************************************************/
MED_Note &MED_Block::Note(LINE_NUM line, TRACK_NUM track)
{
	return (grid[line * numTracks + track]);
}



/******************************************************************************/
/* Cmd() returns the effect command at the position given.                    */
/*                                                                            */
/* Input:  "line" is the line you want the command from.                      */
/*         "track" is the track you want the command from.                    */
/*         "page" is the command number you want.                             */
/*                                                                            */
/* Output: A reference to the command at the position given.                  */
/******************************************************************************/
MED_Cmd &MED_Block::Cmd(LINE_NUM line, TRACK_NUM track, PAGE_NUM page)
{
	return (*(cmdPages[page] + line * numTracks + track));
}



/******************************************************************************/
/* Lines() returns the number of lines in the block.                          */
/*                                                                            */
/* Output: The number of lines.                                               */
/******************************************************************************/
LINE_NUM MED_Block::Lines(void) const
{
	return (numLines);
}



/******************************************************************************/
/* Tracks() returns the number of tracks in the block.                        */
/*                                                                            */
/* Output: The number of tracks.                                              */
/******************************************************************************/
TRACK_NUM MED_Block::Tracks(void) const
{
	return (numTracks);
}



/******************************************************************************/
/* Pages() returns the number of pages in the block.                          */
/*                                                                            */
/* Output: The number of pages.                                               */
/******************************************************************************/
PAGE_NUM MED_Block::Pages(void) const
{
	return (numCmdPages);
}
