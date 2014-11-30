/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

/* Show Frame */
extern HINSTANCE hInst;
static HWND hwndCountdownFrame;
static WORD hFrame;
/* Misc */
static HANDLE hHookModulesLoaded;

/************************* Helpers ************************************/

#define FRAMEELEMENT_BAR     1
#define FRAMEELEMENT_BKGRND  2
#define FRAMEELEMENT_TEXT    3
static COLORREF GetDefaultColor(BYTE id)
{
	switch(id) {
		case FRAMEELEMENT_BAR:
			return RGB(250,0,0); /* same color as used on header icon */
		case FRAMEELEMENT_BKGRND:
			return (COLORREF)db_get_dw(NULL,"CLC","BkColour",CLCDEFAULT_BKCOLOUR);
		case FRAMEELEMENT_TEXT:
			return GetSysColor(COLOR_WINDOWTEXT);
	}
	return 0; /* never happens */
}

static LOGFONT* GetDefaultFont(LOGFONT *lf)
{
	NONCLIENTMETRICS ncm;
	ZeroMemory(&ncm,sizeof(ncm));
	ncm.cbSize=sizeof(ncm);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS,ncm.cbSize,&ncm,0)) {
		*lf=ncm.lfStatusFont;
		return lf;
	}
	return (LOGFONT*)NULL;
}

static HICON SetFrameTitleIcon(WORD hFrame,HICON hNewIcon)
{
	HICON hPrevIcon;
	hPrevIcon=(HICON)CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_ICON,hFrame),0);
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_ICON,hFrame),(LPARAM)hNewIcon);
	if ((int)hPrevIcon==-1) return (HICON)NULL;
	return hPrevIcon;
}

static LRESULT CALLBACK ProgressBarSubclassProc(HWND hwndProgress,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
		case WM_ERASEBKGND:
			return TRUE;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			return SendMessage(GetParent(hwndProgress),msg,wParam,lParam);
	}
	return mir_callNextSubclass(hwndProgress, ProgressBarSubclassProc, msg, wParam, lParam);
}

/************************* Window Class *******************************/

#define COUNTDOWNFRAME_CLASS  _T("AutoShutdownCountdown")

/* Data */
struct CountdownFrameWndData {  /* sizeof=57, max cbClsExtra=40 on Win32 */
	time_t countdown,settingLastTime;
	HANDLE hHookColorsChanged,hHookFontsChanged,hHookIconsChanged;
	HWND hwndIcon,hwndProgress,hwndDesc,hwndTime,hwndToolTip;
	HBRUSH hbrBackground;
	COLORREF clrBackground,clrText;
	HFONT hFont;
	WORD fTimeFlags;
	BYTE flags;
};

/* Flags */
#define FWPDF_PAUSED            0x01
#define FWPDF_PAUSEDSHOWN       0x02
#define FWPDF_COUNTDOWNINVALID  0x04
#define FWPDF_TIMEISCLIPPED     0x08

/* Menu Items */
#define MENUITEM_STOPCOUNTDOWN   1
#define MENUITEM_PAUSECOUNTDOWN  2

/* Messages */
#define M_REFRESH_COLORS    (WM_USER+0)
#define M_REFRESH_ICONS     (WM_USER+1)
#define M_REFRESH_FONTS     (WM_USER+2)
#define M_SET_COUNTDOWN     (WM_USER+3)
#define M_UPDATE_COUNTDOWN  (WM_USER+4)
#define M_CHECK_CLIPPED     (WM_USER+5)
#define M_CLOSE_COUNTDOWN   (WM_USER+6)
#define M_PAUSE_COUNTDOWN   (WM_USER+7)

