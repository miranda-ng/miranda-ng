/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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

CFolderItem::CFolderItem(const char *sectionName, const char *name, const TCHAR *format, const TCHAR *userName)
{
	m_szSection = mir_strdup(sectionName);
	m_szName = mir_strdup(name);
	if (userName)
		m_tszUserName = mir_tstrdup(userName);
	else
		m_tszUserName = mir_a2t(name);
	m_tszFormat = NULL;
	m_tszOldFormat = NULL;
	GetDataFromDatabase(format);
	FolderCreateDirectory();
}

CFolderItem::~CFolderItem()
{
	mir_free(m_szSection);
	mir_free(m_szName);
	mir_free(m_tszFormat);
	mir_free(m_tszOldFormat);
	mir_free(m_tszUserName);
}

void CFolderItem::SetFormat(const TCHAR *newFormat)
{
	mir_free(m_tszOldFormat);
	m_tszOldFormat = m_tszFormat;
	m_tszFormat = mir_tstrdup(*newFormat ? newFormat : MIRANDA_PATHT);
}

int CFolderItem::IsEqual(const CFolderItem *other)
{
	return (IsEqual(other->GetSection(), other->GetUserName()));
}

int CFolderItem::IsEqual(const char *section, const TCHAR *name)
{
	return !mir_tstrcmp(m_tszUserName, name) && !mir_strcmp(m_szSection, section);
}

int CFolderItem::IsEqualTranslated(const char *trSection, const TCHAR *trName)
{
	return !mir_tstrcmp(TranslateTS(m_tszUserName), trName) && !mir_strcmp(Translate(m_szSection), trSection);
}

int CFolderItem::operator ==(const CFolderItem *other)
{
	return IsEqual(other);
}

void CFolderItem::Expand(TCHAR *buffer, int size)
{
	ExpandPath(buffer, m_tszFormat, size);
}

void CFolderItem::Save()
{
	FolderDeleteOldDirectory(FALSE);
	FolderCreateDirectory(FALSE);
	WriteDataToDatabase();
}

int CFolderItem::FolderCreateDirectory(int showFolder)
{
	if (m_tszFormat == NULL)
		return FOLDER_SUCCESS;

	TCHAR buffer[MAX_FOLDER_SIZE];
	ExpandPath(buffer, m_tszFormat, SIZEOF(buffer));
	CreateDirectoryTreeT(buffer);
	if (showFolder)
		ShellExecute(NULL, L"explore", buffer, NULL, NULL, SW_SHOW);

	return (DirectoryExists(buffer)) ? FOLDER_SUCCESS : FOLDER_FAILURE;
}

int CFolderItem::FolderDeleteOldDirectory(int showFolder)
{
	if (!m_tszOldFormat)
		return FOLDER_SUCCESS;

	if (!mir_tstrcmp(m_tszFormat, m_tszOldFormat)) //format wasn't changed
		return FOLDER_SUCCESS;

	TCHAR buffer[MAX_FOLDER_SIZE];
	ExpandPath(buffer, m_tszOldFormat, SIZEOF(buffer));
	RemoveDirectories(buffer);
	int res = (DirectoryExists(buffer)) ? FOLDER_FAILURE : FOLDER_SUCCESS;
	if ((res == FOLDER_FAILURE) && (showFolder))
		ShellExecute(NULL, _T("explore"), buffer, NULL, NULL, SW_SHOW);
	return res;
}

void CFolderItem::GetDataFromDatabase(const TCHAR *szNotFound)
{
	char name[256];
	strcpy_s(name, sizeof(name), m_szSection);
	strcat_s(name, sizeof(name), m_szName);

	TCHAR buffer[MAX_FOLDER_SIZE];
	GetStringFromDatabase(name, szNotFound, buffer, SIZEOF(buffer));
	SetFormat(buffer);
}

void CFolderItem::WriteDataToDatabase()
{
	char szSettingName[256];
	strcpy_s(szSettingName, sizeof(szSettingName), m_szSection);
	strcat_s(szSettingName, sizeof(szSettingName), m_szName);

	if (m_tszFormat)
		db_set_ts(NULL, ModuleName, szSettingName, m_tszFormat);
}
