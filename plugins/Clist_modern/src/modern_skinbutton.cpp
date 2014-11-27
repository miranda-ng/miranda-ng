/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

/*
This file contains code related to new modern free positioned skinned buttons
*/

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_skinengine.h"
#include "hdr/modern_clcpaint.h"
#include "m_api/m_skinbutton.h"

#define MODERNSKINBUTTONCLASS "MirandaModernSkinButtonClass"
BOOL ModernSkinButtonModuleIsLoaded = FALSE;
static LRESULT CALLBACK ModernSkinButtonWndProc(HWND hwndDlg, UINT msg,  WPARAM wParam, LPARAM lParam);
int ModernSkinButtonUnloadModule(WPARAM wParam, LPARAM lParam);
int SkinSelector_DeleteMask(MODERNMASK *mm);
HWND SetToolTip(HWND hwnd, TCHAR * tip);

typedef struct _ModernSkinButtonCtrl
{
	HWND    hwnd;
	BYTE    down; // button state
	BYTE    focus;   // has focus (1 or 0)
	BYTE    hover;
	BYTE    IsSwitcher;
	BOOL    fCallOnPress;
	char    * ID;
	char    * CommandService;
	char    * StateService;
	char    * HandleService;
	char    * ValueDBSection;
	char    * ValueTypeDef;
	int     Left, Top, Bottom, Right;
	HMENU   hMenu;
	TCHAR   * Hint;

} ModernSkinButtonCtrl;
typedef struct _HandleServiceParams
{
	HWND    hwnd;
	DWORD   msg;
	WPARAM  wParam;
	LPARAM  lParam;
	BOOL    handled;
} HandleServiceParams;

static CRITICAL_SECTION csTips;
static HWND hwndToolTips = NULL;

int ModernSkinButtonLoadModule()
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize         = sizeof(wc);
	wc.lpszClassName  = _T(MODERNSKINBUTTONCLASS);
	wc.lpfnWndProc    = ModernSkinButtonWndProc;
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra     = sizeof(ModernSkinButtonCtrl*);
	wc.hbrBackground  = 0;
	wc.style          = CS_GLOBALCLASS;
	RegisterClassEx(&wc);
	InitializeCriticalSection(&csTips);
	ModernSkinButtonModuleIsLoaded = TRUE;
	return 0;
}

int ModernSkinButtonUnloadModule(WPARAM wParam, LPARAM lParam)
{
	DeleteCriticalSection(&csTips);
	return 0;
}

