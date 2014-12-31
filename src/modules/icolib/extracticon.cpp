/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "..\..\core\commonheaders.h"

#ifdef _ASSERT
#undef _ASSERT
#endif
#define _ASSERT(n)
// http://msdn.microsoft.com/library/default.asp?url = /library/en-us/winui/winui/windowsuserinterface/resources/introductiontoresources/resourcereference/resourcestructures/newheader.asp
typedef struct
{
	WORD    Reserved;
	WORD    ResType;
	WORD    ResCount;
}
	NEWHEADER;

#define MAGIC_ICON       0
#define MAGIC_ICO1       1
#define MAGIC_CUR        2
#define MAGIC_BMP        ((WORD)'B'+((WORD)'M'<<8))

#define MAGIC_ANI1       ((WORD)'R'+((WORD)'I'<<8))
#define MAGIC_ANI2       ((WORD)'F'+((WORD)'F'<<8))
#define MAGIC_ANI3       ((WORD)'A'+((WORD)'C'<<8))
#define MAGIC_ANI4       ((WORD)'O'+((WORD)'N'<<8))

#define VER30            0x00030000

void* _RelativeVirtualAddresstoPtr(IMAGE_DOS_HEADER* pDosHeader, DWORD rva)
{
	IMAGE_NT_HEADERS* pPE = (IMAGE_NT_HEADERS*)((BYTE*)pDosHeader + pDosHeader->e_lfanew);
	IMAGE_SECTION_HEADER* pSection = IMAGE_FIRST_SECTION(pPE);
	int i;

	for (i=0; i < pPE->FileHeader.NumberOfSections; i++) {
		IMAGE_SECTION_HEADER* cSection = &pSection[i];
		DWORD size = cSection->Misc.VirtualSize ? cSection->Misc.VirtualSize : cSection->SizeOfRawData;

		if (rva >= cSection->VirtualAddress && rva < cSection->VirtualAddress + size)
			return (LPBYTE)pDosHeader + cSection->PointerToRawData + (rva - cSection->VirtualAddress);
	}

	return NULL;
}

void* _GetResourceTable(IMAGE_DOS_HEADER* pDosHeader)
{
	IMAGE_NT_HEADERS* pPE = (IMAGE_NT_HEADERS*)((BYTE*)pDosHeader + pDosHeader->e_lfanew);

	if (pPE->Signature != IMAGE_NT_SIGNATURE)
		return NULL;
	if (pPE->FileHeader.SizeOfOptionalHeader < 2)
		return NULL;

	// The DataDirectory is an array of 16 structures.
	// Each array entry has a predefined meaning for what it refers to.

	switch (pPE->OptionalHeader.Magic)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
		if (pPE->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER32))
			return _RelativeVirtualAddresstoPtr(pDosHeader, ((PIMAGE_NT_HEADERS32)pPE)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
		break;

	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		if (pPE->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64))
			return _RelativeVirtualAddresstoPtr(pDosHeader, ((PIMAGE_NT_HEADERS64)pPE)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
		break;
	}

	return NULL;
}

IMAGE_RESOURCE_DIRECTORY_ENTRY* _FindResourceBase(void* prt, int resType, int* pCount)
{
	IMAGE_RESOURCE_DIRECTORY* pDir = (IMAGE_RESOURCE_DIRECTORY*)prt;
	IMAGE_RESOURCE_DIRECTORY_ENTRY* pRes;
	int i, count;

	*pCount = 0;

	count = pDir->NumberOfIdEntries + pDir->NumberOfNamedEntries;
	pRes = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pDir+1);

	for (i=0; i < count; i++)
		if (pRes[i].Name == (DWORD)resType) break;

	if (i == count) return NULL;

	pDir = (IMAGE_RESOURCE_DIRECTORY*)((LPBYTE)prt +
		(pRes[i].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY));

	count = pDir->NumberOfIdEntries + pDir->NumberOfNamedEntries;
	*pCount = count;
	pRes = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pDir+1);

	return pRes;
}

int _FindResourceCount(void* prt, int resType)
{
	int count;

	_FindResourceBase(prt, resType, &count);
	return count;
}

void* _FindResource(IMAGE_DOS_HEADER* pDosHeader, void* prt, int resIndex, int resType, DWORD* pcbSize)
{
	int count, index = 0;
	IMAGE_RESOURCE_DIRECTORY_ENTRY* pRes;
	IMAGE_RESOURCE_DATA_ENTRY* pEntry;

	pRes = _FindResourceBase(prt, resType, &count);
	if (resIndex < 0) {
		for (index = 0; index < count; index++)
			if (pRes[index].Name == (DWORD)(-resIndex))
				break;
	}
	else index = resIndex;

	if (index >= count)
		return NULL;

	if (pRes[index].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY) {
		IMAGE_RESOURCE_DIRECTORY* pDir;
		pDir = (IMAGE_RESOURCE_DIRECTORY*)((LPBYTE)prt + (pRes[index].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY));
		pRes = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pDir+1);
		index = 0;
	}

	if (pRes[index].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		return NULL;

	pEntry = (IMAGE_RESOURCE_DATA_ENTRY*)((LPBYTE)prt + pRes[index].OffsetToData);
	*pcbSize = pEntry->Size;
	return _RelativeVirtualAddresstoPtr(pDosHeader, pEntry->OffsetToData);
}

