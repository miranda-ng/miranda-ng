/*
Fingerprint NG (client version) icons module for Miranda NG

Copyright © 2006-12 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
#include <malloc.h>

// Miranda IM SDK includes
#include <newpluginapi.h>
#include <win2k.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_protocols.h>
#include <m_userinfo.h>
#include <m_extraicons.h>
#include <m_message.h>
#include <m_metacontacts.h>

#include <m_fingerprint.h>

//plugins header
#include "version.h"
#include "resource.h"

typedef struct {
	BYTE	b;
	BYTE	g;
	BYTE	r;
	BYTE	a;
} RGBA;

struct KN_FP_MASK
{
	LPSTR	szIconName;
	LPTSTR	szMask;
	LPTSTR	szClientDescription;
	int		iIconIndex;
	int		iSectionFlag;
	BOOL	fNotUseOverlay;

	HANDLE	hIcolibItem;
	LPTSTR	szMaskUpper;
};

typedef struct _foundInfo
{
	DWORD	dwArray;
	HANDLE	hRegisteredImage;
} FOUNDINFO;

#define MIRANDA_CASE				3001	//	Miranda clients
#define MIRANDA_VERSION_CASE		3002	//	Miranda version overlays
#define MIRANDA_PACKS_CASE			3003	//	Miranda packs overlays

#define MULTI_CASE					3004	//	multi-protocol clients
#define AIM_CASE					3005	//	AIM clients
#define GG_CASE						3006	//	Gadu-Gadu clients
#define ICQ_CASE					3008	//	ICQ clients
#define IRC_CASE					3009	//	IRC clients
#define JABBER_CASE					3010	//	Jabber clients
#define MRA_CASE					3011	//	Mail.Ru Agent clients
#define MSN_CASE					3012	//	MSN clients
#define QQ_CASE						3013	//	QQ clients (+ versions)
#define RSS_CASE					3014	//	RSS clients
#define TLEN_CASE					3015	//	Tlen clients (+ versions)
#define WEATHER_CASE				3016	//	Weather clients
#define YAHOO_CASE					3017	//	Yahoo clients (+ versions)
#define FACEBOOK_CASE               3018    //  Facebook clients
#define VK_CASE                     3019    //  VK clients

#define OTHER_PROTOS_CASE			3020	//	other protocols
#define OTHERS_CASE					3021	//	other icons

#define OVERLAYS_RESOURCE_CASE		3022	//	resource overlays
#define OVERLAYS_PLATFORM_CASE		3023	//	platforms overlays
#define OVERLAYS_PROTO_CASE			3024	//	protocols overlays
#define OVERLAYS_UNICODE_CASE		3025	//	unicode overlay
#define OVERLAYS_SECURITY_CASE		3026	//	security overlays

/*
#define OVERLAYS_RESOURCE_ALT_CASE	24		//	alternative (old style) overlays
*/

#define PtrIsValid(p)		(((p)!=0)&&(((HANDLE)(p))!=INVALID_HANDLE_VALUE))
#define SAFE_FREE(p)		{if (PtrIsValid(p)){free((VOID*)p);(p)=NULL;}}

#define LIB_REG		2
#define LIB_USE		3

#define MODULENAME   "Finger"

#define DEFAULT_SKIN_FOLDER		_T("Icons\\Fp_icons.dll")

void ClearFI();

void InitFingerModule(void);

int  OnOptInitialise(WPARAM wParam, LPARAM lParam);
int  OnExtraImageApply(WPARAM wParam, LPARAM lParam);

HICON __fastcall CreateJoinedIcon(HICON hBottom, HICON hTop);
HBITMAP __inline CreateBitmap32(int cx, int cy);
HBITMAP __fastcall CreateBitmap32Point(int cx, int cy, LPVOID* bits);
HANDLE __fastcall GetIconIndexFromFI(LPTSTR szMirVer);

BOOL __fastcall WildCompareW(LPWSTR name, LPWSTR mask);

void __fastcall Prepare(KN_FP_MASK* mask);
void RegisterIcons();

#define WildCompare		WildCompareW
#define GetIconsIndexes	GetIconsIndexesW

extern HINSTANCE g_hInst;
extern HANDLE hHeap;

extern KN_FP_MASK 
	def_kn_fp_mask[], 
	def_kn_fp_overlays_mask[], 
	def_kn_fp_overlays1_mask[], 
	def_kn_fp_overlays2_mask[],
	def_kn_fp_overlays3_mask[],
	def_kn_fp_overlays4_mask[];

extern int DEFAULT_KN_FP_MASK_COUNT, DEFAULT_KN_FP_OVERLAYS_COUNT, DEFAULT_KN_FP_OVERLAYS2_COUNT, DEFAULT_KN_FP_OVERLAYS3_COUNT, DEFAULT_KN_FP_OVERLAYS4_COUNT;

#define UNKNOWN_MASK_NUMBER (DEFAULT_KN_FP_MASK_COUNT - 2)								// second from end
#define NOTFOUND_MASK_NUMBER (DEFAULT_KN_FP_MASK_COUNT - 3)								// third from end
// the last count is how many masks from 2nd layer is used as Miranda version overlays	(counting from the end)
#define DEFAULT_KN_FP_OVERLAYS2_NO_VER_COUNT (DEFAULT_KN_FP_OVERLAYS2_COUNT - 13)
#define DEFAULT_KN_FP_OVERLAYS3_NO_UNICODE_COUNT (DEFAULT_KN_FP_OVERLAYS3_COUNT - 1)
