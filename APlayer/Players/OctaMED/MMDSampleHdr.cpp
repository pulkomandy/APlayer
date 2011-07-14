/******************************************************************************/
/* MMDSampleHdr Interface.                                                    */
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
#include "PResource.h"
#include "PFile.h"

// Player headers
#include "MEDTypes.h"
#include "Sample.h"
#include "OctaMED.h"
#include "MMDSampleHdr.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MMDSampleHdr::MMDSampleHdr(OctaMED *octaMED, PFile *f, PString &errorStr, PResource *res)
{
	med           = octaMED;

	uint32 length = f->Read_B_UINT32();
	numFrames     = length;
	skipThis      = false;
	type          = f->Read_B_UINT16();

	if (type >= 0)
	{
		if (((type & 0x0f) >= 1) && ((type & 0x0f) <= 6))
		{
			// Multi octave
			printf("multi octave\n");
		}
		else
		{
			if ((type & 0x0f) > 7)
			{
				// Unknown sample
				errorStr.LoadString(res, IDS_MED_ERR_UNKNOWN_SAMPLE);
				throw PUserException();
			}
		}

		if (type & MMD_INSTR_16BIT)
		{
			numFrames /= 2;
			sixtBit    = true;
		}
		else
			sixtBit = false;

		if (type & MMD_INSTR_STEREO)
			stereo = true;
		else
			stereo = false;

		if (type & MMD_INSTR_PACK)
		{
			errorStr.LoadString(res, IDS_MED_ERR_PACKED_SAMPLES);
			throw PUserException();
		}
		else
		{
			packType = 0;
			subType  = 0;
		}
	}
	else
	{
		if (type < -2)
		{
			// Unknown instrument type
			errorStr.LoadString(res, IDS_MED_ERR_UNKNOWN_INSTRUMENT);
			throw PUserException();
		}
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MMDSampleHdr::~MMDSampleHdr(void)
{
}



/******************************************************************************/
/* AllocSample() allocate a sample object with the information in this        */
/*      header.                                                               */
/*                                                                            */
/* Output: Pointer to a sample object.                                        */
/******************************************************************************/
Sample *MMDSampleHdr::AllocSample(void) const
{
	return (new Sample(med, numFrames, stereo, sixtBit));
}



/******************************************************************************/
/* ReadSampleData() read the sample data into the sample object given.        */
/*                                                                            */
/* Input:  "f" is a pointer to the file to read from.                         */
/*         "dest" is a pointer to the sample object to store the data in.     */
/******************************************************************************/
void MMDSampleHdr::ReadSampleData(PFile *f, Sample *dest)
{
	uint16 chCnt;

	// Make sure that the dimension for receiving data are correct
	dest->SetProp(numFrames, stereo, sixtBit);

	// Unknown compression
	if (skipThis)
		return;

	switch (packType)
	{
		//
		// No packing
		//
		case 0:
		{
			for (chCnt = 0; chCnt < (stereo ? 2 : 1); chCnt++)
			{
				uint32 cnt2;

				if (sixtBit)
				{
					if (type & MMD_INSTR_DELTACODE)
					{
						int16 prev = f->Read_B_UINT16();
						dest->SetData16(0, chCnt, prev);

						for (cnt2 = 1; cnt2 < numFrames; cnt2++)
							dest->SetData16(cnt2, chCnt, prev += f->Read_B_UINT16());
					}
					else
					{
						for (cnt2 = 0; cnt2 < numFrames; cnt2++)
							dest->SetData16(cnt2, chCnt, f->Read_B_UINT16());
					}
				}
				else
				{
					if (type & MMD_INSTR_DELTACODE)
					{
						int8 prev = f->Read_UINT8();
						dest->SetData8(0, chCnt, prev);

						for (cnt2 = 1; cnt2 < numFrames; cnt2++)
							dest->SetData8(cnt2, chCnt, prev += f->Read_UINT8());
					}
					else
					{
						for (cnt2 = 0; cnt2 < numFrames; cnt2++)
							dest->SetData8(cnt2, chCnt, f->Read_UINT8());
					}
				}
			}
			break;
		}

		default:
		{
			// Should never be here!
			ASSERT(false);
			break;
		}
	}
}



/******************************************************************************/
/* IsSample() checks to see if the sample is a real sample.                   */
/*                                                                            */
/* Output: True if it's a sample, false if not.                               */
/******************************************************************************/
bool MMDSampleHdr::IsSample(void) const
{
	return (type >= 0);
}



/******************************************************************************/
/* IsSynth() checks to see if the sample is a synth sample.                   */
/*                                                                            */
/* Output: True if it's a synth sample, false if not.                         */
/******************************************************************************/
bool MMDSampleHdr::IsSynth(void) const
{
	return (type == -1);
}



/******************************************************************************/
/* IsHybrid() checks to see if the sample is a hybrid sample.                 */
/*                                                                            */
/* Output: True if it's a hybrid sample, false if not.                        */
/******************************************************************************/
bool MMDSampleHdr::IsHybrid(void) const
{
	return (type == -2);
}



/******************************************************************************/
/* IsStereo() checks to see if the sample is a stereo sample.                 */
/*                                                                            */
/* Output: True if it's a stereo sample, false if not.                        */
/******************************************************************************/
bool MMDSampleHdr::IsStereo(void) const
{
	return (stereo);
}
