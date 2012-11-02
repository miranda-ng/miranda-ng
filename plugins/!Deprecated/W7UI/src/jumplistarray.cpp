#include "headers.h"

#pragma pack(push, 1)
typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    WORD           idReserved;   // Reserved (must be 0)
    WORD           idType;       // Resource Type (1 for icons)
    WORD           idCount;      // How many images?
    ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;
#pragma pack(pop)

static void SaveIconToFile(HICON hIcon, TCHAR *szFile)
{
	ICONINFO iconInfo = {0};
	BITMAP bmpColor, bmpMask;
	HANDLE hFile = 0;
	HANDLE hMap = 0;
	BYTE *pFile = 0;

	GetIconInfo(hIcon, &iconInfo);
	GetObject(iconInfo.hbmColor, sizeof(bmpColor), &bmpColor);
	GetObject(iconInfo.hbmMask, sizeof(bmpMask), &bmpMask);

	do
	{
		if (bmpColor.bmBitsPixel <= 8) break;

		DWORD dwColorSize = bmpColor.bmWidthBytes * bmpColor.bmHeight;
		DWORD dwMaskSize = bmpMask.bmWidthBytes * bmpMask.bmHeight;
		DWORD dwFileSize = sizeof(ICONDIR) + sizeof(BITMAPINFOHEADER) + dwColorSize + dwMaskSize;

		hFile = CreateFile(szFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) break;
		hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);
		if (!hMap) break;
		pFile = (BYTE *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0 ,0);
		if (!pFile) break;

		ICONDIR iconDir = {0};
		iconDir.idCount = 1;
		iconDir.idType = 1;
		iconDir.idEntries[0].bWidth = bmpColor.bmWidth;
		iconDir.idEntries[0].bHeight = bmpColor.bmHeight;
		iconDir.idEntries[0].bColorCount = 0;
		iconDir.idEntries[0].wPlanes = bmpColor.bmPlanes;
		iconDir.idEntries[0].wBitCount = bmpColor.bmBitsPixel;
		iconDir.idEntries[0].dwBytesInRes = sizeof(BITMAPINFOHEADER) + dwColorSize + dwMaskSize;
		iconDir.idEntries[0].dwImageOffset = sizeof(ICONDIR);
		MoveMemory(pFile, &iconDir, sizeof(ICONDIR));

		BITMAPINFOHEADER iconBmp = {0};
		iconBmp.biSize = sizeof(iconBmp);
		iconBmp.biWidth = bmpColor.bmWidth;
		iconBmp.biHeight = bmpColor.bmHeight + bmpMask.bmHeight;
		iconBmp.biPlanes = bmpColor.bmPlanes;
		iconBmp.biBitCount = bmpColor.bmBitsPixel;
		iconBmp.biSizeImage = dwColorSize + dwMaskSize;
		MoveMemory(pFile + sizeof(ICONDIR), &iconBmp, sizeof(BITMAPINFOHEADER));

		BYTE *buf = (BYTE *)mir_alloc(dwColorSize);
		GetBitmapBits(iconInfo.hbmColor, dwColorSize, buf);
		for (int row = 0; row < bmpColor.bmHeight; ++row)
		{
			MoveMemory(
				pFile + sizeof(ICONDIR) + sizeof(BITMAPINFOHEADER) + row * bmpColor.bmWidthBytes,
				buf + (bmpColor.bmHeight - row - 1) * bmpColor.bmWidthBytes,
				bmpColor.bmWidthBytes);
		}
		mir_free(buf);

		GetBitmapBits(iconInfo.hbmMask, dwMaskSize, pFile + sizeof(ICONDIR) + sizeof(BITMAPINFOHEADER) + dwColorSize);
	} while(0);

	DeleteObject(iconInfo.hbmColor);
	DeleteObject(iconInfo.hbmMask);

	if (pFile) UnmapViewOfFile(pFile);
	if (hMap) CloseHandle(hMap);
	if (hFile && hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
}

CJumpListArray::CJumpListArray()
{
	CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER, IID_IObjectCollection, (void **)&m_pObjects);
}

CJumpListArray::~CJumpListArray()
{
	if (m_pObjects) m_pObjects->Release();
}

