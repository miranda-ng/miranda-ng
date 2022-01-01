/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

#ifdef _ASSERT
#undef _ASSERT
#endif
#define _ASSERT(n)
// http://msdn.microsoft.com/library/default.asp?url = /library/en-us/winui/winui/windowsuserinterface/resources/introductiontoresources/resourcereference/resourcestructures/newheader.asp
typedef struct
{
	uint16_t    Reserved;
	uint16_t    ResType;
	uint16_t    ResCount;
}
	NEWHEADER;

#define MAGIC_ICON       0
#define MAGIC_ICO1       1
#define MAGIC_CUR        2
#define MAGIC_BMP        ((uint16_t)'B'+((uint16_t)'M'<<8))

#define MAGIC_ANI1       ((uint16_t)'R'+((uint16_t)'I'<<8))
#define MAGIC_ANI2       ((uint16_t)'F'+((uint16_t)'F'<<8))
#define MAGIC_ANI3       ((uint16_t)'A'+((uint16_t)'C'<<8))
#define MAGIC_ANI4       ((uint16_t)'O'+((uint16_t)'N'<<8))

#define VER30            0x00030000

void* _RelativeVirtualAddresstoPtr(IMAGE_DOS_HEADER *pDosHeader, uint32_t rva)
{
	IMAGE_NT_HEADERS *pPE = (IMAGE_NT_HEADERS*)((uint8_t*)pDosHeader + pDosHeader->e_lfanew);
	IMAGE_SECTION_HEADER *pSection = IMAGE_FIRST_SECTION(pPE);

	for (int i = 0; i < pPE->FileHeader.NumberOfSections; i++) {
		IMAGE_SECTION_HEADER* cSection = &pSection[i];
		uint32_t size = cSection->Misc.VirtualSize ? cSection->Misc.VirtualSize : cSection->SizeOfRawData;

		if (rva >= cSection->VirtualAddress && rva < cSection->VirtualAddress + size)
			return (LPBYTE)pDosHeader + cSection->PointerToRawData + (rva - cSection->VirtualAddress);
	}

	return nullptr;
}

void* _GetResourceTable(IMAGE_DOS_HEADER* pDosHeader)
{
	IMAGE_NT_HEADERS *pPE = (IMAGE_NT_HEADERS*)((uint8_t*)pDosHeader + pDosHeader->e_lfanew);
	if (pPE->Signature != IMAGE_NT_SIGNATURE || pPE->FileHeader.SizeOfOptionalHeader < 2)
		return nullptr;

	// The DataDirectory is an array of 16 structures.
	// Each array entry has a predefined meaning for what it refers to.

	switch (pPE->OptionalHeader.Magic) {
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
		if (pPE->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER32))
			return _RelativeVirtualAddresstoPtr(pDosHeader, ((PIMAGE_NT_HEADERS32)pPE)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
		break;

	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		if (pPE->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64))
			return _RelativeVirtualAddresstoPtr(pDosHeader, ((PIMAGE_NT_HEADERS64)pPE)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
		break;
	}

	return nullptr;
}

IMAGE_RESOURCE_DIRECTORY_ENTRY* _FindResourceBase(void *prt, uint32_t resType, int *pCount)
{
	IMAGE_RESOURCE_DIRECTORY *pDir = (IMAGE_RESOURCE_DIRECTORY*)prt;
	int i;

	*pCount = 0;

	int count = pDir->NumberOfIdEntries + pDir->NumberOfNamedEntries;
	IMAGE_RESOURCE_DIRECTORY_ENTRY *pRes = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pDir + 1);

	for (i = 0; i < count; i++)
		if (pRes[i].Name == resType)
			break;

	if (i == count)
		return nullptr;

	pDir = (IMAGE_RESOURCE_DIRECTORY*)((LPBYTE)prt + (pRes[i].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY));
	count = pDir->NumberOfIdEntries + pDir->NumberOfNamedEntries;
	*pCount = count;
	return (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pDir + 1);
}

int _FindResourceCount(void *prt, uint32_t resType)
{
	int count;
	_FindResourceBase(prt, resType, &count);
	return count;
}

