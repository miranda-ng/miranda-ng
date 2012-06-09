
#include "common.h"
#pragma hdrstop

HWND hwndContactList = 0;
HWND hwndTopToolBar = 0;
bool StopArrange;

int BUTTWIDTH = 20;
int BUTTHEIGHT = 16;
bool UseIcoLib = false;

int nextButtonId = 200;
int nButtonsCount = 0;
int lastxpos = 1;
int lastypos = 1;

// ==  == ==  == ==  == == options
COLORREF bkColour;
HBITMAP hBmpBackground;
int backgroundBmpUse;

static CRITICAL_SECTION csButtonsHook;
static int hFrameTopWindow = -1;

TopButtonInt Buttons[MAX_BUTTONS];
SortData arrangedbuts[MAX_BUTTONS];

int SetButtBitmap(int pos);

int applyuserbitmaps(int pos)
{
	int t = IMAGE_BITMAP;
	Buttons[pos].hbBitmapDown = Buttons[pos].hbDefBitmapDown;
	Buttons[pos].hbBitmapUp = Buttons[pos].hbDefBitmapUp;
	if (Buttons[pos].UserDefinedbmDown != NULL) {
		if ( _tcsstr(Buttons[pos].UserDefinedbmDown, _T(".ico"))) 
			t = IMAGE_ICON;
		Buttons[pos].hbBitmapDown = ( HBITMAP )LoadImage(0, Buttons[pos].UserDefinedbmDown, t, 0, 0, LR_LOADFROMFILE);	
	}

	if (Buttons[pos].UserDefinedbmUp != NULL) {
		if ( _tcsstr(Buttons[pos].UserDefinedbmUp, _T(".ico")))
			t = IMAGE_ICON;
		Buttons[pos].hbBitmapUp = ( HBITMAP )LoadImage(0, Buttons[pos].UserDefinedbmUp, t, 0, 0, LR_LOADFROMFILE);
	}

	return 0;
}

int SetAllBitmaps()
{
	lockbut();
	for (int i = 0; i < nButtonsCount; i++) {
		applyuserbitmaps(i);
		SetButtBitmap(i);
	}
	ulockbut();
	return 0;
}

void lockbut()
{
	EnterCriticalSection(&csButtonsHook);
}

void ulockbut()
{
	LeaveCriticalSection(&csButtonsHook);
}

int idtopos(int id)
{
	for ( int i = 0;i<nButtonsCount;i++)
		if (Buttons[i].id == id)
			return i;

	return -1;
}

