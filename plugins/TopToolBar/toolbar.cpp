
#include "common.h"

pfnCustomProc g_CustomProc = NULL;
LPARAM g_CustomProcParam = 0;

HWND hwndContactList = 0;
HWND hwndTopToolBar = 0;
bool StopArrange;

int BUTTWIDTH;
int BUTTHEIGHT;
int BUTTGAP;

int nextButtonId = 200;

//------------ options -------------
COLORREF bkColour;
HBITMAP hBmpBackground, hBmpSeparator;
int backgroundBmpUse;

CRITICAL_SECTION csButtonsHook;
static int hFrameTopWindow = -1;

int sortfunc(const TopButtonInt* a, const TopButtonInt* b)
{
	return a->arrangedpos - b->arrangedpos;
}

LIST<TopButtonInt> Buttons(8, sortfunc);

static void SetAllBitmaps()
{
	mir_cslock lck(csButtonsHook);
	for (int i = 0; i < Buttons.getCount(); i++)
		Buttons[i]->SetBitmap();
}

static TopButtonInt* idtopos(int id, int* pPos=NULL)
{
	for ( int i = 0; i < Buttons.getCount(); i++)
		if (Buttons[i]->id == id) {
			if (pPos) *pPos = i;
			return Buttons[i];
		}

	if (pPos) *pPos = -1;
	return NULL;
}

//----- Service buttons -----
void InsertSBut(int i)
{
	TTBButton ttb = { 0 };
	ttb.cbSize = sizeof(ttb);
	ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_RUN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_RUN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_ISSBUTTON|TTBBF_INTERNAL;
	ttb.wParamDown = i;
	TTBAddButton(( WPARAM )&ttb, 0);
}

void LoadAllSButs()
{
	//must be locked
	int cnt = DBGetContactSettingByte(0, TTB_OPTDIR, "ServiceCnt", 0);
	if (cnt > 0) {
		for (int i = 1; i<=cnt; i++)
		InsertSBut(i);
	}
}

