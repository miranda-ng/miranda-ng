/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (C) 2012-22 Miranda NG team

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
	char *m_szTitle = 0;
	char *m_szFrom = 0;
	char *m_szTags = 0;
	char *m_szTagsStr = 0;

	wchar_t *m_szText = 0;

public:
	CNoteItem();
	CNoteItem(const TiXmlElement *hXml, const char *szFrom = nullptr);
	~CNoteItem();

	void SetData(const char *title, const char *from, const wchar_t *text, const char *tags);

	char* GetTitle() const { return m_szTitle; }
	char* GetFrom() const { return m_szFrom; }
	char* GetTags() const { return m_szTags; }
	char* GetTagsStr() const { return m_szTagsStr; }
	wchar_t* GetText() const { return m_szText; }

	bool HasTag(const char *szTag);

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

	void LoadXml(const TiXmlElement *hXml);
	void SaveXml(TiXmlElement *hXmlParent);

	bool IsModified() { return m_bIsModified; }
	void Modify() { m_bIsModified = true; }
};

#endif // __jabber_notes_h__
