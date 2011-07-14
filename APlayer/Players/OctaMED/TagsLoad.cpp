/******************************************************************************/
/* TagsLoad Interface.                                                        */
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
#include "PFile.h"

// Player headers
#include "MEDTypes.h"
#include "TagsLoad.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
TagsLoad::TagsLoad(PFile *f)
{
	int64 startPos;
	uint32 i;

	// Remember arguments
	file = f;

	// Get the current file position
	startPos = f->GetPosition();

	// Count the number of tags contained in the file
	numTags = 0;
	while (f->Read_B_UINT32() != MMDTAG_END)
	{
		numTags++;
		f->Read_B_UINT32();		// Skip tag data
	}

	// Seek back and allocate memory to hold the tags
	f->Seek(startPos, PFile::pSeekBegin);
	tags = new uint32[numTags * 2];
	if (tags == NULL)
		throw PMemoryException();

	tagChecked = new bool[numTags];
	if (tagChecked == NULL)
	{
		delete[] tags;
		throw PMemoryException();
	}

	for (i = 0; i < numTags; i++)
		tagChecked[i] = false;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
TagsLoad::~TagsLoad(void)
{
	delete[] tags;
	delete[] tagChecked;
}



/******************************************************************************/
/* TagExists() checks to see if the tag given exists in the array.            */
/*                                                                            */
/* Input:  "tagNum" is the tag to search for.                                 */
/*                                                                            */
/* Output: True if it exists, false if not.                                   */
/******************************************************************************/
bool TagsLoad::TagExists(uint32 tagNum)
{
	uint32 cnt;

	for (cnt = 0; cnt < numTags; cnt++)
	{
		if (tags[cnt * 2] == tagNum)
		{
			tagChecked[cnt] = true;
			return (true);
		}
	}

	return (false);
}



/******************************************************************************/
/* TagVal() returns the tag value of the tag given.                           */
/*                                                                            */
/* Input:  "tagNum" is the tag to search for.                                 */
/*                                                                            */
/* Output: Is the tag value or 0 if it couldn't be found.                     */
/******************************************************************************/
uint32 TagsLoad::TagVal(uint32 tagNum)
{
	uint32 cnt;

	for (cnt = 0; cnt < numTags; cnt++)
	{
		if (tags[cnt * 2] == tagNum)
		{
			tagChecked[cnt] = true;
			return (tags[cnt * 2 + 1]);
		}
	}

	return (0);
}



/******************************************************************************/
/* TagVal() returns the tag value of the tag given.                           */
/*                                                                            */
/* Input:  "tagNum" is the tag to search for.                                 */
/*         "defVal" is the value to return it the tag couldn't be found.      */
/*                                                                            */
/* Output: Is the tag value or the default value.                             */
/******************************************************************************/
uint32 TagsLoad::TagVal(uint32 tagNum, uint32 defVal)
{
	uint32 cnt;

	for (cnt = 0; cnt < numTags; cnt++)
	{
		if (tags[cnt * 2] == tagNum)
		{
			tagChecked[cnt] = true;
			return (tags[cnt * 2 + 1]);
		}
	}

	return (defVal);
}



/******************************************************************************/
/* SeekToTag() will set the file pointer to the tag given value.              */
/*                                                                            */
/* Input:  "tagNum" is the tag to search for.                                 */
/******************************************************************************/
void TagsLoad::SeekToTag(uint32 tagNum)
{
	uint32 cnt;

	for (cnt = 0; cnt < numTags; cnt++)
	{
		if (tags[cnt * 2] == tagNum)
		{
			tagChecked[cnt] = true;
			file->Seek(tags[cnt * 2 + 1], PFile::pSeekBegin);
			break;
		}
	}
}
