/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

extern uint8_t nameOrder[NAMEORDERCOUNT];

/////////////////////////////////////////////////////////////////////////////////////////

int ListView_GetItemTextA(HWND hwndLV, int i, int iSubItem, char *pszText, int cchTextMax)
{
	LV_ITEMA lvi;
	lvi.iSubItem = iSubItem;
	lvi.cchTextMax = cchTextMax;
	lvi.pszText = pszText;
	return SendMessageA(hwndLV, LVM_GETITEMTEXTA, (WPARAM)(i), (LPARAM)(LV_ITEMA *)&lvi);
}

int ListView_SetItemTextA(HWND hwndLV, int i, int iSubItem, const char *pszText)
{
	LV_ITEMA lvi;
	lvi.iSubItem = iSubItem;
	lvi.pszText = (char*)pszText;
	return SendMessageA(hwndLV, LVM_SETITEMTEXTA, (WPARAM)(i), (LPARAM)(LV_ITEMA *)&lvi);
}

/////////////////////////////////////////////////////////////////////////////////////////

char* StringFromBlob(uint8_t *blob, uint16_t len)
{
	int j;
	char tmp[16];

	char *data = (char*)mir_alloc(3 * (len + 2));
	data[0] = 0;

	for (j = 0; j < len; j++) {
		mir_snprintf(tmp, "%02X ", blob[j]);
		mir_strcat(data, tmp);
	}
	return data;
}

int WriteBlobFromString(MCONTACT hContact, const char *szModule, const char *szSetting, const char *szValue, int len)
{
	int j = 0, i = 0;
	uint8_t b;
	int tmp, res = 0;
	uint8_t *data = (uint8_t*)mir_alloc(2 + len / 2);

	if (!data)
		return 0;

	while (j < len) {
		b = szValue[j];

		if ((b >= '0' && b <= '9') ||
			(b >= 'A' && b <= 'F') ||
			(b >= 'a' && b <= 'f')) {
			if (sscanf(&szValue[j], "%02X", &tmp) == 1) {
				data[i++] = (uint8_t)(tmp & 0xFF);
				j++;
			}
		}
		j++;
	}


	if (i)
		res = !db_set_blob(hContact, szModule, szSetting, data, (uint16_t)i);

	mir_free(data);
	return res;
}

wchar_t* DBVType(uint8_t type)
{
	switch (type) {
	case DBVT_BYTE:     return L"uint8_t";
	case DBVT_WORD:     return L"uint16_t";
	case DBVT_DWORD:    return L"uint32_t";
	case DBVT_ASCIIZ:   return L"STRING";
	case DBVT_WCHAR:
	case DBVT_UTF8:     return L"UNICODE";
	case DBVT_BLOB:     return L"BLOB";
	case DBVT_DELETED:  return L"DELETED";
	}
	return L"";
}

uint32_t getNumericValue(DBVARIANT *dbv)
{
	switch (dbv->type) {
	case DBVT_DWORD:
		return dbv->dVal;
	case DBVT_WORD:
		return dbv->wVal;
	case DBVT_BYTE:
		return dbv->bVal;
	}
	return 0;
}

int setNumericValue(MCONTACT hContact, const char *module, const char *setting, uint32_t value, int type)
{
	switch (type) {
	case DBVT_BYTE:
		if (value <= 0xFF)
			return !db_set_b(hContact, module, setting, (uint8_t)value);
		break;

	case DBVT_WORD:
		if (value <= 0xFFFF)
			return !db_set_w(hContact, module, setting, (uint16_t)value);
		break;

	case DBVT_DWORD:
		return !db_set_dw(hContact, module, setting, value);
	}
	return 0;
}

int IsRealUnicode(wchar_t *value)
{
	BOOL nonascii = 0;
	WideCharToMultiByte(Langpack_GetDefaultCodePage(), WC_NO_BEST_FIT_CHARS, value, -1, nullptr, 0, nullptr, &nonascii);
	return nonascii;
}

int setTextValue(MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int type)
{
	if (type == DBVT_UTF8 || type == DBVT_WCHAR)
		return !db_set_ws(hContact, module, setting, value);

	if (type == DBVT_ASCIIZ && IsRealUnicode(value))
		return 0;

	return !db_set_s(hContact, module, setting, _T2A(value));
}

int GetValueA(MCONTACT hContact, const char *module, const char *setting, char *value, int length)
{
	DBVARIANT dbv = {};

	if (!module || !setting || !value)
		return 0;

	if (length >= 10 && !db_get_s(hContact, module, setting, &dbv, 0)) {
		switch (dbv.type) {

		case DBVT_ASCIIZ:
			mir_strncpy(value, dbv.pszVal, length);
			break;

		case DBVT_DWORD:
		case DBVT_WORD:
		case DBVT_BYTE:
			_ultoa(getNumericValue(&dbv), value, 10);
			break;

		case DBVT_WCHAR:
			mir_strncpy(value, ptrA(mir_u2a(dbv.pwszVal)), length);
			break;

		case DBVT_UTF8:
			mir_strncpy(value, ptrA(mir_utf8decodeA(dbv.pszVal)), length);
			break;

		case DBVT_DELETED:
			value[0] = 0;
			return 0;
		}

		int type = dbv.type;
		db_free(&dbv);
		return type;
	}

	value[0] = 0;
	return 0;
}