void CJumpListArray::AddItemImpl(TCHAR *icon, int iIcon, TCHAR *title, TCHAR *path, TCHAR *args)
{
	IShellLink *link = NewShellLink(icon, iIcon, title, path, args);
	m_pObjects->AddObject(link);
	link->Release();
}

void CJumpListArray::AddItem(char *mir_icon, TCHAR *title, TCHAR *path, TCHAR *args)
{
	TCHAR icon[MAX_PATH]; int iIcon;
	LoadMirandaIcon(mir_icon, icon, &iIcon);
	AddItemImpl(icon, iIcon, title, path, args);
}

void CJumpListArray::AddItem(int skinicon, TCHAR *title, TCHAR *path, TCHAR *args)
{
	TCHAR icon[MAX_PATH]; int iIcon;
	LoadMirandaIcon(skinicon, icon, &iIcon);
	AddItemImpl(icon, iIcon, title, path, args);
}

void CJumpListArray::AddItem(char *proto, int status, TCHAR *title, TCHAR *path, TCHAR *args)
{
	TCHAR icon[MAX_PATH]; int iIcon;
	LoadMirandaIcon(proto, status, icon, &iIcon);
	AddItemImpl(icon, iIcon, title, path, args);
}

IObjectArray *CJumpListArray::GetArray()
{
	IObjectArray *result = NULL;
	m_pObjects->QueryInterface(IID_IObjectArray, (void **)&result);
	return result;
}

bool CJumpListArray::LoadMirandaIcon(char *mir_icon, TCHAR *icon, int *id)
{
	*id = 0;

	TCHAR *path = Utils_ReplaceVarsT(_T("%miranda_userdata%\\w7ui.IconCache"));
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	TCHAR *name = mir_a2t(mir_icon);
	for (TCHAR *ch = name; *ch; ++ch) if (_tcschr(_T("\\/:*?<>|"), *ch)) *ch = _T('_');
	mir_sntprintf(icon, MAX_PATH, _T("%s\\%s.ico"), path, name);

	mir_free(name);
	mir_free(path);

	HICON hIcon = Skin_GetIcon(mir_icon);
	SaveIconToFile(hIcon, icon);

	return true;
}

bool CJumpListArray::LoadMirandaIcon(int skinicon, TCHAR *icon, int *id)
{
	*id = 0;

	TCHAR *path = Utils_ReplaceVarsT(_T("%miranda_userdata%\\w7ui.IconCache"));
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);
	mir_sntprintf(icon, MAX_PATH, _T("%s\\skinicon$%d.ico"), path, skinicon);
	mir_free(path);

	HICON hIcon = LoadSkinnedIcon(skinicon);
	SaveIconToFile(hIcon, icon);

	return true;
}

bool CJumpListArray::LoadMirandaIcon(char *proto, int status, TCHAR *icon, int *id)
{
	*id = 0;

	TCHAR *path = Utils_ReplaceVarsT(_T("%miranda_userdata%\\w7ui.IconCache"));
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);
	mir_sntprintf(icon, MAX_PATH, _T("%s\\skinprotoicon$") _T(TCHAR_STR_PARAM) _T("$%d.ico"), path, proto, status);
	mir_free(path);

	HICON hIcon = LoadSkinnedProtoIcon(proto, status);
	SaveIconToFile(hIcon, icon);

	return true;
}

IShellLink *CJumpListArray::NewShellLink(TCHAR *icon, int iIcon, TCHAR *title, TCHAR *path, TCHAR *args)
{
	IShellLink *pShellLink = NULL;
	CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pShellLink);

	pShellLink->SetPath(path);
	pShellLink->SetIconLocation(icon, iIcon);
	pShellLink->SetArguments(args);
	pShellLink->SetShowCmd(SW_SHOWDEFAULT);

	IPropertyStore *pPropStore = NULL;
	if (SUCCEEDED(pShellLink->QueryInterface(IID_IPropertyStore, (void **)&pPropStore)))
	{
		PROPVARIANT pv;

		InitPropVariantFromString(title, &pv);
		pPropStore->SetValue(PKEY_Title, pv);
		PropVariantClear(&pv);

		pPropStore->Commit();
		pPropStore->Release();
	}

	return pShellLink;
}