static int ModernSkinButtonPaintWorker(HWND hwnd, HDC whdc)
{
	HDC hdc;
	HBITMAP bmp,oldbmp;
	RECT rc;
	HDC sdc = NULL;
	ModernSkinButtonCtrl* bct = (ModernSkinButtonCtrl *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!bct) return 0;
	if (!IsWindowVisible(hwnd)) return 0;
	if (!whdc && !g_CluiData.fLayered) InvalidateRect(hwnd,NULL,FALSE);

	if (whdc && g_CluiData.fLayered) hdc = whdc;
	else
	{
		//sdc = GetWindowDC(GetParent(hwnd));
		hdc = CreateCompatibleDC(NULL);
	}
	GetClientRect(hwnd,&rc);
	bmp = ske_CreateDIB32(rc.right,rc.bottom);
	oldbmp = (HBITMAP)SelectObject(hdc,bmp);
	if (!g_CluiData.fLayered)
		ske_BltBackImage(bct->hwnd,hdc,NULL);
	{
		MODERNMASK Request = {0};
		//   int res;
		//HBRUSH br = CreateSolidBrush(RGB(255,255,255));
		char * Value = NULL;
		DWORD val = 0;
		{
			if (bct->ValueDBSection && bct->ValueTypeDef)
			{
				char * key;
				char * section;
				DWORD defval = 0;
				char buf[20];
				key = mir_strdup(bct->ValueDBSection);
				section = key;
				if (bct->ValueTypeDef[0] != 's')
					defval = (DWORD)atol(bct->ValueTypeDef+1);
				do
				{
					if (key[0] == '/') {key[0] = '\0'; key++; break;}
					key++;
				} while (key[0] != '\0');
				switch (bct->ValueTypeDef[0])
				{
				case 's':
					{
						Value = db_get_sa(NULL,section,key);
						if (!Value)
							Value = mir_strdup(bct->ValueTypeDef+1);
						break;
					}
				case 'd':
					defval = db_get_dw(NULL,section,key,defval);
					Value = mir_strdup(_ltoa(defval,buf,SIZEOF(buf)));
					break;
				case 'w':
					defval = db_get_w(NULL,section,key,defval);
					Value = mir_strdup(_ltoa(defval,buf,SIZEOF(buf)));
					break;
				case 'b':
					defval = db_get_b(NULL,section,key,defval);
					Value = mir_strdup(_ltoa(defval,buf,SIZEOF(buf)));
					break;
				}
				mir_free(section);
			}

		}
		g_clcPainter.AddParam(&Request,mod_CalcHash("Module"),"MButton",0);
		g_clcPainter.AddParam(&Request,mod_CalcHash("ID"),bct->ID,0);
		g_clcPainter.AddParam(&Request,mod_CalcHash("Down"),bct->down?"1":"0",0);
		g_clcPainter.AddParam(&Request,mod_CalcHash("Focused"),bct->focus?"1":"0",0);
		g_clcPainter.AddParam(&Request,mod_CalcHash("Hovered"),bct->hover?"1":"0",0);
		if (Value) {
			g_clcPainter.AddParam(&Request,mod_CalcHash("Value"),Value,0);
			mir_free(Value);
		}
		SkinDrawGlyphMask(hdc,&rc,&rc,&Request);
		SkinSelector_DeleteMask(&Request);
		// DeleteObject(br);
	}

	if (!whdc && g_CluiData.fLayered)
	{
		RECT r;
		SetRect(&r,bct->Left,bct->Top,bct->Right,bct->Bottom);
		ske_DrawImageAt(hdc,&r);
		//CallingService to immeadeately update window with new image.
	}
	if (whdc && !g_CluiData.fLayered)
	{
		RECT r = {0};
		GetClientRect(bct->hwnd,&r);
		BitBlt(whdc, 0, 0, r.right,r.bottom,hdc, 0, 0, SRCCOPY);
	}
	SelectObject(hdc,oldbmp);
	DeleteObject(bmp);
	if (!whdc || !g_CluiData.fLayered)
	{
		SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
		DeleteDC(hdc);
	}
	//  if (sdc)
	//    ReleaseDC(GetParent(hwnd),sdc);
	return 0;
}

static int ModernSkinButtonToggleDBValue(char * ValueDBSection,char *ValueTypeDef)
{
	if (ValueDBSection && ValueTypeDef)
	{
		char * key;
		char * section;
		char * val;
		char * val2;
		char * Value;
		long l1,l2,curval;
		DWORD defval = 0;
		//        char buf[20];
		key = mir_strdup(ValueDBSection);
		section = key;
		do
		{
			if (key[0] == '/') {key[0] = '\0'; key++; break;}
			key++;
		} while (key[0] != '\0');

		val = mir_strdup(ValueTypeDef+1);
		val2 = val;
		do
		{
			if (val2[0] == '/') {val2[0] = '\0'; val2++; break;}
			val2++;
		} while (val2[0] != '\0');

		if (ValueTypeDef[0] != 's')
		{
			l1 = (DWORD)atol(val);
			l2 = (DWORD)atol(val2);
		}

		switch (ValueTypeDef[0]) {
		case 's':
			Value = db_get_sa(NULL,section,key);
			if (!Value  || (Value && mir_bool_strcmpi(Value,val2)))
				Value = mir_strdup(val);
			else
				Value = mir_strdup(val2);
			db_set_s(NULL,section,key,Value);
			mir_free(Value);
			break;

		case 'd':
			curval = db_get_dw(NULL,section,key,l2);
			curval = (curval == l2)?l1:l2;
			db_set_dw(NULL,section,key,(DWORD)curval);
			break;

		case 'w':
			curval = db_get_w(NULL,section,key,l2);
			curval = (curval == l2)?l1:l2;
			db_set_w(NULL,section,key,(WORD)curval);
			break;

		case 'b':
			curval = db_get_b(NULL,section,key,l2);
			curval = (curval == l2)?l1:l2;
			db_set_b(NULL,section,key,(BYTE)curval);
			break;
		}
		mir_free(section);
		mir_free(val);
	}
	return 0;
}

