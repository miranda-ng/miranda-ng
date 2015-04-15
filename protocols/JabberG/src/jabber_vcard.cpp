/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-15 Miranda NG project

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

#include "jabber.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "jabber_iq.h"
#include "jabber_caps.h"

/////////////////////////////////////////////////////////////////////////////////////////

int CJabberProto::SendGetVcard(const TCHAR *jid)
{
	if (!m_bJabberOnline) return 0;

	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultGetVcard, JABBER_IQ_TYPE_GET, jid);
	m_ThreadInfo->send(XmlNodeIq(pInfo) << XCHILDNS(_T("vCard"), JABBER_FEAT_VCARD_TEMP)
		<< XATTR(_T("prodid"), _T("-//HandGen//NONSGML vGen v1.0//EN")) << XATTR(_T("version"), _T("2.0")));

	return pInfo->GetIqId();
}

/////////////////////////////////////////////////////////////////////////////////////////

static void SetDialogField(CJabberProto *ppro, HWND hwndDlg, int nDlgItem, char* key, bool bTranslate = false)
{
	ptrT tszValue(ppro->getTStringA(key));
	if (tszValue != NULL)
		SetDlgItemText(hwndDlg, nDlgItem, (bTranslate) ? TranslateTS(tszValue) : tszValue);
	else
		SetDlgItemTextA(hwndDlg, nDlgItem, "");
}

static INT_PTR CALLBACK PersonalDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 0;
	CJabberProto *ppro = (CJabberProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CJabberProto*)lParam;
			TranslateDialogDefault(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Male"));
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Female"));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			SendMessage(hwndDlg, WM_JABBER_REFRESH_VCARD, 0, 0);
			ppro->WindowSubscribe(hwndDlg);
		}
		break;

	case WM_JABBER_REFRESH_VCARD:
		SetDialogField(ppro, hwndDlg, IDC_FULLNAME, "FullName");
		SetDialogField(ppro, hwndDlg, IDC_NICKNAME, "Nick");
		SetDialogField(ppro, hwndDlg, IDC_FIRSTNAME, "FirstName");
		SetDialogField(ppro, hwndDlg, IDC_MIDDLE, "MiddleName");
		SetDialogField(ppro, hwndDlg, IDC_LASTNAME, "LastName");
		SetDialogField(ppro, hwndDlg, IDC_BIRTH, "BirthDate");
		SetDialogField(ppro, hwndDlg, IDC_GENDER, "GenderString", true);
		SetDialogField(ppro, hwndDlg, IDC_OCCUPATION, "Role");
		SetDialogField(ppro, hwndDlg, IDC_HOMEPAGE, "Homepage");
		break;

	case WM_COMMAND:
		if (((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE) ||
			((HWND)lParam == GetDlgItem(hwndDlg, IDC_GENDER) && (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE))) {
			ppro->m_vCardUpdates |= (1UL << iPageId);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY*)lParam)->lParam);
				break;
			case PSN_APPLY:
				ppro->m_vCardUpdates &= ~(1UL << iPageId);
				ppro->SaveVcardToDB(hwndDlg, iPageId);
				if (!ppro->m_vCardUpdates)
					ppro->SetServerVcard(ppro->m_bPhotoChanged, ppro->m_szPhotoFileName);
				break;
			}
		}
		break;

	case WM_DESTROY:
		ppro->WindowUnsubscribe(hwndDlg);
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK HomeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 1;
	CJabberProto *ppro = (CJabberProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CJabberProto*)lParam;
			TranslateDialogDefault(hwndDlg);
			for (int i = 0; i < g_cbCountries; i++) {
				if (g_countries[i].id != 0xFFFF && g_countries[i].id != 0) {
					TCHAR *country = mir_a2t(g_countries[i].szName);
					SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_ADDSTRING, 0, (LPARAM)TranslateTS(country));
					mir_free(country);
				}
			}
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			SendMessage(hwndDlg, WM_JABBER_REFRESH_VCARD, 0, 0);
			ppro->WindowSubscribe(hwndDlg);
		}
		break;

	case WM_JABBER_REFRESH_VCARD:
		SetDialogField(ppro, hwndDlg, IDC_ADDRESS1, "Street");
		SetDialogField(ppro, hwndDlg, IDC_ADDRESS2, "Street2");
		SetDialogField(ppro, hwndDlg, IDC_CITY, "City");
		SetDialogField(ppro, hwndDlg, IDC_STATE, "State");
		SetDialogField(ppro, hwndDlg, IDC_ZIP, "ZIP");
		SetDialogField(ppro, hwndDlg, IDC_COUNTRY, "Country", true);
		break;

	case WM_COMMAND:
		if (((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE) ||
			((HWND)lParam == GetDlgItem(hwndDlg, IDC_COUNTRY) && (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE)))
		{
			ppro->m_vCardUpdates |= (1UL << iPageId);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY*)lParam)->lParam);
				break;
			case PSN_APPLY:
				ppro->m_vCardUpdates &= ~(1UL << iPageId);
				ppro->SaveVcardToDB(hwndDlg, iPageId);
				if (!ppro->m_vCardUpdates)
					ppro->SetServerVcard(ppro->m_bPhotoChanged, ppro->m_szPhotoFileName);
				break;
			}
		}
		break;

	case WM_DESTROY:
		ppro->WindowUnsubscribe(hwndDlg);
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK WorkDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 2;
	CJabberProto *ppro = (CJabberProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) { // proto info is available
			ppro = (CJabberProto*)lParam;
			TranslateDialogDefault(hwndDlg);
			for (int i = 0; i < g_cbCountries; i++) {
				if (g_countries[i].id != 0xFFFF && g_countries[i].id != 0) {
					TCHAR *country = mir_a2t(g_countries[i].szName);
					SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_ADDSTRING, 0, (LPARAM)TranslateTS(country));
					mir_free(country);
				}
			}
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			SendMessage(hwndDlg, WM_JABBER_REFRESH_VCARD, 0, 0);
			ppro->WindowSubscribe(hwndDlg);
		}
		break;

	case WM_JABBER_REFRESH_VCARD:
		SetDialogField(ppro, hwndDlg, IDC_COMPANY, "Company");
		SetDialogField(ppro, hwndDlg, IDC_DEPARTMENT, "CompanyDepartment");
		SetDialogField(ppro, hwndDlg, IDC_TITLE, "CompanyPosition");
		SetDialogField(ppro, hwndDlg, IDC_ADDRESS1, "CompanyStreet");
		SetDialogField(ppro, hwndDlg, IDC_ADDRESS2, "CompanyStreet2");
		SetDialogField(ppro, hwndDlg, IDC_CITY, "CompanyCity");
		SetDialogField(ppro, hwndDlg, IDC_STATE, "CompanyState");
		SetDialogField(ppro, hwndDlg, IDC_ZIP, "CompanyZIP");
		SetDialogField(ppro, hwndDlg, IDC_COUNTRY, "CompanyCountry", true);
		break;

	case WM_COMMAND:
		if (((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE) ||
			((HWND)lParam == GetDlgItem(hwndDlg, IDC_COUNTRY) && (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE)))
		{
			ppro->m_vCardUpdates |= (1UL << iPageId);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY*)lParam)->lParam);
				break;
			case PSN_APPLY:
				ppro->m_vCardUpdates &= ~(1UL << iPageId);
				ppro->SaveVcardToDB(hwndDlg, iPageId);
				if (!ppro->m_vCardUpdates)
					ppro->SetServerVcard(ppro->m_bPhotoChanged, ppro->m_szPhotoFileName);
				break;
			}
		}
		break;

	case WM_DESTROY:
		ppro->WindowUnsubscribe(hwndDlg);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct PhotoDlgProcData
{
	CJabberProto *ppro;
	HBITMAP hBitmap;
};

