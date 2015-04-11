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

#ifndef __jabber_notes_h__
#define __jabber_notes_h__

class CNoteItem
{
private:
	TCHAR *m_szTitle;
	TCHAR *m_szFrom;
	TCHAR *m_szText;
	TCHAR *m_szTags;
	TCHAR *m_szTagsStr;

public:
	CNoteItem();
	CNoteItem(HXML hXml, TCHAR *szFrom = 0);
	~CNoteItem();

	void SetData(TCHAR *title, TCHAR *from, TCHAR *text, TCHAR *tags);

	TCHAR *GetTitle() const { return m_szTitle; }
	TCHAR *GetFrom() const { return m_szFrom; }
	TCHAR *GetText() const { return m_szText; }
	TCHAR *GetTags() const { return m_szTags; }
	TCHAR *GetTagsStr() const { return m_szTagsStr; }

	bool HasTag(const TCHAR *szTag);

	bool IsNotEmpty()
	{
		return (m_szTitle && *m_szTitle) || (m_szText && *m_szText);
	}

	static int cmp(const CNoteItem *p1, const CNoteItem *p2);
};

class CNoteList: public OBJLIST<CNoteItem>
{
private:
	bool m_bIsModified;

public:
	CNoteList() :
		OBJLIST<CNoteItem>(10, CNoteItem::cmp),
		m_bIsModified(false)
	{}

	void remove(CNoteItem *p)
	{
		m_bIsModified = true;
		OBJLIST<CNoteItem>::remove(p);
	}

	void AddNote(HXML hXml, TCHAR *szFrom = 0);
	void LoadXml(HXML hXml);
	void SaveXml(HXML hXmlParent);

	bool IsModified() { return m_bIsModified; }
	void Modify() { m_bIsModified = true; }
};

#endif // __jabber_notes_h__
