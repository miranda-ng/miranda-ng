/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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

#include "stdafx.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "jabber_iq.h"
#include "jabber_caps.h"

/////////////////////////////////////////////////////////////////////////////////////////

int CJabberProto::SendGetVcard(MCONTACT hContact)
{
	if (!m_bJabberOnline) return 0;

	CJabberIqInfo *pInfo;

	if (hContact == 0) {
		setDword("LastGetVcard", time(0));
		pInfo = AddIQ(&CJabberProto::OnIqResultGetVcard, JABBER_IQ_TYPE_GET, m_szJabberJID);
	}
	else {
		ptrA jid(getUStringA(hContact, "jid"));
		if (jid == nullptr)
			return -1;

		pInfo = AddIQ(&CJabberProto::OnIqResultGetVcard, JABBER_IQ_TYPE_GET, jid);
	}

	m_ThreadInfo->send(XmlNodeIq(pInfo) << XCHILDNS("vCard", JABBER_FEAT_VCARD_TEMP)
		<< XATTR("prodid", "-//HandGen//NONSGML vGen v1.0//EN") << XATTR("version", "2.0"));

	return pInfo->GetIqId();
}

/////////////////////////////////////////////////////////////////////////////////////////

static void SetDialogField(CJabberProto *ppro, HWND hwndDlg, int nDlgItem, char *key, bool bTranslate = false)
{
	ptrW tszValue(ppro->getWStringA(key));
	if (tszValue != nullptr)
		SetDlgItemText(hwndDlg, nDlgItem, (bTranslate) ? TranslateW(tszValue) : tszValue);
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
					wchar_t *country = mir_a2u(g_countries[i].szName);
					SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_ADDSTRING, 0, (LPARAM)TranslateW(country));
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
			((HWND)lParam == GetDlgItem(hwndDlg, IDC_COUNTRY) && (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE))) {
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
					wchar_t *country = mir_a2u(g_countries[i].szName);
					SendDlgItemMessage(hwndDlg, IDC_COUNTRY, CB_ADDSTRING, 0, (LPARAM)TranslateW(country));
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
			((HWND)lParam == GetDlgItem(hwndDlg, IDC_COUNTRY) && (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE))) {
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

	wchar_t szAvatarFileName[MAX_PATH], szTempPath[MAX_PATH], szTempFileName[MAX_PATH];
	PhotoDlgProcData *dat = (PhotoDlgProcData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (!lParam) break; // Launched from userinfo
		TranslateDialogDefault(hwndDlg);
		Button_SetIcon_IcoLib(hwndDlg, IDC_LOAD, g_plugin.getIconHandle(IDI_OPEN));
		Button_SetIcon_IcoLib(hwndDlg, IDC_DELETE, g_plugin.getIconHandle(IDI_DELETE));
		ShowWindow(GetDlgItem(hwndDlg, IDC_SAVE), SW_HIDE);
		{
			dat = new PhotoDlgProcData;
			dat->ppro = (CJabberProto*)lParam;
			dat->hBitmap = nullptr;
			dat->ppro->m_bPhotoChanged = false;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->ppro->WindowSubscribe(hwndDlg);
		}
		SendMessage(hwndDlg, WM_JABBER_REFRESH_VCARD, 0, 0);
		break;

	case WM_JABBER_REFRESH_VCARD:
		if (dat->hBitmap) {
			DeleteObject(dat->hBitmap);
			dat->hBitmap = nullptr;
			DeleteFile(dat->ppro->m_szPhotoFileName);
			dat->ppro->m_szPhotoFileName[0] = '\0';
		}
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
		dat->ppro->GetAvatarFileName(0, szAvatarFileName, _countof(szAvatarFileName));
		if (_waccess(szAvatarFileName, 0) == 0) {
			if (GetTempPath(_countof(szTempPath), szTempPath) <= 0)
				mir_wstrcpy(szTempPath, L".\\");
			if (GetTempFileName(szTempPath, L"jab", 0, szTempFileName) > 0) {
				dat->ppro->debugLogW(L"Temp file = %s", szTempFileName);
				if (CopyFile(szAvatarFileName, szTempFileName, FALSE) == TRUE) {
					if ((dat->hBitmap = Bitmap_Load(szTempFileName)) != nullptr) {
						FreeImage_Premultiply(dat->hBitmap);
						mir_wstrcpy(dat->ppro->m_szPhotoFileName, szTempFileName);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
					}
					else DeleteFile(szTempFileName);
				}
				else DeleteFile(szTempFileName);
			}
		}

		dat->ppro->m_bPhotoChanged = false;
		InvalidateRect(hwndDlg, nullptr, TRUE);
		UpdateWindow(hwndDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DELETE:
			if (dat->hBitmap) {
				DeleteObject(dat->hBitmap);
				dat->hBitmap = nullptr;
				DeleteFile(dat->ppro->m_szPhotoFileName);
				dat->ppro->m_szPhotoFileName[0] = '\0';
				dat->ppro->m_bPhotoChanged = true;
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
				InvalidateRect(hwndDlg, nullptr, TRUE);
				UpdateWindow(hwndDlg);
				dat->ppro->m_vCardUpdates |= (1UL << iPageId);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_LOAD:
			wchar_t szFilter[512], szFileName[MAX_PATH];
			Bitmap_GetFilter(szFilter, _countof(szFilter));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = szFilter;
			ofn.lpstrCustomFilter = nullptr;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT;
			szFileName[0] = '\0';
			if (GetOpenFileName(&ofn)) {
				struct _stat st;
				HBITMAP hNewBitmap;

				dat->ppro->debugLogW(L"File selected is %s", szFileName);
				if (_wstat(szFileName, &st) < 0 || st.st_size > 40 * 1024) {
					MessageBox(hwndDlg, TranslateT("Only JPG, GIF, and BMP image files smaller than 40 KB are supported."), TranslateT("Jabber vCard"), MB_OK | MB_SETFOREGROUND);
					break;
				}
				if (GetTempPath(_countof(szTempPath), szTempPath) <= 0)
					mir_wstrcpy(szTempPath, L".\\");
				
				if (GetTempFileName(szTempPath, L"jab", 0, szTempFileName) > 0) {
					dat->ppro->debugLogW(L"Temp file = %s", szTempFileName);
					if (CopyFile(szFileName, szTempFileName, FALSE) == TRUE) {
						if ((hNewBitmap = Bitmap_Load(szTempFileName)) != nullptr) {
							if (dat->hBitmap) {
								DeleteObject(dat->hBitmap);
								DeleteFile(dat->ppro->m_szPhotoFileName);
							}

							dat->hBitmap = hNewBitmap;
							mir_wstrcpy(dat->ppro->m_szPhotoFileName, szTempFileName);
							dat->ppro->m_bPhotoChanged = true;
							EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
							InvalidateRect(hwndDlg, nullptr, TRUE);
							UpdateWindow(hwndDlg);
							dat->ppro->m_vCardUpdates |= (1UL << iPageId);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
						else DeleteFile(szTempFileName);
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
			InvalidateRect(hwndCanvas, nullptr, TRUE);
			UpdateWindow(hwndCanvas);
			if (ptSize.x <= rect.right && ptSize.y <= rect.bottom) {
				pt.x = (rect.right - ptSize.x) / 2;
				pt.y = (rect.bottom - ptSize.y) / 2;
				ptFitSize = ptSize;
			}
			else {
				if (((float)(ptSize.x - rect.right)) / ptSize.x > ((float)(ptSize.y - rect.bottom)) / ptSize.y) {
					ptFitSize.x = rect.right;
					ptFitSize.y = (ptSize.y * rect.right) / ptSize.x;
					pt.x = 0;
					pt.y = (rect.bottom - ptFitSize.y) / 2;
				}
				else {
					ptFitSize.x = (ptSize.x * rect.bottom) / ptSize.y;
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
		Button_FreeIcon_IcoLib(hwndDlg, IDC_LOAD);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_DELETE);
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
			mir_snprintf(idstr, "e-mail%d", dat->id);
			ptrA email(dat->ppro->getStringA(idstr));
			if (email != nullptr)
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, email);

			mir_snprintf(idstr, "e-mailFlag%d", (int)lParam);
			uint16_t nFlag = dat->ppro->getWord(idstr, 0);
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
					mir_snprintf(idstr, "e-mail%d", dat->id);
					if (ptrA(dat->ppro->getStringA(idstr)) == nullptr)
						break;
				}

			wchar_t text[128];
			GetDlgItemText(hwndDlg, IDC_EMAIL, text, _countof(text));
			mir_snprintf(idstr, "e-mail%d", dat->id);
			dat->ppro->setWString(idstr, text);
			{
				uint16_t nFlag = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_HOME)) nFlag |= JABBER_VCEMAIL_HOME;
				if (IsDlgButtonChecked(hwndDlg, IDC_WORK)) nFlag |= JABBER_VCEMAIL_WORK;
				if (IsDlgButtonChecked(hwndDlg, IDC_INTERNET)) nFlag |= JABBER_VCEMAIL_INTERNET;
				if (IsDlgButtonChecked(hwndDlg, IDC_X400)) nFlag |= JABBER_VCEMAIL_X400;
				mir_snprintf(idstr, "e-mailFlag%d", dat->id);
				dat->ppro->setWord(idstr, nFlag);
			}
			__fallthrough;

		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			break;
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK EditPhoneDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EditDlgParam *dat = (EditDlgParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		dat = (EditDlgParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		TranslateDialogDefault(hwndDlg);
		if (dat->id >= 0) {
			SetWindowText(hwndDlg, TranslateT("Jabber vCard: Edit Phone Number"));

			char idstr[33];
			mir_snprintf(idstr, "Phone%d", dat->id);
			ptrA phone(dat->ppro->getStringA(idstr));
			if (phone != nullptr)
				SetDlgItemTextA(hwndDlg, IDC_PHONE, phone);

			mir_snprintf(idstr, "PhoneFlag%d", dat->id);
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
			char idstr[33];

			if (dat->id < 0)
				for (dat->id = 0;; dat->id++) {
					mir_snprintf(idstr, "Phone%d", dat->id);
					if (ptrA(dat->ppro->getStringA(idstr)) == nullptr)
						break;
				}

			char text[128];
			GetDlgItemTextA(hwndDlg, IDC_PHONE, text, _countof(text));
			mir_snprintf(idstr, "Phone%d", dat->id);
			dat->ppro->setString(idstr, text);
			{
				uint16_t nFlag = 0;
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
				mir_snprintf(idstr, "PhoneFlag%d", dat->id);
				dat->ppro->setWord(idstr, nFlag);
			}
			__fallthrough;

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
			wchar_t number[20];

			//e-mails
			ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_EMAILS));

			LVITEM lvi;
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = 0;
			for (i=0;;i++) {
				mir_snprintf(idstr, "e-mail%d", i);
				ptrW email(ppro->getWStringA(idstr));
				if (email == nullptr) break;

				mir_snwprintf(number, L"%d", i + 1);
				lvi.pszText = number;
				lvi.lParam = (LPARAM)i;
				ListView_InsertItem(GetDlgItem(hwndDlg, IDC_EMAILS), &lvi);
				ListView_SetItemText(GetDlgItem(hwndDlg, IDC_EMAILS), lvi.iItem, 1, email);
				lvi.iItem++;
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = -1;
			ListView_InsertItem(GetDlgItem(hwndDlg, IDC_EMAILS), &lvi);

			//phones
			ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_PHONES));
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = 0;
			for (i=0;;i++) {
				mir_snprintf(idstr, "Phone%d", i);
				ptrW phone(ppro->getWStringA(idstr));
				if (phone == nullptr) break;

				mir_snwprintf(number, L"%d", i + 1);
				lvi.pszText = number;
				lvi.lParam = (LPARAM)i;
				ListView_InsertItem(GetDlgItem(hwndDlg, IDC_PHONES), &lvi);
				ListView_SetItemText(GetDlgItem(hwndDlg, IDC_PHONES), lvi.iItem, 1, phone);
				lvi.iItem++;
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = -1;
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
						if (nm->nmcd.lItemlParam == -1 && nm->iSubItem == 3)
							hIcon = g_plugin.getIcon(IDI_ADDCONTACT);
						else if (nm->iSubItem == 2 && nm->nmcd.lItemlParam != -1)
							hIcon = g_plugin.getIcon(IDI_EDIT);
						else if (nm->iSubItem == 3 && nm->nmcd.lItemlParam != -1)
							hIcon = g_plugin.getIcon(IDI_DELETE);
						else break;
						DrawIconEx(nm->nmcd.hdc, (rc.left + rc.right - GetSystemMetrics(SM_CXSMICON)) / 2, (rc.top + rc.bottom - GetSystemMetrics(SM_CYSMICON)) / 2, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, nullptr, DI_NORMAL);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
						return TRUE;
					}
				}
				break;

			case NM_CLICK:
				NMLISTVIEW *nm = (NMLISTVIEW *)lParam;
				if (nm->iSubItem < 2)
					break;

				const char *szIdTemplate = (nm->hdr.idFrom == IDC_PHONES) ? "Phone%d" : "e-mail%d";
				const char *szFlagTemplate = (nm->hdr.idFrom == IDC_PHONES) ? "PhoneFlag%d" : "e-mailFlag%d";

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
				if (lvi.lParam == -1) {
					if (hti.iSubItem == 3) {
						//add
						EditDlgParam param = { -1, ppro };
						int res;
						if (nm->hdr.idFrom == IDC_PHONES)
							res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDPHONE), hwndDlg, EditPhoneDlgProc, (LPARAM)&param);
						else
							res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDEMAIL), hwndDlg, EditEmailDlgProc, (LPARAM)&param);
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
							mir_snprintf(idstr, szIdTemplate, i + 1);
							ptrA fieldVal(ppro->getStringA(idstr));
							if (fieldVal == nullptr) break;
							mir_snprintf(idstr, szIdTemplate, i);
							ppro->setString(idstr, fieldVal);

							mir_snprintf(idstr, szFlagTemplate, i + 1);
							uint16_t nFlag = ppro->getWord(idstr, 0);
							mir_snprintf(idstr, szFlagTemplate, i);
							ppro->setWord(idstr, nFlag);
						}
						mir_snprintf(idstr, szIdTemplate, i);
						ppro->delSetting(idstr);
						mir_snprintf(idstr, szFlagTemplate, i);
						ppro->delSetting(idstr);
						SendMessage(hwndDlg, M_REMAKELISTS, 0, 0);
						ppro->m_vCardUpdates |= (1UL << iPageId);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					else if (hti.iSubItem == 2) {
						EditDlgParam param = { (int)lvi.lParam, ppro };
						int res;
						if (nm->hdr.idFrom == IDC_PHONES)
							res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDPHONE), hwndDlg, EditPhoneDlgProc, (LPARAM)&param);
						else
							res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDEMAIL), hwndDlg, EditEmailDlgProc, (LPARAM)&param);
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
	wchar_t text[2048];

	// Page 0: Personal
	switch (iPage) {
	case 0:
		GetDlgItemText(hwndPage, IDC_FULLNAME, text, _countof(text));
		setWString("FullName", text);
		GetDlgItemText(hwndPage, IDC_NICKNAME, text, _countof(text));
		setWString("Nick", text);
		GetDlgItemText(hwndPage, IDC_FIRSTNAME, text, _countof(text));
		setWString("FirstName", text);
		GetDlgItemText(hwndPage, IDC_MIDDLE, text, _countof(text));
		setWString("MiddleName", text);
		GetDlgItemText(hwndPage, IDC_LASTNAME, text, _countof(text));
		setWString("LastName", text);
		GetDlgItemText(hwndPage, IDC_BIRTH, text, _countof(text));
		setWString("BirthDate", text);
		switch (SendDlgItemMessage(hwndPage, IDC_GENDER, CB_GETCURSEL, 0, 0)) {
		case 0:	setString("GenderString", "Male");   break;
		case 1:	setString("GenderString", "Female"); break;
		default: setString("GenderString", "");       break;
		}
		GetDlgItemText(hwndPage, IDC_OCCUPATION, text, _countof(text));
		setWString("Role", text);
		GetDlgItemText(hwndPage, IDC_HOMEPAGE, text, _countof(text));
		setWString("Homepage", text);
		break;
		
	case 1: // Page 1: Home
		GetDlgItemText(hwndPage, IDC_ADDRESS1, text, _countof(text));
		setWString("Street", text);
		GetDlgItemText(hwndPage, IDC_ADDRESS2, text, _countof(text));
		setWString("Street2", text);
		GetDlgItemText(hwndPage, IDC_CITY, text, _countof(text));
		setWString("City", text);
		GetDlgItemText(hwndPage, IDC_STATE, text, _countof(text));
		setWString("State", text);
		GetDlgItemText(hwndPage, IDC_ZIP, text, _countof(text));
		setWString("ZIP", text);
		{
			int i = SendDlgItemMessage(hwndPage, IDC_COUNTRY, CB_GETCURSEL, 0, 0);
			wchar_t *country = mir_a2u((i) ? g_countries[i + 2].szName : g_countries[1].szName);
			setWString("Country", country);
			mir_free(country);
		}
		break;
		
	case 2: // Page 2: Work
		GetDlgItemText(hwndPage, IDC_COMPANY, text, _countof(text));
		setWString("Company", text);
		GetDlgItemText(hwndPage, IDC_DEPARTMENT, text, _countof(text));
		setWString("CompanyDepartment", text);
		GetDlgItemText(hwndPage, IDC_TITLE, text, _countof(text));
		setWString("CompanyPosition", text);
		GetDlgItemText(hwndPage, IDC_ADDRESS1, text, _countof(text));
		setWString("CompanyStreet", text);
		GetDlgItemText(hwndPage, IDC_ADDRESS2, text, _countof(text));
		setWString("CompanyStreet2", text);
		GetDlgItemText(hwndPage, IDC_CITY, text, _countof(text));
		setWString("CompanyCity", text);
		GetDlgItemText(hwndPage, IDC_STATE, text, _countof(text));
		setWString("CompanyState", text);
		GetDlgItemText(hwndPage, IDC_ZIP, text, _countof(text));
		setWString("CompanyZIP", text);
		{
			int i = SendDlgItemMessage(hwndPage, IDC_COUNTRY, CB_GETCURSEL, 0, 0);
			wchar_t *country = mir_a2u((i) ? g_countries[i + 2].szName : g_countries[1].szName);
			setWString("CompanyCountry", country);
			mir_free(country);
		}
		break;

		// Page 3: Photo
		// not needed to be saved into db

	case 4: // Page 4: Note
		GetDlgItemText(hwndPage, IDC_DESC, text, _countof(text));
		setWString("About", text);
		break;
	}
}

