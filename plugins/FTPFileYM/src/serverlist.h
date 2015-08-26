/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "stdafx.h"

class ServerList
{
private:
	static ServerList *instance;
	ServerList() { };
	~ServerList() { instance = NULL; };

public:
	class FTP
	{
	public:
		enum EProtoType
		{	
			FT_STANDARD	= 0, 
			FT_SSL_EXPLICIT, 
			FT_SSL_IMPLICIT, 
			FT_SSH
		};

		bool m_bEnabled;	
		TCHAR m_stzName[64];
		char m_szServer[256];
		char m_szUser[64];
		char m_szPass[64];
		char m_szDir[64];
		char m_szChmod[256];
		char m_szUrl[256];
		EProtoType m_ftpProto;
		int m_iPort;
		bool m_bPassive;

		FTP(int index);

		bool isValid() const;
		char *getProtoString() const;
	};

	static const int FTP_COUNT = 5;

	vector<FTP *> m_items;

	FTP	*operator[] (int i) const { return m_items[i]; };
	void add(FTP *newItem) { m_items.push_back(newItem); }
	size_t size() { return m_items.size(); }

	static ServerList &getInstance() 
	{
		if (!instance)
			instance = new ServerList();
		return *instance;
	};

	void init();
	void deinit();
	void saveToDb() const;
	FTP *getSelected() const;	
};
