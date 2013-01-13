#include "plugin.h"
#include "playSnd.h"
#include "resource.h"
#include "optionsdlg.h"

#include <math.h>

#pragma comment(lib, "comctl32.lib")


LRESULT CALLBACK GroupWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
		case WM_LBUTTONDOWN:
			::MessageBox(0,"LB","",0);
			return 0;
		case WM_PAINT:
		{
			CallWindowProc((WNDPROC)GetWindowLong(hwnd, GWL_USERDATA), hwnd, uMsg, wParam, lParam);

			HDC hdc = GetDC(hwnd);
			RECT rc;
			char szTitle[256];
			HFONT oldfont;
			int oldBk;
				ZeroMemory(&rc, sizeof(rc));
				// Determine a size of the text

				oldBk = SetBkMode(hdc, TRANSPARENT);
				oldfont = (HFONT)SelectObject(hdc, (HFONT)SendMessage(hwnd, WM_GETFONT, 0,0));
				SetBkMode(hdc, TRANSPARENT);
				GetWindowText(hwnd, szTitle, sizeof(szTitle));
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
//
// OptionsDlgProc()
// this handles the options page
// verwaltet die Optionsseite
//
LRESULT CALLBACK OptionsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			HWND grp = GetDlgItem(hwndDlg, IDC_GRP);

			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_AUTO, DBGetContactSettingByte(NULL,SERVICENAME,"AutoPreview", TRUE)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MUTE, DBGetContactSettingByte(NULL,SERVICENAME,"MuteBtn", TRUE)?BST_CHECKED:BST_UNCHECKED);

			hwndOptSlider = GetDlgItem(hwndDlg, IDC_VOLUME);
			SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN,SLIDER_MAX));
			SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPOS, TRUE, DBGetContactSettingDword(NULL,SERVICENAME,"Volume",100));

			SetWindowLong(grp, GWL_USERDATA, GetWindowLong(grp, GWL_WNDPROC));
			SetWindowLong(grp, GWL_WNDPROC, (LONG)GroupWndProc);

			return TRUE;
		}
		case WM_HSCROLL:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_TEST)
			{
				//playSnd::g_bInOption = TRUE;
				playSnd::SetVolume((DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0));
				SkinPlaySound("RecvMsg");
				//playSnd::g_bInOption = FALSE;
				return FALSE;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0,0);			
			break;
			
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				case NM_RELEASEDCAPTURE:
					if(IsDlgButtonChecked(hwndDlg, IDC_AUTO) == BST_CHECKED)
					{
						int value = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
						if(hwndSlider) SendMessage(hwndSlider, TBM_SETPOS, TRUE, value/SLIDER_DIV);

						//playSnd::g_bInOption = TRUE;
						playSnd::SetVolume(value);
						SkinPlaySound("RecvMsg");
						//playSnd::g_bInOption = FALSE;
					}
					break;
				case PSN_APPLY:
				{
					int value = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
					if(hwndSlider) SendMessage(hwndSlider, TBM_SETPOS, TRUE, value/SLIDER_DIV);
					playSnd::SetVolume(value);
					DBWriteContactSettingDword(NULL, SERVICENAME, "Volume", value);
					autoPreview = (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_AUTO) == BST_CHECKED);
					DBWriteContactSettingByte(NULL, SERVICENAME, "AutoPreview", autoPreview);
					value = (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_MUTE) == BST_CHECKED);
					DBWriteContactSettingByte(NULL, SERVICENAME, "MuteBtn", value);
					PostMessage(hwndFrame, WM_USER, value, 0);

					return TRUE;
				}
				case PSN_RESET:
				{
					int value = DBGetContactSettingDword(NULL,SERVICENAME,"Volume",100);
					SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPOS, TRUE, value);
					if(hwndSlider) SendMessage(hwndSlider, TBM_SETPOS, TRUE, value/SLIDER_DIV);
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
