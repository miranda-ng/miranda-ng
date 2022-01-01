/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

//Include
#include "stdafx.h"

#define _EFFECTENUM_FULL_H
#include "modern_effectenum.h"
#undef _EFFECTENUM_FULL_H

#include "modern_sync.h"

//Implementation

#pragma pack(push, 1)
/* tga header */
struct tga_header_t
{
	uint8_t id_lenght;          /* size of image id */
	uint8_t colormap_type;      /* 1 is has a colormap */
	uint8_t image_type;         /* compression type */

	short	cm_first_entry;       /* colormap origin */
	short	cm_length;            /* colormap length */
	uint8_t cm_size;               /* colormap size */

	short	x_origin;             /* bottom left x coord origin */
	short	y_origin;             /* bottom left y coord origin */

	short	width;                /* picture width (in pixels) */
	short	height;               /* picture height (in pixels) */

	uint8_t pixel_depth;        /* bits per pixel: 8, 16, 24 or 32 */
	uint8_t image_descriptor;   /* 24 bits = 0x00; 32 bits = 0x80 */
};
#pragma pack(pop)

/* Global variables */

SKINOBJECTSLIST g_SkinObjectList = { 0 };
CURRWNDIMAGEDATA *g_pCachedWindow = nullptr;

bool g_bPostWasCanceled = false;
bool g_bFullRepaint = false;

int g_mutex_bLockUpdating = 0;

SortedList *gl_plGlyphTexts = nullptr;
SortedList *gl_plSkinFonts = nullptr;

/* Private module variables */

static HANDLE  hSkinLoadedEvent;

static GLYPHIMAGE *pLoadedImages = nullptr;
static uint32_t dwLoadedImagesCount = 0;
static uint32_t dwLoadedImagesAlocated = 0;

static BOOL flag_bUpdateQueued = FALSE;
static BOOL	flag_bJustDrawNonFramedObjects = FALSE;
static BOOL mutex_bLockUpdate = FALSE;

static LIST<EFFECTSSTACKITEM> arEffectStack(10, HandleKeySortT);
static SKINOBJECTSLIST *pCurrentSkin = nullptr;
static char  **pszSettingName = nullptr;
static int   nArrayLen = 0;

static uint8_t  pbGammaWeight[256] = { 0 };
static uint8_t  pbGammaWeightAdv[256] = { 0 };
static BOOL  bGammaWeightFilled = FALSE;

static mir_cs cs_SkinChanging;

static LISTMODERNMASK *MainModernMaskList = nullptr;

/* Private module procedures */
static BOOL ske_GetMaskBit(uint8_t *line, int x);
static INT_PTR  ske_Service_AlphaTextOut(WPARAM wParam, LPARAM lParam);
static INT_PTR ske_Service_DrawIconEx(WPARAM wParam, LPARAM lParam);

static int  ske_AlphaTextOut(HDC hDC, LPCTSTR lpString, int nCount, RECT *lpRect, UINT format, uint32_t ARGBcolor);
static void ske_AddParseTextGlyphObject(char * szGlyphTextID, char * szDefineString, SKINOBJECTSLIST *Skin);
static void ske_AddParseSkinFont(char * szFontID, char * szDefineString);
static int  ske_GetSkinFromDB(char * szSection, SKINOBJECTSLIST * Skin);
static SKINOBJECTDESCRIPTOR* ske_FindObject(const char *szName, SKINOBJECTSLIST *Skin);
static int  ske_LoadSkinFromResource(BOOL bOnlyObjects);
static void ske_PreMultiplyChannels(HBITMAP hbmp, uint8_t Mult);
static int  ske_ValidateSingleFrameImage(FRAMEWND * Frame, BOOL SkipBkgBlitting);
static INT_PTR ske_Service_UpdateFrameImage(WPARAM wParam, LPARAM lParam);
static INT_PTR ske_Service_InvalidateFrameImage(WPARAM wParam, LPARAM lParam);
static INT_PTR ske_Service_DrawTextWithEffect(WPARAM wParam, LPARAM lParam);

static MODERNEFFECT meCurrentEffect = { 0xFF, { 0 }, 0, 0 };

//////////////////////////////////////////////////////////////////////////
// Ini file parser

IniParser::IniParser(wchar_t * tcsFileName, uint8_t flags) : _Flags(flags)
{
	_DoInit();
	if (!tcsFileName) return;

	if (tcsFileName[0] == '%') {
		//TODO: Add parser of resource filename here
		_LoadResourceIni(g_plugin.getInst(), MAKEINTRESOURCEA(IDR_MSF_DEFAULT_SKIN), "MSF");
		return;
	}

	_hFile = _wfopen(tcsFileName, L"r");
	if (_hFile != nullptr) {
		_eType = IT_FILE;
		_isValid = true;
	}
}

IniParser::IniParser(HINSTANCE hInst, const char *  resourceName, const char * resourceType, uint8_t flags) : _Flags(flags)
{
	_DoInit();
	_LoadResourceIni(hInst, resourceName, resourceType);
}

IniParser::~IniParser()
{
	mir_free(_szSection);
	if (_hFile) fclose(_hFile);
	if (_hGlobalRes) {
		UnlockResource(_hGlobalRes);
		FreeResource(_hGlobalRes);
	}

	_szSection = nullptr;
	_hGlobalRes = nullptr;
	_hFile = nullptr;
	_isValid = false;
	_eType = IT_UNKNOWN;
}

HRESULT IniParser::Parse(ParserCallback_t pLineCallBackProc, LPARAM SecCheck)
{
	if (_isValid && pLineCallBackProc) {
		_pLineCallBackProc = pLineCallBackProc;
		_SecCheck = SecCheck;
		switch (_eType) {
		case IT_FILE:
			return _DoParseFile();
		case IT_RESOURCE:
			return _DoParseResource();
		}
	}
	return E_FAIL;
}

HRESULT IniParser::WriteStrToDb(const char * szSection, const char * szName, const char * szValue, IniParser * This)
{
	if (This->_SecCheck) {
		//TODO check security here
		if (wildcmp(szSection, "Skin_Description_Section"))
			return S_OK;
	}
	if ((This->_Flags == IniParser::FLAG_ONLY_OBJECTS) && !wildcmp(szSection, DEFAULTSKINSECTION))
		return S_OK;					 // skip not objects

	switch (szValue[0]) {
	case 'b':
		db_set_b(0, szSection, szName, (uint8_t)atoi(szValue + 1));
		break;

	case 'w':
		db_set_w(0, szSection, szName, (uint16_t)atoi(szValue + 1));
		break;

	case 'd':
		db_set_dw(0, szSection, szName, (uint32_t)atoi(szValue + 1));
		break;

	case 's':
		db_set_s(0, szSection, szName, szValue + 1);
		break;
	}
	return S_OK;
}

int IniParser::GetSkinFolder(IN const wchar_t * szFileName, OUT wchar_t * pszFolderName)
{
	wchar_t *szBuff = mir_wstrdup(szFileName);
	wchar_t *pszPos = szBuff + mir_wstrlen(szBuff);
	while (pszPos > szBuff && *pszPos != '.') { pszPos--; }
	*pszPos = '\0';
	mir_wstrcpy(pszFolderName, szBuff);

	wchar_t custom_folder[MAX_PATH], cus[MAX_PATH];
	wchar_t *b3;
	mir_wstrncpy(custom_folder, pszFolderName, _countof(custom_folder));
	b3 = custom_folder + mir_wstrlen(custom_folder);
	while (b3 > custom_folder && *b3 != '\\') { b3--; }
	*b3 = '\0';

	GetPrivateProfileString(L"Skin_Description_Section", L"SkinFolder", L"", cus, _countof(custom_folder), szFileName);
	if (cus[0] != 0)
		mir_snwprintf(pszFolderName, MAX_PATH, L"%s\\%s", custom_folder, cus);

	mir_free(szBuff);
	PathToRelativeW(pszFolderName, pszFolderName);
	return 0;
}

void IniParser::_DoInit()
{
	_isValid = false;
	_eType = IT_UNKNOWN;
	_szSection = nullptr;
	_hFile = nullptr;
	_hGlobalRes = nullptr;
	_dwSizeOfRes = 0;
	_pPosition = nullptr;
	_pLineCallBackProc = nullptr;
	_SecCheck = 0;
}

void IniParser::_LoadResourceIni(HINSTANCE hInst, const char *  resourceName, const char * resourceType)
{
	if (_eType != IT_UNKNOWN)
		return;

	HRSRC hRSrc = FindResourceA(hInst, resourceName, resourceType);
	if (!hRSrc)
		return;

	_hGlobalRes = LoadResource(hInst, hRSrc);
	if (!_hGlobalRes)
		return;

	_dwSizeOfRes = SizeofResource(hInst, hRSrc);
	_pPosition = (char*)LockResource(_hGlobalRes);

	_isValid = true;
	_eType = IT_RESOURCE;
}

HRESULT IniParser::_DoParseFile()
{
	char szLine[MAX_LINE_LEN];
	_nLine = 0;
	while (fgets(szLine, _countof(szLine), _hFile) != nullptr) {
		size_t len = 0;
		char *pLine = (char *)_RemoveTailings(szLine, len);
		if (len > 0) {
			pLine[len] = '\0';
			if (!_DoParseLine(pLine))
				return E_FAIL;
		}
		else _nLine++;
	};

	return S_OK;
}

HRESULT IniParser::_DoParseResource()
{
	_nLine = 0;
	char szLine[MAX_LINE_LEN];
	char *pos = (char *)_pPosition;

	while (pos < _pPosition + _dwSizeOfRes) {
		int i = 0;
		while (pos < _pPosition + _dwSizeOfRes && *pos != '\n' && *pos != '\0' && i < MAX_LINE_LEN - 1) {
			if ((*pos) != '\r')
				szLine[i++] = *pos;
			pos++;
		}
		szLine[i] = '\0';
		pos++;

		size_t len = 0;
		char *pLine = (char *)_RemoveTailings(szLine, len);
		if (len > 0) {
			pLine[len] = '\0';
			if (!_DoParseLine(pLine))
				return E_FAIL;
		}
		else _nLine++;
	}
	return S_OK;
}

const char *IniParser::_RemoveTailings(const char *szLine, size_t &len)
{
	const char *pStart = szLine;
	while (*pStart == ' ' || *pStart == '\t')
		pStart++; //skip spaces at begin
	const char *pEnd = pStart + mir_strlen(pStart);
	while (pEnd > pStart && (*pEnd == ' ' || *pEnd == '\t' || *pEnd == '\n' || *pEnd == '\r'))
		pEnd--;

	len = pEnd - pStart;
	return pStart;
}

BOOL IniParser::_DoParseLine(char *szLine)
{
	_nLine++;
	size_t len = mir_strlen(szLine);
	if (len == 0)
		return TRUE;

	switch (szLine[0]) {
	case ';':
		return TRUE; // start of comment is found

	case '[':
		//New section start here
		mir_free(_szSection);
		_szSection = nullptr;
		{
			char *tbuf = szLine + 1;	// skip [

			char *ebuf = tbuf;

			while (*ebuf != ']' && *ebuf != '\0') ebuf++;
			if (*ebuf == '\0')
				return FALSE; // no close bracket

			uint32_t sectionLen = ebuf - tbuf + 1;
			_szSection = (char*)mir_alloc(sectionLen + 1);
			mir_strncpy(_szSection, tbuf, sectionLen);
			_szSection[sectionLen] = '\0';
		}
		return TRUE;

	default:
		if (!_szSection)
			return TRUE;  //param found out of section

		char *keyName = szLine;
		char *keyValue = szLine;

		size_t eqPlace = 0, len2 = mir_strlen(keyName);
		while (eqPlace < len2 && keyName[eqPlace] != '=')
			eqPlace++; //find '='

		if (eqPlace == 0 || eqPlace == len2)
			return TRUE; // = not found or no key name //say false

		keyName[eqPlace] = '\0';

		keyValue = keyName + eqPlace + 1;

		//remove tail spaces in Name
		rtrim(keyName);
		while (*keyValue) {
			if (!isspace(*keyValue))
				break;
			keyValue++;
		}
		rtrim(keyValue);
		_pLineCallBackProc(_szSection, keyName, keyValue, this);
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// End of IniParser
//////////////////////////////////////////////////////////////////////////

HRESULT SkinEngineLoadModule()
{
	ModernSkinButtonLoadModule();
	MainModernMaskList = (LISTMODERNMASK*)mir_calloc(sizeof(LISTMODERNMASK));
	//init variables
	g_SkinObjectList.dwObjLPAlocated = 0;
	g_SkinObjectList.dwObjLPReserved = 0;
	g_SkinObjectList.pObjects = nullptr;
	// Initialize GDI+
	InitGdiPlus();
	AniAva_InitModule();
	//create services
	CreateServiceFunction(MS_SKIN_DRAWGLYPH, ske_Service_DrawGlyph);
	CreateServiceFunction(MS_SKINENG_UPTATEFRAMEIMAGE, ske_Service_UpdateFrameImage);
	CreateServiceFunction(MS_SKINENG_INVALIDATEFRAMEIMAGE, ske_Service_InvalidateFrameImage);
	CreateServiceFunction(MS_SKINENG_ALPHATEXTOUT, ske_Service_AlphaTextOut);
	CreateServiceFunction(MS_SKINENG_DRAWICONEXFIX, ske_Service_DrawIconEx);

	CreateServiceFunction(MS_DRAW_TEXT_WITH_EFFECT, ske_Service_DrawTextWithEffect);

	//create event handle
	hSkinLoadedEvent = HookEvent(ME_SKIN_SERVICESCREATED, CLUI_OnSkinLoad);
	NotifyEventHooks(g_CluiData.hEventSkinServicesCreated, 0, 0);
	return S_OK;
}

int SkinEngineUnloadModule()
{
	//unload services
	ModernSkinButtonUnloadModule(0, 0);
	ske_UnloadSkin(&g_SkinObjectList);

	mir_free_and_nil(g_SkinObjectList.pObjects);
	mir_free_and_nil(g_SkinObjectList.pMaskList);
	mir_free_and_nil(MainModernMaskList);

	for (auto &it : arEffectStack)
		mir_free(it);
	arEffectStack.destroy();

	if (g_pCachedWindow) {
		SelectObject(g_pCachedWindow->hBackDC, g_pCachedWindow->hBackOld);
		SelectObject(g_pCachedWindow->hImageDC, g_pCachedWindow->hImageOld);
		DeleteObject(g_pCachedWindow->hBackDIB);
		DeleteObject(g_pCachedWindow->hImageDIB);
		DeleteDC(g_pCachedWindow->hBackDC);
		DeleteDC(g_pCachedWindow->hImageDC);
		ReleaseDC(nullptr, g_pCachedWindow->hScreenDC);
		mir_free_and_nil(g_pCachedWindow);
	}
	GdiFlush();
	DestroyHookableEvent(g_CluiData.hEventSkinServicesCreated);
	AniAva_UnloadModule();
	ShutdownGdiPlus();
	//free variables
	return 1;
}

BOOL ske_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction)
{
	if (g_CluiData.fDisableSkinEngine && !(blendFunction.BlendFlags & 128)) {
		if (nWidthDest != nWidthSrc || nHeightDest != nHeightSrc)
			return StretchBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);
		return BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
	}

	if (blendFunction.BlendFlags & 128) // Use gdi+ engine
		return GDIPlus_AlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
			hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc,
			&blendFunction);

	blendFunction.BlendFlags &= ~128;
	return AlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, blendFunction);
}

struct DCBUFFER
{
	HDC hdcOwnedBy;
	int nUsageID;
	int width;
	int height;
	void *pImage;
	HDC hDC;
	HBITMAP oldBitmap;
	HBITMAP hBitmap;
	uint32_t dwDestroyAfterTime;
};

static int SortBufferList(const DCBUFFER *buf1, const DCBUFFER *buf2)
{
	if (buf1->hdcOwnedBy != buf2->hdcOwnedBy) return (int)(buf1->hdcOwnedBy < buf2->hdcOwnedBy);
	else if (buf1->nUsageID != buf2->nUsageID) return (int)(buf1->nUsageID < buf2->nUsageID);
	else return (int)(buf1->hDC < buf2->hDC);
}

LIST<DCBUFFER> BufferList(2, SortBufferList);
mir_cs BufferListCS;

enum
{
	BUFFER_DRAWICON = 0,
	BUFFER_DRAWIMAGE
};

HDC ske_RequestBufferDC(HDC hdcOwner, int dcID, int width, int height, BOOL fClear)
{
	mir_cslock lck(BufferListCS);
	//Try to find DC in buffer list
	DCBUFFER buf;
	buf.hdcOwnedBy = hdcOwner;
	buf.nUsageID = dcID;
	buf.hDC = nullptr;
	DCBUFFER *pBuf = BufferList.find(&buf);
	if (!pBuf) {
		//if not found - allocate it
		pBuf = (DCBUFFER *)mir_alloc(sizeof(DCBUFFER));
		*pBuf = buf;
		pBuf->width = width;
		pBuf->height = height;
		pBuf->hBitmap = ske_CreateDIB32Point(width, height, &(pBuf->pImage));
		pBuf->hDC = CreateCompatibleDC(hdcOwner);
		pBuf->oldBitmap = (HBITMAP)SelectObject(pBuf->hDC, pBuf->hBitmap);
		pBuf->dwDestroyAfterTime = 0;
		BufferList.insert(pBuf);
	}
	else {
		if (pBuf->width != width || pBuf->height != height) {
			//resize
			SelectObject(pBuf->hDC, pBuf->oldBitmap);
			DeleteObject(pBuf->hBitmap);
			pBuf->width = width;
			pBuf->height = height;
			pBuf->hBitmap = ske_CreateDIB32Point(width, height, &(pBuf->pImage));
			pBuf->oldBitmap = (HBITMAP)SelectObject(pBuf->hDC, pBuf->hBitmap);
		}
		else if (fClear)
			memset(pBuf->pImage, 0, width*height*sizeof(uint32_t));
	}
	pBuf->dwDestroyAfterTime = 0;
	return pBuf->hDC;
}

int ske_ReleaseBufferDC(HDC hDC, int keepTime)
{
	uint32_t dwCurrentTime = GetTickCount();

	// Try to find DC in buffer list - set flag to be release after time;
	mir_cslock lck(BufferListCS);
	for (auto &it : BufferList.rev_iter()) {
		if (it) {
			if (hDC != nullptr && it->hDC == hDC) {
				it->dwDestroyAfterTime = dwCurrentTime + keepTime;
				break;
			}

			if ((it->dwDestroyAfterTime && it->dwDestroyAfterTime < dwCurrentTime) || keepTime == -1) {
				SelectObject(it->hDC, it->oldBitmap);
				DeleteObject(it->hBitmap);
				DeleteDC(it->hDC);
				mir_free(BufferList.removeItem(&it));
			}
		}
	}
	return 0;
}

BOOL ske_SetRgnOpaque(HDC memdc, HRGN hrgn, BOOL force)
{
	if (g_CluiData.fDisableSkinEngine && !force) return TRUE;
	uint32_t rgnsz = GetRegionData(hrgn, 0, nullptr);
	RGNDATA *rdata = (RGNDATA *)mir_alloc(rgnsz);
	GetRegionData(hrgn, rgnsz, rdata);
	RECT *rect = (RECT *)rdata->Buffer;
	for (uint32_t d = 0; d < rdata->rdh.nCount; d++) {
		ske_SetRectOpaque(memdc, &rect[d], force);
	}
	mir_free(rdata);
	return TRUE;
}


