// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <winsock2.h>
#include <prsht.h>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_database.h>
#include <m_protomod.h>
#include <m_langpack.h>

#include "resource.h"
#include "Version.h"
#include "get_time.h"
#include "lan.h"
#include "mlan.h"

#define PROTONAME "EM_LAN_PROTO"

//#define ME_ICQ_STATUSMSGREQ "ICQ/StatusMsgReq"
//
//#define ICQ_MSGTYPE_GETAWAYMSG  0xE8
//#define ICQ_MSGTYPE_GETOCCUMSG  0xE9
//#define ICQ_MSGTYPE_GETNAMSG    0xEA
//#define ICQ_MSGTYPE_GETDNDMSG   0xEB
//#define ICQ_MSGTYPE_GETFFCMSG   0xEC

//#define VERBOSE

#ifdef VERBOSE
#include <fstream>
extern std::fstream emlanLog;
#define EMLOG(x) emlanLog << "[" << __FUNCTION__ << "] [" << __FILE__ << ":" << __LINE__ << "] " << x << std::endl
#define EMLOGIF(x, y) if (y) EMLOG(x)
inline const char* showErrName(int err)
{
	const char* name = "unknown";
	switch (err)
	{
	case 0: name = "NOERROR"; break;
	case WSANOTINITIALISED: name = "WSANOTINITIALIZED"; break;
	case WSAENETDOWN: name = "WSAENETDOWN"; break;
	case WSAEACCES: name = "WSAEACCES"; break;
	case WSAEFAULT: name = "WSAEFAULT"; break;
	case WSAENOTCONN: name = "WSAENOTCONN"; break;
	case WSAEINTR: name = "WSAEINTR"; break;
	case WSAEINPROGRESS: name = "WSAEINPROGRESS"; break;
	case WSAENETRESET: name = "WSAENETRESET"; break;
	case WSAENOTSOCK: name = "WSAENOTSOCK"; break;
	case WSAEOPNOTSUPP: name = "WSAEOPNOTSUPP"; break;
	case WSAESHUTDOWN: name = "WSAESHUTDOWN"; break;
	case WSAEWOULDBLOCK: name = "WSAEWOULDBLOCK"; break;
	case WSAEMSGSIZE: name = "WSAEMSGSIZE"; break;
	case WSAEINVAL: name = "WSAEINVAL"; break;
	case WSAECONNABORTED: name = "WSAECONNABORTED"; break;
	case WSAETIMEDOUT: name = "WSAETIMEDOUT"; break;
	case WSAECONNRESET: name = "WSAECONNRESET"; break;
	case WSAENOBUFS: name = "WSAENOBUFS"; break;
	case WSAEHOSTUNREACH: name = "WSAEHOSTUNREACH"; break;
	}
	return name;
}
#define EMLOGERR() { int err = WSAGetLastError(); const char* name = showErrName(err); EMLOG( "Error: " << err << '(' << name << ')' ); }
#define EMLOGERRIF(y) if (y) EMLOGERR()
#else
#define EMLOG(x)
#define EMLOGIF(x, y)
#define EMLOGERR()
#define EMLOGERRIF()
#endif
