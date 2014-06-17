
#include "common.h"

#define BitChanged(c) (dwFlags ^ Flags) & c

static int maxid = 10000;

TopButtonInt::~TopButtonInt()
{
	if (hwnd)
		DestroyWindow(hwnd);

	if (dwFlags & TTBBF_ISLBUTTON) {
		mir_free(ptszProgram);
	}
	else if (pszService != NULL)
		mir_free(pszService);

	mir_free(pszName);
	mir_free(ptszTooltip);
	mir_free(ptszTooltipDn);
	mir_free(ptszTooltipUp);
}

DWORD TopButtonInt::CheckFlags(DWORD Flags)
{
	int res = 0;
	if (BitChanged(TTBBF_DISABLED)) {
		dwFlags ^= TTBBF_DISABLED;
		EnableWindow(hwnd, (dwFlags & TTBBF_DISABLED) ? FALSE : TRUE);
	}
	if (BitChanged(TTBBF_ASPUSHBUTTON)) {
		dwFlags ^= TTBBF_ASPUSHBUTTON;
		SendMessage(hwnd, BUTTONSETASPUSHBTN, (dwFlags & TTBBF_ASPUSHBUTTON) ? 1 : 0, 0);
	}
	if (BitChanged(TTBBF_SHOWTOOLTIP)) {
		dwFlags ^= TTBBF_SHOWTOOLTIP;
		SendMessage(hwnd, BUTTONADDTOOLTIP, (WPARAM)((dwFlags & TTBBF_SHOWTOOLTIP) ? ptszTooltip : _T("")), BATF_TCHAR);
	}
	// next settings changing visual side, requires additional actions
	if (BitChanged(TTBBF_VISIBLE)) {
		dwFlags ^= TTBBF_VISIBLE;
		res |= TTBBF_VISIBLE;
	}
	if (BitChanged(TTBBF_PUSHED)) {
		dwFlags ^= TTBBF_PUSHED;
		res |= TTBBF_PUSHED;
		bPushed = (dwFlags & TTBBF_PUSHED) ? TRUE : FALSE;
	}
	return res;
}

void TopButtonInt::CreateWnd()
{
	if (!(dwFlags & TTBBF_ISSEPARATOR)) {
		hwnd = CreateWindow(TTB_BUTTON_CLASS, _T(""), BS_PUSHBUTTON | WS_CHILD | WS_TABSTOP | SS_NOTIFY, 0, 0, g_ctrl->nButtonWidth, g_ctrl->nButtonHeight, g_ctrl->hWnd, NULL, hInst, this);

		if (dwFlags & TTBBF_ASPUSHBUTTON)
			SendMessage(hwnd, BUTTONSETASPUSHBTN, 1, 0);

		if (db_get_b(0, TTB_OPTDIR, "UseFlatButton", 1))
			SendMessage(hwnd, BUTTONSETASFLATBTN, TRUE, 0);

		EnableWindow(hwnd, (dwFlags & TTBBF_DISABLED) ? FALSE : TRUE);
	}
	// maybe SEPWIDTH, not g_ctrl->nButtonWidth?
	else
		hwnd = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | SS_NOTIFY, 0, 0, g_ctrl->nButtonWidth, g_ctrl->nButtonHeight, g_ctrl->hWnd, NULL, hInst, 0);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, id);
	SetBitmap();
}

void TopButtonInt::LoadSettings()
{
	char buf[255];

	BYTE oldv = isVisible();
	dwFlags = dwFlags & (~TTBBF_VISIBLE);

	if (dwFlags & TTBBF_ISSEPARATOR) {
		char buf1[10];
		_itoa(wParamDown, buf1, 10);
		char buf2[20];
		AS(buf2, "Sep", buf1);

		arrangedpos = db_get_b(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), Buttons.getCount());
		if (db_get_b(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), oldv) > 0)
			dwFlags |= TTBBF_VISIBLE;
	}
	else if ((dwFlags & TTBBF_ISLBUTTON ) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(wParamDown, buf1, 10);
		char buf2[20];
		AS(buf2, "Launch", buf1);

		mir_free(pszName);
		pszName = db_get_sa(0, TTB_OPTDIR, AS(buf, buf2, "_name"));
		
		mir_free(ptszProgram);
		ptszProgram = db_get_tsa(0, TTB_OPTDIR, AS(buf, buf2, "_lpath"));

		arrangedpos = db_get_b(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), Buttons.getCount());
		if (db_get_b(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), oldv) > 0)
			dwFlags |= TTBBF_VISIBLE;
	}
	else {
		arrangedpos = db_get_b(0, TTB_OPTDIR, AS(buf, pszName, "_Position"), Buttons.getCount());
		if (db_get_b(0, TTB_OPTDIR, AS(buf, pszName, "_Visible"), oldv) > 0)
			dwFlags |= TTBBF_VISIBLE;
	}

	if (Buttons.getIndex(this) != -1)
		arrangedpos = maxid++;
}

void TopButtonInt::SaveSettings(int *SepCnt, int *LaunchCnt)
{
	char buf[255];

	if (SepCnt && (dwFlags & TTBBF_ISSEPARATOR) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(SepCnt[0]++, buf1, 10);
		char buf2[20];
		AS(buf2, "Sep", buf1);

		db_set_b(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), arrangedpos);
		db_set_b(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), isVisible());
	}
	else if (LaunchCnt && (dwFlags & TTBBF_ISLBUTTON ) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(LaunchCnt[0]++, buf1, 10);
		char buf2[20];
		AS(buf2, "Launch", buf1);

		db_set_s(0, TTB_OPTDIR, AS(buf, buf2, "_name"), pszName);
		db_set_ts(0, TTB_OPTDIR, AS(buf, buf2, "_lpath"), ptszProgram);
		db_set_b(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), arrangedpos);
		db_set_b(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), isVisible());
	}
	else {
		db_set_b(0, TTB_OPTDIR, AS(buf, pszName, "_Position"), arrangedpos);
		db_set_b(0, TTB_OPTDIR, AS(buf, pszName, "_Visible"), isVisible());
	}
}

void TopButtonInt::SetBitmap()
{
	int curstyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	curstyle &= (~SS_BITMAP);
	curstyle &= (~SS_ICON);

	if (dwFlags & TTBBF_ISSEPARATOR) {
		SetWindowLongPtr(hwnd, GWL_STYLE, curstyle | SS_BITMAP);
		SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpSeparator);
		SendMessage(hwnd, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpSeparator);
	}
	else {
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & SS_ICON)
			SetWindowLongPtr(hwnd, GWL_STYLE, curstyle | SS_ICON);

		TCHAR *pTooltip;
		if (bPushed) {
			SendMessage(hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(hIconDn ? hIconDn : hIconUp));
			SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);

			pTooltip = ptszTooltipDn ? ptszTooltipDn : ptszTooltipUp;
		}
		else {
			SendMessage(hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconUp);
			SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
			pTooltip = ptszTooltipUp;
		}
		if (pTooltip)
			SendMessage(hwnd, BUTTONADDTOOLTIP, (WPARAM)TranslateTH(hLangpack, pTooltip), BATF_TCHAR);
	}
}