static LRESULT CALLBACK FrameWndProc(HWND hwndFrame,UINT msg,WPARAM wParam,LPARAM lParam)
{
	struct CountdownFrameWndData *dat=(struct CountdownFrameWndData*)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);

	switch(msg) {
		case WM_NCCREATE:  /* init window data */
			dat=(struct CountdownFrameWndData*)mir_calloc(sizeof(*dat));
			SetWindowLongPtr(hwndFrame, GWLP_USERDATA, (LONG_PTR)dat);
			if (dat==NULL) return FALSE; /* creation failed */
			dat->fTimeFlags=*(WORD*)((CREATESTRUCT*)lParam)->lpCreateParams;
			dat->flags=FWPDF_COUNTDOWNINVALID;
			break;
		case WM_CREATE:  /*  create childs */
		{	CREATESTRUCT *params=(CREATESTRUCT*)lParam;
			dat->hwndIcon=CreateWindowEx(WS_EX_NOPARENTNOTIFY,
					_T("Static"),
					NULL,
					WS_CHILD|WS_VISIBLE|SS_ICON|SS_CENTERIMAGE|SS_NOTIFY,
					3, 0,
					GetSystemMetrics(SM_CXICON),
					GetSystemMetrics(SM_CYICON),
					hwndFrame,
					NULL,
					params->hInstance,
					NULL);
			dat->hwndProgress=CreateWindowEx(WS_EX_NOPARENTNOTIFY,
					PROGRESS_CLASS,
					(dat->fTimeFlags&SDWTF_ST_TIME)?TranslateT("Shutdown at:"):TranslateT("Time left:"),
					WS_CHILD|WS_VISIBLE|PBS_SMOOTH,
					GetSystemMetrics(SM_CXICON)+5,
					5, 90,
					(GetSystemMetrics(SM_CXICON)/2)-5,
					hwndFrame,
					NULL,
					params->hInstance,
					NULL);
			if (dat->hwndProgress==NULL) return -1; /* creation failed, calls WM_DESTROY */
			SendMessage(dat->hwndProgress,PBM_SETSTEP,1,0);
			mir_subclassWindow(dat->hwndProgress, ProgressBarSubclassProc);
			dat->hwndDesc=CreateWindowEx(WS_EX_NOPARENTNOTIFY,
					_T("Static"),
					(dat->fTimeFlags&SDWTF_ST_TIME)?TranslateT("Shutdown at:"):TranslateT("Time left:"),
					WS_CHILD|WS_VISIBLE|SS_LEFTNOWORDWRAP|SS_NOTIFY,
					GetSystemMetrics(SM_CXICON)+5,
					(GetSystemMetrics(SM_CXICON)/2),
					75,
					(GetSystemMetrics(SM_CXICON)/2),
					hwndFrame,
					NULL,
					params->hInstance,
					NULL);
			dat->hwndTime=CreateWindowEx(WS_EX_NOPARENTNOTIFY,
					_T("Static"),
					NULL, /* hh:mm:ss */
					WS_CHILD|WS_VISIBLE|SS_RIGHT|SS_NOTIFY|SS_ENDELLIPSIS,
					(GetSystemMetrics(SM_CXICON)+80),
					(GetSystemMetrics(SM_CXICON)/2),
					35,
					(GetSystemMetrics(SM_CXICON)/2),
					hwndFrame,
					NULL,
					params->hInstance,
					NULL);
			if (dat->hwndTime==NULL) return -1; /* creation failed, calls WM_DESTROY */
			/* create tooltips */
			TTTOOLINFO ti;
			dat->hwndToolTip=CreateWindowEx(WS_EX_TOPMOST,
					TOOLTIPS_CLASS,
					NULL,
					WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX,
					CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT,
					hwndFrame,
					NULL,
					params->hInstance,
					NULL);
			if (dat->hwndToolTip != NULL) {
				SetWindowPos(dat->hwndToolTip,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
				ZeroMemory(&ti,sizeof(ti));
				ti.cbSize=sizeof(ti);
				ti.hwnd=hwndFrame;
				ti.uFlags=TTF_IDISHWND|TTF_SUBCLASS|TTF_TRANSPARENT;
				ti.lpszText=LPSTR_TEXTCALLBACK; /* commctl 4.70+ */
				ti.uId=(UINT)dat->hwndTime; /* in-place tooltip */
				SendMessage(dat->hwndToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);
				ti.uFlags&=~TTF_TRANSPARENT;
				ti.uId=(UINT)dat->hwndProgress;
				SendMessage(dat->hwndToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);
				if (dat->hwndDesc != NULL) {
					ti.uId=(UINT)dat->hwndDesc;
					SendMessage(dat->hwndToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);
				}
				if (dat->hwndIcon != NULL) {
					ti.uId=(UINT)dat->hwndIcon;
					SendMessage(dat->hwndToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);
				}
			}
			/* init layout */
			dat->hHookColorsChanged=HookEventMessage(ME_COLOUR_RELOAD,hwndFrame,M_REFRESH_COLORS);
			dat->hHookFontsChanged=HookEventMessage(ME_FONT_RELOAD,hwndFrame,M_REFRESH_FONTS);
			dat->hHookIconsChanged=HookEventMessage(ME_SKIN2_ICONSCHANGED,hwndFrame,M_REFRESH_ICONS);
			SendMessage(hwndFrame,M_REFRESH_COLORS,0,0);
			SendMessage(hwndFrame,M_REFRESH_FONTS,0,0);
			SendMessage(hwndFrame,M_REFRESH_ICONS,0,0);
			SendMessage(hwndFrame,M_SET_COUNTDOWN,0,0);
			SendMessage(hwndFrame,M_UPDATE_COUNTDOWN,0,0);
			if (!SetTimer(hwndFrame,1,1000,NULL)) return -1; /* creation failed, calls WM_DESTROY */
			return 0;
		}
		case WM_DESTROY:
		{
			if (dat==NULL) return 0;
			UnhookEvent(dat->hHookColorsChanged);
			UnhookEvent(dat->hHookFontsChanged);
			UnhookEvent(dat->hHookIconsChanged);
			/* other childs are destroyed automatically */
			if (dat->hwndToolTip != NULL) DestroyWindow(dat->hwndToolTip);
			HICON hIcon=(HICON)SendMessage(dat->hwndIcon,STM_SETIMAGE,IMAGE_ICON,0);
			break;
		}
		case WM_NCDESTROY:
			if (dat==NULL) return 0;
			if (dat->hFont != NULL) DeleteObject(dat->hFont);
			if (dat->hbrBackground != NULL) DeleteObject(dat->hbrBackground);
			mir_free(dat);
			SetWindowLongPtr(hwndFrame, GWLP_USERDATA, 0);
			break;
		case WM_SIZE:
		{
			RECT rc;
			LONG width,height;
			UINT defflg=SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE;
			SetRect(&rc,0,0,LOWORD(lParam),HIWORD(lParam)); /* width,height */
			/* workaround: reduce flickering of frame in clist */
			InvalidateRect(hwndFrame,&rc,FALSE);
			HDWP hdwp=BeginDeferWindowPos(3);
			/* progress */
			width=rc.right-GetSystemMetrics(SM_CXICON)-10;
			height=rc.bottom-(GetSystemMetrics(SM_CYICON)/2)-5;
			hdwp=DeferWindowPos(hdwp,dat->hwndProgress,NULL,0,0,width,height,SWP_NOMOVE|defflg);
			/* desc */
			if (dat->hwndDesc != NULL) hdwp=DeferWindowPos(hdwp,dat->hwndDesc,NULL,GetSystemMetrics(SM_CXICON)+5,5+height,0,0,SWP_NOSIZE|defflg);
			/* time */
			hdwp=DeferWindowPos(hdwp,dat->hwndTime,NULL,GetSystemMetrics(SM_CXICON)+85,5+height,width-80,(GetSystemMetrics(SM_CXICON)/2),defflg);
			EndDeferWindowPos(hdwp);
			PostMessage(hwndFrame,M_CHECK_CLIPPED,0,0);
			return 0;
		}
		case M_REFRESH_COLORS:
		{
			COLORREF clrBar;
			if (FontService_GetColor(_T("Automatic Shutdown"),_T("Progress Bar"),&clrBar))
				clrBar=GetDefaultColor(FRAMEELEMENT_BAR);
			if (FontService_GetColor(_T("Automatic Shutdown"),_T("Background"),&dat->clrBackground))
				dat->clrBackground=GetDefaultColor(FRAMEELEMENT_BKGRND);
			if (dat->hbrBackground != NULL) DeleteObject(dat->hbrBackground);
			dat->hbrBackground=CreateSolidBrush(dat->clrBackground);
			SendMessage(dat->hwndProgress,PBM_SETBARCOLOR,0,(LPARAM)clrBar);
			SendMessage(dat->hwndProgress,PBM_SETBKCOLOR,0,(LPARAM)dat->clrBackground);
			InvalidateRect(hwndFrame,NULL,TRUE);
			return 0;
		}
		case M_REFRESH_ICONS:
			return 0;
		case M_REFRESH_FONTS:
		{
			LOGFONT lf;
			if (!FontService_GetFont(_T("Automatic Shutdown"),_T("Countdown on Frame"),&dat->clrText,&lf)) {
				if (dat->hFont != NULL) DeleteObject(dat->hFont);
				dat->hFont=CreateFontIndirect(&lf);
			}
			else {
				dat->clrText=GetDefaultColor(FRAMEELEMENT_TEXT);
				if (GetDefaultFont(&lf) != NULL) {
					if (dat->hFont != NULL) DeleteObject(dat->hFont);
					dat->hFont=CreateFontIndirect(&lf);
				}
			}
			if (dat->hwndDesc != NULL)
				SendMessage(dat->hwndDesc,WM_SETFONT,(WPARAM)dat->hFont,FALSE);
			SendMessage(dat->hwndTime,WM_SETFONT,(WPARAM)dat->hFont,FALSE);
			InvalidateRect(hwndFrame,NULL,FALSE);
			return 0;
		}
		case WM_SYSCOLORCHANGE:
			SendMessage(hwndFrame,M_REFRESH_COLORS,0,0);
			break;
		case WM_SETTINGCHANGE: /* colors depend on windows settings */
			SendMessage(hwndFrame,M_REFRESH_COLORS,0,0);
			SendMessage(hwndFrame,M_REFRESH_FONTS,0,0);
			SendMessage(hwndFrame,M_UPDATE_COUNTDOWN,0,0);
			RedrawWindow(hwndFrame,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
			break;
		case WM_TIMECHANGE: /* windows system clock changed */
			SendMessage(hwndFrame,M_SET_COUNTDOWN,0,0);
			PostMessage(hwndFrame,M_UPDATE_COUNTDOWN,0,0);
			break;
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
			SetTextColor((HDC)wParam,dat->clrText);
			SetBkColor((HDC)wParam,dat->clrBackground);
			return (BOOL)dat->hbrBackground;
		case WM_ERASEBKGND:
		{	RECT rc;
			if (dat->hbrBackground != NULL && GetClientRect(hwndFrame,&rc)) {
				FillRect((HDC)wParam,&rc,dat->hbrBackground);
				return TRUE;
			}
			return FALSE;
		}
		case M_SET_COUNTDOWN:
			if (dat->fTimeFlags&SDWTF_ST_TIME) {
				dat->settingLastTime=(time_t)db_get_dw(NULL,"AutoShutdown","TimeStamp",SETTING_TIMESTAMP_DEFAULT);
				dat->countdown=time(NULL);
				if (dat->settingLastTime>dat->countdown) dat->countdown=dat->settingLastTime-dat->countdown;
				else dat->countdown=0;
			}
			else if (dat->flags&FWPDF_COUNTDOWNINVALID) {
				dat->countdown=(time_t)db_get_dw(NULL,"AutoShutdown","Countdown",SETTING_COUNTDOWN_DEFAULT);
				dat->countdown*=(time_t)db_get_dw(NULL,"AutoShutdown","CountdownUnit",SETTING_COUNTDOWNUNIT_DEFAULT);
			}
			dat->flags&=~FWPDF_COUNTDOWNINVALID;
			/* commctl 4.70+, Win95: 1-100 will work fine (wrap around) */
			SendMessage(dat->hwndProgress,PBM_SETRANGE32,0,(LPARAM)dat->countdown);
			return 0;
		case WM_TIMER:
			if (dat==NULL) return 0;
			if (dat->countdown != 0 && !(dat->flags&FWPDF_COUNTDOWNINVALID) && !(dat->flags&FWPDF_PAUSED)) {
				dat->countdown--;
				PostMessage(dat->hwndProgress,PBM_STEPIT,0,0);
			}
			if (IsWindowVisible(hwndFrame)) PostMessage(hwndFrame,M_UPDATE_COUNTDOWN,0,0);
			if (dat->countdown==0) {
				SendMessage(hwndFrame,M_CLOSE_COUNTDOWN,0,0);
				ServiceShutdown(0,TRUE);
				ServiceStopWatcher(0,0);
			}
			return 0;
		case WM_SHOWWINDOW:
			/* the text is kept unchanged while hidden */
			if ((BOOL)wParam) SendMessage(hwndFrame,M_UPDATE_COUNTDOWN,0,0);
			break;
		case M_UPDATE_COUNTDOWN:
			if (dat->flags&FWPDF_PAUSED && !(dat->flags&FWPDF_PAUSEDSHOWN)) {
				SetWindowText(dat->hwndTime,TranslateT("Paused"));
				dat->flags|=FWPDF_PAUSEDSHOWN;
			}
			else {
				TCHAR szOutput[256];
				if (dat->fTimeFlags&SDWTF_ST_TIME)
					GetFormatedDateTime(szOutput,SIZEOF(szOutput),dat->settingLastTime,TRUE);
				else GetFormatedCountdown(szOutput,SIZEOF(szOutput),dat->countdown);
				SetWindowText(dat->hwndTime,szOutput);
				PostMessage(hwndFrame,M_CHECK_CLIPPED,0,0);
				/* update tooltip text (if shown) */
				if (dat->hwndToolTip != NULL && !(dat->flags&FWPDF_PAUSED)) {
					TTTOOLINFO ti;
					ti.cbSize=sizeof(ti);
					if (SendMessage(dat->hwndToolTip,TTM_GETCURRENTTOOL,0,(LPARAM)&ti) && (HWND)ti.uId != dat->hwndIcon)
						SendMessage(dat->hwndToolTip,TTM_UPDATE,0,0);
				} else dat->flags&=~FWPDF_PAUSEDSHOWN;
			}
			return 0;
		case M_CLOSE_COUNTDOWN:
			KillTimer(hwndFrame,1);
			dat->countdown=0;
			dat->flags&=~FWPDF_PAUSED;
			SendMessage(hwndFrame,M_UPDATE_COUNTDOWN,0,0);
			dat->flags|=FWPDF_COUNTDOWNINVALID;
			/* step up to upper range */
			SendMessage(dat->hwndProgress,PBM_SETPOS,SendMessage(dat->hwndProgress,PBM_GETRANGE,FALSE,0),0);
			SetWindowLongPtr(dat->hwndProgress, GWL_STYLE, GetWindowLongPtr(dat->hwndProgress, GWL_STYLE) | PBM_SETMARQUEE);
			SendMessage(dat->hwndProgress,PBM_SETMARQUEE,TRUE,10); /* marquee for rest of time */
			return 0;
		case M_PAUSE_COUNTDOWN:
			if (dat->flags&FWPDF_PAUSED) {
				/* unpause */
				dat->flags&=~(FWPDF_PAUSED|FWPDF_PAUSEDSHOWN);
				SendMessage(hwndFrame,M_SET_COUNTDOWN,0,0);
				SendMessage(dat->hwndProgress,PBM_SETSTATE,PBST_NORMAL,0); /* WinVista+ */
			}
			else {
				/* pause */
				dat->flags|=FWPDF_PAUSED;
				SendMessage(dat->hwndProgress,PBM_SETSTATE,PBST_PAUSED,0); /* WinVista+ */
			}
			SendMessage(hwndFrame,M_UPDATE_COUNTDOWN,0,0);
			return 0;
		case WM_CONTEXTMENU:
		{	HMENU hContextMenu;
			POINT pt;
			if (dat->flags&FWPDF_COUNTDOWNINVALID) return 0;
			POINTSTOPOINT(pt,MAKEPOINTS(lParam));
			if (pt.x==-1 && pt.y==-1) { /* invoked by keyboard */
				RECT rc;
				/* position in middle above rect */
				if (!GetWindowRect(hwndFrame, &rc)) return 0;
				pt.x=rc.left+((int)(rc.right-rc.left)/2);
				pt.y=rc.top+((int)(rc.bottom-rc.top)/2);
			}
			hContextMenu=CreatePopupMenu();
			if (hContextMenu != NULL) {
				AppendMenu(hContextMenu,MF_STRING,MENUITEM_PAUSECOUNTDOWN,(dat->flags&FWPDF_PAUSED)?TranslateT("&Unpause Countdown"):TranslateT("&Pause Countdown"));
				SetMenuDefaultItem(hContextMenu,MENUITEM_PAUSECOUNTDOWN,FALSE);
				AppendMenu(hContextMenu,MF_STRING,MENUITEM_STOPCOUNTDOWN,TranslateT("&Cancel Countdown"));
				TrackPopupMenuEx(hContextMenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_HORPOSANIMATION|TPM_VERPOSANIMATION|TPM_RIGHTBUTTON,pt.x,pt.y,hwndFrame,NULL);
				DestroyMenu(hContextMenu);
			}
			return 0;
		}
		case WM_LBUTTONDBLCLK:
			if (!(dat->flags&FWPDF_COUNTDOWNINVALID))
				SendMessage(hwndFrame,M_PAUSE_COUNTDOWN,0,0);
			return 0;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case MENUITEM_STOPCOUNTDOWN:
					/* close only countdown window when other watcher types running */
					if (dat->fTimeFlags&~(SDWTF_SPECIFICTIME|SDWTF_ST_MASK))
						CloseCountdownFrame(); /* something else is running */
					else ServiceStopWatcher(0,0); /* calls CloseCountdownFrame() */
					return 0;
				case MENUITEM_PAUSECOUNTDOWN:
					SendMessage(hwndFrame,M_PAUSE_COUNTDOWN,0,0);
					return 0;
			}
			break;
		case M_CHECK_CLIPPED: /* for in-place tooltip on dat->hwndTime */
		{	RECT rc;
			HDC hdc;
			SIZE size;
			HFONT hFontPrev=NULL;
			TCHAR szOutput[256];
			dat->flags&=~FWPDF_TIMEISCLIPPED;
			if (GetWindowText(dat->hwndTime,szOutput,SIZEOF(szOutput)))
				if (GetClientRect(dat->hwndTime,&rc)) {
					hdc=GetDC(dat->hwndTime);
					if (hdc != NULL) {
						if (dat->hFont != NULL)
							hFontPrev = (HFONT)SelectObject(hdc,dat->hFont);
						if (GetTextExtentPoint32(hdc,szOutput,lstrlen(szOutput),&size))
							if (size.cx>=(rc.right-rc.left))
								dat->flags&=FWPDF_TIMEISCLIPPED;
						if (dat->hFont != NULL)
							SelectObject(hdc,hFontPrev);
						ReleaseDC(dat->hwndTime,hdc);
					}
				}
			return 0;
		}
		case WM_NOTIFY:
			if (((NMHDR*)lParam)->hwndFrom==dat->hwndToolTip)
				switch(((NMHDR*)lParam)->code) {
					case TTN_SHOW: /* 'in-place' tooltip on dat->hwndTime */
						if (dat->flags&FWPDF_TIMEISCLIPPED && (HWND)wParam==dat->hwndTime) {
							RECT rc;
							if (GetWindowRect(dat->hwndTime,&rc)) {
								SetWindowLongPtr(dat->hwndToolTip, GWL_STYLE, GetWindowLongPtr(dat->hwndToolTip, GWL_STYLE) | TTS_NOANIMATE);
								SetWindowLongPtr(dat->hwndToolTip, GWL_EXSTYLE, GetWindowLongPtr(dat->hwndToolTip, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
								SendMessage(dat->hwndToolTip,TTM_ADJUSTRECT,TRUE,(LPARAM)&rc);
								SetWindowPos(dat->hwndToolTip,NULL,rc.left,rc.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
								return TRUE; /* self-defined position */
							}
						}
						SetWindowLongPtr(dat->hwndToolTip, GWL_STYLE, GetWindowLongPtr(dat->hwndToolTip, GWL_STYLE) & (~TTS_NOANIMATE));
						SetWindowLongPtr(dat->hwndToolTip, GWL_EXSTYLE, GetWindowLongPtr(dat->hwndToolTip, GWL_EXSTYLE) & (~WS_EX_TRANSPARENT));
						return 0;
					case TTN_POP:
						/* workaround #5: frame does not get redrawn after
						 * in-place tooltip	hidden on dat->hwndTime */
						RedrawWindow(hwndCountdownFrame,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
						return 0;
					case TTN_NEEDTEXT:
					{	NMTTDISPINFO *ttdi=(NMTTDISPINFO*)lParam;
						if (dat->flags&FWPDF_TIMEISCLIPPED && (HWND)wParam==dat->hwndTime) {
							if (GetWindowText(dat->hwndTime,ttdi->szText,SIZEOF(ttdi->szText)))
								ttdi->lpszText=ttdi->szText;
						}
						else if ((HWND)wParam==dat->hwndIcon)
							ttdi->lpszText=TranslateT("Automatic Shutdown");
						else {
							TCHAR szTime[SIZEOF(ttdi->szText)];
							if (dat->fTimeFlags&SDWTF_ST_TIME)
								GetFormatedDateTime(szTime,SIZEOF(szTime),dat->settingLastTime,FALSE);
							else GetFormatedCountdown(szTime,SIZEOF(szTime),dat->countdown);
							mir_sntprintf(ttdi->szText,SIZEOF(ttdi->szText),_T("%s %s"),(dat->fTimeFlags&SDWTF_ST_TIME)?TranslateT("Shutdown at:"):TranslateT("Time left:"),szTime);
							ttdi->lpszText=ttdi->szText;
						}
						return 0;
					}
				}
			break;
	}
	return DefWindowProc(hwndFrame,msg,wParam,lParam);
}

/************************* Show Frame *********************************/

void ShowCountdownFrame(WORD fTimeFlags)
{
	hwndCountdownFrame=CreateWindowEx(WS_EX_CONTROLPARENT|WS_EX_NOPARENTNOTIFY|WS_EX_TRANSPARENT,
	                                  COUNTDOWNFRAME_CLASS,
	                                  NULL,
	                                  WS_CHILD|WS_TABSTOP,
	                                  0, 0,
	                                  GetSystemMetrics(SM_CXICON)+103,
	                                  GetSystemMetrics(SM_CYICON)+2,
	                                  (HWND)CallService(MS_CLUI_GETHWND,0,0),
	                                  NULL,
	                                  hInst,
	                                  &fTimeFlags);
	if (hwndCountdownFrame==NULL) return;

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		CLISTFrame clf = { sizeof(clf) };
		clf.hIcon=Skin_GetIcon("AutoShutdown_Active"); /* CListFrames does not make a copy */
		clf.align=alBottom;
		clf.height=GetSystemMetrics(SM_CYICON);
		clf.Flags=F_VISIBLE|F_SHOWTBTIP|F_NOBORDER|F_SKINNED;
		clf.name=Translate("AutoShutdown");
		clf.TBname=Translate("Automatic Shutdown");
		clf.hWnd=hwndCountdownFrame;
		hFrame=(WORD)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&clf,0);
		if (hFrame) {
			ShowWindow(hwndCountdownFrame,SW_SHOW);
			CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBTIPNAME,hFrame),(LPARAM)clf.name);
			/* HACKS TO FIX CLUI FRAMES:
			 * *** why is CLUIFrames is horribly buggy??! *** date: sept 2005, nothing changed until sept 2006
			 * workaround #1: MS_CLIST_FRAMES_REMOVEFRAME does not finish with destroy cycle (clist_modern, clist_nicer crashes) */
			SendMessage((HWND)CallService(MS_CLUI_GETHWND,0,0),WM_SIZE,0,0);
			/* workaround #2: drawing glitch after adding a frame (frame positioned wrongly when hidden) */
			CallService(MS_CLIST_FRAMES_UPDATEFRAME,hFrame,FU_FMPOS|FU_FMREDRAW);
			/* workaround #3: MS_CLIST_FRAMES_SETFRAMEOPTIONS does cause redrawing problems */
			if (!(CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,hFrame),0)&F_VISIBLE))
				CallService(MS_CLIST_FRAMES_SHFRAME,hFrame,0);
			/* workaround #4: MS_CLIST_FRAMES_SHFRAME does cause redrawing problems when frame was hidden */
			RedrawWindow(hwndCountdownFrame,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
			/* workaround #5: for in-place tooltip TTN_POP
			 * workaround #6 and #7: see CloseCountdownFrame() */
		}
	}
}

