/******************************************************************************/
/* Global functions implementation file.                                      */
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
#include "PResource.h"
#include "PSynchronize.h"
#include "PSystem.h"


/******************************************************************************/
/* Initialize static member variables.                                        */
/******************************************************************************/
PResource PSystem::polyResource("lib/PolyKit.so");
bool PSystem::rndInit = false;



/******************************************************************************/
/* PlayBeep() will play the default system beep sound.                        */
/******************************************************************************/
void PSystem::PlayBeep(void)
{
	beep();
}



/******************************************************************************/
/* Sleep() will wait the number of milliseconds given.                        */
/*                                                                            */
/* Input:  "milliSeconds" is the time to wait.                                */
/******************************************************************************/
void PSystem::Sleep(uint32 milliSeconds)
{
	snooze(milliSeconds * 1000);
}



/******************************************************************************/
/* Random() will return a random number between 0 and max.                    */
/*                                                                            */
/* Input:  "max" is the max number you want.                                  */
/*                                                                            */
/* Output: Is a random number.                                                */
/******************************************************************************/
uint32 PSystem::Random(uint32 max)
{
	// If max is 0, well, we then return 0 as the random number
	if (max == 0)
		return (0);

	if (!rndInit)
	{
		// Initialize random generator
		srand(real_time_clock_usecs());
		rndInit = true;
	}

	// This is the recommended method for getting a number between
	// 0 and max. The high bits are more random than the low bits.
    return ((uint32) ((double)(max + 1) * rand() / (RAND_MAX + 1.0)));
}



/******************************************************************************/
/* ConvertOSError() will convert an OS specific error code to a PolyKit error */
/*      code.                                                                 */
/*                                                                            */
/* Input:  "osError" is the OS error code.                                    */
/*                                                                            */
/* Output: Is the PolyKit error code.                                         */
/******************************************************************************/
uint32 PSystem::ConvertOSError(uint32 osError)
{
	uint32 polyErr = P_ERR_ANY;

	switch (osError)
	{
		//
		// General errors
		//
		case B_OK:
		{
			polyErr = P_OK;
			break;
		}

		case B_ERROR:
		{
			polyErr = P_ERR_ANY;
			break;
		}

		case B_BAD_VALUE:
		case B_BAD_THREAD_ID:
		case B_BAD_TEAM_ID:
		case B_BAD_ADDRESS:
		case EAFNOSUPPORT:
		case EPROTOTYPE:
		case EPROTONOSUPPORT:
		case ENOPROTOOPT:
		{
			polyErr = P_GEN_ERR_BAD_ARGUMENT;
			break;
		}

		case B_NO_MEMORY:
		{
			polyErr = P_GEN_ERR_NO_MEMORY;
			break;
		}

		case B_NO_MORE_FDS:
		case B_NO_MORE_THREADS:
		{
			polyErr = P_GEN_ERR_NO_RESOURCES;
			break;
		}

		case B_NO_INIT:
		{
			polyErr = P_GEN_ERR_NO_INIT;
			break;
		}

		case B_NOT_ALLOWED:
		case B_BAD_THREAD_STATE:
		{
			polyErr = P_GEN_ERR_CANT_PERFORM;
			break;
		}

		case B_BAD_IMAGE_ID:
		{
			polyErr = P_GEN_ERR_BAD_IDENTIFIER;
			break;
		}

		case B_BAD_INDEX:
		{
			polyErr = P_GEN_ERR_BAD_INDEX;
			break;
		}

		case B_TIMED_OUT:
		{
			polyErr = P_GEN_ERR_TIMEOUT;
			break;
		}

		//
		// File errors
		//
		case B_FILE_ERROR:
		{
			polyErr = P_FILE_ERR_FILE;
			break;
		}

		case B_NAME_TOO_LONG:
		{
			polyErr = P_FILE_ERR_NAME_TOO_LONG;
			break;
		}

		case B_ENTRY_NOT_FOUND:
		{
			polyErr = P_FILE_ERR_ENTRY_NOT_FOUND;
			break;
		}

		case B_FILE_EXISTS:
		{
			polyErr = P_FILE_ERR_ENTRY_EXISTS;
			break;
		}

		case B_DEVICE_FULL:
		{
			polyErr = P_FILE_ERR_DISK_FULL;
			break;
		}

		case B_NOT_A_DIRECTORY:
		{
			polyErr = P_FILE_ERR_NOT_A_DIRECTORY;
			break;
		}

		case B_LINK_LIMIT:
		{
			polyErr = P_FILE_ERR_LINK;
			break;
		}

		case B_BUSY:
		{
			polyErr = P_FILE_ERR_LOCKED;
			break;
		}

		//
		// Security errors
		//
		case B_PERMISSION_DENIED:
		{
			polyErr = P_SEC_ERR_ACCESS_DENIED;
			break;
		}

		//
		// Sound errors
		//
		case B_STREAM_NOT_FOUND:
		case B_SERVER_NOT_FOUND:
		case 0x80004100:
		{
			polyErr = P_SOUND_ERR_NO_SOFTWARE;
			break;
		}

		case B_RESOURCE_NOT_FOUND:
		case B_RESOURCE_UNAVAILABLE:
		case 0x80004101:
		{
			polyErr = P_SOUND_ERR_NO_SOUNDCARD;
			break;
		}

		case B_BAD_SUBSCRIBER:
		case B_SUBSCRIBER_NOT_ENTERED:
		case B_BUFFER_NOT_AVAILABLE:
		{
			polyErr = P_SOUND_ERR_UNKNOWN;
			break;
		}

		//
		// Network errors
		//
		case EINTR:
		{
			polyErr = P_NETWORK_ERR_INTERRUPTED;
			break;
		}

		case EWOULDBLOCK:
		{
			polyErr = P_NETWORK_ERR_WOULD_BLOCK;
			break;
		}

		case ECONNRESET:
		case ENOTCONN:
		{
			polyErr = P_NETWORK_ERR_SOCKET_CLOSED;
			break;
		}

		case EADDRINUSE:
		case ENETUNREACH:
		{
			polyErr = P_NETWORK_ERR_INTERFACE_BUSY;
			break;
		}

		case EISCONN:
		{
			polyErr = P_NETWORK_ERR_ALREADY_CONNECTED;
			break;
		}

		case ECONNREFUSED:
		{
			polyErr = P_NETWORK_ERR_CONNECTION_REJECTED;
			break;
		}

#ifdef PDEBUG
		default:
		{
			printf("Error %ld: %s\n", osError, strerror(osError));
			ASSERT(false);
			break;
		}
#endif
	}

	return (polyErr);
}