BOOL ske_SetRectOpaque(HDC memdc, RECT *fr, BOOL force)
{
	int f = 0;
	uint8_t *bits;
	BITMAP bmp;

	if (g_CluiData.fDisableSkinEngine && !force)
		return TRUE;

	HBITMAP hbmp = (HBITMAP)GetCurrentObject(memdc, OBJ_BITMAP);
	GetObject(hbmp, sizeof(bmp), &bmp);

	if (bmp.bmPlanes != 1)
		return FALSE;

	if (!bmp.bmBits) {
		f = 1;
		bits = (uint8_t*)mir_alloc(bmp.bmWidthBytes*bmp.bmHeight);
		GetBitmapBits(hbmp, bmp.bmWidthBytes*bmp.bmHeight, bits);
	}
	else
		bits = (uint8_t*)bmp.bmBits;

	int sx = (fr->left > 0) ? fr->left : 0;
	int sy = (fr->top > 0) ? fr->top : 0;
	int ex = (fr->right < bmp.bmWidth) ? fr->right : bmp.bmWidth;
	int ey = (fr->bottom < bmp.bmHeight) ? fr->bottom : bmp.bmHeight;

	int width = ex - sx;

	uint8_t *pLine = ((uint8_t *)bits) + (bmp.bmHeight - sy - 1) * bmp.bmWidthBytes + (sx << 2) + 3;
	for (int y = 0; y < (ey - sy); y++) {
		uint8_t *pColumn = pLine;
		for (int x = 0; x < width; x++) {
			*pColumn = 255;
			pColumn += 4;
		}
		pLine -= bmp.bmWidthBytes;
	}
	if (f) {
		SetBitmapBits(hbmp, bmp.bmWidthBytes*bmp.bmHeight, bits);
		mir_free(bits);
	}
	// DeleteObject(hbmp);
	return 1;
}

static BOOL ske_SkinFillRectByGlyph(HDC hDest, HDC hSource, RECT *rFill, RECT *rGlyph, RECT *rClip, uint8_t mode, uint8_t drawMode)
{
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

	//initializations
	if (mode == FM_STRETCH) {
		HDC mem2dc = nullptr;
		HBITMAP mem2bmp = nullptr, oldbmp = nullptr;
		RECT wr;
		IntersectRect(&wr, rClip, rFill);
		if ((wr.bottom - wr.top)*(wr.right - wr.left) == 0) return 0;
		if (drawMode != 2) {
			mem2dc = CreateCompatibleDC(hDest);
			mem2bmp = ske_CreateDIB32(wr.right - wr.left, wr.bottom - wr.top);
			oldbmp = (HBITMAP)SelectObject(mem2dc, mem2bmp);
		}

		if (drawMode == 0 || drawMode == 2) {
			if (drawMode == 0) {
				ske_AlphaBlend(mem2dc, rFill->left - wr.left, rFill->top - wr.top, rFill->right - rFill->left, rFill->bottom - rFill->top,
					hSource, rGlyph->left, rGlyph->top, rGlyph->right - rGlyph->left, rGlyph->bottom - rGlyph->top, bf);
				ske_AlphaBlend(hDest, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, mem2dc, 0, 0, wr.right - wr.left, wr.bottom - wr.top, bf);
			}
			else {
				ske_AlphaBlend(hDest, rFill->left, rFill->top, rFill->right - rFill->left, rFill->bottom - rFill->top,
					hSource, rGlyph->left, rGlyph->top, rGlyph->right - rGlyph->left, rGlyph->bottom - rGlyph->top, bf);

			}
		}
		else {
			//            BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, 0 };
			ske_AlphaBlend(mem2dc, rFill->left - wr.left, rFill->top - wr.top, rFill->right - rFill->left, rFill->bottom - rFill->top,
				hSource, rGlyph->left, rGlyph->top, rGlyph->right - rGlyph->left, rGlyph->bottom - rGlyph->top, bf);
			ske_AlphaBlend(hDest, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, mem2dc, 0, 0, wr.right - wr.left, wr.bottom - wr.top, bf);
		}
		if (drawMode != 2) {
			SelectObject(mem2dc, oldbmp);
			DeleteObject(mem2bmp);
			DeleteDC(mem2dc);
		}
		return 1;
	}
	else if (mode == FM_TILE_VERT && (rGlyph->bottom - rGlyph->top > 0) && (rGlyph->right - rGlyph->left > 0)) {
		RECT wr;
		IntersectRect(&wr, rClip, rFill);
		if ((wr.bottom - wr.top)*(wr.right - wr.left) == 0) return 0;
		HDC mem2dc = CreateCompatibleDC(hDest);
		//SetStretchBltMode(mem2dc, HALFTONE);
		HBITMAP mem2bmp = ske_CreateDIB32(wr.right - wr.left, rGlyph->bottom - rGlyph->top);
		HBITMAP oldbmp = (HBITMAP)SelectObject(mem2dc, mem2bmp);
		if (!oldbmp)
			return 0;

		/// draw here
		{
			int  y = 0;
			int w = rFill->right - rFill->left;
			int h = rGlyph->bottom - rGlyph->top;
			if (h > 0 && (wr.bottom - wr.top)*(wr.right - wr.left) != 0) {
				w = wr.right - wr.left;
				{
					//                   BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, 0 };
					ske_AlphaBlend(mem2dc, -(wr.left - rFill->left), 0, rFill->right - rFill->left, h, hSource, rGlyph->left, rGlyph->top, rGlyph->right - rGlyph->left, h, bf);
					//StretchBlt(mem2dc,-(wr.left-rFill->left), 0, rFill->right-rFill->left,h,hSource,rGlyph->left,rGlyph->top,rGlyph->right-rGlyph->left,h,SRCCOPY);
				}
				if (drawMode == 0 || drawMode == 2) {
					if (drawMode == 0) {
						int dy = (wr.top - rFill->top) % h;
						if (dy >= 0) {
							y = wr.top;
							int ht = (y + h - dy <= wr.bottom) ? (h - dy) : (wr.bottom - wr.top);
							BitBlt(hDest, wr.left, y, w, ht, mem2dc, 0, dy, SRCCOPY);
						}

						y = wr.top + h - dy;
						while (y < wr.bottom - h) {
							BitBlt(hDest, wr.left, y, w, h, mem2dc, 0, 0, SRCCOPY);
							y += h;
						}
						if (y <= wr.bottom)
							BitBlt(hDest, wr.left, y, w, wr.bottom - y, mem2dc, 0, 0, SRCCOPY);

					}
					else {
						y = wr.top;
						while (y < wr.bottom - h) {
							//                             BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, 0 };
							ske_AlphaBlend(hDest, wr.left, y, w, h, mem2dc, 0, 0, w, h, bf);
							y += h;
						}
						if (y <= wr.bottom) {
							//                           BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, 0 };
							ske_AlphaBlend(hDest, wr.left, y, w, wr.bottom - y, mem2dc, 0, 0, w, wr.bottom - y, bf);
						}
					}

				}
				else {
					BLENDFUNCTION bf2 = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					int dy = (wr.top - rFill->top) % h;
					if (dy >= 0) {
						y = wr.top;
						int ht = (y + h - dy <= wr.bottom) ? (h - dy) : (wr.bottom - wr.top);
						ske_AlphaBlend(hDest, wr.left, y, w, ht, mem2dc, 0, dy, w, ht, bf2);
					}

					y = wr.top + h - dy;
					while (y < wr.bottom - h) {
						ske_AlphaBlend(hDest, wr.left, y, w, h, mem2dc, 0, 0, w, h, bf2);
						y += h;
					}
					if (y <= wr.bottom)
						ske_AlphaBlend(hDest, wr.left, y, w, wr.bottom - y, mem2dc, 0, 0, w, wr.bottom - y, bf2);
				}
			}
		}
		SelectObject(mem2dc, oldbmp);
		DeleteObject(mem2bmp);
		DeleteDC(mem2dc);
	}
	else if (mode == FM_TILE_HORZ && (rGlyph->right - rGlyph->left > 0) && (rGlyph->bottom - rGlyph->top > 0) && (rFill->bottom - rFill->top) > 0 && (rFill->right - rFill->left) > 0) {
		RECT wr;
		int w = rGlyph->right - rGlyph->left;
		int h = rFill->bottom - rFill->top;
		IntersectRect(&wr, rClip, rFill);
		if ((wr.bottom - wr.top)*(wr.right - wr.left) == 0) return 0;
		h = wr.bottom - wr.top;
		HDC mem2dc = CreateCompatibleDC(hDest);

		HBITMAP mem2bmp = ske_CreateDIB32(w, h);
		HBITMAP oldbmp = (HBITMAP)SelectObject(mem2dc, mem2bmp);

		if (!oldbmp)
			return 0;
		/// draw here
		{
			int  x = 0;
			{
				//SetStretchBltMode(mem2dc, HALFTONE);
				//StretchBlt(mem2dc, 0, 0, w,h,hSource,rGlyph->left+(wr.left-rFill->left),rGlyph->top,w,h,SRCCOPY);

				//                    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, 0 };
				ske_AlphaBlend(mem2dc, 0, -(wr.top - rFill->top), w, rFill->bottom - rFill->top, hSource, rGlyph->left, rGlyph->top, w, rGlyph->bottom - rGlyph->top, bf);
				if (drawMode == 0 || drawMode == 2) {
					if (drawMode == 0) {
						int dx = (wr.left - rFill->left) % w;
						if (dx >= 0) {
							x = wr.left;
							int wt = (x + w - dx <= wr.right) ? (w - dx) : (wr.right - wr.left);
							BitBlt(hDest, x, wr.top, wt, h, mem2dc, dx, 0, SRCCOPY);
						}
						x = wr.left + w - dx;
						while (x < wr.right - w) {
							BitBlt(hDest, x, wr.top, w, h, mem2dc, 0, 0, SRCCOPY);
							x += w;
						}
						if (x <= wr.right)
							BitBlt(hDest, x, wr.top, wr.right - x, h, mem2dc, 0, 0, SRCCOPY);
					}
					else {
						int dx = (wr.left - rFill->left) % w;
						x = wr.left - dx;
						while (x < wr.right - w) {
							ske_AlphaBlend(hDest, x, wr.top, w, h, mem2dc, 0, 0, w, h, bf);
							x += w;
						}
						if (x <= wr.right)
							ske_AlphaBlend(hDest, x, wr.top, wr.right - x, h, mem2dc, 0, 0, wr.right - x, h, bf);
					}

				}
				else {
					BLENDFUNCTION bf2 = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					int dx = (wr.left - rFill->left) % w;
					if (dx >= 0) {
						x = wr.left;
						int wt = (x + w - dx <= wr.right) ? (w - dx) : (wr.right - wr.left);
						ske_AlphaBlend(hDest, x, wr.top, wt, h, mem2dc, dx, 0, wt, h, bf2);
					}
					x = wr.left + w - dx;
					while (x < wr.right - w) {
						ske_AlphaBlend(hDest, x, wr.top, w, h, mem2dc, 0, 0, w, h, bf2);
						x += w;
					}
					if (x <= wr.right)
						ske_AlphaBlend(hDest, x, wr.top, wr.right - x, h, mem2dc, 0, 0, wr.right - x, h, bf2);
				}
			}
		}
		SelectObject(mem2dc, oldbmp);
		DeleteObject(mem2bmp);
		DeleteDC(mem2dc);
	}
	else if (mode == FM_TILE_BOTH && (rGlyph->right - rGlyph->left > 0) && (rGlyph->bottom - rGlyph->top > 0)) {
		int w = rGlyph->right - rGlyph->left;
		int  x = 0;
		int h = rFill->bottom - rFill->top;
		RECT wr;
		IntersectRect(&wr, rClip, rFill);
		if ((wr.bottom - wr.top)*(wr.right - wr.left) == 0) return 0;
		HDC mem2dc = CreateCompatibleDC(hDest);
		HBITMAP mem2bmp = ske_CreateDIB32(w, wr.bottom - wr.top);
		h = wr.bottom - wr.top;
		HBITMAP oldbmp = (HBITMAP)SelectObject(mem2dc, mem2bmp);
#ifdef _DEBUG
		if (!oldbmp)
			(nullptr, "Tile bitmap not selected", "ERROR", MB_OK);
#endif
		/// draw here
		{
			//fill temp bitmap
			{
				int dy = (wr.top - rFill->top) % (rGlyph->bottom - rGlyph->top);
				int y = -dy;
				while (y < wr.bottom - wr.top) {

					ske_AlphaBlend(mem2dc, 0, y, w, rGlyph->bottom - rGlyph->top, hSource, rGlyph->left, rGlyph->top, w, rGlyph->bottom - rGlyph->top, bf);
					y += rGlyph->bottom - rGlyph->top;
				}

				//--
				//end temp bitmap
				if (drawMode == 0 || drawMode == 2) {
					if (drawMode == 0) {
						int dx = (wr.left - rFill->left) % w;
						if (dx >= 0) {
							x = wr.left;
							int wt = (x + w - dx <= wr.right) ? (w - dx) : (wr.right - wr.left);
							BitBlt(hDest, x, wr.top, wt, h, mem2dc, dx, 0, SRCCOPY);
						}
						x = wr.left + w - dx;
						while (x < wr.right - w) {
							BitBlt(hDest, x, wr.top, w, h, mem2dc, 0, 0, SRCCOPY);
							x += w;
						}
						if (x <= wr.right)
							BitBlt(hDest, x, wr.top, wr.right - x, h, mem2dc, 0, 0, SRCCOPY);
					}
					else {
						int dx = (wr.left - rFill->left) % w;
						x = wr.left - dx;
						while (x < wr.right - w) {
							ske_AlphaBlend(hDest, x, wr.top, w, h, mem2dc, 0, 0, w, h, bf);
							x += w;
						}
						if (x <= wr.right)
							ske_AlphaBlend(hDest, x, wr.top, wr.right - x, h, mem2dc, 0, 0, wr.right - x, h, bf);
					}

				}
				else {
					BLENDFUNCTION bf2 = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					int dx = (wr.left - rFill->left) % w;
					if (dx >= 0) {
						x = wr.left;
						int wt = (x + w - dx <= wr.right) ? (w - dx) : (wr.right - wr.left);
						ske_AlphaBlend(hDest, x, wr.top, wt, h, mem2dc, dx, 0, wt, h, bf2);
					}
					x = wr.left + w - dx;
					while (x < wr.right - w) {
						ske_AlphaBlend(hDest, x, wr.top, w, h, mem2dc, 0, 0, w, h, bf2);
						x += w;
					}
					if (x <= wr.right)
						ske_AlphaBlend(hDest, x, wr.top, wr.right - x, h, mem2dc, 0, 0, wr.right - x, h, bf2);
				}
			}
		}
		SelectObject(mem2dc, oldbmp);
		DeleteObject(mem2bmp);
		DeleteDC(mem2dc);
	}
	return 1;

}

HBITMAP ske_CreateDIB32(int cx, int cy)
{
	return ske_CreateDIB32Point(cx, cy, nullptr);
}

HBITMAP ske_CreateDIB32Point(int cx, int cy, void **bits)
{
	if (cx < 0 || cy < 0)
		return nullptr;

	BITMAPINFO RGB32BitsBITMAPINFO = { 0 };
	RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	RGB32BitsBITMAPINFO.bmiHeader.biWidth = cx;//bm.bmWidth;
	RGB32BitsBITMAPINFO.bmiHeader.biHeight = cy;//bm.bmHeight;
	RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
	RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;
	// pointer used for direct Bitmap pixels access

	UINT *ptPixels;
	HBITMAP DirectBitmap = CreateDIBSection(nullptr,
		&RGB32BitsBITMAPINFO,
		DIB_RGB_COLORS,
		(void **)&ptPixels,
		nullptr, 0);
	if ((DirectBitmap == nullptr || ptPixels == nullptr) && cx != 0 && cy != 0) {
#ifdef _DEBUG
		MessageBoxA(nullptr, "Object not allocated. Check GDI object count", "ERROR", MB_OK | MB_ICONERROR);
		DebugBreak();
#endif
		;
	}
	else memset(ptPixels, 0, cx*cy * 4);
	if (bits != nullptr) *bits = ptPixels;
	return DirectBitmap;
}

HRGN ske_CreateOpaqueRgn(uint8_t Level, bool Opaque)
{
	if (!g_pCachedWindow)
		return nullptr;

	RGBQUAD *buf = (RGBQUAD *)g_pCachedWindow->hImageDIBByte;
	if (buf == nullptr)
		return nullptr;

	unsigned int cRect = 64;
	PRGNDATA pRgnData = (PRGNDATA)mir_alloc(sizeof(RGNDATAHEADER) + (cRect)*sizeof(RECT));
	memset(pRgnData, 0, sizeof(RGNDATAHEADER));
	pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
	pRgnData->rdh.iType = RDH_RECTANGLES;

	for (int y = 0; y < g_pCachedWindow->Height; ++y) {
		bool inside = false;
		bool lastin = false;
		unsigned int entry = 0;

		for (int x = 0; x < g_pCachedWindow->Width; ++x) {
			inside = Opaque ? (buf->rgbReserved > Level) : (buf->rgbReserved < Level);
			++buf;

			if (inside != lastin) {
				if (inside) {
					lastin = true;
					entry = x;
				}
				else {
					if (pRgnData->rdh.nCount == cRect) {
						cRect = cRect + 64;
						pRgnData = (PRGNDATA)mir_realloc(pRgnData, sizeof(RGNDATAHEADER) + (cRect)*sizeof(RECT));
					}
					SetRect(((LPRECT)pRgnData->Buffer) + pRgnData->rdh.nCount, entry, g_pCachedWindow->Height - y, x, g_pCachedWindow->Height - y + 1);

					pRgnData->rdh.nCount++;
					lastin = false;
				}
			}
		}

		if (lastin) {
			if (pRgnData->rdh.nCount == cRect) {
				cRect = cRect + 64;
				pRgnData = (PRGNDATA)mir_realloc(pRgnData, sizeof(RGNDATAHEADER) + (cRect)*sizeof(RECT));
			}
			SetRect(((LPRECT)pRgnData->Buffer) + pRgnData->rdh.nCount, entry, g_pCachedWindow->Height - y, g_pCachedWindow->Width, g_pCachedWindow->Height - y + 1);

			pRgnData->rdh.nCount++;
		}
	}

	HRGN hRgn = ExtCreateRegion(nullptr, sizeof(RGNDATAHEADER) + pRgnData->rdh.nCount*sizeof(RECT), (LPRGNDATA)pRgnData);
	mir_free(pRgnData);
	return hRgn;
}

