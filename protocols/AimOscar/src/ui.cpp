/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HANDLE hThemeButton = NULL;
COLORREF foreground=0;
COLORREF background=0xffffff;
COLORREF custColours[16]={0};

static int CALLBACK EnumFontsProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX* /*lpntme*/, int /*FontType*/, LPARAM lParam)
{
	if (!IsWindow((HWND) lParam))
		return FALSE;
	if (SendMessage((HWND) lParam, CB_FINDSTRINGEXACT, 1, (LPARAM) lpelfe->elfLogFont.lfFaceName) == CB_ERR)
		SendMessage((HWND) lParam, CB_ADDSTRING, 0, (LPARAM) lpelfe->elfLogFont.lfFaceName);
	return TRUE;
}

void DrawMyControl(HDC hDC, HWND /*hwndButton*/, HANDLE hTheme, UINT iState, RECT rect)
{
	BOOL bIsPressed = (iState & ODS_SELECTED);
	BOOL bIsFocused  = (iState & ODS_FOCUS);
	if (hTheme)
	{
		DWORD state = (bIsPressed)?PBS_PRESSED:PBS_NORMAL;
		if (state == PBS_NORMAL)
		{
			if (bIsFocused)
				state = PBS_DEFAULTED;
		}
		rect.top-=1;
		rect.left-=1;
		DrawThemeBackground(hTheme, hDC, BP_PUSHBUTTON,state, &rect, NULL);
	}
	else
	{
		if (bIsFocused)
		{
			HBRUSH br = CreateSolidBrush(RGB(0,0,0));
			FrameRect(hDC, &rect, br);
			InflateRect(&rect, -1, -1);
			DeleteObject(br);
		} // if
		COLORREF crColor = GetSysColor(COLOR_BTNFACE);
		HBRUSH	brBackground = CreateSolidBrush(crColor);
		FillRect(hDC,&rect, brBackground);
		DeleteObject(brBackground);
		// Draw pressed button
		if (bIsPressed)
		{
			HBRUSH brBtnShadow = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
			FrameRect(hDC, &rect, brBtnShadow);
			DeleteObject(brBtnShadow);
		}
		else // ...else draw non pressed button
		{
			UINT uState = DFCS_BUTTONPUSH;
			DrawFrameControl(hDC, &rect, DFC_BUTTON, uState);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

static INT_PTR CALLBACK userinfo_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAimProto* ppro = (CAimProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwndDlg, IDC_BOLD, BUTTONSETASPUSHBTN, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETEVENTMASK, 0, ENM_CHANGE|ENM_SELCHANGE|ENM_REQUESTRESIZE);
			SendDlgItemMessage(hwndDlg, IDC_BACKGROUNDCOLORPICKER, CPM_SETCOLOUR, 0, 0x00ffffff);
			LOGFONT lf ={0};
			HDC hdc = GetDC(hwndDlg);
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfFaceName[0] = 0;
			lf.lfPitchAndFamily = 0;
			EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC) EnumFontsProc, (LPARAM) GetDlgItem(hwndDlg, IDC_TYPEFACE), 0);
			ReleaseDC(hwndDlg, hdc);
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("8"));
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("10"));
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("12"));
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("14"));
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("18"));
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("24"));
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("36"));
			SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_SETCURSEL, 2, 0);
			if (SendDlgItemMessage(hwndDlg, IDC_TYPEFACE, CB_SELECTSTRING, 1, (LPARAM)TEXT("Arial"))!=CB_ERR)
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(cf);
				cf.yHeight=12*20;
				cf.dwMask=CFM_SIZE|CFM_FACE;
				mir_tstrcpy(cf.szFaceName, TEXT("Arial"));
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
			else
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(cf);
				cf.yHeight=12*20;
				cf.dwMask=CFM_SIZE;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
			break;
		}
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_SIZING:
		{
			RECT* rect=(RECT*)lParam;
#define MIN_HEIGHT 200
#define MIN_WIDTH 400
			if (WMSZ_RIGHT==wParam||WMSZ_TOPRIGHT==wParam||WMSZ_BOTTOMRIGHT==wParam)
			{
				if (rect->right-rect->left<MIN_WIDTH)
					rect->right=rect->left+MIN_WIDTH;
			}
			if (WMSZ_LEFT==wParam||WMSZ_TOPLEFT==wParam||WMSZ_BOTTOMLEFT==wParam)
			{
				if (rect->right-rect->left<MIN_WIDTH)
					rect->left=rect->right-MIN_WIDTH;
			}
			if (WMSZ_TOP==wParam||WMSZ_TOPRIGHT==wParam||WMSZ_TOPLEFT==wParam)
			{
				if (rect->bottom-rect->top<MIN_HEIGHT)
					rect->top=rect->bottom-MIN_HEIGHT;
			}
			if (WMSZ_BOTTOM==wParam||WMSZ_BOTTOMLEFT==wParam||WMSZ_BOTTOMRIGHT==wParam)
			{
				if (rect->bottom-rect->top<MIN_HEIGHT)
					rect->bottom=rect->top+MIN_HEIGHT;
			}
			break;
		}

	case WM_SIZE:
		{
			int width=LOWORD(lParam);
			int height=HIWORD(lParam);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_PROFILE),HWND_TOP,6,60,width-12,height-67,0);	// this 'case' should go away
			SetWindowPos(GetDlgItem(hwndDlg, IDC_SETPROFILE),HWND_TOP,width-97,height-224,0,0,SWP_NOSIZE);	// since there's no profile window resize anymore
			break;
		}
	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_PROFILE:
			if (((LPNMHDR)lParam)->code==EN_SELCHANGE)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_FACE | CFM_SIZE;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				if (SendDlgItemMessage(hwndDlg, IDC_TYPEFACE, CB_SELECTSTRING, 1, (LPARAM)cfOld.szFaceName)==-1)
				{
					SendDlgItemMessage(hwndDlg, IDC_TYPEFACE, CB_ADDSTRING, 0, (LPARAM)cfOld.szFaceName);
					SendDlgItemMessage(hwndDlg, IDC_TYPEFACE, CB_SELECTSTRING, 1, (LPARAM)cfOld.szFaceName);
				}
				char size[10];
				_itoa(cfOld.yHeight/20,size,sizeof(size));
				//SetDlgItemText(hwndDlg, IDC_FONTSIZE, size);
				SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_SELECTSTRING, 1, (LPARAM)size);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_SUPERSCRIPT), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_NORMALSCRIPT), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_SUBSCRIPT), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_BOLD), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_ITALIC), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_UNDERLINE), NULL, FALSE);
			}
			else if (((LPNMHDR)lParam)->code==EN_REQUESTRESIZE)
			{
				//	REQRESIZE* rr= (REQRESIZE*)lParam;
				//SetWindowPos(GetDlgItem(hwndDlg, IDC_PROFILE),HWND_TOP,rr->rc.left,rr->rc.top,rr->rc.right,rr->rc.bottom,0);
			}
			break;

		default:
			if (((LPNMHDR)lParam)->code == PSN_PARAMCHANGED)
			{
				ppro = (CAimProto*)((PSHNOTIFY*)lParam)->lParam;
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ppro);

				DBVARIANT dbv;
				if (!db_get_utf(NULL, ppro->m_szModuleName, AIM_KEY_PR, &dbv))
				{
					html_decode(dbv.pszVal);
					TCHAR *txt = mir_utf8decodeT(dbv.pszVal);
					SetDlgItemText(hwndDlg, IDC_PROFILE, txt);
					mir_free(txt);
					db_free(&dbv);
				}
			}
		}
		break;

	case WM_DRAWITEM:
		{
			CloseThemeData(hThemeButton);
			hThemeButton = OpenThemeData(GetDlgItem(hwndDlg, IDC_BOLD), L"Button");
			LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
			if (lpDIS->CtlID == IDC_SUPERSCRIPT)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_SUPERSCRIPT;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isSuper = (cfOld.dwEffects & CFE_SUPERSCRIPT) && (cfOld.dwMask & CFM_SUPERSCRIPT);
				if (isSuper)
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState|ODS_SELECTED, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("sup_scrpt"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("sup_scrpt");
				}
				else
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("nsup_scrpt"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("nsup_scrpt");
				}
			}
			else if (lpDIS->CtlID == IDC_NORMALSCRIPT)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_SUBSCRIPT|CFM_SUPERSCRIPT;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isSub = (cfOld.dwEffects & CFE_SUBSCRIPT) && (cfOld.dwMask & CFM_SUBSCRIPT);
				BOOL isSuper = (cfOld.dwEffects & CFE_SUPERSCRIPT) && (cfOld.dwMask & CFM_SUPERSCRIPT);
				if (!isSub&&!isSuper)
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState|ODS_SELECTED, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("norm_scrpt"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("norm_scrpt");
				}
				else
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("nnorm_scrpt"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("nnorm_scrpt");
				}
			}
			else if (lpDIS->CtlID == IDC_SUBSCRIPT)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_SUBSCRIPT;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isSub = (cfOld.dwEffects & CFE_SUBSCRIPT) && (cfOld.dwMask & CFM_SUBSCRIPT);
				if (isSub)
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState|ODS_SELECTED, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("sub_scrpt"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("sub_scrpt");
				}
				else
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("nsub_scrpt"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("nsub_scrpt");
				}
			}
			else if (lpDIS->CtlID == IDC_BOLD)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_BOLD;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isBold = (cfOld.dwEffects & CFE_BOLD) && (cfOld.dwMask & CFM_BOLD);
				if (!isBold)
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("nbold"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("nbold");
				}
				else
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BOLD),hThemeButton,lpDIS->itemState|ODS_SELECTED, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("bold"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("bold");
				}
			}
			else if (lpDIS->CtlID == IDC_ITALIC)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_ITALIC;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isItalic = (cfOld.dwEffects & CFE_ITALIC) && (cfOld.dwMask & CFM_ITALIC);
				if (!isItalic)
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_ITALIC),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("nitalic"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("nitalic");
				}
				else
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_ITALIC),hThemeButton,lpDIS->itemState|ODS_SELECTED, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("italic"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("italic");
				}
			}
			else if (lpDIS->CtlID == IDC_UNDERLINE)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_UNDERLINE;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isUnderline = (cfOld.dwEffects & CFE_UNDERLINE) && (cfOld.dwMask & CFM_UNDERLINE);
				if (!isUnderline)
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_UNDERLINE),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("nundrln"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("nundrln");
				}
				else
				{
					DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_UNDERLINE),hThemeButton,lpDIS->itemState|ODS_SELECTED, lpDIS->rcItem);
					DrawIconEx(lpDIS->hDC, 4, 5, LoadIconEx("undrln"), 16, 16, 0, 0, DI_NORMAL);
					ReleaseIconEx("undrln");
				}
			}
			else if (lpDIS->CtlID == IDC_FOREGROUNDCOLOR)
			{
				DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_FOREGROUNDCOLOR),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
				DrawIconEx(lpDIS->hDC, 4, 2, LoadIconEx("foreclr"), 16, 16, 0, 0, DI_NORMAL);
				ReleaseIconEx("foreclr");
				HBRUSH	hbr = CreateSolidBrush(foreground);
				HPEN hp = CreatePen(PS_SOLID, 1, ~foreground&0x00ffffff);
				SelectObject(lpDIS->hDC,hp);
				RECT rect=lpDIS->rcItem;
				rect.top+=18;
				rect.bottom-=4;
				rect.left+=5;
				rect.right-=5;
				Rectangle(lpDIS->hDC,rect.left-1,rect.top-1,rect.right+1,rect.bottom+1);
				FillRect(lpDIS->hDC,&rect, hbr);
				DeleteObject(hbr);
				DeleteObject(hp);
			}
			else if (lpDIS->CtlID == IDC_FOREGROUNDCOLORPICKER)
			{
				DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_FOREGROUNDCOLORPICKER),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
				HBRUSH	hbr = CreateSolidBrush(foreground);
				HPEN hp = CreatePen(PS_SOLID, 1,~foreground&0x00ffffff);
				SelectObject(lpDIS->hDC,hbr);
				SelectObject(lpDIS->hDC,hp);
				POINT tri[3];
				tri[0].x=3;
				tri[0].y=10;
				tri[1].x=9;
				tri[1].y=10;
				tri[2].x=6;
				tri[2].y=15;
				Polygon(lpDIS->hDC,tri,3);
				DeleteObject(hbr);
				DeleteObject(hp);
			}
			else if (lpDIS->CtlID == IDC_BACKGROUNDCOLOR)
			{
				DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BACKGROUNDCOLOR),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
				DrawIconEx(lpDIS->hDC, 4, 2, LoadIconEx("backclr"), 16, 16, 0, 0, DI_NORMAL);
				ReleaseIconEx("backclr");
				HBRUSH	hbr = CreateSolidBrush(background);
				HPEN hp = CreatePen(PS_SOLID, 1, ~background&0x00ffffff);
				SelectObject(lpDIS->hDC,hp);
				RECT rect=lpDIS->rcItem;
				rect.top+=18;
				rect.bottom-=4;
				rect.left+=5;
				rect.right-=5;
				Rectangle(lpDIS->hDC,rect.left-1,rect.top-1,rect.right+1,rect.bottom+1);
				FillRect(lpDIS->hDC,&rect, hbr);
				DeleteObject(hbr);
				DeleteObject(hp);
			}
			else if (lpDIS->CtlID == IDC_BACKGROUNDCOLORPICKER)
			{
				DrawMyControl(lpDIS->hDC,GetDlgItem(hwndDlg, IDC_BACKGROUNDCOLORPICKER),hThemeButton,lpDIS->itemState, lpDIS->rcItem);
				HBRUSH	hbr = CreateSolidBrush(background);
				HPEN hp = CreatePen(PS_SOLID, 1,~background&0x00ffffff);
				SelectObject(lpDIS->hDC,hbr);
				SelectObject(lpDIS->hDC,hp);
				POINT tri[3];
				tri[0].x=3;
				tri[0].y=10;
				tri[1].x=9;
				tri[1].y=10;
				tri[2].x=6;
				tri[2].y=15;
				Polygon(lpDIS->hDC,tri,3);
				DeleteObject(hbr);
				DeleteObject(hp);
			}
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PROFILE:
			if (HIWORD(wParam) == EN_CHANGE)
				EnableWindow(GetDlgItem(hwndDlg, IDC_SETPROFILE), TRUE);
			break;

		case IDC_SETPROFILE:
			{
				char* buf = rtf_to_html(hwndDlg, IDC_PROFILE);
				db_set_utf(NULL, ppro->m_szModuleName, AIM_KEY_PR, buf);
				if (ppro->state==1)
					ppro->aim_set_profile(ppro->hServerConn, ppro->seqno, buf);//also see set caps for profile setting

				mir_free(buf);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SETPROFILE), FALSE);
			}
			break;

		case IDC_SUPERSCRIPT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask=CFM_SUPERSCRIPT;
				cf.dwEffects=CFE_SUPERSCRIPT;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
				InvalidateRect(GetDlgItem(hwndDlg, IDC_NORMALSCRIPT), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_SUBSCRIPT), NULL, FALSE);
			}
			break;

		case IDC_NORMALSCRIPT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask=CFM_SUPERSCRIPT;
				cf.dwEffects &= ~CFE_SUPERSCRIPT;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
				InvalidateRect(GetDlgItem(hwndDlg, IDC_SUPERSCRIPT), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_SUBSCRIPT), NULL, FALSE);
			}
			break;

		case IDC_SUBSCRIPT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask=CFM_SUBSCRIPT;
				cf.dwEffects=CFE_SUBSCRIPT;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
				InvalidateRect(GetDlgItem(hwndDlg, IDC_SUPERSCRIPT), NULL, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_NORMALSCRIPT), NULL, FALSE);
			}
			break;

		case IDC_BOLD:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_BOLD;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isBold = (cfOld.dwEffects & CFE_BOLD) && (cfOld.dwMask & CFM_BOLD);
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwEffects = isBold ? 0 : CFE_BOLD;
				cf.dwMask = CFM_BOLD;
				CheckDlgButton(hwndDlg, IDC_BOLD, !isBold ? BST_CHECKED : BST_UNCHECKED);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
			}
			break;

		case IDC_ITALIC:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_ITALIC;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isItalic = (cfOld.dwEffects & CFE_ITALIC) && (cfOld.dwMask & CFM_ITALIC);
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwEffects = isItalic ? 0 : CFE_ITALIC;
				cf.dwMask = CFM_ITALIC;
				CheckDlgButton(hwndDlg, IDC_ITALIC, !isItalic ? BST_CHECKED : BST_UNCHECKED);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
			}
			break;

		case IDC_UNDERLINE:
			if (HIWORD(wParam)==BN_CLICKED)
			{
				CHARFORMAT2 cfOld;
				cfOld.cbSize = sizeof(CHARFORMAT2);
				cfOld.dwMask = CFM_UNDERLINE;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
				BOOL isUnderline = (cfOld.dwEffects & CFE_UNDERLINE) && (cfOld.dwMask & CFM_UNDERLINE);
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwEffects = isUnderline ? 0 : CFE_UNDERLINE;
				cf.dwMask = CFM_UNDERLINE;
				CheckDlgButton(hwndDlg, IDC_UNDERLINE, !isUnderline ? BST_CHECKED : BST_UNCHECKED);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
			}
			break;

		case IDC_FOREGROUNDCOLOR:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask=CFM_COLOR;
				cf.dwEffects=0;
				cf.crTextColor=foreground;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetWindowPos(GetDlgItem(hwndDlg, IDC_FOREGROUNDCOLORPICKER),GetDlgItem(hwndDlg, IDC_FOREGROUNDCOLOR),0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
			}
			break;

		case IDC_FOREGROUNDCOLORPICKER:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHOOSECOLOR cc={0};
				custColours[0]=foreground;
				custColours[1]=background;
				cc.lStructSize=sizeof(CHOOSECOLOR);
				cc.hwndOwner=hwndDlg;
				cc.hInstance=(HWND)GetModuleHandle(NULL);
				cc.lpCustColors=custColours;
				cc.Flags=CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
				if (ChooseColor(&cc))
				{
					foreground=cc.rgbResult;
					InvalidateRect(GetDlgItem(hwndDlg, IDC_FOREGROUNDCOLOR), NULL, FALSE);
				}
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
			}
			break;

		case IDC_BACKGROUNDCOLOR:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask=CFM_BACKCOLOR;
				cf.dwEffects=0;
				cf.crBackColor=background;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
			}
			break;

		case IDC_BACKGROUNDCOLORPICKER:
			if (HIWORD(wParam)==BN_CLICKED)
			{
				CHOOSECOLOR cc={0};
				custColours[0]=foreground;
				custColours[1]=background;
				cc.lStructSize=sizeof(CHOOSECOLOR);
				cc.hwndOwner=hwndDlg;
				cc.hInstance=(HWND)GetModuleHandle(NULL);
				cc.lpCustColors=custColours;
				cc.Flags=CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
				if (ChooseColor(&cc))
				{
					background=cc.rgbResult;
					InvalidateRect(GetDlgItem(hwndDlg, IDC_BACKGROUNDCOLOR), NULL, FALSE);
				}
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
			}
			break;

		case IDC_TYPEFACE:
			if (HIWORD(wParam)==CBN_SELENDOK)
			{
				CHARFORMAT2A cf;
				cf.cbSize = sizeof(cf);
				cf.dwMask=CFM_FACE;
				cf.dwEffects=0;
				SendDlgItemMessage(hwndDlg, IDC_TYPEFACE, CB_GETLBTEXT, SendDlgItemMessage(hwndDlg, IDC_TYPEFACE, CB_GETCURSEL, 0, 0),(LPARAM)cf.szFaceName);
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
				break;
			}
			break;

		case IDC_FONTSIZE:
			if (HIWORD(wParam)==CBN_SELENDOK)
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask=CFM_SIZE;
				cf.dwEffects=0;
				char chsize[5] = "";
				SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_GETLBTEXT, SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_GETCURSEL, 0, 0),(LPARAM)chsize);
				//strlcpy(cf.szFaceName,size,mir_strlen(size)+1);
				cf.yHeight=atoi(chsize)*20;
				SendDlgItemMessage(hwndDlg, IDC_PROFILE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SetFocus(GetDlgItem(hwndDlg, IDC_PROFILE));
				break;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// View admin dialog

INT_PTR CALLBACK admin_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAimProto* ppro = (CAimProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	DBVARIANT dbv;

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_FNAME, EM_LIMITTEXT, 63, 0);
		SendDlgItemMessage(hwndDlg, IDC_CEMAIL, EM_LIMITTEXT, 253, 0);
		SendDlgItemMessage(hwndDlg, IDC_CPW, EM_LIMITTEXT, 253, 0);
		SendDlgItemMessage(hwndDlg, IDC_NPW1, EM_LIMITTEXT, 253, 0);
		SendDlgItemMessage(hwndDlg, IDC_NPW2, EM_LIMITTEXT, 253, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_PARAMCHANGED:
			ppro = (CAimProto*)((LPPSHNOTIFY)lParam)->lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ppro);

			if (ppro->wait_conn(ppro->hAdminConn, ppro->hAdminEvent, 0x07))             // Make a connection
			{
				ppro->aim_admin_request_info(ppro->hAdminConn,ppro->admin_seqno,0x01);	// Get our screenname
				ppro->aim_admin_request_info(ppro->hAdminConn,ppro->admin_seqno,0x11);	// Get our email
			}

		case PSN_INFOCHANGED:
			if (!ppro->getString(AIM_KEY_SN, &dbv))
			{
				SetDlgItemTextA(hwndDlg, IDC_FNAME, dbv.pszVal);
				db_free(&dbv);
			}
			if (!ppro->getString(AIM_KEY_EM, &dbv))
			{
				SetDlgItemTextA(hwndDlg, IDC_CEMAIL, dbv.pszVal);
				db_free(&dbv);
			}
			break;
		}
		break;

	case WM_COMMAND:
		if  (LOWORD(wParam) == IDC_SAVECHANGES)
		{
			if (!ppro->wait_conn(ppro->hAdminConn, ppro->hAdminEvent, 0x07))             // Make a connection
				break;

			char name[64];
			GetDlgItemTextA(hwndDlg, IDC_FNAME, name, SIZEOF(name));
			if (mir_strlen(trim_str(name)) > 0 && !ppro->getString(AIM_KEY_SN, &dbv))
			{
				if (mir_strcmp(name, dbv.pszVal))
					ppro->aim_admin_format_name(ppro->hAdminConn,ppro->admin_seqno,name);
				db_free(&dbv);
			}

			char email[254];
			GetDlgItemTextA(hwndDlg, IDC_CEMAIL, email, SIZEOF(email));
			if (mir_strlen(trim_str(email)) > 1 && !ppro->getString(AIM_KEY_EM, &dbv)) // Must be greater than 1 or a SNAC error is thrown.
			{
				if (mir_strcmp(email, dbv.pszVal))
					ppro->aim_admin_change_email(ppro->hAdminConn,ppro->admin_seqno,email);
				db_free(&dbv);
			}

			ShowWindow(GetDlgItem(hwndDlg, IDC_PINFO), SW_HIDE);

			char cpw[256], npw1[256], npw2[256];
			GetDlgItemTextA(hwndDlg, IDC_CPW, cpw, SIZEOF(cpw));
			GetDlgItemTextA(hwndDlg, IDC_NPW1, npw1, SIZEOF(npw1));
			GetDlgItemTextA(hwndDlg, IDC_NPW2, npw2, SIZEOF(npw2));
			if (cpw[0] != 0 && npw1[0] != 0 && npw2[0] != 0)
			{
				// AOL only requires that you send the current password and a (single) new password.
				// Let's allow the client to type (two) new passwords incase they make a mistake so we
				// can handle any input error locally.
				if (mir_strcmp(npw1,npw2) == 0)
				{
					ppro->aim_admin_change_password(ppro->hAdminConn,ppro->admin_seqno,cpw,npw1);
				}
				else
				{
					SetDlgItemTextA(hwndDlg, IDC_CPW, "");
					SetDlgItemTextA(hwndDlg, IDC_NPW1, "");
					SetDlgItemTextA(hwndDlg, IDC_NPW2, "");
					ShowWindow(GetDlgItem(hwndDlg, IDC_PINFO), SW_SHOW);
				}
			}
		}
		else if (LOWORD(wParam) == IDC_CONFIRM)	// Confirmation
		{
			if (ppro->wait_conn(ppro->hAdminConn, ppro->hAdminEvent, 0x07))             // Make a connection
				ppro->aim_admin_account_confirm(ppro->hAdminConn,ppro->admin_seqno);
		}
		break;
	}
	return FALSE;
}

