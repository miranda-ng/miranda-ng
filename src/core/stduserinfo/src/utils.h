/*

Standard User Info plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#define SVS_NORMAL        0
#define SVS_GENDER        1
#define SVS_ZEROISUNSPEC  2
#define SVS_IP            3
#define SVS_COUNTRY       4
#define SVS_MONTH         5
#define SVS_SIGNED        6
#define SVS_TIMEZONE      7
#define SVS_MARITAL    	  8

/////////////////////////////////////////////////////////////////////////////////////////

struct DataItem
{
	const char *szSetting;
	int idCtrl, special;
};

template <size_t _Size>
bool IsEmptyValue(MCONTACT hContact, DataItem(&buffer)[_Size], const char *szModule = nullptr)
{
	if (szModule == nullptr)
		szModule = Proto_GetBaseAccountName(hContact);

	DBVARIANT dbv;
	for (auto &it : buffer) {
		if (db_get_s(hContact, szModule, it.szSetting, &dbv, 0) != 0)
			continue;

		db_free(&dbv);
		return false;
	}

	return true;
}

template <size_t _Size>
void SetValue(HWND hwndDlg, MCONTACT hContact, DataItem(&buffer)[_Size], const char *szModule = nullptr)
{
	if (szModule == nullptr)
		szModule = Proto_GetBaseAccountName(hContact);

	for (auto &it : buffer) {
		char *pstr = nullptr;
		wchar_t *pwstr = nullptr, wstr[80];

		DBVARIANT dbv = { DBVT_DELETED };

		bool unspecified;
		if (szModule == nullptr)
			unspecified = true;
		else
			unspecified = db_get_s(hContact, szModule, it.szSetting, &dbv, 0) != 0;

		if (!unspecified) {
			switch (dbv.type) {
			case DBVT_BYTE:
				if (it.special == SVS_GENDER) {
					if (dbv.cVal == 'M') pwstr = TranslateT("Male");
					else if (dbv.cVal == 'F') pwstr = TranslateT("Female");
					else unspecified = 1;
				}
				else if (it.special == SVS_MONTH) {
					if (dbv.bVal > 0 && dbv.bVal <= 12) {
						pwstr = wstr;
						GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1 - 1 + dbv.bVal, wstr, _countof(wstr));
					}
					else unspecified = 1;
				}
				else if (it.special == SVS_TIMEZONE) {
					if (dbv.cVal == -100) unspecified = 1;
					else {
						pwstr = wstr;
						mir_snwprintf(wstr, dbv.cVal ? L"UTC%+d:%02d" : L"UTC", -dbv.cVal / 2, (dbv.cVal & 1) * 30);
					}
				}
				else if (it.special == SVS_MARITAL) {
					switch (dbv.cVal) {
					case 0:
						pwstr = TranslateT("<not specified>");
						break;
					case 10:
						pwstr = TranslateT("Single");
						break;
					case 11:
						pwstr = TranslateT("Close relationships");
						break;
					case 12:
						pwstr = TranslateT("Engaged");
						break;
					case 20:
						pwstr = TranslateT("Married");
						break;
					case 30:
						pwstr = TranslateT("Divorced");
						break;
					case 31:
						pwstr = TranslateT("Separated");
						break;
					case 40:
						pwstr = TranslateT("Widowed");
						break;
					case 50:
						pwstr = TranslateT("Actively searching");
						break;
					case 60:
						pwstr = TranslateT("In love");
						break;
					case 70:
						pwstr = TranslateT("It's complicated");
						break;
					case 80:
						pwstr = TranslateT("In a civil union");
						break;
					default:
						unspecified = 1;
					}
				}
				else {
					unspecified = (it.special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
					pwstr = _itow(it.special == SVS_SIGNED ? dbv.cVal : dbv.bVal, wstr, 10);
				}
				break;

			case DBVT_WORD:
				if (it.special == SVS_COUNTRY) {
					uint16_t wSave = dbv.wVal;
					if (wSave == (uint16_t)-1) {
						char szSettingName[100];
						mir_snprintf(szSettingName, "%sName", it.szSetting);
						if (!db_get_ws(hContact, szModule, szSettingName, &dbv)) {
							pwstr = dbv.pwszVal;
							unspecified = false;
							break;
						}
					}

					pwstr = TranslateW(_A2T((char *)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, wSave, 0)));
					unspecified = pwstr == nullptr;
				}
				else {
					unspecified = (it.special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
					pwstr = _itow(it.special == SVS_SIGNED ? dbv.sVal : dbv.wVal, wstr, 10);
				}
				break;

			case DBVT_DWORD:
				unspecified = (it.special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
				if (it.special == SVS_IP) {
					struct in_addr ia;
					ia.S_un.S_addr = htonl(dbv.dVal);
					mir_wstrncpy(wstr, _A2T(inet_ntoa(ia)), _countof(wstr));
					pwstr = wstr;
					if (dbv.dVal == 0)
						unspecified = 1;
				}
				else pwstr = _itow(it.special == SVS_SIGNED ? dbv.lVal : dbv.dVal, wstr, 10);
				break;

			case DBVT_ASCIIZ:
				unspecified = (it.special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
				pstr = dbv.pszVal;
				break;

			case DBVT_UTF8:
				unspecified = (it.special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
				if (!unspecified) {
					SetDlgItemTextW(hwndDlg, it.idCtrl, TranslateW(ptrW(mir_utf8decodeW(dbv.pszVal))));
					goto LBL_Exit;
				}

				mir_utf8decode(dbv.pszVal, &pwstr);
				break;

			default:
				pwstr = wstr;
				mir_wstrcpy(wstr, L"???");
				break;
			}
		}

		if (unspecified)
			SetDlgItemText(hwndDlg, it.idCtrl, TranslateT("<not specified>"));
		else if (pwstr != nullptr)
			SetDlgItemText(hwndDlg, it.idCtrl, pwstr);
		else
			SetDlgItemTextA(hwndDlg, it.idCtrl, pstr);

LBL_Exit:
		EnableWindow(GetDlgItem(hwndDlg, it.idCtrl), !unspecified);
		db_free(&dbv);
	}
}
