#ifndef _COMMON_H_
#define _COMMON_H_

//#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <newpluginapi.h>

#include <m_options.h>
#include <m_database.h>

#include <m_protoint.h>
#include <m_protomod.h>
#include <m_protosvc.h>

#include <m_netlib.h>

#include <m_json.h>
#include <m_langpack.h>
#include <m_string.h>

#include "version.h"
#include "..\res\resource.h"

#define MODULE "DropBox"

extern HINSTANCE     g_hInstance;
extern HANDLE        g_hNetlibUser;

#endif //_COMMON_H_