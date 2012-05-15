/*
 * Copyright (c) 2003 Rozhuk Ivan <rozhuk.im@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */



#if !defined(AFX_SOCKET_FUNCTIONS__H__INCLUDED_)
#define AFX_SOCKET_FUNCTIONS__H__INCLUDED_

#if(_WIN32_WINNT >= 0x0501)
static LPFN_CONNECTEX ConnectEx=NULL;
static LPFN_DISCONNECTEX DisconnectEx=NULL;
static LPFN_TRANSMITPACKETS TransmitPackets=NULL;
#endif


#ifndef NTOHS

__inline u_short USHORT_FLIP(u_short usIn)
{	
return(((usIn<<8) | (usIn>>8)));
}
#define NTOHS(in)   USHORT_FLIP(in)
#define HTONS(in)   USHORT_FLIP(in)

#endif



#ifndef NTOHL

__inline u_long ULONG_FLIP(u_long ulIn)
{
#if defined (_M_IA64) || defined (_M_AMD64)
return((((ulIn<<8) & 0x00ff0000) | (ulIn<<24) | ((ulIn>>8) & 0x0000ff00) | (ulIn>>24)));
#else
	__asm
	{
		mov		eax,ulIn
		bswap	eax
		mov		ulIn,eax
	};
return(ulIn);
#endif
}

#define NTOHL(in) ULONG_FLIP(in)
#define HTONL(in) ULONG_FLIP(in)

#endif





#define CLOSE_SOCKET(skt) if (skt) {closesocket(skt);skt=INVALID_SOCKET;}



__inline BOOL SocketGetACCEPTCONN(SOCKET skt)
{
	BOOL bRet;
	int iSize=sizeof(BOOL);
	if (getsockopt(skt,SOL_SOCKET,SO_ACCEPTCONN,(char*)&bRet,(int*)&iSize)!=NO_ERROR) bRet=FALSE;
return(bRet);
}


__inline BOOL SocketGetBROADCAST(SOCKET skt)
{
	BOOL bRet;
	int iSize=sizeof(BOOL);
	if (getsockopt(skt,SOL_SOCKET,SO_BROADCAST,(char*)&bRet,(int*)&iSize)!=NO_ERROR) bRet=FALSE;
return(bRet);
}

__inline int SocketSetBROADCAST(SOCKET skt,BOOL bBroadcast)
{
return(setsockopt(skt,SOL_SOCKET,SO_BROADCAST,(const char*)&bBroadcast,sizeof(BOOL)));
}



__inline int SocketGetCONNECT_TIME(SOCKET skt)
{
	int iSeconds,iSize=sizeof(int);
	if (getsockopt(skt,SOL_SOCKET,SO_CONNECT_TIME,(char*)&iSeconds,(int*)&iSize)!=NO_ERROR) iSeconds=-1;
return(iSeconds);
}


__inline BOOL SocketGetKEEPALIVE(SOCKET skt)
{
	BOOL bRet;
	int iSize=sizeof(BOOL);
	if (getsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,(char*)&bRet,(int*)&iSize)!=NO_ERROR) bRet=FALSE;
return(bRet);
}

__inline int SocketSetKEEPALIVE(SOCKET skt,BOOL bKeepAlive)
{
return(setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,(const char*)&bKeepAlive,sizeof(BOOL)));
}



__inline int SocketGetMAX_MSG_SIZE(SOCKET skt)
{
	int iMaxMsgSize,iSize=sizeof(int);
	if (getsockopt(skt,SOL_SOCKET,SO_MAX_MSG_SIZE,(char*)&iMaxMsgSize,(int*)&iSize)!=NO_ERROR) iMaxMsgSize=-1;
return(iMaxMsgSize);
}


__inline int SocketSetEXCLUSIVEADDRUSE(SOCKET skt,BOOL bExclusiveAddrUse)
{
return(setsockopt(skt,SOL_SOCKET,SO_EXCLUSIVEADDRUSE,(const char*)&bExclusiveAddrUse,sizeof(BOOL)));
}


