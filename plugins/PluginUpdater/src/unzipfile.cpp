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

	void fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
}

bool extractCurrentFile(unzFile uf, TCHAR* ptszDestPath, TCHAR* ptszBackPath)
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
	TCHAR* p = mir_utf8decodeT(filename);
	if (p == NULL)
		p = mir_a2t(filename);
	mir_sntprintf(tszDestFile, SIZEOF(tszDestFile), _T("%s\\%s"), ptszDestPath, p);
	mir_sntprintf(tszBackFile, SIZEOF(tszBackFile), _T("%s\\%s"), ptszBackPath, p);
	mir_free(p);

	for (p = tszDestFile; *p; ++p) if (*p == '/') *p = '\\'; 
	for (p = tszBackFile; *p; ++p) if (*p == '/') *p = '\\'; 

	CreatePathToFileT(tszDestFile);
	CreatePathToFileT(tszBackFile);

	if ( !(file_info.external_fa & FILE_ATTRIBUTE_DIRECTORY)) {
		err = unzOpenCurrentFile(uf);
		if (err != UNZ_OK)
			return false;

		DeleteFile(tszBackFile);
		MoveFile(tszDestFile, tszBackFile);

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
	return true;
}

void unzip(const TCHAR* ptszZipFile, TCHAR* ptszDestPath, TCHAR* ptszBackPath)
{
	zlib_filefunc64_def ffunc;
	fill_fopen64_filefunc(&ffunc);

	unzFile uf = unzOpen2_64(ptszZipFile, &ffunc);
	if (uf) {
		do {
			extractCurrentFile(uf, ptszDestPath, ptszBackPath);
		}
			while (unzGoToNextFile(uf) == UNZ_OK);
		unzClose(uf);
	}
}