static char *_skipblank(char * str) //str will be modified;
{
	char * endstr = str+strlen(str);
	while ((*str == ' ' || *str == '\t') && *str != '\0') str++;
	while ((*endstr == ' ' || *endstr == '\t') && *endstr != '\0' && endstr < str) endstr--;
	if (*endstr != '\0')
	{
		endstr++;
		*endstr = '\0';
	}
	return str;
}

static int _CallServiceStrParams(IN char * toParce, OUT int *Return)
{
	char * pszService;
	char * param1 = NULL;
	char * param2 = NULL;
	int paramCount = 0;
	int result = 0;

	pszService = mir_strdup(toParce);
	if (!pszService)
		return 0;
	if (strlen(pszService) == 0) {
		mir_free(pszService);
		return 0;
	}
	param2 = strrchr(pszService, '%');
	if (param2)
	{
		paramCount++;
		*param2 = '\0';	param2++;
		_skipblank(param2);
		if (strlen(param2) == 0)
			param2 = NULL;
	}
	param1 = strrchr(pszService, '%');
	if (param1)
	{
		paramCount++;
		*param1 = '\0';	param1++;
		_skipblank(param1);
		if (strlen(param1) == 0)
			param1 = NULL;
	}
	if (param1 && *param1 == '\"')
	{
		param1++;
		*(param1+strlen(param1)) = '\0';
	}
	else if (param1)
	{
		param1 = (char*)atoi(param1);
	}
	if (param2 && *param2 == '\"')
	{
		param2++;
		*(param2+strlen(param2)) = '\0';
	}
	else if (param2)
		param2 = (char*)atoi(param2);

	if (paramCount == 1)
	{
		param1 = param2;
		param2 = NULL;
	}
	if (!ServiceExists(pszService))
	{
		result = 0;
	}
	else
	{
		int ret = 0;
		result = 1;
		ret = CallService(pszService, (WPARAM)param1, (WPARAM)param2);
		if (Return) *Return = ret;
	}
	mir_free(pszService);
	return result;
}