int CAimProto::OnUserInfoInit(WPARAM wParam,LPARAM lParam)
{
	if (!lParam)//hContact
	{
		OPTIONSDIALOGPAGE odp = { 0 };
		odp.position = -1900000000;
		odp.flags = ODPF_USERINFOTAB | ODPF_TCHAR;
		odp.hInstance = hInstance;
		odp.ptszTitle = m_tszUserName;
		odp.dwInitParam = LPARAM(this);

		odp.ptszTab = LPGENT("Profile");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO);
		odp.pfnDlgProc = userinfo_dialog;
		UserInfo_AddPage(wParam, &odp);

		odp.ptszTab = LPGENT("Admin");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_ADMIN);
		odp.pfnDlgProc = admin_dialog;
		UserInfo_AddPage(wParam, &odp);
	}
	return 0;
}

INT_PTR CAimProto::EditProfile(WPARAM, LPARAM)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_AIM), NULL, userinfo_dialog, LPARAM(this));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options dialog

static INT_PTR CALLBACK options_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAimProto* ppro = (CAimProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		ppro = (CAimProto*)lParam;
		{
			DBVARIANT dbv;
			if (!ppro->getString(AIM_KEY_SN, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_SN, dbv.pszVal);
				db_free(&dbv);
			}
			if (!ppro->getString(AIM_KEY_NK, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_NK, dbv.pszVal);
				db_free(&dbv);
			}
			else if (!ppro->getString(AIM_KEY_SN, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_NK, dbv.pszVal);
				db_free(&dbv);
			}
			if (!ppro->getString(AIM_KEY_PW, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_PW, dbv.pszVal);
				db_free(&dbv);
			}
			if (!ppro->getString(AIM_KEY_HN, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_HN, dbv.pszVal);
				db_free(&dbv);
			}
			else SetDlgItemTextA(hwndDlg, IDC_HN, ppro->getByte(AIM_KEY_DSSL, 0) ? AIM_DEFAULT_SERVER_NS : AIM_DEFAULT_SERVER);

			SetDlgItemInt(hwndDlg, IDC_PN, ppro->get_default_port(), FALSE);

			CheckDlgButton(hwndDlg, IDC_DC, ppro->getByte(AIM_KEY_DC, 0) ? BST_CHECKED : BST_UNCHECKED);//Message Delivery Confirmation
			CheckDlgButton(hwndDlg, IDC_FP, ppro->getByte(AIM_KEY_FP, 0) ? BST_CHECKED : BST_UNCHECKED);//force proxy
			CheckDlgButton(hwndDlg, IDC_AT, ppro->getByte(AIM_KEY_AT, 0) ? BST_CHECKED : BST_UNCHECKED);//Account Type Icons
			CheckDlgButton(hwndDlg, IDC_ES, ppro->getByte(AIM_KEY_ES, 0) ? BST_CHECKED : BST_UNCHECKED);//Extended Status Type Icons
			CheckDlgButton(hwndDlg, IDC_HF, ppro->getByte(AIM_KEY_HF, 0) ? BST_CHECKED : BST_UNCHECKED);//Fake hiptopness
			CheckDlgButton(hwndDlg, IDC_DM, ppro->getByte(AIM_KEY_DM, 0) ? BST_CHECKED : BST_UNCHECKED);//Disable Sending Mode Message
			CheckDlgButton(hwndDlg, IDC_FI, ppro->getByte(AIM_KEY_FI, 1) ? BST_CHECKED : BST_UNCHECKED);//Format incoming messages
			CheckDlgButton(hwndDlg, IDC_FO, ppro->getByte(AIM_KEY_FO, 1) ? BST_CHECKED : BST_UNCHECKED);//Format outgoing messages
			CheckDlgButton(hwndDlg, IDC_II, ppro->getByte(AIM_KEY_II, 0) ? BST_CHECKED : BST_UNCHECKED);//Instant Idle
			CheckDlgButton(hwndDlg, IDC_CM, ppro->getByte(AIM_KEY_CM, 0) ? BST_CHECKED : BST_UNCHECKED);//Check Mail
			CheckDlgButton(hwndDlg, IDC_MG, ppro->getByte(AIM_KEY_MG, 1) ? BST_CHECKED : BST_UNCHECKED);//Manage Groups
			CheckDlgButton(hwndDlg, IDC_DA, ppro->getByte(AIM_KEY_DA, 0) ? BST_CHECKED : BST_UNCHECKED);//Disable Avatars
			CheckDlgButton(hwndDlg, IDC_DSSL, ppro->getByte(AIM_KEY_DSSL, 0) ? BST_CHECKED : BST_UNCHECKED);//Disable SSL
			CheckDlgButton(hwndDlg, IDC_FSC, ppro->getByte(AIM_KEY_FSC, 0) ? BST_CHECKED : BST_UNCHECKED);//Force Single Client
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DSSL:
			{
				bool dssl = IsDlgButtonChecked(hwndDlg, IDC_DSSL) != 0;
				SetDlgItemTextA(hwndDlg, IDC_HN, dssl ? AIM_DEFAULT_SERVER_NS : AIM_DEFAULT_SERVER);
				SetDlgItemInt(hwndDlg, IDC_PN, dssl ? AIM_DEFAULT_PORT : AIM_DEFAULT_SSL_PORT, FALSE);
			}
			break;

		case IDC_SVRRESET:
			SetDlgItemTextA(hwndDlg, IDC_HN,
				IsDlgButtonChecked(hwndDlg, IDC_DSSL) ? AIM_DEFAULT_SERVER_NS : AIM_DEFAULT_SERVER);
			SetDlgItemInt(hwndDlg, IDC_PN,ppro->get_default_port(), FALSE);
			break;

		case IDC_SN:
		case IDC_PN:
		case IDC_NK:
		case IDC_PW:
		case IDC_HN:
			if (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus())
				return 0;
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code)
		{
		case PSN_APPLY:
			{
				char str[128];
				//SN
				GetDlgItemTextA(hwndDlg, IDC_SN, str, SIZEOF(str));
				if (str[0] != 0)
					ppro->setString(AIM_KEY_SN, str);
				else
					ppro->delSetting(AIM_KEY_SN);
				//END SN

				//NK
				if (GetDlgItemTextA(hwndDlg, IDC_NK, str, SIZEOF(str)))
					ppro->setString(AIM_KEY_NK, str);
				else
				{
					GetDlgItemTextA(hwndDlg, IDC_SN, str, SIZEOF(str));
					ppro->setString(AIM_KEY_NK, str);
				}
				//END NK

				//PW
				GetDlgItemTextA(hwndDlg, IDC_PW, str, SIZEOF(str));
				if (str[0] != 0)
					ppro->setString(AIM_KEY_PW, str);
				else
					ppro->delSetting(AIM_KEY_PW);
				//END PW

				//HN
				GetDlgItemTextA(hwndDlg, IDC_HN, str, SIZEOF(str));
				if (str[0] != 0 && mir_strcmp(str, AIM_DEFAULT_SERVER))
					ppro->setString(AIM_KEY_HN, str);
				else
					ppro->delSetting(AIM_KEY_HN);
				//END HN

				//Delivery Confirmation
				ppro->setByte(AIM_KEY_DC, IsDlgButtonChecked(hwndDlg, IDC_DC) != 0);
				//End Delivery Confirmation

				//Disable Avatar
				ppro->setByte(AIM_KEY_DA, IsDlgButtonChecked(hwndDlg, IDC_DA) != 0);
				//Disable Avatar

				//Disable SSL
				ppro->setByte(AIM_KEY_DSSL, IsDlgButtonChecked(hwndDlg, IDC_DSSL) != 0);
				//Disable SSL

				//Force Single Login
				ppro->setByte(AIM_KEY_FSC, IsDlgButtonChecked(hwndDlg, IDC_FSC) != 0);
				//Force Single Login

				//Force Proxy Transfer
				ppro->setByte(AIM_KEY_FP, IsDlgButtonChecked(hwndDlg, IDC_FP) != 0);
				//End Force Proxy Transfer

				//PN
				int port = GetDlgItemInt(hwndDlg, IDC_PN, NULL, FALSE);
				if (port > 0 && port != (ppro->getByte(AIM_KEY_DSSL, 0) ? AIM_DEFAULT_PORT : AIM_DEFAULT_SSL_PORT))
					ppro->setWord(AIM_KEY_PN, (WORD)port);
				else
					ppro->delSetting(AIM_KEY_PN);
				//END PN

				//Disable Account Type Icons
				if (IsDlgButtonChecked(hwndDlg, IDC_AT))
				{
					int acc_disabled = ppro->getByte(AIM_KEY_AT, 0);
					if (!acc_disabled)
						remove_AT_icons(ppro);
					ppro->setByte(AIM_KEY_AT, 1);
				}
				else
				{
					int acc_disabled = ppro->getByte(AIM_KEY_AT, 0);
					if (acc_disabled)
						add_AT_icons(ppro);
					ppro->setByte(AIM_KEY_AT, 0);
				}
				//END
				//Disable Extra Status Icons
				if (IsDlgButtonChecked(hwndDlg, IDC_ES))
				{
					int es_disabled = ppro->getByte(AIM_KEY_ES, 0);
					ppro->setByte(AIM_KEY_ES, 1);
					if (!es_disabled)
						remove_ES_icons(ppro);
				}
				else
				{
					int es_disabled = ppro->getByte(AIM_KEY_ES, 0);
					ppro->setByte(AIM_KEY_ES, 0);
					if (es_disabled)
						add_ES_icons(ppro);
				}
				//End

				//Fake Hiptop
				if (IsDlgButtonChecked(hwndDlg, IDC_HF))
				{
					int hf = ppro->getByte(AIM_KEY_HF, 0);
					if (!hf)
						ShowWindow(GetDlgItem(hwndDlg, IDC_MASQ), SW_SHOW);
					ppro->setByte(AIM_KEY_HF, 1);
				}
				else
				{
					int hf = ppro->getByte(AIM_KEY_HF, 0);
					if (hf)
						ShowWindow(GetDlgItem(hwndDlg, IDC_MASQ), SW_SHOW);
					ppro->setByte(AIM_KEY_HF, 0);
				}
				//End

				//Disable Mode Message Sending
				ppro->setByte(AIM_KEY_DM, IsDlgButtonChecked(hwndDlg, IDC_DM) != 0);
				//End Disable Mode Message Sending

				//Format Incoming Messages
				ppro->setByte(AIM_KEY_FI, IsDlgButtonChecked(hwndDlg, IDC_FI) != 0);
				//End Format Incoming Messages

				//Format Outgoing Messages
				ppro->setByte(AIM_KEY_FO, IsDlgButtonChecked(hwndDlg, IDC_FO) != 0);
				//End Format Outgoing Messages

				//Instant Idle on Login
				ppro->setByte(AIM_KEY_II, IsDlgButtonChecked(hwndDlg, IDC_II) != 0);
				//End
				//Check Mail on Login
				ppro->setByte(AIM_KEY_CM, IsDlgButtonChecked(hwndDlg, IDC_CM) != 0);
				//End

				//Manage Groups
				ppro->setByte(AIM_KEY_MG, IsDlgButtonChecked(hwndDlg, IDC_MG) != 0);
				//End
			}
		}
		break;
	}
	return FALSE;
}


