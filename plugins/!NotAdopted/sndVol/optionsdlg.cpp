#include "plugin.h"
#include "playSnd.h"
#include "resource.h"
#include "optionsdlg.h"

#include <math.h>

#pragma comment(lib, "comctl32.lib")

extern void SetRadioVolume(int value);
extern COLORREF g_clrback;
extern HBRUSH g_HBRback;
extern int g_mRadioMuted;
extern HWND hwndMRadio;
extern HICON hIconMRadio, hIconMRadio_off;

void LoadBackgroundSettings()
{
	HWND hwnd;

	if(g_HBRback)
		DeleteObject(g_HBRback);

	if(DBGetContactSettingByte(NULL, SERVICENAME, "def_bg", 1))
		g_clrback = GetSysColor(COLOR_3DFACE);
	else
		g_clrback = DBGetContactSettingDword(NULL, SERVICENAME, "bgc", GetSysColor(COLOR_3DFACE));

	g_HBRback = CreateSolidBrush(g_clrback);
	hwnd = GetFocus();
	InvalidateRect(hwndSlider, NULL, TRUE);
	SetFocus(hwndSlider);
	RedrawWindow(hwndFrame, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
	SetFocus(hwnd);
}

LRESULT CALLBACK GroupWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
		case WM_LBUTTONDOWN:
			::MessageBox(0,_T("LB"),_T(""),0);
			return 0;
		case WM_PAINT:
		{
			CallWindowProc((WNDPROC)GetWindowLong(hwnd, GWL_USERDATA), hwnd, uMsg, wParam, lParam);

			HDC hdc = GetDC(hwnd);
			RECT rc;
			TCHAR szTitle[256];
			HFONT oldfont;
			int oldBk;
				ZeroMemory(&rc, sizeof(rc));
				// Determine a size of the text

				oldBk = SetBkMode(hdc, TRANSPARENT);
				oldfont = (HFONT)SelectObject(hdc, (HFONT)SendMessage(hwnd, WM_GETFONT, 0,0));
				SetBkMode(hdc, TRANSPARENT);
				GetWindowText(hwnd, szTitle, sizeof(szTitle) / sizeof(TCHAR));
				DrawText(hdc, szTitle, lstrlen(szTitle), &rc, DT_SINGLELINE|DT_CALCRECT);

				rc.left += 8; rc.right += 8 + 16 + 4;

				FillRect(hdc, &rc, (HBRUSH) (COLOR_BTNFACE+1));
				DrawIconEx(hdc, rc.left+2, rc.top, hSoundOn, 16,16, 0, NULL, DI_NORMAL);
				
				rc.left += 2 + 16 + 2;
				//rc.right += 2 + 16 + 2;
				DrawText(hdc, szTitle, lstrlen(szTitle), &rc, DT_SINGLELINE|DT_WORD_ELLIPSIS);

				SetBkMode(hdc, oldBk);

				SelectObject(hdc, oldfont);

			ReleaseDC(hwnd, hdc);

			return 0;
		}
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hwnd, GWL_USERDATA), hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK OptionsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
		case WM_INITDIALOG:
		{
			HWND grp = GetDlgItem(hwndDlg, IDC_GRP);

			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_AUTO, DBGetContactSettingByte(NULL,SERVICENAME,"AutoPreview", TRUE)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MUTE, DBGetContactSettingByte(NULL,SERVICENAME,"MuteBtn", TRUE)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MRADIO, DBGetContactSettingByte(NULL, SERVICENAME, "mRadioAdjust", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MUTERADIO, DBGetContactSettingByte(NULL, SERVICENAME, "mRadioMute", 0) ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(hwndDlg, IDC_MRADIO), ServiceExists(MS_RADIO_SETVOL));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MUTERADIO), ServiceExists(MS_RADIO_SETVOL));

			hwndOptSlider = GetDlgItem(hwndDlg, IDC_VOLUME);
			SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN,SLIDER_MAX));
			SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPOS, TRUE, DBGetContactSettingDword(NULL,SERVICENAME,"Volume",100));

			SetWindowLong(grp, GWL_USERDATA, GetWindowLong(grp, GWL_WNDPROC));
			SetWindowLong(grp, GWL_WNDPROC, (LONG)GroupWndProc);
			SendDlgItemMessage(hwndDlg, IDC_BACKCOLOR, CPM_SETCOLOUR, 0, (LPARAM)DBGetContactSettingDword(NULL, SERVICENAME, "bgc", GetSysColor(COLOR_3DFACE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKCOLOR), DBGetContactSettingByte(NULL, SERVICENAME, "def_bg", 1) ? FALSE : TRUE);
			CheckDlgButton(hwndDlg, IDC_DEFCOLOR, DBGetContactSettingByte(NULL, SERVICENAME, "def_bg", 1));
			return TRUE;
		}
		case WM_HSCROLL:
		{
			int value = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
			if(hwndSlider)
				SendMessage(hwndSlider, TBM_SETPOS, TRUE, value);
			else
				DBWriteContactSettingDword(NULL, SERVICENAME, "Volume", value);
			playSnd::SetVolume(value);
			if(IsDlgButtonChecked(hwndDlg, IDC_AUTO))
				SkinPlaySound("AlertMsg");
			SetRadioVolume(value);
			break;
		}			
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_TEST) {
				playSnd::SetVolume((DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0));
				SkinPlaySound("AlertMsg");
				return FALSE;
			}
			else if(LOWORD(wParam) == IDC_MRADIO) {
				DBWriteContactSettingByte(NULL, SERVICENAME, "mRadioAdjust", IsDlgButtonChecked(hwndDlg, IDC_MRADIO) ? 1 : 0);
				if(IsDlgButtonChecked(hwndDlg, IDC_MRADIO) && ServiceExists(MS_RADIO_SETVOL)) {
					int value = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
					CallService(MS_RADIO_SETVOL, (WPARAM)value, 0);
				}
			}
			else if(LOWORD(wParam) == IDC_MUTERADIO) {
				int useSound = DBGetContactSettingByte(NULL, "Skin", "UseSound", 1);
				int value;
				DBWriteContactSettingByte(NULL, SERVICENAME, "mRadioMute", IsDlgButtonChecked(hwndDlg, IDC_MUTERADIO) ? 1 : 0);
				if(ServiceExists(MS_RADIO_SETVOL)) {
					if(IsDlgButtonChecked(hwndDlg, IDC_MUTERADIO))
						g_mRadioMuted = !useSound;
					value = g_mRadioMuted ? 0 : (int)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
					CallService(MS_RADIO_SETVOL, (WPARAM)value, 0);
					DBWriteContactSettingByte(NULL, "mRadio", "Volume", (BYTE)value);
					SendMessage(hwndMRadio, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(g_mRadioMuted ? hIconMRadio_off : hIconMRadio));
				}
			}
			else if(LOWORD(wParam) == IDC_DEFCOLOR) {
				DBWriteContactSettingByte(NULL, SERVICENAME, "def_bg", IsDlgButtonChecked(hwndDlg, IDC_DEFCOLOR) ? 1 : 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BACKCOLOR), DBGetContactSettingByte(NULL, SERVICENAME, "def_bg", 1) ? FALSE : TRUE);
				LoadBackgroundSettings();
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0,0);			
			break;
			
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)	{
				case NM_RELEASEDCAPTURE:
					if(IsDlgButtonChecked(hwndDlg, IDC_AUTO) == BST_CHECKED) {
						int value = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
						if(hwndSlider) 
							SendMessage(hwndSlider, TBM_SETPOS, TRUE, value);

						playSnd::SetVolume(value);
						SkinPlaySound("AlertMsg");
						SetRadioVolume(value);
					}
					break;
				case PSN_APPLY:
				{
					int value = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
					if(hwndSlider) 
						SendMessage(hwndSlider, TBM_SETPOS, TRUE, value);
					playSnd::SetVolume(value);

					DBWriteContactSettingDword(NULL, SERVICENAME, "Volume", value);
					autoPreview = (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_AUTO) == BST_CHECKED);
					DBWriteContactSettingByte(NULL, SERVICENAME, "AutoPreview", autoPreview);
					value = (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_MUTE) == BST_CHECKED);
					DBWriteContactSettingByte(NULL, SERVICENAME, "MuteBtn", value);
					PostMessage(hwndFrame, WM_USER, value, 0);
					DBWriteContactSettingDword(NULL, SERVICENAME, "bgc", (DWORD)SendDlgItemMessage(hwndDlg, IDC_BACKCOLOR, CPM_GETCOLOUR, 0, 0));
					LoadBackgroundSettings();
					return TRUE;
				}
				case PSN_RESET:
				{
					int value = DBGetContactSettingDword(NULL,SERVICENAME,"Volume",100);
					SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPOS, TRUE, value);
					if(hwndSlider) 
						SendMessage(hwndSlider, TBM_SETPOS, TRUE, value);
					playSnd::SetVolume(value);
					return TRUE;
				}
			}
			break;

		case WM_DESTROY:
			hwndOptSlider = NULL;
			return FALSE;
	}
	return FALSE;
}
