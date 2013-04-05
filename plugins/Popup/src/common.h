/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

#ifndef COMMON_H
#define COMMON_H

void PopUpPreview();

#define MAX_POPUPS 48

#define DB_READ_SUCCESS		0
#define DB_READ_FAIL		-1

//===== Definitions =====
#define LIGHTEN   24
#define DARKEN    64

inline void GetBmpSize(HBITMAP hbm, SIZE *sz)
{
	GetBitmapDimensionEx(hbm, sz);
}

inline void DebugMsg(LPTSTR msg){
	if (PopUpOptions.debug){
		MessageBox(NULL, msg, _T("debug"), MB_OK);
	}
}

//===== Percentile to Byte and viceversa =====
inline int Byte2Percentile(int vByte) { return (vByte*100)/255; }
inline int Percentile2Byte(int vPerc) { return (vPerc*255)/100; }

//===== Strings & MirandaDB ==================
inline char *db_get_s(HANDLE hContact, const char *ModuleName, const char *SettingName, const char *Default)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	dbcgs.szModule = ModuleName;
	dbcgs.pValue = &dbv;
	dbcgs.szSetting = SettingName;

	CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&dbcgs);

	char *result = 0;
	if (dbv.type == DBVT_ASCIIZ)
	{
		result = mir_strdup(dbv.pszVal);
	}
	else if (Default)
	{
		result = mir_strdup(Default);
	}

	CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
	return result;
}

inline INT_PTR DBGetContactSettingStringX(HANDLE hContact, const char *ModuleName, const char *SettingName, const char *Default, const int retType)
{
	INT_PTR ret = NULL;
	BOOL result = 0;
	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	dbcgs.szModule = ModuleName;
	dbcgs.szSetting = SettingName;
	dbcgs.pValue = &dbv;
	dbv.type=(BYTE)retType;

	result = CallService(MS_DB_CONTACT_GETSETTING_STR, (WPARAM)hContact, (LPARAM)&dbcgs);

	switch(retType) {
		case DBVT_ASCIIZ:
			ret = (INT_PTR)mir_strdup(result ? Default : dbv.pszVal);
			break;
		case DBVT_WCHAR:
			if (!result) {
				ret = (INT_PTR)mir_wstrdup(dbv.pwszVal);
			}
			else {
				ret = (INT_PTR)mir_a2u(Default);
			}
			break;
		default:
			break;
	}
	if (!result)
		CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
	return ret;
}

inline void AddTooltipTranslated(HWND hwndToolTip, HWND hwnd, int id, RECT rc, char *text)
{

		TOOLINFO ti = {0};
		ti.cbSize = sizeof(TOOLINFO);

		ti.hwnd = hwnd;
		ti.uId = id;
		SendMessage(hwndToolTip, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

		LPWSTR wtext = mir_a2u(text);

		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwnd;
		ti.uId = id;
		ti.hinst = hInst;
		ti.lpszText = TranslateW(wtext);
		ti.rect = rc;
		SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

		mir_free(wtext);

}

#endif //COMMON_H
