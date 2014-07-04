/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-14 Miranda NG project,
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

#include "commonheaders.h"

//the cache has not been loaded when these functions are used

int CDb3Mmap::CreateDbHeaders(const DBSignature& _sign)
{
	DWORD bytesWritten;

	CopyMemory(m_dbHeader.signature, &_sign, sizeof(m_dbHeader.signature));

	m_dbHeader.version = DB_095_1_VERSION;
	m_dbHeader.ofsFileEnd = sizeof(struct DBHeader);
	m_dbHeader.slackSpace = 0;
	m_dbHeader.contactCount = 0;
	m_dbHeader.ofsFirstContact = 0;
	m_dbHeader.ofsModuleNames = 0;
	m_dbHeader.ofsUser = 0;
	//create user
	m_dbHeader.ofsUser = m_dbHeader.ofsFileEnd;
	m_dbHeader.ofsFileEnd += sizeof(DBContact);
	SetFilePointer(m_hDbFile, 0, NULL, FILE_BEGIN);
	WriteFile(m_hDbFile, &m_dbHeader, sizeof(m_dbHeader), &bytesWritten,NULL);

	DBContact user = { 0 };
	user.signature = DBCONTACT_SIGNATURE;
	SetFilePointer(m_hDbFile, m_dbHeader.ofsUser, NULL, FILE_BEGIN);
	WriteFile(m_hDbFile, &user, sizeof(DBContact), &bytesWritten, NULL);
	FlushFileBuffers(m_hDbFile);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static TCHAR tszOldHeaders[] = 
	LPGENT("This profile is too old to be updated with PluginUpdater, your database must be converted first.\n\nWould you like to read how to fix this?");

int CDb3Mmap::CheckDbHeaders()
{
	if (memcmp(m_dbHeader.signature, &dbSignatureU, sizeof(m_dbHeader.signature)) &&
		 memcmp(m_dbHeader.signature, &dbSignatureE, sizeof(m_dbHeader.signature)))
	{
		if (!memcmp(&m_dbHeader.signature, &dbSignatureIM, sizeof(m_dbHeader.signature)) ||
			 !memcmp(&m_dbHeader.signature, &dbSignatureSA, sizeof(m_dbHeader.signature)) ||
			 !memcmp(&m_dbHeader.signature, &dbSignatureSD, sizeof(m_dbHeader.signature)))
		{
			if (IDYES == MessageBox(NULL, TranslateTS(tszOldHeaders), TranslateT("Obsolete database format"), MB_YESNO | MB_ICONWARNING)) {
				TCHAR tszCurPath[MAX_PATH];
				GetModuleFileName(NULL, tszCurPath, SIZEOF(tszCurPath));
				TCHAR *p = _tcsrchr(tszCurPath, '\\');
				if (p) *p = 0;

				HKEY hPathSetting;
				if (!RegCreateKey(HKEY_CURRENT_USER, _T("Software\\Miranda NG"), &hPathSetting)) {
					RegSetValue(hPathSetting, _T("InstallPath"), REG_SZ, tszCurPath, sizeof(tszCurPath));
					RegCloseKey(hPathSetting);
				}

				CallService(MS_UTILS_OPENURL, 0, LPARAM("http://wiki.miranda-ng.org/index.php?title=Updating_pre-0.94.9_version_to_0.95.1_and_later"));
				Sleep(1000);
				exit(0);
			}
		}
		return EGROKPRF_UNKHEADER;
	}

	switch (m_dbHeader.version) {
	case DB_094_VERSION:
	case DB_095_1_VERSION:
	case DB_095_VERSION:
		break;

	default:
		return EGROKPRF_VERNEWER;
	}
	
	if (m_dbHeader.ofsUser == 0)
		return EGROKPRF_DAMAGED;

	return 0;
}

void CDb3Mmap::WriteSignature(DBSignature &sign)
{
	memcpy(&m_dbHeader.signature, &sign, sizeof(DBSignature));
	DBWrite(0, &sign, sizeof(DBSignature));
}