//----- Launch buttons -----
INT_PTR LaunchService(WPARAM wParam, LPARAM lParam)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);

	if ( CreateProcess(NULL, Buttons[lParam]->program, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return 0;
}

void InsertLBut(int i)
{
	TTBButton ttb = { 0 };
	ttb.cbSize = sizeof(ttb);
	ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_RUN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_ISLBUTTON | TTBBF_INTERNAL;
	ttb.name = LPGEN("Default");
	ttb.program = _T("Execute Path");
	ttb.wParamDown = i;
	TTBAddButton(( WPARAM )&ttb, 0);
}

void LoadAllLButs()
{
	//must be locked
	int cnt = DBGetContactSettingByte(0, TTB_OPTDIR, "LaunchCnt", 0);
	for (int i = 1; i<=cnt; i++)
		InsertLBut(i);
}

//----- Separators -----

void InsertSeparator(int i)
{
	TTBButton ttb = { 0 };
	ttb.cbSize = sizeof(ttb);
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_ISSEPARATOR | TTBBF_INTERNAL;
	ttb.wParamDown = i;
	TTBAddButton((WPARAM)&ttb, 0);
}

void LoadAllSeparators()
{
	//must be locked
	int cnt = DBGetContactSettingByte(0, TTB_OPTDIR, "SepCnt", 0);
	for (int i = 1; i<=cnt; i++)
		InsertSeparator(i);
}

int SaveAllButtonsOptions()
{
	int SeparatorCnt = 0;
	int LaunchCnt = 0;
	{
		mir_cslock lck(csButtonsHook);
		for (int i = 0; i < Buttons.getCount(); i++)
			Buttons[i]->SaveSettings(&SeparatorCnt, &LaunchCnt);
	}
	DBWriteContactSettingByte(0, TTB_OPTDIR, "SepCnt", SeparatorCnt);
	DBWriteContactSettingByte(0, TTB_OPTDIR, "LaunchCnt", LaunchCnt);
	return 0;
}

int ttbOptionsChanged()
{
	//load options
	bkColour = DBGetContactSettingDword(NULL, TTB_OPTDIR, "BkColour", TTBDEFAULT_BKCOLOUR);
	if (hBmpBackground) {
		DeleteObject(hBmpBackground);
		hBmpBackground = NULL;
	}

	if (DBGetContactSettingByte(NULL, TTB_OPTDIR, "UseBitmap", TTBDEFAULT_USEBITMAP)) {
		DBVARIANT dbv;
		if (!DBGetContactSetting(NULL, TTB_OPTDIR, "BkBitmap", &dbv)) {
			hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	backgroundBmpUse = DBGetContactSettingWord(NULL, TTB_OPTDIR, "BkBmpUse", TTBDEFAULT_BKBMPUSE);

	RECT rc;
	GetClientRect(hwndTopToolBar, &rc);
	InvalidateRect(hwndTopToolBar, &rc, TRUE);
	UpdateWindow(hwndTopToolBar);

	return 0;
}

INT_PTR TTBRemoveButton(WPARAM wParam, LPARAM lParam)
{
	mir_cslock lck(csButtonsHook);

	int idx;
	TopButtonInt* b = idtopos(wParam, &idx);
	if (b == NULL)
		return -1;
	
	RemoveFromOptions(b->id);

	Buttons.remove(idx);
	delete b;

	ArrangeButtons();
	return 0;
}

static bool nameexists(const char *name)
{
	if (name == NULL)
		return false;

	for (int i = 0; i < Buttons.getCount(); i++)
		if ( !lstrcmpA(Buttons[i]->name, name))
			return true;

	return false;
}

HICON LoadIconFromLibrary(char *Name, HICON hIcon, HANDLE& phIcolib)
{		
	char iconame[256];
	_snprintf(iconame, SIZEOF(iconame), "toptoolbar_%s", Name);
	if (phIcolib == NULL) {
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(sid);
		sid.pszSection = "Toolbar";				
		sid.pszName = iconame;
		sid.pszDefaultFile = NULL;
		sid.pszDescription = Name;
		sid.hDefaultIcon = hIcon;
		phIcolib = Skin_AddIcon(&sid);
	}
	return Skin_GetIconByHandle(phIcolib);
}

static void ReloadIcons()
{
	mir_cslock lck(csButtonsHook);
	for (int i = 0; i < Buttons.getCount(); i++) {
		TopButtonInt* b = Buttons[i];

		char buf[256];
		if (b->hIconHandleUp) {
			sprintf(buf, "%s_up", b->name);
			b->hIconUp = LoadIconFromLibrary(buf, b->hIconUp, b->hIconHandleUp);
		}
		if (b->hIconHandleDn) {
			sprintf(buf, "%s_dn", b->name);
			b->hIconDn = LoadIconFromLibrary(buf, b->hIconDn, b->hIconHandleDn);
		}
	}
}

TopButtonInt* CreateButton(TTBButton* but)
{
	TopButtonInt* b = new TopButtonInt;
	b->id = nextButtonId++;

	b->dwFlags = but->dwFlags;

	b->wParamUp = but->wParamUp;
	b->lParamUp = but->lParamUp;
	b->wParamDown = but->wParamDown;
	b->lParamDown = but->lParamDown;

	if ( !(b->dwFlags & TTBBF_ISSEPARATOR)) {
		b->bPushed = (but->dwFlags & TTBBF_PUSHED) ? TRUE : FALSE;

		if (but->dwFlags & TTBBF_ISLBUTTON) {
			if (but->program != NULL)
				b->program = _tcsdup(but->program);
			b->pszService = _strdup(TTB_LAUNCHSERVICE);
		}
		else {
			b->program = NULL;
			if (but->pszService != NULL)
				b->pszService = _strdup(but->pszService);
			else
				b->pszService = NULL;
		}

		if (but->name != NULL)
			b->name = _strdup(but->name);
		else
			b->name = NULL;

		if (b->dwFlags & TTBBF_ICONBYHANDLE) {
		  if (but->hIconHandleDn)
				b->hIconDn = Skin_GetIconByHandle(but->hIconHandleDn);
			else
				b->hIconDn = 0;
			b->hIconUp = Skin_GetIconByHandle(but->hIconHandleUp);
		}
		else {
			b->hIconDn = but->hIconDn;
			b->hIconUp = but->hIconUp;
		}

		char buf[256];
		sprintf(buf, "%s_up", b->name);
		b->hIconUp = LoadIconFromLibrary(buf, b->hIconUp, b->hIconHandleUp);
		if (b->hIconDn) {
			sprintf(buf, "%s_dn", b->name);
			b->hIconDn = LoadIconFromLibrary(buf, b->hIconDn, b->hIconHandleDn);
		}
	}
	return b;
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

	TopButtonInt* b;
	{	
		mir_cslock lck(csButtonsHook);

		TTBButton *but = (TTBButton*)wParam;
		if (but->cbSize != sizeof(TTBButton) || (!(but->dwFlags && TTBBF_ISLBUTTON) && nameexists(but->name)))
			return -1;

		b = CreateButton(but);
		b->LoadSettings();
		Buttons.insert(b);
		b->CreateWnd();
	}

	ArrangeButtons();
	AddToOptions(b);
	return b->id;
}

int ArrangeButtons()
{
	if (StopArrange == TRUE)
		return 0;

	mir_cslock lck(csButtonsHook);

	RECT winrc;
	GetClientRect(hwndTopToolBar, &winrc);
	winrc.left = winrc.right-winrc.left;
	if (winrc.left == 0)
		return 0;
	int newheight;

	StopArrange = true;
	int ypos = 1;
	int xpos = BUTTGAP;
	newheight = BUTTHEIGHT+1;
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, hFrameTopWindow), BUTTHEIGHT+2);

	int uFlags;

	for (int i = 0; i < Buttons.getCount(); i++) {
		TopButtonInt *b = Buttons[i];

		// not visible - hide and skip
		if ( !(b->dwFlags & TTBBF_VISIBLE)) {
			ShowWindow(b->hwnd, SW_HIDE);
			continue;
		}

		if ( b->isSep())
			uFlags = SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW;
		else
			uFlags = SWP_NOZORDER | SWP_SHOWWINDOW;

		// check, if need to move to next line
		if (xpos+BUTTWIDTH+BUTTGAP > winrc.left) {
			xpos = BUTTGAP;
			ypos += BUTTHEIGHT+2;
			newheight = ypos+BUTTHEIGHT+1;
			CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, hFrameTopWindow), newheight);
		}

		SetWindowPos(b->hwnd, 0, xpos, ypos, BUTTWIDTH, BUTTHEIGHT, uFlags);
		InvalidateRect(b->hwnd, NULL, TRUE);

		xpos += (b->isSep()) ? SEPWIDTH+2 : BUTTWIDTH+BUTTGAP;

	}
	StopArrange = false;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toolbar services

