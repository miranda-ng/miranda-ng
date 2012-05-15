/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-11  George Hazan
Copyright ( C ) 2007-09  Maxim Mluhov
Copyright ( C ) 2007-09  Victor Pavlychko

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Revision       : $Revision: 13452 $
Last change on : $Date: 2011-03-17 21:12:56 +0200 (Чт, 17 мар 2011) $
Last change by : $Author: george.hazan $

*/

#ifndef _JABBER_ICOLIB_H_
#define _JABBER_ICOLIB_H_

struct CJabberProto;

class CIconPool
{
public:
	CIconPool(CJabberProto *proto);
	~CIconPool();

	void RegisterIcon(const char *name, const char *filename, int iconid, TCHAR *szSection, TCHAR *szDescription);

	HANDLE GetIcolibHandle(const char *name);
	char *GetIcolibName(const char *name);
	HICON GetIcon(const char *name, bool big = false);
	HANDLE GetClistHandle(const char *name);

private:
	struct CPoolItem
	{
		char *m_name;
		char *m_szIcolibName;
		HANDLE m_hIcolibItem;
		HANDLE m_hClistItem;

		static int cmp(const CPoolItem *p1, const CPoolItem *p2);

		CPoolItem();
		~CPoolItem();
	};

	CJabberProto *m_proto;
	OBJLIST<CPoolItem> m_items;
	HANDLE m_hOnExtraIconsRebuild;

	CPoolItem *FindItemByName(const char *name);

	int __cdecl OnExtraIconsRebuild(WPARAM, LPARAM);
	static bool ExtraIconsSupported();
};

#endif // _JABBER_ICOLIB_H_
