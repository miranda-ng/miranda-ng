/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_NEWPLUGINAPI_H__
#define M_NEWPLUGINAPI_H__

#include <m_core.h>

#define PLUGIN_MAKE_VERSION(a, b, c, d)   (((((DWORD)(a))&0xFF)<<24)|((((DWORD)(b))&0xFF)<<16)|((((DWORD)(c))&0xFF)<<8)|(((DWORD)(d))&0xFF))
#define MAXMODULELABELLENGTH 64

#if defined(_UNICODE)
	#define UNICODE_AWARE 0x0001
#else
	#define UNICODE_AWARE 0x0000
#endif

#define STATIC_PLUGIN    0x0002

/* The UUID structure below is used to for plugin UUID's and module type definitions */
typedef struct _MUUID {
  unsigned long a;
  unsigned short b;
  unsigned short c;
  unsigned char d[8];
} MUUID;


/* Used to define the end of the MirandaPluginInterface list */
#define MIID_LAST  {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}}

/* Replaceable internal modules interface ids */
#define MIID_HISTORY        {0x5ca0cbc1, 0x999a, 0x4ea2, {0x8b, 0x44, 0xf8, 0xf6, 0x7d, 0x7f, 0x8e, 0xbe}}
#define MIID_UIUSERINFO     {0x570b931c, 0x9af8, 0x48f1, {0xad, 0x9f, 0xc4, 0x49, 0x8c, 0x61, 0x8a, 0x77}}
#define MIID_SRURL          {0x5192445c, 0xf5e8, 0x46c0, {0x8f, 0x9e, 0x2b, 0x6d, 0x43, 0xe5, 0xc7, 0x53}}
#define MIID_SRAUTH         {0x377780b9, 0x2b3b, 0x405b, {0x9f, 0x36, 0xb3, 0xc4, 0x87, 0x8e, 0x6f, 0x33}}
#define MIID_SRAWAY         {0x5ab54c76, 0x1b4c, 0x4a00, {0xb4, 0x04, 0x48, 0xcb, 0xea, 0x5f, 0xef, 0xe7}}
#define MIID_SREMAIL        {0xd005b5a6, 0x1b66, 0x445a, {0xb6, 0x03, 0x74, 0xd4, 0xd4, 0x55, 0x2d, 0xe2}}
#define MIID_SRFILE         {0x989d104d, 0xacb7, 0x4ee0, {0xb9, 0x6d, 0x67, 0xce, 0x46, 0x53, 0xb6, 0x95}}
#define MIID_UIHELP         {0xf2d35c3c, 0x861a, 0x4cc3, {0xa7, 0x8f, 0xd1, 0xf7, 0x85, 0x04, 0x41, 0xcb}}
#define MIID_UIHISTORY      {0x7f7e3d98, 0xce1f, 0x4962, {0x82, 0x84, 0x96, 0x85, 0x50, 0xf1, 0xd3, 0xd9}}
#define MIID_AUTOAWAY       {0x9c87f7dc, 0x3bd7, 0x4983, {0xb7, 0xfb, 0xb8, 0x48, 0xfd, 0xbc, 0x91, 0xf0}}
#define MIID_USERONLINE     {0x130829e0, 0x2463, 0x4ff8, {0xbb, 0xc8, 0xce, 0x73, 0xc0, 0x18, 0x84, 0x42}}
#define MIID_IDLE           {0x296f9f3b, 0x5b6f, 0x40e5, {0x8f, 0xb0, 0xa6, 0x49, 0x6c, 0x18, 0xbf, 0x0a}}
#define MIID_CRYPTO         {0x415ca6e1, 0x895f, 0x40e6, {0x87, 0xbd, 0x9b, 0x39, 0x60, 0x16, 0xd0, 0xe5}}

/* Common plugin interfaces (core plugins) */
#define MIID_DATABASE       {0xae77fd33, 0xe484, 0x4dc7, {0x8c, 0xbc, 0x09, 0x9f, 0xed, 0xcc, 0xcf, 0xdd}}
#define MIID_CLIST          {0x9d8da8bf, 0x665b, 0x4908, {0x9e, 0x61, 0x9f, 0x75, 0x98, 0xae, 0x33, 0x0e}}
#define MIID_CHAT           {0x23576a43, 0x3a26, 0x4357, {0x9b, 0x1b, 0x4a, 0x71, 0x9e, 0x42, 0x5d, 0x48}}
#define MIID_SRMM           {0x58c7eea6, 0xf9db, 0x4dd9, {0x80, 0x36, 0xae, 0x80, 0x2b, 0xc0, 0x41, 0x4c}}
#define MIID_IMPORT         {0x5f3bcad4, 0x75f8, 0x476e, {0xb3, 0x6b, 0x2b, 0x30, 0x70, 0x32, 0x49, 0x0c}}
#define MIID_TESTPLUGIN     {0x53b974f4, 0x3c74, 0x4dba, {0x8f, 0xc2, 0x6f, 0x92, 0xfe, 0x01, 0x3b, 0x8c}}

/* Special exception interface for protocols.
   This interface allows more than one plugin to implement it at the same time
*/
#define MIID_PROTOCOL    {0x2a3c815e, 0xa7d9, 0x424b, {0xba, 0x30, 0x2, 0xd0, 0x83, 0x22, 0x90, 0x85}}

#define MIID_SERVICEMODE    {0x8a92c026, 0x953a, 0x4f5f, { 0x99, 0x21, 0xf2, 0xc2, 0xdc, 0x19, 0x5e, 0xc5}}

/* Each service mode plugin must implement MS_SERVICEMODE_LAUNCH
   This service might return one of the following values:
	SERVICE_CONTINUE - load Miranda normally, like there's no service plugins at all
	SERVICE_ONLYDB - load database and then execute service plugin only
	SERVICE_MONOPOLY - execute only service plugin, even without database
	SERVICE_FAILED - terminate Miranda execution
*/

#define SERVICE_CONTINUE    0
#define SERVICE_ONLYDB      1
#define SERVICE_MONOPOLY    2
#define SERVICE_FAILED      (-1)

#define MS_SERVICEMODE_LAUNCH "ServiceMode/Launch"

typedef struct PLUGININFOEX_tag
{
	int cbSize;
	char *shortName;
	DWORD version;
	char *description;
	char *author;
	char *authorEmail;
	char *copyright;
	char *homepage;
	BYTE flags;	   // right now the only flag, UNICODE_AWARE, is recognized here
	MUUID uuid;    // plugin's unique identifier
}
	PLUGININFOEX;

//Miranda/System/LoadModule event
//called when a plugin is being loaded dynamically
//wParam = PLUGININFOEX*
//lParam = HINSTANCE of the loaded plugin
#define ME_SYSTEM_MODULELOAD "Miranda/System/LoadModule"

//Miranda/System/UnloadModule event
//called when a plugin is being unloaded dynamically
//wParam = PLUGININFOEX*
//lParam = HINSTANCE of the plugin to be unloaded
#define ME_SYSTEM_MODULEUNLOAD "Miranda/System/UnloadModule"

#endif // M_NEWPLUGINAPI_H__