void CJabberProto::AppendVcardFromDB(TiXmlElement *n, char *tag, char *key)
{
	if (n == nullptr || tag == nullptr || key == nullptr)
		return;

	ptrA tszValue(getUStringA(key));
	n << XCHILD(tag, tszValue);
}

void CJabberProto::SetServerVcard(BOOL bPhotoChanged, wchar_t *szPhotoFileName)
{
	if (!m_bJabberOnline) return;

	int  i;
	char idstr[33];

	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultSetVcard, JABBER_IQ_TYPE_SET));
	TiXmlElement *v = iq << XCHILDNS("vCard", JABBER_FEAT_VCARD_TEMP);

	AppendVcardFromDB(v, "FN", "FullName");

	TiXmlElement *n = v << XCHILD("N");
	AppendVcardFromDB(n, "GIVEN", "FirstName");
	AppendVcardFromDB(n, "MIDDLE", "MiddleName");
	AppendVcardFromDB(n, "FAMILY", "LastName");

	AppendVcardFromDB(v, "NICKNAME", "Nick");
	AppendVcardFromDB(v, "BDAY", "BirthDate");
	AppendVcardFromDB(v, "GENDER", "GenderString");

	for (i = 0;; i++) {
		mir_snprintf(idstr, "e-mail%d", i);
		ptrA email(getUStringA(idstr));
		if (email == nullptr)
			break;

		TiXmlElement *e = v << XCHILD("EMAIL", email);
		AppendVcardFromDB(e, "USERID", idstr);

		mir_snprintf(idstr, "e-mailFlag%d", i);
		uint16_t nFlag = getWord(idstr, 0);
		if (nFlag & JABBER_VCEMAIL_HOME)     e << XCHILD("HOME");
		if (nFlag & JABBER_VCEMAIL_WORK)     e << XCHILD("WORK");
		if (nFlag & JABBER_VCEMAIL_INTERNET) e << XCHILD("INTERNET");
		if (nFlag & JABBER_VCEMAIL_X400)     e << XCHILD("X400");
	}

	n = v << XCHILD("ADR");
	n << XCHILD("HOME");
	AppendVcardFromDB(n, "STREET", "Street");
	AppendVcardFromDB(n, "EXTADR", "Street2");
	AppendVcardFromDB(n, "EXTADD", "Street2");	// for compatibility with client using old vcard format
	AppendVcardFromDB(n, "LOCALITY", "City");
	AppendVcardFromDB(n, "REGION", "State");
	AppendVcardFromDB(n, "PCODE", "ZIP");
	AppendVcardFromDB(n, "CTRY", "Country");
	AppendVcardFromDB(n, "COUNTRY", "Country");	// for compatibility with client using old vcard format

	n = v << XCHILD("ADR");
	n << XCHILD("WORK");
	AppendVcardFromDB(n, "STREET", "CompanyStreet");
	AppendVcardFromDB(n, "EXTADR", "CompanyStreet2");
	AppendVcardFromDB(n, "EXTADD", "CompanyStreet2");	// for compatibility with client using old vcard format
	AppendVcardFromDB(n, "LOCALITY", "CompanyCity");
	AppendVcardFromDB(n, "REGION", "CompanyState");
	AppendVcardFromDB(n, "PCODE", "CompanyZIP");
	AppendVcardFromDB(n, "CTRY", "CompanyCountry");
	AppendVcardFromDB(n, "COUNTRY", "CompanyCountry");	// for compatibility with client using old vcard format

	n = v << XCHILD("ORG");
	AppendVcardFromDB(n, "ORGNAME", "Company");
	AppendVcardFromDB(n, "ORGUNIT", "CompanyDepartment");

	AppendVcardFromDB(v, "TITLE", "CompanyPosition");
	AppendVcardFromDB(v, "ROLE", "Role");
	AppendVcardFromDB(v, "URL", "Homepage");
	AppendVcardFromDB(v, "DESC", "About");

	for (i = 0;; i++) {
		mir_snprintf(idstr, "Phone%d", i);
		ptrW phone(getWStringA(idstr));
		if (phone == nullptr)
			break;

		n = v << XCHILD("TEL");
		AppendVcardFromDB(n, "NUMBER", idstr);

		mir_snprintf(idstr, "PhoneFlag%d", i);
		uint16_t nFlag = getWord(idstr, 0);
		if (nFlag & JABBER_VCTEL_HOME)  n << XCHILD("HOME");
		if (nFlag & JABBER_VCTEL_WORK)  n << XCHILD("WORK");
		if (nFlag & JABBER_VCTEL_VOICE) n << XCHILD("VOICE");
		if (nFlag & JABBER_VCTEL_FAX)   n << XCHILD("FAX");
		if (nFlag & JABBER_VCTEL_PAGER) n << XCHILD("PAGER");
		if (nFlag & JABBER_VCTEL_MSG)   n << XCHILD("MSG");
		if (nFlag & JABBER_VCTEL_CELL)  n << XCHILD("CELL");
		if (nFlag & JABBER_VCTEL_VIDEO) n << XCHILD("VIDEO");
		if (nFlag & JABBER_VCTEL_BBS)   n << XCHILD("BBS");
		if (nFlag & JABBER_VCTEL_MODEM) n << XCHILD("MODEM");
		if (nFlag & JABBER_VCTEL_ISDN)  n << XCHILD("ISDN");
		if (nFlag & JABBER_VCTEL_PCS)   n << XCHILD("PCS");
	}

	wchar_t szAvatarName[MAX_PATH], *szFileName;
	GetAvatarFileName(0, szAvatarName, _countof(szAvatarName));
	if (bPhotoChanged)
		szFileName = szPhotoFileName;
	else
		szFileName = szAvatarName;

	// Set photo element, also update the global jabberVcardPhotoFileName to reflect the update
	debugLogW(L"Before update, file name = %s", szFileName);
	if (szFileName == nullptr || szFileName[0] == 0) {
		v << XCHILD("PHOTO");
		DeleteFile(szAvatarName);
		delSetting("AvatarHash");
	}
	else {
		debugLogW(L"Saving picture from %s", szFileName);

		struct _stat st;
		if (_wstat(szFileName, &st) >= 0) {
			// Note the FILE_SHARE_READ attribute so that the CopyFile can succeed
			HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile != INVALID_HANDLE_VALUE) {
				ptrA buffer((char*)mir_alloc(st.st_size));
				if (buffer != nullptr) {
					DWORD nRead;
					if (ReadFile(hFile, buffer, st.st_size, &nRead, nullptr)) {
						ptrA str(mir_base64_encode(buffer, nRead));
						const char *szFileType = ProtoGetAvatarMimeType(ProtoGetBufferFormat(buffer));
						if (str != nullptr && szFileType != nullptr) {
							n = v << XCHILD("PHOTO");
							n << XCHILD("TYPE", szFileType);
							n << XCHILD("BINVAL", str);

							// NEED TO UPDATE OUR AVATAR HASH:
							uint8_t digest[MIR_SHA1_HASH_SIZE];
							mir_sha1_ctx sha1ctx;
							mir_sha1_init(&sha1ctx);
							mir_sha1_append(&sha1ctx, (uint8_t*)(LPSTR)buffer, nRead);
							mir_sha1_finish(&sha1ctx, digest);

							char buf[MIR_SHA1_HASH_SIZE * 2 + 1];
							bin2hex(digest, sizeof(digest), buf);

							if (bPhotoChanged) {
								DeleteFile(szAvatarName);

								GetAvatarFileName(0, szAvatarName, _countof(szAvatarName));
								CopyFile(szFileName, szAvatarName, FALSE);
							}

							setString("AvatarHash", buf);
						}
					}
				}
				CloseHandle(hFile);
			}
		}
	}

	XmlNodeHash hasher;
	v->Accept(&hasher);
	setString("VCardHash", hasher.getResult());

	m_ThreadInfo->send(iq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnUserInfoInit_VCard(WPARAM wParam, LPARAM)
{
	m_vCardUpdates = 0;
	m_bPhotoChanged = false;
	m_szPhotoFileName[0] = 0;

	OPTIONSDIALOGPAGE odp = {};
	odp.dwInitParam = (LPARAM)this;
	odp.flags = ODPF_UNICODE | ODPF_USERINFOTAB | ODPF_DONTTRANSLATE;
	odp.szTitle.w = m_tszUserName;

	odp.pfnDlgProc = PersonalDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_PERSONAL);
	odp.szTab.w = LPGENW("General");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = ContactDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_CONTACT);
	odp.szTab.w = LPGENW("Contacts");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = HomeDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_HOME);
	odp.szTab.w = LPGENW("Home");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = WorkDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_WORK);
	odp.szTab.w = LPGENW("Work");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = PhotoDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_PHOTO);
	odp.szTab.w = LPGENW("Photo");
	g_plugin.addUserInfo(wParam, &odp);

	odp.pfnDlgProc = NoteDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_VCARD_NOTE);
	odp.szTab.w = LPGENW("Note");
	g_plugin.addUserInfo(wParam, &odp);

	SendGetVcard(0);
}
