/******************************************************************************/
/* APServerCommunication header file.                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APServerCommunication_h
#define __APServerCommunication_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PList.h"

// APlayerKit headers
#include "Import_Export.h"
#include "APList.h"


/******************************************************************************/
/* Define commands send to/from the APlayer server. Only used for intern      */
/* purpose.                                                                   */
/******************************************************************************/
#define APSERVER_MSG_CONNECT		'_con'
#define APSERVER_MSG_DISCONNECT		'_dis'
#define APSERVER_MSG_DATA			'_dat'
#define APSERVER_MSG_OK				'_ok_'
#define APSERVER_MSG_ERROR			'_err'



/******************************************************************************/
/* Error numbers that can be returned from a server command.                  */
/******************************************************************************/
#define APCMDERR_ARGLIST			1
#define APCMDERR_INVALID_HANDLE		2
#define APCMDERR_FILE				3
#define APCMDERR_UNKNOWN_MODULE		4
#define APCMDERR_LOAD_MODULE		5
#define APCMDERR_PLAYER_INIT		6



/******************************************************************************/
/* Define data function prototype which is used when data is sent from the    */
/* server to a client add-on.                                                 */
/******************************************************************************/
typedef void (*ClientDataFunc)(void *userData, PString command, PList<PString> &arguments);



/******************************************************************************/
/* APClient class                                                             */
/******************************************************************************/
class APClient : public BLooper
{
public:
	APClient(PString clientName);
	virtual ~APClient(void);

	void SetDataFunction(ClientDataFunc dataFunc, void *userData);

protected:
	virtual void MessageReceived(BMessage *message);

	ClientDataFunc clientDataFunc;
	void *clientUserData;
};



/******************************************************************************/
/* APServerCommunication class                                                */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_APKIT APServerCommunication
{
public:
	APServerCommunication(void);
	virtual ~APServerCommunication(void);

	void *ConnectToServer(PString clientName, ClientDataFunc dataFunc, void *userData);
	void DisconnectFromServer(void *handle);
	PString SendCommand(void *handle, PString command);

	static PString AddArgument(PString command, PString arg);

	// Functions under this line are only called by
	// the APlayer server. Do not call these functions
	// from an add-on!
	void SetServerLooper(BLooper *looper);

protected:
	BLooper *serverLooper;
	APList<APClient *> clients;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
