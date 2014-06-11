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

void PopupPreview();

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
	if (PopupOptions.debug){
		MessageBox(NULL, msg, _T("debug"), MB_OK);
	}
}

//===== Percentile to Byte and viceversa =====
inline int Byte2Percentile(int vByte) { return (vByte*100)/255; }
inline int Percentile2Byte(int vPerc) { return (vPerc*255)/100; }

//===== Strings & MirandaDB ==================
inline char *db_get_s(MCONTACT hContact, const char *ModuleName, const char *SettingName, const char *Default)
{
	DBVARIANT dbv;
	db_get(hContact, ModuleName, SettingName, &dbv);

	char *result = 0;
	if (dbv.type == DBVT_ASCIIZ)
		result = mir_strdup(dbv.pszVal);
	else if (Default)
		result = mir_strdup(Default);

	db_free(&dbv);
	return result;
}

inline INT_PTR DBGetContactSettingStringX(MCONTACT hContact, const char *ModuleName, const char *SettingName, const char *Default, const int retType)
{
	INT_PTR ret = NULL;

	DBVARIANT dbv;
	BOOL result = db_get_s(hContact, ModuleName, SettingName, &dbv, retType);

	switch(retType) {
	case DBVT_ASCIIZ:
		ret = (INT_PTR)mir_strdup(result ? Default : dbv.pszVal);
		break;
	case DBVT_TCHAR:
		if (!result)
			ret = (INT_PTR)mir_tstrdup(dbv.ptszVal);
		else
			ret = (INT_PTR)mir_a2t(Default);
		break;
	}
	if (!result)
		db_free(&dbv);
	return ret;
}

inline void AddTooltipTranslated(HWND hwndToolTip, HWND hwnd, int id, RECT rc, char *text)
{

		TOOLINFO ti = {0};
		ti.cbSize = sizeof(TOOLINFO);

		ti.hwnd = hwnd;
		ti.uId = id;
		SendMessage(hwndToolTip, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

		LPTSTR wtext = mir_a2t(text);

		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwnd;
		ti.uId = id;
		ti.hinst = hInst;
		ti.lpszText = TranslateTS(wtext);
		ti.rect = rc;
		SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

		mir_free(wtext);

}

#endif //COMMON_H