int DBSaveButtonSettings(int butpos)
{
	char buf[255];
	memset(buf, 0, SIZEOF(buf));	

	DBWriteContactSettingWord(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_Position"), Buttons[butpos].arrangedpos);

	DWORD oldv = Buttons[butpos].dwFlags & TTBBF_VISIBLE;
	DBWriteContactSettingDword(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_Visible"), oldv);

	DBWriteContactSettingString(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_BmpDown"), "");
	DBWriteContactSettingString(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_BmpUp"), "");
	
	if (Buttons[butpos].UserDefinedbmDown != NULL)
		DBWriteContactSettingTString(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_BmpDown"), Buttons[butpos].UserDefinedbmDown);

	if (Buttons[butpos].UserDefinedbmUp != NULL)
		DBWriteContactSettingTString(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_BmpUp"), Buttons[butpos].UserDefinedbmUp);

	return 0;
}

int SaveAllButtonsOptions()
{
	lockbut();
	for (int i = 0; i < nButtonsCount; i++)
		DBSaveButtonSettings(i);

	ulockbut();
	return 0;
}

int DBLoadButtonSettings(int butpos)
{
	char buf[255];
	memset(buf, 0, SIZEOF(buf));	

	//bool
	Buttons[butpos].arrangedpos = DBGetContactSettingWord(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_Position"), MAX_BUTTONS);
	DWORD oldv = Buttons[butpos].dwFlags & TTBBF_VISIBLE;
	Buttons[butpos].dwFlags = Buttons[butpos].dwFlags & (~TTBBF_VISIBLE);
	if ( DBGetContactSettingDword(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_Visible"), oldv) > 0 )
		Buttons[butpos].dwFlags |= TTBBF_VISIBLE;

	Buttons[butpos].UserDefinedbmDown = DBGetStringT(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_BmpDown"));
	if ( Buttons[butpos].UserDefinedbmDown != NULL && Buttons[butpos].UserDefinedbmDown[0] == 0) {
		mir_free(Buttons[butpos].UserDefinedbmDown);
		Buttons[butpos].UserDefinedbmDown = NULL;
	}

	Buttons[butpos].UserDefinedbmUp = DBGetStringT(0, TTB_OPTDIR, AS(buf, Buttons[butpos].name, "_BmpUp"));
	if ( Buttons[butpos].UserDefinedbmUp != NULL && Buttons[butpos].UserDefinedbmUp[0] == 0) { 
		mir_free(Buttons[butpos].UserDefinedbmUp);
		Buttons[butpos].UserDefinedbmUp = NULL;
	}
	return 0;
}

int ttbOptionsChanged()
{
	//load options
	if (TRUE) {
		DBVARIANT dbv;
		bkColour = DBGetContactSettingDword(NULL, TTB_OPTDIR, "BkColour", TTBDEFAULT_BKCOLOUR);
		if (hBmpBackground) {DeleteObject(hBmpBackground); hBmpBackground = NULL;}
		if (DBGetContactSettingByte(NULL, TTB_OPTDIR, "UseBitmap", TTBDEFAULT_USEBITMAP)) {
			if (!DBGetContactSetting(NULL, TTB_OPTDIR, "BkBitmap", &dbv)) {
				hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
				DBFreeVariant(&dbv);
			}
		}
		backgroundBmpUse = DBGetContactSettingWord(NULL, TTB_OPTDIR, "BkBmpUse", TTBDEFAULT_BKBMPUSE);
	}

	RECT rc;
	GetClientRect(hwndTopToolBar, &rc);
	InvalidateRect(hwndTopToolBar, &rc, TRUE);
	UpdateWindow(hwndTopToolBar);

	ArrangeButtons();
	SetAllBitmaps();
	SaveAllSeparators();
	SaveAllButtonsOptions();

	return 0;
}

static int RemoveItemFromList(int pos, TopButtonInt *lpButtons, int *ButtonsItemCount)
{
	memcpy( &lpButtons[pos], &lpButtons[pos+1], sizeof(TopButtonInt)*(*ButtonsItemCount-pos-1));
	*ButtonsItemCount--;
	return 0;
}

INT_PTR TTBRemoveButton(WPARAM wParam, LPARAM lParam)
{
	int pos;

	lockbut();
	pos = idtopos(wParam);
	if (pos<0 || pos >= nButtonsCount){ulockbut();return -1;}
	
	DestroyWindow(Buttons[pos].hwnd);
	if (Buttons[pos].pszServiceDown != NULL)
		free(Buttons[pos].pszServiceDown);
	if (Buttons[pos].pszServiceUp != NULL)
		free(Buttons[pos].pszServiceUp);
	RemoveItemFromList(pos, Buttons, &nButtonsCount);
	ArrangeButtons();
	ulockbut();
	OptionsPageRebuild();
	return 0;

}
static HBITMAP DrawBorderForBitmap(HBITMAP hb, BOOL up)
{
	HBITMAP Border, workbmp;
	HDC workdc, destdc, srcdc;

	Border = LoadBitmap(hInst, MAKEINTRESOURCE(up?IDB_BORDERUP:IDB_BORDERDN));
	workdc = GetDC(hwndContactList);
	destdc = CreateCompatibleDC(workdc);
	srcdc = CreateCompatibleDC(workdc);

	workbmp = CreateBitmap(BUTTWIDTH, BUTTHEIGHT, 1, GetDeviceCaps(workdc, BITSPIXEL), NULL);
	SelectObject(destdc, workbmp);

	SelectObject(srcdc, Border);

	BitBlt(destdc, 0, 0, BUTTWIDTH, BUTTHEIGHT, srcdc, 0, 0, SRCCOPY);
	SelectObject(srcdc, hb);
	BitBlt(destdc, 1, 1, BUTTWIDTH-4, BUTTHEIGHT-4, srcdc, 0, 0, SRCCOPY);
			
	DeleteDC(destdc);
	DeleteDC(srcdc);
	ReleaseDC(hwndContactList, workdc);
	return workbmp;
}

static int UpdateToolTip(int bpos)
{
	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(ti);
	ti.lpszText = Buttons[bpos].tooltip;
	ti.hinst = hInst;
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS ;
	ti.uId = (UINT_PTR)Buttons[bpos].hwnd;
	return SendMessage(Buttons[bpos].hwndTip, TTM_UPDATETIPTEXT , 0, (LPARAM)&ti);
}

bool nameexists(const char *name)
{
	for (int i = 0; i < nButtonsCount; i++)
		if (strcmp(Buttons[i].name, name) == 0)
			return TRUE;

	return FALSE;
}

HICON LoadIconFromLibrary(char *SectName, char *Name, char *Description, HICON hIcon, bool RegisterIt, bool *RegistredOk)
{		
	if (RegistredOk)
		*RegistredOk = FALSE;

	if (Name != NULL && *Name != 0) {				
		char iconame[256];
		_snprintf(iconame, SIZEOF(iconame), "toptoolbar_%s", Name);
		if ( ServiceExists(MS_SKIN2_ADDICON)) {
			if (RegisterIt) {
				SKINICONDESC sid = {0};
				sid.cbSize = sizeof(sid);
				sid.pszSection = Translate(SectName);				
				sid.pszName = iconame;
				sid.pszDefaultFile = NULL;
				sid.pszDescription = Description;
				sid.hDefaultIcon = hIcon;
				CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

				if (RegistredOk)
					*RegistredOk = TRUE;
			}
			return ((HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)iconame));
		}
	}

	return hIcon;
}

int CreateOneWindow(int ButtonPos)
{
	if (DBGetContactSettingByte(0, TTB_OPTDIR, "UseMirandaButtonClass", UseMirandaButtonClassDefaultValue) && !(Buttons[ButtonPos].dwFlags & TTBBF_ISSEPARATOR))
		Buttons[ButtonPos].hwnd = CreateWindow(MYMIRANDABUTTONCLASS, _T(""), BS_PUSHBUTTON|WS_CHILD|WS_TABSTOP|SS_NOTIFY, 0, 0, BUTTWIDTH, BUTTHEIGHT, hwndTopToolBar, NULL, hInst, 0);
	else 
		Buttons[ButtonPos].hwnd = CreateWindow( _T("STATIC"), _T(""), WS_CHILD|SS_NOTIFY, 0, 0, BUTTWIDTH, BUTTHEIGHT, hwndTopToolBar, NULL, hInst, 0);

	SetWindowLongPtr(Buttons[ButtonPos].hwnd, GWLP_USERDATA, Buttons[ButtonPos].id);
	if (DBGetContactSettingByte(0, TTB_OPTDIR, "UseFlatButton", 1))
		SendMessage(Buttons[ButtonPos].hwnd, BUTTONSETASFLATBTN, 0, 0);

	return 0;
}

INT_PTR TTBAddButton(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
		return -1;

	if (hwndContactList == 0)
		hwndContactList = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	//oops clui even now not loaded...sorry no buttons available
	if (hwndContactList == 0)
		return -1;

	lockbut();
	if (nButtonsCount == MAX_BUTTONS)
		return -1;

	int i = nButtonsCount;
	TTBButtonV2 but = { 0 };
	if (((TTBButton*)wParam)->cbSize == sizeof(TTBButton))
		memcpy(&but, (void*)wParam, sizeof(TTBButton));

	if (((TTBButton*)wParam)->cbSize == sizeof(TTBButtonV2))
		memcpy(&but, (void*)wParam, sizeof(TTBButtonV2));

	if ( but.name == NULL || nameexists(but.name)) {
		ulockbut();
		return -1;
	}
	
	Buttons[i].id = nextButtonId++;
	Buttons[i].hbBitmapDown = but.hbBitmapDown;
	Buttons[i].hbBitmapUp = but.hbBitmapUp;
	Buttons[i].hbDefBitmapDown = but.hbBitmapDown;
	Buttons[i].hbDefBitmapUp  = but.hbBitmapUp;
	
	if (but.pszServiceDown != NULL) Buttons[i].pszServiceDown = strdup(but.pszServiceDown);
	if (but.pszServiceUp != NULL) Buttons[i].pszServiceUp = strdup(but.pszServiceUp);
	Buttons[i].name = strdup(but.name);
	Buttons[i].dwFlags = but.dwFlags;

	if (Buttons[i].dwFlags & TTBBF_DRAWBORDER) {
		Buttons[i].hbWBordBitmapDown = DrawBorderForBitmap(Buttons[i].hbBitmapDown, FALSE);
		Buttons[i].hbWBordBitmapUp = DrawBorderForBitmap(Buttons[i].hbBitmapUp, TRUE);
	}

	Buttons[i].wParamUp = but.wParamUp;
	Buttons[i].lParamUp = but.lParamUp;
	Buttons[i].wParamDown = but.wParamDown;
	Buttons[i].lParamDown = but.lParamDown;

	Buttons[i].dwFlags = but.dwFlags;

	Buttons[i].bPushed = (but.dwFlags&TTBBF_PUSHED)?TRUE:FALSE;

	CreateOneWindow(i);

	Buttons[i].hIconUp  = but.hIconUp ;
	Buttons[i].hIconDn  = but.hIconDn ;
	
	if (UseIcoLib && (!(Buttons[i].dwFlags & TTBBF_ISSEPARATOR))) {
		char buf[256];
		sprintf(buf, "%s_up", Buttons[i].name);
		Buttons[i].hIconUp = LoadIconFromLibrary("TopToolBar", buf, buf, but.hIconDn, TRUE, NULL);
		sprintf(buf, "%s_dn", Buttons[i].name);
		Buttons[i].hIconDn = LoadIconFromLibrary("TopToolBar", buf, buf, but.hIconUp, TRUE, NULL);
	}

	Buttons[i].hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, 
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
		CW_USEDEFAULT, CW_USEDEFAULT, 
		CW_USEDEFAULT, CW_USEDEFAULT, 
		hwndTopToolBar, NULL, hInst, 
		NULL);

	SetWindowPos(Buttons[i].hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(ti);
	ti.lpszText = _T("");
	ti.hinst = hInst;
	ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS ;
	ti.uId = (UINT_PTR)Buttons[i].hwnd;
	SendMessage(Buttons[i].hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

	SendMessage(Buttons[i].hwndTip, TTM_ACTIVATE, (WPARAM)(Buttons[i].dwFlags&TTBBF_SHOWTOOLTIP)?TRUE:FALSE, 0);

	SetWindowLongPtr(Buttons[i].hwnd, GWLP_USERDATA, Buttons[i].id);

	nButtonsCount++;

	int retval = Buttons[i].id;
	DBLoadButtonSettings(i);
	applyuserbitmaps(i);

	SetButtBitmap(i);
	ulockbut();
	ArrangeButtons();
	OptionsPageRebuild();
	return retval;
}

int sortfunc(const void *a, const void *b)
{
	SortData *sd1 = ( SortData* )a, *sd2 = ( SortData* )b;

	if (sd1->arrangeval > sd2->arrangeval)
		return 1;
	if (sd1->arrangeval < sd2->arrangeval)
		return -1;
	return 0;
}

bool isSep(int i)
{
	if (Buttons[i].dwFlags & TTBBF_ISSEPARATOR) 
		return TRUE;
	return FALSE;
}

int getbutw(int i)
{
	RECT rc;
	GetWindowRect(Buttons[i].hwnd, &rc);
	return rc.right-rc.left;
}

int MakeSortArr(bool vis)
{
	int v = 0;
	memset(&arrangedbuts, 0, sizeof(arrangedbuts));

	for (int i = 0; i < nButtonsCount; i++) {
		if (vis && !(Buttons[i].dwFlags & TTBBF_VISIBLE))
			continue;

		arrangedbuts[v].arrangeval = Buttons[i].arrangedpos;
		arrangedbuts[v].oldpos = i;
		v++;
	}	

	if ( v > 0 )
		qsort( arrangedbuts, v, sizeof(SortData), sortfunc);

	return v;
}

int RecreateWindows()
{
	lockbut();
	for (int i = 0; i < nButtonsCount; i++) {
		if (Buttons[i].hwnd) {
			DestroyWindow(Buttons[i].hwnd);
			Buttons[i].hwnd = NULL;
			CreateOneWindow(i);
		}
	}
	
	ulockbut();
	SetAllBitmaps();
	return (0);
}

int ArrangeButtons()
{
	int i;
	int newheight, cnt, perline;

	if (StopArrange == TRUE)
		return 0;

	BUTTHEIGHT = DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTHEIGHT", 16);
	BUTTWIDTH = DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTWIDTH", 20);

	lockbut();
	lastxpos = 1;

	int v = MakeSortArr(TRUE);
	if ( v > 0) {
		for (i = 0; i < nButtonsCount; i++)
			if ( !(Buttons[i].dwFlags & TTBBF_VISIBLE))
				ShowWindow(Buttons[i].hwnd, SW_HIDE);

		RECT winrc;
		GetClientRect(hwndTopToolBar, &winrc);
		winrc.left = winrc.right-winrc.left;
		winrc.top = winrc.bottom-winrc.top;

		int totwidth = 1;
		int visbut = 0;
		for (i = 0; i < v; i++ ) {
			if (!(Buttons[arrangedbuts[i].oldpos].dwFlags&TTBBF_VISIBLE)) continue;
			totwidth+= BUTTWIDTH+1;
			visbut++;
		}

		if (winrc.left == 0)
			return 0;

		perline = winrc.left/(BUTTWIDTH+1);
		if (perline == 0)
			perline = 1;

		cnt = (visbut)/perline;
		if (cnt*perline < visbut)
			cnt++;
		newheight = 1+((cnt))*(BUTTHEIGHT+2);

		StopArrange = true;
		lastypos = 1;
		lastxpos = 1;
		newheight = BUTTHEIGHT+1;
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, hFrameTopWindow), BUTTHEIGHT+2);
		for (i = 0; i < v; i++) {
			int w1, w2, x;
			if (!(Buttons[arrangedbuts[i].oldpos].dwFlags & TTBBF_VISIBLE)){
				ShowWindow(Buttons[arrangedbuts[i].oldpos].hwnd, SW_HIDE);
				continue;
			}

			//	TTBBF_ISSEPARATOR
			x = SWP_NOZORDER | SWP_SHOWWINDOW;
			if ( Buttons[arrangedbuts[i].oldpos].dwFlags & TTBBF_ISSEPARATOR)
				x = SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW;

			SetWindowPos(Buttons[arrangedbuts[i].oldpos].hwnd, 0, lastxpos, lastypos, BUTTWIDTH, BUTTHEIGHT, x);
			InvalidateRect(Buttons[arrangedbuts[i].oldpos].hwnd, NULL, TRUE);

			//lastxpos += BUTTWIDTH+1;
			if (i == v-1)
				break;
			if ( !(Buttons[arrangedbuts[i+1].oldpos].dwFlags & TTBBF_VISIBLE))
				continue;

			w1 = getbutw(arrangedbuts[i+1].oldpos);
			w2 = getbutw(arrangedbuts[i].oldpos);
			if (lastxpos+w1+w2+1 > winrc.left) {
				lastxpos = 1;		
				lastypos += BUTTHEIGHT+2;
				if (Buttons[arrangedbuts[i+1].oldpos].dwFlags & TTBBF_VISIBLE) {
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, hFrameTopWindow), lastypos+BUTTHEIGHT+1);
					newheight = lastypos+BUTTHEIGHT+1;
				}
			}
			else {
				lastxpos += (isSep(arrangedbuts[i].oldpos)) ? SEPWIDTH+2 : BUTTWIDTH+1;
				if ( isSep(arrangedbuts[i+1].oldpos))
					lastxpos++;
			}
		}
		StopArrange = false;
	}

	v = MakeSortArr(FALSE);
	ulockbut();
	return 1;
}

int SetButtBitmap(int pos)
{
	int t = IMAGE_BITMAP;
	TCHAR *curname = (Buttons[pos].bPushed)?(Buttons[pos].UserDefinedbmDown):(Buttons[pos].UserDefinedbmUp);

	int curstyle = GetWindowLongPtr(Buttons[pos].hwnd, GWL_STYLE);
	curstyle &= (~SS_BITMAP);
	curstyle &= (~SS_ICON);

	if (!UseIcoLib || (Buttons[pos].dwFlags&TTBBF_ISSEPARATOR)) {
		if (curname != NULL) {
			if ( _tcsstr(curname, _T(".ico"))) 
				t = IMAGE_ICON;

			curstyle |= (t == IMAGE_ICON || UseIcoLib) ? SS_ICON : SS_BITMAP;
			SetWindowLongPtr(Buttons[pos].hwnd, GWL_STYLE, curstyle);
		}
		else SetWindowLongPtr(Buttons[pos].hwnd, GWL_STYLE, curstyle|(Buttons[pos].hbBitmapDown == NULL?SS_ICON:SS_BITMAP));
	}
	else if (GetWindowLongPtr(Buttons[pos].hwnd, GWL_STYLE)&SS_ICON)
		SetWindowLongPtr(Buttons[pos].hwnd, GWL_STYLE, curstyle|SS_ICON);

	if ( !UseIcoLib || (Buttons[pos].dwFlags & TTBBF_ISSEPARATOR)) {
		if (!(Buttons[pos].dwFlags & TTBBF_DRAWBORDER)) {
			if (Buttons[pos].hbBitmapDown == NULL) {
				t = IMAGE_ICON;
				SendMessage(Buttons[pos].hwnd, STM_SETIMAGE, t, (LPARAM)((Buttons[pos].bPushed)?(Buttons[pos].hIconDn):(Buttons[pos].hIconUp)));
				SendMessage(Buttons[pos].hwnd, BM_SETIMAGE, t, (LPARAM)((Buttons[pos].bPushed)?(Buttons[pos].hIconDn):(Buttons[pos].hIconUp)));
			}
			else {
				SendMessage(Buttons[pos].hwnd, STM_SETIMAGE, t, (LPARAM)((Buttons[pos].bPushed)?(Buttons[pos].hbBitmapDown):(Buttons[pos].hbBitmapUp)));
				SendMessage(Buttons[pos].hwnd, BM_SETIMAGE, t, (LPARAM)((Buttons[pos].bPushed)?(Buttons[pos].hbBitmapDown):(Buttons[pos].hbBitmapUp)));
			}
		}
		else SendMessage(Buttons[pos].hwnd, STM_SETIMAGE, t, (LPARAM)((Buttons[pos].bPushed)?(Buttons[pos].hbWBordBitmapDown):(Buttons[pos].hbWBordBitmapUp)));	
	}
	else SendMessage(Buttons[pos].hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)((Buttons[pos].bPushed)?(Buttons[pos].hIconDn):(Buttons[pos].hIconUp)));

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toolbar services

//wparam = hTTBButton
//lparam = state 
INT_PTR TTBSetState(WPARAM wParam, LPARAM lParam)
{
	lockbut();
	int pos = idtopos(wParam);
	if (pos<0 || pos >= nButtonsCount){ulockbut();;return -1;}

	Buttons[pos].bPushed = (lParam&TTBST_PUSHED)?TRUE:FALSE;
	Buttons[pos].bPushed = (lParam&TTBST_RELEASED)?FALSE:TRUE;
	SetButtBitmap(pos);
	ulockbut();
	return 0;
}

//wparam = hTTBButton
//lparam = 0
//return = state
INT_PTR TTBGetState(WPARAM wParam, LPARAM lParam)
{
	lockbut();
	int pos = idtopos(wParam);
	if (pos < 0 || pos >= nButtonsCount) {
		ulockbut();
		return -1;
	}

	int retval = (Buttons[pos].bPushed == TRUE) ? TTBST_PUSHED : TTBST_RELEASED;
	ulockbut();
	return retval;
}

INT_PTR TTBGetOptions(WPARAM wParam, LPARAM lParam)
{
	int retval;

	lockbut();
	int pos = idtopos(HIWORD(wParam));
	if (pos < 0 || pos >= nButtonsCount) {
		ulockbut();
		return -1;
	}

	switch(LOWORD(wParam)) {
	case TTBO_FLAGS:
		retval = Buttons[pos].dwFlags & (!TTBBF_PUSHED);
		if (Buttons[pos].bPushed)
			retval |= TTBBF_PUSHED;
		break;

	case TTBO_POS:
		retval = (int)pos;
		break;
	
	case TTBO_TIPNAME:
		retval = (int)Buttons[pos].tooltip;
		break;

	case TTBO_ALLDATA:
		if (lParam) {
			lpTTBButton lpTTB = (lpTTBButton)lParam;
			if (lpTTB->cbSize != sizeof(TTBButton))
				break;
				
			lpTTB->dwFlags = Buttons[pos].dwFlags & (!TTBBF_PUSHED);
			if (Buttons[pos].bPushed)
				lpTTB->dwFlags |= TTBBF_PUSHED;

			lpTTB->hbBitmapDown = Buttons[pos].hbBitmapDown;
			lpTTB->hbBitmapUp = Buttons[pos].hbBitmapUp;
			lpTTB->lParamUp = Buttons[pos].lParamUp;
			lpTTB->wParamUp = Buttons[pos].wParamUp;
			lpTTB->lParamDown = Buttons[pos].lParamDown;
			lpTTB->wParamDown = Buttons[pos].wParamDown;

			lpTTB->pszServiceDown = _strdup(Buttons[pos].pszServiceDown);
			lpTTB->pszServiceUp = _strdup(Buttons[pos].pszServiceUp);

			retval = ( INT_PTR )lpTTB;
		}
		break;

	default:
		retval = -1;
		break;
	}
	
	ulockbut();
	return retval;
}


INT_PTR TTBSetOptions(WPARAM wParam, LPARAM lParam)
{
	int pos, retval;

	lockbut();
	pos = idtopos(HIWORD(wParam));
	if (pos<0 || pos >= nButtonsCount){ulockbut();return -1;}

	switch(LOWORD(wParam)) {
	case TTBO_FLAGS:
		if (Buttons[pos].dwFlags == lParam)
			break;

		Buttons[pos].dwFlags = lParam;
      Buttons[pos].bPushed = (Buttons[pos].dwFlags & TTBBF_PUSHED) ? TRUE : FALSE;
		if (Buttons[pos].dwFlags & TTBBF_DRAWBORDER) {
			Buttons[pos].hbWBordBitmapDown = DrawBorderForBitmap(Buttons[pos].hbBitmapDown, FALSE);
			Buttons[pos].hbWBordBitmapUp = DrawBorderForBitmap(Buttons[pos].hbBitmapUp, TRUE);
		}

		SetButtBitmap(pos);
		SendMessage(Buttons[pos].hwndTip, TTM_ACTIVATE, (WPARAM)(Buttons[pos].dwFlags & TTBBF_SHOWTOOLTIP) ? TRUE : FALSE, 0);
		retval = 1;
		break;

	case TTBO_POS:
		if (lParam >= 0 || lParam < nButtonsCount) {
			TopButtonInt tempttb = Buttons[lParam];
			Buttons[lParam] = Buttons[pos];
			Buttons[pos] = tempttb;

			ArrangeButtons();
			retval = 1;
		}
		break;
	
	case TTBO_TIPNAME:
		if (lParam == 0)
			break;

		if (Buttons[pos].tooltip != NULL)
			free(Buttons[pos].tooltip);
		Buttons[pos].tooltip = _tcsdup( TranslateTS( _A2T((LPCSTR)lParam)));
		UpdateToolTip(pos);
		retval = 1;
		break;

	case TTBO_ALLDATA:
		if (lParam) {
			lpTTBButton lpTTB = (lpTTBButton)lParam;
			if (lpTTB->cbSize != sizeof(TTBButton))
				break;

			Buttons[pos].dwFlags = lpTTB->dwFlags;
			Buttons[pos].hbBitmapDown = lpTTB->hbBitmapDown;
			Buttons[pos].hbBitmapUp = lpTTB->hbBitmapUp;
			Buttons[pos].lParamUp = lpTTB->lParamUp;
			Buttons[pos].wParamUp = lpTTB->wParamUp;
			Buttons[pos].lParamDown = lpTTB->lParamDown;
			Buttons[pos].wParamDown = lpTTB->wParamDown;

			if (Buttons[pos].pszServiceDown != NULL)
				free(Buttons[pos].pszServiceDown);
			if (Buttons[pos].pszServiceUp != NULL)
				free(Buttons[pos].pszServiceUp);

			Buttons[pos].pszServiceDown = _strdup(lpTTB->pszServiceDown);
			Buttons[pos].pszServiceUp = _strdup(lpTTB->pszServiceUp);

			if (Buttons[pos].dwFlags & TTBBF_DRAWBORDER) {
				Buttons[pos].hbWBordBitmapDown = DrawBorderForBitmap(Buttons[pos].hbBitmapDown, FALSE);
				Buttons[pos].hbWBordBitmapUp = DrawBorderForBitmap(Buttons[pos].hbBitmapUp, TRUE);
			}

			Buttons[pos].bPushed = (Buttons[pos].dwFlags&TTBBF_PUSHED)?TRUE:FALSE;
			SendMessage(Buttons[pos].hwndTip, TTM_ACTIVATE, (WPARAM)(Buttons[pos].dwFlags&TTBBF_SHOWTOOLTIP)?TRUE:FALSE, 0);
			ArrangeButtons();
			retval = 1;
		}
		break;

	default:
		retval = -1;
		break;
	}
	
	ulockbut();
	return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toolbar window procedure

static void PaintToolbar(HWND hwnd)
{
	InvalidateRect(hwnd, 0, FALSE);

	PAINTSTRUCT paintst;	
	HDC hdc = BeginPaint(hwnd, &paintst);
	RECT *rcPaint = &paintst.rcPaint;

	RECT clRect;
	GetClientRect(hwnd, &clRect);
	if (rcPaint == NULL) rcPaint = &clRect;

	int yScroll = 0;
	int y = -yScroll;

	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hBmpOsb = CreateBitmap(clRect.right, clRect.bottom, 1, GetDeviceCaps(hdc, BITSPIXEL), NULL);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmpOsb);
	SetBkMode(hdcMem, TRANSPARENT);

	HBRUSH hBrush, hoBrush;

	hBrush = CreateSolidBrush(bkColour);
	hoBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
	FillRect(hdcMem, rcPaint, hBrush);
	SelectObject(hdcMem, hoBrush);
	DeleteObject(hBrush);
	if (hBmpBackground) {
		BITMAP bmp;
		HDC hdcBmp;
		int x, y;
		int maxx, maxy;
		int destw, desth;

		GetObject(hBmpBackground, sizeof(bmp), &bmp);
		hdcBmp = CreateCompatibleDC(hdcMem);
		SelectObject(hdcBmp, hBmpBackground);
		y = backgroundBmpUse&CLBF_SCROLL?-yScroll:0;
		maxx = backgroundBmpUse&CLBF_TILEH?clRect.right:1;
		maxy = backgroundBmpUse&CLBF_TILEV?maxy = rcPaint->bottom:y+1;
		switch(backgroundBmpUse&CLBM_TYPE) {
		case CLB_STRETCH:
			if (backgroundBmpUse&CLBF_PROPORTIONAL) {
				if (clRect.right*bmp.bmHeight<clRect.bottom*bmp.bmWidth) {
					desth = clRect.bottom;
					destw = desth*bmp.bmWidth/bmp.bmHeight;
				}
				else {
					destw = clRect.right;
					desth = destw*bmp.bmHeight/bmp.bmWidth;
				}
			}
			else {
				destw = clRect.right;
				desth = clRect.bottom;
			}
			break;
		case CLB_STRETCHH:
			if (backgroundBmpUse&CLBF_PROPORTIONAL) {
				destw = clRect.right;
				desth = destw*bmp.bmHeight/bmp.bmWidth;
				if (backgroundBmpUse&CLBF_TILEVTOROWHEIGHT)
					desth = BUTTHEIGHT+2;
			}
			else {
				destw = clRect.right;
				desth = bmp.bmHeight;
			}
			break;
		case CLB_STRETCHV:
			if (backgroundBmpUse&CLBF_PROPORTIONAL) {
				desth = clRect.bottom;
				destw = desth*bmp.bmWidth/bmp.bmHeight;
			}
			else {
				destw = bmp.bmWidth;
				desth = clRect.bottom;
			}
			break;
		default:    //clb_topleft
			destw = bmp.bmWidth;
			desth = bmp.bmHeight;
			if (backgroundBmpUse&CLBF_TILEVTOROWHEIGHT)
				desth = BUTTHEIGHT+2;
			break;
		}

		for (; y < maxy; y += desth) {
			if (y < rcPaint->top - desth) continue;
			for (x = 0; x < maxx; x += destw)
				StretchBlt(hdcMem, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		}
		DeleteDC(hdcBmp);
	}
	BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right-rcPaint->left, rcPaint->bottom-rcPaint->top, hdcMem, rcPaint->left, rcPaint->top, SRCCOPY);
	SelectObject(hdcMem, hOldBmp);
	DeleteDC(hdcMem);
	DeleteObject(hBmpOsb);
	paintst.fErase = FALSE;
	EndPaint(hwnd, &paintst);	
}

LRESULT CALLBACK TopToolBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CREATE:
		hwndTopToolBar = hwnd;
		return FALSE;

	case  WM_MOVE:
		return 0;

	case WM_WINDOWPOSCHANGING:
	case WM_SIZE:
		ArrangeButtons();
		return 0;

	case WM_PAINT:
		PaintToolbar(hwnd);
		return 0;

	case WM_LBUTTONDOWN:
		if (DBGetContactSettingByte(NULL, "CLUI", "ClientAreaDrag", 0)) {
			POINT pt;
			GetCursorPos(&pt);
			return SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE|HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		return 0;	

	case WM_NOTIFY:
		return 0;

	case WM_COMMAND:
		if ((HIWORD(wParam) == STN_CLICKED || HIWORD(wParam) == STN_DBLCLK)) {
			int id = GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
			if (id != 0) {
				lockbut();
				int pos = idtopos(id);
				if (isSep(pos)) {
					ulockbut();
					return 0;
				}

				if (Buttons[pos].bPushed) {
					//Dn -> Up
					Buttons[pos].bPushed = !Buttons[pos].bPushed;
					SetButtBitmap(pos);

					if (Buttons[pos].pszServiceUp != NULL)
						CallService(Buttons[pos].pszServiceUp, Buttons[pos].wParamUp, Buttons[pos].lParamUp);
				}
				else {
					//Up -> Dn
					Buttons[pos].bPushed = !Buttons[pos].bPushed;
					SetButtBitmap(pos);

					if (Buttons[pos].pszServiceDown != NULL)
						CallService(Buttons[pos].pszServiceDown, Buttons[pos].wParamDown, Buttons[pos].lParamDown);
				}
				ulockbut();
			}
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return(TRUE);
}

static TCHAR pluginname[] = _T("TopToolBar");

int addTopToolBarWindow(HWND parent)
{
	WNDCLASS wndclass;
	wndclass.style         = 0;
	wndclass.lpfnWndProc   = TopToolBarProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);//NULL;//(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = pluginname;
	RegisterClass(&wndclass);
	HWND pluginwind = CreateWindow(pluginname, pluginname, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
		0, 0, 0, 0, parent, NULL, hInst, NULL);

	ttbOptionsChanged();

	CLISTFrame Frame = { 0 };
	Frame.cbSize = sizeof(Frame);
	Frame.tname = pluginname;
	Frame.hWnd = pluginwind;
	Frame.align = alTop;
	Frame.Flags = F_VISIBLE | F_NOBORDER | F_LOCKED | F_TCHAR;
	Frame.height = 18;
	return (int)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(0, idEvent);
	NotifyEventHooks(hHookTTBModuleLoaded, 0, 0);
}

static INT_PTR OnEventFire(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_SYSTEM_REMOVEWAIT, wParam, 0);
	StopArrange = FALSE;
	NotifyEventHooks(hHookTTBModuleLoaded, 0, 0);
	return 0;
}

int OnIconChange(WPARAM wParam, LPARAM lParam)
{
	lockbut();
	for (int i = 0; i < nButtonsCount; i++) {
		char buf[256];
		sprintf(buf, "%s_up", Buttons[i].name);
		Buttons[i].hIconUp = LoadIconFromLibrary("TopToolBar", buf, buf, Buttons[i].hIconUp, FALSE, NULL);
		sprintf(buf, "%s_dn", Buttons[i].name);
		Buttons[i].hIconDn = LoadIconFromLibrary("TopToolBar", buf, buf, Buttons[i].hIconDn, FALSE, NULL);
	}
	ulockbut();
	RecreateWindows();
	SetAllBitmaps();
	ArrangeButtons();
	return 0;
}

static int OnBGChange(WPARAM wParam, LPARAM lParam)
{
	ttbOptionsChanged();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int OnModulesLoad(WPARAM wParam, LPARAM lParam)
{
	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		MessageBox(0, TranslateT("Frames Services not found - plugin disabled.You need MultiWindow plugin."), _T("TopToolBar"), 0);
		return 0;
	}

	hwndContactList = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

	OptionsOpened = false;
	UseIcoLib = DBGetContactSettingByte(0, TTB_OPTDIR, "UseIcoLib", UseIcoLibDefaultValue);
	if (UseIcoLib)
		arHooks.insert( HookEvent(ME_SKIN2_ICONSCHANGED, OnIconChange));

	hFrameTopWindow = addTopToolBarWindow(hwndContactList);
	LoadInternalButtons(( HWND )CallService(MS_CLUI_GETHWNDTREE, 0, 0));
	SaveAllButtonsOptions();

	LoadAllSeparators();
	InitLBut();

	StopArrange = FALSE;
	ArrangeButtons();

	HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);//anonymous event
	if (hEvent != 0)
		CallService(MS_SYSTEM_WAITONHANDLE, (WPARAM)hEvent, (LPARAM)"TTB_ONSTARTUPFIRE");
	
	if ( ServiceExists(MS_BACKGROUNDCONFIG_REGISTER)) {
		char buf[256];
		sprintf(buf, "TopToolBar Background/%s", TTB_OPTDIR);
		CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)buf, 0);
		arHooks.insert( HookEvent(ME_BACKGROUNDCONFIG_CHANGED, OnBGChange));
	}	

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadToolbarModule()
{
	InitializeCriticalSection(&csButtonsHook);
	StopArrange = TRUE;

	arHooks.insert( HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoad));
	arHooks.insert( HookEvent(ME_OPT_INITIALISE, TTBOptInit));

	arServices.insert( CreateServiceFunction(MS_TTB_ADDBUTTON, TTBAddButton));
	arServices.insert( CreateServiceFunction(MS_TTB_REMOVEBUTTON, TTBRemoveButton));

	arServices.insert( CreateServiceFunction(MS_TTB_SETBUTTONSTATE, TTBSetState));
	arServices.insert( CreateServiceFunction(MS_TTB_GETBUTTONSTATE, TTBGetState));
	
	arServices.insert( CreateServiceFunction(MS_TTB_GETBUTTONOPTIONS, TTBGetOptions));
	arServices.insert( CreateServiceFunction(MS_TTB_SETBUTTONOPTIONS, TTBSetOptions));
	arServices.insert( CreateServiceFunction(TTB_ADDSEPARATOR, InsertNewFreeSeparator));
	arServices.insert( CreateServiceFunction(TTB_REMOVESEPARATOR, DeleteSeparator));
	
	arServices.insert( CreateServiceFunction("TTB_ONSTARTUPFIRE", OnEventFire));

	BUTTHEIGHT = DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTHEIGHT", 16);
	BUTTWIDTH = DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTWIDTH", 20);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int UnloadToolbarModule()
{
	SaveAllSeparators();

	DeleteCriticalSection(&csButtonsHook);
	return 0;
}