static INT_PTR CALLBACK PhotoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 3;

	TCHAR szAvatarFileName[MAX_PATH], szTempPath[MAX_PATH], szTempFileName[MAX_PATH];
	PhotoDlgProcData* dat = (PhotoDlgProcData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (!lParam) break; // Launched from userinfo
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_LOAD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_OPEN), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
		SendDlgItemMessage(hwndDlg, IDC_LOAD, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_DELETE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_DELETE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
		SendDlgItemMessage(hwndDlg, IDC_DELETE, BUTTONSETASFLATBTN, TRUE, 0);
		ShowWindow(GetDlgItem(hwndDlg, IDC_SAVE), SW_HIDE);
		{
			dat = new PhotoDlgProcData;
			dat->ppro = (CJabberProto*)lParam;
			dat->hBitmap = NULL;
			dat->ppro->m_bPhotoChanged = FALSE;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->ppro->WindowSubscribe(hwndDlg);
		}
		SendMessage(hwndDlg, WM_JABBER_REFRESH_VCARD, 0, 0);
		break;

	case WM_JABBER_REFRESH_VCARD:
		if (dat->hBitmap) {
			DeleteObject(dat->hBitmap);
			dat->hBitmap = NULL;
			DeleteFile(dat->ppro->m_szPhotoFileName);
			dat->ppro->m_szPhotoFileName[0] = '\0';
		}
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
		dat->ppro->GetAvatarFileName(NULL, szAvatarFileName, SIZEOF(szAvatarFileName));
		if (_taccess(szAvatarFileName, 0) == 0) {
			if (GetTempPath(SIZEOF(szTempPath), szTempPath) <= 0)
				_tcscpy(szTempPath, _T(".\\"));
			if (GetTempFileName(szTempPath, _T("jab"), 0, szTempFileName) > 0) {
				dat->ppro->debugLog(_T("Temp file = %s"), szTempFileName);
				if (CopyFile(szAvatarFileName, szTempFileName, FALSE) == TRUE) {
					if ((dat->hBitmap = (HBITMAP)CallService(MS_UTILS_LOADBITMAPT, 0, (LPARAM)szTempFileName)) != NULL) {
						FIP->FI_Premultiply(dat->hBitmap);
						_tcscpy(dat->ppro->m_szPhotoFileName, szTempFileName);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
					}
					else DeleteFile(szTempFileName);
				}
				else DeleteFile(szTempFileName);
			}
		}

		dat->ppro->m_bPhotoChanged = FALSE;
		InvalidateRect(hwndDlg, NULL, TRUE);
		UpdateWindow(hwndDlg);
		break;

	case WM_JABBER_CHANGED:
		dat->ppro->SetServerVcard(dat->ppro->m_bPhotoChanged, dat->ppro->m_szPhotoFileName);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DELETE:
			if (dat->hBitmap) {
				DeleteObject(dat->hBitmap);
				dat->hBitmap = NULL;
				DeleteFile(dat->ppro->m_szPhotoFileName);
				dat->ppro->m_szPhotoFileName[0] = '\0';
				dat->ppro->m_bPhotoChanged = TRUE;
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
				InvalidateRect(hwndDlg, NULL, TRUE);
				UpdateWindow(hwndDlg);
				dat->ppro->m_vCardUpdates |= (1UL << iPageId);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_LOAD:
			TCHAR szFilter[512];
			TCHAR szFileName[MAX_PATH];

			CallService(MS_UTILS_GETBITMAPFILTERSTRINGST, SIZEOF(szFilter), (LPARAM)szFilter);

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = szFilter;
			ofn.lpstrCustomFilter = NULL;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT;
			szFileName[0] = '\0';
			if (GetOpenFileName(&ofn)) {
				struct _stat st;
				HBITMAP hNewBitmap;

				dat->ppro->debugLog(_T("File selected is %s"), szFileName);
				if (_tstat(szFileName, &st) < 0 || st.st_size > 40 * 1024) {
					MessageBox(hwndDlg, TranslateT("Only JPG, GIF, and BMP image files smaller than 40 KB are supported."), TranslateT("Jabber vCard"), MB_OK | MB_SETFOREGROUND);
					break;
				}
				if (GetTempPath(SIZEOF(szTempPath), szTempPath) <= 0)
					_tcscpy(szTempPath, _T(".\\"));
				if (GetTempFileName(szTempPath, _T("jab"), 0, szTempFileName) > 0) {
					dat->ppro->debugLog(_T("Temp file = %s"), szTempFileName);
					if (CopyFile(szFileName, szTempFileName, FALSE) == TRUE) {
						char* pszTemp = mir_t2a(szTempFileName);
						if ((hNewBitmap = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)pszTemp)) != NULL) {
							if (dat->hBitmap) {
								DeleteObject(dat->hBitmap);
								DeleteFile(dat->ppro->m_szPhotoFileName);
							}

							dat->hBitmap = hNewBitmap;
							_tcscpy(dat->ppro->m_szPhotoFileName, szTempFileName);
							dat->ppro->m_bPhotoChanged = TRUE;
							EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
							InvalidateRect(hwndDlg, NULL, TRUE);
							UpdateWindow(hwndDlg);
							dat->ppro->m_vCardUpdates |= (1UL << iPageId);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
						else DeleteFile(szTempFileName);

						mir_free(pszTemp);
					}
					else DeleteFile(szTempFileName);
				}
			}
			break;
		}
		break;

	case WM_PAINT:
		if (dat->hBitmap) {
			BITMAP bm;
			POINT ptSize, ptOrg, pt, ptFitSize;
			RECT rect;

			HWND hwndCanvas = GetDlgItem(hwndDlg, IDC_CANVAS);
			HDC hdcCanvas = GetDC(hwndCanvas);
			HDC hdcMem = CreateCompatibleDC(hdcCanvas);
			SelectObject(hdcMem, dat->hBitmap);
			SetMapMode(hdcMem, GetMapMode(hdcCanvas));
			GetObject(dat->hBitmap, sizeof(BITMAP), (LPVOID)&bm);
			ptSize.x = bm.bmWidth;
			ptSize.y = bm.bmHeight;
			DPtoLP(hdcCanvas, &ptSize, 1);
			ptOrg.x = ptOrg.y = 0;
			DPtoLP(hdcMem, &ptOrg, 1);
			GetClientRect(hwndCanvas, &rect);
			InvalidateRect(hwndCanvas, NULL, TRUE);
			UpdateWindow(hwndCanvas);
			if (ptSize.x <= rect.right && ptSize.y <= rect.bottom) {
				pt.x = (rect.right - ptSize.x) / 2;
				pt.y = (rect.bottom - ptSize.y) / 2;
				ptFitSize = ptSize;
			}
			else {
				if (((float)(ptSize.x - rect.right)) / ptSize.x > ((float)(ptSize.y - rect.bottom)) / ptSize.y) {
					ptFitSize.x = rect.right;
					ptFitSize.y = (ptSize.y*rect.right) / ptSize.x;
					pt.x = 0;
					pt.y = (rect.bottom - ptFitSize.y) / 2;
				}
				else {
					ptFitSize.x = (ptSize.x*rect.bottom) / ptSize.y;
					ptFitSize.y = rect.bottom;
					pt.x = (rect.right - ptFitSize.x) / 2;
					pt.y = 0;
				}
			}

			RECT rc;
			GetClientRect(hwndCanvas, &rc);
			if (IsThemeActive())
				DrawThemeParentBackground(hwndCanvas, hdcCanvas, &rc);
			else
				FillRect(hdcCanvas, &rc, (HBRUSH)GetSysColorBrush(COLOR_BTNFACE));

			if (bm.bmBitsPixel == 32) {
				BLENDFUNCTION bf = { 0 };
				bf.AlphaFormat = AC_SRC_ALPHA;
				bf.BlendOp = AC_SRC_OVER;
				bf.SourceConstantAlpha = 255;
				GdiAlphaBlend(hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, bf);
			}
			else {
				SetStretchBltMode(hdcCanvas, COLORONCOLOR);
				StretchBlt(hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, SRCCOPY);
			}

			DeleteDC(hdcMem);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY*)lParam)->lParam);
				break;

			case PSN_APPLY:
				dat->ppro->m_vCardUpdates &= ~(1UL << iPageId);
				dat->ppro->SaveVcardToDB(hwndDlg, iPageId);
				if (!dat->ppro->m_vCardUpdates)
					dat->ppro->SetServerVcard(dat->ppro->m_bPhotoChanged, dat->ppro->m_szPhotoFileName);
				break;
			}
		}
		break;

	case WM_DESTROY:
		DestroyIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_LOAD, BM_SETIMAGE, IMAGE_ICON, 0));
		DestroyIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_DELETE, BM_SETIMAGE, IMAGE_ICON, 0));
		dat->ppro->WindowUnsubscribe(hwndDlg);
		if (dat->hBitmap) {
			dat->ppro->debugLogA("Delete bitmap");
			DeleteObject(dat->hBitmap);
			DeleteFile(dat->ppro->m_szPhotoFileName);
		}
		delete dat;
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK NoteDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 4;
	CJabberProto *ppro = (CJabberProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (!lParam) break; // Launched from userinfo
		ppro = (CJabberProto*)lParam;
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SendMessage(hwndDlg, WM_JABBER_REFRESH_VCARD, 0, 0);
		ppro->WindowSubscribe(hwndDlg);
		break;

	case WM_JABBER_REFRESH_VCARD:
		SetDialogField(ppro, hwndDlg, IDC_DESC, "About");
		break;

	case WM_COMMAND:
		if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE) {
			ppro->m_vCardUpdates |= (1UL << iPageId);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY*)lParam)->lParam);
				break;
			case PSN_APPLY:
				ppro->m_vCardUpdates &= ~(1UL<<iPageId);
				ppro->SaveVcardToDB(hwndDlg, iPageId);
				if (!ppro->m_vCardUpdates)
					ppro->SetServerVcard(ppro->m_bPhotoChanged, ppro->m_szPhotoFileName);
				break;
			}
		}
		break;
	case WM_DESTROY:
		ppro->WindowUnsubscribe(hwndDlg);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct EditDlgParam
{
	int id;
	CJabberProto *ppro;
};

