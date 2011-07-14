/******************************************************************************/
/* APlayer Reverb Agent class.                                                */
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
#include "PResource.h"
#include "PSettings.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "Layout.h"

// Agent headers
#include "ReverbAgent.h"
#include "ReverbView.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.01f



/******************************************************************************/
/* Macros                                                                     */
/*                                                                            */
/* Constant definitions                                                       */
/* ====================                                                       */
/*                                                                            */
/* REVERBERATION     Controls the duration of the reverb. Larger values       */
/*                   represent a shorter reverb loop. Smaller values extend   */
/*                   the reverb but can result in more of an echo-ish sound.  */
/******************************************************************************/
#define REVERBERATION			110000L



/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *reverbSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ReverbAgent::ReverbAgent(APGlobalData *global, PString fileName) : APAddOnAgent(global)
{
	// Fill out the version variable
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize reverb variables
	allocated = false;

	rvBufL1   = NULL;
	rvBufL2   = NULL;
	rvBufL3   = NULL;
	rvBufL4   = NULL;
	rvBufL5   = NULL;
	rvBufL6   = NULL;
	rvBufL7   = NULL;
	rvBufL8   = NULL;

	rvBufR1   = NULL;
	rvBufR2   = NULL;
	rvBufR3   = NULL;
	rvBufR4   = NULL;
	rvBufR5   = NULL;
	rvBufR6   = NULL;
	rvBufR7   = NULL;
	rvBufR8   = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ReverbAgent::~ReverbAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float ReverbAgent::GetVersion(void)
{
	return (AgentVersion);
}



/******************************************************************************/
/* GetConfigInfo() will return a pointer to a config structure.               */
/*                                                                            */
/* Output: Is a pointer to a config structure.                                */
/******************************************************************************/
const APConfigInfo *ReverbAgent::GetConfigInfo(void)
{
	// Load the settings if not already loaded
	LoadSettings();

	// Create background view
	cfgInfo.view     = new ReverbView(globalData, res);
	cfgInfo.settings = reverbSettings;
	cfgInfo.fileName = "Reverb.ini";

	return (&cfgInfo);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 ReverbAgent::GetSupportFlags(int32 index)
{
	return (apaDSP);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString ReverbAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_REVERB_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString ReverbAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_REVERB_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* InitAgent() will initialize the agent.                                     */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool ReverbAgent::InitAgent(int32 index)
{
	// Load the settings if not already loaded
	LoadSettings();

	return (true);
}



/******************************************************************************/
/* EndAgent() will clean up the agent.                                        */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/******************************************************************************/
void ReverbAgent::EndAgent(int32 index)
{
	// Free any allocated buffers
	FreeReverbBuffers();
}



/******************************************************************************/
/* Run() will run one command in the agent.                                   */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*         "command" is the command to run.                                   */
/*         "args" is a pointer to a structure containing the arguments. The   */
/*         structure is different for each command.                           */
/*                                                                            */
/* Output: The result from the command.                                       */
/*                                                                            */
/* Except: PSoundException.                                                   */
/******************************************************************************/
ap_result ReverbAgent::Run(int32 index, uint32 command, void *args)
{
	switch (command)
	{
		// Do the DSP stuff
		case APPA_DSP:
		{
			DSP((APAgent_DSP *)args);
			return (AP_OK);
		}

		// Stop the agent
		case APPA_STOP:
		{
			Stop();
			return (AP_OK);
		}
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* LoadSettings() will load the settings if not already loaded.               */
/******************************************************************************/
void ReverbAgent::LoadSettings(void)
{
	// Check to see if we need to allocate the settings object
	if (reverbSettings == NULL)
	{
		// Now allocate the settings
		reverbSettings = new PSettings();
		if (reverbSettings == NULL)
			throw PMemoryException();

		// Load the settings into the memory
		try
		{
			reverbSettings->LoadFile("Reverb.ini", "Polycode", "APlayer");
		}
		catch(PFileException e)
		{
			// Well, the file does probably not exists, so ignore the error
			;
		}

		// Fix the settings
		FixSettings();
	}
}



/******************************************************************************/
/* FixSettings() puts all the default setting values into the settings object.*/
/******************************************************************************/
void ReverbAgent::FixSettings(void)
{
	if (!reverbSettings->EntryExist("General", "Reverb"))
		reverbSettings->WriteIntEntryValue("General", "Reverb", 0);
}



/******************************************************************************/
/* DSP() do the reverb stuff.                                                 */
/*                                                                            */
/* Input:  "dspInfo" is a pointer to the information needed to do the DSP.    */
/******************************************************************************/
void ReverbAgent::DSP(APAgent_DSP *dspInfo)
{
	uint8 rev;

	// Get the reverb value
	rev = reverbSettings->GetIntEntryValue("General", "Reverb", 0);

	// Check to see if we need to make reverb
	if (rev != 0)
	{
		// Check for out of bounds
		if (rev > 15)
			rev = 15;

		// Allocate the reverb buffers if not already allocated
		try
		{
			AllocReverbBuffers(dspInfo->frequency);
		}
		catch(PMemoryException e)
		{
			return;
		}

		if (dspInfo->stereo)
			MixReverbStereo(dspInfo->buffer, dspInfo->todo >> 1, rev);
		else
			MixReverbMono(dspInfo->buffer, dspInfo->todo, rev);
	}
	else
		FreeReverbBuffers();
}



/******************************************************************************/
/* Stop() stop the reverb and free any resources used.                        */
/******************************************************************************/
void ReverbAgent::Stop(void)
{
	// Free any allocated buffers
	FreeReverbBuffers();
}



/******************************************************************************/
/* AllocReverbBuffers() allocate the buffers needed to do the reverb thing.   */
/*                                                                            */
/* Input:  "mixerFreq" is the mixer frequency.                                */
/******************************************************************************/
void ReverbAgent::AllocReverbBuffers(uint32 mixerFreq)
{
	if (!allocated)
	{
		// Initialize reverb
		rvc1 = (5000L * mixerFreq) / REVERBERATION;
		rvc2 = (5078L * mixerFreq) / REVERBERATION;
		rvc3 = (5313L * mixerFreq) / REVERBERATION;
		rvc4 = (5703L * mixerFreq) / REVERBERATION;
		rvc5 = (6250L * mixerFreq) / REVERBERATION;
		rvc6 = (6953L * mixerFreq) / REVERBERATION;
		rvc7 = (7813L * mixerFreq) / REVERBERATION;
		rvc8 = (8828L * mixerFreq) / REVERBERATION;

		rvrIndex = 0;

		rvBufL1 = new int32[rvc1 + 1];
		if (rvBufL1 == NULL)
			throw PMemoryException();

		rvBufL2 = new int32[rvc2 + 1];
		if (rvBufL2 == NULL)
			throw PMemoryException();

		rvBufL3 = new int32[rvc3 + 1];
		if (rvBufL3 == NULL)
			throw PMemoryException();

		rvBufL4 = new int32[rvc4 + 1];
		if (rvBufL4 == NULL)
			throw PMemoryException();

		rvBufL5 = new int32[rvc5 + 1];
		if (rvBufL5 == NULL)
			throw PMemoryException();

		rvBufL6 = new int32[rvc6 + 1];
		if (rvBufL6 == NULL)
			throw PMemoryException();

		rvBufL7 = new int32[rvc7 + 1];
		if (rvBufL7 == NULL)
			throw PMemoryException();

		rvBufL8 = new int32[rvc8 + 1];
		if (rvBufL8 == NULL)
			throw PMemoryException();

		memset(rvBufL1, 0, (rvc1 + 1) * sizeof(int32));
		memset(rvBufL2, 0, (rvc2 + 1) * sizeof(int32));
		memset(rvBufL3, 0, (rvc3 + 1) * sizeof(int32));
		memset(rvBufL4, 0, (rvc4 + 1) * sizeof(int32));
		memset(rvBufL5, 0, (rvc5 + 1) * sizeof(int32));
		memset(rvBufL6, 0, (rvc6 + 1) * sizeof(int32));
		memset(rvBufL7, 0, (rvc7 + 1) * sizeof(int32));
		memset(rvBufL8, 0, (rvc8 + 1) * sizeof(int32));

		rvBufR1 = new int32[rvc1 + 1];
		if (rvBufR1 == NULL)
			throw PMemoryException();

		rvBufR2 = new int32[rvc2 + 1];
		if (rvBufR2 == NULL)
			throw PMemoryException();

		rvBufR3 = new int32[rvc3 + 1];
		if (rvBufR3 == NULL)
			throw PMemoryException();

		rvBufR4 = new int32[rvc4 + 1];
		if (rvBufR4 == NULL)
			throw PMemoryException();

		rvBufR5 = new int32[rvc5 + 1];
		if (rvBufR5 == NULL)
			throw PMemoryException();

		rvBufR6 = new int32[rvc6 + 1];
		if (rvBufR6 == NULL)
			throw PMemoryException();

		rvBufR7 = new int32[rvc7 + 1];
		if (rvBufR7 == NULL)
			throw PMemoryException();

		rvBufR8 = new int32[rvc8 + 1];
		if (rvBufR8 == NULL)
			throw PMemoryException();

		memset(rvBufR1, 0, (rvc1 + 1) * sizeof(int32));
		memset(rvBufR2, 0, (rvc2 + 1) * sizeof(int32));
		memset(rvBufR3, 0, (rvc3 + 1) * sizeof(int32));
		memset(rvBufR4, 0, (rvc4 + 1) * sizeof(int32));
		memset(rvBufR5, 0, (rvc5 + 1) * sizeof(int32));
		memset(rvBufR6, 0, (rvc6 + 1) * sizeof(int32));
		memset(rvBufR7, 0, (rvc7 + 1) * sizeof(int32));
		memset(rvBufR8, 0, (rvc8 + 1) * sizeof(int32));

		allocated = true;
	}
}



/******************************************************************************/
/* FreeReverbBuffers() frees the memory used for the reverb buffers.          */
/******************************************************************************/
void ReverbAgent::FreeReverbBuffers(void)
{
	if (allocated)
	{
		// Deallocate the reverb buffers
		delete[] rvBufR8;
		delete[] rvBufR7;
		delete[] rvBufR6;
		delete[] rvBufR5;
		delete[] rvBufR4;
		delete[] rvBufR3;
		delete[] rvBufR2;
		delete[] rvBufR1;

		rvBufR8 = NULL;
		rvBufR7 = NULL;
		rvBufR6 = NULL;
		rvBufR5 = NULL;
		rvBufR4 = NULL;
		rvBufR3 = NULL;
		rvBufR2 = NULL;
		rvBufR1 = NULL;

		delete[] rvBufL8;
		delete[] rvBufL7;
		delete[] rvBufL6;
		delete[] rvBufL5;
		delete[] rvBufL4;
		delete[] rvBufL3;
		delete[] rvBufL2;
		delete[] rvBufL1;

		rvBufL8 = NULL;
		rvBufL7 = NULL;
		rvBufL6 = NULL;
		rvBufL5 = NULL;
		rvBufL4 = NULL;
		rvBufL3 = NULL;
		rvBufL2 = NULL;
		rvBufL1 = NULL;

		allocated = false;
	}
}



/******************************************************************************/
/* MixReverbMono() creates reverb in a mono buffer.                           */
/*                                                                            */
/* Input:  "source" is a pointer to the buffer to do the reverb on.           */
/*         "todo" is the number of samples the buffer is.                     */
/*         "rev" is the reverb value.                                         */
/******************************************************************************/
#define COMPUTE_LOC(n)		loc##n = rvrIndex % rvc##n
#define COMPUTE_LECHO(n)	rvBufL##n [loc##n] = speedUp + ((reverbPct * rvBufL##n [loc##n]) >> 7)
#define COMPUTE_RECHO(n)	rvBufR##n [loc##n] = speedUp + ((reverbPct * rvBufR##n [loc##n]) >> 7)

void ReverbAgent::MixReverbMono(int32 *source, int32 count, uint8 rev)
{
	uint32 speedUp;
	int32 reverbPct;
	uint32 loc1, loc2, loc3, loc4;
	uint32 loc5, loc6, loc7, loc8;

	reverbPct = 58 + (rev << 2);

	COMPUTE_LOC(1);
	COMPUTE_LOC(2);
	COMPUTE_LOC(3);
	COMPUTE_LOC(4);
	COMPUTE_LOC(5);
	COMPUTE_LOC(6);
	COMPUTE_LOC(7);
	COMPUTE_LOC(8);

	while (count--)
	{
		// Compute the left channel echo buffers
		speedUp = *source >> 3;

		COMPUTE_LECHO(1);
		COMPUTE_LECHO(2);
		COMPUTE_LECHO(3);
		COMPUTE_LECHO(4);
		COMPUTE_LECHO(5);
		COMPUTE_LECHO(6);
		COMPUTE_LECHO(7);
		COMPUTE_LECHO(8);

		// Prepare to compute actual finalized data
		rvrIndex++;

		COMPUTE_LOC(1);
		COMPUTE_LOC(2);
		COMPUTE_LOC(3);
		COMPUTE_LOC(4);
		COMPUTE_LOC(5);
		COMPUTE_LOC(6);
		COMPUTE_LOC(7);
		COMPUTE_LOC(8);

		// Left channel
		*source++ += rvBufL1[loc1] - rvBufL2[loc2] + rvBufL3[loc3] - rvBufL4[loc4] +
					 rvBufL5[loc5] - rvBufL6[loc6] + rvBufL7[loc7] - rvBufL8[loc8];
	}
}



/******************************************************************************/
/* MixReverbStereo() creates reverb in a stereo buffer.                       */
/*                                                                            */
/* Input:  "source" is a pointer to the buffer to do the reverb on.           */
/*         "todo" is the number of samples the buffer is.                     */
/*         "rev" is the reverb value.                                         */
/******************************************************************************/
void ReverbAgent::MixReverbStereo(int32 *source, int32 count, uint8 rev)
{
	uint32 speedUp;
	int32 reverbPct;
	uint32 loc1, loc2, loc3, loc4;
	uint32 loc5, loc6, loc7, loc8;

	reverbPct = 63 + (rev << 2);

	COMPUTE_LOC(1);
	COMPUTE_LOC(2);
	COMPUTE_LOC(3);
	COMPUTE_LOC(4);
	COMPUTE_LOC(5);
	COMPUTE_LOC(6);
	COMPUTE_LOC(7);
	COMPUTE_LOC(8);

	while (count--)
	{
		// Compute the left channel echo buffers
		speedUp = *source >> 3;

		COMPUTE_LECHO(1);
		COMPUTE_LECHO(2);
		COMPUTE_LECHO(3);
		COMPUTE_LECHO(4);
		COMPUTE_LECHO(5);
		COMPUTE_LECHO(6);
		COMPUTE_LECHO(7);
		COMPUTE_LECHO(8);

		// Compute the right channel echo buffers
		speedUp = source[1] >> 3;

		COMPUTE_RECHO(1);
		COMPUTE_RECHO(2);
		COMPUTE_RECHO(3);
		COMPUTE_RECHO(4);
		COMPUTE_RECHO(5);
		COMPUTE_RECHO(6);
		COMPUTE_RECHO(7);
		COMPUTE_RECHO(8);

		// Prepare to compute actual finalized data
		rvrIndex++;

		COMPUTE_LOC(1);
		COMPUTE_LOC(2);
		COMPUTE_LOC(3);
		COMPUTE_LOC(4);
		COMPUTE_LOC(5);
		COMPUTE_LOC(6);
		COMPUTE_LOC(7);
		COMPUTE_LOC(8);

		// Left channel then right channel
		*source++ += rvBufL1[loc1] - rvBufL2[loc2] + rvBufL3[loc3] - rvBufL4[loc4] +
					 rvBufL5[loc5] - rvBufL6[loc6] + rvBufL7[loc7] - rvBufL8[loc8];
		*source++ += rvBufR1[loc1] - rvBufR2[loc2] + rvBufR3[loc3] - rvBufR4[loc4] +
					 rvBufR5[loc5] - rvBufR6[loc6] + rvBufR7[loc7] - rvBufR8[loc8];
	}
}
