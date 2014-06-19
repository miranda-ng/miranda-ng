//This file is part of HTTPServer a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef HTTP_SERVER_GLOB_H
#define HTTP_SERVER_GLOB_H

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <string>
using namespace std;

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_clui.h>
#include <m_userinfo.h>
#include <m_netlib.h>
#include <m_message.h>
#include <m_popup.h>
#include <m_protosvc.h>

#include <m_HTTPServer.h>

#include "FileShareNode.h"
#include "HttpUser.h"
#include "GuiElements.h"
#include "MimeHandling.h"
#include "resource.h"
#include "IndexCreation.h"
#include "Version.h"


#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

#define MODULE "HTTPServer"
#define MSG_BOX_TITEL TranslateT("Miranda NG HTTP-Server")

#define SplitIpAddress( p ) (BYTE)(p>>24),(BYTE)(p>>16),(BYTE)(p>>8),(BYTE)(p)

extern HINSTANCE hInstance;

extern HANDLE hNetlibUser;

extern bool bShutdownInProgress;
bool bWriteConfigurationFile();

void LogEvent(const char * pszTitle, const char * pszLog);
bool bOpenLogFile();

extern char szPluginPath[MAX_PATH];
extern int nPluginPathLen;

extern DWORD dwLocalIpAddress;
extern DWORD dwLocalPortUsed;
extern DWORD dwExternalIpAddress;

extern int nMaxUploadSpeed;
extern int nMaxConnectionsTotal;
extern int nMaxConnectionsPerUser;
extern int nDefaultDownloadLimit;
extern bool bIsOnline;
extern bool bLimitOnlyWhenOnline;

extern void* (*MirandaMalloc)(size_t);
extern void (*MirandaFree)(void*);

#endif