UINT _ExtractFromExe(HANDLE hFile, int iconIndex, int cxIconSize, int cyIconSize, HICON *phicon, UINT flags)
{
	int retval = 0;
	DWORD fileLen = GetFileSize(hFile, NULL);
	HANDLE hFileMap = INVALID_HANDLE_VALUE, pFile = NULL;
	IMAGE_DOS_HEADER* pDosHeader;
	void* pRes;
	DWORD cbSize = 0;
	NEWHEADER* pIconDir;
	int idIcon;
	LPBITMAPINFOHEADER pIcon;
	//  UINT res = 0;

	hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hFileMap == NULL) goto cleanup;

	pFile = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	if (pFile == NULL) goto cleanup;

	pDosHeader = (IMAGE_DOS_HEADER*)(void*)pFile;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) goto cleanup;
	if (pDosHeader->e_lfanew <= 0) goto cleanup;
	if ((DWORD)(pDosHeader->e_lfanew) >= fileLen) goto cleanup;

	pRes = _GetResourceTable(pDosHeader);
	if (!pRes) goto cleanup;
	if (!phicon) {
		retval = _FindResourceCount(pRes, (int)RT_GROUP_ICON);
		goto cleanup;
	}

	pIconDir = (NEWHEADER*)_FindResource(pDosHeader, pRes, iconIndex, (int)RT_GROUP_ICON, &cbSize);
	if (!pIconDir) goto cleanup;
	if (pIconDir->Reserved || pIconDir->ResType != RES_ICON) goto cleanup;

	idIcon = LookupIconIdFromDirectoryEx((LPBYTE)pIconDir, TRUE, cxIconSize, cyIconSize, flags);
	pIcon = (LPBITMAPINFOHEADER)_FindResource(pDosHeader, pRes, -idIcon, (int)RT_ICON, &cbSize);
	if (!pIcon) goto cleanup;

	if (pIcon->biSize != sizeof(BITMAPINFOHEADER) && pIcon->biSize != sizeof(BITMAPCOREHEADER)) {
		_ASSERT(0);
		goto cleanup;
	}

	*phicon = CreateIconFromResourceEx((LPBYTE)pIcon, cbSize, TRUE, VER30, cxIconSize, cyIconSize, flags);
	retval = 1;

cleanup:
	if (pFile) UnmapViewOfFile(pFile);
	if (hFileMap != INVALID_HANDLE_VALUE) CloseHandle(hFileMap);

	return retval;
}

UINT _ExtractFromICO(LPCTSTR pFileName, int iconIndex, int cxIcon, int cyIcon, HICON* phicon, UINT flags)
{
	HICON hicon;

	if (iconIndex >= 1)
		return 0;

	//  do we just want a count?
	if (!phicon)
		return 1;

	flags |= LR_LOADFROMFILE;
	hicon = (HICON)LoadImage(NULL, pFileName, IMAGE_ICON, cxIcon, cyIcon, flags);
	if (!hicon)
		return 0;

	*phicon = hicon;
	return 1;
}

UINT _ExtractIconEx(LPCTSTR lpszFile, int iconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT flags)
{
	HANDLE hFile;
	WORD magic[6];
	DWORD read = 0;
	UINT res = 0;

	if (cxIcon == GetSystemMetrics(SM_CXICON) && cyIcon == GetSystemMetrics(SM_CYICON))
		res = ExtractIconEx(lpszFile, iconIndex, phicon, NULL, 1);
	else if (cxIcon == GetSystemMetrics(SM_CXSMICON) && cyIcon == GetSystemMetrics(SM_CYSMICON))
		res = ExtractIconEx(lpszFile, iconIndex, NULL, phicon, 1);
	else if (cxIcon == 0 || cyIcon == 0)
		res = ExtractIconEx(lpszFile, iconIndex, NULL, phicon, 1);
	// check if the api succeded, if not try our method too
	if (res) return res;

	hFile = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	// failed to read file signature
	if (!ReadFile(hFile, &magic, sizeof(magic), &read, NULL) || (read != sizeof(magic))) {
		CloseHandle(hFile);
		return 0;
	}

	switch (magic[0]) {
	case IMAGE_DOS_SIGNATURE:
		res = _ExtractFromExe(hFile, iconIndex, cxIcon, cyIcon, phicon, flags);
		break;

	case MAGIC_ANI1: //  ani cursors are RIFF file of type 'ACON'
		if (magic[1] == MAGIC_ANI2 && magic[4] == MAGIC_ANI3 && magic[5] == MAGIC_ANI4)
			res = _ExtractFromICO(lpszFile, iconIndex, cxIcon, cyIcon, phicon, flags);
		break;

	case MAGIC_ICON:
		if ((magic[1] == MAGIC_ICO1 || magic[1] == MAGIC_CUR) && magic[2] >= 1)
			res = _ExtractFromICO(lpszFile, iconIndex, cxIcon, cyIcon, phicon, flags);

		break;
	}

	CloseHandle(hFile);
	return res;
}
