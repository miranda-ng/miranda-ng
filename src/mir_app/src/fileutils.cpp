/*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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
#include "file.h"

wchar_t* PFTS_StringToTchar(int flags, const MAllStrings s)
{
	if (flags & PFTS_UTF)
		return mir_utf8decodeW(s.a);
	if (flags & PFTS_UNICODE)
		return mir_wstrdup(s.w);
	return mir_a2u(s.a);
}

int PFTS_CompareWithTchar(PROTOFILETRANSFERSTATUS *ft, const MAllStrings s, wchar_t *r)
{
	if (ft->flags & PFTS_UTF)
		return mir_wstrcmp(Utf2T(s.a), r);

	if (ft->flags & PFTS_UNICODE)
		return mir_wstrcmp(s.w, r);

	return mir_wstrcmp(_A2T(s.a), r);
}

void CopyProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	*dest = *src;
	
	if (src->szCurrentFile.w)
		dest->szCurrentFile.w = PFTS_StringToTchar(src->flags, src->szCurrentFile);
	
	if (src->pszFiles.w) {
		dest->pszFiles.w = (wchar_t **)mir_alloc(sizeof(wchar_t *) * src->totalFiles);
		for (int i = 0; i < src->totalFiles; i++) {
			MAllStrings s = { src->pszFiles.a[i] };
			dest->pszFiles.w[i] = PFTS_StringToTchar(src->flags, s);
		}
	}
	
	if (src->szWorkingDir.w)
		dest->szWorkingDir.w = PFTS_StringToTchar(src->flags, src->szWorkingDir);
	
	dest->flags &= ~PFTS_UTF;
	dest->flags |= PFTS_UNICODE;
}

void UpdateProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	dest->hContact = src->hContact;
	dest->flags = src->flags;
	if (dest->totalFiles != src->totalFiles) {
		for (int i = 0; i < dest->totalFiles; i++) mir_free(dest->pszFiles.w[i]);
		mir_free(dest->pszFiles.w);
		dest->pszFiles.w = nullptr;
		dest->totalFiles = src->totalFiles;
	}
	if (src->pszFiles.w) {
		if (!dest->pszFiles.w)
			dest->pszFiles.w = (wchar_t **)mir_calloc(sizeof(wchar_t *) * src->totalFiles);
		for (int i = 0; i < src->totalFiles; i++) {
			MAllStrings fname; fname.w = src->pszFiles.w[i];
			if (!dest->pszFiles.w[i] || !fname.w || PFTS_CompareWithTchar(src, fname, dest->pszFiles.w[i])) {
				mir_free(dest->pszFiles.w[i]);
				if (fname.w)
					dest->pszFiles.w[i] = PFTS_StringToTchar(src->flags, fname);
				else
					dest->pszFiles.w[i] = nullptr;
			}
		}
	}
	else if (dest->pszFiles.w) {
		for (int i = 0; i < dest->totalFiles; i++)
			mir_free(dest->pszFiles.w[i]);
		mir_free(dest->pszFiles.w);
		dest->pszFiles.w = nullptr;
	}

	dest->currentFileNumber = src->currentFileNumber;
	dest->totalBytes = src->totalBytes;
	dest->totalProgress = src->totalProgress;
	if (src->szWorkingDir.w && (!dest->szWorkingDir.w || PFTS_CompareWithTchar(src, src->szWorkingDir, dest->szWorkingDir.w))) {
		mir_free(dest->szWorkingDir.w);
		if (src->szWorkingDir.w)
			dest->szWorkingDir.w = PFTS_StringToTchar(src->flags, src->szWorkingDir);
		else
			dest->szWorkingDir.w = nullptr;
	}

	if (!dest->szCurrentFile.w || !src->szCurrentFile.w || PFTS_CompareWithTchar(src, src->szCurrentFile, dest->szCurrentFile.w)) {
		mir_free(dest->szCurrentFile.w);
		if (src->szCurrentFile.w)
			dest->szCurrentFile.w = PFTS_StringToTchar(src->flags, src->szCurrentFile);
		else
			dest->szCurrentFile.w = nullptr;
	}
	dest->currentFileSize = src->currentFileSize;
	dest->currentFileProgress = src->currentFileProgress;
	dest->currentFileTime = src->currentFileTime;
	dest->flags &= ~PFTS_UTF;
	dest->flags |= PFTS_UNICODE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Triple redirection sucks but is needed to nullify the array pointer

void FreeFilesMatrix(wchar_t ***files)
{
	if (*files == nullptr)
		return;

	// Free each filename in the pointer array
	wchar_t **pFile = *files;
	while (*pFile != nullptr) {
		mir_free(*pFile);
		*pFile = nullptr;
		pFile++;
	}

	// Free the array itself
	mir_free(*files);
	*files = nullptr;
}

void FreeProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *fts)
{
	mir_free(fts->szCurrentFile.w);
	if (fts->pszFiles.w) {
		for (int i = 0; i < fts->totalFiles; i++) mir_free(fts->pszFiles.w[i]);
		mir_free(fts->pszFiles.w);
	}
	mir_free(fts->szWorkingDir.w);
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringW CreateUniqueFileName(const wchar_t *pszOriginalFile)
{
	const wchar_t *pszExtension, *pszFilename;
	if ((pszFilename = wcsrchr(pszOriginalFile, '\\')) == nullptr)
		pszFilename = pszOriginalFile;
	if ((pszExtension = wcsrchr(pszFilename + 1, '.')) == nullptr)
		pszExtension = pszFilename + mir_wstrlen(pszFilename);

	CMStringW buf;
	for (int i = 1;; i++) {
		buf.Format(L"%.*s (%d)%s", unsigned(pszExtension - pszOriginalFile), pszOriginalFile, i, pszExtension);
		if (_waccess(buf, 0) != 0)
			break;
	}
	return buf;
}

CMStringW FindUniqueFileName(const wchar_t *pszOriginalFile)
{
	if (_waccess(pszOriginalFile, 0))
		return pszOriginalFile;

	return CreateUniqueFileName(pszOriginalFile);
}

/////////////////////////////////////////////////////////////////////////////////////////

int GetRegValue(HKEY hKeyBase, const wchar_t *szSubKey, const wchar_t *szValue, wchar_t *szOutput, int cbOutput)
{
	HKEY hKey;
	if (RegOpenKeyEx(hKeyBase, szSubKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
		return 0;

	DWORD cbOut = cbOutput;
	auto ret = RegQueryValueEx(hKey, szValue, nullptr, nullptr, (uint8_t *)szOutput, &cbOut);
	RegCloseKey(hKey);
	return ret == ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define UNITS_BYTES     1   // 0 <= size<1000: "%d bytes"
#define UNITS_KBPOINT1	2	// 1000 <= size<100*1024: "%.1f KB"
#define UNITS_KBPOINT0  3   // 100*1024 <= size<1024*1024: "%d KB"
#define UNITS_MBPOINT2  4   // 1024*1024 <= size: "%.2f MB"
#define UNITS_GBPOINT3  5   // 1024*1024*1024 <= size: "%.3f GB"

void GetSensiblyFormattedSize(__int64 size, wchar_t *szOut, int cchOut, int unitsOverride, int appendUnits, int *unitsUsed)
{
	if (!unitsOverride) {
		if (size < 1000) unitsOverride = UNITS_BYTES;
		else if (size < 100 * 1024) unitsOverride = UNITS_KBPOINT1;
		else if (size < 1024 * 1024) unitsOverride = UNITS_KBPOINT0;
		else if (size < 1024 * 1024 * 1024) unitsOverride = UNITS_MBPOINT2;
		else unitsOverride = UNITS_GBPOINT3;
	}

	if (unitsUsed)
		*unitsUsed = unitsOverride;

	switch (unitsOverride) {
	case UNITS_BYTES: mir_snwprintf(szOut, cchOut, L"%u%s%s", (int)size, appendUnits ? L" " : L"", appendUnits ? TranslateT("bytes") : L""); break;
	case UNITS_KBPOINT1: mir_snwprintf(szOut, cchOut, L"%.1lf%s", size / 1024.0, appendUnits ? L" KB" : L""); break;
	case UNITS_KBPOINT0: mir_snwprintf(szOut, cchOut, L"%u%s", (int)(size / 1024), appendUnits ? L" KB" : L""); break;
	case UNITS_GBPOINT3: mir_snwprintf(szOut, cchOut, L"%.3f%s", (size >> 20) / 1024.0, appendUnits ? L" GB" : L""); break;
	default: mir_snwprintf(szOut, cchOut, L"%.2lf%s", size / 1048576.0, appendUnits ? L" MB" : L""); break;
	}
}
