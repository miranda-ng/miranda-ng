/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: commonheaders.h 133 2010-09-30 06:27:18Z silvercircle $
 *
 */

//#define _USE_D2D 1

#define _WIN32_WINNT 0x0502
#define _WIN32_IE 0x0501

#undef FASTCALL

#define TSAPI __stdcall
#define FASTCALL __fastcall

#ifdef _MSC_VER
	#if _MSC_VER < 1400
		#define uintptr_t UINT_PTR
		#define _localtime32 localtime
		#define __time32_t time_t
	#endif
#endif

int _DebugTraceW(const wchar_t *fmt, ...);
int _DebugTraceA(const char *fmt, ...);

#include <windows.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <time.h>
#include <shlwapi.h>
#include <Richedit.h>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_system_cpp.h>
#include <m_langpack.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_addcontact.h>
#include <m_timezones.h>
#include <m_cluiframes.h>
#include <m_clui.h>
#include <m_icolib.h>
#include <m_fontservice.h>
#include <m_xstatus.h>
#include <m_extraicons.h>
#include <m_variables.h>
#include <m_ignore.h>

#include <m_metacontacts.h>

#include <agg_rendering_buffer.h>
#include <agg_renderer_base.h>
#include <agg_pixfmt_rgba.h>
#include <agg_renderer_scanline.h>
#include <agg_scanline_p.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_rasterizer_outline_aa.h>
#include <agg_ellipse.h>
#include <agg_rounded_rect.h>
#include <agg_path_storage.h>
#include <agg_path_storage_integer.h>
#include <agg_span_allocator.h>
#include <agg_span_gouraud_rgba.h>
#include <agg_span_gradient.h>
#include <agg_span_interpolator_linear.h>
#include <agg_span_interpolator_trans.h>
#include "../cluiframes/cluiframes.h"

#ifdef _USE_D2D
	#include <d2d1Helper.h>
#endif

#include  <resource.h>

#include  <win2k.h>
#include  <newpluginapi.h>
#include  <m_imgsrvc.h>
#include  <m_system.h>
#include  <m_database.h>
#include  <m_langpack.h>
#include  <m_button.h>
#include  <m_clist.h>
#include  <m_clistint.h>
#include  <m_cluiframes.h>
#include  <m_genmenu.h>
#include  <m_options.h>
#include  <m_protosvc.h>
#include  <m_utils.h>
#include  <m_skin.h>
#include  <m_contacts.h>
#include  <m_icolib.h>
#include  <m_clc.h>
#include  <m_clui.h>
#include  <m_userinfo.h>
#include  <m_history.h>
#include  <m_addcontact.h>
#include  <m_file.h>
#include  <m_fontservice.h>
#include  <m_acc.h>
#include  <m_hotkeys.h>
#include  <m_genmenu.h>
#include  <m_timezones.h>

#include  <m_metacontacts.h>

extern IconItemT iconItem[];

	/*
	 * text shadow types (DrawThemeTextEx() / Vista+ uxtheme)
	 */
	#define TST_NONE			0
	#define TST_SINGLE			1
	#define TST_CONTINUOUS		2

	typedef struct _DWM_THUMBNAIL_PROPERTIES
	{
		DWORD dwFlags;
		RECT rcDestination;
		RECT rcSource;
		BYTE opacity;
		BOOL fVisible;
		BOOL fSourceClientAreaOnly;
	} DWM_THUMBNAIL_PROPERTIES, *PDWM_THUMBNAIL_PROPERTIES;

	enum DWMWINDOWATTRIBUTE
	{
	    DWMWA_NCRENDERING_ENABLED = 1,      // [get] Is non-client rendering enabled/disabled
	    DWMWA_NCRENDERING_POLICY,           // [set] Non-client rendering policy
	    DWMWA_TRANSITIONS_FORCEDISABLED,    // [set] Potentially enable/forcibly disable transitions
	    DWMWA_ALLOW_NCPAINT,                // [set] Allow contents rendered in the non-client area to be visible on the DWM-drawn frame.
	    DWMWA_CAPTION_BUTTON_BOUNDS,        // [get] Bounds of the caption button area in window-relative space.
	    DWMWA_NONCLIENT_RTL_LAYOUT,         // [set] Is non-client content RTL mirrored
	    DWMWA_FORCE_ICONIC_REPRESENTATION,  // [set] Force this window to display iconic thumbnails.
	    DWMWA_FLIP3D_POLICY,                // [set] Designates how Flip3D will treat the window.
	    DWMWA_EXTENDED_FRAME_BOUNDS,        // [get] Gets the extended frame bounds rectangle in screen space
	    DWMWA_HAS_ICONIC_BITMAP,            // [set] Indicates an available bitmap when there is no better thumbnail representation.
	    DWMWA_DISALLOW_PEEK,                // [set] Don't invoke Peek on the window.
	    DWMWA_EXCLUDED_FROM_PEEK,           // [set] LivePreview exclusion information
	    DWMWA_LAST
	};

	#define DWM_TNP_RECTDESTINATION	0x00000001
	#define DWM_TNP_RECTSOURCE 0x00000002
	#define DWM_TNP_OPACITY	0x00000004
	#define DWM_TNP_VISIBLE	0x00000008
	#define DWM_TNP_SOURCECLIENTAREAONLY 0x00000010

	#define DWM_SIT_DISPLAYFRAME    0x00000001  // Display a window frame around the provided bitmap

	typedef HANDLE HTHUMBNAIL;
	typedef HTHUMBNAIL* PHTHUMBNAIL;

