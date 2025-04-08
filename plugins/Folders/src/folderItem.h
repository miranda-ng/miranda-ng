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

#ifndef M_FOLDERS_FOLDER_ITEM_H
#define M_FOLDERS_FOLDER_ITEM_H

#define FOLDERS_NO_HELPER_FUNCTIONS
#include "m_folders.h"
#undef FOLDERS_NO_HELPER_FUNCTIONS


#define FOLDER_SUCCESS 1
#define FOLDER_FAILURE 0

class CFolderItem
{
	HPLUGIN m_hPlugin;
	char  *m_szSection, *m_szName;
	wchar_t *m_tszFormat, *m_tszOldFormat;

	void GetDataFromDatabase(const wchar_t *szNotFound);
	void WriteDataToDatabase();

	int FolderCreateDirectory(int showFolder = 0);
	int FolderDeleteOldDirectory(int showFolder = 0);

public:
	CFolderItem(HPLUGIN, const char *sectionName, const char *name, const wchar_t *format);
	virtual ~CFolderItem();

	CMStringW Expand();
	void Save();

	bool IsEqual(const CFolderItem *other);
	bool IsEqual(const char *section, const char *name);
	int operator ==(const CFolderItem *other);

	__inline HPLUGIN GetPlugin() const { return m_hPlugin; }
	__inline const char* GetSection() const { return m_szSection; }
	__inline const char* GetName() const { return m_szName; }
	__inline const wchar_t* GetFormat() const { return m_tszFormat; }
	void SetFormat(const wchar_t *newFormat);
};

#endif //M_FOLDERS_FOLDER_ITEM_H