//wparam = hTTBButton
//lparam = state 
INT_PTR TTBSetState(WPARAM wParam, LPARAM lParam)
{
	mir_cslock lck(csButtonsHook);

	TopButtonInt* b = idtopos(wParam);
	if (b == NULL)
		return -1;

	b->bPushed = (lParam&TTBST_PUSHED)?TRUE:FALSE;
	b->bPushed = (lParam&TTBST_RELEASED)?FALSE:TRUE;
	b->SetBitmap();
	return 0;
}

//wparam = hTTBButton
//lparam = 0
//return = state
INT_PTR TTBGetState(WPARAM wParam, LPARAM lParam)
{
	mir_cslock lck(csButtonsHook);
	TopButtonInt* b = idtopos(wParam);
	if (b == NULL)
		return -1;

	int retval = (b->bPushed == TRUE) ? TTBST_PUSHED : TTBST_RELEASED;
	return retval;
}

INT_PTR TTBGetOptions(WPARAM wParam, LPARAM lParam)
{
	INT_PTR retval;

	mir_cslock lck(csButtonsHook);
	TopButtonInt* b = idtopos(wParam);
	if (b == NULL)
		return -1;

	switch(LOWORD(wParam)) {
	case TTBO_FLAGS:
		retval = b->dwFlags & (~TTBBF_PUSHED);
		if (b->bPushed)
			retval |= TTBBF_PUSHED;
		break;

	case TTBO_TIPNAME:
		retval = (INT_PTR)b->tooltip;
		break;

	case TTBO_ALLDATA:
		if (lParam) {
			lpTTBButton lpTTB = (lpTTBButton)lParam;
			if (lpTTB->cbSize != sizeof(TTBButton))
				break;
				
			lpTTB->dwFlags = b->dwFlags & (~TTBBF_PUSHED);
			if (b->bPushed)
				lpTTB->dwFlags |= TTBBF_PUSHED;

			lpTTB->hIconDn = b->hIconDn;
			lpTTB->hIconUp = b->hIconUp;

			lpTTB->lParamUp = b->lParamUp;
			lpTTB->wParamUp = b->wParamUp;
			lpTTB->lParamDown = b->lParamDown;
			lpTTB->wParamDown = b->wParamDown;

			if (b->dwFlags & TTBBF_ISLBUTTON)
				lpTTB->program = _tcsdup(b->program);
			else
				lpTTB->pszService = _strdup(b->pszService);

			retval = ( INT_PTR )lpTTB;
		}
		break;

	default:
		retval = -1;
		break;
	}
	
	return retval;
}