static LRESULT CALLBACK ModernSkinButtonWndProc(HWND hwndDlg, UINT msg,  WPARAM wParam, LPARAM lParam)
{
	ModernSkinButtonCtrl* bct = (msg != WM_NCCREATE)?(ModernSkinButtonCtrl *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA):0;
	if (bct && bct->HandleService && IsBadStringPtrA(bct->HandleService,255))
		bct->HandleService = NULL;

	if (bct)
		if (bct->HandleService && ServiceExists(bct->HandleService)) {
			HandleServiceParams MSG = {0};
			MSG.hwnd = hwndDlg;
			MSG.msg = msg;
			MSG.wParam = wParam;
			MSG.lParam = lParam;
			int t = CallService(bct->HandleService,(WPARAM)&MSG,0);
			if (MSG.handled) return t;
		}

	switch(msg) {
	case WM_NCCREATE:
		SetWindowLongPtr(hwndDlg, GWL_STYLE, GetWindowLongPtr(hwndDlg, GWL_STYLE)|BS_OWNERDRAW);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		if (((CREATESTRUCT *)lParam)->lpszName) SetWindowText(hwndDlg, ((CREATESTRUCT *)lParam)->lpszName);
		return TRUE;

	case WM_DESTROY:
		if (bct == NULL)
			break;
		EnterCriticalSection(&csTips);
		if (hwndToolTips) {
			TOOLINFO ti;
			ZeroMemory(&ti, sizeof(ti));
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND;
			ti.hwnd = bct->hwnd;
			ti.uId = (UINT_PTR)bct->hwnd;
			if (SendMessage(hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti)) {
				SendMessage(hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);
			}
			if (SendMessage(hwndToolTips, TTM_GETTOOLCOUNT, 0, (LPARAM)&ti) == 0) {
				DestroyWindow(hwndToolTips);
				hwndToolTips = NULL;
			}
		}
		LeaveCriticalSection(&csTips);
		mir_free(bct->ID);
		mir_free(bct->CommandService);
		mir_free(bct->StateService);
		mir_free(bct->HandleService);
		mir_free(bct->Hint);
		mir_free(bct->ValueDBSection);
		mir_free(bct->ValueTypeDef);
		mir_free(bct);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;	// DONT! fall thru

	case WM_SETCURSOR:
		{
			HCURSOR hCurs1 = LoadCursor(NULL, IDC_ARROW);
			if (hCurs1) SetCursor(hCurs1);
			if (bct) SetToolTip(hwndDlg, bct->Hint);
		}
		return 1;

	case WM_PRINT:
		if (IsWindowVisible(hwndDlg))
			ModernSkinButtonPaintWorker(hwndDlg,(HDC)wParam);
		break;

	case WM_PAINT:
		if (IsWindowVisible(hwndDlg) && !g_CluiData.fLayered) {
			PAINTSTRUCT ps = {0};
			BeginPaint(hwndDlg,&ps);
			ModernSkinButtonPaintWorker(hwndDlg,(HDC)ps.hdc);
			EndPaint(hwndDlg,&ps);
		}
		return DefWindowProc(hwndDlg, msg, wParam, lParam);

	case WM_CAPTURECHANGED:
		bct->hover = 0;
		bct->down = 0;
		ModernSkinButtonPaintWorker(bct->hwnd,0);
		break;

	case WM_MOUSEMOVE:
		if (!bct->hover) {
			SetCapture(bct->hwnd);
			bct->hover = 1;
			ModernSkinButtonPaintWorker(bct->hwnd,0);
		}
		else {
			POINT t = UNPACK_POINT(lParam);
			ClientToScreen(bct->hwnd,&t);
			if (WindowFromPoint(t) != bct->hwnd)
				ReleaseCapture();
		}
		return 0;

	case WM_LBUTTONDOWN:
		bct->down = 1;
		SetForegroundWindow(GetParent(bct->hwnd));
		ModernSkinButtonPaintWorker(bct->hwnd,0);
		if (bct->CommandService && IsBadStringPtrA(bct->CommandService,255))
			bct->CommandService = NULL;
		if (bct->fCallOnPress) {
			if (bct->CommandService) {
				if (!_CallServiceStrParams(bct->CommandService, NULL) &&   (bct->ValueDBSection && bct->ValueTypeDef))
						ModernSkinButtonToggleDBValue(bct->ValueDBSection,bct->ValueTypeDef);
			}
			bct->down = 0;

			ModernSkinButtonPaintWorker(bct->hwnd,0);
		}
		return 0;

	case WM_LBUTTONUP:
		if (bct->down) {
			ReleaseCapture();
			bct->hover = 0;
			bct->down = 0;
			ModernSkinButtonPaintWorker(bct->hwnd,0);
			if (bct->CommandService && IsBadStringPtrA(bct->CommandService,255))
				bct->CommandService = NULL;
			if (bct->CommandService)
				if (_CallServiceStrParams(bct->CommandService, NULL))
				{}
				else if (bct->ValueDBSection && bct->ValueTypeDef)
					ModernSkinButtonToggleDBValue(bct->ValueDBSection,bct->ValueTypeDef);
		}
	}
	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}

