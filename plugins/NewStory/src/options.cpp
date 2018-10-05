#include "stdafx.h"

Options options;
int OptShowPage = 0;

#undef MODULENAME
#define MODULENAME "PopUp"

void LoadOptions()
{
	return;
}

static int OptionsInitialize(WPARAM wParam, LPARAM lParam);

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
}

BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int OptionsInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.w = TranslateT("Newstory");
	//	odp.pszGroup = Translate("PopUps");
	odp.position = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptionsDlgProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

static void ThemeDialogBackground(HWND hwnd) {
	//if (IsWinVerXPPlus())
	{
		static HMODULE hThemeAPI = NULL;
		if (!hThemeAPI) hThemeAPI = GetModuleHandle(_T("uxtheme"));
		if (hThemeAPI) {
			HRESULT(STDAPICALLTYPE *MyEnableThemeDialogTexture)(HWND, DWORD) = (HRESULT(STDAPICALLTYPE*)(HWND, DWORD))GetProcAddress(hThemeAPI, "EnableThemeDialogTexture");
			if (MyEnableThemeDialogTexture)
				MyEnableThemeDialogTexture(hwnd, 0x00000002 | 0x00000004); //0x00000002|0x00000004=ETDT_ENABLETAB
		}
	}
}

BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TCITEM tci;
		RECT rc;

		tci.mask = TCIF_PARAM | TCIF_TEXT;
		tci.lParam = (LPARAM)CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_OPT_PASSWORDS), GetDlgItem(hwnd, IDC_TAB), OptPasswordsDlgProc);
		tci.pszText = TranslateT("Passwords (not ready yet)");
		if (tci.lParam)	TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_TAB), 0, &tci);
		GetClientRect(GetDlgItem(hwnd, IDC_TAB), &rc);
		TabCtrl_AdjustRect(GetDlgItem(hwnd, IDC_TAB), FALSE, &rc);
		ThemeDialogBackground((HWND)tci.lParam);
		MoveWindow((HWND)tci.lParam, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		ShowWindow((HWND)tci.lParam, (OptShowPage == 2) ? SW_SHOW : SW_HIDE);

		tci.mask = TCIF_PARAM | TCIF_TEXT;
		tci.lParam = (LPARAM)CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_OPT_TEMPLATES), GetDlgItem(hwnd, IDC_TAB), OptTemplatesDlgProc);
		tci.pszText = TranslateT("Templates");
		if (tci.lParam)	TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_TAB), 0, &tci);
		GetClientRect(GetDlgItem(hwnd, IDC_TAB), &rc);
		TabCtrl_AdjustRect(GetDlgItem(hwnd, IDC_TAB), FALSE, &rc);
		ThemeDialogBackground((HWND)tci.lParam);
		MoveWindow((HWND)tci.lParam, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		ShowWindow((HWND)tci.lParam, (OptShowPage == 1) ? SW_SHOW : SW_HIDE);

		tci.mask = TCIF_PARAM | TCIF_TEXT;
		tci.lParam = (LPARAM)CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_OPT_GENERAL), GetDlgItem(hwnd, IDC_TAB), 0);
		tci.pszText = TranslateT("General");
		if (tci.lParam)	TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_TAB), 0, &tci);
		GetClientRect(GetDlgItem(hwnd, IDC_TAB), &rc);
		TabCtrl_AdjustRect(GetDlgItem(hwnd, IDC_TAB), FALSE, &rc);
		ThemeDialogBackground((HWND)tci.lParam);
		MoveWindow((HWND)tci.lParam, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		ShowWindow((HWND)tci.lParam, (OptShowPage == 0) ? SW_SHOW : SW_HIDE);

		TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_TAB), OptShowPage);
		OptShowPage = 0;
		return FALSE;
	}

	case PSM_CHANGED:
	{
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		break;
	}

	case WM_DESTROY:
	{
		OptShowPage = 0;
		break;
	}

	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case 0:
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_RESET:
			case PSN_APPLY:
			{
				TCITEM tci;
				int i, count;
				tci.mask = TCIF_PARAM;
				count = TabCtrl_GetItemCount(GetDlgItem(hwnd, IDC_TAB));
				for (i = 0; i < count; i++)
				{
					TabCtrl_GetItem(GetDlgItem(hwnd, IDC_TAB), i, &tci);
					SendMessage((HWND)tci.lParam, WM_NOTIFY, wParam, lParam);
				}
				break;
			}
			}
			break;
		}

		case IDC_TAB:
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case TCN_SELCHANGING:
			{
				TCITEM tci;
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwnd, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB)), &tci);
				ShowWindow((HWND)tci.lParam, SW_HIDE);
				break;
			}
			case TCN_SELCHANGE:
			{
				TCITEM tci;
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwnd, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB)), &tci);
				ShowWindow((HWND)tci.lParam, SW_SHOW);
				break;
			}
			}
			break;
		}
		}
		break;
	}
	}
	return FALSE;
}