static int ske_DrawSkinObject(SKINDRAWREQUEST *preq, GLYPHOBJECT *pobj)
{
	HDC memdc = nullptr, glyphdc = nullptr;
	int k = 0;
	//BITMAP bmp = {0};
	HBITMAP membmp = nullptr, oldbmp = nullptr, oldglyph = nullptr;
	uint8_t Is32Bit = 0;
	RECT PRect;
	POINT mode2offset = { 0 };
	int depth = 0;
	int mode = 0; //0-FastDraw, 1-DirectAlphaDraw, 2-BufferedAlphaDraw

	if (!(preq && pobj)) return -1;
	if ((!pobj->hGlyph || pobj->hGlyph == (HBITMAP)-1) && ((pobj->Style & 7) == ST_IMAGE || (pobj->Style & 7) == ST_FRAGMENT || (pobj->Style & 7) == ST_SOLARIZE)) return 0;
	// Determine painting mode
	depth = GetDeviceCaps(preq->hDC, BITSPIXEL);
	depth = depth < 16 ? 16 : depth;
	Is32Bit = pobj->bmBitsPixel == 32;
	if ((!Is32Bit && pobj->dwAlpha == 255) && pobj->Style != ST_BRUSH) mode = 0;
	else if (pobj->dwAlpha == 255 && pobj->Style != ST_BRUSH) mode = 1;
	else mode = 2;
	// End painting mode

	//force mode

	if (preq->rcClipRect.bottom - preq->rcClipRect.top*preq->rcClipRect.right - preq->rcClipRect.left == 0)
		preq->rcClipRect = preq->rcDestRect;
	IntersectRect(&PRect, &preq->rcDestRect, &preq->rcClipRect);
	if (IsRectEmpty(&PRect)) {
		return 0;
	}
	if (mode == 2) {
		memdc = CreateCompatibleDC(preq->hDC);
		membmp = ske_CreateDIB32(PRect.right - PRect.left, PRect.bottom - PRect.top);
		oldbmp = (HBITMAP)SelectObject(memdc, membmp);
		if (oldbmp == nullptr) {
			SelectObject(memdc, oldbmp);
			DeleteDC(memdc);
			DeleteObject(membmp);
			return 0;
		}
	}

	if (mode != 2) memdc = preq->hDC;
	{
		if (pobj->hGlyph && pobj->hGlyph != (HBITMAP)-1) {
			glyphdc = CreateCompatibleDC(preq->hDC);
			oldglyph = (HBITMAP)SelectObject(glyphdc, pobj->hGlyph);
		}
		// Drawing
		{
			RECT rFill, rGlyph, rClip;
			if ((pobj->Style & 7) == ST_BRUSH) {
				HBRUSH br = CreateSolidBrush(pobj->dwColor);
				RECT fr;
				if (mode == 2) {
					SetRect(&fr, 0, 0, PRect.right - PRect.left, PRect.bottom - PRect.top);
					FillRect(memdc, &fr, br);
					ske_SetRectOpaque(memdc, &fr);
					// FillRectAlpha(memdc,&fr,pobj->dwColor|0xFF000000);
				}
				else {
					fr = PRect;
					// SetRect(&fr, 0, 0, PRect.right-PRect.left,PRect.bottom-PRect.top);
					FillRect(preq->hDC, &fr, br);
				}
				DeleteObject(br);
				k = -1;
			}
			else {
				if (mode == 2) {
					mode2offset.x = PRect.left;
					mode2offset.y = PRect.top;
					OffsetRect(&PRect, -mode2offset.x, -mode2offset.y);
				}
				rClip = (preq->rcClipRect);

				{
					int lft = 0;
					int top = 0;
					int rgh = pobj->bmWidth;
					int btm = pobj->bmHeight;
					if ((pobj->Style & 7) == ST_FRAGMENT) {
						lft = pobj->clipArea.x;
						top = pobj->clipArea.y;
						rgh = min(rgh, lft + pobj->szclipArea.cx);
						btm = min(btm, top + pobj->szclipArea.cy);
					}

					// Draw center...
					if (1) {
						rFill.top = preq->rcDestRect.top + pobj->dwTop;
						rFill.bottom = preq->rcDestRect.bottom - pobj->dwBottom;
						rFill.left = preq->rcDestRect.left + pobj->dwLeft;
						rFill.right = preq->rcDestRect.right - pobj->dwRight;

						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);

						rGlyph.top = top + pobj->dwTop;
						rGlyph.left = lft + pobj->dwLeft;
						rGlyph.right = rgh - pobj->dwRight;
						rGlyph.bottom = btm - pobj->dwBottom;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, pobj->FitMode, mode);
					}

					// Draw top side...
					if (1) {
						rFill.top = preq->rcDestRect.top;
						rFill.bottom = preq->rcDestRect.top + pobj->dwTop;
						rFill.left = preq->rcDestRect.left + pobj->dwLeft;
						rFill.right = preq->rcDestRect.right - pobj->dwRight;

						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);

						rGlyph.top = top + 0;
						rGlyph.left = lft + pobj->dwLeft;
						rGlyph.right = rgh - pobj->dwRight;
						rGlyph.bottom = top + pobj->dwTop;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, pobj->FitMode & FM_TILE_HORZ, mode);
					}
					// Draw bottom side...
					if (1) {
						rFill.top = preq->rcDestRect.bottom - pobj->dwBottom;
						rFill.bottom = preq->rcDestRect.bottom;
						rFill.left = preq->rcDestRect.left + pobj->dwLeft;
						rFill.right = preq->rcDestRect.right - pobj->dwRight;

						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);


						rGlyph.top = btm - pobj->dwBottom;
						rGlyph.left = lft + pobj->dwLeft;
						rGlyph.right = rgh - pobj->dwRight;
						rGlyph.bottom = btm;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, pobj->FitMode & FM_TILE_HORZ, mode);
					}
					// Draw left side...
					if (1) {
						rFill.top = preq->rcDestRect.top + pobj->dwTop;
						rFill.bottom = preq->rcDestRect.bottom - pobj->dwBottom;
						rFill.left = preq->rcDestRect.left;
						rFill.right = preq->rcDestRect.left + pobj->dwLeft;

						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);


						rGlyph.top = top + pobj->dwTop;
						rGlyph.left = lft;
						rGlyph.right = lft + pobj->dwLeft;
						rGlyph.bottom = btm - pobj->dwBottom;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, pobj->FitMode & FM_TILE_VERT, mode);
					}

					// Draw right side...
					if (1) {
						rFill.top = preq->rcDestRect.top + pobj->dwTop;
						rFill.bottom = preq->rcDestRect.bottom - pobj->dwBottom;
						rFill.left = preq->rcDestRect.right - pobj->dwRight;
						rFill.right = preq->rcDestRect.right;

						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);


						rGlyph.top = top + pobj->dwTop;
						rGlyph.left = rgh - pobj->dwRight;
						rGlyph.right = rgh;
						rGlyph.bottom = btm - pobj->dwBottom;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, pobj->FitMode & FM_TILE_VERT, mode);
					}


					// Draw Top-Left corner...
					if (1) {
						rFill.top = preq->rcDestRect.top;
						rFill.bottom = preq->rcDestRect.top + pobj->dwTop;
						rFill.left = preq->rcDestRect.left;
						rFill.right = preq->rcDestRect.left + pobj->dwLeft;

						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);


						rGlyph.top = top;
						rGlyph.left = lft;
						rGlyph.right = lft + pobj->dwLeft;
						rGlyph.bottom = top + pobj->dwTop;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, 0, mode);
					}
					// Draw Top-Right corner...
					if (1) {
						rFill.top = preq->rcDestRect.top;
						rFill.bottom = preq->rcDestRect.top + pobj->dwTop;
						rFill.left = preq->rcDestRect.right - pobj->dwRight;
						rFill.right = preq->rcDestRect.right;

						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);


						rGlyph.top = top;
						rGlyph.left = rgh - pobj->dwRight;
						rGlyph.right = rgh;
						rGlyph.bottom = top + pobj->dwTop;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, 0, mode);
					}

					// Draw Bottom-Left corner...
					if (1) {
						rFill.top = preq->rcDestRect.bottom - pobj->dwBottom;
						rFill.bottom = preq->rcDestRect.bottom;
						rFill.left = preq->rcDestRect.left;
						rFill.right = preq->rcDestRect.left + pobj->dwLeft;


						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);


						rGlyph.left = lft;
						rGlyph.right = lft + pobj->dwLeft;
						rGlyph.top = btm - pobj->dwBottom;
						rGlyph.bottom = btm;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, 0, mode);
					}
					// Draw Bottom-Right corner...
					if (1) {
						rFill.top = preq->rcDestRect.bottom - pobj->dwBottom;
						rFill.bottom = preq->rcDestRect.bottom;
						rFill.left = preq->rcDestRect.right - pobj->dwRight;
						rFill.right = preq->rcDestRect.right;


						if (mode == 2)
							OffsetRect(&rFill, -mode2offset.x, -mode2offset.y);

						rGlyph.left = rgh - pobj->dwRight;
						rGlyph.right = rgh;
						rGlyph.top = btm - pobj->dwBottom;
						rGlyph.bottom = btm;

						k += ske_SkinFillRectByGlyph(memdc, glyphdc, &rFill, &rGlyph, &PRect, 0, mode);
					}
				}

			}

			if ((k > 0 || k == -1) && mode == 2) {
				{
					BLENDFUNCTION bf = { AC_SRC_OVER, 0, pobj->dwAlpha, uint8_t(pobj->bmBitsPixel == 32 && pobj->Style != ST_BRUSH ? AC_SRC_ALPHA : 0) };
					OffsetRect(&PRect, mode2offset.x, mode2offset.y);
					ske_AlphaBlend(preq->hDC, PRect.left, PRect.top, PRect.right - PRect.left, PRect.bottom - PRect.top,
						memdc, 0, 0, PRect.right - PRect.left, PRect.bottom - PRect.top, bf);
				}
			}
		}
		//free GDI resources
		//--++--

		//free GDI resources
		{

			if (oldglyph) SelectObject(glyphdc, oldglyph);
			if (glyphdc) DeleteDC(glyphdc);
		}
		if (mode == 2) {
			SelectObject(memdc, oldbmp);
			DeleteDC(memdc);
			DeleteObject(membmp);
		}

	}
	if (pobj->plTextList && pobj->plTextList->realCount > 0) {
		HFONT hOldFont;
		for (int i = 0; i < pobj->plTextList->realCount; i++) {
			GLYPHTEXT *gt = (GLYPHTEXT *)pobj->plTextList->items[i];
			if (!gt->hFont) {
				if (gl_plSkinFonts && gl_plSkinFonts->realCount > 0) {
					int j = 0;
					for (j = 0; j < gl_plSkinFonts->realCount; j++) {
						SKINFONT *sf = (SKINFONT*)gl_plSkinFonts->items[j];
						if (sf->szFontID && !mir_strcmp(sf->szFontID, gt->szFontID)) {
							gt->hFont = sf->hFont;
							break;
						}
					}
				}
				if (!gt->hFont) gt->hFont = (HFONT)-1;
			}
			if (gt->hFont != (HFONT)-1) {
				RECT rc = { 0 };
				hOldFont = (HFONT)SelectObject(preq->hDC, gt->hFont);



				if (gt->RelativeFlags & 2) rc.left = preq->rcDestRect.right + gt->iLeft;
				else if (gt->RelativeFlags & 1) rc.left = ((preq->rcDestRect.right - preq->rcDestRect.left) >> 1) + gt->iLeft;
				else rc.left = preq->rcDestRect.left + gt->iLeft;

				if (gt->RelativeFlags & 8) rc.top = preq->rcDestRect.bottom + gt->iTop;
				else if (gt->RelativeFlags & 4) rc.top = ((preq->rcDestRect.bottom - preq->rcDestRect.top) >> 1) + gt->iTop;
				else rc.top = preq->rcDestRect.top + gt->iTop;

				if (gt->RelativeFlags & 32) rc.right = preq->rcDestRect.right + gt->iRight;
				else if (gt->RelativeFlags & 16) rc.right = ((preq->rcDestRect.right - preq->rcDestRect.left) >> 1) + gt->iRight;
				else rc.right = preq->rcDestRect.left + gt->iRight;

				if (gt->RelativeFlags & 128) rc.bottom = preq->rcDestRect.bottom + gt->iBottom;
				else if (gt->RelativeFlags & 64) rc.bottom = ((preq->rcDestRect.bottom - preq->rcDestRect.top) >> 1) + gt->iBottom;
				else rc.bottom = preq->rcDestRect.top + gt->iBottom;

				ske_AlphaTextOut(preq->hDC, gt->stText, -1, &rc, gt->dwFlags, gt->dwColor);
				SelectObject(preq->hDC, hOldFont);
			}
		}
	}

	return 0;
}



int ske_AddDescriptorToSkinObjectList(SKINOBJECTDESCRIPTOR *lpDescr, SKINOBJECTSLIST *Skin)
{
	SKINOBJECTSLIST *sk = (Skin ? Skin : &g_SkinObjectList);
	if (!mir_strcmpi(lpDescr->szObjectID, "_HEADER_"))
		return 0;
	//check if new object allready presents.
	for (uint32_t i = 0; i < sk->dwObjLPAlocated; i++)
		if (!mir_strcmp(sk->pObjects[i].szObjectID, lpDescr->szObjectID))
			return 0;
	// Realocated list to add space for new object
	if (sk->dwObjLPAlocated + 1 > sk->dwObjLPReserved) {
		sk->pObjects = (SKINOBJECTDESCRIPTOR*)mir_realloc(sk->pObjects, sizeof(SKINOBJECTDESCRIPTOR)*(sk->dwObjLPReserved + 1)/*alloc step*/);
		sk->dwObjLPReserved++;
	}
	{ //filling new objects field
		sk->pObjects[sk->dwObjLPAlocated].bType = lpDescr->bType;
		sk->pObjects[sk->dwObjLPAlocated].Data = nullptr;
		sk->pObjects[sk->dwObjLPAlocated].szObjectID = mir_strdup(lpDescr->szObjectID);
		//  sk->Objects[sk->dwObjLPAlocated].szObjectName = mir_strdup(lpDescr->szObjectName);
		if (lpDescr->Data != nullptr) {   //Copy defaults values
			switch (lpDescr->bType) {
			case OT_GLYPHOBJECT:
				{
					GLYPHOBJECT *gl = (GLYPHOBJECT *)lpDescr->Data;
					sk->pObjects[sk->dwObjLPAlocated].Data = mir_alloc(sizeof(GLYPHOBJECT));
					GLYPHOBJECT *obdat = (GLYPHOBJECT *)sk->pObjects[sk->dwObjLPAlocated].Data;
					memcpy(obdat, gl, sizeof(GLYPHOBJECT));
					if (gl->szFileName != nullptr) {
						obdat->szFileName = mir_strdup(gl->szFileName);
						replaceStr(gl->szFileName, nullptr);
					}
					else obdat->szFileName = nullptr;

					obdat->hGlyph = nullptr;
					break;
				}
			}

		}
	}
	sk->dwObjLPAlocated++;
	return 1;
}

static SKINOBJECTDESCRIPTOR* ske_FindObject(const char *szName, SKINOBJECTSLIST *Skin)
{
	SKINOBJECTSLIST *sk = (Skin == nullptr) ? (&g_SkinObjectList) : Skin;
	return skin_FindObjectByRequest((char *)szName, sk->pMaskList);
}

static SKINOBJECTDESCRIPTOR* ske_FindObjectByMask(MODERNMASK *pModernMask, SKINOBJECTSLIST *Skin)
{
	SKINOBJECTSLIST *sk = (Skin == nullptr) ? (&g_SkinObjectList) : Skin;
	return sk->pMaskList ? skin_FindObjectByMask(pModernMask, sk->pMaskList) : nullptr;
}

