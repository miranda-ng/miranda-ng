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

#include "list.h"

COptionsList &lstPluginOptions = COptionsList();

COptionsList::COptionsList(int initialCapacity)
{
	_plugins = NULL;
	_count = 0;
	_capacity = 0;
	
	Enlarge(initialCapacity);
}

COptionsList::~COptionsList()
{
	Clear();
	free(_plugins);
}

void COptionsList::Clear()
{
	int i;
	for (i = 0; i < Count(); i++)
	{
		delete _plugins[i];
	}
	_count = 0;
}

int COptionsList::Count() const
{
	return _count;
}

int COptionsList::Capacity() const
{
	return _capacity;
}

void COptionsList::EnsureCapacity()
{
	if (_count >= _capacity)
		{
			Enlarge(_capacity / 2);
		}
}

void COptionsList::Enlarge(int increaseAmount)
{
	int newSize = _capacity + increaseAmount;
	_plugins = (PPluginOptions *) realloc(_plugins, newSize * sizeof(PPluginOptions));
	_capacity = newSize;
}

int COptionsList::Contains(const WCHAR *plugin, const WCHAR *group, const WCHAR *title) const
{
	int pos = Index(plugin, group, title);
	return (pos >= 0);
}

int COptionsList::Index(const WCHAR *plugin, const WCHAR *group, const WCHAR *title) const
{
	int i;
	DWORD hPlugin = LangPackHashW(plugin);
	DWORD hGroup = LangPackHashW(group);
	DWORD hTitle = LangPackHashW(title);
	PPluginOptions p;
	
	for (i = 0; i < Count(); i++)
	{
		p = _plugins[i];
		if ((hPlugin == p->pluginHash) && (hGroup == p->groupHash) && (hTitle == p->titleHash))
		{
			return i;
		}
	}
	
	return -1;
}

int COptionsList::Add(const WCHAR *plugin, const WCHAR *group, const WCHAR *title)
{
	int pos = Index(plugin, group, title);
	if (pos < 0)
	{
		EnsureCapacity();
		
		PPluginOptions s = new TPluginOptions(plugin, group, title, LangPackHashW(plugin), LangPackHashW(group), LangPackHashW(title));
		_plugins[_count++] = s;
		if (hOptDlg)
		{
			SendMessage(hOptDlg, FOM_UPDATEPLUGINS, 0, 0);
		}
		pos = _count - 1;
	}

	return pos;
}

int COptionsList::Remove(int index)
{
	if ((index < 0) && (index >= Count()))
	{
		return 1;
	}
	
	int i;
	PPluginOptions tmp = _plugins[index];
	for (i = index; i < _count - 1; i++)
	{
		_plugins[i] = _plugins[i + 1];
	}
	_count--;
	delete tmp;
	
	return 0;
}

const PPluginOptions COptionsList::operator [](int index)
{
	if ((index < 0) || (index >= Count()))
	{
		return NULL;
	}
	
	return _plugins[index];
}

char *BuildPluginUniqueID(PPluginOptions plugin, char *out, int size, int type)
{
	mir_snprintf(out, size, "%lu-%lu-%lu-%s", plugin->pluginHash, plugin->groupHash, plugin->titleHash, (type == ID_GROUP) ? "G" : "T");
	
	return out;
}

WCHAR *GetPluginGroup(PPluginOptions plugin, WCHAR *group, int count)
{
	char name[256];
	WCHAR *res = plugin->szGroup;

	BuildPluginUniqueID(plugin, name, 256, ID_GROUP);
	
	GetStringFromDatabase(name, plugin->szGroup, group, count);
	res = (wcslen(group) == 0) ? NULL : group;
	
	return res;
}

WCHAR *GetPluginTitle(PPluginOptions plugin, WCHAR *title, int count)
{
	char name[256];
	WCHAR *res = plugin->szTitle;
	
	BuildPluginUniqueID(plugin, name, 256, ID_TITLE);
	
	GetStringFromDatabase(name, plugin->szTitle, title, count);
	res = (wcslen(title) == 0) ? NULL : title;
	
	return res;
}

void SavePluginGroup(PPluginOptions plugin, WCHAR *newGroup)
{
	char name[256];
	int del = 0;
	
	BuildPluginUniqueID(plugin, name, 256, ID_GROUP);
	
	if (plugin->szGroup)
	{
		del = (wcscmp(plugin->szGroup, newGroup) == 0) ? 1 : 0;
	}
	else{
		del = ((!newGroup) || (wcslen(newGroup) == 0)) ? 1 : 0;
	}
	
	if (del)
	{
		DBDeleteContactSetting(NULL, ModuleName, name);
	}
	else{
		DBWriteContactSettingWString(NULL, ModuleName, name, newGroup);
	}
}

void SavePluginTitle(PPluginOptions plugin, WCHAR *newTitle)
{
	char name[256];
	int del = 0;
	
	BuildPluginUniqueID(plugin, name, sizeof(name), ID_TITLE);
	
	if (plugin->szTitle)
	{
		del = (wcscmp(plugin->szTitle, newTitle) == 0) ? 1 : 0;
	}
	else{
		del = ((!newTitle) || (wcslen(newTitle)) == 0) ? 1 : 0;
	}
	
	if (del)
	{
		DBDeleteContactSetting(NULL, ModuleName, name);
	}
	else{
		DBWriteContactSettingWString(NULL, ModuleName, name, newTitle);
	}
}