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

#define _CRT_SECURE_NO_WARNINGS

#ifndef M_FOLDERS_FOLDER_ITEM_H
#define M_FOLDERS_FOLDER_ITEM_H

#include <string.h>
#include <malloc.h>
#include <windows.h>

#define FOLDERS_NO_HELPER_FUNCTIONS
#include "m_folders.h"
#undef FOLDERS_NO_HELPER_FUNCTIONS


#define FOLDER_SUCCESS 1
#define FOLDER_FAILURE 0

class CFolderItem
{
	char  *m_szSection, *m_szName;
	TCHAR *m_tszFormat, *m_tszOldFormat, *m_tszUserName;

	void GetDataFromDatabase(const TCHAR *szNotFound);
	void WriteDataToDatabase();

	int FolderCreateDirectory(int showFolder = 0);
	int FolderDeleteOldDirectory(int showFolder = 0);
public:
	CFolderItem(const char *sectionName, const char *name, const TCHAR *format, const TCHAR *userName);
	virtual ~CFolderItem();

	void Expand(TCHAR *buffer, int size);
	void Save();

	int IsEqual(const CFolderItem *other);
	int IsEqual(const char *section, const TCHAR *name);
	int IsEqualTranslated(const char *trSection, const TCHAR *trName);
	int operator ==(const CFolderItem *other);

	__inline const char*  GetSection() const { return m_szSection; }
	__inline const char*  GetName() const { return m_szName; }
	__inline const TCHAR* GetUserName() const { return m_tszUserName; }
	__inline const TCHAR* GetFormat() const { return m_tszFormat; }
	void SetFormat(const TCHAR *newFormat);
};

typedef CFolderItem *PFolderItem;

#endif //M_FOLDERS_FOLDER_ITEM_H