SKINOBJECTDESCRIPTOR* ske_FindObjectByName(const char *szName, uint8_t objType, SKINOBJECTSLIST *Skin)
{
	SKINOBJECTSLIST *sk = (Skin == nullptr) ? (&g_SkinObjectList) : Skin;
	for (uint32_t i = 0; i < sk->dwObjLPAlocated; i++) {
		if (sk->pObjects[i].bType == objType || objType == OT_ANY) {
			if (!mir_strcmp(sk->pObjects[i].szObjectID, szName))
				return &(sk->pObjects[i]);
		}
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Paint glyph
// wParam - LPSKINDRAWREQUEST
// lParam - possible direct pointer to modern mask
//////////////////////////////////////////////////////////////////////////

INT_PTR ske_Service_DrawGlyph(WPARAM wParam, LPARAM lParam)
{
	auto *preq = (SKINDRAWREQUEST *)wParam;
	if (preq == nullptr)
		return -1;

	mir_cslock lck(cs_SkinChanging);

	SKINOBJECTDESCRIPTOR *pgl = (lParam ? ske_FindObjectByMask((MODERNMASK*)lParam, nullptr) : ske_FindObject(preq->szObjectID, nullptr));
	if (pgl == nullptr) return -1;
	if (pgl->Data == nullptr) return -1;

	GLYPHOBJECT *gl = (GLYPHOBJECT*)pgl->Data;
	int iStyle = gl->Style & 7;
	if (iStyle == ST_SKIP)
		return ST_SKIP;

	if (gl->hGlyph == nullptr && gl->hGlyph != (HBITMAP)-1 && (iStyle == ST_IMAGE || iStyle == ST_FRAGMENT || iStyle == ST_SOLARIZE)) {
		if (gl->szFileName) {
			gl->hGlyph = ske_LoadGlyphImage(_A2T(gl->szFileName));
			if (gl->hGlyph) {
				BITMAP bmp = { 0 };
				GetObject(gl->hGlyph, sizeof(BITMAP), &bmp);
				gl->bmBitsPixel = (uint8_t)bmp.bmBitsPixel;
				gl->bmHeight = bmp.bmHeight;
				gl->bmWidth = bmp.bmWidth;
			}
			else gl->hGlyph = (HBITMAP)-1; //invalid
		}
	}
	return ske_DrawSkinObject(preq, gl);
}


void ske_PreMultiplyChannels(HBITMAP hbmp, uint8_t Mult)
{
	BITMAP bmp;
	BOOL flag = FALSE;
	uint8_t *pBitmapBits;
	uint32_t Len;
	int bh, bw, y, x;

	GetObject(hbmp, sizeof(BITMAP), (LPSTR)&bmp);
	bh = bmp.bmHeight;
	bw = bmp.bmWidth;
	Len = bh * bw * 4;
	flag = (bmp.bmBits == nullptr);
	if (flag) {
		pBitmapBits = (LPBYTE)mir_alloc(Len);
		GetBitmapBits(hbmp, Len, pBitmapBits);
	}
	else
		pBitmapBits = (uint8_t *)bmp.bmBits;
	for (y = 0; y < bh; ++y) {
		uint8_t *pPixel = pBitmapBits + bw * 4 * y;

		for (x = 0; x < bw; ++x) {
			if (Mult) {
				pPixel[0] = pPixel[0] * pPixel[3] / 255;
				pPixel[1] = pPixel[1] * pPixel[3] / 255;
				pPixel[2] = pPixel[2] * pPixel[3] / 255;
			}
			else {
				pPixel[3] = 255;
			}
			pPixel += 4;
		}
	}
	if (flag) {
		Len = SetBitmapBits(hbmp, Len, pBitmapBits);
		mir_free(pBitmapBits);
	}
	return;
}

int ske_GetFullFilename(wchar_t *buf, const wchar_t *file, wchar_t *skinfolder, BOOL madeAbsolute)
{
	wchar_t *SkinPlace = db_get_wsa(0, SKIN, "SkinFolder");
	if (SkinPlace == nullptr)
		SkinPlace = mir_wstrdup(L"\\Skin\\default");

	wchar_t b2[MAX_PATH];
	if (file[0] != '\\' && file[1] != ':')
		mir_snwprintf(b2, L"%s\\%s", (skinfolder == nullptr) ? SkinPlace : ((INT_PTR)skinfolder != -1) ? skinfolder : L"", file);
	else
		mir_wstrncpy(b2, file, _countof(b2));

	if (madeAbsolute) {
		if (b2[0] == '\\' && b2[1] != '\\')
			PathToAbsoluteW(b2 + 1, buf);
		else
			PathToAbsoluteW(b2, buf);
	}
	else mir_wstrncpy(buf, b2, MAX_PATH);

	mir_free(SkinPlace);
	return 0;
}

/*
This function is required to load TGA to dib buffer myself
Major part of routines is from http://tfcduke.developpez.com/tutoriel/format/tga/fichiers/tga.c
*/

static BOOL ske_ReadTGAImageData(void *From, uint32_t fromSize, uint8_t *destBuf, uint32_t bufSize, BOOL RLE)
{
	uint8_t *pos = destBuf;
	uint8_t *from = fromSize ? (uint8_t *)From : nullptr;
	FILE *fp = !fromSize ? (FILE *)From : nullptr;
	uint32_t destCount = 0;
	uint32_t fromCount = 0;
	if (!RLE) {
		while (((from && fromCount < fromSize) || (fp && fromCount < bufSize))
			&& (destCount < bufSize)) {
			uint8_t r = from ? from[fromCount++] : (uint8_t)fgetc(fp);
			uint8_t g = from ? from[fromCount++] : (uint8_t)fgetc(fp);
			uint8_t b = from ? from[fromCount++] : (uint8_t)fgetc(fp);
			uint8_t a = from ? from[fromCount++] : (uint8_t)fgetc(fp);
			pos[destCount++] = r;
			pos[destCount++] = g;
			pos[destCount++] = b;
			pos[destCount++] = a;

			if (destCount > bufSize) break;
			if (from) 	if (fromCount < fromSize) break;
		}
	}
	else {
		uint8_t rgba[4];
		uint8_t packet_header;
		uint8_t *ptr = pos;
		uint8_t size;
		int i;
		while (ptr < pos + bufSize) {
			/* read first byte */
			packet_header = from ? from[fromCount] : (uint8_t)fgetc(fp);
			if (from) from++;
			size = 1 + (packet_header & 0x7f);
			if (packet_header & 0x80) {
				/* run-length packet */
				if (from) {
					*((uint32_t*)rgba) = *((uint32_t*)(from + fromCount));
					fromCount += 4;
				}
				else fread(rgba, sizeof(uint8_t), 4, fp);
				for (i = 0; i < size; ++i, ptr += 4) {
					ptr[2] = rgba[2];
					ptr[1] = rgba[1];
					ptr[0] = rgba[0];
					ptr[3] = rgba[3];
				}
			}
			else {	/* not run-length packet */
				for (i = 0; i < size; ++i, ptr += 4) {
					ptr[0] = from ? from[fromCount++] : (uint8_t)fgetc(fp);
					ptr[1] = from ? from[fromCount++] : (uint8_t)fgetc(fp);
					ptr[2] = from ? from[fromCount++] : (uint8_t)fgetc(fp);
					ptr[3] = from ? from[fromCount++] : (uint8_t)fgetc(fp);
				}
			}
		}
	}
	return TRUE;
}

static HBITMAP ske_LoadGlyphImage_TGA(const wchar_t *szFilename)
{
	uint8_t *colormap = nullptr;
	int cx = 0, cy = 0;
	BOOL err = FALSE;
	tga_header_t header;
	if (!szFilename) return nullptr;
	if (!wildcmpiw(szFilename, L"*\\*%.tga")) {
		//Loading TGA image from file
		FILE *fp = _wfopen(szFilename, L"rb");
		if (!fp) {
			TRACEVAR("error: couldn't open \"%s\"!\n", szFilename);
			return nullptr;
		}
		/* read header */
		fread(&header, sizeof(tga_header_t), 1, fp);
		if ((header.pixel_depth != 32) || ((header.image_type != 10) && (header.image_type != 2))) {
			fclose(fp);
			return nullptr;
		}

		/*memory allocation */
		colormap = (uint8_t*)mir_alloc(header.width*header.height * 4);
		cx = header.width;
		cy = header.height;
		fseek(fp, header.id_lenght, SEEK_CUR);
		fseek(fp, header.cm_length, SEEK_CUR);
		err = !ske_ReadTGAImageData((void*)fp, 0, colormap, header.width*header.height * 4, header.image_type == 10);
		fclose(fp);
	}
	else {
		/* reading from resources IDR_TGA_DEFAULT_SKIN */
		HRSRC hRSrc = FindResourceA(g_plugin.getInst(), MAKEINTRESOURCEA(IDR_TGA_DEFAULT_SKIN), "TGA");
		if (!hRSrc) return nullptr;
		HGLOBAL hRes = LoadResource(g_plugin.getInst(), hRSrc);
		if (!hRes) return nullptr;
		uint32_t size = SizeofResource(g_plugin.getInst(), hRSrc);
		uint8_t *mem = (uint8_t*)LockResource(hRes);
		if (size > sizeof(header)) {
			tga_header_t *tgahdr = (tga_header_t*)mem;
			if (tgahdr->pixel_depth == 32 && (tgahdr->image_type == 2 || tgahdr->image_type == 10)) {
				colormap = (uint8_t*)mir_alloc(tgahdr->width*tgahdr->height * 4);
				cx = tgahdr->width;
				cy = tgahdr->height;
				ske_ReadTGAImageData((void*)(mem + sizeof(tga_header_t) + tgahdr->id_lenght + tgahdr->cm_length), size - (sizeof(tga_header_t) + tgahdr->id_lenght + tgahdr->cm_length), colormap, cx*cy * 4, tgahdr->image_type == 10);
			}
		}
		FreeResource(hRes);
	}

	if (colormap == nullptr)
		return nullptr;

	// create dib section
	uint8_t *pt;
	HBITMAP hbmp = ske_CreateDIB32Point(cx, cy, (void**)&pt);
	if (hbmp)
		memcpy(pt, colormap, cx*cy * 4);
	mir_free(colormap);
	return hbmp;
}

static HBITMAP ske_LoadGlyphImageByDecoders(const wchar_t *tszFileName)
{
	if (!wcschr(tszFileName, '%') && !PathFileExists(tszFileName))
		return nullptr;

	const wchar_t *ext = wcsrchr(tszFileName, '.');
	if (ext == nullptr)
		return nullptr;

	BITMAP bmpInfo;
	HBITMAP hBitmap;
	bool f = false;

	if (!mir_wstrcmpi(ext, L".tga")) {
		hBitmap = ske_LoadGlyphImage_TGA(tszFileName);
		f = true;
	}
	else hBitmap = Bitmap_Load(tszFileName);

	if (hBitmap == nullptr)
		return nullptr;

	GetObject(hBitmap, sizeof(BITMAP), &bmpInfo);
	if (bmpInfo.bmBitsPixel == 32)
		ske_PreMultiplyChannels(hBitmap, f);
	else {
		HDC dc32 = CreateCompatibleDC(nullptr);
		HDC dc24 = CreateCompatibleDC(nullptr);
		HBITMAP hBitmap32 = ske_CreateDIB32(bmpInfo.bmWidth, bmpInfo.bmHeight);
		HBITMAP obmp24 = (HBITMAP)SelectObject(dc24, hBitmap);
		HBITMAP obmp32 = (HBITMAP)SelectObject(dc32, hBitmap32);
		BitBlt(dc32, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, dc24, 0, 0, SRCCOPY);
		SelectObject(dc24, obmp24);
		SelectObject(dc32, obmp32);
		DeleteDC(dc24);
		DeleteDC(dc32);
		DeleteObject(hBitmap);
		hBitmap = hBitmap32;
		ske_PreMultiplyChannels(hBitmap, 0);
	}
	return hBitmap;
}

static HBITMAP ske_skinLoadGlyphImage(const wchar_t *tszFileName)
{
	if (!wildcmpiw(tszFileName, L"*.tga"))
		return GDIPlus_LoadGlyphImage(tszFileName);

	return ske_LoadGlyphImageByDecoders(tszFileName);
}

HBITMAP ske_LoadGlyphImage(const wchar_t *tszFileName)
{
	// try to find image in loaded
	wchar_t szFile[MAX_PATH] = { 0 };
	ske_GetFullFilename(szFile, tszFileName, g_SkinObjectList.szSkinPlace, TRUE);

	mir_cslock lck(cs_SkinChanging);

	if (pLoadedImages) {
		for (uint32_t i = 0; i < dwLoadedImagesCount; i++) {
			if (!mir_wstrcmpi(pLoadedImages[i].szFileName, szFile)) {
				pLoadedImages[i].dwLoadedTimes++;
				return pLoadedImages[i].hGlyph;
			}
		}
	}

	// load new image
	HBITMAP hbmp = ske_skinLoadGlyphImage(szFile);
	if (hbmp == nullptr)
		return nullptr;

	// add to loaded list
	if (dwLoadedImagesCount + 1 > dwLoadedImagesAlocated) {
		pLoadedImages = (GLYPHIMAGE*)mir_realloc(pLoadedImages, sizeof(GLYPHIMAGE)*(dwLoadedImagesCount + 1));
		if (!pLoadedImages)
			return nullptr;
		dwLoadedImagesAlocated++;
	}

	pLoadedImages[dwLoadedImagesCount].dwLoadedTimes = 1;
	pLoadedImages[dwLoadedImagesCount].hGlyph = hbmp;
	pLoadedImages[dwLoadedImagesCount].szFileName = mir_wstrdup(szFile);
	dwLoadedImagesCount++;
	return hbmp;
}

int ske_UnloadGlyphImage(HBITMAP hbmp)
{
	for (uint32_t i = 0; i < dwLoadedImagesCount && pLoadedImages; i++) {
		if (hbmp != pLoadedImages[i].hGlyph)
			continue;

		pLoadedImages[i].dwLoadedTimes--;
		if (pLoadedImages[i].dwLoadedTimes == 0) {
			LPGLYPHIMAGE gl = &(pLoadedImages[i]);
			replaceStrW(gl->szFileName, nullptr);
			memmove(&(pLoadedImages[i]), &(pLoadedImages[i + 1]), sizeof(GLYPHIMAGE) * (dwLoadedImagesCount - i - 1));
			dwLoadedImagesCount--;
			DeleteObject(hbmp);
			if (dwLoadedImagesCount == 0) {
				dwLoadedImagesAlocated = 0;
				mir_free_and_nil(pLoadedImages);
			}
		}
		return 0;
	}
	DeleteObject(hbmp);
	return 0;
}

int ske_UnloadSkin(SKINOBJECTSLIST *Skin)
{
	ClearMaskList(Skin->pMaskList);

	//clear font list
	if (gl_plSkinFonts && gl_plSkinFonts->realCount > 0) {
		for (int i = 0; i < gl_plSkinFonts->realCount; i++) {
			SKINFONT *sf = (SKINFONT *)gl_plSkinFonts->items[i];
			if (sf) {
				mir_free(sf->szFontID);
				DeleteObject(sf->hFont);
				mir_free(sf);
			}
		}
		List_Destroy(gl_plSkinFonts);
		mir_free_and_nil(gl_plSkinFonts);
	}

	replaceStrW(Skin->szSkinPlace, nullptr);
	if (Skin->pTextList) List_Destroy(Skin->pTextList);
	mir_free_and_nil(Skin->pTextList);
	ModernSkinButtonDeleteAll();
	if (Skin->dwObjLPAlocated == 0)
		return 0;

	for (uint32_t i = 0; i < Skin->dwObjLPAlocated; i++) {
		switch (Skin->pObjects[i].bType) {
		case OT_GLYPHOBJECT:
			GLYPHOBJECT *dt = (GLYPHOBJECT*)Skin->pObjects[i].Data;
			if (dt->hGlyph && dt->hGlyph != (HBITMAP)-1)
				ske_UnloadGlyphImage(dt->hGlyph);
			dt->hGlyph = nullptr;
			replaceStr(dt->szFileName, nullptr);

			if (dt->plTextList && dt->plTextList->realCount > 0) {
				for (int k = 0; k < dt->plTextList->realCount; k++) {
					GLYPHTEXT *gt = (GLYPHTEXT *)dt->plTextList->items[k];
					if (gt) {
						mir_free(gt->stText);
						mir_free(gt->stValueText);
						mir_free(gt->szFontID);
						mir_free(gt->szGlyphTextID);
						mir_free(gt);
					}
				}
				List_Destroy(dt->plTextList);
				mir_free(dt->plTextList);
			}
			mir_free(dt);
			break;
		}
		replaceStr(Skin->pObjects[i].szObjectID, nullptr);
	}
	mir_free_and_nil(Skin->pObjects);
	Skin->pTextList = nullptr;
	Skin->dwObjLPAlocated = 0;
	Skin->dwObjLPReserved = 0;
	return 0;
}

static void RegisterMaskByParce(const char *szSetting, char *szValue, SKINOBJECTSLIST *pSkin)
{
	size_t i, val_len = mir_strlen(szValue);

	for (i = 0; i < val_len; i++)
		if (szValue[i] == ':')
			break;

	if (i < val_len) {
		char *Obj, *Mask;
		int res;
		uint32_t ID = atoi(szSetting + 1);
		Mask = szValue + i + 1;
		Obj = (char*)mir_alloc(i + 2);
		mir_strncpy(Obj, szValue, i + 1);
		Obj[i + 1] = '\0';
		res = AddStrModernMaskToList(ID, Mask, Obj, pSkin->pMaskList);
		mir_free(Obj);
	}
}

static int ske_ProcessLoadindString(const char *szSetting, char *szValue)
{
	if (!pCurrentSkin) return 0;
	if (szSetting[0] == '$')
		RegisterObjectByParce((char *)szSetting, szValue);
	else if (szSetting[0] == '#')
		RegisterButtonByParce((char *)szSetting, szValue);
	else if (szSetting[0] == '@')
		RegisterMaskByParce((char *)szSetting, szValue, pCurrentSkin); ///
	else if (szSetting[0] == 't')
		ske_AddParseTextGlyphObject((char*)szSetting, szValue, pCurrentSkin);
	else if (szSetting[0] == 'f')
		ske_AddParseSkinFont((char*)szSetting, szValue);
	else return 0;
	return 1;
}

static int ske_enumdb_SkinObjectsProc(const char *szSetting, void *)
{
	ptrA value(db_get_sa(0, SKIN, szSetting));
	ske_ProcessLoadindString(szSetting, value);
	return 0;
}

static int ske_SortTextGlyphObjectFunc(void *first, void *second)
{
	GLYPHTEXT *p1 = *(GLYPHTEXT**)first, *p2 = *(GLYPHTEXT**)second;
	return mir_strcmp(p1->szGlyphTextID, p2->szGlyphTextID);
}

static void ske_LinkSkinObjects(SKINOBJECTSLIST *pObjectList)
{
	// LINK Mask with objects
	for (uint32_t i = 0; i < pObjectList->pMaskList->dwMaskCnt; i++) {
		MODERNMASK *mm = &(pObjectList->pMaskList->pl_Masks[i]);
		void *pObject = (void *)ske_FindObjectByName(mm->szObjectName, OT_ANY, (SKINOBJECTSLIST *)pObjectList);
		replaceStr(mm->szObjectName, nullptr);
		mm->bObjectFound = TRUE;
		mm->pObject = pObject;
	}

	if (pObjectList->pTextList) {
		// LINK Text with objects
		for (int i = 0; i < pObjectList->pTextList->realCount; i++) {
			GLYPHTEXT *glText = (GLYPHTEXT *)pObjectList->pTextList->items[i];
			SKINOBJECTDESCRIPTOR *lpobj = ske_FindObjectByName(glText->szObjectName, OT_GLYPHOBJECT, pObjectList);
			replaceStr(glText->szObjectName, nullptr);
			GLYPHOBJECT *globj = nullptr;
			if (lpobj)
				globj = (GLYPHOBJECT*)lpobj->Data;
			if (globj) {
				if (!globj->plTextList) {
					globj->plTextList = List_Create(0, 1);
					globj->plTextList->sortFunc = ske_SortTextGlyphObjectFunc;
				}
				List_Insert(globj->plTextList, (void*)glText, globj->plTextList->realCount);
				qsort(globj->plTextList->items, globj->plTextList->realCount, sizeof(GLYPHTEXT*), (int(*)(const void*, const void*))globj->plTextList->sortFunc);
				pObjectList->pTextList->items[i] = nullptr;
			}
			else {
				GLYPHTEXT *gt = glText;
				if (gt) {
					mir_free(gt->stText);
					mir_free(gt->stValueText);
					mir_free(gt->szFontID);
					mir_free(gt->szGlyphTextID);
					mir_free(gt);
				}
			}
		}
		List_Destroy(pObjectList->pTextList);
		mir_free_and_nil(pObjectList->pTextList);
	}
}

// Getting skin objects and masks from DB

static int ske_GetSkinFromDB(char *, SKINOBJECTSLIST *Skin)
{
	if (Skin == nullptr) return 0;
	ske_UnloadSkin(Skin);
	g_CluiData.fDisableSkinEngine = db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT);
	// window borders
	if (g_CluiData.fDisableSkinEngine) {
		g_CluiData.LeftClientMargin = 0;
		g_CluiData.RightClientMargin = 0;
		g_CluiData.TopClientMargin = 0;
		g_CluiData.BottomClientMargin = 0;
		return 0;
	}

	// window borders
	g_CluiData.LeftClientMargin = (int)db_get_b(0, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT);
	g_CluiData.RightClientMargin = (int)db_get_b(0, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT);
	g_CluiData.TopClientMargin = (int)db_get_b(0, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT);
	g_CluiData.BottomClientMargin = (int)db_get_b(0, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT);

	Skin->pMaskList = (LISTMODERNMASK*)mir_alloc(sizeof(LISTMODERNMASK));
	memset(Skin->pMaskList, 0, sizeof(LISTMODERNMASK));
	Skin->szSkinPlace = db_get_wsa(0, SKIN, "SkinFolder");
	if (!Skin->szSkinPlace || (wcschr(Skin->szSkinPlace, '%') && !db_get_b(0, SKIN, "Modified", 0))) {
		BOOL bOnlyObjects = FALSE;
		if (Skin->szSkinPlace && wcschr(Skin->szSkinPlace, '%'))
			bOnlyObjects = TRUE;
		mir_free(Skin->szSkinPlace);
		Skin->szSkinPlace = mir_wstrdup(L"%Default%");
		ske_LoadSkinFromResource(bOnlyObjects);
	}

	// Load objects
	pCurrentSkin = Skin;
	db_enum_settings(0, ske_enumdb_SkinObjectsProc, SKIN);

	SortMaskList(pCurrentSkin->pMaskList);
	ske_LinkSkinObjects(pCurrentSkin);

	// Load Masks
	return 0;
}

// surrogate to be called from outside
void ske_LoadSkinFromDB(void)
{
	ske_GetSkinFromDB(SKIN, &g_SkinObjectList);
	g_CluiData.dwKeyColor = db_get_dw(0, "ModernSettings", "KeyColor", (uint32_t)SETTING_KEYCOLOR_DEFAULT);
}

static int ske_LoadSkinFromResource(BOOL bOnlyObjects)
{
	IniParser parser(g_plugin.getInst(), MAKEINTRESOURCEA(IDR_MSF_DEFAULT_SKIN), "MSF", bOnlyObjects ? IniParser::FLAG_ONLY_OBJECTS : IniParser::FLAG_WITH_SETTINGS);
	if (parser.CheckOK()) {
		db_delete_module(0, "ModernSkin");
		db_set_s(0, SKIN, "SkinFolder", "%Default%");
		db_set_s(0, SKIN, "SkinFile", "%Default%");
		parser.Parse(IniParser::WriteStrToDb, 0);
	}
	return 0;
}

// Load data from ini file
int ske_LoadSkinFromIniFile(wchar_t *szFileName, BOOL bOnlyObjects)
{
	if (wcschr(szFileName, '%'))
		return ske_LoadSkinFromResource(bOnlyObjects);

	IniParser parser(szFileName, bOnlyObjects ? IniParser::FLAG_ONLY_OBJECTS : IniParser::FLAG_WITH_SETTINGS);
	if (!parser.CheckOK())
		return 0;

	db_delete_module(0, "ModernSkin");

	wchar_t skinFolder[MAX_PATH], skinFile[MAX_PATH];
	IniParser::GetSkinFolder(szFileName, skinFolder);
	PathToRelativeW(szFileName, skinFile);

	db_set_ws(0, SKIN, "SkinFolder", skinFolder);
	db_set_ws(0, SKIN, "SkinFile", skinFile);

	parser.Parse(IniParser::WriteStrToDb, 1);
	return 0;
}

BOOL ske_TextOut(HDC hdc, int x, int y, LPCTSTR lpString, int nCount)
{
	SIZE sz;
	GetTextExtentPoint32(hdc, lpString, nCount, &sz);

	RECT rc = { 0 };
	SetRect(&rc, x, y, x + sz.cx, y + sz.cy);
	ske_DrawText(hdc, lpString, nCount, &rc, DT_NOCLIP | DT_SINGLELINE | DT_LEFT);
	return 1;
}

static INT_PTR ske_Service_AlphaTextOut(WPARAM wParam, LPARAM)
{
	if (!wParam) return 0;

	AlphaTextOutParams ap = *(AlphaTextOutParams*)wParam;
	return ske_AlphaTextOut(ap.hDC, ap.lpString, ap.nCount, ap.lpRect, ap.format, ap.ARGBcolor);
}

static __inline void ske_SetMatrix(sbyte *matrix,
	sbyte a, sbyte b, sbyte c,
	sbyte d, sbyte e, sbyte f,
	sbyte g, sbyte h, sbyte i)
{
	matrix[0] = a;	matrix[1] = b;	matrix[2] = c;
	matrix[3] = d;	matrix[4] = e;	matrix[5] = f;
	matrix[6] = g;	matrix[7] = h;	matrix[8] = i;
}

bool ske_ResetTextEffect(HDC hdc)
{
	int idx = arEffectStack.getIndex((EFFECTSSTACKITEM*)&hdc);
	if (idx == -1)
		return false;

	mir_free(arEffectStack[idx]);
	arEffectStack.remove(idx);
	return true;
}

bool ske_SelectTextEffect(HDC hdc, uint8_t EffectID, uint32_t FirstColor, uint32_t SecondColor)
{
	if (EffectID > MAXPREDEFINEDEFFECTS)
		return false;

	if (EffectID == -1)
		return ske_ResetTextEffect(hdc);

	EFFECTSSTACKITEM *effect = arEffectStack.find((EFFECTSSTACKITEM*)&hdc);
	if (effect == nullptr) {
		effect = (EFFECTSSTACKITEM *)mir_alloc(sizeof(EFFECTSSTACKITEM));
		effect->hdc = hdc;
		arEffectStack.insert(effect);
	}

	effect->EffectID = EffectID;
	effect->FirstColor = FirstColor;
	effect->SecondColor = SecondColor;
	return true;
}

static bool ske_GetTextEffect(HDC hdc, MODERNEFFECT *modernEffect)
{
	if (!modernEffect)
		return false;

	EFFECTSSTACKITEM *effect = arEffectStack.find((EFFECTSSTACKITEM*)&hdc);
	if (effect == nullptr)
		return false;

	modernEffect->EffectID = effect->EffectID;
	modernEffect->EffectColor1 = effect->FirstColor;
	modernEffect->EffectColor2 = effect->SecondColor;
	modernEffect->EffectMatrix = ModernEffectsEnum[effect->EffectID];
	return true;
}

static bool ske_DrawTextEffect(uint8_t *destPt, uint8_t *maskPt, uint32_t width, uint32_t height, MODERNEFFECT *effect)
{
	sbyte *buf;
	sbyte *outbuf;
	sbyte *bufline, *buflineTop, *buflineMid;
	int sign = 0;
	uint8_t *maskline, *destline;
	uint8_t al, rl, gl, bl, ad, rd, gd, bd;
	int k = 0;
	uint32_t x, y;
	sbyte *matrix;
	uint8_t mcTopStart;
	uint8_t mcBottomEnd;
	uint8_t mcLeftStart;
	uint8_t mcRightEnd;
	uint8_t effectCount;
	int minX = width;
	int maxX = 0;
	int minY = height;
	int maxY = 0;
	if (effect->EffectID == 0xFF) return false;
	if (!width || !height) return false;
	if (!destPt) return false;
	buf = (sbyte*)mir_alloc(width*height*sizeof(uint8_t));
	{
		matrix = effect->EffectMatrix.matrix;
		mcTopStart = 2 - effect->EffectMatrix.topEffect;
		mcBottomEnd = 3 + effect->EffectMatrix.bottomEffect;
		mcLeftStart = 2 - effect->EffectMatrix.leftEffect;
		mcRightEnd = 3 + effect->EffectMatrix.rightEffect;
		effectCount = effect->EffectMatrix.cycleCount;
	}
	al = 255 - ((uint8_t)(effect->EffectColor1 >> 24));
	rl = GetRValue(effect->EffectColor1);
	gl = GetGValue(effect->EffectColor1);
	bl = GetBValue(effect->EffectColor1);
	ad = 255 - ((uint8_t)(effect->EffectColor2 >> 24));
	rd = GetRValue(effect->EffectColor2);
	gd = GetGValue(effect->EffectColor2);
	bd = GetBValue(effect->EffectColor2);

	// Fill buffer by mid values of image
	for (y = 0; y < height; y++) {
		bufline = buf + y*width;
		maskline = maskPt + ((y*width) << 2);
		for (x = 0; x < width; x++) {
			uint8_t a = (sbyte)(uint32_t)((maskline[0] + maskline[2] + maskline[1] + maskline[1]) >> 4);
			*bufline = a;
			if (a != 0) {
				minX = min((int)x, minX);
				minY = min((int)y, minY);
				maxX = max((int)x, maxX);
				maxY = max((int)y, maxY);
			}
			bufline++;
			maskline += 4;
		}
	}
	// Here perform effect on buffer and place results to outbuf
	for (k = 0; k < (effectCount & 0x7F); k++) {
		minX = max(0, minX + mcLeftStart - 2);
		minY = max(0, minY + mcTopStart - 2);
		maxX = min((int)width, maxX + mcRightEnd - 1);
		maxY = min((int)height, maxY + mcBottomEnd - 1);

		outbuf = (sbyte*)mir_alloc(width*height*sizeof(sbyte));
		memset(outbuf, 0, width*height*sizeof(sbyte));
		for (y = (uint32_t)minY; y < (uint32_t)maxY; y++) {
			int val;
			bufline = outbuf + y*width + minX;
			buflineMid = buf + y*width + minX;
			for (x = (uint32_t)minX; x < (uint32_t)maxX; x++) {
				int matrixHor, matrixVer;
				val = 0;
				for (matrixVer = mcTopStart; matrixVer < mcBottomEnd; matrixVer++) {
					int buflineStep = width*(matrixVer - 2);
					int as = y + matrixVer - 2;
					sbyte *buflineTopS = nullptr;
					if (as >= 0 && (uint32_t)as < height) buflineTopS = buflineMid + buflineStep;

					for (matrixHor = mcLeftStart; matrixHor < mcRightEnd; matrixHor++) {
						buflineTop = buflineTopS;
						int a = x + matrixHor - 2;
						if (buflineTop && a >= 0 && (uint32_t)a < width) buflineTop += matrixHor - 2;
						else buflineTop = nullptr;
						if (buflineTop)
							val += ((*buflineTop)*matrix[matrixVer * 5 + matrixHor]);
					}
				}
				val = (val + 1) >> 5;
				*bufline = (sbyte)((val>127) ? 127 : (val < -125) ? -125 : val);
				bufline++;
				buflineMid++;
			}
		}
		mir_free(buf);
		buf = outbuf;
	}
	{
		uint8_t r1, b1, g1, a1;
		b1 = bl; r1 = rl; g1 = gl; a1 = al; sign = 1;
		//perform out to dest
		for (y = 0; y < height; y++) {
			bufline = buf + y*width;
			destline = destPt + ((y*width) << 2);
			for (x = 0; x < width; x++) {
				sbyte val = *bufline;
				uint8_t absVal = ((val < 0) ? -val : val);

				if (val != 0) {
					if (val > 0 && sign < 0) {
						b1 = bl; r1 = rl; g1 = gl; a1 = al; sign = 1;
					}
					else if (val < 0 && sign>0) {
						b1 = bd; r1 = rd; g1 = gd; a1 = ad; sign = -1;
					}

					absVal = absVal*a1 / 255;

					destline[0] = ((destline[0] * (128 - absVal)) + absVal*b1) >> 7;
					destline[1] = ((destline[1] * (128 - absVal)) + absVal*g1) >> 7;
					destline[2] = ((destline[2] * (128 - absVal)) + absVal*r1) >> 7;
					destline[3] += ((255 - destline[3])*(a1*absVal)) / 32640;
				}
				bufline++;
				destline += 4;
			}
		}
		mir_free(buf);
	}
	return false;
}

static int ske_AlphaTextOut(HDC hDC, LPCTSTR lpString, int nCount, RECT *lpRect, UINT format, uint32_t ARGBcolor)
{
	if (!(lpString && lpRect))
		return 0;

	// Step first fill fast calc correction tables:
	static bool _tables_empty = true;
	static uint8_t gammaTbl[256];			// Gamma correction table
	static uint16_t blueMulTbl[256];		// blue  coefficient multiplication table
	static uint16_t greenMulTbl[256];		// green coefficient multiplication table
	static uint16_t redMulTbl[256];			// red   coefficient multiplication table
	if (_tables_empty) {
		// fill tables
		double gammaCfPw = 1000 / (double)DBGetContactSettingRangedWord(0, "ModernData", "AlphaTextOutGamma", 700, 1, 5000);
		uint8_t blueCf = db_get_b(0, "ModernData", "AlphaTextOutBlueCorrection", 28);
		uint8_t redCf = db_get_b(0, "ModernData", "AlphaTextOutRed Correction", 77);
		uint8_t greenCf = db_get_b(0, "ModernData", "AlphaTextOutGreen Correction", 151);

		for (int i = 0; i < 256; i++) {
			gammaTbl[i] = (uint8_t)(255 * pow((double)i / 255, gammaCfPw));
			blueMulTbl[i] = i * blueCf;
			redMulTbl[i] = i * redCf;
			greenMulTbl[i] = i * greenCf;
		}
	}

	// Calc len of input string
	if (nCount == -1)
		nCount = (int)mir_wstrlen(lpString);

	// retrieve destination bitmap bits
	HBITMAP hDestBitmap = (HBITMAP)GetCurrentObject(hDC, OBJ_BITMAP);
	BITMAP  bmpDest;
	GetObject(hDestBitmap, sizeof(BITMAP), &bmpDest);

	bool destHasNotDIB = (bmpDest.bmBits == nullptr);
	uint8_t *pDestBits;
	if (destHasNotDIB) {
		pDestBits = (uint8_t*)mir_alloc(bmpDest.bmHeight * bmpDest.bmWidthBytes);
		GetBitmapBits(hDestBitmap, bmpDest.bmHeight*bmpDest.bmWidthBytes, pDestBits);
	}
	else
		pDestBits = (uint8_t*)bmpDest.bmBits;

	// Creating offscreen buffer
	HDC hOffscreenDC = CreateCompatibleDC(hDC);

	// Font to be used to draw text
	HFONT hFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);
	HFONT hOldOffscreenFont = (HFONT)SelectObject(hOffscreenDC, hFont);

	// Calculating text geometric size
	RECT workRect = *lpRect;
	int workRectWidth = workRect.right - workRect.left;
	int workRectHeight = workRect.bottom - workRect.top;
	if (workRectWidth <= 0 || workRectHeight <= 0) {
		if (destHasNotDIB)
			mir_free(pDestBits);
		return 0;
	}

	SIZE textSize;
	GetTextExtentPoint32(hOffscreenDC, lpString, nCount, &textSize);

	LPCTSTR lpWorkString = lpString;
	BOOL bNeedFreeWorkString = FALSE;

	// if we need to cut the text with ellipsis
	if ((format & DT_END_ELLIPSIS) && textSize.cx > workRectWidth) {
		// Calc geometric width of ellipsis
		SIZE szEllipsis;
		GetTextExtentPoint32A(hOffscreenDC, "...", 3, &szEllipsis);
		szEllipsis.cx++;                                          // CORRECTION: some width correction

		// Calc count of visible chars
		int visibleCharCount = nCount;
		if (workRectWidth > szEllipsis.cx)
			GetTextExtentExPoint(hOffscreenDC, lpString, nCount, workRectWidth - szEllipsis.cx, &visibleCharCount, nullptr, &textSize);
		else
			GetTextExtentExPoint(hOffscreenDC, lpString, nCount, 0, &visibleCharCount, nullptr, &textSize);

		// replace end of string by elipsis
		bNeedFreeWorkString = TRUE;
		lpWorkString = (wchar_t*)mir_alloc((visibleCharCount + 4) * sizeof(wchar_t));

		memcpy((void*)lpWorkString, lpString, visibleCharCount * sizeof(wchar_t));
		memcpy((void*)(lpWorkString + visibleCharCount), L"...", 4 * sizeof(wchar_t)); // 3 + 1

		nCount = visibleCharCount + 3;
	}

	// Calc sizes and offsets

	textSize.cx += 2;             // CORRECTION: for italic

	int drx = 0;                  // x-axis offset of draw point

	if (workRectWidth > textSize.cx) {
		if (format & (DT_RIGHT | DT_RTLREADING))
			drx = workRectWidth - textSize.cx;
		else if (format & DT_CENTER)
			drx = (workRectWidth - textSize.cx) >> 1;
	}
	else textSize.cx = workRectWidth;

	int dry = 0;                  // y-axis offset of draw point

	if (workRectHeight > textSize.cy) {
		if (format & DT_BOTTOM)
			dry = workRectHeight - textSize.cy;
		else if (format & DT_VCENTER)
			dry = (workRectHeight - textSize.cy) >> 1;
	}
	else textSize.cy = workRectHeight;

	textSize.cx += 4;    // CORRECTION: for effects ???
	textSize.cy += 4;	 // CORRECTION: for effects ???

	if (textSize.cx > 0 && textSize.cy > 0) { // Ok we need to paint
		// probably here are mess ofscreen and temp buff dc

		//Create bitmap image for offscreen
		uint8_t *bits = nullptr;
		HBITMAP hbmp = ske_CreateDIB32Point(textSize.cx, textSize.cy, (void**)&bits);
		if (bits != nullptr) {
			HBITMAP holdbmp = (HBITMAP)SelectObject(hOffscreenDC, hbmp);

			//Create buffer bitmap image for temp text
			uint8_t *bufbits = nullptr;
			HBITMAP bufbmp = ske_CreateDIB32Point(textSize.cx, textSize.cy, (void**)&bufbits);
			if (bufbits != nullptr) {
				HDC     bufDC = CreateCompatibleDC(hDC);
				HBITMAP bufoldbmp = (HBITMAP)SelectObject(bufDC, bufbmp);
				HFONT   hOldBufFont = (HFONT)SelectObject(bufDC, hFont);
				SetBkColor(bufDC, RGB(0, 0, 0));
				SetTextColor(bufDC, RGB(255, 255, 255));

				// Copy from destination to temp buffer
				BitBlt(hOffscreenDC, 0, 0, textSize.cx, textSize.cy, hDC, workRect.left + drx - 2, workRect.top + dry - 2, SRCCOPY);

				//Draw text on offscreen bitmap
				TextOut(bufDC, 2, 2, lpWorkString, nCount);

				MODERNEFFECT effect;
				if (ske_GetTextEffect(hDC, &effect))
					ske_DrawTextEffect(bits, bufbits, textSize.cx, textSize.cy, &effect);

				// RenderText
				RECT drawRect;
				drawRect.left = 0; drawRect.top = 0;
				drawRect.right = textSize.cx;
				drawRect.bottom = textSize.cy;

				uint32_t width = textSize.cx;
				uint32_t heigh = textSize.cy;

				uint8_t *pDestScanLine, *pBufScanLine, *pix, *bufpix;

				uint8_t al = 255 - ((uint8_t)(ARGBcolor >> 24));
				uint8_t r = GetRValue(ARGBcolor);
				uint8_t g = GetGValue(ARGBcolor);
				uint8_t b = GetBValue(ARGBcolor);

				for (uint32_t y = 2; y < heigh - 2; y++) {
					int lineBytes = y * (width << 2);

					pDestScanLine = bits + lineBytes;
					pBufScanLine = bufbits + lineBytes;

					for (uint32_t x = 2; x < width - 2; x++) {
						pix = pDestScanLine + (x << 2);
						bufpix = pBufScanLine + (x << 2);

						// Monochromatic
						uint8_t bx = gammaTbl[bufpix[0]];
						uint8_t gx = gammaTbl[bufpix[1]];
						uint8_t rx = gammaTbl[bufpix[2]];

						if (al != 255) {
							bx *= al / 255;
							gx *= al / 255;
							rx *= al / 255;
						}

						uint8_t ax = (uint8_t)(((uint32_t)rx * 77 + (uint32_t)gx * 151 + (uint32_t)bx * 28 + 128) / 256);
						if (ax) {
							//Normalize components to gray
							uint8_t axx = 255 - ((r + g + b) >> 2); // Coefficient of grayance, more white font - more gray edges
							uint16_t atx = ax * (255 - axx);
							bx = (atx + bx * axx) / 255;
							gx = (atx + gx * axx) / 255;
							rx = (atx + rx * axx) / 255;

							short brx = (short)((b - pix[0])*bx / 255);
							short grx = (short)((g - pix[1])*gx / 255);
							short rrx = (short)((r - pix[2])*rx / 255);

							pix[0] += brx;
							pix[1] += grx;
							pix[2] += rrx;
							pix[3] = (uint8_t)(ax + (uint8_t)(255 - ax)*pix[3] / 255);
						}
					}
				}

				// Blit to destination
				BitBlt(hDC, workRect.left + drx - 2, workRect.top + dry - 2, textSize.cx, textSize.cy, hOffscreenDC, 0, 0, SRCCOPY);

				//free resources
				SelectObject(bufDC, bufoldbmp);
				DeleteObject(bufbmp);
				SelectObject(bufDC, hOldBufFont);
				DeleteDC(bufDC);
			}
			SelectObject(hOffscreenDC, holdbmp);
			DeleteObject(hbmp);
		}
	}

	// Final cleanup
	SelectObject(hOffscreenDC, hOldOffscreenFont);
	DeleteDC(hOffscreenDC);

	if (destHasNotDIB)
		mir_free(pDestBits);

	if (bNeedFreeWorkString)
		mir_free((void*)lpWorkString);

	return 0;
}

