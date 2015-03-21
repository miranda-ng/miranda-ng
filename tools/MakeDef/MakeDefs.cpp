// MakeDefs.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <io.h>
#include <stdio.h> 
#include <stdlib.h>

#include <windows.h>

#include "h_collection.h"
#include "h_util.h"

struct HExport
{
	char*	mName;
	char*	mAliasName;
	int	mOrdinalValue;
	int	mExportedByOrdinal;		
	long	mAddress;
};

struct HExportsColl1 : public SortedCollection<HExport>
{
				HExportsColl1() : 
					SortedCollection<HExport>( 0, 300 )
					{}

	virtual	int compare( const void* p1, const void* p2 ) const
				{	return _stricmp(( char* )p1, ( char* )p2 );
				}

	virtual	const void* keyOf( const void* pItem ) const
				{	return (( HExport* )pItem )->mName;
				}
};

struct HExportsColl2 : public SortedCollection<HExport>
{
				HExportsColl2() : 
					SortedCollection<HExport>( 0, 300 )
					{}

	virtual	int compare( const void* p1, const void* p2 ) const
				{	return _stricmp(( char* )p1, ( char* )p2 );
				}

	virtual	const void* keyOf( const void* pItem ) const
				{	return (( HExport* )pItem )->mAliasName;
				}
};

HExportsColl1 tOldDefs;
HExportsColl2 tOldDefsAliases;

int   gMaxOrdinal = -1;
int   gNewExports = 0;
int	gRebuild = 0;
int	gNoSort = 0;
char* gImportDllName = NULL;

HSmallAllocator gMM( 16000 );

//====[ Prints standard Win32 error message ]============================================

void PrintWin32Error(DWORD tErrorCode)
{
	char tBuffer[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, tBuffer, sizeof(tBuffer), NULL);
	fputs(tBuffer, stderr);
	fputc('\n', stderr);
}

//====[ Deletes all mapping resources ]==================================================