INT_PTR TTBSetOptions(WPARAM wParam, LPARAM lParam)
{
	int retval;

	mir_cslock lck(csButtonsHook);
	TopButtonInt* b = idtopos(wParam);
	if (b == NULL)
		return -1;

	switch(LOWORD(wParam)) {
	case TTBO_FLAGS:
		if (b->dwFlags == lParam)
			break;

		retval = b->CheckFlags(lParam);
		
		if (retval & TTBBF_PUSHED)
			b->SetBitmap();
		if (retval & TTBBF_VISIBLE) {
			ArrangeButtons();
			b->SaveSettings(0,0);
		}
				
		retval = 1;
		break;

	case TTBO_TIPNAME:
		if (lParam == 0)
			break;

		if (b->tooltip != NULL)
			free(b->tooltip);
		b->tooltip = _tcsdup( TranslateTS( _A2T((LPCSTR)lParam)));
		SendMessage(b->hwnd,BUTTONADDTOOLTIP,(WPARAM)b->tooltip,BATF_UNICODE);
		retval = 1;
		break;

	case TTBO_ALLDATA:
		if (lParam) {
			lpTTBButton lpTTB = (lpTTBButton)lParam;
			if (lpTTB->cbSize != sizeof(TTBButton))
				break;

			retval = b->CheckFlags(lpTTB->dwFlags);

			int changed = 0;
			if (b->hIconUp != lpTTB->hIconUp) {
				b->hIconUp = lpTTB->hIconUp;
				changed = 1;
			}
			if (b->hIconDn != lpTTB->hIconDn) {
				b->hIconDn = lpTTB->hIconDn;
				changed = 1;
			}
			if (changed)
				b->SetBitmap();

			if (retval & TTBBF_VISIBLE) {
				ArrangeButtons();
				b->SaveSettings(0,0);
			}

			if (b->dwFlags & TTBBF_ISLBUTTON) {
				if (b->program != NULL)
					free(b->program);
				b->program = _tcsdup(lpTTB->program);
//				don't save program changing in use buttons
//				changed = 1; // for launch buttons from user, not options (options don't share handles)
			}
			else {
				if (b->pszService != NULL)
					free(b->pszService);
				b->pszService = _strdup(lpTTB->pszService);
			}

			b->lParamUp = lpTTB->lParamUp;
			b->wParamUp = lpTTB->wParamUp;
			b->lParamDown = lpTTB->lParamDown;
			b->wParamDown = lpTTB->wParamDown;

			retval = 1;
		}
		break;

	default:
		retval = -1;
		break;
	}
	
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
		y = backgroundBmpUse & CLBF_SCROLL ? -yScroll : 0;
		maxx = backgroundBmpUse & CLBF_TILEH ? clRect.right : 1;
		maxy = backgroundBmpUse & CLBF_TILEV ? maxy = rcPaint->bottom : y+1;
		switch(backgroundBmpUse & CLBM_TYPE) {
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
				if (backgroundBmpUse & CLBF_TILEVTOROWHEIGHT)
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

	case WM_MOVE:
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
				mir_cslock lck(csButtonsHook);
				TopButtonInt* b = idtopos(id);
				if (b == NULL || b->isSep())
					return 0;

				// flag inversion inside condition coz we uses Up -> Down for non-push buttons
				// condition and inversion can be moved to main condition end
				if (b->bPushed) { //Dn -> Up

					if (b->dwFlags & TTBBF_ASPUSHBUTTON)
						b->bPushed = !b->bPushed;

					if (!(b->dwFlags & TTBBF_ISLBUTTON)) // must be always true
						if (b->pszService != NULL)
							CallService(b->pszService, b->wParamUp, b->lParamUp);
				}
				else { //Up -> Dn
					if (b->dwFlags & TTBBF_ASPUSHBUTTON)
						b->bPushed = !b->bPushed;

					if (b->pszService != NULL)
						CallService(b->pszService, b->wParamDown, b->lParamDown);
				}

				b->SetBitmap();
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
	WNDCLASS wndclass = { 0 };
	wndclass.lpfnWndProc   = TopToolBarProc;
	wndclass.cbWndExtra    = sizeof(void*);
	wndclass.hInstance     = hInst;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);//NULL;//(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszClassName = pluginname;
	RegisterClass(&wndclass);

	HWND pluginwind = CreateWindow(pluginname, pluginname, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
		0, 0, 0, 0, parent, NULL, hInst, NULL);

	CLISTFrame Frame = { 0 };
	Frame.cbSize = sizeof(Frame);
	Frame.tname = pluginname;
	Frame.hWnd = pluginwind;
	Frame.align = alTop;
	Frame.Flags = F_VISIBLE | F_NOBORDER | F_LOCKED | F_TCHAR;
	Frame.height = 18;
	return (int)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);
}