static int ske_DrawTextWithEffectWorker(HDC hdc, LPCTSTR lpString, int nCount, RECT *lpRect, UINT format, FONTEFFECT *effect)
{
	if (format & DT_CALCRECT)
		return DrawText(hdc, lpString, nCount, lpRect, format);

	if (format & DT_RTLREADING)
		SetTextAlign(hdc, TA_RTLREADING);

	uint32_t color = GetTextColor(hdc);
	RECT r = *lpRect;
	OffsetRect(&r, 1, 1);
	uint32_t form = format;
	if (effect && effect->effectIndex)
		ske_SelectTextEffect(hdc, effect->effectIndex - 1, effect->baseColour, effect->secondaryColour);

	int res = ske_AlphaTextOut(hdc, lpString, nCount, lpRect, form, color);

	if (effect && effect->effectIndex)
		ske_ResetTextEffect(hdc);

	return res;
}

INT_PTR ske_Service_DrawTextWithEffect(WPARAM wParam, LPARAM)
{
	DrawTextWithEffectParam *p = (DrawTextWithEffectParam *)wParam;
	return ske_DrawTextWithEffectWorker(p->hdc, p->lpchText, p->cchText, p->lprc, p->dwDTFormat, p->pEffect);
}

BOOL ske_DrawText(HDC hdc, LPCTSTR lpString, int nCount, RECT *lpRect, UINT format)
{
	RECT r = *lpRect;
	OffsetRect(&r, 1, 1);
	if (format & DT_RTLREADING)
		SetTextAlign(hdc, TA_RTLREADING);
	if (format & DT_CALCRECT)
		return DrawText(hdc, lpString, nCount, lpRect, format);
	if (format & DT_FORCENATIVERENDER || g_CluiData.fDisableSkinEngine)
		return DrawText(hdc, lpString, nCount, lpRect, format & ~DT_FORCENATIVERENDER);

	uint32_t form = format;
	uint32_t color = GetTextColor(hdc);
	return ske_AlphaTextOut(hdc, lpString, nCount, lpRect, form, color);
}

