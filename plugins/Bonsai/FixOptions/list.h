/*
Bonsai plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#ifndef M_BONSAI_LIST_H
#define M_BONSAI_LIST_H

#define INITIAL_SIZE 20

#include "commonheaders.h"

#define ID_GROUP    1
#define ID_TITLE    2

struct TPluginOptions{
	WCHAR *szPlugin;
	WCHAR *szGroup;
	WCHAR *szTitle;
	
	DWORD pluginHash;
	DWORD groupHash;
	DWORD titleHash;
	
	TPluginOptions(const WCHAR *plugin, const WCHAR *group, const WCHAR *title, DWORD pluginHash, DWORD groupHash, DWORD titleHash)
	{
		szPlugin = _wcsdup(plugin);
		szGroup = _wcsdup(group);
		szTitle = _wcsdup(title);
		this->pluginHash = pluginHash;
		this->groupHash = groupHash;
		this->titleHash = titleHash;
	}
	
	~TPluginOptions()
	{
		free(this->szPlugin);
		free(this->szGroup);
		free(this->szTitle);
	}
};

typedef TPluginOptions *PPluginOptions;

class COptionsList{
	protected:
		PPluginOptions *_plugins;
		int _count;
		int _capacity;
		
		void Enlarge(int increaseAmount);
		void EnsureCapacity();		
	
	public:
		COptionsList(int initialSize = INITIAL_SIZE);
		~COptionsList();

		void Clear();
		
		int Add(const WCHAR *plugin, const WCHAR *group, const WCHAR *title);
		int Remove(int index);
		int Contains(const WCHAR *plugin, const WCHAR *group, const WCHAR *title) const;
		int Index(const WCHAR *plugin, const WCHAR *group, const WCHAR *title) const;
		
		const PPluginOptions operator [](int index);
		
		int Count() const;
		int Capacity() const;
};

extern COptionsList &lstPluginOptions; //list of services

WCHAR *BuildPluginUniqueID(PPluginOptions plugin, WCHAR *out, int size, int type);
WCHAR *GetPluginGroup(PPluginOptions plugin, WCHAR *group, int count);
WCHAR *GetPluginTitle(PPluginOptions plugin, WCHAR *title, int count);
void SavePluginGroup(PPluginOptions plugin, WCHAR *newGroup);
void SavePluginTitle(PPluginOptions plugin, WCHAR *newTitle);

#endif //M_SERVICESLIST_LIST_H