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

#include "folderItem.h"
#include "commonheaders.h"
#include "services.h"

CFolderItem::CFolderItem(const char *sectionName, const char *name, const char *format, const DWORD flags)
{
	strncpy(szSection, sectionName, sizeof(szSection));
	szFormat = NULL;
	szOldFormat = NULL;
	szFormatW = NULL;
	szOldFormatW = NULL;
	strncpy(szName, name, sizeof(szName));
	this->flags = flags;
	GetDataFromDatabase(format);
	FolderCreateDirectory();
}

void MyFree(void *data)
{
	if (data)
		{
			free(data);
		}
}

CFolderItem::~CFolderItem()
{
//	WriteDataToDatabase();
	if (IsUnicode())
		{
			MyFree(szFormatW);
			MyFree(szOldFormatW);
		}
		else{
			MyFree(szFormat);
			MyFree(szOldFormat);
		}
}

const char *CFolderItem::GetSection() const
{
	return szSection;
}

const char *CFolderItem::GetName() const
{
	return szName;
}

const char *CFolderItem::GetFormat() const
{
	return szFormat;
}

const wchar_t *CFolderItem::GetFormatW() const
{
	return szFormatW;
}

void CFolderItem::SetFormat(const char *newFormat)
{
	MyFree(szOldFormat);
	szOldFormat = szFormat;
	szFormat = _strdup((strlen(newFormat) > 0) ? newFormat : MIRANDA_PATH);
}

void CFolderItem::SetFormatW(const wchar_t *newFormat)
{
	MyFree(szOldFormatW);
	szOldFormatW = szFormatW;
	szFormatW = _wcsdup((wcslen(newFormat) > 0) ? newFormat : MIRANDA_PATHW);
}

int CFolderItem::IsUnicode() const
{
	return (flags & FF_UNICODE);
}

int CFolderItem::IsEqual(const CFolderItem *other)
{
	return (IsEqual(other->GetSection(), other->GetName()));
}

int CFolderItem::IsEqual(const char *section, const char *name)
{
	return ((strcmp(szName, name) == 0) && (strcmp(szSection, section) == 0));
}

int CFolderItem::IsEqualTranslated(const char *trSection, const char *trName)
{
	return ((strcmp(Translate(szName), trName) == 0) && (strcmp(Translate(szSection), trSection) == 0));
}

int CFolderItem::operator ==(const CFolderItem *other)
{
	return IsEqual(other);
}

void CFolderItem::Expand(char *buffer, int size)
{
	if (IsUnicode())
		{
			ExpandPathW((wchar_t *) buffer, szFormatW, size);
		}
		else{
			ExpandPath(buffer, szFormat, size);
		}
}

void CFolderItem::Save()
{
	int res = FolderDeleteOldDirectory(FALSE);
	//FolderCreateDirectory(!res);
	FolderCreateDirectory(FALSE);
	WriteDataToDatabase();
}

int CFolderItem::FolderCreateDirectory(int showFolder)
{
	int res = FOLDER_SUCCESS;
	if (IsUnicode())
		{
			wchar_t buffer[MAX_FOLDER_SIZE];
			if (szFormatW)
				{
					ExpandPathW(buffer, szFormatW, MAX_FOLDER_SIZE);
					CreateDirectories(buffer);
					if (showFolder)
						{
							ShellExecuteW(NULL, L"explore", buffer, NULL, NULL, SW_SHOW);
						}
					res = (DirectoryExists(buffer)) ? FOLDER_SUCCESS : FOLDER_FAILURE;
				}
		}
		else{
			char buffer[MAX_FOLDER_SIZE];
			if (szFormat)
				{
					ExpandPath(buffer, szFormat, MAX_FOLDER_SIZE);
					CreateDirectories(buffer);
					if (showFolder)
						{
							ShellExecuteA(NULL, "explore", buffer, NULL, NULL, SW_SHOW);
						}
					res = (DirectoryExists(buffer)) ? FOLDER_SUCCESS : FOLDER_FAILURE;
				}
		}
	return res;
}

int CFolderItem::FolderDeleteOldDirectory(int showFolder)
{
	int res = FOLDER_SUCCESS;
	if (IsUnicode())
		{
			wchar_t buffer[MAX_FOLDER_SIZE];
			if (szOldFormatW)
				{
					if (wcscmp(szFormatW, szOldFormatW) == 0) //format wasn't changed
						{
							return res;
						}
					ExpandPathW(buffer, szOldFormatW, MAX_FOLDER_SIZE);
					RemoveDirectories(buffer);
					res = (DirectoryExists(buffer)) ? FOLDER_FAILURE : FOLDER_SUCCESS;
					if ((res == FOLDER_FAILURE) && (showFolder))
						{
							ShellExecuteW(NULL, L"explore", buffer, NULL, NULL, SW_SHOW);
						}
				}
		}
		else{
			char buffer[MAX_FOLDER_SIZE];
			if (szOldFormat)
				{
					if (strcmp(szFormat, szOldFormat) == 0) //format wasn't changed
						{
							return res;
						}
					ExpandPath(buffer, szOldFormat, MAX_FOLDER_SIZE);
					RemoveDirectories(buffer);
					res = (DirectoryExists(buffer)) ? FOLDER_FAILURE : FOLDER_SUCCESS;
					if ((res == FOLDER_FAILURE) && (showFolder))
						{
							ShellExecuteA(NULL, "explore", buffer, NULL, NULL, SW_SHOW);
						}
				}
		}
	return res;
}

void CFolderItem::GetDataFromDatabase(const char *szNotFound)
{
	char name[256];
	strcpy(name, szSection);
	strcat(name, szName);
	
	if (IsUnicode())
		{
			wchar_t buffer[MAX_FOLDER_SIZE];
			GetStringFromDatabase(name, (const wchar_t *) szNotFound, buffer, MAX_FOLDER_SIZE);
			SetFormatW(buffer);
		}
		else{
			char buffer[MAX_FOLDER_SIZE];
			GetStringFromDatabase(name, szNotFound, buffer, MAX_FOLDER_SIZE);
			SetFormat(buffer);
		}
}

void CFolderItem::WriteDataToDatabase()
{
	char name[256];
	strcpy(name, szSection);
	strcat(name, szName);

	if (IsUnicode())
		{
			if (szFormatW)
				{
					WriteStringToDatabase(name, szFormatW);
				}
		}
		else{
			if (szFormat)
				{
					WriteStringToDatabase(name, szFormat);
				}
		}
}