void CloseCountdownFrame(void)
{
	if (hwndCountdownFrame != NULL) {
		SendMessage(hwndCountdownFrame,M_CLOSE_COUNTDOWN,0,0);
		if (hFrame) {
			/* HACKS TO FIX CLUIFrames:
			 * workaround #6: MS_CLIST_FRAMES_REMOVEFRAME does not finish with
			 * destroy cycle (clist_modern, clist_nicer crashes) */
			CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,hFrame),(LPARAM)CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,hFrame),0)&(~F_VISIBLE));
			#if !defined(_DEBUG)
				/* workaround #7: MS_CLIST_FRAMES_REMOVEFRAME crashes after two calls
				 * clist_nicer crashes after some calls (bug in it) */
				CallService(MS_CLIST_FRAMES_REMOVEFRAME,hFrame,0);
			#endif
		}
		else DestroyWindow(hwndCountdownFrame);
		hwndCountdownFrame=NULL;
		hFrame=0;
	}

}

/************************* Misc ***************************************/

static int FrameModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		LOGFONT lf;
		/* built-in font module is not available before this hook */
		COLORREF clr = GetDefaultColor(FRAMEELEMENT_TEXT);
		FontService_RegisterFont("AutoShutdown","CountdownFont",LPGENT("Automatic Shutdown"),LPGENT("Countdown on Frame"),LPGENT("Automatic Shutdown"),LPGENT("Background"),0,FALSE,GetDefaultFont(&lf),clr);
		clr=GetDefaultColor(FRAMEELEMENT_BKGRND);
		FontService_RegisterColor("AutoShutdown","BkgColor",LPGENT("Automatic Shutdown"),LPGENT("Background"),clr);
		if ( !IsThemeActive()) {
			/* progressbar color can only be changed with classic theme */
			clr=GetDefaultColor(FRAMEELEMENT_BAR);
			FontService_RegisterColor("AutoShutdown","ProgressColor",TranslateT("Automatic Shutdown"),TranslateT("Progress Bar"),clr);
		}
	}
	return 0;
}

int InitFrame(void)
{
	WNDCLASSEX wcx;
	ZeroMemory(&wcx,sizeof(wcx));
	wcx.cbSize        =sizeof(wcx);
	wcx.style         =CS_DBLCLKS|CS_PARENTDC;
	wcx.lpfnWndProc   =FrameWndProc;
	wcx.hInstance     =hInst;
	wcx.hCursor       =(HCURSOR)LoadImage(NULL,IDC_ARROW,IMAGE_CURSOR,0,0,LR_SHARED);
	wcx.lpszClassName =COUNTDOWNFRAME_CLASS;
	if (!RegisterClassEx(&wcx)) return 1;

	hwndCountdownFrame=NULL;
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,FrameModulesLoaded);
	return 0;
}

void UninitFrame(void)
{
	/* frame closed by UninitWatcher() */
	UnregisterClass(COUNTDOWNFRAME_CLASS,hInst); /* fails if window still exists */
	UnhookEvent(hHookModulesLoaded);
}
