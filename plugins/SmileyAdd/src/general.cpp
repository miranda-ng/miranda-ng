/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2012 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "general.h"

static ULONG_PTR g_gdiplusToken = 0;
static bool gdiPlusFail = false;

//
// General functions
//
int CalculateTextHeight(HDC hdc, CHARFORMAT2* chf)
{
	HDC hcdc = CreateCompatibleDC(hdc);

	int logPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
	HFONT hFont = CreateFont(-(chf->yHeight * logPixelsY / 1440), 0, 0, 0, 
		chf->wWeight, chf->dwEffects & CFE_ITALIC ? 1 : 0, 0, 0,
		chf->bCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		chf->szFaceName);
	SelectObject(hcdc, hFont);

	SIZE fontSize;
	GetTextExtentPoint32(hcdc, _T(")"), 1, &fontSize);

	DeleteObject(hFont);
	DeleteDC(hcdc);

	return fontSize.cy;
}

HICON GetDefaultIcon(bool copy)
{
	HICON resIco = Skin_GetIcon("SmileyAdd_ButtonSmiley");
	if (resIco == NULL) 
		resIco = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SMILINGICON), IMAGE_ICON, 0, 0, copy ? 0 : LR_SHARED);
	else if (copy) {
		resIco = (HICON)CopyImage(resIco, IMAGE_ICON, 0, 0, 0);
		Skin_ReleaseIcon("SmileyAdd_ButtonSmiley");
	}

	return resIco;
}

const TCHAR* GetImageExt(CMString &fname)
{
	const TCHAR* ext = _T("");

	int fileId = _topen(fname.c_str(), O_RDONLY | _O_BINARY);
	if (fileId != -1) {
		BYTE buf[6];

		int bytes = _read(fileId, buf, sizeof(buf));
		if (bytes > 4) {
			if ( *(unsigned short*)buf == 0xd8ff )
				ext = _T("jpg");
			else if ( *(unsigned short*)buf == 0x4d42 )
				ext = _T("bmp");
			else if ( *(unsigned*)buf == 0x474e5089 )
				ext = _T("png");
			else if ( *(unsigned*)buf == 0x38464947 )
				ext = _T("gif");
		}
		_close(fileId);
	}
	return ext;
}

HICON ImageList_GetIconFixed (HIMAGELIST himl, INT i, UINT fStyle)
{
	ICONINFO ii;
	HICON hIcon;
	HBITMAP hOldDstBitmap;
	HDC hdcDst;

	int cx, cy;
	ImageList_GetIconSize(himl, &cx, &cy);

	hdcDst = CreateCompatibleDC(NULL);

	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;

	/* draw mask*/
	ii.hbmMask = CreateBitmap (cx, cy, 1, 1, NULL);
	hOldDstBitmap = (HBITMAP)SelectObject (hdcDst, ii.hbmMask);
	PatBlt(hdcDst, 0, 0, cx, cy, WHITENESS);
	ImageList_Draw(himl, i, hdcDst, 0, 0, fStyle | ILD_MASK);

	/* draw image*/
	ii.hbmColor = CreateBitmap (cx, cy, 1, 32, NULL);
	SelectObject (hdcDst, ii.hbmColor);
	PatBlt (hdcDst, 0, 0, cx, cy, BLACKNESS);
	ImageList_Draw(himl, i, hdcDst, 0, 0, fStyle | ILD_TRANSPARENT);

	/*
	* CreateIconIndirect requires us to deselect the bitmaps from
	* the DCs before calling
	*/
	SelectObject(hdcDst, hOldDstBitmap);

	hIcon = CreateIconIndirect (&ii);

	DeleteObject (ii.hbmMask);
	DeleteObject (ii.hbmColor);
	DeleteDC (hdcDst);

	return hIcon;
}

void pathToRelative(const CMString& pSrc, CMString& pOut)
{
	TCHAR szOutPath[MAX_PATH];
	PathToRelativeT(pSrc.c_str(), szOutPath);
	pOut = szOutPath;
}

void pathToAbsolute(const CMString& pSrc, CMString& pOut) 
{
	TCHAR szOutPath[MAX_PATH];

	TCHAR *szVarPath = Utils_ReplaceVarsT(pSrc.c_str());
	if (szVarPath == (TCHAR*)CALLSERVICE_NOTFOUND || szVarPath == NULL) {
		TCHAR szExpPath[MAX_PATH];
		ExpandEnvironmentStrings(pSrc.c_str(), szExpPath, SIZEOF(szExpPath));
		PathToAbsoluteT(szExpPath, szOutPath);
	}
	else {
		PathToAbsoluteT(szVarPath, szOutPath);
		mir_free(szVarPath);
	}
	pOut = szOutPath;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UrlDecode - converts URL chars like %20 into printable characters

static int __fastcall SingleHexToDecimal(char c)
{
	if (c >= '0' && c <= '9') return c-'0';
	if (c >= 'a' && c <= 'f') return c-'a'+10;
	if (c >= 'A' && c <= 'F') return c-'A'+10;
	return -1;
}

void  UrlDecode(char* str)
{
	char* s = str, *d = str;

	while(*s) {
		if (*s == '%') {
			int digit1 = SingleHexToDecimal(s[1]);
			if (digit1 != -1) {
				int digit2 = SingleHexToDecimal(s[2]);
				if (digit2 != -1) {
					s += 3;
					*d++ = (char)((digit1 << 4) | digit2);
					continue;
				}	
			}	
		}
		*d++ = *s++;
	}

	*d = 0;
}


bool InitGdiPlus(void)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	static const TCHAR errmsg[] = _T("GDI+ not installed.\n")
		_T("GDI+ can be downloaded here: http://www.microsoft.com/downloads");

	__try {
		if (g_gdiplusToken == 0 && !gdiPlusFail)
			Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		gdiPlusFail = true;
		ReportError(errmsg);
	}

	return !gdiPlusFail;
}

void DestroyGdiPlus(void)
{
	if (g_gdiplusToken != 0) {
		Gdiplus::GdiplusShutdown(g_gdiplusToken);
		g_gdiplusToken = 0;
	}
}

MCONTACT DecodeMetaContact(MCONTACT hContact)
{
	if (hContact == NULL)
		return NULL;
	
	MCONTACT hReal = db_mc_getMostOnline(hContact);
	if (hReal == NULL || hReal == (MCONTACT)CALLSERVICE_NOTFOUND)
		hReal = hContact;

	return hReal;
}

bool IsSmileyProto(char* proto)
{
	return proto && strcmp(proto, META_PROTO) && (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & (PF1_IM | PF1_CHAT));
}

void ReportError(const TCHAR* errmsg)
{
	static const TCHAR title[] = _T("Miranda SmileyAdd");

	POPUPDATAT pd = {0};
	_tcscpy(pd.lpwzContactName, title);
	_tcscpy(pd.lpwzText, errmsg);
	pd.iSeconds = -1;
	if (PUAddPopupT(&pd) == CALLSERVICE_NOTFOUND)
		MessageBox(NULL, errmsg, title, MB_OK | MB_ICONWARNING | MB_TOPMOST);
}