__inline BOOL SocketGetREUSEADDR(SOCKET skt)
{
	BOOL bRet;
	int iSize=sizeof(BOOL);
	if (getsockopt(skt,SOL_SOCKET,SO_REUSEADDR,(char*)&bRet,(int*)&iSize)!=NO_ERROR) bRet=FALSE;
return(bRet);
}

__inline int SocketSetREUSEADDR(SOCKET skt,BOOL bReuseAddr)
{
return(setsockopt(skt,SOL_SOCKET,SO_REUSEADDR,(const char*)&bReuseAddr,sizeof(BOOL)));
}


__inline int SocketSetRCVBUF(SOCKET skt,unsigned int uiBuffSize)
{
return(setsockopt(skt,SOL_SOCKET,SO_RCVBUF,(const char*)&uiBuffSize,sizeof(int)));
}


__inline int SocketSetSNDBUF(SOCKET skt,unsigned int uiBuffSize)
{
return(setsockopt(skt,SOL_SOCKET,SO_SNDBUF,(const char*)&uiBuffSize,sizeof(int)));
}



__inline int SocketSetUPDATE_ACCEPT_CONTEXT(SOCKET skt,SOCKET sktAccept)
{
return(setsockopt(skt,SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,(char*)&sktAccept,sizeof(SOCKET)));
}


#if(_WIN32_WINNT >= 0x0501)
__inline int SocketSetUPDATE_CONNECT_CONTEXT(SOCKET skt)
{
return(setsockopt(skt,SOL_SOCKET,SO_UPDATE_CONNECT_CONTEXT,NULL,0));
}
#endif




__inline BOOL SocketGetHDRINCL(SOCKET skt)
{
	BOOL bRet;
	int iSize=sizeof(BOOL);
	if (getsockopt(skt,IPPROTO_IP,IP_HDRINCL,(char*)&bRet,(int*)&iSize)!=NO_ERROR) bRet=FALSE;
return(bRet);
}

__inline int SocketSetHDRINCL(SOCKET skt,BOOL bProvideIPHdr)
{
return(setsockopt(skt,IPPROTO_IP,IP_HDRINCL,(const char*)&bProvideIPHdr,sizeof(DWORD)));
}


#define TOS_DEFAULT				0
#define TOS_MIN_MONETARY_COST	2
#define TOS_MIN_RELIABILITY		4
#define TOS_MAX_THROUGHPUT		8
#define TOS_MIN_DELAY			16
#define TOS_MAX_SECURITY		30

__inline int SocketSetTOS(SOCKET skt,DWORD dwTOS)
{
return(setsockopt(skt,IPPROTO_IP,IP_TOS,(const char*)&dwTOS,sizeof(DWORD)));
}


__inline int SocketSetTTL(SOCKET skt,UINT iTTL)
{
return(setsockopt(skt,IPPROTO_IP,IP_TTL,(const char*)&iTTL,sizeof(DWORD)));
}


__inline int SocketSetTCP_NODELAY(SOCKET skt,BOOL bTCPNoDelay)
{
return(setsockopt(skt,IPPROTO_TCP,TCP_NODELAY,(const char*)&bTCPNoDelay,sizeof(DWORD)));
}

/*
int                  // OUT: whatever setsockopt() returns
join_source_group(int sd, u_int32 grpaddr, 
   u_int32 srcaddr, u_int32 iaddr)
{
   struct ip_mreq_source imr; 
   
   imr.imr_multiaddr.s_addr  = grpaddr;
   imr.imr_sourceaddr.s_addr = srcaddr;
   imr.imr_interface.s_addr  = iaddr;
   return setsockopt(sd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &imr, sizeof(imr));  
}

int
leave_source_group(int sd, u_int32 grpaddr, 
   u_int32 srcaddr, u_int32 iaddr)
{
   struct ip_mreq_source imr;

   imr.imr_multiaddr.s_addr  = grpaddr;
   imr.imr_sourceaddr.s_addr = srcaddr;
   imr.imr_interface.s_addr  = iaddr;
   return setsockopt(sd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, &imr, sizeof(imr));
}*/


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
__inline BOOL WINAPI DisconnectExTF(SOCKET hSocket,LPOVERLAPPED lpOverlapped,DWORD dwFlags,DWORD reserved)
{
	reserved=0;
	dwFlags&=TF_REUSE_SOCKET;
	dwFlags|=TF_DISCONNECT;
return(TransmitFile(hSocket,NULL,0,0,lpOverlapped,NULL,dwFlags));
}



