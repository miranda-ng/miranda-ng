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
	if (!m_bJabberOnline)
		return 0;

	CJabberIqInfo *pInfo;

	if (hContact == 0) {
		setDword("LastGetVcard", time(0));
		pInfo = AddIQ(&CJabberProto::OnIqResultGetVcard, JABBER_IQ_TYPE_GET, m_szJabberJID);
	}
	else {
		ptrA jid(ContactToJID(hContact));
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

class JabberVcardBaseDlg : public CUserInfoPageDlg
{
	int iPageId;

protected:
	CJabberProto *ppro;

public:
	JabberVcardBaseDlg(CJabberProto *_ppro, int dlgId, int pageId) :
		CUserInfoPageDlg(g_plugin, dlgId),
		ppro(_ppro),
		iPageId(pageId)
	{}

	bool OnInitDialog() override
	{
		ppro->WindowSubscribe(m_hwnd);
		return true;
	}

	bool OnApply() override
	{
		ppro->SaveVcardToDB(m_hwnd, iPageId);

		ppro->m_vCardUpdates &= ~(1UL << iPageId);
		if (!ppro->m_vCardUpdates)
			ppro->SetServerVcard(ppro->m_bPhotoChanged, ppro->m_szPhotoFileName);
		return true;
	}

	void OnDestroy() override
	{
		ppro->WindowUnsubscribe(m_hwnd);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

struct JabberVcardPersonalDlg : public JabberVcardBaseDlg
{
	JabberVcardPersonalDlg(CJabberProto *_ppro) :
		JabberVcardBaseDlg(_ppro, IDD_VCARD_PERSONAL, 0)
	{}

	bool OnInitDialog() override
	{
		JabberVcardBaseDlg::OnInitDialog();
		SendDlgItemMessage(m_hwnd, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Male"));
		SendDlgItemMessage(m_hwnd, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Female"));
		return true;
	}

	bool OnRefresh() override
	{
		SetDialogField(ppro, m_hwnd, IDC_FULLNAME, "FullName");
		SetDialogField(ppro, m_hwnd, IDC_NICKNAME, "Nick");
		SetDialogField(ppro, m_hwnd, IDC_FIRSTNAME, "FirstName");
		SetDialogField(ppro, m_hwnd, IDC_MIDDLE, "MiddleName");
		SetDialogField(ppro, m_hwnd, IDC_LASTNAME, "LastName");
		SetDialogField(ppro, m_hwnd, IDC_BIRTH, "BirthDate");
		SetDialogField(ppro, m_hwnd, IDC_GENDER, "GenderString", true);
		SetDialogField(ppro, m_hwnd, IDC_OCCUPATION, "Role");
		SetDialogField(ppro, m_hwnd, IDC_HOMEPAGE, "Homepage");
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Home vcard dialog

struct JabberVcardHomeDlg : public JabberVcardBaseDlg
{
	JabberVcardHomeDlg(CJabberProto *_ppro) :
		JabberVcardBaseDlg(_ppro, IDD_VCARD_HOME, 1)
	{
	}

	bool OnInitDialog() override
	{
		JabberVcardBaseDlg::OnInitDialog();
		for (int i = 0; i < g_cbCountries; i++) {
			if (g_countries[i].id != 0xFFFF && g_countries[i].id != 0) {
				wchar_t *country = mir_a2u(g_countries[i].szName);
				SendDlgItemMessage(m_hwnd, IDC_COUNTRY, CB_ADDSTRING, 0, (LPARAM)TranslateW(country));
				mir_free(country);
			}
		}
		return true;
	}

	bool OnRefresh() override
	{
		SetDialogField(ppro, m_hwnd, IDC_ADDRESS1, "Street");
		SetDialogField(ppro, m_hwnd, IDC_ADDRESS2, "Street2");
		SetDialogField(ppro, m_hwnd, IDC_CITY, "City");
		SetDialogField(ppro, m_hwnd, IDC_STATE, "State");
		SetDialogField(ppro, m_hwnd, IDC_ZIP, "ZIP");
		SetDialogField(ppro, m_hwnd, IDC_COUNTRY, "Country", true);
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Work vcard dialog

struct JabberVcardWorkDlg : public JabberVcardBaseDlg
{
	JabberVcardWorkDlg(CJabberProto *_ppro) :
		JabberVcardBaseDlg(_ppro, IDD_VCARD_WORK, 2)
	{
	}

	bool OnInitDialog() override
	{
		JabberVcardBaseDlg::OnInitDialog();

		for (int i = 0; i < g_cbCountries; i++) {
			if (g_countries[i].id != 0xFFFF && g_countries[i].id != 0) {
				wchar_t *country = mir_a2u(g_countries[i].szName);
				SendDlgItemMessage(m_hwnd, IDC_COUNTRY, CB_ADDSTRING, 0, (LPARAM)TranslateW(country));
				mir_free(country);
			}
		}
		return true;
	}

	bool OnRefresh() override
	{
		SetDialogField(ppro, m_hwnd, IDC_COMPANY, "Company");
		SetDialogField(ppro, m_hwnd, IDC_DEPARTMENT, "CompanyDepartment");
		SetDialogField(ppro, m_hwnd, IDC_TITLE, "CompanyPosition");
		SetDialogField(ppro, m_hwnd, IDC_ADDRESS1, "CompanyStreet");
		SetDialogField(ppro, m_hwnd, IDC_ADDRESS2, "CompanyStreet2");
		SetDialogField(ppro, m_hwnd, IDC_CITY, "CompanyCity");
		SetDialogField(ppro, m_hwnd, IDC_STATE, "CompanyState");
		SetDialogField(ppro, m_hwnd, IDC_ZIP, "CompanyZIP");
		SetDialogField(ppro, m_hwnd, IDC_COUNTRY, "CompanyCountry", true);
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Photo vcard dialog

class JabberVcardPhotoDlg : public JabberVcardBaseDlg
{
	HBITMAP hBitmap;

	UI_MESSAGE_MAP(JabberVcardPhotoDlg, JabberVcardBaseDlg);
		UI_MESSAGE(WM_PAINT, OnPaint);
	UI_MESSAGE_MAP_END();

public:
	JabberVcardPhotoDlg(CJabberProto *_ppro) :
		JabberVcardBaseDlg(_ppro, IDD_VCARD_PHOTO, 3)
	{
	}

	bool OnInitDialog() override
	{
		JabberVcardBaseDlg::OnInitDialog();

		Button_SetIcon_IcoLib(m_hwnd, IDC_LOAD, g_plugin.getIconHandle(IDI_OPEN));
		Button_SetIcon_IcoLib(m_hwnd, IDC_DELETE, g_plugin.getIconHandle(IDI_DELETE));
		ShowWindow(GetDlgItem(m_hwnd, IDC_SAVE), SW_HIDE);

		ppro->m_bPhotoChanged = false;
		return true;
	}

	void OnDestroy() override
	{
		JabberVcardBaseDlg::OnDestroy();

		Button_FreeIcon_IcoLib(m_hwnd, IDC_LOAD);
		Button_FreeIcon_IcoLib(m_hwnd, IDC_DELETE);
		if (hBitmap) {
			ppro->debugLogA("Delete bitmap");
			DeleteObject(hBitmap);
			DeleteFile(ppro->m_szPhotoFileName);
		}
	}

	bool OnRefresh() override
	{
		if (hBitmap) {
			DeleteObject(hBitmap);
			hBitmap = nullptr;
			DeleteFile(ppro->m_szPhotoFileName);
			ppro->m_szPhotoFileName[0] = '\0';
		}

		wchar_t szAvatarFileName[MAX_PATH], szTempPath[MAX_PATH], szTempFileName[MAX_PATH];
		EnableWindow(GetDlgItem(m_hwnd, IDC_DELETE), FALSE);
		ppro->GetAvatarFileName(0, szAvatarFileName, _countof(szAvatarFileName));
		if (_waccess(szAvatarFileName, 0) == 0) {
			if (GetTempPath(_countof(szTempPath), szTempPath) <= 0)
				mir_wstrcpy(szTempPath, L".\\");
			if (GetTempFileName(szTempPath, L"jab", 0, szTempFileName) > 0) {
				ppro->debugLogW(L"Temp file = %s", szTempFileName);
				if (CopyFile(szAvatarFileName, szTempFileName, FALSE) == TRUE) {
					if ((hBitmap = Bitmap_Load(szTempFileName)) != nullptr) {
						FreeImage_Premultiply(hBitmap);
						mir_wstrcpy(ppro->m_szPhotoFileName, szTempFileName);
						EnableWindow(GetDlgItem(m_hwnd, IDC_DELETE), TRUE);
					}
					else DeleteFile(szTempFileName);
				}
				else DeleteFile(szTempFileName);
			}
		}

		ppro->m_bPhotoChanged = false;
		InvalidateRect(m_hwnd, nullptr, TRUE);
		UpdateWindow(m_hwnd);
		return false;
	}

	void onClick_Delete(CCtrlButton *)
	{
		if (hBitmap == nullptr)
			return;

		DeleteObject(hBitmap);
		hBitmap = nullptr;
		DeleteFile(ppro->m_szPhotoFileName);
		ppro->m_szPhotoFileName[0] = '\0';
		ppro->m_bPhotoChanged = true;
		EnableWindow(GetDlgItem(m_hwnd, IDC_DELETE), FALSE);
		InvalidateRect(m_hwnd, nullptr, TRUE);
		UpdateWindow(m_hwnd);
		NotifyChange();
	}

	void onClick_Load(CCtrlButton *)
	{
		wchar_t szFilter[512], szFileName[MAX_PATH];
		Bitmap_GetFilter(szFilter, _countof(szFilter));

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hwnd;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrCustomFilter = nullptr;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT;
		szFileName[0] = '\0';
		if (GetOpenFileName(&ofn)) {
			struct _stat st;
			HBITMAP hNewBitmap;

			ppro->debugLogW(L"File selected is %s", szFileName);
			if (_wstat(szFileName, &st) < 0 || st.st_size > 40 * 1024) {
				MessageBox(m_hwnd, TranslateT("Only JPG, GIF, and BMP image files smaller than 40 KB are supported."), TranslateT("Jabber vCard"), MB_OK | MB_SETFOREGROUND);
				return;
			}

			wchar_t szTempFileName[MAX_PATH], szTempPath[MAX_PATH];
			if (GetTempPath(_countof(szTempPath), szTempPath) <= 0)
				mir_wstrcpy(szTempPath, L".\\");

			if (GetTempFileName(szTempPath, L"jab", 0, szTempFileName) > 0) {
				ppro->debugLogW(L"Temp file = %s", szTempFileName);
				if (CopyFile(szFileName, szTempFileName, FALSE) == TRUE) {
					if ((hNewBitmap = Bitmap_Load(szTempFileName)) != nullptr) {
						if (hBitmap) {
							DeleteObject(hBitmap);
							DeleteFile(ppro->m_szPhotoFileName);
						}

						hBitmap = hNewBitmap;
						mir_wstrcpy(ppro->m_szPhotoFileName, szTempFileName);
						ppro->m_bPhotoChanged = true;
						EnableWindow(GetDlgItem(m_hwnd, IDC_DELETE), TRUE);
						InvalidateRect(m_hwnd, nullptr, TRUE);
						UpdateWindow(m_hwnd);
						NotifyChange();
					}
					else DeleteFile(szTempFileName);
				}
				else DeleteFile(szTempFileName);
			}
		}
	}

	INT_PTR OnPaint(UINT, WPARAM, LPARAM)
	{
		if (hBitmap == nullptr)
			return FALSE;

		BITMAP bm;
		POINT ptSize, ptOrg, pt, ptFitSize;
		RECT rect;

		HWND hwndCanvas = GetDlgItem(m_hwnd, IDC_CANVAS);
		HDC hdcCanvas = GetDC(hwndCanvas);
		HDC hdcMem = CreateCompatibleDC(hdcCanvas);
		SelectObject(hdcMem, hBitmap);
		SetMapMode(hdcMem, GetMapMode(hdcCanvas));
		GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
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
		return FALSE;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Note vcard dialog

struct JabberVcardNoteDlg : public JabberVcardBaseDlg
{
	JabberVcardNoteDlg(CJabberProto *_ppro) :
		JabberVcardBaseDlg(_ppro, IDD_VCARD_WORK, 4)
	{}

	bool OnRefresh() override
	{
		SetDialogField(ppro, m_hwnd, IDC_DESC, "About");
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Email vcard dialog

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

class JabberVcardContactDlg : public JabberVcardBaseDlg
{
	CCtrlListView phones, emails;

public:
	JabberVcardContactDlg(CJabberProto *_ppro) :
		JabberVcardBaseDlg(_ppro, IDD_VCARD_CONTACT, 5),
		phones(this, IDC_PHONES),
		emails(this, IDC_EMAILS)
	{
		phones.OnClick = emails.OnClick = Callback(this, &JabberVcardContactDlg::onClick_Lists);
		phones.OnCustomDraw = emails.OnCustomDraw = Callback(this, &JabberVcardContactDlg::onDraw_Lists);
	}

	bool OnInitDialog() override
	{
		JabberVcardBaseDlg::OnInitDialog();

		RECT rc;
		GetClientRect(emails.GetHwnd(), &rc);
		rc.right -= GetSystemMetrics(SM_CXVSCROLL);

		LVCOLUMN lvc;
		lvc.mask = LVCF_WIDTH;
		lvc.cx = 30;
		emails.InsertColumn(0, &lvc);
		phones.InsertColumn(0, &lvc);
		lvc.cx = rc.right - 30 - 40;
		emails.InsertColumn(1, &lvc);
		phones.InsertColumn(1, &lvc);
		lvc.cx = 20;
		emails.InsertColumn(2, &lvc);
		emails.InsertColumn(3, &lvc);
		phones.InsertColumn(2, &lvc);
		phones.InsertColumn(3, &lvc);
		return true;
	}

	bool OnRefresh() override
	{
		int i;
		char idstr[33];
		wchar_t number[20];

		//e-mails
		emails.DeleteAllItems();

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
			emails.InsertItem(&lvi);
			emails.SetItemText(lvi.iItem, 1, email);
			lvi.iItem++;
		}
		lvi.mask = LVIF_PARAM;
		lvi.lParam = -1;
		emails.InsertItem(&lvi);

		//phones
		phones.DeleteAllItems();

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
			phones.InsertItem(&lvi);
			phones.SetItemText(lvi.iItem, 1, phone);
			lvi.iItem++;
		}
		lvi.mask = LVIF_PARAM;
		lvi.lParam = -1;
		phones.InsertItem(&lvi);
		return false;
	}

	void onDraw_Lists(CCtrlListView::TEventInfo *ev)
	{
		NMLVCUSTOMDRAW *nm = ev->nmcd;

		switch (nm->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
		case CDDS_ITEMPREPAINT:
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW);
			return;

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
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
		}
	}

	void onClick_Lists(CCtrlListView::TEventInfo *ev)
	{
		NMLISTVIEW *nm = ev->nmlv;
		if (nm->iSubItem < 2)
			return;

		const char *szIdTemplate = (nm->hdr.idFrom == IDC_PHONES) ? "Phone%d" : "e-mail%d";
		const char *szFlagTemplate = (nm->hdr.idFrom == IDC_PHONES) ? "PhoneFlag%d" : "e-mailFlag%d";

		LVHITTESTINFO hti;
		hti.pt.x = (short)LOWORD(GetMessagePos());
		hti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(nm->hdr.hwndFrom, &hti.pt);
		if (ListView_SubItemHitTest(nm->hdr.hwndFrom, &hti) == -1)
			return;

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
					res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDPHONE), m_hwnd, EditPhoneDlgProc, (LPARAM)&param);
				else
					res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDEMAIL), m_hwnd, EditEmailDlgProc, (LPARAM)&param);
				if (res != IDOK)
					return;
				OnRefresh();
				NotifyChange();
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
				OnRefresh();
				NotifyChange();
			}
			else if (hti.iSubItem == 2) {
				EditDlgParam param = { (int)lvi.lParam, ppro };
				int res;
				if (nm->hdr.idFrom == IDC_PHONES)
					res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDPHONE), m_hwnd, EditPhoneDlgProc, (LPARAM)&param);
				else
					res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_VCARD_ADDEMAIL), m_hwnd, EditEmailDlgProc, (LPARAM)&param);
				if (res != IDOK)
					return;
				OnRefresh();
				NotifyChange();
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

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

void CJabberProto::SetServerVcard(bool bPhotoChanged, wchar_t *szPhotoFileName)
{
	if (!m_bJabberOnline) return;

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

	for (int i = 0;; i++) {
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

	for (int i = 0;; i++) {
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

	AppendPhotoToVcard(v, bPhotoChanged, szPhotoFileName);

	XmlNodeHash hasher;
	v->Accept(&hasher);
	setString("VCardHash", hasher.getResult());

	m_ThreadInfo->send(iq);
}

void CJabberProto::AppendPhotoToVcard(TiXmlElement *v, bool bPhotoChanged, const wchar_t *szPhotoFileName, MCONTACT hContact)
{
	wchar_t szAvatarName[MAX_PATH];
	GetAvatarFileName(hContact, szAvatarName, _countof(szAvatarName));
	
	const wchar_t *szFileName = (bPhotoChanged) ? szPhotoFileName : szAvatarName;

	// Set photo element, also update the global jabberVcardPhotoFileName to reflect the update
	debugLogW(L"Before update, file name = %s", szFileName);
	if (szFileName == nullptr || szFileName[0] == 0) {
		v << XCHILD("PHOTO");
		DeleteFileW(szAvatarName);
		delSetting(hContact, "AvatarHash");
		return;
	}

	debugLogW(L"Saving picture from %s", szFileName);

	struct _stat st;
	if (_wstat(szFileName, &st) < 0)
		return;

	// Note the FILE_SHARE_READ attribute so that the CopyFile can succeed
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	
	ptrA buffer((char*)mir_alloc(st.st_size));
	if (buffer != nullptr) {
		DWORD nRead;
		if (ReadFile(hFile, buffer, st.st_size, &nRead, nullptr)) {
			ptrA str(mir_base64_encode(buffer, nRead));
			const char *szFileType = ProtoGetAvatarMimeType(ProtoGetBufferFormat(buffer));
			if (str != nullptr && szFileType != nullptr) {
				auto *n = v << XCHILD("PHOTO");
				n << XCHILD("TYPE", szFileType);
				n << XCHILD("BINVAL", str);

				// NEED TO UPDATE OUR AVATAR HASH:
				uint8_t digest[MIR_SHA1_HASH_SIZE];
				mir_sha1_ctx sha1ctx;
				mir_sha1_init(&sha1ctx);
				mir_sha1_append(&sha1ctx, (uint8_t *)(LPSTR)buffer, nRead);
				mir_sha1_finish(&sha1ctx, digest);

				char buf[MIR_SHA1_HASH_SIZE * 2 + 1];
				bin2hex(digest, sizeof(digest), buf);

				if (bPhotoChanged) {
					DeleteFileW(szAvatarName);

					GetAvatarFileName(hContact, szAvatarName, _countof(szAvatarName));
					CopyFileW(szFileName, szAvatarName, FALSE);
				}

				setString(hContact, "AvatarHash", buf);
			}
		}
	}
	CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnUserInfoInit_VCard(WPARAM wParam, LPARAM)
{
	m_vCardUpdates = 0;
	m_bPhotoChanged = false;
	m_szPhotoFileName[0] = 0;

	USERINFOPAGE uip = {};
	uip.flags = ODPF_UNICODE | ODPF_USERINFOTAB | ODPF_ICON;
	uip.szGroup.w = m_tszUserName;
	uip.dwInitParam = (LPARAM)Skin_GetProtoIcon(m_szModuleName, ID_STATUS_ONLINE);

	uip.pDialog = new JabberVcardPersonalDlg(this);
	uip.szTitle.w = LPGENW("General");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new JabberVcardContactDlg(this);
	uip.szTitle.w = LPGENW("Contacts");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new JabberVcardHomeDlg(this);
	uip.szTitle.w = LPGENW("Home");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new JabberVcardWorkDlg(this);
	uip.szTitle.w = LPGENW("Work");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new JabberVcardPhotoDlg(this);
	uip.szTitle.w = LPGENW("Photo");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new JabberVcardNoteDlg(this);
	uip.szTitle.w = LPGENW("Note");
	g_plugin.addUserInfo(wParam, &uip);

	CheckOmemoUserInfo(wParam, uip);
	SendGetVcard(0);
}
