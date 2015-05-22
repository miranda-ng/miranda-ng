// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2001-2004 Richard Hughes, Martin Öberg
// Copyright © 2004-2009 Joe Kucera, Bio
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  ChangeInfo Plugin stuff
// -----------------------------------------------------------------------------

#include "stdafx.h"

void ChangeInfoData::LoadSettingsFromDb(int keepChanged)
{
	for (int i = 0; i < settingCount; i++) {
		const SettingItem &si = setting[i];
		if (si.displayType == LI_DIVIDER)
			continue;
		
		SettingItemData &sid = settingData[i];
		if (keepChanged && sid.changed)
			continue;
		
		if (si.dbType == DBVT_ASCIIZ || si.dbType == DBVT_UTF8)
			SAFE_FREE((void**)(char**)&sid.value);
		else if (!keepChanged)
			sid.value = 0;

		sid.changed = 0;

		if (si.displayType & LIF_PASSWORD)
			continue;

		DBVARIANT dbv = { DBVT_DELETED };
		if (!ppro->getSetting(NULL, si.szDbSetting, &dbv)) {
			switch (dbv.type) {
			case DBVT_ASCIIZ:
				sid.value = (LPARAM)ppro->getSettingStringUtf(NULL, si.szDbSetting, NULL);
				break;

			case DBVT_UTF8:
				sid.value = (LPARAM)null_strdup(dbv.pszVal);
				break;

			case DBVT_WORD:
				if (si.displayType & LIF_SIGNED)
					sid.value = dbv.sVal;
				else
					sid.value = dbv.wVal;
				break;

			case DBVT_BYTE:
				if (si.displayType & LIF_SIGNED)
					sid.value = dbv.cVal;
				else
					sid.value = dbv.bVal;
				break;

#ifdef _DEBUG
			default:
				MessageBoxA(NULL, "That's not supposed to happen either", "Huh?", MB_OK);
				break;
#endif
			}
			db_free(&dbv);
		}

		char buf[MAX_PATH];
		TCHAR tbuf[MAX_PATH];

		if (utf8_to_tchar_static(GetItemSettingText(i, buf, SIZEOF(buf)), tbuf, SIZEOF(tbuf)))
			ListView_SetItemText(hwndList, i, 1, tbuf);
	}
}

void ChangeInfoData::FreeStoredDbSettings(void)
{
	for (int i = 0; i < settingCount; i++)
		if (setting[i].dbType == DBVT_ASCIIZ || setting[i].dbType == DBVT_UTF8)
			SAFE_FREE((void**)&settingData[i].value);
}

int ChangeInfoData::ChangesMade(void)
{
	for (int i = 0; i < settingCount; i++)
		if (settingData[i].changed)
			return 1;
	return 0;
}

void ChangeInfoData::ClearChangeFlags(void)
{
	for (int i = 0; i < settingCount; i++)
		settingData[i].changed = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct PwConfirmDlgParam
{
	CIcqProto* ppro;
	char* Pass;
};

static INT_PTR CALLBACK PwConfirmDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PwConfirmDlgParam* dat = (PwConfirmDlgParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_LIMITTEXT, 15, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				char szTest[16];

				GetDlgItemTextA(hwndDlg, IDC_OLDPASS, szTest, SIZEOF(szTest));

				if (mir_strcmp(szTest, dat->ppro->GetUserPassword(TRUE))) {
					MessageBox(hwndDlg, TranslateT("The password does not match your current password. Check Caps Lock and try again."), TranslateT("Change ICQ Details"), MB_OK);
					SendDlgItemMessage(hwndDlg, IDC_OLDPASS, EM_SETSEL, 0, (LPARAM)-1);
					SetFocus(GetDlgItem(hwndDlg, IDC_OLDPASS));
					break;
				}

				GetDlgItemTextA(hwndDlg, IDC_PASSWORD, szTest, SIZEOF(szTest));
				if (mir_strcmp(szTest, dat->Pass)) {
					MessageBox(hwndDlg, TranslateT("The password does not match the password you originally entered. Check Caps Lock and try again."), TranslateT("Change ICQ Details"), MB_OK);
					SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_SETSEL, 0, (LPARAM)-1);
					SetFocus(GetDlgItem(hwndDlg, IDC_PASSWORD));
					break;
				}
			}
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			break;
		}
		break;
	}
	return FALSE;
}

int ChangeInfoData::SaveSettingsToDb(HWND hwndDlg)
{
	int ret = 1;

	for (int i = 0; i < settingCount; i++) {
		SettingItemData &sid = settingData[i];
		if (!sid.changed)
			continue;

		const SettingItem &si = setting[i];
		if (!(si.displayType & LIF_ZEROISVALID) && sid.value == 0) {
			ppro->delSetting(si.szDbSetting);
			continue;
		}
		switch (si.dbType) {
		case DBVT_ASCIIZ:
			if (si.displayType & LIF_PASSWORD) {
				size_t nSettingLen = mir_strlen((char*)sid.value);
				if (nSettingLen > 8 || nSettingLen < 1) {
					MessageBox(hwndDlg, TranslateT("The ICQ server does not support passwords longer than 8 characters. Please use a shorter password."), TranslateT("Change ICQ Details"), MB_OK);
					ret = 0;
					break;
				}
				PwConfirmDlgParam param = { ppro, (char*)sid.value };
				if (IDOK != DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PWCONFIRM), hwndDlg, PwConfirmDlgProc, (LPARAM)&param)) {
					ret = 0;
					break;
				}
				mir_strcpy(ppro->m_szPassword, (char*)sid.value);
			}
			else {
				if (*(char*)sid.value)
					db_set_utf(NULL, ppro->m_szModuleName, si.szDbSetting, (char*)sid.value);
				else
					ppro->delSetting(si.szDbSetting);
			}
			break;

		case DBVT_UTF8:
			if (*(char*)sid.value)
				db_set_utf(NULL, ppro->m_szModuleName, si.szDbSetting, (char*)sid.value);
			else
				ppro->delSetting(si.szDbSetting);
			break;

		case DBVT_WORD:
			ppro->setWord(si.szDbSetting, (WORD)sid.value);
			break;

		case DBVT_BYTE:
			ppro->setByte(si.szDbSetting, (BYTE)sid.value);
			break;
		}
	}
	return ret;
}