int GetValueW(MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int length)
{
	DBVARIANT dbv = {};

	if (!module || !setting || !value)
		return 0;

	if (length >= 10 && !db_get_s(hContact, module, setting, &dbv, 0)) {
		switch (dbv.type) {

		case DBVT_ASCIIZ:
			mir_wstrncpy(value, ptrW(mir_a2u(dbv.pszVal)), length);
			break;

		case DBVT_DWORD:
		case DBVT_WORD:
		case DBVT_BYTE:
			_ultow(getNumericValue(&dbv), value, 10);
			break;

		case DBVT_WCHAR:
			mir_wstrncpy(value, dbv.pwszVal, length);
			break;

		case DBVT_UTF8:
			mir_wstrncpy(value, ptrW(mir_utf8decodeW(dbv.pszVal)), length);
			break;

		case DBVT_DELETED:
			value[0] = 0;
			return 0;
		}

		int type = dbv.type;
		db_free(&dbv);
		return type;
	}

	value[0] = 0;
	return 0;
}

int GetContactName(MCONTACT hContact, const char *proto, wchar_t *value, int maxlen)
{
	if (!value)
		return 0;

	if (!hContact) {
		mir_wstrncpy(value, TranslateT("Settings"), maxlen);
		return 1;
	}

	char *szProto = (char*)proto;
	char tmp[FLD_SIZE];
	wchar_t name[NAME_SIZE]; name[0] = 0;

	if (hContact && (!proto || !proto[0]))
		if (!db_get_static(hContact, "Protocol", "p", tmp, _countof(tmp)))
			szProto = tmp;

	for (int i = 0; i < NAMEORDERCOUNT - 1; i++) {
		switch (nameOrder[i]) {
		case 0: // custom name
			GetValueW(hContact, "CList", "MyHandle", name, _countof(name));
			break;

		case 1: // nick
			if (!szProto) break;
			GetValueW(hContact, szProto, "Nick", name, _countof(name));
			break;

		case 2: // First Name
			// if (!szProto) break;
			// GetValueW(hContact, szProto, "FirstName", name, _countof(name));
			break;

		case 3: // E-mail
			if (!szProto) break;
			GetValueW(hContact, szProto, "e-mail", name, _countof(name));
			break;

		case 4: // Last Name
			// GetValueW(hContact, szProto, "LastName", name, _countof(name));
			break;

		case 5: // Unique id
			if (szProto) {
				// protocol must define a PFLAG_UNIQUEIDSETTING
				const char *uid = Proto_GetUniqueId(szProto);
				if (uid)
					GetValueW(hContact, szProto, uid, name, _countof(name));
			}
			break;

		case 6: // first + last name
			if (szProto) {
				GetValueW(hContact, szProto, "FirstName", name, _countof(name));

				int len = (int)mir_wstrlen(name);
				if (len + 2 < _countof(name)) {
					if (len)
						mir_wstrncat(name, L" ", _countof(name));
					len++;
					GetValueW(hContact, szProto, "LastName", &name[len], _countof(name) - len);
				}
			}
			break;
		}

		if (name[0])
			break;
	}

	if (!name[0])
		mir_wstrncpy(name, TranslateT("<UNKNOWN>"), _countof(name));

	if (szProto && szProto[0]) {
		if (g_Order)
			mir_snwprintf(value, maxlen, L"(%S) %s", szProto, name);
		else
			mir_snwprintf(value, maxlen, L"%s (%S)", name, szProto);
	}
	else mir_wstrncpy(value, name, maxlen);

	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	if (!pa->IsEnabled()) {
		mir_wstrncat(value, L" ", maxlen);
		mir_wstrncat(value, TranslateT("[UNLOADED]"), maxlen);
	}

	return 1;
}

int ApplyProtoFilter(MCONTACT hContact)
{
	if (g_Mode == MODE_ALL)	return 0;

	int loaded = 0;
	char szProto[FLD_SIZE];

	if (!db_get_static(hContact, "Protocol", "p", szProto, _countof(szProto)))
		loaded = Proto_GetAccount(szProto) ? 1 : 0;

	if ((loaded && g_Mode == MODE_UNLOADED) || (!loaded && g_Mode == MODE_LOADED))
		return 1;

	return 0;
}

void loadListSettings(HWND hwnd, ColumnsSettings *cs)
{
	LVCOLUMN sLC = {};
	sLC.fmt = LVCFMT_LEFT;
	sLC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	int i = 0;
	while (cs[i].name) {
		sLC.pszText = TranslateW(cs[i].name);
		sLC.cx = g_plugin.getWord(cs[i].dbname, cs[i].defsize);
		ListView_InsertColumn(hwnd, cs[i].index, &sLC);
		i++;
	}
}

void saveListSettings(HWND hwnd, ColumnsSettings *cs)
{
	char tmp[FLD_SIZE];
	LVCOLUMN sLC = {};
	sLC.mask = LVCF_WIDTH;
	int i = 0;
	while (cs[i].name) {
		if (ListView_GetColumn(hwnd, cs[i].index, &sLC)) {
			mir_snprintf(tmp, cs[i].dbname, i);
			g_plugin.setWord(tmp, (uint16_t)sLC.cx);
		}
		i++;
	}
}

int CALLBACK ColumnsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	ColumnsSortParams params = *(ColumnsSortParams *)myParam;
	const int maxSize = 1024;
	wchar_t text1[maxSize];
	wchar_t text2[maxSize];
	ListView_GetItemText(params.hList, lParam1, params.column, text1, _countof(text1));
	ListView_GetItemText(params.hList, lParam2, params.column, text2, _countof(text2));

	int res = mir_wstrcmpi(text1, text2);
	return (params.column == params.last) ? -res : res;
}
