/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (ñ) 2012-15 Miranda NG project

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

#ifndef _JABBER_ICOLIB_H_
#define _JABBER_ICOLIB_H_

struct CJabberProto;

class CIconPool
{
public:
	CIconPool();
	~CIconPool();

	void RegisterIcon(const char *name, TCHAR *filename, int iconid, TCHAR *szSection, TCHAR *szDescription);

	HANDLE GetIcolibHandle(const char *name);
	char *GetIcolibName(const char *name);
	HICON GetIcon(const char *name, bool big = false);

private:
	struct CPoolItem
	{
		char *m_name;
		char *m_szIcolibName;
		HANDLE m_hIcolibItem;

		static int cmp(const CPoolItem *p1, const CPoolItem *p2);

		CPoolItem();
		~CPoolItem();
	};

	OBJLIST<CPoolItem> m_items;

	CPoolItem *FindItemByName(const char *name);
};

#endif // _JABBER_ICOLIB_H_