static INT_PTR OnEventFire(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_SYSTEM_REMOVEWAIT, wParam, 0);
	StopArrange = FALSE;
	NotifyEventHooks(hHookTTBModuleLoaded, 0, 0);

	if (g_CustomProc) {
		mir_cslock lck(csButtonsHook);

		for (int i=0; i < Buttons.getCount(); i++) {
			TopButtonInt* p = Buttons[i];
			g_CustomProc((HANDLE)p->id, p->hwnd, g_CustomProcParam);
		}
	}

	return 0;
}

int OnIconChange(WPARAM wParam, LPARAM lParam)
{
	ReloadIcons();
	SetAllBitmaps();
	return 0;
}

static int OnBGChange(WPARAM wParam, LPARAM lParam)
{
	ttbOptionsChanged();
	return 0;
}

static INT_PTR TTBSetCustomProc(WPARAM wParam, LPARAM lParam)
{
	g_CustomProc = (pfnCustomProc)wParam;
	g_CustomProcParam = lParam;
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

	hFrameTopWindow = addTopToolBarWindow(hwndContactList);
	LoadInternalButtons(( HWND )CallService(MS_CLUI_GETHWNDTREE, 0, 0));
	LoadAllSeparators();
	LoadAllLButs();

	StopArrange = FALSE;
	ArrangeButtons();

	HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);//anonymous event
	if (hEvent != 0)
		CallService(MS_SYSTEM_WAITONHANDLE, (WPARAM)hEvent, (LPARAM)"TTB_ONSTARTUPFIRE");
	
	if ( ServiceExists(MS_BACKGROUNDCONFIG_REGISTER)) {
		char buf[256];
		sprintf(buf, "TopToolBar Background/%s", TTB_OPTDIR);
		CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)buf, 0);
		HookEvent(ME_BACKGROUNDCONFIG_CHANGED, OnBGChange);
	}	

	ttbOptionsChanged();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadToolbarModule()
{
	InitializeCriticalSection(&csButtonsHook);
	StopArrange = TRUE;
	hBmpSeparator = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SEP));

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoad);
	HookEvent(ME_SKIN2_ICONSCHANGED, OnIconChange);
	HookEvent(ME_OPT_INITIALISE, TTBOptInit);

	CreateServiceFunction(MS_TTB_ADDBUTTON, TTBAddButton);
	CreateServiceFunction(MS_TTB_REMOVEBUTTON, TTBRemoveButton);

	CreateServiceFunction(MS_TTB_SETBUTTONSTATE, TTBSetState);
	CreateServiceFunction(MS_TTB_GETBUTTONSTATE, TTBGetState);
	
	CreateServiceFunction(MS_TTB_GETBUTTONOPTIONS, TTBGetOptions);
	CreateServiceFunction(MS_TTB_SETBUTTONOPTIONS, TTBSetOptions);

	CreateServiceFunction(TTB_LAUNCHSERVICE, LaunchService);
	
	CreateServiceFunction("TopToolBar/SetCustomProc", TTBSetCustomProc);
	CreateServiceFunction("TTB_ONSTARTUPFIRE", OnEventFire);

	BUTTHEIGHT = DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTHEIGHT", DEFBUTTHEIGHT);
	BUTTWIDTH = DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTWIDTH", DEFBUTTWIDTH);
	BUTTGAP = DBGetContactSettingByte(0, TTB_OPTDIR, "BUTTGAP", DEFBUTTGAP);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int UnloadToolbarModule()
{
	DeleteObject(hBmpSeparator);
	DeleteCriticalSection(&csButtonsHook);

	for (int i=0; i < Buttons.getCount(); i++)
		delete Buttons[i];
	Buttons.destroy();
	return 0;
}