static INT_PTR CALLBACK EditEmailDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EditDlgParam *dat = (EditDlgParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		dat = (EditDlgParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		TranslateDialogDefault(hwndDlg);

		if (lParam >= 0) {
			SetWindowText(hwndDlg, TranslateT("Jabber vCard: Edit Email Address"));

			char idstr[33];
			mir_snprintf(idstr, SIZEOF(idstr), "e-mail%d", dat->id);
			ptrA email(dat->ppro->getStringA(idstr));
			if (email != NULL)
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, email);

			mir_snprintf(idstr, SIZEOF(idstr), "e-mailFlag%d", lParam);
			WORD nFlag = dat->ppro->getWord(idstr, 0);
			if (nFlag & JABBER_VCEMAIL_HOME) CheckDlgButton(hwndDlg, IDC_HOME, BST_CHECKED);
			if (nFlag & JABBER_VCEMAIL_WORK) CheckDlgButton(hwndDlg, IDC_WORK, BST_CHECKED);
			if (nFlag & JABBER_VCEMAIL_INTERNET) CheckDlgButton(hwndDlg, IDC_INTERNET, BST_CHECKED);
			if (nFlag & JABBER_VCEMAIL_X400) CheckDlgButton(hwndDlg, IDC_X400, BST_CHECKED);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			char idstr[33];

			if (dat->id < 0)
				for (dat->id = 0;; dat->id++) {
					mir_snprintf(idstr, SIZEOF(idstr), "e-mail%d", dat->id);
					if (ptrA(dat->ppro->getStringA(idstr)) == NULL)
						break;
				}

			TCHAR text[128];
			GetDlgItemText(hwndDlg, IDC_EMAIL, text, SIZEOF(text));
			mir_snprintf(idstr, SIZEOF(idstr), "e-mail%d", dat->id);
			dat->ppro->setTString(idstr, text);
			{
				WORD nFlag = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_HOME)) nFlag |= JABBER_VCEMAIL_HOME;
				if (IsDlgButtonChecked(hwndDlg, IDC_WORK)) nFlag |= JABBER_VCEMAIL_WORK;
				if (IsDlgButtonChecked(hwndDlg, IDC_INTERNET)) nFlag |= JABBER_VCEMAIL_INTERNET;
				if (IsDlgButtonChecked(hwndDlg, IDC_X400)) nFlag |= JABBER_VCEMAIL_X400;
				mir_snprintf(idstr, SIZEOF(idstr), "e-mailFlag%d", dat->id);
				dat->ppro->setWord(idstr, nFlag);
			}
			// fall through
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			break;
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK EditPhoneDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EditDlgParam* dat = (EditDlgParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		dat = (EditDlgParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		TranslateDialogDefault(hwndDlg);
		if (dat->id >= 0) {
			SetWindowText(hwndDlg, TranslateT("Jabber vCard: Edit Phone Number"));

			char idstr[33];
			mir_snprintf(idstr, SIZEOF(idstr), "Phone%d", dat->id);
			ptrA phone(dat->ppro->getStringA(idstr));
			if (phone != NULL)
				SetDlgItemTextA(hwndDlg, IDC_PHONE, phone);

			mir_snprintf(idstr, SIZEOF(idstr), "PhoneFlag%d", dat->id);
			int nFlag = dat->ppro->getWord(idstr, 0);
			if (nFlag & JABBER_VCTEL_HOME)  CheckDlgButton(hwndDlg, IDC_HOME, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_WORK)  CheckDlgButton(hwndDlg, IDC_WORK, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_VOICE) CheckDlgButton(hwndDlg, IDC_VOICE, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_FAX)   CheckDlgButton(hwndDlg, IDC_FAX, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_PAGER) CheckDlgButton(hwndDlg, IDC_PAGER, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_MSG)   CheckDlgButton(hwndDlg, IDC_MSG, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_CELL)  CheckDlgButton(hwndDlg, IDC_CELL, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_VIDEO) CheckDlgButton(hwndDlg, IDC_VIDEO, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_BBS)   CheckDlgButton(hwndDlg, IDC_BBS, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_MODEM) CheckDlgButton(hwndDlg, IDC_MODEM, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_ISDN)  CheckDlgButton(hwndDlg, IDC_ISDN, BST_CHECKED);
			if (nFlag & JABBER_VCTEL_PCS)   CheckDlgButton(hwndDlg, IDC_PCS, BST_CHECKED);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		{
			char idstr[33];

			if (dat->id < 0)
				for (dat->id = 0;; dat->id++) {
					mir_snprintf(idstr, SIZEOF(idstr), "Phone%d", dat->id);
					if (ptrA(dat->ppro->getStringA(idstr)) == NULL)
						break;
				}

			char text[128];
			GetDlgItemTextA(hwndDlg, IDC_PHONE, text, SIZEOF(text));
			mir_snprintf(idstr, SIZEOF(idstr), "Phone%d", dat->id);
			dat->ppro->setString(idstr, text);

			WORD nFlag = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_HOME))  nFlag |= JABBER_VCTEL_HOME;
			if (IsDlgButtonChecked(hwndDlg, IDC_WORK))  nFlag |= JABBER_VCTEL_WORK;
			if (IsDlgButtonChecked(hwndDlg, IDC_VOICE)) nFlag |= JABBER_VCTEL_VOICE;
			if (IsDlgButtonChecked(hwndDlg, IDC_FAX))   nFlag |= JABBER_VCTEL_FAX;
			if (IsDlgButtonChecked(hwndDlg, IDC_PAGER)) nFlag |= JABBER_VCTEL_PAGER;
			if (IsDlgButtonChecked(hwndDlg, IDC_MSG))   nFlag |= JABBER_VCTEL_MSG;
			if (IsDlgButtonChecked(hwndDlg, IDC_CELL))  nFlag |= JABBER_VCTEL_CELL;
			if (IsDlgButtonChecked(hwndDlg, IDC_VIDEO)) nFlag |= JABBER_VCTEL_VIDEO;
			if (IsDlgButtonChecked(hwndDlg, IDC_BBS))   nFlag |= JABBER_VCTEL_BBS;
			if (IsDlgButtonChecked(hwndDlg, IDC_MODEM)) nFlag |= JABBER_VCTEL_MODEM;
			if (IsDlgButtonChecked(hwndDlg, IDC_ISDN))  nFlag |= JABBER_VCTEL_ISDN;
			if (IsDlgButtonChecked(hwndDlg, IDC_PCS))   nFlag |= JABBER_VCTEL_PCS;
			mir_snprintf(idstr, SIZEOF(idstr), "PhoneFlag%d", dat->id);
			dat->ppro->setWord(idstr, nFlag);
		}
		// fall through
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			break;
		}
	}
	return FALSE;
}