/******************************************************************************/
/* GetErrorString() will convert an PolyKit error code to a readable string.  */
/*                                                                            */
/* Input:  "error" is the PolyKit error code.                                 */
/*                                                                            */
/* Output: Is the readable string.                                            */
/******************************************************************************/
PString PSystem::GetErrorString(uint32 error)
{
	PString errStr;

	// Get the string
	errStr.LoadString(&polyResource, error);

	return (errStr);
}



/******************************************************************************/
/* GetOSVersion() will return the version of the operative system.            */
/*                                                                            */
/* Output: Is the version of the operative system.                            */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PSystem::POperativeSystem PSystem::GetOSVersion(void)
{
#if __p_os == __p_beos

#ifdef __POWERPC__

	return (pBeOS_ppc);

#else

	return (pBeOS_x86);

#endif

#endif
}



/******************************************************************************/
/* GetSystemLocaleString() retrieves the locale string from the system and    */
/*      return it.                                                            */
/*                                                                            */
/* Input:  "localeNum" is the locale string number to retrieve.               */
/*                                                                            */
/* Output: The locale string.                                                 */
/*                                                                            */
/* Except: PSystemException.                                                  */
/******************************************************************************/
PString PSystem::GetSystemLocaleString(PLocaleString localeNum)
{
	PString localeStr;
	int32 resID;

	// Find the resource ID to use
	resID = (int32)localeNum + 10000;

	// Get the string and return it
	localeStr.LoadString(&polyResource, resID);
	if (localeStr.IsEmpty())
		throw PSystemException(P_GEN_ERR_BAD_ARGUMENT);

	return (localeStr);
}