HWND SetToolTip(HWND hwnd, TCHAR * tip)
{
	TOOLINFO ti;
	if (!tip) return 0;
	EnterCriticalSection(&csTips);
	if (!hwndToolTips) {
		//  hwndToolTips = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

		hwndToolTips = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			hwnd, NULL, GetModuleHandle(NULL),
			NULL);

		SetWindowPos(hwndToolTips, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_IDISHWND;
	ti.hwnd = hwnd;
	ti.uId = (UINT_PTR)hwnd;
	if (SendMessage(hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti)) {
		SendMessage(hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);
	}
	ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
	ti.uId = (UINT_PTR)hwnd;
	ti.lpszText = (TCHAR*)tip;
	SendMessage(hwndToolTips,TTM_ADDTOOL, 0, (LPARAM)&ti);

	LeaveCriticalSection(&csTips);
	return hwndToolTips;
}



typedef struct _MButton
{
	HWND hwnd;
	BYTE    ConstrainPositionFrom;  //(BBRRTTLL)  L = 0 - from left, L = 1 from right, L = 2 from center
	int OrL,OrR,OrT,OrB;
	int minW,minH;
	ModernSkinButtonCtrl * bct;

} MButton;
MButton * Buttons = NULL;
DWORD ButtonsCount = 0;

#define _center_h( rc ) (((rc)->right + (rc)->left ) >> 1)
#define _center_v( rc ) (((rc)->bottom + (rc)->top ) >> 1)

int ModernSkinButton_AddButton(HWND parent,
							   char * ID,
							   char * CommandService,
							   char * StateDefService,
							   char * HandeService,
							   int Left,
							   int Top,
							   int Right,
							   int Bottom,
							   DWORD sbFlags,
							   TCHAR * Hint,
							   char * DBkey,
							   char * TypeDef,
							   int MinWidth, int MinHeight)
{
	//  if (!parent) return 0;
	if (!ModernSkinButtonModuleIsLoaded) return 0;
	if (!Buttons)
		Buttons = (MButton*)mir_alloc(sizeof(MButton));
	else
		Buttons = (MButton*)mir_realloc(Buttons,sizeof(MButton)*(ButtonsCount+1));
	{
		//HWND hwnd;
		RECT rc = {0};
		ModernSkinButtonCtrl* bct;
		int l,r,b,t;
		if (parent) GetClientRect(parent,&rc);
		l = ( sbFlags & SBF_ALIGN_TL_RIGHT )   ? ( rc.right + Left ) :
          ( sbFlags & SBF_ALIGN_TL_HCENTER ) ? ( _center_h( &rc ) + Left ) :
          ( rc.left + Left );

        t = ( sbFlags & SBF_ALIGN_TL_BOTTOM )  ? ( rc.bottom + Top ) :
          ( sbFlags & SBF_ALIGN_TL_VCENTER ) ? ( _center_v( &rc ) + Top ) :
          ( rc.top+Top );

        r = ( sbFlags & SBF_ALIGN_BR_RIGHT )   ? ( rc.right + Right ) :
          ( sbFlags & SBF_ALIGN_BR_HCENTER ) ? ( _center_h( &rc) + Right ) :
          ( rc.left + Right );

        b = ( sbFlags & SBF_ALIGN_BR_BOTTOM )  ? ( rc.bottom + Bottom ) :
          ( sbFlags & SBF_ALIGN_BR_VCENTER ) ? ( _center_v( &rc ) + Bottom ) :
          ( rc.top + Bottom );
		bct = (ModernSkinButtonCtrl *)mir_alloc(sizeof(ModernSkinButtonCtrl));
		memset(bct, 0, sizeof(ModernSkinButtonCtrl));
		bct->Left = l;
		bct->Right = r;
		bct->Top = t;
		bct->Bottom = b;
		bct->fCallOnPress = ( sbFlags & SBF_CALL_ON_PRESS ) != 0;
		bct->HandleService = mir_strdup(HandeService);
		bct->CommandService = mir_strdup(CommandService);
		bct->StateService = mir_strdup(StateDefService);
		if (DBkey && &DBkey != '\0') bct->ValueDBSection = mir_strdup(DBkey); else bct->ValueDBSection = NULL;
		if (TypeDef && &TypeDef != '\0') bct->ValueTypeDef = mir_strdup(TypeDef); else bct->ValueTypeDef = mir_strdup("sDefault");
		bct->ID=mir_strdup(ID);
		bct->Hint = mir_tstrdup(Hint);
		Buttons[ButtonsCount].bct = bct;
		Buttons[ButtonsCount].hwnd = NULL;
		Buttons[ButtonsCount].OrL = Left;
		Buttons[ButtonsCount].OrT = Top;
		Buttons[ButtonsCount].OrR = Right;
		Buttons[ButtonsCount].OrB = Bottom;
		Buttons[ButtonsCount].ConstrainPositionFrom = (BYTE)sbFlags;
		Buttons[ButtonsCount].minH = MinHeight;
		Buttons[ButtonsCount].minW = MinWidth;
		ButtonsCount++;
		//  CLUI_ShowWindowMod(hwnd,SW_SHOW);
	}
	return 0;
}



static int ModernSkinButtonErase(int l,int t,int r, int b)
{
	DWORD i;
	if (!ModernSkinButtonModuleIsLoaded) return 0;
	if (!g_CluiData.fLayered) return 0;
	if (!g_pCachedWindow) return 0;
	if (!g_pCachedWindow->hImageDC  || !g_pCachedWindow->hBackDC) return 0;
	if (!(l || r || t || b))
	{
		for (i=0; i < ButtonsCount; i++)
		{
			if (pcli->hwndContactList && Buttons[i].hwnd != NULL)
			{
				//TODO: Erase button
				BitBlt(g_pCachedWindow->hImageDC,Buttons[i].bct->Left,Buttons[i].bct->Top,Buttons[i].bct->Right-Buttons[i].bct->Left,Buttons[i].bct->Bottom-Buttons[i].bct->Top,
					g_pCachedWindow->hBackDC,Buttons[i].bct->Left,Buttons[i].bct->Top,SRCCOPY);
			}
		}
	}
	else
	{
		BitBlt(g_pCachedWindow->hImageDC,l,t,r-l,b-t, g_pCachedWindow->hBackDC,l,t,SRCCOPY);
	}
	return 0;
}

static HWND ModernSkinButtonCreateWindow(ModernSkinButtonCtrl * bct, HWND parent)
{
	HWND hwnd;

	if (bct == NULL) return FALSE;
	{
		TCHAR *UnicodeID;
		UnicodeID = mir_a2u(bct->ID);
		hwnd = CreateWindow(_T(MODERNSKINBUTTONCLASS),UnicodeID,WS_VISIBLE|WS_CHILD,bct->Left,bct->Top,bct->Right-bct->Left,bct->Bottom-bct->Top,parent,NULL,g_hInst,NULL);
		mir_free(UnicodeID);
	}

	bct->hwnd = hwnd;
	bct->focus = 0;
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)bct);
	return hwnd;
}

