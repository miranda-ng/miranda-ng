/*
Wannabe OSD
This plugin tries to become miranda's standard OSD ;-)

(C) 2005 Andrej Krutak

Distributed under GNU's GPL 2 or later
*/

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_clist.h>
#include <m_protosvc.h>
#include <m_ignore.h>
#include <win2k.h>
#include <m_message.h>

#include "resource.h"
#include "version.h"

#define THIS_MODULE "mirandaosd"

//SETTINGS DEFAULTS
#define DEFAULT_FNT_HEIGHT -30
#define DEFAULT_FNT_WIDTH 0
#define DEFAULT_FNT_ESCAPEMENT 0
#define DEFAULT_FNT_ORIENTATION 0
#define DEFAULT_FNT_WEIGHT 700
#define DEFAULT_FNT_ITALIC 0
#define DEFAULT_FNT_UNDERLINE 0
#define DEFAULT_FNT_STRIKEOUT 0
#define DEFAULT_FNT_CHARSET 0
#define DEFAULT_FNT_OUTPRECISION 3
#define DEFAULT_FNT_CLIPRECISION 2
#define DEFAULT_FNT_QUALITY 1
#define DEFAULT_FNT_PITCHANDFAM 49
#define DEFAULT_FNT_FACENAME _T("Arial")

#define DEFAULT_CLRMSG RGB(255, 100, 0) //fore
#define DEFAULT_CLRSTATUS RGB(40, 160, 255) //fore
#define DEFAULT_CLRSHADOW RGB(0, 0, 0) //bk
#define DEFAULT_BKCLR RGB(255, 255, 255)
#define DEFAULT_ALIGN 1
#define DEFAULT_SALIGN 1
#define DEFAULT_DISTANCE 2
#define DEFAULT_ALTSHADOW 0
#define DEFAULT_TRANPARENT 1
#define DEFAULT_WINX 500
#define DEFAULT_WINY 100
#define DEFAULT_WINXPOS 10
#define DEFAULT_WINYPOS 10
#define DEFAULT_ALPHA 175
#define DEFAULT_TIMEOUT 3000
#define DEFAULT_SHOWSHADOW 1
#define DEFAULT_ANNOUNCEMESSAGES 1
#define DEFAULT_ANNOUNCESTATUS 1
#define DEFAULT_ANNOUNCE 0x00000002 //status mask
#define DEFAULT_SHOWMYSTATUS 1
#define DEFAULT_MESSAGEFORMAT _T("Message from %n: %m")
#define DEFAULT_SHOWMSGWIN 1
#define DEFAULT_SHOWWHEN 0x00000002

//HOOKS
#define ME_STATUSCHANGE_CONTACTSTATUSCHANGED "Miranda/StatusChange/ContactStatusChanged"

int ContactStatusChanged(WPARAM wParam, LPARAM lParam);
int ProtoAck(WPARAM wparam,LPARAM lparam);
int ContactSettingChanged(WPARAM wparam,LPARAM lparam);
int HookedNewEvent(WPARAM wParam, LPARAM lParam);

//ANNOUNCING MESSAGES FROM OUTSIDE ;-)
INT_PTR OSDAnnounce(WPARAM wParam, LPARAM lParam);



#define ID_STATUS_MIN	ID_STATUS_OFFLINE
#define ID_STATUS_MAX	ID_STATUS_OUTTOLUNCH

typedef struct _plgsettings {
	int align, salign, altShadow, showShadow, a_user, distance, onlyfromlist, showmystatus;
	int showMsgWindow;
	int messages; //also other events...
	TCHAR msgformat[256];
	int winx, winy, winxpos, winypos, alpha, transparent, timeout;
	COLORREF clr_msg, clr_status, clr_shadow, bkclr;
	DWORD announce, showWhen;
	LOGFONT lf;
} plgsettings;

typedef struct _osdmsg {
	TCHAR *text;
	int timeout; //ms
	COLORREF color;
	void (*callback)(unsigned int param);
	int param;
} osdmsg;

//#define logmsg(x) logmsg2(x)

#ifndef logmsg
#define logmsg(x) //x
#endif

int OptionsInit(WPARAM wparam,LPARAM lparam);

void loadDBSettings(plgsettings *ps);

extern HINSTANCE hI;
extern HWND hwnd;
extern HANDLE hservosda;
extern HANDLE hHookedNewEvent, hHookedInit, hProtoAck, hContactSettingChanged, hHookContactStatusChanged, hContactStatusChanged, hpluginShutDown;