#ifndef BPPF_ERASE
	typedef enum _BP_BUFFERFORMAT
	{
		BPBF_COMPATIBLEBITMAP,    // Compatible bitmap
		BPBF_DIB,                 // Device-independent bitmap
		BPBF_TOPDOWNDIB,          // Top-down device-independent bitmap
		BPBF_TOPDOWNMONODIB       // Top-down monochrome device-independent bitmap
	} BP_BUFFERFORMAT;


	typedef struct _BP_PAINTPARAMS
	{
		DWORD                       cbSize;
		DWORD                       dwFlags; // BPPF_ flags
		const RECT *                prcExclude;
		const BLENDFUNCTION *       pBlendFunction;
	} BP_PAINTPARAMS, *PBP_PAINTPARAMS;

	#define BPPF_ERASE               1
	#define BPPF_NOCLIP              2
	#define BPPF_NONCLIENT           4
#endif

	typedef struct _DWM_BLURBEHIND
	{
		DWORD dwFlags;
		BOOL fEnable;
		HRGN hRgnBlur;
		BOOL fTransitionOnMaximized;
	} DWM_BLURBEHIND, *PDWM_BLURBEHIND;

	#define DWM_BB_ENABLE 1

#ifndef LOCALE_SISO3166CTRYNAME2
	#define LOCALE_SISO3166CTRYNAME2      0x00000068   // 3 character ISO country name, eg "USA Vista+
	#define LOCALE_SISO639LANGNAME2       0x00000067   // 3 character ISO abbreviated language name, eg "eng"
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
	#define WM_DWMCOMPOSITIONCHANGED        0x031E
	#define WM_DWMCOLORIZATIONCOLORCHANGED  0x0320
#endif

#ifndef WM_DWMSENDICONICTHUMBNAIL
	#define WM_DWMSENDICONICTHUMBNAIL           0x0323
	#define WM_DWMSENDICONICLIVEPREVIEWBITMAP   0x0326
#endif

	// some typedefs

#include  <memory>
typedef std::unique_ptr<std::basic_string<wchar_t> > pSmartWstring;

#include "m_cln_skinedit.h"
#include "clui.h"
#include <m_avatars.h>
#include "config.h"
#include "clc.h"
#include "skin.h"
#include "gfx.h"
#include "utils.h"
#include "clist.h"
#include "rowheight_funcs.h"

// shared vars
extern HINSTANCE g_hInst;

#define MAX_REGS(_A_) (sizeof(_A_)/sizeof(_A_[0]))
#define CXSMICON 16
#define CYSMICON 16

extern CLIST_INTERFACE coreCli;

#define safe_sizeof(a) (sizeof((a)) / sizeof((a)[0]))

BOOL __forceinline GetItemByStatus(int status, TStatusItem *retitem);

void FreeAndNil( void** );

#define  EXTRA_ICON_RES0	0	// only used by nicer
#define  EXTRA_ICON_EMAIL	1
#define  EXTRA_ICON_PROTO	2	// used by mwclist and modern
#define  EXTRA_ICON_RES1	2	// only used by nicer
#define  EXTRA_ICON_SMS		3
#define  EXTRA_ICON_ADV1	4
#define  EXTRA_ICON_ADV2	5
#define  EXTRA_ICON_WEB		6
#define  EXTRA_ICON_CLIENT	7
#define  EXTRA_ICON_VISMODE	8	// only used by modern
#define  EXTRA_ICON_RES2	8	// only used by nicer
#define  EXTRA_ICON_ADV3	9
#define  EXTRA_ICON_ADV4	10