HICON ske_ImageList_GetIcon(HIMAGELIST himl, int i)
{
	IMAGEINFO imi = {};
	BITMAP bm = { 0 };
	if (i != -1) {
		ImageList_GetImageInfo(himl, i, &imi);
		GetObject(imi.hbmImage, sizeof(bm), &bm);
		// stupid bug of Microsoft
		// Icons bitmaps are not premultiplied
		// So Imagelist_AddIcon - premultiply alpha
		// But incorrect - it is possible that alpha will
		// be less than color and
		// ImageList_GetIcon will return overflowed colors
		// TODO: Direct draw Icon from imagelist without
		// extracting of icon
		if (bm.bmBitsPixel == 32) {
			uint8_t *bits = (uint8_t*)bm.bmBits;
			if (!bits) {
				bits = (uint8_t*)mir_alloc(bm.bmWidthBytes*bm.bmHeight);
				GetBitmapBits(imi.hbmImage, bm.bmWidthBytes*bm.bmHeight, bits);
			}

			uint8_t *bcbits = bits + (bm.bmHeight - imi.rcImage.bottom)*bm.bmWidthBytes + (imi.rcImage.left*bm.bmBitsPixel >> 3);
			for (int iy = 0; iy < imi.rcImage.bottom - imi.rcImage.top; iy++) {
				int x;
				// Dummy microsoft fix - alpha can be less than r,g or b
				// Looks like color channels in icons should be non-premultiplied with alpha
				// But AddIcon store it premultiplied (incorrectly cause can be Alpha == 7F, but R,G or B == 80
				// So i check that alpha is 0x7F and set it to 0x80
				uint32_t *c = ((uint32_t*)bcbits);
				for (x = 0; x < imi.rcImage.right - imi.rcImage.left; x++) {
					uint32_t val = *c;
					uint8_t a = (uint8_t)((val) >> 24);
					if (a != 0) {
						uint8_t r = (uint8_t)((val & 0xFF0000) >> 16);
						uint8_t g = (uint8_t)((val & 0xFF00) >> 8);
						uint8_t b = (uint8_t)(val & 0xFF);
						if (a < r || a < g || a < b) {
							a = max(max(r, g), b);
							val = a << 24 | r << 16 | g << 8 | b;
							*c = val;
						}
					}
					c++;
				}
				bcbits += bm.bmWidthBytes;
			}

			if (!bm.bmBits) {
				SetBitmapBits(imi.hbmImage, bm.bmWidthBytes*bm.bmHeight, bits);
				mir_free(bits);
			}
		}
	}
	return ImageList_GetIcon(himl, i, ILD_NORMAL);
}

BOOL ske_ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
{
	// the routine to directly draw icon from image list without creating icon from there - should be some faster
	if (i < 0)
		return FALSE;

	if (g_CluiData.fDisableSkinEngine)
		return ImageList_DrawEx(himl, i, hdcDst, x, y, dx, dy, rgbBk, rgbFg, fStyle);

	uint8_t alpha;
	if (fStyle & ILD_BLEND25)
		alpha = 64;
	else if (fStyle & ILD_BLEND50)
		alpha = 128;
	else
		alpha = 255;

	HICON hIcon = ske_ImageList_GetIcon(himl, i);
	if (hIcon == nullptr)
		return FALSE;

	ske_DrawIconEx(hdcDst, x, y, hIcon, dx ? dx : GetSystemMetrics(SM_CXSMICON), dy ? dy : GetSystemMetrics(SM_CYSMICON), 0, nullptr, DI_NORMAL | (alpha << 24));
	DestroyIcon(hIcon);
	return TRUE;
}

static INT_PTR ske_Service_DrawIconEx(WPARAM wParam, LPARAM)
{
	DrawIconFixParam *p = (DrawIconFixParam*)wParam;
	if (!p)
		return 0;

	return ske_DrawIconEx(p->hdc, p->xLeft, p->yTop, p->hIcon, p->cxWidth, p->cyWidth, p->istepIfAniCur, p->hbrFlickerFreeDraw, p->diFlags);
}


BOOL ske_DrawIconEx(HDC hdcDst, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags)
{
	ICONINFO ici;
	uint8_t alpha = (uint8_t)((diFlags & 0xFF000000) >> 24);

	HBITMAP tBmp = nullptr;
	uint8_t *imbits, *imimagbits, *immaskbits;
	uint8_t *t1, *t2, *t3;

	//lockimagelist
	uint8_t hasmask = FALSE, no32bit = FALSE, noMirrorMask = FALSE, hasalpha = FALSE;
	alpha = alpha ? alpha : 255;

	if (g_CluiData.fDisableSkinEngine && !(diFlags & 0x80))
		return DrawIconEx(hdcDst, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags & 0xFFFF7F);

	if (!GetIconInfo(hIcon, &ici))
		return 0;

	BITMAP imbt;
	GetObject(ici.hbmColor, sizeof(BITMAP), &imbt);
	if (imbt.bmWidth*imbt.bmHeight == 0) {
		DeleteObject(ici.hbmColor);
		DeleteObject(ici.hbmMask);
		return 0;
	}

	BITMAP immaskbt;
	GetObject(ici.hbmMask, sizeof(BITMAP), &immaskbt);
	uint32_t cy = imbt.bmHeight;

	if (imbt.bmBitsPixel != 32) {
		no32bit = TRUE;
		HDC tempDC1 = CreateCompatibleDC(hdcDst);
		tBmp = ske_CreateDIB32(imbt.bmWidth, imbt.bmHeight);
		if (tBmp) {
			GetObject(tBmp, sizeof(BITMAP), &imbt);
			HBITMAP otBmp = (HBITMAP)SelectObject(tempDC1, tBmp);
			DrawIconEx(tempDC1, 0, 0, hIcon, imbt.bmWidth, imbt.bmHeight, istepIfAniCur, hbrFlickerFreeDraw, DI_IMAGE);
			noMirrorMask = TRUE;
			SelectObject(tempDC1, otBmp);
		}
		DeleteDC(tempDC1);
	}

	bool NoDIBImage = (imbt.bmBits == nullptr);
	if (NoDIBImage) {
		imimagbits = (uint8_t*)mir_alloc(cy*imbt.bmWidthBytes);
		GetBitmapBits(ici.hbmColor, cy*imbt.bmWidthBytes, (void*)imimagbits);
	}
	else imimagbits = (uint8_t*)imbt.bmBits;

	if (immaskbt.bmBits == nullptr) {
		immaskbits = (uint8_t*)mir_alloc(cy*immaskbt.bmWidthBytes);
		GetBitmapBits(ici.hbmMask, cy*immaskbt.bmWidthBytes, (void*)immaskbits);
	}
	else immaskbits = (uint8_t*)immaskbt.bmBits;

	HDC imDC = CreateCompatibleDC(hdcDst);
	uint32_t icy = imbt.bmHeight;
	uint32_t cx = imbt.bmWidth;
	HBITMAP imBmp = ske_CreateDIB32Point(cx, icy, (void**)&imbits);
	HBITMAP oldBmp = (HBITMAP)SelectObject(imDC, imBmp);
	if (imbits != nullptr && imimagbits != nullptr && immaskbits != nullptr) {
		int x, y;
		int mwb = immaskbt.bmWidthBytes;
		int mwb2 = imbt.bmWidthBytes;
		int bottom = icy;
		int right = cx;
		int top = 0;
		int h = icy;
		for (y = top; (y < bottom) && !hasmask; y++) {
			t1 = immaskbits + y*mwb;
			for (x = 0; (x < mwb) && !hasmask; x++)
				hasmask |= (*(t1 + x) != 0);
		}

		for (y = top; (y < bottom) && !hasalpha; y++) {
			t1 = imimagbits + (cy - y - 1)*mwb2;
			for (x = 0; (x < right) && !hasalpha; x++)
				hasalpha |= (*(t1 + (x << 2) + 3) != 0);
		}

		for (y = 0; y < (int)icy; y++) {
			t1 = imimagbits + (h - y - 1 - top)*mwb2;
			t2 = imbits + (!no32bit ? y : (icy - y - 1))*mwb2;
			t3 = immaskbits + (noMirrorMask ? y : (h - y - 1 - top))*mwb;
			for (x = 0; x < right; x++) {
				uint8_t mask = 0;
				uint8_t a = 0;
				uint32_t *src = (uint32_t*)(t1 + (x << 2));
				uint32_t *dest = (uint32_t*)(t2 + (x << 2));
				if (hasalpha && !hasmask)
					a = ((uint8_t*)src)[3];
				else {
					mask = ((1 << (7 - x % 8))&(*(t3 + (x >> 3)))) != 0;
					if (mask) {
						if (!hasalpha) {
							*dest = 0;
							continue;
						}

						if (((uint8_t*)src)[3]>0)
							a = ((uint8_t*)src)[3];
						else
							a = 0;
					}
					else if (hasalpha || hasmask)
						a = (((uint8_t*)src)[3] > 0 ? ((uint8_t*)src)[3] : 255);
					else if (!hasalpha && !hasmask)
						a = 255;
					else { *dest = 0; continue; }
				}
				if (a > 0) {
					((uint8_t*)dest)[3] = a;
					((uint8_t*)dest)[0] = ((uint8_t*)src)[0] * a / 255;
					((uint8_t*)dest)[1] = ((uint8_t*)src)[1] * a / 255;
					((uint8_t*)dest)[2] = ((uint8_t*)src)[2] * a / 255;
				}
				else *dest = 0;
			}
		}
	}

	BLENDFUNCTION bf = { AC_SRC_OVER, diFlags & 128, alpha, AC_SRC_ALPHA };
	ske_AlphaBlend(hdcDst, xLeft, yTop, cxWidth, cyWidth, imDC, 0, 0, cx, icy, bf);

	if (immaskbt.bmBits == nullptr) mir_free(immaskbits);
	if (imbt.bmBits == nullptr) mir_free(imimagbits);
	SelectObject(imDC, oldBmp);
	DeleteObject(imBmp);
	if (no32bit)DeleteObject(tBmp);
	DeleteObject(ici.hbmColor);
	DeleteObject(ici.hbmMask);
	SelectObject(imDC, GetStockObject(DEFAULT_GUI_FONT));
	DeleteDC(imDC);
	return 1;
}

int ske_PrepareImageButDontUpdateIt(RECT *r)
{
	if (!g_CluiData.fLayered)
		return ske_ReCreateBackImage(FALSE, r);

	mutex_bLockUpdate = 1;
	ske_DrawNonFramedObjects(TRUE, r);
	ske_ValidateFrameImageProc(r);
	mutex_bLockUpdate = 0;
	return 0;
}

int ske_RedrawCompleteWindow()
{
	if (g_CluiData.fLayered) {
		ske_DrawNonFramedObjects(TRUE, nullptr);
		CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, 0, 0);
	}
	else RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);

	return 0;
}

// Request to repaint frame or change/drop callback data
// wParam = hWnd of called frame
// lParam = pointer to sPaintRequest (or nullptr to redraw all)
// return 2 - already queued, data updated, 1-have been queued, 0 - failure

static INT_PTR ske_Service_UpdateFrameImage(WPARAM wParam, LPARAM)           // Immideately recall paint routines for frame and refresh image
{
	if (MirandaLoading())
		return 0;

	RECT wnd;
	bool NoCancelPost = false;
	bool IsAnyQueued = false;
	if (!g_CluiData.mutexOnEdgeSizing)
		GetWindowRect(g_clistApi.hwndContactList, &wnd);
	else
		wnd = g_rcEdgeSizingRect;

	if (!g_CluiData.fLayered) {
		RedrawWindow((HWND)wParam, nullptr, nullptr, RDW_UPDATENOW | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
		return 0;
	}

	if (g_pCachedWindow == nullptr) ske_ValidateFrameImageProc(&wnd);
	else if (g_pCachedWindow->Width != wnd.right - wnd.left || g_pCachedWindow->Height != wnd.bottom - wnd.top) ske_ValidateFrameImageProc(&wnd);
	else if (wParam == 0) ske_ValidateFrameImageProc(&wnd);
	else { // all Ok Update Single Frame
		// TO BE LOCKED OR PROXIED
		FRAMEWND *frm = FindFrameByItsHWND((HWND)wParam);
		if (!frm)
			ske_ValidateFrameImageProc(&wnd);
		// Validate frame, update window image and remove it from queue
		else {
			if (frm->UpdateRgn) {
				DeleteObject(frm->UpdateRgn);
				frm->UpdateRgn = nullptr;
			}
			ske_ValidateSingleFrameImage(frm, 0);
			ske_UpdateWindowImage();
			NoCancelPost = 1;
			//-- Remove frame from queue
			if (flag_bUpdateQueued) {
				frm->bQueued = 0;
				for (int i = 0; i < g_nFramesCount; i++)
					if (IsAnyQueued |= g_pfwFrames[i].bQueued)
						break;
			}
		}
	}

	if ((!NoCancelPost || !IsAnyQueued) && flag_bUpdateQueued) { // no any queued updating cancel post or need to cancel post
		flag_bUpdateQueued = 0;
		g_bPostWasCanceled = true;
	}
	return 1;
}

static INT_PTR ske_Service_InvalidateFrameImage(WPARAM wParam, LPARAM lParam)       // Post request for updating
{
	if (MirandaLoading()) return 0;

	if (wParam) {
		FRAMEWND *frm = FindFrameByItsHWND((HWND)wParam);
		sPaintRequest *pr = (sPaintRequest*)lParam;
		if (!g_CluiData.fLayered || (frm && frm->floating))
			return InvalidateRect((HWND)wParam, pr ? (RECT*)&(pr->rcUpdate) : nullptr, FALSE);

		if (frm) {
			if (frm->PaintCallbackProc != nullptr) {
				frm->PaintData = (sPaintRequest *)pr;
				frm->bQueued = 1;
				if (pr) {
					HRGN r2;
					if (!IsRectEmpty(&pr->rcUpdate)) {
						RECT rcClient;
						RECT rcUpdate;
						GetClientRect(frm->hWnd, &rcClient);
						IntersectRect(&rcUpdate, &rcClient, &pr->rcUpdate);
						if (IsRectEmpty(&rcUpdate))
							return 0;
						r2 = CreateRectRgn(rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom);
					}
					else {
						RECT r;
						GetClientRect(frm->hWnd, &r);
						r2 = CreateRectRgn(r.left, r.top, r.right, r.bottom);
					}

					if (!frm->UpdateRgn) {
						frm->UpdateRgn = CreateRectRgn(0, 0, 1, 1);
						CombineRgn(frm->UpdateRgn, r2, nullptr, RGN_COPY);
					}
					else CombineRgn(frm->UpdateRgn, frm->UpdateRgn, r2, RGN_OR);
					DeleteObject(r2);
				}
			}
		}
		else Sync(QueueAllFramesUpdating, true);
	}
	else Sync(QueueAllFramesUpdating, true);

	if (!flag_bUpdateQueued || g_bPostWasCanceled)
		if (PostMessage(g_clistApi.hwndContactList, UM_UPDATE, 0, 0)) {
			flag_bUpdateQueued = 1;
			g_bPostWasCanceled = false;
		}
	return 1;
}

static int ske_ValidateSingleFrameImage(FRAMEWND *Frame, BOOL SkipBkgBlitting)                              // Calling frame paint proc
{
	if (!g_pCachedWindow) { TRACE("ske_ValidateSingleFrameImage calling without cached\n"); return 0; }
	if (Frame->hWnd == (HWND)-1 && !Frame->PaintCallbackProc) { TRACE("ske_ValidateSingleFrameImage calling without FrameProc\n"); return 0; }

	// if ok update image
	RECT rcPaint, wnd;
	RECT ru = { 0 };
	int w1, h1, x1, y1;

	CLUI_SizingGetWindowRect(g_clistApi.hwndContactList, &wnd);
	rcPaint = Frame->wndSize;
	{
		int dx, dy, bx, by;
		if (g_CluiData.mutexOnEdgeSizing) {
			dx = rcPaint.left - wnd.left;
			dy = rcPaint.top - wnd.top;
			bx = rcPaint.right - wnd.right;
			by = rcPaint.bottom - wnd.bottom;
			wnd = g_rcEdgeSizingRect;
			rcPaint.left = wnd.left + dx;
			rcPaint.top = wnd.top + dy;
			rcPaint.right = wnd.right + bx;
			rcPaint.bottom = wnd.bottom + by;
		}
	}

	int w = rcPaint.right - rcPaint.left;
	int h = rcPaint.bottom - rcPaint.top;
	if (w <= 0 || h <= 0) {
		TRACE("Frame size smaller than 0\n");
		return 0;
	}
	int x = rcPaint.left;
	int y = rcPaint.top;
	HDC hdc = CreateCompatibleDC(g_pCachedWindow->hImageDC);
	HBITMAP n = ske_CreateDIB32(w, h);
	HBITMAP o = (HBITMAP)SelectObject(hdc, n);

	if (Frame->UpdateRgn && !SkipBkgBlitting) {
		GetRgnBox(Frame->UpdateRgn, &ru);
		{
			RECT rc;
			GetClientRect(Frame->hWnd, &rc);
			if (ru.top < 0) ru.top = 0;
			if (ru.left < 0) ru.left = 0;
			if (ru.right > rc.right) ru.right = rc.right;
			if (ru.bottom > rc.bottom) ru.bottom = rc.bottom;
		}
		if (!IsRectEmpty(&ru)) {
			x1 = ru.left;
			y1 = ru.top;
			w1 = ru.right - ru.left;
			h1 = ru.bottom - ru.top;
		}
		else {
			x1 = 0; y1 = 0; w1 = w; h1 = h;
		}

		// copy image at hdc
		BitBlt(hdc, x1, y1, w1, h1, g_pCachedWindow->hBackDC, x + x1, y + y1, SRCCOPY);

		Frame->PaintCallbackProc(Frame->hWnd, hdc, &ru, Frame->UpdateRgn, Frame->dwFlags, Frame->PaintData);
	}
	else {
		RECT r;
		GetClientRect(Frame->hWnd, &r);
		HRGN rgnUpdate = CreateRectRgn(r.left, r.top, r.right, r.bottom);
		ru = r;
		if (!IsRectEmpty(&ru)) {
			x1 = ru.left;
			y1 = ru.top;
			w1 = ru.right - ru.left;
			h1 = ru.bottom - ru.top;
		}
		else {
			x1 = 0; y1 = 0; w1 = w; h1 = h;
		}

		// copy image at hdc
		if (SkipBkgBlitting)  //image already at foreground
			BitBlt(hdc, x1, y1, w1, h1, g_pCachedWindow->hImageDC, x + x1, y + y1, SRCCOPY);
		else
			BitBlt(hdc, x1, y1, w1, h1, g_pCachedWindow->hBackDC, x + x1, y + y1, SRCCOPY);

		Frame->PaintCallbackProc(Frame->hWnd, hdc, &r, rgnUpdate, Frame->dwFlags, Frame->PaintData);
		ru = r;
		DeleteObject(rgnUpdate);
	}
	DeleteObject(Frame->UpdateRgn);
	Frame->UpdateRgn = nullptr;

	if (!IsRectEmpty(&ru)) {
		x1 = ru.left;
		y1 = ru.top;
		w1 = ru.right - ru.left;
		h1 = ru.bottom - ru.top;
	}
	else {
		x1 = 0; y1 = 0; w1 = w; h1 = h;
	}

	BitBlt(g_pCachedWindow->hImageDC, x + x1, y + y1, w1, h1, hdc, x1, y1, SRCCOPY);

	if (GetWindowLongPtr(Frame->hWnd, GWL_STYLE) & WS_VSCROLL) {
		//Draw vertical scroll bar
		//
		SCROLLBARINFO si = { 0 };
		si.cbSize = sizeof(SCROLLBARINFO);
		GetScrollBarInfo(Frame->hWnd, OBJID_VSCROLL, &si);

		RECT rLine = (si.rcScrollBar);
		RECT rUpBtn = rLine;
		RECT rDnBtn = rLine;
		RECT rThumb = rLine;

		rUpBtn.bottom = rUpBtn.top + si.dxyLineButton;
		rDnBtn.top = rDnBtn.bottom - si.dxyLineButton;
		rThumb.top = rLine.top + si.xyThumbTop;
		rThumb.bottom = rLine.top + si.xyThumbBottom;

		int dx = Frame->wndSize.right - rLine.right;
		int dy = -rLine.top + Frame->wndSize.top;

		OffsetRect(&rLine, dx, dy);
		OffsetRect(&rUpBtn, dx, dy);
		OffsetRect(&rDnBtn, dx, dy);
		OffsetRect(&rThumb, dx, dy);
		BitBlt(g_pCachedWindow->hImageDC, rLine.left, rLine.top, rLine.right - rLine.left, rLine.bottom - rLine.top, g_pCachedWindow->hBackDC, rLine.left, rLine.top, SRCCOPY);

		char req[255];
		mir_snprintf(req, "Main,ID=ScrollBar,Frame=%S,Part=Back", Frame->name);
		SkinDrawGlyph(g_pCachedWindow->hImageDC, &rLine, &rLine, req);
		mir_snprintf(req, "Main,ID=ScrollBar,Frame=%S,Part=Thumb", Frame->name);
		SkinDrawGlyph(g_pCachedWindow->hImageDC, &rThumb, &rThumb, req);
		mir_snprintf(req, "Main,ID=ScrollBar, Frame=%S,Part=UpLineButton", Frame->name);
		SkinDrawGlyph(g_pCachedWindow->hImageDC, &rUpBtn, &rUpBtn, req);
		mir_snprintf(req, "Main,ID=ScrollBar,Frame=%S,Part=DownLineButton", Frame->name);
		SkinDrawGlyph(g_pCachedWindow->hImageDC, &rDnBtn, &rDnBtn, req);
	}

	SelectObject(hdc, o);
	DeleteObject(n);
	DeleteDC(hdc);
	return 1;
}

int ske_BltBackImage(HWND destHWND, HDC destDC, RECT *BltClientRect)
{
	POINT ptMainWnd = { 0 };
	POINT ptChildWnd = { 0 };
	RECT w = { 0 };
	if (g_CluiData.fDisableSkinEngine) {
		FillRect(destDC, BltClientRect, GetSysColorBrush(COLOR_3DFACE));
		return 0;
	}
	ske_ReCreateBackImage(FALSE, nullptr);
	if (BltClientRect) w = *BltClientRect;
	else GetClientRect(destHWND, &w);
	ptChildWnd.x = w.left;
	ptChildWnd.y = w.top;
	ClientToScreen(destHWND, &ptChildWnd);
	ClientToScreen(g_clistApi.hwndContactList, &ptMainWnd);
	//TODO if main not relative to client area
	return BitBlt(destDC, w.left, w.top, (w.right - w.left), (w.bottom - w.top), g_pCachedWindow->hBackDC, (ptChildWnd.x - ptMainWnd.x), (ptChildWnd.y - ptMainWnd.y), SRCCOPY);

}

int ske_ReCreateBackImage(BOOL Erase, RECT *w)
{
	RECT wnd = { 0 };
	BOOL IsNewCache = 0;
	if (g_CluiData.fDisableSkinEngine) return 0;
	GetClientRect(g_clistApi.hwndContactList, &wnd);
	if (w) wnd = *w;
	//-- Check cached.
	if (g_pCachedWindow == nullptr) {
		//-- Create New Cache
		g_pCachedWindow = (CURRWNDIMAGEDATA*)mir_calloc(sizeof(CURRWNDIMAGEDATA));
		g_pCachedWindow->hScreenDC = GetDC(nullptr);
		g_pCachedWindow->hBackDC = CreateCompatibleDC(g_pCachedWindow->hScreenDC);
		g_pCachedWindow->hImageDC = CreateCompatibleDC(g_pCachedWindow->hScreenDC);
		g_pCachedWindow->Width = wnd.right - wnd.left;
		g_pCachedWindow->Height = wnd.bottom - wnd.top;
		if (g_pCachedWindow->Width != 0 && g_pCachedWindow->Height != 0) {
			g_pCachedWindow->hImageDIB = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hImageDIBByte));
			g_pCachedWindow->hBackDIB = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hBackDIBByte));
			g_pCachedWindow->hImageOld = (HBITMAP)SelectObject(g_pCachedWindow->hImageDC, g_pCachedWindow->hImageDIB);
			g_pCachedWindow->hBackOld = (HBITMAP)SelectObject(g_pCachedWindow->hBackDC, g_pCachedWindow->hBackDIB);
		}
		IsNewCache = 1;
	}

	if (g_pCachedWindow->Width != wnd.right - wnd.left || g_pCachedWindow->Height != wnd.bottom - wnd.top) {
		HBITMAP hb1 = nullptr, hb2 = nullptr;
		g_pCachedWindow->Width = wnd.right - wnd.left;
		g_pCachedWindow->Height = wnd.bottom - wnd.top;
		if (g_pCachedWindow->Width != 0 && g_pCachedWindow->Height != 0) {
			hb1 = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hImageDIBByte));
			hb2 = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hBackDIBByte));
			SelectObject(g_pCachedWindow->hImageDC, hb1);
			SelectObject(g_pCachedWindow->hBackDC, hb2);
		}
		else {
			SelectObject(g_pCachedWindow->hImageDC, g_pCachedWindow->hImageOld);
			SelectObject(g_pCachedWindow->hBackDC, g_pCachedWindow->hBackOld);
		}
		if (g_pCachedWindow->hImageDIB) DeleteObject(g_pCachedWindow->hImageDIB);
		if (g_pCachedWindow->hBackDIB) DeleteObject(g_pCachedWindow->hBackDIB);
		g_pCachedWindow->hImageDIB = hb1;
		g_pCachedWindow->hBackDIB = hb2;
		IsNewCache = 1;
	}

	if ((Erase || IsNewCache) && (g_pCachedWindow->Width != 0 && g_pCachedWindow->Height != 0)) {
		HBITMAP hb2 = ske_CreateDIB32(g_pCachedWindow->Width, g_pCachedWindow->Height);
		SelectObject(g_pCachedWindow->hBackDC, hb2);
		DeleteObject(g_pCachedWindow->hBackDIB);
		g_pCachedWindow->hBackDIB = hb2;
		FillRect(g_pCachedWindow->hBackDC, &wnd, GetSysColorBrush(COLOR_BTNFACE));
		SkinDrawGlyph(g_pCachedWindow->hBackDC, &wnd, &wnd, "Main,ID=Background,Opt=Non-Layered");
		ske_SetRectOpaque(g_pCachedWindow->hBackDC, &wnd);
	}
	return 1;
}

