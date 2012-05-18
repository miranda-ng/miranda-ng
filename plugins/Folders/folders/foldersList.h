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

#ifndef M_FOLDERS_LIST_H
#define M_FOLDERS_LIST_H

#include <string.h>
#include <malloc.h>

#include "folderItem.h"

#define FOLDERS_NO_HELPER_FUNCTIONS
#include "m_folders.h"
#undef FOLDERS_NO_HELPER_FUNCTIONS
#include "newpluginapi.h"
#include "m_langpack.h"

class CFoldersList{
	protected:
		PFolderItem *list; //the list
		int count;
		int capacity;
		
		void Enlarge(int increaseAmount);
		void EnsureCapacity();
		
	public:
		CFoldersList(int initialSize = 10);
		virtual ~CFoldersList();
		
		void Clear();
		int Add(CFolderItem *item);
		int Add(FOLDERSDATA data);
		void Remove(CFolderItem *item);
		void Remove(int uniqueID);
		int Contains(CFolderItem *item);
		int Contains(const char *section, const char *name);
		
		int Count();
		int Capacity();
		
		PFolderItem Get(int index);
		PFolderItem Get(const char *section, const char *name);
		PFolderItem Get(const WCHAR *section, const WCHAR *name);
		PFolderItem GetTranslated(const char *trSection, const char *trName);
		PFolderItem GetTranslated(WCHAR *trSection, const WCHAR *trName);
		int Expand(int index, char *szResult, int size);
		void Save();
};


#endif