__inline DWORD SocketsInitialize()
{
	DWORD dwRetErrorCode;
	WSADATA wsaData;

	if ((dwRetErrorCode=WSAStartup(MAKEWORD(1,1),&wsaData))==NO_ERROR)
	{// version 1.1 OK
		WSACleanup();
		
		dwRetErrorCode=WSAStartup(wsaData.wHighVersion,&wsaData);
	}
return(dwRetErrorCode);
}


#if(_WIN32_WINNT >= 0x0501)
__inline DWORD SocketsInitializeEx(DWORD dwFlags)
{
	DWORD dwRetErrorCode;
	WSADATA wsaData;

	dwFlags=0;
	if ((dwRetErrorCode=WSAStartup(MAKEWORD(1,1),&wsaData))==NO_ERROR)
	{// version 1.1 OK
		WSACleanup();
		if ((dwRetErrorCode=WSAStartup(wsaData.wHighVersion,&wsaData))==NO_ERROR)
		{// max version initialized
			SOCKET skt;

			if ((skt=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED))!=INVALID_SOCKET)
			{
				DWORD dwSize;
				GUID ExtensionGuidCONNECTEX=WSAID_CONNECTEX;
				GUID ExtensionGuidDISCONNECTEX=WSAID_DISCONNECTEX;
				GUID ExtensionGuidTRANSMITPACKETS=WSAID_TRANSMITPACKETS;

				dwRetErrorCode=NO_ERROR;

				if (WSAIoctl(skt,SIO_GET_EXTENSION_FUNCTION_POINTER,&ExtensionGuidCONNECTEX,sizeof(GUID),&ConnectEx,sizeof(FARPROC),&dwSize,NULL,NULL)==0)
				{
					if (ConnectEx==NULL) dwRetErrorCode=WSAGetLastError();
				}else{
					dwRetErrorCode=WSAGetLastError();
				}


				if (WSAIoctl(skt,SIO_GET_EXTENSION_FUNCTION_POINTER,&ExtensionGuidDISCONNECTEX,sizeof(GUID),&DisconnectEx,sizeof(FARPROC),&dwSize,NULL,NULL)==0)
				{
					if (DisconnectEx==NULL)
					{
						DisconnectEx=DisconnectExTF;
						//dwRetErrorCode=WSAGetLastError();
					}
				}else{
					dwRetErrorCode=WSAGetLastError();
				}

				if (WSAIoctl(skt,SIO_GET_EXTENSION_FUNCTION_POINTER,&ExtensionGuidTRANSMITPACKETS,sizeof(GUID),&TransmitPackets,sizeof(FARPROC),&dwSize,NULL,NULL)==0)
				{
					if (TransmitPackets==NULL) dwRetErrorCode=WSAGetLastError();
				}else{
					dwRetErrorCode=WSAGetLastError();
				}

				closesocket(skt);
			}else{
				dwRetErrorCode=WSAGetLastError();
			}
		}
	}
return(dwRetErrorCode);
}
#endif



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///////////////////////////SOCKADDR_STORAGE/////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
#if(_WIN32_WINNT >= 0x0501)