int ske_DrawNonFramedObjects(BOOL Erase, RECT *r)
{
	RECT w, wnd;
	if (r) w = *r;
	else CLUI_SizingGetWindowRect(g_clistApi.hwndContactList, &w);
	if (!g_CluiData.fLayered) return ske_ReCreateBackImage(FALSE, nullptr);
	if (g_pCachedWindow == nullptr)
		return ske_ValidateFrameImageProc(&w);

	wnd = w;
	OffsetRect(&w, -w.left, -w.top);
	if (Erase) {
		HBITMAP hb2;
		hb2 = ske_CreateDIB32(g_pCachedWindow->Width, g_pCachedWindow->Height);
		SelectObject(g_pCachedWindow->hBackDC, hb2);
		DeleteObject(g_pCachedWindow->hBackDIB);
		g_pCachedWindow->hBackDIB = hb2;
	}

	SkinDrawGlyph(g_pCachedWindow->hBackDC, &w, &w, "Main,ID=Background");

	//--Draw frames captions
	for (int i = 0; i < g_nFramesCount; i++) {
		if (g_pfwFrames[i].TitleBar.ShowTitleBar && g_pfwFrames[i].visible && !g_pfwFrames[i].floating) {
			RECT rc;
			SetRect(&rc, g_pfwFrames[i].wndSize.left, g_pfwFrames[i].wndSize.top - g_nTitleBarHeight - g_CluiData.nGapBetweenTitlebar, g_pfwFrames[i].wndSize.right, g_pfwFrames[i].wndSize.top - g_CluiData.nGapBetweenTitlebar);
			Sync(DrawTitleBar, g_pCachedWindow->hBackDC, &rc, g_pfwFrames[i].id);
		}
	}
	g_mutex_bLockUpdating = 1;

	flag_bJustDrawNonFramedObjects = 1;
	return 0;
}

// Calling queued frame paint procs and refresh image
int ske_ValidateFrameImageProc(RECT *r)
{
	RECT wnd = { 0 };
	BOOL IsNewCache = 0;
	BOOL IsForceAllPainting = 0;
	if (r) wnd = *r;
	else GetWindowRect(g_clistApi.hwndContactList, &wnd);
	if (wnd.right - wnd.left == 0 || wnd.bottom - wnd.top == 0)
		return 0;

	g_mutex_bLockUpdating = 1;

	//-- Check cached.
	if (g_pCachedWindow == nullptr) {
		//-- Create New Cache
		g_pCachedWindow = (CURRWNDIMAGEDATA*)mir_calloc(sizeof(CURRWNDIMAGEDATA));
		g_pCachedWindow->hScreenDC = GetDC(nullptr);
		g_pCachedWindow->hBackDC = CreateCompatibleDC(g_pCachedWindow->hScreenDC);
		g_pCachedWindow->hImageDC = CreateCompatibleDC(g_pCachedWindow->hScreenDC);
		g_pCachedWindow->Width = wnd.right - wnd.left;
		g_pCachedWindow->Height = wnd.bottom - wnd.top;
		g_pCachedWindow->hImageDIB = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hImageDIBByte));
		g_pCachedWindow->hBackDIB = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hBackDIBByte));
		g_pCachedWindow->hImageOld = (HBITMAP)SelectObject(g_pCachedWindow->hImageDC, g_pCachedWindow->hImageDIB);
		g_pCachedWindow->hBackOld = (HBITMAP)SelectObject(g_pCachedWindow->hBackDC, g_pCachedWindow->hBackDIB);
		IsNewCache = 1;
	}
	if (g_pCachedWindow->Width != wnd.right - wnd.left || g_pCachedWindow->Height != wnd.bottom - wnd.top) {
		HBITMAP hb1, hb2;
		g_pCachedWindow->Width = wnd.right - wnd.left;
		g_pCachedWindow->Height = wnd.bottom - wnd.top;
		hb1 = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hImageDIBByte));
		hb2 = ske_CreateDIB32Point(g_pCachedWindow->Width, g_pCachedWindow->Height, (void**)&(g_pCachedWindow->hBackDIBByte));
		SelectObject(g_pCachedWindow->hImageDC, hb1);
		SelectObject(g_pCachedWindow->hBackDC, hb2);
		DeleteObject(g_pCachedWindow->hImageDIB);
		DeleteObject(g_pCachedWindow->hBackDIB);
		g_pCachedWindow->hImageDIB = hb1;
		g_pCachedWindow->hBackDIB = hb2;
		IsNewCache = 1;
	}
	if (IsNewCache) {
		ske_DrawNonFramedObjects(0, &wnd);
		IsForceAllPainting = 1;
	}
	if (flag_bJustDrawNonFramedObjects) {
		IsForceAllPainting = 1;
		flag_bJustDrawNonFramedObjects = 0;
	}
	if (IsForceAllPainting) {
		BitBlt(g_pCachedWindow->hImageDC, 0, 0, g_pCachedWindow->Width, g_pCachedWindow->Height, g_pCachedWindow->hBackDC, 0, 0, SRCCOPY);
		Sync(QueueAllFramesUpdating, true);
	}
	//-- Validating frames
	for (int i = 0; i < g_nFramesCount; i++)
		if (g_pfwFrames[i].PaintCallbackProc && g_pfwFrames[i].visible && !g_pfwFrames[i].floating)
			if (g_pfwFrames[i].bQueued || IsForceAllPainting)
				ske_ValidateSingleFrameImage(&g_pfwFrames[i], IsForceAllPainting);

	g_mutex_bLockUpdating = 1;
	ModernSkinButtonRedrawAll();
	g_mutex_bLockUpdating = 0;
	if (!mutex_bLockUpdate)
		ske_UpdateWindowImageRect(&wnd);

	//-- Clear queue
	Sync(QueueAllFramesUpdating, false);
	flag_bUpdateQueued = 0;
	g_bPostWasCanceled = false;
	return 1;
}

int ske_UpdateWindowImage()
{
	if (MirandaExiting())
		return 0;

	if (g_CluiData.fLayered) {
		RECT r;
		GetWindowRect(g_clistApi.hwndContactList, &r);
		return ske_UpdateWindowImageRect(&r);
	}
	else ske_ReCreateBackImage(FALSE, nullptr);
	ske_ApplyTranslucency();
	return 0;
}

int ske_UpdateWindowImageRect(RECT *r)                                     // Update window with current image and
{
	//if not validity -> ValidateImageProc
	//else Update using current alpha
	RECT wnd = *r;

	if (!g_CluiData.fLayered) return ske_ReCreateBackImage(FALSE, nullptr);
	if (g_pCachedWindow == nullptr) return ske_ValidateFrameImageProc(&wnd);
	if (g_pCachedWindow->Width != wnd.right - wnd.left || g_pCachedWindow->Height != wnd.bottom - wnd.top) return ske_ValidateFrameImageProc(&wnd);
	if (g_bFullRepaint) {
		g_bFullRepaint = false;
		return ske_ValidateFrameImageProc(&wnd);
	}
	ske_JustUpdateWindowImageRect(&wnd);
	return 0;
}

void ske_ApplyTranslucency()
{
	int IsTransparancy;
	HWND hwnd = g_clistApi.hwndContactList;
	BOOL layered = (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) ? TRUE : FALSE;

	IsTransparancy = g_CluiData.fSmoothAnimation || g_bTransparentFlag;
	if (!g_bTransparentFlag && !g_CluiData.fSmoothAnimation && g_CluiData.bCurrentAlpha != 0)
		g_CluiData.bCurrentAlpha = 255;

	if (!g_CluiData.fLayered && IsTransparancy) {
		if (!layered)
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (uint8_t)g_CluiData.bCurrentAlpha, LWA_ALPHA);
	}

	AniAva_RedrawAllAvatars(FALSE);
	return;
}

int ske_JustUpdateWindowImage()
{
	RECT r;
	if (!g_CluiData.fLayered) {
		ske_ApplyTranslucency();
		return 0;
	}
	GetWindowRect(g_clistApi.hwndContactList, &r);
	return ske_JustUpdateWindowImageRect(&r);
}

// Update window image
int ske_JustUpdateWindowImageRect(RECT *rty)
{
	if (!g_CluiData.fLayered) {
		ske_ApplyTranslucency();
		return 0;
	}
	if (!g_clistApi.hwndContactList)
		return 0;

	RECT wnd = *rty;
	RECT rect = wnd;
	POINT dest = { 0 }, src = { 0 };
	dest.x = rect.left;
	dest.y = rect.top;
	SIZE sz = { rect.right - rect.left, rect.bottom - rect.top };
	if (g_CluiData.fLayered) {
		if (!(GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & WS_EX_LAYERED))
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
		Sync(SetAlpha, g_CluiData.bCurrentAlpha);

		BLENDFUNCTION bf = { AC_SRC_OVER, 0, g_CluiData.bCurrentAlpha, AC_SRC_ALPHA };
		UpdateLayeredWindow(g_clistApi.hwndContactList, g_pCachedWindow->hScreenDC, &dest, &sz, g_pCachedWindow->hImageDC, &src, RGB(1, 1, 1), &bf, ULW_ALPHA);
		g_CluiData.fAeroGlass = false;
		CLUI_UpdateAeroGlass();
	}
	else InvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);
	return 0;
}

int ske_DrawImageAt(HDC hdc, RECT *rc)
{
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	BitBlt(g_pCachedWindow->hImageDC, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, g_pCachedWindow->hBackDC, rc->left, rc->top, SRCCOPY);
	ske_AlphaBlend(g_pCachedWindow->hImageDC, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, hdc, 0, 0, rc->right - rc->left, rc->bottom - rc->top, bf);
	if (!g_mutex_bLockUpdating)
		ske_UpdateWindowImage();
	return 0;
}

HBITMAP ske_GetCurrentWindowImage()
{
	return g_pCachedWindow->hImageDIB;
}

/*
*  Glyph text routine
*/

static uint32_t ske_HexToARGB(char *Hex)
{
	char buf[10] = { 0 };
	char buf2[11] = { 0 };
	mir_snprintf(buf, "%s\n", Hex);
	if (buf[1] == 'x' || buf[1] == 'X')
		mir_snprintf(buf2, "0x%s\n", buf + 2);
	else
		mir_snprintf(buf2, "0x%s\n", buf);
	buf2[10] = '\0';

	char *st;
	uint32_t AARRGGBB = strtoul(buf2, &st, 16);
	uint8_t alpha = (uint8_t)((AARRGGBB & 0xFF000000) >> 24);
	alpha = 255 - ((alpha == 0) ? 255 : alpha);
	AARRGGBB = (alpha << 24) + ((AARRGGBB & 0x00FF0000) >> 16) + ((AARRGGBB & 0x000000FF) << 16) + (AARRGGBB & 0x0000FF00);
	return AARRGGBB;
}

static wchar_t *ske_ReAppend(wchar_t *lfirst, wchar_t *lsecond, int len)
{
	size_t l1 = lfirst ? mir_wstrlen(lfirst) : 0;
	size_t l2 = (len ? len : (mir_wstrlen(lsecond) + 1));
	wchar_t *buf = (wchar_t *)mir_alloc((l1 + l2 + 1)*sizeof(wchar_t));
	if (lfirst) memmove(buf, lfirst, l1*sizeof(wchar_t));
	memmove(buf + l1, lsecond, l2*sizeof(wchar_t));
	mir_free(lfirst);
	if (len) buf[l1 + l2] = '\0';
	return buf;
}

wchar_t* ske_ReplaceVar(wchar_t *var)
{
	if (!var) return mir_wstrdup(L"");
	if (!mir_wstrcmpi(var, L"Profile")) {
		char buf[MAX_PATH] = { 0 };
		Profile_GetNameA(MAX_PATH, buf);

		char *p = strrchr(buf, '.');
		if (p) *p = 0;

		mir_free(var);
		return mir_a2u(buf);
	}

	mir_free(var);
	return mir_wstrdup(L"");
}