static INT_PTR CALLBACK privacy_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const int btns[] = { IDC_ALLOWALL, IDC_BLOCKALL, IDC_ALLOWBELOW, IDC_BLOCKBELOW, IDC_ALLOWCONT };
	CAimProto* ppro = (CAimProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	int i;

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		ppro = (CAimProto*)lParam;

		CheckRadioButton(hwndDlg, IDC_ALLOWALL, IDC_BLOCKBELOW, btns[ppro->pd_mode-1]);

		for (i=0; i<ppro->allow_list.getCount(); ++i)
			SendDlgItemMessageA(hwndDlg, IDC_ALLOWLIST, LB_ADDSTRING, 0, (LPARAM)ppro->allow_list[i].name);

		for (i=0; i<ppro->block_list.getCount(); ++i)
			SendDlgItemMessageA(hwndDlg, IDC_BLOCKLIST, LB_ADDSTRING, 0, (LPARAM)ppro->block_list[i].name);

		CheckDlgButton(hwndDlg, IDC_SIS, (ppro->pref1_flags & 0x400) ? BST_CHECKED : BST_CHECKED);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_ALLOWADD)
		{
			char nick[80];
			GetDlgItemTextA(hwndDlg, IDC_ALLOWEDIT, nick, SIZEOF(nick));
			SendDlgItemMessageA(hwndDlg, IDC_ALLOWLIST, LB_ADDSTRING, 0, (LPARAM)trim_str(nick));
		}
		else if (LOWORD(wParam) == IDC_BLOCKADD)
		{
			char nick[80];
			GetDlgItemTextA(hwndDlg, IDC_BLOCKEDIT, nick, SIZEOF(nick));
			SendDlgItemMessageA(hwndDlg, IDC_BLOCKLIST, LB_ADDSTRING, 0, (LPARAM)trim_str(nick));
		}
		else if (LOWORD(wParam) == IDC_ALLOWREMOVE)
		{
			i = SendDlgItemMessage(hwndDlg, IDC_ALLOWLIST, LB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_ALLOWLIST, LB_DELETESTRING, i, 0);
		}
		else if (LOWORD(wParam) == IDC_BLOCKREMOVE)
		{
			i = SendDlgItemMessage(hwndDlg, IDC_BLOCKLIST, LB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BLOCKLIST, LB_DELETESTRING, i, 0);
		}

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->code == PSN_APPLY)
		{
			ppro->aim_ssi_update(ppro->hServerConn, ppro->seqno, true);
			for (i=0; i<5; ++i)
			{
				if (IsDlgButtonChecked(hwndDlg, btns[i]) && ppro->pd_mode != i + 1)
				{
					ppro->pd_mode = (char)(i + 1);
					ppro->pd_flags = 1;
					ppro->aim_set_pd_info(ppro->hServerConn, ppro->seqno);
					break;
				}
			}
			for (i=0; i<ppro->block_list.getCount(); ++i)
			{
				BdListItem& pd = ppro->block_list[i];
				if (SendDlgItemMessageA(hwndDlg, IDC_BLOCKLIST, LB_FINDSTRING, (WPARAM)-1, (LPARAM)pd.name) == LB_ERR)
				{
					ppro->aim_delete_contact(ppro->hServerConn, ppro->seqno, pd.name, pd.item_id, 0, 3, false);
					ppro->block_list.remove(i--);
				}
			}
			i = SendDlgItemMessage(hwndDlg, IDC_BLOCKLIST, LB_GETCOUNT, 0, 0);
			for (; i--;)
			{
				char nick[80];
				SendDlgItemMessageA(hwndDlg, IDC_BLOCKLIST, LB_GETTEXT, i, (LPARAM)nick);
				if (ppro->block_list.find_id(nick) == 0)
				{
					unsigned short id = ppro->block_list.add(nick);
					ppro->aim_add_contact(ppro->hServerConn, ppro->seqno, nick, id, 0, 3);
				}
			}

			for (i=0; i<ppro->allow_list.getCount(); ++i)
			{
				BdListItem& pd = ppro->allow_list[i];
				if (SendDlgItemMessageA(hwndDlg, IDC_ALLOWLIST, LB_FINDSTRING, (WPARAM)-1, (LPARAM)pd.name) == LB_ERR)
				{
					ppro->aim_delete_contact(ppro->hServerConn, ppro->seqno, pd.name, pd.item_id, 0, 2, false);
					ppro->allow_list.remove(i--);
				}
			}
			i = SendDlgItemMessage(hwndDlg, IDC_ALLOWLIST, LB_GETCOUNT, 0, 0);
			for (; i--;)
			{
				char nick[80];
				SendDlgItemMessageA(hwndDlg, IDC_ALLOWLIST, LB_GETTEXT, i, (LPARAM)nick);
				if (ppro->allow_list.find_id(nick) == 0)
				{
					unsigned short id = ppro->allow_list.add(nick);
					ppro->aim_add_contact(ppro->hServerConn, ppro->seqno, nick, id, 0, 2);
				}
			}

			unsigned mask = (IsDlgButtonChecked(hwndDlg, IDC_SIS) == BST_CHECKED) << 10;
			if ((ppro->pref1_flags & 0x400) ^ mask)
			{
				ppro->pref1_flags = (ppro->pref1_flags & ~0x400) | mask;
				ppro->aim_ssi_update_preferences(ppro->hServerConn, ppro->seqno);
			}

			ppro->aim_ssi_update(ppro->hServerConn, ppro->seqno, false);
		}
		break;
	}
	return FALSE;
}


int CAimProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 1003000;
	odp.hInstance = hInstance;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.ptszTab   = LPGENT("Basic");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_AIM);
	odp.pfnDlgProc = options_dialog;
	Options_AddPage(wParam, &odp);

	odp.ptszTab     = LPGENT("Privacy");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PRIVACY);
	odp.pfnDlgProc  = privacy_dialog;
	Options_AddPage(wParam, &odp);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Brief account info dialog

INT_PTR CALLBACK first_run_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAimProto* ppro = (CAimProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CAimProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			DBVARIANT dbv;
			if (!ppro->getString(AIM_KEY_SN, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_SN, dbv.pszVal);
				db_free(&dbv);
			}

			if (!ppro->getString(AIM_KEY_PW, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_PW, dbv.pszVal);
				db_free(&dbv);
			}
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NEWAIMACCOUNTLINK) {
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://www.aim.com/redirects/inclient/register.adp");
			return TRUE;
		}

		if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
			switch(LOWORD(wParam)) {
			case IDC_SN:
			case IDC_PW:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) {
			char str[128];
			GetDlgItemTextA(hwndDlg, IDC_SN, str, SIZEOF(str));
			ppro->setString(AIM_KEY_SN, str);

			GetDlgItemTextA(hwndDlg, IDC_PW, str, SIZEOF(str));
			ppro->setString(AIM_KEY_PW, str);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CAimProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam (hInstance, MAKEINTRESOURCE(IDD_AIMACCOUNT),
		 (HWND)lParam, first_run_dialog, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Instant idle dialog

INT_PTR CALLBACK instant_idle_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAimProto* ppro = (CAimProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		ppro = (CAimProto*)lParam;
		{
			WindowSetIcon(hwndDlg, "idle");
			unsigned long it = ppro->getDword(AIM_KEY_IIT, 0);
			unsigned long hours = it / 60;
			unsigned long minutes = it % 60;
			SetDlgItemInt(hwndDlg, IDC_IIH, hours,0);
			SetDlgItemInt(hwndDlg, IDC_IIM, minutes,0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		WindowFreeIcon(hwndDlg);
		break;

	case WM_COMMAND:
		{
			unsigned long hours=GetDlgItemInt(hwndDlg, IDC_IIH,0,0);
			unsigned short minutes=(unsigned short)GetDlgItemInt(hwndDlg, IDC_IIM,0,0);
			if (minutes > 59)
				minutes = 59;

			ppro->setDword(AIM_KEY_IIT, hours*60+minutes);
			switch (LOWORD(wParam)) {
			case IDOK:
				//Instant Idle
				if (ppro->state == 1) {
					ppro->aim_set_idle(ppro->hServerConn,ppro->seqno,hours * 60 * 60 + minutes * 60);
					ppro->instantidle=1;
				}
				EndDialog(hwndDlg, IDOK);
				break;

			case IDCANCEL:
				ppro->aim_set_idle(ppro->hServerConn,ppro->seqno,0);
				ppro->instantidle=0;
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Join chat dialog

INT_PTR CALLBACK join_chat_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAimProto* ppro = (CAimProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		ppro = (CAimProto*)lParam;
		WindowSetIcon(hwndDlg, "aol");
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		WindowFreeIcon(hwndDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			char room[128];
			GetDlgItemTextA(hwndDlg, IDC_ROOM, room, SIZEOF(room));
			if (ppro->state == 1 && room[0] != 0) {
				chatnav_param* par = new chatnav_param(room, 4);
				ppro->ForkThread(&CAimProto::chatnav_request_thread, par);
			}
			EndDialog(hwndDlg, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		break;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////
// Invite to chat dialog

static void clist_chat_invite_send(MCONTACT hItem, HWND hwndList, chat_list_item* item, CAimProto* ppro, char *msg)
{
	if (hItem == NULL)
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				clist_chat_invite_send(hItemT, hwndList, item, ppro, msg);
		}
		else {
			int chk = SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
			if (chk) {
				if (IsHContactInfo(hItem)) {
					TCHAR buf[128] = _T("");
					SendMessage(hwndList, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)buf);

					char* sn = mir_t2a(buf);
					ppro->aim_chat_invite(ppro->hServerConn, ppro->seqno,
						item->cookie, item->exchange, item->instance, sn, msg);
					mir_free(sn);
				}
				else {
					DBVARIANT dbv;
					if (!ppro->getString(hItem, AIM_KEY_SN, &dbv)) {
						ppro->aim_chat_invite(ppro->hServerConn, ppro->seqno,
							item->cookie, item->exchange, item->instance, dbv.pszVal, msg);
						db_free(&dbv);
					}
				}
			}
		}
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
	}
}

static void clist_validate_contact(MCONTACT hItem, HWND hwndList, CAimProto* ppro)
{
	if (!ppro->is_my_contact(hItem) || ppro->isChatRoom(hItem) ||
			ppro->getWord(hItem, AIM_KEY_ST, ID_STATUS_OFFLINE) == ID_STATUS_ONTHEPHONE)
		SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

static void clist_chat_prepare(MCONTACT hItem, HWND hwndList, CAimProto* ppro)
{
	if (hItem == NULL)
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		MCONTACT hItemN = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				clist_chat_prepare(hItemT, hwndList, ppro);
		}
		else if (IsHContactContact(hItem))
			clist_validate_contact(hItem, hwndList, ppro);

		hItem = hItemN;
   }
}

INT_PTR CALLBACK invite_to_chat_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	invite_chat_param* param = (invite_chat_param*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		param = (invite_chat_param*)lParam;

		WindowSetIcon(hwndDlg, "aol");
		SetDlgItemTextA(hwndDlg, IDC_ROOMNAME, param->id);
		SetDlgItemTextA(hwndDlg, IDC_MSG, Translate("Join me in this buddy chat!"));
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_NCDESTROY:
		WindowFreeIcon(hwndDlg);
		delete param;
		break;

	case WM_NOTIFY:
	{
		NMCLISTCONTROL* nmc = (NMCLISTCONTROL*)lParam;
		if (nmc->hdr.idFrom == IDC_CCLIST)
		{
			switch (nmc->hdr.code)
			{
			case CLN_NEWCONTACT:
				if (param && (nmc->flags & (CLNF_ISGROUP | CLNF_ISINFO)) == 0)
					clist_validate_contact((MCONTACT)nmc->hItem, nmc->hdr.hwndFrom, param->ppro);
				break;

			case CLN_LISTREBUILT:
				if (param)
					clist_chat_prepare(NULL, nmc->hdr.hwndFrom, param->ppro);
				break;
			}
		}
	}
	break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_ADDSCR:
				if (param->ppro->state == 1)
				{
					TCHAR sn[64];
					GetDlgItemText(hwndDlg, IDC_EDITSCR, sn, SIZEOF(sn));

					CLCINFOITEM cii = {0};
					cii.cbSize = sizeof(cii);
					cii.flags = CLCIIF_CHECKBOX | CLCIIF_BELOWCONTACTS;
					cii.pszText = sn;

					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
					SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_SETCHECKMARK, (LPARAM)hItem, 1);
				}
				break;

			case IDOK:
				{
					chat_list_item* item = param->ppro->find_chat_by_id(param->id);
					if (item)
					{
						char msg[1024];
						GetDlgItemTextA(hwndDlg, IDC_MSG, msg, SIZEOF(msg));

						HWND hwndList = GetDlgItem(hwndDlg, IDC_CCLIST);
						clist_chat_invite_send(NULL, hwndList, item, param->ppro, msg);
					}
					EndDialog(hwndDlg, IDOK);
				}
				break;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat request dialog

INT_PTR CALLBACK chat_request_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	invite_chat_req_param* param = (invite_chat_req_param*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		param = (invite_chat_req_param*)lParam;

		WindowSetIcon(hwndDlg, "aol");

		SetDlgItemTextA(hwndDlg, IDC_ROOMNAME, strrchr(param->cnp->id, '-')+1);
		SetDlgItemTextA(hwndDlg, IDC_SCREENNAME,  param->name);
		SetDlgItemTextA(hwndDlg, IDC_MSG, param->message);
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		WindowFreeIcon(hwndDlg);
		delete param;
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				param->ppro->ForkThread(&CAimProto::chatnav_request_thread, param->cnp);
				EndDialog(hwndDlg, IDOK);
				break;

			case IDCANCEL:
				param->ppro->aim_chat_deny(param->ppro->hServerConn,param->ppro->seqno,param->name,param->icbm_cookie);
				delete param->cnp;
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}
	return FALSE;
}


void CALLBACK chat_request_cb(PVOID dwParam)
{
	CreateDialogParam (hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE_REQ),
		 NULL, chat_request_dialog, (LPARAM)dwParam);
}
