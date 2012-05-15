/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"

#if defined( _UNICODE )
	#define STR_VERSION_FORMAT "%s%S%S"
#else
	#define STR_VERSION_FORMAT "%s%s%s"
#endif

INT_PTR CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int iState = 0;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{	TCHAR filename[MAX_PATH], *productCopyright;
			DWORD unused;
			DWORD verInfoSize;
			UINT blockSize;
			PVOID pVerInfo;

			GetModuleFileName(NULL,filename,SIZEOF(filename));
			verInfoSize=GetFileVersionInfoSize(filename,&unused);
			pVerInfo=mir_alloc(verInfoSize);
			GetFileVersionInfo(filename,0,verInfoSize,pVerInfo);
			VerQueryValue(pVerInfo,_T("\\StringFileInfo\\000004b0\\LegalCopyright"),(LPVOID*)&productCopyright,&blockSize);
			SetDlgItemText(hwndDlg,IDC_DEVS,productCopyright);
			mir_free(pVerInfo);
		}
		{	char productVersion[56], *p;
            int isAnsi = 0;
			TCHAR str[64];
			CallService(MS_SYSTEM_GETVERSIONTEXT,SIZEOF(productVersion),(LPARAM)productVersion);
            // Hide Unicode from version text as it is assumed at this point
            p = strstr(productVersion, " Unicode"); 
			if (p)
				*p = '\0';
            else
                isAnsi = 1;
			mir_sntprintf(str,SIZEOF(str),_T(STR_VERSION_FORMAT), TranslateT("v"), productVersion, isAnsi?" ANSI":"");
            {
                TCHAR oldTitle[256], newTitle[256];
				GetDlgItemText( hwndDlg, IDC_HEADERBAR, oldTitle, SIZEOF( oldTitle ));
				mir_sntprintf( newTitle, SIZEOF(newTitle), oldTitle, str );
				SetDlgItemText( hwndDlg, IDC_HEADERBAR, newTitle );
			}
            
			mir_sntprintf(str,SIZEOF(str),TranslateT("Built %s %s"),_T(__DATE__),_T(__TIME__));
			SetDlgItemText(hwndDlg,IDC_BUILDTIME,str);
		}
		ShowWindow(GetDlgItem(hwndDlg, IDC_CREDITSFILE), SW_HIDE);
		{	
			HRSRC   hResInfo  = FindResource(hMirandaInst,MAKEINTRESOURCE(IDR_CREDITS),_T("TEXT"));
			DWORD   ResSize   = SizeofResource(hMirandaInst,hResInfo);
			HGLOBAL hRes      = LoadResource(hMirandaInst,hResInfo);
			char*   pszMsg    = (char*)LockResource(hRes);
			if (pszMsg)
			{
				char* pszMsgt = (char*)alloca(ResSize + 1);
				memcpy(pszMsgt, pszMsg, ResSize); pszMsgt[ResSize] = 0;

				TCHAR *ptszMsg;
				if (ResSize >=3 && pszMsgt[0] == '\xef' && pszMsgt[1] == '\xbb' && pszMsgt[2] == '\xbf')
					ptszMsg = Utf8DecodeT(pszMsgt + 3);
				else
					ptszMsg = mir_a2t_cp(pszMsgt, 1252);

				SetDlgItemText(hwndDlg, IDC_CREDITSFILE, ptszMsg);
				UnlockResource(pszMsg);
				mir_free(ptszMsg);
			}
			FreeResource(hRes);
		}
		Window_SetIcon_IcoLib(hwndDlg, SKINICON_OTHER_MIRANDA);
		return TRUE;

	case WM_COMMAND:
		switch( LOWORD( wParam )) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDC_CONTRIBLINK:
			if (iState) {
				iState = 0;
				SetDlgItemText(hwndDlg, IDC_CONTRIBLINK, TranslateT("Credits >"));
				ShowWindow(GetDlgItem(hwndDlg, IDC_DEVS), SW_SHOW);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BUILDTIME), SW_SHOW);
				ShowWindow(GetDlgItem(hwndDlg, IDC_CREDITSFILE), SW_HIDE);
			}
			else {
				iState = 1;
				SetDlgItemText(hwndDlg, IDC_CONTRIBLINK, TranslateT("< Copyright"));
				ShowWindow(GetDlgItem(hwndDlg, IDC_DEVS), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BUILDTIME), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_CREDITSFILE), SW_SHOW);
			}
			break;
		}
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		switch ( GetWindowLongPtr(( HWND )lParam, GWL_ID )) {
		case IDC_WHITERECT:
		case IDC_BUILDTIME:
		case IDC_CREDITSFILE:
		case IDC_DEVS:
			SetTextColor((HDC)wParam,GetSysColor(COLOR_WINDOWTEXT));
			break;
		default:
			return FALSE;
      }
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_DESTROY:
		Window_FreeIcon_IcoLib( hwndDlg );
		{	
			HFONT hFont=(HFONT)SendDlgItemMessage(hwndDlg,IDC_VERSION,WM_GETFONT,0,0);
			SendDlgItemMessage(hwndDlg,IDC_VERSION,WM_SETFONT,SendDlgItemMessage(hwndDlg,IDOK,WM_GETFONT,0,0),0);
			DeleteObject(hFont);				
		}
		break;
	}
	return FALSE;
}
