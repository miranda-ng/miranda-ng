/*
Fingerprint Mod+ (client version) icons module for Miranda IM

Copyright © 2006-2007 Artem Shpynov aka FYR, Bio, Faith Healer. 2009-2010 HierOS

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

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _CRT_SECURE_NO_DEPRECATE

//Start of header
// Native include
#include <windows.h>
#include <windowsx.h>
#include <malloc.h>

// Miranda IM SDK includes
#include <newpluginapi.h>
#include <win2k.h>
#include <m_cluiframes.h>
#include <m_database.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_protocols.h>
#include <m_userinfo.h>

// plugins SDK
#include <m_extraicons.h>
#include <m_folders.h>
#include <m_updater.h>

//plugins header
#include "version.h"
#include "m_fingerprint.h"
#include "resource.h"			  
#include "utilities.h"

#if defined(__GNUC__)
#define _alloca alloca
//#define FASTCALL
#else
#define FASTCALL __fastcall
#endif

typedef struct {
	BYTE	b;
	BYTE	g;
	BYTE	r;
	BYTE	a;
} RGBA;

typedef struct _knfpMask
{
	LPSTR	szIconName;
	LPTSTR	szMask;
	LPTSTR	szClientDescription;
	LPTSTR	szIconFileName;
	int		iIconIndex;
	int		iSectionFlag;
	BOOL	fNotUseOverlay;
} KN_FP_MASK;

typedef struct _foundInfo
{
	DWORD	dwArray;
	HANDLE	hRegisteredImage;
} FOUNDINFO;

#define MIRANDA_CASE			1
#define MULTIPROTOCOL_CASE		2
#define ICQ_OFF_CASE			3
#define ICQ_UNOFF_CASE			4
#define JABBER_CASE				5
#define MSN_CASE				6
#define AIM_CASE				7
#define YAHOO_CASE				8
#define IRC_CASE				9
#define VOIP_CASE				10
#define SKYPE_CASE				11
#define GADU_CASE				12
#define PACKS_CASE				13
#define MRA_CASE				14
#define OVER1_CASE				15
#define SECURITY_CASE			16
#define OTHER_CASE				17
#define OVER2_CASE				18
#define PLATFORM_CASE			19
#define WEATHER_CASE			20
#define RSS_CASE				21
#define QQ_CASE					22
#define TLEN_CASE				23

typedef struct _settingsInfo
{
	int idCtrl;
	LPCSTR szSetName;
} SETTINGS_INFO;

SETTINGS_INFO settings [] =
{
	{IDC_OVERLAY1, "Overlay1"},
	{IDC_OVERLAY2, "Overlay2"},
	{IDC_OVERLAY3, "Overlay3"},
	{IDC_VERSION, "ShowVersion"},

	{IDC_GROUPMIRANDA, "GroupMiranda"},
	{IDC_GROUPMULTI, "GroupMulti"},
	{IDC_GROUPPACKS, "GroupPacks"},
	{IDC_GROUPOTHERS, "GroupOtherProtos"},

	{IDC_GROUPAIM, "GroupAim"},
	{IDC_GROUPGADU, "GroupGadu"},
	{IDC_GROUPICQ, "GroupIcq"}, 
	{IDC_GROUPIRC, "GroupIrc"},
	{IDC_GROUPJABBER, "GroupJabber"},
	{IDC_GROUPMAIL, "GroupMail"},
	{IDC_GROUPMSN, "GroupMsn"},
	{IDC_GROUPQQ, "GroupQQ"},
	{IDC_GROUPRSS, "GroupRSS"},
	{IDC_GROUPSKYPE, "GroupSkype"},
	{IDC_GROUPTLEN, "GroupTlen"},
	{IDC_GROUPVOIP, "GroupVoIP"},
	{IDC_GROUPWEATHER, "GroupWeather"},
	{IDC_GROUPYAHOO, "GroupYahoo"}
};
#define DEFAULT_SETTINGS_COUNT SIZEOF(settings)

#define PtrIsValid(p)		(((p)!=0)&&(((HANDLE)(p))!=INVALID_HANDLE_VALUE))
#define SAFE_FREE(p)		{if (PtrIsValid(p)){free((VOID*)p);(p)=NULL;}}

#define DEFAULT_SKIN_FOLDER		_T("Icons\\Fp_ClientIcons")

extern BYTE gbUnicodeAPI;
