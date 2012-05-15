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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/common.h $
Revision       : $Revision: 1622 $
Last change on : $Date: 2010-06-23 23:32:21 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
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
	if(PopUpOptions.debug){
		MessageBox(NULL, msg, _T("debug"), MB_OK);
	}
}

/* not used
inline void  GetDCBmpSize(HDC hdc, SIZE *sz)
{
	BITMAP bmp;
	GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(bmp), &bmp);
	sz->cx = bmp.bmWidth;
	sz->cy = bmp.bmHeight;
}

inline HBITMAP myLoadBitmap(LPCTSTR fn)
{
	HBITMAP res = (HBITMAP)LoadImage(NULL, fn, IMAGE_BITMAP, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_LOADFROMFILE);
	
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	FILE *f = _tfopen (fn, _T("rb"));
	fread(&bfh, sizeof(bfh), 1, f);
	fread(&bih, sizeof(bih), 1, f);
	fclose(f);
	SetBitmapDimensionEx(res, bih.biWidth, bih.biHeight, NULL);

	return res;
}
*/

//===== Percentile to Byte and viceversa =====
inline int Byte2Percentile(int vByte) { return (vByte*100)/255; }
inline int Percentile2Byte(int vPerc) { return (vPerc*255)/100; }

//===== Strings & MirandaDB ==================
inline char *DBGetContactSettingString(HANDLE hContact, const char *ModuleName, const char *SettingName, const char *Default)
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
			if(!result) {
				ret = (INT_PTR)mir_wstrdup(dbv.pwszVal);
			}
			else {
				ret = (INT_PTR)mir_a2u(Default);
			}
			break;
		default:
			break;
	}
	if(!result)
		CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
	return ret;
}

/* not used
#if !defined(_UNICODE)
inline void SetWindowTextTraslated(HWND hwnd, const char *text)
{
	if (!(hwnd && text)) return;
	if (g_popup.isOsUnicode && IsWindowUnicode(hwnd) && MySetWindowTextW)
	{
		UINT codepage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
		int size = MultiByteToWideChar(codepage, 0, text, lstrlenA(text), 0, 0);
		WCHAR *wtext = new WCHAR[size+1];
		MultiByteToWideChar(codepage, 0, text, lstrlenA(text), wtext, size+1); wtext[size]=0;
		MySetWindowTextW(hwnd, TranslateW(wtext));
		delete [] wtext;
	} else
	{
		SetWindowTextA(hwnd, Translate(text));
	}
}
#endif
*/
/*/dedrecatet (tricky thing to minimize memory fragmentation)
inline wchar_t* a2u( char* src )
{
	int codepage = CallService( MS_LANGPACK_GETCODEPAGE, 0, 0 );

	int cbLen = MultiByteToWideChar( codepage, 0, src, -1, NULL, 0 );
	wchar_t* result = ( wchar_t* )mir_alloc( sizeof( wchar_t )*(cbLen+1));
	if ( result == NULL )
		return NULL;

	MultiByteToWideChar( codepage, 0, src, -1, result, cbLen );
	result[ cbLen ] = 0;
	return result;
}

#define nb_a2u_init()	\
	int nb_a2u_codepage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);	\
	int nb_a2u_len = 0;	\

#define nb_a2u(dst, src)	\
	bool dst##_free = false;	\
	WCHAR *dst = NULL;	\
	nb_a2u_len = MultiByteToWideChar(nb_a2u_codepage, 0, src, -1, NULL, 0);	\
	__try	\
	{	\
		dst = (WCHAR *)_alloca(sizeof(WCHAR)*(nb_a2u_len+1));	\
	}	\
	__except( EXCEPTION_EXECUTE_HANDLER )	\
	{	\
		dst = (WCHAR *)mir_alloc(sizeof(WCHAR)*(nb_a2u_len+1));	\
		dst##_free = true;	\
	}	\
	MultiByteToWideChar(nb_a2u_codepage, 0, src, -1, dst, nb_a2u_len);	\
	dst[nb_a2u_len] = 0;

#define nb_a2u_free(dst)	\
	if (dst##_free)	\
		mir_free(dst);
*/

inline void AddTooltipTranslated(HWND hwndToolTip, HWND hwnd, int id, RECT rc, char *text)
{
#if defined(_UNICODE)
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

#else
	if (g_popup.isOsUnicode && MySendMessageW)
	{
		TOOLINFOW ti = {0};
		ti.cbSize = sizeof(TOOLINFO);

		ti.hwnd = hwnd;
		ti.uId = id;
		MySendMessageW(hwndToolTip, TTM_DELTOOLW, 0, (LPARAM) (LPTOOLINFOW) &ti);

		LPWSTR wtext = mir_a2u(text);

		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwnd;
		ti.uId = id;
		ti.hinst = hInst;
		ti.lpszText = TranslateW(wtext);
		ti.rect = rc;
		MySendMessageW(hwndToolTip, TTM_ADDTOOLW, 0, (LPARAM) (LPTOOLINFOW) &ti);

		mir_free(wtext);
	} else
	{
		TOOLINFOA ti = {0};
		ti.cbSize = sizeof(TOOLINFO);

		ti.hwnd = hwnd;
		ti.uId = id;
		SendMessage(hwndToolTip, TTM_DELTOOLA, 0, (LPARAM) (LPTOOLINFOA) &ti);

		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwnd;
		ti.uId = id;
		ti.hinst = hInst;
		ti.lpszText = Translate(text);
		ti.rect = rc;
		SendMessage(hwndToolTip, TTM_ADDTOOLA, 0, (LPARAM) (LPTOOLINFOA) &ti);
	}
#endif
}

/* not used
inline UINT getCodepageFromCharset(BYTE charset)
{
	switch (charset)
	{
		case ANSI_CHARSET:			return 1252;
		case BALTIC_CHARSET:		return 1257;
		case CHINESEBIG5_CHARSET:	return 950;
		case DEFAULT_CHARSET:		return CP_ACP;
		case EASTEUROPE_CHARSET:	return 1250;
		case GB2312_CHARSET:		return 936;
		case GREEK_CHARSET:			return 1253;
		case HANGUL_CHARSET:		return 949;
		case MAC_CHARSET:			return CP_MACCP;
		case OEM_CHARSET:			return CP_OEMCP;
		case RUSSIAN_CHARSET:		return 1251;
		case SHIFTJIS_CHARSET:		return 932;
//		case SYMBOL_CHARSET:		return ;
		case TURKISH_CHARSET:		return 1254;
		case VIETNAMESE_CHARSET:	return 1258;
		case JOHAB_CHARSET:			return 1361;
		case ARABIC_CHARSET:		return 1256;
		case HEBREW_CHARSET:		return 1255;
		default:					return CP_ACP;
	}
}
*/

#endif //COMMON_H