wchar_t *ske_ParseText(wchar_t *stzText)
{
	size_t len = mir_wstrlen(stzText);
	wchar_t *result = nullptr;
	size_t stpos = 0, curpos = 0;

	while (curpos < len) {
		//1 find first %
		while (curpos < len && stzText[curpos] != (wchar_t)'%')
			curpos++;
		if (curpos < len) { //% found
			if (curpos - stpos > 0)
				result = ske_ReAppend(result, stzText + stpos, int(curpos - stpos));
			stpos = curpos + 1;
			curpos++;
			//3 find second %
			while (curpos < len && stzText[curpos] != (wchar_t)'%')
				curpos++;
			if (curpos >= len)
				break;
			if (curpos - stpos > 0) {
				wchar_t *var = (wchar_t *)mir_alloc((curpos - stpos + 1)*sizeof(wchar_t));
				memcpy(var, stzText + stpos, (curpos - stpos)*sizeof(wchar_t));
				var[curpos - stpos] = (wchar_t)'\0';
				var = ske_ReplaceVar(var);
				result = ske_ReAppend(result, var, 0);
				mir_free(var);
			}
			else result = ske_ReAppend(result, L"%", 0);

			curpos++;
			stpos = curpos;
		}
		else {
			if (curpos - stpos > 0)
				result = ske_ReAppend(result, stzText + stpos, int(curpos - stpos));
			break;
		}
	}
	return result;
}
/*
*   Parse text object string, find glyph object and add text to it.
*   szGlyphTextID and Define string is:
*   t[szGlyphTextID] = s[HostObjectID],[Left],[Top],[Right],[Bottom],[LTRBHV],[FontID],[Color1],[reservedforColor2],[Text]
*/

static void ske_AddParseTextGlyphObject(char *szGlyphTextID, char *szDefineString, SKINOBJECTSLIST *Skin)
{
	char buf[255] = { 0 };
	GetParamN(szDefineString, buf, sizeof(buf), 0, ',', TRUE);
	if (buf[0] == 0)
		return;

	GLYPHTEXT *glText = (GLYPHTEXT*)mir_calloc(sizeof(GLYPHTEXT));
	glText->szGlyphTextID = mir_strdup(szGlyphTextID);
	glText->szObjectName = mir_strdup(buf);
	glText->iLeft = atoi(GetParamN(szDefineString, buf, sizeof(buf), 1, ',', TRUE));
	glText->iTop = atoi(GetParamN(szDefineString, buf, sizeof(buf), 2, ',', TRUE));
	glText->iRight = atoi(GetParamN(szDefineString, buf, sizeof(buf), 3, ',', TRUE));
	glText->iBottom = atoi(GetParamN(szDefineString, buf, sizeof(buf), 4, ',', TRUE));
	{
		memset(buf, 0, 6);
		GetParamN(szDefineString, buf, sizeof(buf), 5, ',', TRUE);
		buf[0] &= 95; buf[1] &= 95; buf[2] &= 95; buf[3] &= 95; buf[4] &= 95; buf[5] &= 95;   //to uppercase: &01011111 (0-95)
		glText->RelativeFlags =
			(buf[0] == 'C' ? 1 : ((buf[0] == 'R') ? 2 : 0))       //[BC][RC][BC][RC] --- Left relative
			| (buf[1] == 'C' ? 4 : ((buf[1] == 'B') ? 8 : 0))       //  |   |   |--------- Top relative
			| (buf[2] == 'C' ? 16 : ((buf[2] == 'R') ? 32 : 0))     //  |   |--------------Right relative
			| (buf[3] == 'C' ? 64 : ((buf[3] == 'B') ? 128 : 0));   //  |------------------Bottom relative
		glText->dwFlags = (buf[4] == 'C' ? DT_CENTER : ((buf[4] == 'R') ? DT_RIGHT : DT_LEFT))
			| (buf[5] == 'C' ? DT_VCENTER : ((buf[5] == 'B') ? DT_BOTTOM : DT_TOP));
	}
	glText->szFontID = mir_strdup(GetParamN(szDefineString, buf, sizeof(buf), 6, ',', TRUE));

	glText->dwColor = ske_HexToARGB(GetParamN(szDefineString, buf, sizeof(buf), 7, ',', TRUE));
	glText->dwShadow = ske_HexToARGB(GetParamN(szDefineString, buf, sizeof(buf), 8, ',', TRUE));
	glText->stValueText = mir_a2u(GetParamN(szDefineString, buf, sizeof(buf), 9, ',', TRUE));
	glText->stText = ske_ParseText(glText->stValueText);

	if (!Skin->pTextList)
		Skin->pTextList = List_Create(0, 1);
	List_InsertPtr(Skin->pTextList, glText);
}


/*
*   Parse font definition string.
*   szGlyphTextID and Define string is:
*   f[szFontID] = s[FontTypefaceName],[size],[BIU]
*/
static void ske_AddParseSkinFont(char *szFontID, char *szDefineString)
{
	SKINFONT *sf = (SKINFONT*)mir_calloc(sizeof(SKINFONT));
	if (!sf)
		return;

	LOGFONTA logfont = { 0 };
	logfont.lfCharSet = DEFAULT_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	char buf[255];
	strncpy_s(logfont.lfFaceName, GetParamN(szDefineString, buf, sizeof(buf), 0, ',', TRUE), _TRUNCATE);
	logfont.lfHeight = atoi(GetParamN(szDefineString, buf, sizeof(buf), 1, ',', TRUE));
	if (logfont.lfHeight < 0) {
		HDC hdc = CreateCompatibleDC(nullptr);
		logfont.lfHeight = (long)-MulDiv(logfont.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		DeleteDC(hdc);
	}
	logfont.lfHeight = -logfont.lfHeight;
	GetParamN(szDefineString, buf, sizeof(buf), 2, ',', TRUE);
	buf[0] &= 95; buf[1] &= 95; buf[2] &= 95;
	logfont.lfWeight = (buf[0] == 'B') ? FW_BOLD : FW_NORMAL;
	logfont.lfItalic = (buf[1] == 'I') ? 1 : 0;
	logfont.lfUnderline = (buf[2] == 'U') ? 1 : 0;

	sf->hFont = CreateFontIndirectA(&logfont);
	if (sf->hFont) {
		sf->szFontID = mir_strdup(szFontID);
		if (!gl_plSkinFonts)
			gl_plSkinFonts = List_Create(0, 1);
		if (gl_plSkinFonts)
			List_Insert(gl_plSkinFonts, sf, gl_plSkinFonts->realCount);
		else
			mir_free(sf);
	}
	else mir_free(sf);
}

/*
 *   ske_CheckHasAlfaChannel - checks if image has at least one uint8_t in alpha chennel
 *                  that is not a 0. (is image real 32 bit or just 24 bit)
 */
static BOOL ske_CheckHasAlfaChannel(uint8_t *from, int widthByte, int height)
{
	uint32_t *pt = (uint32_t*)from;
	for (int j = 0; j < height; j++) {
		uint8_t *add = (uint8_t*)pt + widthByte;
		while (pt < (uint32_t*)add) {
			if ((*pt & 0xFF000000) != 0)
				return TRUE;
			pt++;
		}
		pt = (uint32_t*)(from + widthByte*j);
	}
	return FALSE;
}

/*
 *   ske_CheckIconHasMask - checks if mask image has at least one that is not a 0.
 *                  Not sure is ir required or not
 */
static BOOL ske_CheckIconHasMask(uint8_t *from)
{
	for (int i = 0; i < 16 * 16 / 8; i++)
		if (from[i] != 0)
			return TRUE;

	return FALSE;
}

/*
 *   ske_GetMaskBit - return value of apropriate mask bit in line at x position
 */
static BOOL ske_GetMaskBit(uint8_t *line, int x)
{
	return ((*(line + (x >> 3)))&(0x01 << (7 - (x & 0x07)))) != 0;
}

/*
 *    ske_Blend  - alpha ske_Blend ARGB values of 2 pixels. X1 - underlaying,
 *            X2 - overlaying points.
 */

static uint32_t ske_Blend(uint32_t X1, uint32_t X2, uint8_t alpha)
{
	uint8_t a1 = (uint8_t)(X1 >> 24);
	uint8_t a2 = (uint8_t)(((X2 >> 24)*alpha) >> 8);
	uint8_t r1 = (uint8_t)(X1 >> 16);
	uint8_t r2 = (uint8_t)(X2 >> 16);
	uint8_t g1 = (uint8_t)(X1 >> 8);
	uint8_t g2 = (uint8_t)(X2 >> 8);
	uint8_t b1 = (uint8_t)(X1);
	uint8_t b2 = (uint8_t)(X2);

	uint8_t a_1 = ~a1;
	uint8_t a_2 = ~a2;
	uint16_t am = (uint16_t)a1*a_2;

	/*  it is possible to use >>8 instead of /255 but it is require additional
	*   checking of alphavalues
	*/
	uint16_t ar = a1 + (((uint16_t)a_1*a2) / 255);
	// if a2 more than 0 than result should be more
	// or equal (if a1 == 0) to a2, else in combination
	// with mask we can get here black points

	ar = (a2 > ar) ? a2 : ar;

	if (ar == 0) return 0;

	uint16_t arm = ar * 255;
	uint16_t rr = (((uint16_t)r1*am + (uint16_t)r2*a2 * 255)) / arm;
	uint16_t gr = (((uint16_t)g1*am + (uint16_t)g2*a2 * 255)) / arm;
	uint16_t br = (((uint16_t)b1*am + (uint16_t)b2*a2 * 255)) / arm;
	return (ar << 24) | (rr << 16) | (gr << 8) | br;
}

/*
 *    CreateJoinedIcon  - creates new icon by drawing hTop over hBottom.
 */

HICON ske_CreateJoinedIcon(HICON hBottom, HICON hTop, uint8_t alpha)
{
	ICONINFO iNew = { 0 };
	ICONINFO iciBottom = { 0 };
	ICONINFO iciTop = { 0 };

	BITMAP bmp_top = { 0 };
	BITMAP bmp_top_mask = { 0 };

	BITMAP bmp_bottom = { 0 };
	BITMAP bmp_bottom_mask = { 0 };

	HDC tempDC = CreateCompatibleDC(nullptr);

	uint8_t *ptPixels;
	HBITMAP nImage = ske_CreateDIB32Point(16, 16, (void**)&ptPixels);
	HBITMAP oImage = (HBITMAP)SelectObject(tempDC, nImage);

	GetIconInfo(hBottom, &iciBottom);
	GetObject(iciBottom.hbmColor, sizeof(BITMAP), &bmp_bottom);
	GetObject(iciBottom.hbmMask, sizeof(BITMAP), &bmp_bottom_mask);

	GetIconInfo(hTop, &iciTop);
	GetObject(iciTop.hbmColor, sizeof(BITMAP), &bmp_top);
	GetObject(iciTop.hbmMask, sizeof(BITMAP), &bmp_top_mask);

	if (bmp_bottom.bmBitsPixel == 32 && bmp_top.bmBitsPixel == 32) {
		uint8_t *BottomBuffer, *TopBuffer, *BottomMaskBuffer, *TopMaskBuffer;
		uint8_t *bb, *tb, *bmb, *tmb;
		uint8_t *db = ptPixels;
		int vstep_d = 16 * 4;
		int vstep_b = bmp_bottom.bmWidthBytes;
		int vstep_t = bmp_top.bmWidthBytes;
		int vstep_bm = bmp_bottom_mask.bmWidthBytes;
		int vstep_tm = bmp_top_mask.bmWidthBytes;
		alpha = alpha ? alpha : 255;
		if (bmp_bottom.bmBits) bb = BottomBuffer = (uint8_t*)bmp_bottom.bmBits;
		else {
			BottomBuffer = (uint8_t*)mir_alloc(bmp_bottom.bmHeight*bmp_bottom.bmWidthBytes);
			GetBitmapBits(iciBottom.hbmColor, bmp_bottom.bmHeight*bmp_bottom.bmWidthBytes, BottomBuffer);
			bb = BottomBuffer + vstep_b*(bmp_bottom.bmHeight - 1);
			vstep_b = -vstep_b;
		}

		if (bmp_top.bmBits) tb = TopBuffer = (uint8_t*)bmp_top.bmBits;
		else {
			TopBuffer = (uint8_t*)mir_alloc(bmp_top.bmHeight*bmp_top.bmWidthBytes);
			GetBitmapBits(iciTop.hbmColor, bmp_top.bmHeight*bmp_top.bmWidthBytes, TopBuffer);
			tb = TopBuffer + vstep_t*(bmp_top.bmHeight - 1);
			vstep_t = -vstep_t;
		}

		if (bmp_bottom_mask.bmBits) {
			BottomMaskBuffer = (uint8_t*)bmp_bottom_mask.bmBits;
			bmb = BottomMaskBuffer;
		}
		else {
			BottomMaskBuffer = (uint8_t*)mir_alloc(bmp_bottom_mask.bmHeight*bmp_bottom_mask.bmWidthBytes);
			GetBitmapBits(iciBottom.hbmMask, bmp_bottom_mask.bmHeight*bmp_bottom_mask.bmWidthBytes, BottomMaskBuffer);
			bmb = BottomMaskBuffer + vstep_bm*(bmp_bottom_mask.bmHeight - 1);
			vstep_bm = -vstep_bm;

		}

		if (bmp_top_mask.bmBits) {
			TopMaskBuffer = (uint8_t*)bmp_top_mask.bmBits;
			tmb = TopMaskBuffer;
		}
		else {
			TopMaskBuffer = (uint8_t*)mir_alloc(bmp_top_mask.bmHeight*bmp_top_mask.bmWidthBytes);
			GetBitmapBits(iciTop.hbmMask, bmp_top_mask.bmHeight*bmp_top_mask.bmWidthBytes, TopMaskBuffer);
			tmb = TopMaskBuffer + vstep_tm*(bmp_top_mask.bmHeight - 1);
			vstep_tm = -vstep_tm;
		}

		BOOL topHasAlpha = ske_CheckHasAlfaChannel(TopBuffer, bmp_top.bmWidthBytes, bmp_top.bmHeight);
		BOOL bottomHasAlpha = ske_CheckHasAlfaChannel(BottomBuffer, bmp_bottom.bmWidthBytes, bmp_bottom.bmHeight);
		BOOL topHasMask = ske_CheckIconHasMask(TopMaskBuffer);
		BOOL bottomHasMask = ske_CheckIconHasMask(BottomMaskBuffer);
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				BOOL mask_b = ske_GetMaskBit(bmb, x);
				BOOL mask_t = ske_GetMaskBit(tmb, x);
				uint32_t bottom_d = ((uint32_t*)bb)[x];
				uint32_t top_d = ((uint32_t*)tb)[x];
				if (topHasMask) {
					if (mask_t == 1 && !topHasAlpha)  top_d &= 0xFFFFFF;
					else if (!topHasAlpha) top_d |= 0xFF000000;
				}
				if (bottomHasMask) {
					if (mask_b == 1 && !bottomHasAlpha) bottom_d &= 0xFFFFFF;
					else if (!bottomHasAlpha) bottom_d |= 0xFF000000;
				}
				((uint32_t*)db)[x] = ske_Blend(bottom_d, top_d, alpha);
			}
			bb += vstep_b;
			tb += vstep_t;
			bmb += vstep_bm;
			tmb += vstep_tm;
			db += vstep_d;
		}

		if (!bmp_bottom.bmBits) mir_free(BottomBuffer);
		if (!bmp_top.bmBits) mir_free(TopBuffer);
		if (!bmp_bottom_mask.bmBits) mir_free(BottomMaskBuffer);
		if (!bmp_top_mask.bmBits) mir_free(TopMaskBuffer);
	}
	else {
		ske_DrawIconEx(tempDC, 0, 0, hBottom, 16, 16, 0, nullptr, DI_NORMAL);
		ske_DrawIconEx(tempDC, 0, 0, hTop, 16, 16, 0, nullptr, DI_NORMAL | (alpha << 24));
	}

	DeleteObject(iciBottom.hbmColor);
	DeleteObject(iciTop.hbmColor);
	DeleteObject(iciBottom.hbmMask);
	DeleteObject(iciTop.hbmMask);

	SelectObject(tempDC, oImage);
	DeleteDC(tempDC);

	uint8_t p[32] = { 0 };
	HBITMAP nMask = CreateBitmap(16, 16, 1, 1, (void*)&p);
	{
		HDC tempDC2 = CreateCompatibleDC(nullptr);
		HDC tempDC3 = CreateCompatibleDC(nullptr);
		HBITMAP hbm = CreateCompatibleBitmap(tempDC3, 16, 16);
		HBITMAP obmp = (HBITMAP)SelectObject(tempDC2, nMask);
		HBITMAP obmp2 = (HBITMAP)SelectObject(tempDC3, hbm);
		DrawIconEx(tempDC2, 0, 0, hBottom, 16, 16, 0, nullptr, DI_MASK);
		DrawIconEx(tempDC3, 0, 0, hTop, 16, 16, 0, nullptr, DI_MASK);
		BitBlt(tempDC2, 0, 0, 16, 16, tempDC3, 0, 0, SRCAND);
		SelectObject(tempDC2, obmp);
		SelectObject(tempDC3, obmp2);
		DeleteObject(hbm);
		DeleteDC(tempDC2);
		DeleteDC(tempDC3);
	}
	iNew.fIcon = TRUE;
	iNew.hbmColor = nImage;
	iNew.hbmMask = nMask;
	HICON res = CreateIconIndirect(&iNew);
	DeleteObject(nImage);
	DeleteObject(nMask);
	return res;
}

#define NEWJOINEDSTR(destination, first, separator, last) \
	destination = (char*)alloca(mir_strlen(first)+mir_strlen(separator)+mir_strlen(last)+1);	\
	if (destination) { \
		*destination = '\0'; \
		mir_strcat(destination,first); \
		mir_strcat(destination,separator); \
		mir_strcat(destination,last); \
			}

#define SKINSETSECTION "SkinnedSettings"

BOOL SkinDBGetContactSetting(MCONTACT hContact, const char *szSection, const char *szKey, DBVARIANT *retdbv, BOOL *bSkinned)
{
	if (!hContact) {  //only for not contact settings
		char *szSkinKey;
		NEWJOINEDSTR(szSkinKey, szSection, "@", szKey);
		if (!db_get(hContact, SKINSETSECTION, szSkinKey, retdbv)) {
			if (bSkinned) *bSkinned = TRUE;
			return FALSE;
		}
	}
	// not skinned
	if (bSkinned) bSkinned = FALSE;
	return db_get(hContact, szSection, szKey, retdbv);
}

uint8_t SkinDBGetContactSettingByte(MCONTACT hContact, const char *szSection, const char *szKey, uint8_t bDefault)
{
	DBVARIANT dbv = { 0 };
	BOOL bSkinned = FALSE;
	if (!SkinDBGetContactSetting(hContact, szSection, szKey, &dbv, &bSkinned)) {
		if (dbv.type == DBVT_BYTE) {
			uint8_t retVal = dbv.bVal;
			db_free(&dbv);
			return retVal;
		}
		else {
			db_free(&dbv);
			if (!bSkinned)
				return db_get_b(hContact, szSection, szKey, bDefault);
		}
	}
	return bDefault;
}

uint16_t SkinDBGetContactSettingWord(MCONTACT hContact, const char *szSection, const char *szKey, uint16_t wDefault)
{
	BOOL bSkinned = FALSE;
	DBVARIANT dbv = { 0 };
	if (!SkinDBGetContactSetting(hContact, szSection, szKey, &dbv, &bSkinned)) {
		if (dbv.type == DBVT_WORD) {
			uint16_t retVal = dbv.wVal;
			db_free(&dbv);
			return retVal;
		}
		db_free(&dbv);
		if (!bSkinned)
			return db_get_w(hContact, szSection, szKey, wDefault);
	}
	return wDefault;
}

uint32_t SkinDBGetContactSettingDword(MCONTACT hContact, const char *szSection, const char *szKey, uint32_t dwDefault)
{
	DBVARIANT dbv = { 0 };
	BOOL bSkinned = FALSE;
	if (!SkinDBGetContactSetting(hContact, szSection, szKey, &dbv, &bSkinned)) {
		if (dbv.type == DBVT_DWORD) {
			uint32_t retVal = dbv.dVal;
			db_free(&dbv);
			return retVal;
		}
		db_free(&dbv);
		if (!bSkinned)
			return db_get_dw(hContact, szSection, szKey, dwDefault);
	}
	return dwDefault;
}