__inline DWORD SockAddrInDataSet(LPCVOID lpcsasSockAddrStorage,DWORD dwPort,LPCVOID lpcAddress,SIZE_T dwAddressSize)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (lpcsasSockAddrStorage && lpcAddress)
	{
		switch(dwAddressSize){
		case sizeof(in_addr):
			((sockaddr_in*)lpcsasSockAddrStorage)->sin_family=AF_INET;
			((sockaddr_in*)lpcsasSockAddrStorage)->sin_port=HTONS((WORD)dwPort);
			(*((DWORD*)&(((sockaddr_in*)lpcsasSockAddrStorage)->sin_addr)))=(*((DWORD*)lpcAddress));
			break;
		case sizeof(in6_addr):
			((sockaddr_in6*)lpcsasSockAddrStorage)->sin6_family=AF_INET6;
			((sockaddr_in6*)lpcsasSockAddrStorage)->sin6_port=HTONS((WORD)dwPort);
			memmove(&(((sockaddr_in6*)lpcsasSockAddrStorage)->sin6_addr),lpcAddress,sizeof(in6_addr));
			break;
		default:
			dwRetErrorCode=ERROR_INVALID_PARAMETER;
			break;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


__inline DWORD SockAddrInPortSet(LPCVOID lpcsasSockAddrStorage,DWORD dwPort)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (lpcsasSockAddrStorage)
	{
		switch(((SOCKADDR_STORAGE*)lpcsasSockAddrStorage)->ss_family){
		case AF_INET:
			((sockaddr_in*)lpcsasSockAddrStorage)->sin_port=HTONS((WORD)dwPort);
			break;
		case AF_INET6:
			((sockaddr_in6*)lpcsasSockAddrStorage)->sin6_port=HTONS((WORD)dwPort);
			break;
		default:
			dwRetErrorCode=ERROR_INVALID_PARAMETER;
			break;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


__inline DWORD SockAddrInAddressSet(LPCVOID lpcsasSockAddrStorage,LPCVOID lpcAddress,SIZE_T dwAddressSize)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (lpcsasSockAddrStorage && lpcAddress)
	{
		switch(dwAddressSize){
		case sizeof(in_addr):
			((sockaddr_in*)lpcsasSockAddrStorage)->sin_family=AF_INET;
			(*((DWORD*)&(((sockaddr_in*)lpcsasSockAddrStorage)->sin_addr)))=(*((DWORD*)lpcAddress));
			break;
		case sizeof(in6_addr):
			((sockaddr_in6*)lpcsasSockAddrStorage)->sin6_family=AF_INET6;
			memmove(&(((sockaddr_in6*)lpcsasSockAddrStorage)->sin6_addr),lpcAddress,sizeof(in6_addr));
			break;
		default:
			dwRetErrorCode=ERROR_INVALID_PARAMETER;
			break;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


__inline DWORD SockAddrAddressSet(LPCVOID lpcsasSockAddrStorage,LPCVOID lpcAddress,SIZE_T dwAddressSize)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (lpcsasSockAddrStorage && lpcAddress)
	{
		switch(dwAddressSize){
		case sizeof(in_addr):
			((sockaddr*)lpcsasSockAddrStorage)->sa_family=AF_INET;
			(*((DWORD*)&(((sockaddr*)lpcsasSockAddrStorage)->sa_data)))=(*((DWORD*)lpcAddress));
			break;
		case sizeof(in6_addr):
			((sockaddr*)lpcsasSockAddrStorage)->sa_family=AF_INET6;
			memmove(&(((sockaddr*)lpcsasSockAddrStorage)->sa_data),lpcAddress,sizeof(in6_addr));
			break;
		default:
			dwRetErrorCode=ERROR_INVALID_PARAMETER;
			break;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


__inline SIZE_T SockAddrGetSize(LPCVOID lpcsasSockAddrStorage)
{
	SIZE_T dwRet;

	if (lpcsasSockAddrStorage)
	{
		switch(((SOCKADDR_STORAGE*)lpcsasSockAddrStorage)->ss_family){
		case AF_INET:
			dwRet=sizeof(sockaddr_in);
			break;
		case AF_INET6:
			dwRet=sizeof(sockaddr_in6);
			break;
		default:
			dwRet=sizeof(SOCKADDR_STORAGE);
			break;
		}
	}else{
		dwRet=0;
	}
return(dwRet);
}


#endif
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#endif //AFX_SOCKET_FUNCTIONS__H__INCLUDED_