
#include "common.h"

#define BitChanged(c) (dwFlags ^ Flags) & c

TopButtonInt::~TopButtonInt()
{
	if (dwFlags & TTBBF_ISLBUTTON) {
		if (program != NULL)
			free(program);
	}
	else if (pszService != NULL)
		free(pszService);

	if (name != NULL)
		free(name);
	if (tooltip != NULL)
		free(tooltip);
}

DWORD TopButtonInt::CheckFlags(DWORD Flags)
{
	int res = 0;
	if (BitChanged(TTBBF_DISABLED)) {
		dwFlags^=TTBBF_DISABLED;
		EnableWindow(hwnd,(dwFlags & TTBBF_DISABLED)?FALSE:TRUE);
	}
	if (BitChanged(TTBBF_ASPUSHBUTTON)) {
		dwFlags^=TTBBF_ASPUSHBUTTON;
		SendMessage(hwnd, BUTTONSETASPUSHBTN, (dwFlags & TTBBF_ASPUSHBUTTON)?1:0, 0);
	}
	if (BitChanged(TTBBF_SHOWTOOLTIP)) {
		dwFlags^=TTBBF_SHOWTOOLTIP;
		SendMessage(hwnd,BUTTONADDTOOLTIP,
			(WPARAM)((dwFlags & TTBBF_SHOWTOOLTIP)?tooltip:L""),BATF_UNICODE);
	}
	// next settings changing visual side, requires additional actions
	if (BitChanged(TTBBF_VISIBLE)) {
		dwFlags^=TTBBF_VISIBLE;
		res |= TTBBF_VISIBLE;
	}
	if (BitChanged(TTBBF_PUSHED)) {
		dwFlags^=TTBBF_PUSHED;
		res |= TTBBF_PUSHED;
		bPushed = (dwFlags & TTBBF_PUSHED) ? TRUE : FALSE;
	}
	return res;
}

void TopButtonInt::CreateWnd()
{
	if ( !(dwFlags & TTBBF_ISSEPARATOR)) {
		hwnd = CreateWindow(MIRANDABUTTONCLASS, _T(""), BS_PUSHBUTTON|WS_CHILD|WS_TABSTOP|SS_NOTIFY, 0, 0, BUTTWIDTH, BUTTHEIGHT, hwndTopToolBar, NULL, hInst, 0);

		if (dwFlags & TTBBF_ASPUSHBUTTON)
			SendMessage(hwnd, BUTTONSETASPUSHBTN, 1, 0);

		if (DBGetContactSettingByte(0, TTB_OPTDIR, "UseFlatButton", 1))
			SendMessage(hwnd, BUTTONSETASFLATBTN, TRUE, 0);

	  EnableWindow(hwnd,(dwFlags & TTBBF_DISABLED)?FALSE:TRUE);
	}
	// maybe SEPWIDTH, not BUTTWIDTH?
	else 
		hwnd = CreateWindow( _T("STATIC"), _T(""), WS_CHILD|SS_NOTIFY, 0, 0, BUTTWIDTH, BUTTHEIGHT, hwndTopToolBar, NULL, hInst, 0);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, id);
	SetBitmap();
}

void TopButtonInt::LoadSettings()
{
	char buf[255];

	BYTE oldv = (dwFlags & TTBBF_VISIBLE) != 0;
	dwFlags = dwFlags & (~TTBBF_VISIBLE);

	if (dwFlags & TTBBF_ISSEPARATOR) {
		char buf1[10];
		_itoa(wParamDown, buf1, 10);
		char buf2[20];
		AS(buf2, "Sep", buf1);

		arrangedpos = DBGetContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), Buttons.getCount());
		if ( DBGetContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), oldv) > 0 )
			dwFlags |= TTBBF_VISIBLE;
	}
	else if ((dwFlags & TTBBF_ISLBUTTON ) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(wParamDown, buf1, 10);
		char buf2[20];
		AS(buf2, "Launch", buf1);

		if (name != NULL) free(name);
		name = DBGetString(0, TTB_OPTDIR, AS(buf, buf2, "_name"));
		if (program != NULL) free(program);
		program = DBGetStringT(0, TTB_OPTDIR, AS(buf, buf2, "_lpath"));

		arrangedpos = DBGetContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), Buttons.getCount());
		if ( DBGetContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), oldv) > 0 )
			dwFlags |= TTBBF_VISIBLE;
	}
	else {
		arrangedpos = DBGetContactSettingByte(0, TTB_OPTDIR, AS(buf, name, "_Position"), Buttons.getCount());
		if ( DBGetContactSettingByte(0, TTB_OPTDIR, AS(buf, name, "_Visible"), oldv) > 0 )
			dwFlags |= TTBBF_VISIBLE;
	}
}

void TopButtonInt::SaveSettings(int *SepCnt, int *LaunchCnt)
{
	char buf[255];

	if (SepCnt && (dwFlags & TTBBF_ISSEPARATOR) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(SepCnt[0]++, buf1, 10);
		char buf2[20];
		AS(buf2, "Sep", buf1);

		DBWriteContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), arrangedpos);
		DBWriteContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), dwFlags & TTBBF_VISIBLE);
	}
	else if (LaunchCnt && (dwFlags & TTBBF_ISLBUTTON ) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(LaunchCnt[0]++, buf1, 10);
		char buf2[20];
		AS(buf2, "Launch", buf1);

		DBWriteContactSettingString(0, TTB_OPTDIR, AS(buf, buf2, "_name"), name);
		DBWriteContactSettingTString(0, TTB_OPTDIR, AS(buf, buf2, "_lpath"), program);
		DBWriteContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Position"), arrangedpos);
		DBWriteContactSettingByte(0, TTB_OPTDIR, AS(buf, buf2, "_Visible"), dwFlags & TTBBF_VISIBLE);
	}
	else {
		DBWriteContactSettingByte(0, TTB_OPTDIR, AS(buf, name, "_Position"), arrangedpos);
		DBWriteContactSettingByte(0, TTB_OPTDIR, AS(buf, name, "_Visible"), dwFlags & TTBBF_VISIBLE);
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
		SendMessage(hwnd, BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM)hBmpSeparator);
	}
	else {
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & SS_ICON)
			SetWindowLongPtr(hwnd, GWL_STYLE, curstyle | SS_ICON);

		HICON bicon = (hIconDn)?hIconDn:hIconUp;

		SendMessage(hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)((bPushed)?(bicon):(hIconUp)));
		SendMessage(hwnd, BM_SETCHECK, bPushed?BST_CHECKED:BST_UNCHECKED ,0);
	}
}
