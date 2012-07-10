/* 
Copyright (C) 2008 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "commons.h"

#include "options.h"



// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;


static BOOL CALLBACK SkinOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


// Functions //////////////////////////////////////////////////////////////////////////////////////


int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.pszGroup = "Skins";
	odp.pszTab = "Skin";
	odp.pfnDlgProc = SkinOptDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SKIN_OPT);
	odp.flags = ODPF_BOLDGROUPS;

	for(unsigned int i = 0; i < dlgs.size(); i++)
	{
		MirandaSkinnedDialog * dlg = dlgs[i];
		odp.pszTitle = (char *) dlg->getDescription(); // Yeah, yeah, I know...
		odp.dwInitParam = (LPARAM) dlg;
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	}

	return 0;
}


void InitOptions()
{
	LoadOptions();
	
	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


void LoadOptions()
{
}

static void GetTextMetric(HFONT hFont, TEXTMETRIC *tm)
{
	HDC hdc = GetDC(NULL);
	HFONT hOldFont = (HFONT) SelectObject(hdc, hFont);
	GetTextMetrics(hdc, tm);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
}


static BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}


#define V_SPACE 5
#define H_INITIAL_SPACE 10
#define MAX_TEXT_SIZE 128

static BOOL CALLBACK SkinOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			MirandaSkinnedDialog * dlg = (MirandaSkinnedDialog *) lParam;
			_ASSERT(dlg != NULL);
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) NULL);

			std::vector<std::tstring> skins;
			getAvaiableSkins(skins, dlg);
			for(unsigned int i = 0; i < skins.size(); i++)
			{
				std::tstring &sk = skins[i];
				SendDlgItemMessage(hwndDlg, IDC_SKIN, CB_ADDSTRING, 0, (LONG) skins[i].c_str());
			}
			SendDlgItemMessage(hwndDlg, IDC_SKIN, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)dlg->getSkinName());

			HWND skinOptsLabel = GetDlgItem(hwndDlg, IDC_SKIN_OPTS_L);

			SkinOptions *opts = dlg->getOpts();
			if (opts == NULL)
			{
				ShowWindow(skinOptsLabel, SW_HIDE);
				ShowScrollBar(hwndDlg, SB_VERT, FALSE);
			}
			else
			{
				HWND skinLabel = GetDlgItem(hwndDlg, IDC_SKIN_L);
				HWND skinCombo = GetDlgItem(hwndDlg, IDC_SKIN);

				RECT labelRc = {0};
				GetWindowRect(skinLabel, &labelRc);
				ScreenToClient(hwndDlg, &labelRc);
				labelRc.left += H_INITIAL_SPACE;

				RECT valueRc = {0};
				GetWindowRect(skinCombo, &valueRc);
				ScreenToClient(hwndDlg, &valueRc);

				RECT lineRc = {0};
				GetWindowRect(skinOptsLabel, &lineRc);
				ScreenToClient(hwndDlg, &lineRc);

				HFONT hFont = (HFONT) SendMessage(hwndDlg, WM_GETFONT, 0, 0);
				TEXTMETRIC font;
				GetTextMetric(hFont, &font);

				int lineHeight = max(font.tmHeight, 16) + 4;
				int y = lineRc.bottom + V_SPACE;
				int id = IDC_SKIN_OPTS_L + 1;

				for (unsigned int i = 0; i < opts->getNumOptions(); i++)
				{
					SkinOption *opt = opts->getOption(i);

					switch(opt->getType())
					{
						case CHECKBOX:
						{
							HWND chk = CreateWindow(_T("BUTTON"), opt->getDescription(), 
									WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX, 
									labelRc.left, y, 
									lineRc.right - labelRc.left, 
									lineHeight, hwndDlg, (HMENU) id, hInst, NULL);
							SendMessage(chk, BM_SETCHECK, opt->getValueCheckbox() ? BST_CHECKED : BST_UNCHECKED, 0);
							SendMessage(chk, WM_SETFONT, (WPARAM) hFont, FALSE);

							break;
						}
						case NUMBER:
						{
							std::tstring tmp = opt->getDescription();
							tmp += _T(":");
							HWND lbl = CreateWindow(_T("STATIC"), tmp.c_str(), 
									WS_CHILD | WS_VISIBLE, 
									labelRc.left, y + (lineHeight - font.tmHeight) / 2, 
									labelRc.right - labelRc.left, font.tmHeight, 
									hwndDlg, (HMENU) id + 2, hInst, NULL);
							SendMessage(lbl, WM_SETFONT, (WPARAM) hFont, FALSE);

							HWND edit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
									WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, 
									valueRc.left, y, 
									(valueRc.right - valueRc.left) / 2, lineHeight, 
									hwndDlg, (HMENU) id, hInst, NULL);
							SendMessage(edit, WM_SETFONT, (WPARAM) hFont, FALSE);
							SendMessage(edit, EM_LIMITTEXT, 10, 0);

							HWND spin = CreateWindow(UPDOWN_CLASS, NULL, 
									WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_NOTHOUSANDS | UDS_HOTTRACK, 
									valueRc.left, y, 
									1, 1, 
									hwndDlg, (HMENU) (id + 1), hInst, NULL);
							SendMessage(spin, WM_SETFONT, (WPARAM) hFont, FALSE);
							SendMessage(spin, UDM_SETBUDDY, (WPARAM) edit, 0);
							SendMessage(spin, UDM_SETRANGE, 0, MAKELONG(min(0x7fff, opt->getMax()), max(-0x7fff, min(0x7fff, opt->getMin()))));
							SendMessage(spin, UDM_SETPOS, 0, MAKELONG(opt->getValueNumber(), 0));

							break;
						}
						case TEXT:
						{
							std::tstring tmp = opt->getDescription();
							tmp += _T(":");
							HWND lbl = CreateWindow(_T("STATIC"), tmp.c_str(), 
									WS_CHILD | WS_VISIBLE, 
									labelRc.left, y + (lineHeight - font.tmHeight) / 2, 
									labelRc.right - labelRc.left, font.tmHeight, 
									hwndDlg, (HMENU) id + 1, hInst, NULL);
							SendMessage(lbl, WM_SETFONT, (WPARAM) hFont, FALSE);

							HWND edit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
									WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 
									valueRc.left, y, 
									lineRc.right - valueRc.left, lineHeight, 
									hwndDlg, (HMENU) id, hInst, NULL);
							SendMessage(edit, WM_SETFONT, (WPARAM) hFont, FALSE);
							SendMessage(edit, EM_LIMITTEXT, MAX_TEXT_SIZE, 0);

							SetWindowText(edit, opt->getValueText());

							break;
						}
					}

					id += 3;
					y += lineHeight + V_SPACE;
				}

				RECT rc = {0};
				GetClientRect(hwndDlg, &rc);
				int avaiable = rc.bottom - rc.top;
				int total = y - V_SPACE;
				int current = 0;

				SCROLLINFO si; 
				si.cbSize = sizeof(si); 
				si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
				si.nMin   = 0; 
				si.nMax   = total; 
				si.nPage  = avaiable; 
				si.nPos   = current; 
				SetScrollInfo(hwndDlg, SB_VERT, &si, TRUE); 
			}

			TranslateDialogDefault(hwndDlg);

			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) dlg);

			break;
		}

		case WM_VSCROLL: 
		{ 
			if (lParam != 0)
				break;

			SCROLLINFO si = {0};
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
			GetScrollInfo(hwndDlg, SB_VERT, &si);

			int total = si.nMax;
			int avaiable = si.nPage;
			int current = si.nPos;

			HFONT hFont = (HFONT) SendMessage(hwndDlg, WM_GETFONT, 0, 0);
			TEXTMETRIC font;
			GetTextMetric(hFont, &font);
			int lineHeight = max(font.tmHeight, 16) + 4;

			int yDelta;     // yDelta = new_pos - current_pos 
			int yNewPos;    // new position 
 
			switch (LOWORD(wParam)) 
			{ 
				case SB_PAGEUP: 
					yNewPos = current - avaiable / 2; 
					break;  
				case SB_PAGEDOWN: 
					yNewPos = current + avaiable / 2; 
					break; 
				case SB_LINEUP: 
					yNewPos = current - lineHeight; 
					break; 
				case SB_LINEDOWN: 
					yNewPos = current + lineHeight; 
					break; 
				case SB_THUMBPOSITION: 
					yNewPos = HIWORD(wParam); 
					break; 
				case SB_THUMBTRACK:
					yNewPos = HIWORD(wParam); 
					break;
				default: 
					yNewPos = current; 
			} 

			yNewPos = min(total - avaiable, max(0, yNewPos)); 
 
			if (yNewPos == current) 
				break; 
 
			yDelta = yNewPos - current; 
			current = yNewPos; 
 
			// Scroll the window. (The system repaints most of the 
			// client area when ScrollWindowEx is called; however, it is 
			// necessary to call UpdateWindow in order to repaint the 
			// rectangle of pixels that were invalidated.) 
 
			ScrollWindowEx(hwndDlg, 0, -yDelta, (CONST RECT *) NULL, 
				(CONST RECT *) NULL, (HRGN) NULL, (LPRECT) NULL, 
				/* SW_ERASE | SW_INVALIDATE | */ SW_SCROLLCHILDREN); 
			UpdateWindow(hwndDlg); 
			InvalidateRect(hwndDlg, NULL, TRUE);
 
			// Reset the scroll bar. 
 
			si.fMask  = SIF_POS; 
			si.nPos   = current; 
			SetScrollInfo(hwndDlg, SB_VERT, &si, TRUE); 

			break; 
		}

		case WM_COMMAND:
		{
			MirandaSkinnedDialog * dlg = (MirandaSkinnedDialog *) GetWindowLong(hwndDlg, GWL_USERDATA);
			if (dlg == NULL)
				break;

			SkinOptions *opts = dlg->getOpts();

			if (LOWORD(wParam) == IDC_SKIN)
			{
				if (HIWORD(wParam) == CBN_SELCHANGE && (HWND)lParam == GetFocus())
				{
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

					// Disable all options
					if (opts != NULL)
					{
						int id = IDC_SKIN_OPTS_L + 1;
						for (unsigned int i = 0; i < opts->getNumOptions(); i++)
						{
							EnableWindow(GetDlgItem(hwndDlg, id), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, id+1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, id+2), FALSE);
							id += 3;
						}
					}
				}
				break;
			}

			if (opts == NULL)
				break;

			bool changed = false;
			int id = IDC_SKIN_OPTS_L + 1;
			for (unsigned int i = 0; i < opts->getNumOptions() && !changed; i++)
			{
				SkinOption *opt = opts->getOption(i);

				if (LOWORD(wParam) == id)
				{
					switch(opt->getType())
					{
						case CHECKBOX:
						{
							changed = true;
							break;
						}
						case NUMBER:
						case TEXT:
						{
							// Don't make apply enabled during buddy set
							if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
								changed = true;
							break;
						}
					}
				}

				id += 3;
			}

			if (changed)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR) lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				MirandaSkinnedDialog * dlg = (MirandaSkinnedDialog *) GetWindowLong(hwndDlg, GWL_USERDATA);
				if (dlg == NULL)
					break;

				bool changedSkin = false;

				// TODO Correctly handle changing skins
				int pos = SendDlgItemMessage(hwndDlg, IDC_SKIN, CB_GETCURSEL, 0, 0);
				if (pos != CB_ERR)
				{
					TCHAR tmp[1024];
					GetWindowText(GetDlgItem(hwndDlg, IDC_SKIN), tmp, MAX_REGS(tmp));

					changedSkin = (lstrcmp(dlg->getSkinName(), tmp) != 0);

					dlg->setSkinName(tmp);
				}

				SkinOptions *opts = dlg->getOpts();
				if (opts != NULL && !changedSkin)
				{
					int id = IDC_SKIN_OPTS_L + 1;
					for (unsigned int i = 0; i < opts->getNumOptions(); i++)
					{
						SkinOption *opt = opts->getOption(i);

						switch(opt->getType())
						{
							case CHECKBOX:
							{
								opt->setValueCheckbox(IsDlgButtonChecked(hwndDlg, id) != 0);
								break;
							}
							case NUMBER:
							{
								opt->setValueNumber(SendDlgItemMessage(hwndDlg, id + 1, UDM_GETPOS, 0, 0));
								break;
							}
							case TEXT:
							{
								TCHAR tmp[MAX_TEXT_SIZE];
								GetDlgItemText(hwndDlg, id, tmp, MAX_TEXT_SIZE);
								opt->setValueText(tmp);
								break;
							}
						}

						id += 3;
					}

					dlg->storeToDB(opts);
				}

				return TRUE;
			}
			break;
		}
	}

	return 0;
}