#define M_REMAKELISTS  (WM_USER+1)
static INT_PTR CALLBACK ContactDlgProc(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	const unsigned long iPageId = 5;
	CJabberProto *ppro = (CJabberProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (!lParam) break; // Launched from userinfo
		ppro = (CJabberProto*)lParam;
		{
			LVCOLUMN lvc;
			RECT rc;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			TranslateDialogDefault(hwndDlg);
			GetClientRect(GetDlgItem(hwndDlg, IDC_EMAILS), &rc);
			rc.right -= GetSystemMetrics(SM_CXVSCROLL);
			lvc.mask = LVCF_WIDTH;
			lvc.cx = 30;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_EMAILS), 0, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PHONES), 0, &lvc);
			lvc.cx = rc.right - 30 - 40;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_EMAILS), 1, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PHONES), 1, &lvc);
			lvc.cx = 20;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_EMAILS), 2, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_EMAILS), 3, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PHONES), 2, &lvc);
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_PHONES), 3, &lvc);
			SendMessage(hwndDlg, M_REMAKELISTS, 0, 0);

			ppro->WindowSubscribe(hwndDlg);
		}
		break;
	case M_REMAKELISTS:
		{
			int i;
			char idstr[33];
			TCHAR number[20];

			//e-mails
			ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_EMAILS));

			LVITEM lvi;
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = 0;
			for (i=0;;i++) {
				mir_snprintf(idstr, SIZEOF(idstr), "e-mail%d", i);
				ptrT email( ppro->getTStringA(idstr));
				if (email == NULL) break;

				mir_sntprintf(number, SIZEOF(number), _T("%d"), i+1);
				lvi.pszText = number;
				lvi.lParam = (LPARAM)i;
				ListView_InsertItem(GetDlgItem(hwndDlg, IDC_EMAILS), &lvi);
				ListView_SetItemText(GetDlgItem(hwndDlg, IDC_EMAILS), lvi.iItem, 1, email);
				lvi.iItem++;
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = (LPARAM)(-1);
			ListView_InsertItem(GetDlgItem(hwndDlg, IDC_EMAILS), &lvi);

			//phones
			ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_PHONES));
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = 0;
			for (i=0;;i++) {
				mir_snprintf(idstr, SIZEOF(idstr), "Phone%d", i);
				ptrT phone( ppro->getTStringA(idstr));
				if (phone == NULL) break;

				mir_sntprintf(number, SIZEOF(number), _T("%d"), i+1);
				lvi.pszText = number;
				lvi.lParam = (LPARAM)i;
				ListView_InsertItem(GetDlgItem(hwndDlg, IDC_PHONES), &lvi);
				ListView_SetItemText(GetDlgItem(hwndDlg, IDC_PHONES), lvi.iItem, 1, phone);
				lvi.iItem++;
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = (LPARAM)(-1);
			ListView_InsertItem(GetDlgItem(hwndDlg, IDC_PHONES), &lvi);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY*)lParam)->lParam);
				break;

			case PSN_APPLY:
				ppro->m_vCardUpdates &= ~(1UL << iPageId);
				ppro->SaveVcardToDB(hwndDlg, iPageId);
				if (!ppro->m_vCardUpdates)
					ppro->SetServerVcard(ppro->m_bPhotoChanged, ppro->m_szPhotoFileName);
				break;
			}
			break;

		case IDC_EMAILS:
		case IDC_PHONES:
			switch (((LPNMHDR)lParam)->code) {
			case NM_CUSTOMDRAW:
				{
					NMLVCUSTOMDRAW *nm = (NMLVCUSTOMDRAW *)lParam;

					switch (nm->nmcd.dwDrawStage) {
					case CDDS_PREPAINT:
					case CDDS_ITEMPREPAINT:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW);
						return TRUE;

					case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
						RECT rc;
						HICON hIcon;

						ListView_GetSubItemRect(nm->nmcd.hdr.hwndFrom, nm->nmcd.dwItemSpec, nm->iSubItem, LVIR_LABEL, &rc);
						if (nm->nmcd.lItemlParam == (LPARAM)(-1) && nm->iSubItem == 3)
							hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ADDCONTACT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
						else if (nm->iSubItem == 2 && nm->nmcd.lItemlParam != (LPARAM)(-1))
							hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_EDIT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
						else if (nm->iSubItem == 3 && nm->nmcd.lItemlParam != (LPARAM)(-1))
							hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_DELETE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
						else break;
						DrawIconEx(nm->nmcd.hdc, (rc.left + rc.right - GetSystemMetrics(SM_CXSMICON)) / 2, (rc.top + rc.bottom - GetSystemMetrics(SM_CYSMICON)) / 2, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
						DestroyIcon(hIcon);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
						return TRUE;
					}
				}
				break;

			case NM_CLICK:
				NMLISTVIEW *nm = (NMLISTVIEW *)lParam;
				if (nm->iSubItem < 2)
					break;

				const char* szIdTemplate = (nm->hdr.idFrom == IDC_PHONES) ? "Phone%d" : "e-mail%d";
				const char* szFlagTemplate = (nm->hdr.idFrom == IDC_PHONES) ? "PhoneFlag%d" : "e-mailFlag%d";

				LVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(nm->hdr.hwndFrom, &hti.pt);
				if (ListView_SubItemHitTest(nm->hdr.hwndFrom, &hti) == -1)
					break;

				LVITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = hti.iItem;
				lvi.iSubItem = 0;
				ListView_GetItem(nm->hdr.hwndFrom, &lvi);
				if (lvi.lParam == (LPARAM)(-1)) {
					if (hti.iSubItem == 3) {
						//add
						EditDlgParam param = { -1, ppro };
						int res;
						if (nm->hdr.idFrom == IDC_PHONES)
							res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VCARD_ADDPHONE), hwndDlg, EditPhoneDlgProc, (LPARAM)&param);
						else
							res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VCARD_ADDEMAIL), hwndDlg, EditEmailDlgProc, (LPARAM)&param);
						if (res != IDOK)
							break;
						SendMessage(hwndDlg, M_REMAKELISTS, 0, 0);
						ppro->m_vCardUpdates |= (1UL << iPageId);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
				else {
					if (hti.iSubItem == 3) {
						//delete
						char idstr[33];

						int i;
						for (i = lvi.lParam;; i++) {
							mir_snprintf(idstr, SIZEOF(idstr), szIdTemplate, i + 1);
							ptrA fieldVal(ppro->getStringA(idstr));
							if (fieldVal == NULL) break;
							mir_snprintf(idstr, SIZEOF(idstr), szIdTemplate, i);
							ppro->setString(idstr, fieldVal);

							mir_snprintf(idstr, SIZEOF(idstr), szFlagTemplate, i + 1);
							WORD nFlag = ppro->getWord(idstr, 0);
							mir_snprintf(idstr, SIZEOF(idstr), szFlagTemplate, i);
							ppro->setWord(idstr, nFlag);
						}
						mir_snprintf(idstr, SIZEOF(idstr), szIdTemplate, i);
						ppro->delSetting(idstr);
						mir_snprintf(idstr, SIZEOF(idstr), szFlagTemplate, i);
						ppro->delSetting(idstr);
						SendMessage(hwndDlg, M_REMAKELISTS, 0, 0);
						ppro->m_vCardUpdates |= (1UL << iPageId);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					else if (hti.iSubItem == 2) {
						EditDlgParam param = { lvi.lParam, ppro };
						int res;
						if (nm->hdr.idFrom == IDC_PHONES)
							res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VCARD_ADDPHONE), hwndDlg, EditPhoneDlgProc, (LPARAM)&param);
						else
							res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VCARD_ADDEMAIL), hwndDlg, EditEmailDlgProc, (LPARAM)&param);
						if (res != IDOK)
							break;
						SendMessage(hwndDlg, M_REMAKELISTS, 0, 0);
						ppro->m_vCardUpdates |= (1UL << iPageId);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
		}
		break;

	case WM_SETCURSOR:
		if (LOWORD(lParam) != HTCLIENT) break;
		if (GetForegroundWindow() == GetParent(hwndDlg)) {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwndDlg, &pt);
			SetFocus(ChildWindowFromPoint(hwndDlg, pt));	  //ugly hack because listviews ignore their first click
		}
		break;

	case WM_DESTROY:
		ppro->WindowUnsubscribe(hwndDlg);
		break;
	}
	return FALSE;
}

