
#include "stdafx.h"

#define BitChanged(c) (dwFlags ^ Flags) & c

static int maxid = 10000;

TopButtonInt::~TopButtonInt()
{
	if (hwnd)
		DestroyWindow(hwnd);

	if (dwFlags & TTBBF_ISLBUTTON) {
		mir_free(ptszProgram);
	}
	else if (pszService != nullptr)
		mir_free(pszService);

	mir_free(pszName);
	mir_free(ptszTooltip);
	mir_free(ptszTooltipDn);
	mir_free(ptszTooltipUp);
}

uint32_t TopButtonInt::CheckFlags(uint32_t Flags)
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
		SendMessage(hwnd, BUTTONADDTOOLTIP, (WPARAM)((dwFlags & TTBBF_SHOWTOOLTIP) ? ptszTooltip : L""), BATF_UNICODE);
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
		hwnd = CreateWindow(TTB_BUTTON_CLASS, L"", BS_PUSHBUTTON | WS_CHILD | WS_TABSTOP | SS_NOTIFY, 0, 0, g_ctrl->nButtonWidth, g_ctrl->nButtonHeight, g_ctrl->hWnd, nullptr, g_plugin.getInst(), this);

		if (dwFlags & TTBBF_ASPUSHBUTTON)
			SendMessage(hwnd, BUTTONSETASPUSHBTN, 1, 0);

		if (g_plugin.getByte("UseFlatButton", 1))
			SendMessage(hwnd, BUTTONSETASFLATBTN, TRUE, 0);

		EnableWindow(hwnd, (dwFlags & TTBBF_DISABLED) ? FALSE : TRUE);
	}
	// maybe SEPWIDTH, not g_ctrl->nButtonWidth?
	else
		hwnd = CreateWindow(L"STATIC", L"", WS_CHILD | SS_NOTIFY, 0, 0, g_ctrl->nButtonWidth, g_ctrl->nButtonHeight, g_ctrl->hWnd, nullptr, g_plugin.getInst(), nullptr);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, id);
	SetBitmap();
}

void TopButtonInt::LoadSettings()
{
	char buf[255];

	uint8_t oldv = isVisible();
	dwFlags = dwFlags & (~TTBBF_VISIBLE);

	if (dwFlags & TTBBF_ISSEPARATOR) {
		char buf1[10];
		_itoa(wParamDown, buf1, 10);
		char buf2[20];
		AS(buf2, "Sep", buf1);

		arrangedpos = g_plugin.getByte(AS(buf, buf2, "_Position"), Buttons.getCount());
		if (g_plugin.getByte(AS(buf, buf2, "_Visible"), oldv) > 0)
			dwFlags |= TTBBF_VISIBLE;
	}
	else if ((dwFlags & TTBBF_ISLBUTTON ) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(wParamDown, buf1, 10);
		char buf2[20];
		AS(buf2, "Launch", buf1);

		mir_free(pszName);
		pszName = g_plugin.getStringA(AS(buf, buf2, "_name"));
		
		mir_free(ptszProgram);
		ptszProgram = g_plugin.getWStringA(AS(buf, buf2, "_lpath"));

		arrangedpos = g_plugin.getByte(AS(buf, buf2, "_Position"), Buttons.getCount());
		if (g_plugin.getByte(AS(buf, buf2, "_Visible"), oldv) > 0)
			dwFlags |= TTBBF_VISIBLE;
	}
	else {
		arrangedpos = g_plugin.getByte(AS(buf, pszName, "_Position"), Buttons.getCount());
		if (g_plugin.getByte(AS(buf, pszName, "_Visible"), oldv) > 0)
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

		g_plugin.setByte(AS(buf, buf2, "_Position"), arrangedpos);
		g_plugin.setByte(AS(buf, buf2, "_Visible"), isVisible());
	}
	else if (LaunchCnt && (dwFlags & TTBBF_ISLBUTTON ) && (dwFlags & TTBBF_INTERNAL)) {
		char buf1[10];
		_itoa(LaunchCnt[0]++, buf1, 10);
		char buf2[20];
		AS(buf2, "Launch", buf1);

		g_plugin.setString(AS(buf, buf2, "_name"), pszName);
		g_plugin.setWString(AS(buf, buf2, "_lpath"), ptszProgram);
		g_plugin.setByte(AS(buf, buf2, "_Position"), arrangedpos);
		g_plugin.setByte(AS(buf, buf2, "_Visible"), isVisible());
	}
	else {
		g_plugin.setByte(AS(buf, pszName, "_Position"), arrangedpos);
		g_plugin.setByte(AS(buf, pszName, "_Visible"), isVisible());
	}
}

void TopButtonInt::SetBitmap()
{
	int curstyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	curstyle &= (~SS_BITMAP);
	curstyle &= (~SS_ICON);

	if (dwFlags & TTBBF_ISSEPARATOR) {
		SetWindowLongPtr(hwnd, GWL_STYLE, curstyle | SS_ICON);
		SendMessage(hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(iconList[9].hIcolib));
	}
	else {
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & SS_ICON)
			SetWindowLongPtr(hwnd, GWL_STYLE, curstyle | SS_ICON);

		wchar_t *pTooltip;
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
			SendMessage(hwnd, BUTTONADDTOOLTIP, (WPARAM)TranslateW_LP(pTooltip, pPlugin), BATF_UNICODE);
	}
}
