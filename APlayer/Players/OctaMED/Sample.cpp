/******************************************************************************/
/* Sample Interface.                                                          */
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

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Instr.h"
#include "Song.h"
#include "Sample.h"


/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "octaMED" is a pointer to the core object.                         */
/*         "length" is the length in samples.                                 */
/*         "stereo" indicate if the sample is in stereo or mono.              */
/*         "sixtBit" indicate if the sample is in 16-bit or 8-bit.            */
/******************************************************************************/
Sample::Sample(OctaMED *octaMED, uint32 length, bool stereo, bool sixtBit)
{
	med        = octaMED;

	s_dataLen  = sixtBit ? 2 * sizeof(int8) : sizeof(int8);
	s_length   = length;		// Sample length in samples
	s_isStereo = stereo;
	s_is16Bit  = sixtBit;
	channels   = 0;				// Null samples have a special channel value

	s_data[0]  = NULL;
	s_data[1]  = NULL;

	if (s_length != 0)
	{
		s_data[0] = new int8[ByteLength()];
		channels  = 1;

		if (stereo)
		{
			s_data[1] = new int8[ByteLength()];
			channels  = 2;
		}
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Sample::~Sample(void)
{
	delete[] s_data[1];
	delete[] s_data[0];
}



/******************************************************************************/
/* SetProp() set the sample properties.                                       */
/*                                                                            */
/* Input:  "length" is the length in samples.                                 */
/*         "stereo" indicate if the sample is in stereo or mono.              */
/*         "sixtBit" indicate if the sample is in 16-bit or 8-bit.            */
/******************************************************************************/
void Sample::SetProp(uint32 length, bool stereo, bool sixtBit)
{
	SetStereo(stereo);
	Set16Bit(sixtBit);
	SetLength(length);
}



/******************************************************************************/
/* SetLength() set the sample length.                                         */
/*                                                                            */
/* Input:  "newLen" is the length in samples.                                 */
/*         "clear" indicate if you want to clear the sample data.             */
/******************************************************************************/
void Sample::SetLength(uint32 newLen, bool clear)
{
	uint16 chCnt;
	int32 numChannels = s_isStereo ? 2 : 1;

	for (chCnt = 0; chCnt < numChannels; chCnt++)
	{
		if (newLen != s_length)
		{
			if (newLen > 0)
			{
				ASSERT(s_data[chCnt] == NULL);

				int8 *newData = new int8[s_dataLen * newLen];
				if (newData == NULL)
					throw PMemoryException();

				// Clear trailing if needed
				if (!clear && (newLen > s_length))
					memset(newData + (s_length * s_dataLen), 0, s_dataLen * (newLen - s_length));

				s_data[chCnt] = newData;
			}
			else
			{
				delete[] s_data[chCnt];
				s_data[chCnt] = NULL;
			}
		}

		// Perform clear if required
		if ((newLen != 0) && clear)
			memset(s_data[chCnt], 0, s_dataLen * newLen);
	}

	s_length = newLen;
	if (s_length == 0)
		channels = 0;		// Implies: no data
	else
		channels = numChannels;

	ValidateLoop();
}



/******************************************************************************/
/* SetStereo() set the stereo flag.                                           */
/*                                                                            */
/* Input:  "stereo" indicate if the sample is in stereo or mono.              */
/******************************************************************************/
void Sample::SetStereo(bool stereo)
{
	if (stereo == s_isStereo)
		return;

	if (stereo)
	{
		// TN: No conversion implemented
		ASSERT(false);
//XX
/*		if (s_length != 0)
		{
			s_data[1] = new uint8[s_length * (s_is16Bit ? 2 : 1)];
			if (s_data[1] == NULL)
				throw PMemoryException();

			memcpy(s_data[1], s_data[0], s_length * s_dataLen);
			channels = 2;
		}
*/
		s_isStereo = true;
	}
	else
	{
/*		if (s_length != 0)
		{
			// TN: We don't mix the stereo sample together to a mono samples!
			ASSERT(false);

			delete[] s_data[1];
			s_data[1] = NULL;
			channels  = 1;
		}
*/
		s_isStereo = false;
	}
}



/******************************************************************************/
/* Set16Bit() set the 16-bit flag.                                            */
/*                                                                            */
/* Input:  "sixtBit" indicate if the sample is in 16-bit or 8-bit.            */
/******************************************************************************/
void Sample::Set16Bit(bool sixtBit)
{
	if (s_is16Bit == sixtBit)
		return;

	if (s_length != 0)
	{
		// TN: No conversion implemented
		ASSERT(false);
	}

	s_dataLen = sixtBit ? sizeof(int16) : sizeof(int8);
	s_is16Bit = sixtBit;
}



/******************************************************************************/
/* SetData8() will set a single sample at a given channel.                    */
/*                                                                            */
/* Input:  "pos" is the position to set the sample at.                        */
/*         "ch" is the channel number.                                        */
/*         "smp" is the sample itself.                                        */
/******************************************************************************/
void Sample::SetData8(uint32 pos, uint16 ch, int8 smp)
{
	s_data[ch][pos] = smp;
}



/******************************************************************************/
/* SetData16() will set a single sample at a given channel.                   */
/*                                                                            */
/* Input:  "pos" is the position to set the sample at.                        */
/*         "ch" is the channel number.                                        */
/*         "smp" is the sample itself.                                        */
/******************************************************************************/
void Sample::SetData16(uint32 pos, uint16 ch, int16 smp)
{
	((int16 *)s_data[ch])[pos] = smp;
}



/******************************************************************************/
/* GetLength() returns the length of the sample.                              */
/*                                                                            */
/* Output: The length in samples.                                             */
/******************************************************************************/
uint32 Sample::GetLength(void) const
{
	return (s_length);
}



/******************************************************************************/
/* GetSampleAddress() returns the address to the sample data.                 */
/*                                                                            */
/* Input:  "ch" is the channel number to return the address of.               */
/*                                                                            */
/* Output: The address.                                                       */
/******************************************************************************/
const int8 *Sample::GetSampleAddress(uint16 ch) const
{
	return (s_data[ch]);
}



/******************************************************************************/
/* Is16Bit() tells if the sample is in 16 or 8 bit.                           */
/*                                                                            */
/* Output: True for 16-bit, false for 8-bit.                                  */
/******************************************************************************/
bool Sample::Is16Bit(void) const
{
	return (s_is16Bit);
}



/******************************************************************************/
/* GetInstrument()                                                            */
/******************************************************************************/
Instr *Sample::GetInstrument(void) const
{
	if (med->sg != NULL)
		return (med->sg->Sample2Instrument(this));

	return (NULL);
}



/******************************************************************************/
/* ValidateLoop()                                                             */
/******************************************************************************/
void Sample::ValidateLoop(void)
{
	Instr *instr = GetInstrument();
	if (instr != NULL)
		instr->ValidateLoop();
}



/******************************************************************************/
/* IsSynthSound()                                                             */
/******************************************************************************/
bool Sample::IsSynthSound(void) const
{
	return (false);
}



/******************************************************************************/
/* ByteLength()                                                               */
/******************************************************************************/
int32 Sample::ByteLength(void) const
{
	return (s_length * s_dataLen);
}