void CJabberProto::SaveVcardToDB(HWND hwndPage, int iPage)
{
	TCHAR text[2048];

	// Page 0: Personal
	switch (iPage) {
	case 0:
		GetDlgItemText(hwndPage, IDC_FULLNAME, text, SIZEOF(text));
		setTString("FullName", text);
		GetDlgItemText(hwndPage, IDC_NICKNAME, text, SIZEOF(text));
		setTString("Nick", text);
		GetDlgItemText(hwndPage, IDC_FIRSTNAME, text, SIZEOF(text));
		setTString("FirstName", text);
		GetDlgItemText(hwndPage, IDC_MIDDLE, text, SIZEOF(text));
		setTString("MiddleName", text);
		GetDlgItemText(hwndPage, IDC_LASTNAME, text, SIZEOF(text));
		setTString("LastName", text);
		GetDlgItemText(hwndPage, IDC_BIRTH, text, SIZEOF(text));
		setTString("BirthDate", text);
		switch (SendDlgItemMessage(hwndPage, IDC_GENDER, CB_GETCURSEL, 0, 0)) {
			case 0:	setString("GenderString", "Male");   break;
			case 1:	setString("GenderString", "Female"); break;
			default: setString("GenderString", "");       break;
		}
		GetDlgItemText(hwndPage, IDC_OCCUPATION, text, SIZEOF(text));
		setTString("Role", text);
		GetDlgItemText(hwndPage, IDC_HOMEPAGE, text, SIZEOF(text));
		setTString("Homepage", text);
		break;

	// Page 1: Home
	case 1:
		GetDlgItemText(hwndPage, IDC_ADDRESS1, text, SIZEOF(text));
		setTString("Street", text);
		GetDlgItemText(hwndPage, IDC_ADDRESS2, text, SIZEOF(text));
		setTString("Street2", text);
		GetDlgItemText(hwndPage, IDC_CITY, text, SIZEOF(text));
		setTString("City", text);
		GetDlgItemText(hwndPage, IDC_STATE, text, SIZEOF(text));
		setTString("State", text);
		GetDlgItemText(hwndPage, IDC_ZIP, text, SIZEOF(text));
		setTString("ZIP", text);
		{
			int i = SendDlgItemMessage(hwndPage, IDC_COUNTRY, CB_GETCURSEL, 0, 0);
			TCHAR *country = mir_a2t((i) ? g_countries[i + 2].szName : g_countries[1].szName);
			setTString("Country", country);
			mir_free(country);
		}
		break;

	// Page 2: Work
	case 2:
		GetDlgItemText(hwndPage, IDC_COMPANY, text, SIZEOF(text));
		setTString("Company", text);
		GetDlgItemText(hwndPage, IDC_DEPARTMENT, text, SIZEOF(text));
		setTString("CompanyDepartment", text);
		GetDlgItemText(hwndPage, IDC_TITLE, text, SIZEOF(text));
		setTString("CompanyPosition", text);
		GetDlgItemText(hwndPage, IDC_ADDRESS1, text, SIZEOF(text));
		setTString("CompanyStreet", text);
		GetDlgItemText(hwndPage, IDC_ADDRESS2, text, SIZEOF(text));
		setTString("CompanyStreet2", text);
		GetDlgItemText(hwndPage, IDC_CITY, text, SIZEOF(text));
		setTString("CompanyCity", text);
		GetDlgItemText(hwndPage, IDC_STATE, text, SIZEOF(text));
		setTString("CompanyState", text);
		GetDlgItemText(hwndPage, IDC_ZIP, text, SIZEOF(text));
		setTString("CompanyZIP", text);
		{
			int i = SendDlgItemMessage(hwndPage, IDC_COUNTRY, CB_GETCURSEL, 0, 0);
			TCHAR *country = mir_a2t((i) ? g_countries[i + 2].szName : g_countries[1].szName);
			setTString("CompanyCountry", country);
			mir_free(country);
		}
		break;

	// Page 3: Photo
	// not needed to be saved into db

	// Page 4: Note
	case 4:
		GetDlgItemText(hwndPage, IDC_DESC, text, SIZEOF(text));
		setTString("About", text);
		break;
	}
}

