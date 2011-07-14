/******************************************************************************/
/* PSocket header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PSocket_h
#define __PSocket_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSynchronize.h"
#include "PList.h"
#include "ImportExport.h"


/******************************************************************************/
/* Define BONE macros etc.                                                    */
/******************************************************************************/
#ifdef BONE_VERSION

// Bone uses file close
#define closesocket		close

#endif



/******************************************************************************/
/* PSocket class                                                              */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PSocket
{
public:
	enum PProtocol { pTCP_IP };
	enum PCommunication { pStream, pDatagram };

	PSocket(void);
	PSocket(const PSocket &source);
	virtual ~PSocket(void);

	void UseProtocol(PProtocol protocol, PString address, PCommunication type = pStream);
	void SetTimeout(uint32 timeout);

	virtual void ListenForConnection(void);
	virtual void CreateConnection(void);
	virtual void CloseConnection(void);

	virtual void ReceiveData(void *data, uint32 *length);
	virtual void SendData(const void *data, uint32 length);

	void AddWaitObjects(PSync *first, ...);
	void RemoveWaitObjects(PSync *first, ...);
	void SetWaitObjects(const PSocket *source);

	PString GetObjectAddress(bool withPort = false);
	PString GetAddressFromName(PString name);
	PString GetNameFromAddress(PString address);
	PString GetConnectedAddress(bool withPort = false);

	PSocket & operator = (const PSocket &source);

protected:
	union SocketAddr
	{
		sockaddr_in sockIn;		// TCP/IP
	};

	void CopyIt(PSocket &source);

	void InitializeTCPIP(PString address);

	void EventWait(int waitSocket, uint32 waitFlag, uint32 timeout);

	bool clone;
	bool listenInitialized;
	bool connected;
	bool socketClosed;
	bool useSendTo;

	uint32 timeoutValue;

	PProtocol useProtocol;
	PCommunication useType;
	PString useAddress;
	unsigned short usePort;
	int useSocket;

	int sockLen;
	SocketAddr sockAddr;

	int connectedSocket;
	SocketAddr clientSockAddr;

	PList<PSync *> waitObjects;
};



/******************************************************************************/
/* PLineSocket class                                                          */
/******************************************************************************/
class _IMPEXP_PKLIB PLineSocket : public PSocket
{
public:
	PLineSocket(void);
	PLineSocket(const PLineSocket &source);
	virtual ~PLineSocket(void);

	void SetEOLCode(PString eol);
	void SetMaxLineLength(uint32 length);

	PString GetEOLCode(void) const;
	int32 GetMaxLineLength(void) const;

	virtual void ReceiveData(void *data, uint32 *length);

	virtual PString ReceiveLine(PCharacterSet *characterSet = NULL);
	virtual void SendLine(PString line, PCharacterSet *characterSet = NULL);

	PLineSocket & operator = (const PLineSocket &source);

protected:
	void CopyIt(const PLineSocket &source);

	PString eolCode;
	int32 maxBufferSize;

	uint8 *buffer;
	int32 bufferLen;
	int32 bufferFilled;
};



/******************************************************************************/
/* PProxySocket class                                                         */
/******************************************************************************/
class _IMPEXP_PKLIB PProxySocket : public PLineSocket
{
public:
	PProxySocket(void);
	PProxySocket(const PProxySocket &source);
	virtual ~PProxySocket(void);

	enum PProxyType { pHTTP };

	void EnableProxy(bool enable);
	void SetProxyServer(PProxyType type, PString address);

	bool IsEnabled(void) const;
	PProxyType GetProxyType(void) const;
	PString GetProxyAddress(void) const;

	virtual void CreateConnection(void);
	virtual void CloseConnection(void);

	PProxySocket & operator = (const PProxySocket &source);

protected:
	void CopyIt(const PProxySocket &source);

	bool proxyEnabled;
	PProxyType proxyType;
	PString proxyAddress;

	PLineSocket connectionSocket;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