int ModernSkinButtonRedrawAll(HDC hdc)
{
	DWORD i;
	if (!ModernSkinButtonModuleIsLoaded) return 0;
	g_mutex_bLockUpdating++;
	for (i=0; i < ButtonsCount; i++)
	{
		if (pcli->hwndContactList && Buttons[i].hwnd == NULL)
			Buttons[i].hwnd = ModernSkinButtonCreateWindow(Buttons[i].bct,pcli->hwndContactList);
		ModernSkinButtonPaintWorker(Buttons[i].hwnd,0);
	}
	g_mutex_bLockUpdating--;
	return 0;
}

int ModernSkinButtonDeleteAll()
{
	if (!ModernSkinButtonModuleIsLoaded)
		return 0;

	for (size_t i=0; i < ButtonsCount; i++)
		if (Buttons[i].hwnd)
			DestroyWindow(Buttons[i].hwnd);

	mir_free_and_nil(Buttons);
	ButtonsCount = 0;
	return 0;
}

int ModernSkinButton_ReposButtons(HWND parent, BYTE draw, RECT *r)
{
	DWORD i;
	RECT rc;
	RECT clr;
	RECT rd;
	BOOL altDraw = FALSE;
	static SIZE oldWndSize = {0};
	if (!ModernSkinButtonModuleIsLoaded) return 0;
	GetWindowRect(parent,&rd);
	GetClientRect(parent,&clr);
	if (!r)
		GetWindowRect(parent,&rc);
	else
		rc = *r;
	if (g_CluiData.fLayered && ( draw & SBRF_DO_ALT_DRAW ))
	{
		int sx,sy;
		sx = rd.right-rd.left;
		sy = rd.bottom-rd.top;
		if (sx != oldWndSize.cx || sy != oldWndSize.cy)
			altDraw = TRUE;//EraseButtons();
		oldWndSize.cx = sx;
		oldWndSize.cy = sy;
	}

	OffsetRect(&rc,-rc.left,-rc.top);
	rc.right = rc.left+(clr.right-clr.left);
	rc.bottom = rc.top+(clr.bottom-clr.top);
	for (i=0; i < ButtonsCount; i++)
	{
		int l,r,b,t;
		RECT oldRect = {0};
		int sbFlags = Buttons[i].ConstrainPositionFrom;
		if (parent && Buttons[i].hwnd == NULL)
		{
			Buttons[i].hwnd = ModernSkinButtonCreateWindow(Buttons[i].bct,parent);
			altDraw = FALSE;
		}

        l = ( sbFlags & SBF_ALIGN_TL_RIGHT )   ? ( rc.right + Buttons[i].OrL ) :
            ( sbFlags & SBF_ALIGN_TL_HCENTER ) ? ( _center_h( &rc ) + Buttons[i].OrL ) :
            ( rc.left + Buttons[i].OrL );

        t = ( sbFlags & SBF_ALIGN_TL_BOTTOM )  ? ( rc.bottom + Buttons[i].OrT ) :
            ( sbFlags & SBF_ALIGN_TL_VCENTER ) ? ( _center_v( &rc ) + Buttons[i].OrT ) :
            ( rc.top + Buttons[i].OrT );

        r = ( sbFlags & SBF_ALIGN_BR_RIGHT )   ? ( rc.right + Buttons[i].OrR ) :
            ( sbFlags & SBF_ALIGN_BR_HCENTER ) ? ( _center_h( &rc) + Buttons[i].OrR ) :
            ( rc.left + Buttons[i].OrR );

        b = ( sbFlags & SBF_ALIGN_BR_BOTTOM )  ? ( rc.bottom + Buttons[i].OrB ) :
            ( sbFlags & SBF_ALIGN_BR_VCENTER ) ? ( _center_v( &rc ) + Buttons[i].OrB ) :
            ( rc.top + Buttons[i].OrB );

        SetWindowPos(Buttons[i].hwnd,HWND_TOP,l,t,r-l,b-t,0);
		if (  (rc.right-rc.left < Buttons[i].minW /* &&  Buttons[i].minW != 0*/)
			 || (rc.bottom-rc.top < Buttons[i].minH /* &&  Buttons[i].minH != 0*/))
			CLUI_ShowWindowMod(Buttons[i].hwnd,SW_HIDE);
		else
			CLUI_ShowWindowMod(Buttons[i].hwnd,SW_SHOW);
		if ((1 || altDraw) &&
			(Buttons[i].bct->Left != l  ||
			Buttons[i].bct->Top != t   ||
			Buttons[i].bct->Right != r ||
			Buttons[i].bct->Bottom != b))
		{
			//Need to erase in old location
			ModernSkinButtonErase(Buttons[i].bct->Left,Buttons[i].bct->Top,Buttons[i].bct->Right,Buttons[i].bct->Bottom);
		}

		Buttons[i].bct->Left = l;
		Buttons[i].bct->Top = t;
		Buttons[i].bct->Right = r;
		Buttons[i].bct->Bottom = b;


	}
	if ( draw & SBRF_DO_REDRAW_ALL ) ModernSkinButtonRedrawAll(0);
	return 0;
}