void UnloadFile(HANDLE hFile, HANDLE hMap, BYTE* pMap)
{
	if (pMap)
		::UnmapViewOfFile(pMap);

	if (hMap)
		::CloseHandle(hMap);

	if (hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(hFile);
}

//====[ Loads PE image file to memory mapping ]==========================================

bool LoadFile(LPCTSTR pszPathName, HANDLE& phFile, HANDLE& phMap, BYTE*& ppMap)
{
	phFile = INVALID_HANDLE_VALUE;
	phMap = NULL;
	ppMap = NULL;

	DWORD tErrorCode;

	phFile = ::CreateFile(pszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (phFile == INVALID_HANDLE_VALUE) {
		tErrorCode = GetLastError();
		fprintf(stderr, "Error opening '%s': ", pszPathName);
		PrintWin32Error(tErrorCode);
		return false;
	}

	if ((phMap = ::CreateFileMapping(phFile, NULL, PAGE_READONLY, 0, 0, NULL)) == NULL) {
		tErrorCode = GetLastError();
		fprintf(stderr, "Error creating file mapping over '%s': ", pszPathName);
		PrintWin32Error(tErrorCode);
		UnloadFile(phFile, phMap, ppMap);
		return false;
	}

	if ((ppMap = (BYTE*)::MapViewOfFile(phMap, FILE_MAP_READ, 0, 0, 0)) == NULL) {
		tErrorCode = GetLastError();
		fprintf(stderr, "Error reading mapped file '%s': ", pszPathName);
		PrintWin32Error(tErrorCode);
		UnloadFile(phFile, phMap, ppMap);
		return false;
	}

	return true;
}

//====[ Verifies DOS header ]============================================================

IMAGE_DOS_HEADER* GetImageDosHeader(void* pFilePtr)
{
	if (::IsBadReadPtr(pFilePtr, sizeof(IMAGE_DOS_HEADER)))
		return NULL;

	IMAGE_DOS_HEADER* pIDH = (IMAGE_DOS_HEADER*)pFilePtr;
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	return pIDH;
}

//====[ Verifies PE header ]=============================================================

IMAGE_NT_HEADERS* GetImageNtHeaders(IMAGE_DOS_HEADER* pIDH)
{
	IMAGE_NT_HEADERS* pINTH = (IMAGE_NT_HEADERS*)((UINT_PTR)pIDH + pIDH->e_lfanew);
	if (::IsBadReadPtr(pINTH, sizeof(UINT_PTR)))
		return NULL;

	if (pINTH->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	return pINTH;
}

//====[ Maps virtual address to file offset ]============================================

UINT_PTR RVAtoAddress(UINT_PTR dwRVA, IMAGE_NT_HEADERS* pINTH, void* pMap)
{
	IMAGE_SECTION_HEADER* pISH = IMAGE_FIRST_SECTION(pINTH);
	if (!pISH)
		return 0;

	for (DWORD i = 0; i < pINTH->FileHeader.NumberOfSections; i++) {
		UINT_PTR dwStart = pISH->VirtualAddress;
		UINT_PTR dwEnd = dwStart + pISH->SizeOfRawData;

		if (dwRVA >= dwStart && dwRVA < dwEnd)
			return (UINT_PTR)pMap + (UINT_PTR)pISH->PointerToRawData + dwRVA - dwStart;

		pISH++;
	}

	return 0;
}

static bool sCheckAddress(HExport* p1, void* p2)
{
	return p1->mAddress == *(long*)p2;
}

void IterateFunction(FILE* file, char* pszName, int ordinal = -1)
{
	if (pszName == NULL)
		return;

	HExport temp;
	temp.mAliasName = temp.mName = pszName;
	temp.mOrdinalValue = ordinal;

	HExport* tNewExport = tOldDefs.Search(pszName);
	if (tNewExport == NULL)
		tNewExport = tOldDefsAliases.Search(pszName);

	if (tNewExport == NULL) {
		gNewExports++;

		if (gRebuild) {
			tNewExport = new(gMM)HExport;
			tNewExport->mAliasName = NULL;
			tNewExport->mName = gMM.placeStr(pszName);
			tNewExport->mExportedByOrdinal = true;
			tNewExport->mOrdinalValue = ordinal;
			tOldDefs.insert(tNewExport);
		}
		else fprintf(file, "%s @%d NONAME\n", pszName, ++gMaxOrdinal);
	}
	else if (gRebuild)
		tNewExport->mOrdinalValue = ordinal;
}

//====[ Lists all exports in a section ]=================================================

static char sttMsgHdr[] = "Fatal error reading PE-image headers";

bool GetExport(FILE* file, IMAGE_NT_HEADERS* pINTH, BYTE* pMap, bool listExports)
{
	IMAGE_SECTION_HEADER* pISH = IMAGE_FIRST_SECTION(pINTH);
	if (!pISH)
		return false;

	IMAGE_DATA_DIRECTORY*	tDir;
	IMAGE_NT_HEADERS64*		pINTH64 = NULL;

	if (pINTH->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64)
		pINTH64 = (IMAGE_NT_HEADERS64*)pINTH;
	else if (pINTH->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
		pINTH64 = (IMAGE_NT_HEADERS64*)pINTH;
	else
		if (pINTH->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
			fprintf(stderr, "%s: machine code '%08x' is not supported\n", sttMsgHdr, pINTH->FileHeader.Machine);
			return false;
		}

	int tDirectoryID = (listExports) ? IMAGE_DIRECTORY_ENTRY_EXPORT : IMAGE_DIRECTORY_ENTRY_IMPORT;

	if (pINTH64 != NULL)
		tDir = &pINTH64->OptionalHeader.DataDirectory[tDirectoryID];
	else
		tDir = &pINTH->OptionalHeader.DataDirectory[tDirectoryID];

	if (tDir->Size == 0 || tDir->VirtualAddress == 0)
		return false;

	DWORD i;
	for (i = 0; i < pINTH->FileHeader.NumberOfSections; i++, pISH++) {
		if (tDir->VirtualAddress >= pISH->VirtualAddress &&
			tDir->VirtualAddress + tDir->Size <= pISH->VirtualAddress + pISH->SizeOfRawData)
			break;
	}

	if (i == pINTH->FileHeader.NumberOfSections) {
		fprintf(stderr, "%s: required PE-image section not found\n", sttMsgHdr);
		return false;
	}

	BYTE* tSection = pMap + tDir->VirtualAddress - (pISH->VirtualAddress - pISH->PointerToRawData);

	if (listExports) {
		IMAGE_EXPORT_DIRECTORY* pIED = (IMAGE_EXPORT_DIRECTORY*)tSection;
		if (pIED->NumberOfNames == 0)
			return true;

		WORD* pOrdinals = (WORD*)::RVAtoAddress(pIED->AddressOfNameOrdinals, pINTH, pMap);

		DWORD* pAddrOfNames = (DWORD*)::RVAtoAddress(pIED->AddressOfNames, pINTH, pMap);
		if (pAddrOfNames) {
			for (DWORD i = 0; i < pIED->NumberOfNames; i++) {
				int tOrdinal = -1;
				if (pOrdinals != NULL)
					tOrdinal = pOrdinals[i] + pIED->Base;

				IterateFunction(file, (char*)::RVAtoAddress(pAddrOfNames[i], pINTH, pMap), tOrdinal);
			}
		}
	}
	else {
		for (IMAGE_IMPORT_DESCRIPTOR* pIID = (IMAGE_IMPORT_DESCRIPTOR*)tSection;
			pIID->Characteristics != 0;
			pIID++) {
			if (gImportDllName != NULL) {
				char* tDllName = (char*)tSection + pIID->Name - tDir->VirtualAddress;
				if (tDllName == NULL)
					return false;

				if (_stricmp(tDllName, gImportDllName) != 0)
					continue;
			}

			DWORD* pAddrOfFuncs = (DWORD*)(tSection + pIID->Characteristics - tDir->VirtualAddress);
			while (*pAddrOfFuncs != 0) {
				if (!(pAddrOfFuncs[0] & 0x80000000)) {
					IMAGE_IMPORT_BY_NAME* tImport = (IMAGE_IMPORT_BY_NAME*)(tSection + *pAddrOfFuncs - tDir->VirtualAddress);
					IterateFunction(file, (char*)&tImport->Name);
				}

				pAddrOfFuncs++;
			}
		}
	}

	fprintf(stderr, "\rFinished processing DLL, %d new exports added", gNewExports);
	if (gNewExports > 0)
		fprintf(stderr, "\nYou should relink your DLL\n\n");

	return true;
}

//====[ Main ]===========================================================================

int main(int argc, char** argv)
{
	fprintf(stderr, "DEF file intelligent creator v.1.4, copyright (c) George Hazan (ghazan@postman.ru), 1999-2004.\nSpecial thanks to Andrey No for PE image scanner sources\n\n");

	//----[ Command line parsing ]--------------------------------------------------------

	char* tDefFileName = NULL;
	char* tDllFileName = NULL;
	bool  tUseExports = true;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '/') {
LBL_Usage:
			fprintf(stderr, "Usage: MakeDefs /DEF:<def_file> /IMAGE:<exe_or_dll_file> [/REBUILDALL] [/NOSORT] [ /IMPORTS [:<dll_name> ]]\n");
			return 200;
		}

		if (_memicmp(&argv[i][1], "DEF:", 4) == 0)
			tDefFileName = &argv[i][5];
		else if (_memicmp(&argv[i][1], "IMAGE:", 6) == 0)
			tDllFileName = &argv[i][7];
		else if (_memicmp(&argv[i][1], "IMPORTS", 7) == 0) {
			tUseExports = false;
			gImportDllName = (argv[i][8] == ':') ? argv[i] + 9 : NULL;
		}
		else if (_memicmp(&argv[i][1], "REBUILDALL", 10) == 0)
			gRebuild = 1;
		else if (_memicmp(&argv[i][1], "NOSORT", 6) == 0)
			gNoSort = 1;
		else goto LBL_Usage;
	}

	if (tDefFileName == NULL || tDllFileName == NULL)
		goto LBL_Usage;

	//----[ Command line ok, opening data files ]-----------------------------------------

	int tResult = 0, tLineNo = 0;

	char	tBuffer[1024];
	strncpy_s(tBuffer, "mkdXXXXXX", _TRUNCATE);

	char *tTemplate = _mktemp(tBuffer);
	FILE* tTempFile = fopen(tTemplate, "wt");
	if (tTempFile == NULL) {
		fprintf(stderr, "Unable to create temporary file %s\n", tTemplate);
		return 100;
	}

	FILE* tInFile = fopen(tDefFileName, "rt");
	if (tInFile == NULL) {
		fprintf(stderr, "Creating new DEF file: '%s'...\n", tDefFileName);

		char tDrive[_MAX_DRIVE], tPath[_MAX_DIR], tFName[_MAX_FNAME], tExt[_MAX_EXT];
		_splitpath(tDllFileName, tDrive, tPath, tFName, tExt);

		fprintf(tTempFile, "LIBRARY %s.DLL\nDESCRIPTION\n\nEXPORTS\n", tFName);
		gMaxOrdinal = 0;
	}
	else {
		fprintf(stderr, "Processing DEF file: '%s'...\n", tDefFileName);

		while (fgets(tBuffer, sizeof(tBuffer), tInFile) != NULL) {
			fputs(tBuffer, tTempFile);
			tLineNo++;

			if (_stricmp(trim(tBuffer), "EXPORTS") == 0)
				goto LBL_DefFileOk;
		}

LBL_DefFileBad:
		fprintf(stderr, "'%s is not a valid DEF file\n", tDefFileName);
		tResult = 101;

LBL_ErrorExit:
		fclose(tTempFile);
		_unlink(tTemplate);
		return tResult;

LBL_DefFileOk:
		if (gRebuild)
			gMaxOrdinal = 0;

		while (fgets(tBuffer, sizeof(tBuffer), tInFile) != NULL) {
			if (!gRebuild)
				fputs(tBuffer, tTempFile);

			tLineNo++;

			char* p = trim(tBuffer);
			if (*p == EOS || *p == ';')
				continue;

			char* tEndIdent = p + strcspn(p, " \t");
			if (tEndIdent == p) {
				fprintf(stderr, "Line %d: invalid export identifier\n", tLineNo);
				goto LBL_DefFileBad;
			}

			*tEndIdent++ = 0;
			tEndIdent += strspn(tEndIdent, " \t");

			HExport* tNewExport = new(gMM)HExport;
			tNewExport->mName = gMM.placeStr(p);

			if (*tEndIdent == '=') {
				p = ltrim(tEndIdent + 1);
				tEndIdent = p + strcspn(p, " \t");
				if (tEndIdent == p) {
					fprintf(stderr, "Line %d: invalid export identifier\n", tLineNo);
					goto LBL_DefFileBad;
				}

				*tEndIdent++ = 0;
				tEndIdent += strspn(tEndIdent, " \t");

				tNewExport->mAliasName = gMM.placeStr(p);
			}
			else tNewExport->mAliasName = "";

			tOldDefs.insert(tNewExport);
			tOldDefsAliases.insert(tNewExport);

			if (*tEndIdent != '@') {
				fprintf(stderr, "Line %d: symbol '@' not found\n", tLineNo);
				goto LBL_DefFileBad;
			}

			p = ltrim(tEndIdent + 1);
			if (*p == '?')
				tNewExport->mOrdinalValue = -1;
			else {
				int tOrdinal;
				if (sscanf(p, "%d", &tOrdinal) != 1) {
					fprintf(stderr, "Line %d: invalid ordinal value\n", tLineNo);
					goto LBL_DefFileBad;
				}

				if (!gRebuild && tOrdinal > gMaxOrdinal)
					gMaxOrdinal = tOrdinal;

				tNewExport->mOrdinalValue = tOrdinal;
			}

			tEndIdent = p + strcspn(p, " \t");
			tNewExport->mExportedByOrdinal = (_stricmp(trim(tEndIdent), "NONAME") == 0);
		}

		fclose(tInFile);
		fprintf(stderr, "\rFinished processing '%s', maximum ordinal value is %d\n", tDefFileName, gMaxOrdinal);
	}

	fprintf(stderr, "Scanning DLL...\n");

	HANDLE hFile;
	HANDLE hMap;
	BYTE*  pMap;
	if (!LoadFile(tDllFileName, hFile, hMap, pMap)) {
		fprintf(stderr, "'%s' does not exist or seems not to be a valid PE image\n", tDllFileName);
		tResult = 102;
		goto LBL_ErrorExit;
	}

	IMAGE_DOS_HEADER* pIDH = GetImageDosHeader(pMap);
	if (!pIDH) {
		fprintf(stderr, "'%s' does not contain a valid DOS header\n", tDllFileName);
		tResult = 103;
		goto LBL_ErrorExit;
	}

	IMAGE_NT_HEADERS* pINTH = GetImageNtHeaders(pIDH);
	if (!pINTH) {
		fprintf(stderr, "'%s' does not contain a valid PE image header\n", tDllFileName);
		tResult = 104;
		goto LBL_ErrorExit;
	}

	if (!GetExport(tTempFile, pINTH, pMap, tUseExports)) {
		fprintf(stderr, "Fatal error reading export table from '%s'\n", tDllFileName);
		tResult = 105;
		goto LBL_ErrorExit;
	}

	UnloadFile(hFile, hMap, pMap);

	if (gRebuild) {
		for (int i = 0; i < tOldDefs.getCount(); i++) {
			HExport* E = tOldDefs[i];
			if (E->mAliasName[0] == EOS)
				fputs(E->mName, tTempFile);
			else
				fprintf(tTempFile, "%s = %s", E->mName, E->mAliasName);

			if (E->mOrdinalValue == -1)
				fputs(" @?", tTempFile);
			else
				fprintf(tTempFile, " @%d", (gNoSort) ? E->mOrdinalValue : i + 1);

			if (E->mExportedByOrdinal)
				fputs(" NONAME", tTempFile);

			fputc('\n', tTempFile);
		}
	}

	fclose(tTempFile);

	if (gNewExports != 0 || gRebuild) {
		_unlink(tDefFileName);
		rename(tTemplate, tDefFileName);
		return 1;
	}

	_unlink(tTemplate);
	return 0;
}