void* _FindResource(IMAGE_DOS_HEADER *pDosHeader, void *prt, int resIndex, uint32_t resType, uint32_t *pcbSize)
{
	int count, index = 0;

	IMAGE_RESOURCE_DIRECTORY_ENTRY *pRes = _FindResourceBase(prt, resType, &count);
	if (resIndex < 0) {
		for (index = 0; index < count; index++)
			if (pRes[index].Name == (uint32_t)(-resIndex))
				break;
	}
	else index = resIndex;

	if (index >= count)
		return nullptr;

	if (pRes[index].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY) {
		IMAGE_RESOURCE_DIRECTORY* pDir;
		pDir = (IMAGE_RESOURCE_DIRECTORY*)((LPBYTE)prt + (pRes[index].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY));
		pRes = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pDir+1);
		index = 0;
	}

	if (pRes[index].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		return nullptr;

	IMAGE_RESOURCE_DATA_ENTRY *pEntry = (IMAGE_RESOURCE_DATA_ENTRY*)((LPBYTE)prt + pRes[index].OffsetToData);
	*pcbSize = pEntry->Size;
	return _RelativeVirtualAddresstoPtr(pDosHeader, pEntry->OffsetToData);
}

UINT _ExtractFromExe(HANDLE hFile, int iconIndex, int cxIconSize, int cyIconSize, HICON *phicon, UINT flags)
{
	int retval = 0;
	uint32_t fileLen = GetFileSize(hFile, nullptr);

	HANDLE pFile = nullptr, hFileMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (hFileMap == nullptr)
		goto cleanup;

	pFile = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	if (pFile == nullptr)
		goto cleanup;

	IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)(void*)pFile;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) goto cleanup;
	if (pDosHeader->e_lfanew <= 0) goto cleanup;
	if ((uint32_t)(pDosHeader->e_lfanew) >= fileLen) goto cleanup;

	void *pRes = _GetResourceTable(pDosHeader);
	if (!pRes) goto cleanup;
	if (!phicon) {
		retval = _FindResourceCount(pRes, (ULONG_PTR)RT_GROUP_ICON);
		goto cleanup;
	}

	uint32_t cbSize = 0;
	NEWHEADER *pIconDir = (NEWHEADER*)_FindResource(pDosHeader, pRes, iconIndex, (ULONG_PTR)RT_GROUP_ICON, &cbSize);
	if (!pIconDir) goto cleanup;
	if (pIconDir->Reserved || pIconDir->ResType != RES_ICON) goto cleanup;

	int idIcon = LookupIconIdFromDirectoryEx((LPBYTE)pIconDir, TRUE, cxIconSize, cyIconSize, flags);
	LPBITMAPINFOHEADER pIcon = (LPBITMAPINFOHEADER)_FindResource(pDosHeader, pRes, -idIcon, (ULONG_PTR)RT_ICON, &cbSize);
	if (!pIcon) goto cleanup;

	if (pIcon->biSize != sizeof(BITMAPINFOHEADER) && pIcon->biSize != sizeof(BITMAPCOREHEADER)) {
		_ASSERT(0);
		goto cleanup;
	}

	*phicon = CreateIconFromResourceEx((LPBYTE)pIcon, cbSize, TRUE, VER30, cxIconSize, cyIconSize, flags);
	retval = 1;

cleanup:
	if (pFile)
		UnmapViewOfFile(pFile);
	if (hFileMap != INVALID_HANDLE_VALUE)
		CloseHandle(hFileMap);
	return retval;
}

UINT _ExtractFromICO(LPCTSTR pFileName, int iconIndex, int cxIcon, int cyIcon, HICON* phicon, UINT flags)
{
	if (iconIndex >= 1)
		return 0;

	//  do we just want a count?
	if (!phicon)
		return 1;

	flags |= LR_LOADFROMFILE;
	HICON hicon = (HICON)LoadImage(nullptr, pFileName, IMAGE_ICON, cxIcon, cyIcon, flags);
	if (!hicon)
		return 0;

	*phicon = hicon;
	return 1;
}

UINT _ExtractIconEx(LPCTSTR lpszFile, int iconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT flags)
{
	UINT res = 0;
	if (cxIcon == g_iIconX && cyIcon == g_iIconY)
		res = ExtractIconEx(lpszFile, iconIndex, phicon, nullptr, 1);
	else if (cxIcon == g_iIconSX && cyIcon == g_iIconSY)
		res = ExtractIconEx(lpszFile, iconIndex, nullptr, phicon, 1);
	else if (cxIcon == 0 || cyIcon == 0)
		res = ExtractIconEx(lpszFile, iconIndex, nullptr, phicon, 1);
	// check if the api succeded, if not try our method too
	if (res)
		return res;

	HANDLE hFile = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	// failed to read file signature
	DWORD read = 0;
	uint16_t magic[6];
	if (!ReadFile(hFile, &magic, sizeof(magic), &read, nullptr) || (read != sizeof(magic))) {
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
