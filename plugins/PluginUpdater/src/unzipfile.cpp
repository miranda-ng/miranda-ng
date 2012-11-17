/*
Copyright (C) 2012 George Hazan

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt. If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "common.h"

extern "C"
{
	#include "Minizip\unzip.h"

	void fill_fopen64_filefunc(zlib_filefunc64_def *pzlib_filefunc_def);
}

static void PrepareFileName(TCHAR *dest, size_t destSize, const TCHAR *ptszPath, const TCHAR *ptszFileName)
{
	mir_sntprintf(dest, destSize, _T("%s\\%s"), ptszPath, ptszFileName);

	for (TCHAR *p = dest; *p; ++p)
		if (*p == '/')
			*p = '\\'; 
}

void BackupFile(TCHAR *ptszSrcFileName, TCHAR *ptszBackFileName)
{
	CreatePathToFileT(ptszBackFileName);
	DeleteFile(ptszBackFileName);
	if ( MoveFile(ptszSrcFileName, ptszBackFileName) == 0) { // use copy on error
		CopyFile(ptszSrcFileName, ptszBackFileName, FALSE);
		DeleteFile(ptszSrcFileName);
	}
}

bool extractCurrentFile(unzFile uf, TCHAR *ptszDestPath, TCHAR *ptszBackPath)
{
	int err = UNZ_OK;
	unz_file_info64 file_info;
	char filename[MAX_PATH];
	char buf[8192];

	err = unzGetCurrentFileInfo64(uf, &file_info, filename, sizeof(filename), buf, sizeof(buf), NULL, 0);
	if (err != UNZ_OK)
		return false;

	if (!opts.bUpdateIcons && !_strnicmp(filename, "Icons/", 6))
		return true;

	TCHAR tszDestFile[MAX_PATH], tszBackFile[MAX_PATH];
	TCHAR *ptszNewName = mir_utf8decodeT(filename);
	if (ptszNewName == NULL)
		ptszNewName = mir_a2t(filename);

	if ( !(file_info.external_fa & FILE_ATTRIBUTE_DIRECTORY)) {
		err = unzOpenCurrentFile(uf);
		if (err != UNZ_OK)
			return false;

		PrepareFileName(tszDestFile, SIZEOF(tszDestFile), ptszDestPath, ptszNewName);
		PrepareFileName(tszBackFile, SIZEOF(tszBackFile), ptszBackPath, ptszNewName);
		BackupFile(tszDestFile, tszBackFile);

		PrepareFileName(tszDestFile, SIZEOF(tszDestFile), ptszDestPath, ptszNewName);
		CreatePathToFileT(tszDestFile);
		
		HANDLE hFile = CreateFile(tszDestFile, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
			CREATE_ALWAYS, file_info.external_fa, 0);

		if (hFile != INVALID_HANDLE_VALUE) {
			while (true) {
				err = unzReadCurrentFile(uf, buf, sizeof(buf));
				if (err <= 0)
					break;

				DWORD bytes;
				if (!WriteFile(hFile, buf, err, &bytes, FALSE)) {
					err = UNZ_ERRNO;
					break;
				}
			}

			FILETIME ftLocal, ftCreate, ftLastAcc, ftLastWrite;
			GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);
			DosDateTimeToFileTime(HIWORD(file_info.dosDate), LOWORD(file_info.dosDate), &ftLocal);
			LocalFileTimeToFileTime(&ftLocal, &ftLastWrite);
			SetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);

			CloseHandle(hFile);
			unzCloseCurrentFile(uf); /* don't lose the error */
		}
	}
	mir_free(ptszNewName);
	return true;
}

bool unzip(const TCHAR *ptszZipFile, TCHAR *ptszDestPath, TCHAR *ptszBackPath)
{
	bool bResult = true;

	zlib_filefunc64_def ffunc;
	fill_fopen64_filefunc(&ffunc);
	
	unzFile uf = unzOpen2_64(ptszZipFile, &ffunc);
	if (uf) {
		do {
			if ( !extractCurrentFile(uf, ptszDestPath, ptszBackPath))
				bResult = false;
		}
			while (unzGoToNextFile(uf) == UNZ_OK);
		unzClose(uf);
	}

	return bResult;
}
