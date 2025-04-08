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

#include "stdafx.h"

CFolderItem::CFolderItem(HPLUGIN hPlugin, const char *sectionName, const char *name, const wchar_t *format) :
	m_hPlugin(hPlugin)
{
	m_szSection = mir_strdup(sectionName);
	m_szName = mir_strdup(name);
	m_tszFormat = nullptr;
	m_tszOldFormat = nullptr;
	GetDataFromDatabase(format);
	FolderCreateDirectory();
}

CFolderItem::~CFolderItem()
{
	mir_free(m_szSection);
	mir_free(m_szName);
	mir_free(m_tszFormat);
	mir_free(m_tszOldFormat);
}

void CFolderItem::SetFormat(const wchar_t *newFormat)
{
	mir_free(m_tszOldFormat);
	m_tszOldFormat = m_tszFormat;
	m_tszFormat = mir_wstrdup(*newFormat ? newFormat : MIRANDA_PATHW);
}

bool CFolderItem::IsEqual(const CFolderItem *other)
{
	return IsEqual(other->GetSection(), other->GetName());
}

bool CFolderItem::IsEqual(const char *section, const char *name)
{
	return !mir_strcmp(m_szName, name) && !mir_strcmp(m_szSection, section);
}

int CFolderItem::operator ==(const CFolderItem *other)
{
	return IsEqual(other);
}

CMStringW CFolderItem::Expand()
{
	return ExpandPath(m_tszFormat);
}

void CFolderItem::Save()
{
	FolderDeleteOldDirectory(FALSE);
	FolderCreateDirectory(FALSE);
	WriteDataToDatabase();
}

int CFolderItem::FolderCreateDirectory(int showFolder)
{
	if (m_tszFormat == nullptr)
		return FOLDER_SUCCESS;

	CMStringW buffer(ExpandPath(m_tszFormat));
	CreateDirectoryTreeW(buffer);
	if (showFolder)
		ShellExecute(nullptr, L"explore", buffer, nullptr, nullptr, SW_SHOW);

	return (DirectoryExists(buffer)) ? FOLDER_SUCCESS : FOLDER_FAILURE;
}

int CFolderItem::FolderDeleteOldDirectory(int showFolder)
{
	if (!m_tszOldFormat)
		return FOLDER_SUCCESS;

	if (!mir_wstrcmp(m_tszFormat, m_tszOldFormat)) //format wasn't changed
		return FOLDER_SUCCESS;

	CMStringW buffer(ExpandPath(m_tszOldFormat));
	RemoveDirectories(buffer);
	int res = (DirectoryExists(buffer)) ? FOLDER_FAILURE : FOLDER_SUCCESS;
	if ((res == FOLDER_FAILURE) && (showFolder))
		ShellExecute(nullptr, L"explore", buffer, nullptr, nullptr, SW_SHOW);
	return res;
}

void CFolderItem::GetDataFromDatabase(const wchar_t *szNotFound)
{
	char szSettingName[256];
	strcpy_s(szSettingName, _countof(szSettingName), m_szSection);
	strcat_s(szSettingName, _countof(szSettingName), m_szName);

	ptrW tszValue(g_plugin.getWStringA(szSettingName));
	SetFormat(tszValue != NULL ? tszValue : szNotFound);
}

void CFolderItem::WriteDataToDatabase()
{
	char szSettingName[256];
	strcpy_s(szSettingName, sizeof(szSettingName), m_szSection);
	strcat_s(szSettingName, sizeof(szSettingName), m_szName);

	if (m_tszFormat)
		g_plugin.setWString(szSettingName, m_tszFormat);
}
