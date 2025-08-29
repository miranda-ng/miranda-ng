/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Single file HTTP transaction

int DownloadFile(FILEURL *pFileURL, HNETLIBCONN &nlc)
{
	char szMirVer[100];
	Miranda_GetVersionText(szMirVer, _countof(szMirVer));
	if (auto *p = strchr(szMirVer, '('))
		*p = 0;
	rtrim(szMirVer);

	char szOsVer[100];
	OS_GetShortString(szOsVer, _countof(szOsVer));

	CMStringA szUserAgent("Miranda NG/");
	szUserAgent.Append(szMirVer);
	szUserAgent.AppendFormat(" (%s", szOsVer);
	#ifdef _WIN64
		szUserAgent.Append("; Win64; x64");
	#endif
	szUserAgent.Append(")");

	ptrA szUrl(mir_u2a(pFileURL->wszDownloadURL));

	MHttpRequest nlhr(REQUEST_GET);
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_PERSISTENT;
	nlhr.nlc = nlc;
	nlhr.m_szUrl = szUrl;
	nlhr.AddHeader("User-Agent", szUserAgent);
	nlhr.AddHeader("Connection", "close");
	nlhr.AddHeader("Cache-Control", "no-cache");
	nlhr.AddHeader("Pragma", "no-cache");

	for (int i = 0; i < MAX_RETRIES; i++) {
		Netlib_LogfW(g_hNetlibUser, L"Downloading file %s to %s (attempt %d)", pFileURL->wszDownloadURL, pFileURL->wszDiskPath, i + 1);
		NLHR_PTR pReply(Netlib_HttpTransaction(g_hNetlibUser, &nlhr));
		if (pReply == nullptr) {
			Netlib_LogfW(g_hNetlibUser, L"Downloading file %s failed, host is propably temporary down.", pFileURL->wszDownloadURL);
			nlc = nullptr;
			continue;
		}

		nlc = pReply->nlc;
		if (pReply->resultCode != 200 || pReply->body.IsEmpty()) {
			Netlib_LogfW(g_hNetlibUser, L"Downloading file %s failed with error %d", pFileURL->wszDownloadURL, pReply->resultCode);
			return pReply->resultCode;
		}

		// Check CRC sum
		if (pFileURL->CRCsum) {
			int crc = crc32(0, (unsigned char *)pReply->body.c_str(), pReply->body.GetLength());
			if (crc != pFileURL->CRCsum) {
				// crc check failed, try again
				Netlib_LogfW(g_hNetlibUser, L"crc check failed for file %s", pFileURL->wszDiskPath);
				continue;
			}
		}

		// everything is ok, write file down
		DWORD dwBytes;
		HANDLE hFile = CreateFile(pFileURL->wszDiskPath, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE) {
			// write the downloaded file directly
			WriteFile(hFile, pReply->body, pReply->body.GetLength(), &dwBytes, nullptr);
			CloseHandle(hFile);
		}
		else {
			// try to write it via PU stub
			TFileName wszTempFile;
			mir_snwprintf(wszTempFile, L"%s\\pulocal.tmp", g_wszTempPath);
			hFile = CreateFile(wszTempFile, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile != INVALID_HANDLE_VALUE) {
				WriteFile(hFile, pReply->body, pReply->body.GetLength(), &dwBytes, nullptr);
				CloseHandle(hFile);
				PU::SafeMoveFile(wszTempFile, pFileURL->wszDiskPath);
			}
		}
		return ERROR_SUCCESS;
	}

	// no more retries, return previous error code
	Netlib_LogfW(g_hNetlibUser, L"Downloading file %s failed, giving up", pFileURL->wszDownloadURL);
	return 500;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Folder creation

static int __cdecl CompareDirs(const CMStringW *s1, const CMStringW *s2)
{
	return mir_wstrcmp(s1->c_str(), s2->c_str());
}

void CreateWorkFolders(TFileName &wszTempFolder, TFileName &wszBackupFolder)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	mir_snwprintf(wszBackupFolder, L"%s\\Backups\\BKP%04d-%02d-%02d %02d-%02d-%02d", g_wszRoot, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	PU::SafeCreateDirectory(wszBackupFolder);

	mir_snwprintf(wszTempFolder, L"%s\\Temp", g_wszRoot);
	PU::SafeCreateDirectory(wszTempFolder);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Folder removal

void RemoveBackupFolders()
{
	TFileName wszMask;
	mir_snwprintf(wszMask, L"%s\\Backups\\BKP*", g_wszRoot);

	WIN32_FIND_DATAW fdata;
	HANDLE hFind = FindFirstFileW(wszMask, &fdata);
	if (!hFind)
		return;

	// sort folder names alphabetically
	OBJLIST<CMStringW> arNames(1, CompareDirs);
	do {
		if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			arNames.insert(new CMStringW(fdata.cFileName));
	} while (FindNextFileW(hFind, &fdata));

	FindClose(hFind);

	// remove all folders with lesser dates if there're more than 10 folders
	if (PU::PrepareEscalation()) {
		while (arNames.getCount() > g_plugin.iNumberBackups) {
			mir_snwprintf(wszMask, L"%s\\Backups\\%s", g_wszRoot, arNames[0].c_str());
			PU::SafeDeleteDirectory(wszMask);
			arNames.remove(00);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// moves updated files back to Miranda's folder

static void SafeMoveFolder(const wchar_t *wszSrc, const wchar_t *wszDest)
{
	TFileName wszNewSrc, wszNewDest;
	mir_snwprintf(wszNewSrc, L"%s\\*", wszSrc);

	WIN32_FIND_DATAW fdata;
	HANDLE hFind = FindFirstFileW(wszNewSrc, &fdata);
	if (!hFind)
		return;

	do {
		if (!mir_wstrcmp(fdata.cFileName, L".") || !mir_wstrcmp(fdata.cFileName, L".."))
			continue;

		mir_snwprintf(wszNewSrc, L"%s\\%s", wszSrc, fdata.cFileName);
		mir_snwprintf(wszNewDest, L"%s\\%s", wszDest, fdata.cFileName);

		if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			SafeMoveFolder(wszNewSrc, wszNewDest);
		else
			PU::SafeMoveFile(wszNewSrc, wszNewDest);
	}
		while (FindNextFileW(hFind, &fdata));

	FindClose(hFind);
}

void RollbackChanges(TFileName &pwszBackupFolder)
{
	SafeMoveFolder(pwszBackupFolder, g_mirandaPath);

	PU::SafeDeleteDirectory(pwszBackupFolder);
}

int BackupFile(wchar_t *pwszSrcFileName, wchar_t *pwszBackFileName)
{
	if (_waccess(pwszSrcFileName, 0))
		return 0;

	PU::SafeCreateFilePath(pwszBackFileName);

	if (int err = PU::SafeMoveFile(pwszSrcFileName, pwszBackFileName)) {
		Netlib_LogfW(g_hNetlibUser, L"Error moving file %s to %s: %d", pwszSrcFileName, pwszBackFileName, err);
		return err;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MFilePath InvertMirandaPlatform()
{
	MFilePath wszPath;
	#ifdef _WIN64
		wszPath.Format(L"%s\\miranda32.exe", g_mirandaPath.get());
	#else
		wszPath.Format(L"%s\\miranda64.exe", g_mirandaPath.get());
	#endif
	return wszPath;
}

void DoRestart()
{
	BOOL bRestartCurrentProfile = g_plugin.getBool("RestartCurrentProfile", true);
	if (g_plugin.bChangePlatform) {
		CallServiceSync(MS_SYSTEM_RESTART, bRestartCurrentProfile, (LPARAM)InvertMirandaPlatform().c_str());
		g_plugin.bChangePlatform.Delete();
	}
	else CallServiceSync(MS_SYSTEM_RESTART, bRestartCurrentProfile);
}