void CJabberProto::AppendVcardFromDB(HXML n, char *tag, char *key)
{
	if (n == NULL || tag == NULL || key == NULL)
		return;

	ptrT tszValue(getTStringA(key));
	n << XCHILD(_A2T(tag), tszValue);
}

void CJabberProto::SetServerVcard(BOOL bPhotoChanged, TCHAR* szPhotoFileName)
{
	if (!m_bJabberOnline) return;

	int  i;
	char idstr[33];

	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultSetVcard, JABBER_IQ_TYPE_SET));
	HXML v = iq << XCHILDNS(_T("vCard"), JABBER_FEAT_VCARD_TEMP);

	AppendVcardFromDB(v, "FN", "FullName");

	HXML n = v << XCHILD(_T("N"));
	AppendVcardFromDB(n, "GIVEN", "FirstName");
	AppendVcardFromDB(n, "MIDDLE", "MiddleName");
	AppendVcardFromDB(n, "FAMILY", "LastName");

	AppendVcardFromDB(v, "NICKNAME", "Nick");
	AppendVcardFromDB(v, "BDAY", "BirthDate");
	AppendVcardFromDB(v, "GENDER", "GenderString");

	for (i = 0;; i++) {
		mir_snprintf(idstr, SIZEOF(idstr), "e-mail%d", i);
		ptrT email(getTStringA(idstr));
		if (email == NULL)
			break;

		HXML e = v << XCHILD(_T("EMAIL"), email);
		AppendVcardFromDB(e, "USERID", idstr);

		mir_snprintf(idstr, SIZEOF(idstr), "e-mailFlag%d", i);
		WORD nFlag = getWord(idstr, 0);
		if (nFlag & JABBER_VCEMAIL_HOME)     e << XCHILD(_T("HOME"));
		if (nFlag & JABBER_VCEMAIL_WORK)     e << XCHILD(_T("WORK"));
		if (nFlag & JABBER_VCEMAIL_INTERNET) e << XCHILD(_T("INTERNET"));
		if (nFlag & JABBER_VCEMAIL_X400)     e << XCHILD(_T("X400"));
	}

	n = v << XCHILD(_T("ADR"));
	n << XCHILD(_T("HOME"));
	AppendVcardFromDB(n, "STREET", "Street");
	AppendVcardFromDB(n, "EXTADR", "Street2");
	AppendVcardFromDB(n, "EXTADD", "Street2");	// for compatibility with client using old vcard format
	AppendVcardFromDB(n, "LOCALITY", "City");
	AppendVcardFromDB(n, "REGION", "State");
	AppendVcardFromDB(n, "PCODE", "ZIP");
	AppendVcardFromDB(n, "CTRY", "Country");
	AppendVcardFromDB(n, "COUNTRY", "Country");	// for compatibility with client using old vcard format

	n = v << XCHILD(_T("ADR"));
	n << XCHILD(_T("WORK"));
	AppendVcardFromDB(n, "STREET", "CompanyStreet");
	AppendVcardFromDB(n, "EXTADR", "CompanyStreet2");
	AppendVcardFromDB(n, "EXTADD", "CompanyStreet2");	// for compatibility with client using old vcard format
	AppendVcardFromDB(n, "LOCALITY", "CompanyCity");
	AppendVcardFromDB(n, "REGION", "CompanyState");
	AppendVcardFromDB(n, "PCODE", "CompanyZIP");
	AppendVcardFromDB(n, "CTRY", "CompanyCountry");
	AppendVcardFromDB(n, "COUNTRY", "CompanyCountry");	// for compatibility with client using old vcard format

	n = v << XCHILD(_T("ORG"));
	AppendVcardFromDB(n, "ORGNAME", "Company");
	AppendVcardFromDB(n, "ORGUNIT", "CompanyDepartment");

	AppendVcardFromDB(v, "TITLE", "CompanyPosition");
	AppendVcardFromDB(v, "ROLE", "Role");
	AppendVcardFromDB(v, "URL", "Homepage");
	AppendVcardFromDB(v, "DESC", "About");

	for (i = 0;; i++) {
		mir_snprintf(idstr, SIZEOF(idstr), "Phone%d", i);
		ptrT phone(getTStringA(idstr));
		if (phone == NULL)
			break;

		n = v << XCHILD(_T("TEL"));
		AppendVcardFromDB(n, "NUMBER", idstr);

		mir_snprintf(idstr, SIZEOF(idstr), "PhoneFlag%d", i);
		WORD nFlag = getWord(idstr, 0);
		if (nFlag & JABBER_VCTEL_HOME)  n << XCHILD(_T("HOME"));
		if (nFlag & JABBER_VCTEL_WORK)  n << XCHILD(_T("WORK"));
		if (nFlag & JABBER_VCTEL_VOICE) n << XCHILD(_T("VOICE"));
		if (nFlag & JABBER_VCTEL_FAX)   n << XCHILD(_T("FAX"));
		if (nFlag & JABBER_VCTEL_PAGER) n << XCHILD(_T("PAGER"));
		if (nFlag & JABBER_VCTEL_MSG)   n << XCHILD(_T("MSG"));
		if (nFlag & JABBER_VCTEL_CELL)  n << XCHILD(_T("CELL"));
		if (nFlag & JABBER_VCTEL_VIDEO) n << XCHILD(_T("VIDEO"));
		if (nFlag & JABBER_VCTEL_BBS)   n << XCHILD(_T("BBS"));
		if (nFlag & JABBER_VCTEL_MODEM) n << XCHILD(_T("MODEM"));
		if (nFlag & JABBER_VCTEL_ISDN)  n << XCHILD(_T("ISDN"));
		if (nFlag & JABBER_VCTEL_PCS)   n << XCHILD(_T("PCS"));
	}

	TCHAR szAvatarName[MAX_PATH], *szFileName;
	GetAvatarFileName(NULL, szAvatarName, SIZEOF(szAvatarName));
	if (bPhotoChanged)
		szFileName = szPhotoFileName;
	else
		szFileName = szAvatarName;

	// Set photo element, also update the global jabberVcardPhotoFileName to reflect the update
	debugLog(_T("Before update, file name = %s"), szFileName);
	if (szFileName == NULL || szFileName[0] == 0) {
		v << XCHILD(_T("PHOTO"));
		DeleteFile(szAvatarName);
		delSetting("AvatarSaved");
		delSetting("AvatarHash");
	}
	else {
		debugLog(_T("Saving picture from %s"), szFileName);

		struct _stat st;
		if (_tstat(szFileName, &st) >= 0) {
			// Note the FILE_SHARE_READ attribute so that the CopyFile can succeed
			HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				ptrA buffer((char*)mir_alloc(st.st_size));
				if (buffer != NULL) {
					DWORD nRead;
					if (ReadFile(hFile, buffer, st.st_size, &nRead, NULL)) {
						ptrA str(mir_base64_encode((PBYTE)(LPSTR)buffer, nRead));
						if (str != NULL) {
							n = v << XCHILD(_T("PHOTO"));
							TCHAR *szFileType;
							switch (ProtoGetBufferFormat(buffer)) {
								case PA_FORMAT_PNG:  szFileType = _T("image/png");   break;
								case PA_FORMAT_GIF:  szFileType = _T("image/gif");   break;
								case PA_FORMAT_BMP:  szFileType = _T("image/bmp");   break;
								default:             szFileType = _T("image/jpeg");  break;
							}
							n << XCHILD(_T("TYPE"), szFileType);
							n << XCHILD(_T("BINVAL"), _A2T(str));

							// NEED TO UPDATE OUR AVATAR HASH:
							BYTE digest[MIR_SHA1_HASH_SIZE];
							mir_sha1_ctx sha1ctx;
							mir_sha1_init(&sha1ctx);
							mir_sha1_append(&sha1ctx, (BYTE*)(LPSTR)buffer, nRead);
							mir_sha1_finish(&sha1ctx, digest);

							char buf[MIR_SHA1_HASH_SIZE * 2 + 1];
							bin2hex(digest, sizeof(digest), buf);

							m_options.AvatarType = ProtoGetBufferFormat(buffer);

							if (bPhotoChanged) {
								DeleteFile(szAvatarName);

								GetAvatarFileName(NULL, szAvatarName, SIZEOF(szAvatarName));
								CopyFile(szFileName, szAvatarName, FALSE);
							}

							setString("AvatarHash", buf);
							setString("AvatarSaved", buf);
						}
					}
				}
				CloseHandle(hFile);
			}
		}
	}

	m_ThreadInfo->send(iq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnUserInfoInit_VCard(WPARAM wParam, LPARAM)
{
	m_vCardUpdates = 0;
	m_bPhotoChanged = FALSE;
	m_szPhotoFileName[0] = 0;

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.dwInitParam = (LPARAM)this;
	odp.flags = ODPF_TCHAR | ODPF_USERINFOTAB | ODPF_DONTTRANSLATE;
	odp.ptszTitle = m_tszUserName;

	odp.pfnDlgProc = PersonalDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_PERSONAL);
	odp.ptszTab = LPGENT("General");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = ContactDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_CONTACT);
	odp.ptszTab = LPGENT("Contacts");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = HomeDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_HOME);
	odp.ptszTab = LPGENT("Home");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = WorkDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_WORK);
	odp.ptszTab = LPGENT("Work");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = PhotoDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_PHOTO);
	odp.ptszTab = LPGENT("Photo");
	UserInfo_AddPage(wParam, &odp);

	odp.pfnDlgProc = NoteDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_NOTE);
	odp.ptszTab = LPGENT("Note");
	UserInfo_AddPage(wParam, &odp);

	SendGetVcard(m_szJabberJID);
}
