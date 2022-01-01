/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef M_OPTIONS_FILTERING_H
#define M_OPTIONS_FILTERING_H

extern HANDLE hOptionsInitialize;

typedef uint32_t PageHash;

void	AddFilterString(const PageHash key, const wchar_t *data);
BOOL	ContainsFilterString(const PageHash key, wchar_t *data);
void	ClearFilterStrings();
void	GetDialogStrings(int enableKeywordFiltering, const PageHash key, wchar_t *pluginName, HWND hWnd,  wchar_t *group, wchar_t *title, wchar_t *tab, wchar_t *name);

_inline wchar_t *_tcslwr_locale(wchar_t *buf)
{
	LCMapString(Langpack_GetDefaultLocale() , LCMAP_LOWERCASE, buf, (int)mir_wstrlen(buf), buf, (int)mir_wstrlen(buf));
	return buf;
}

typedef LIST<wchar_t> KeywordList;
class CPageKeywords
{
	PageHash	_pageHashKey;
	KeywordList _pageKeyWords;
	static int _KeyWordsSortFunc(const wchar_t* p1, const wchar_t* p2)	{ return mir_wstrcmp(p1, p2); };

public:
	CPageKeywords(PageHash pageHashKey) : _pageHashKey(pageHashKey), _pageKeyWords(1, _KeyWordsSortFunc) {};
	~CPageKeywords()
	{
		for (auto &it : _pageKeyWords)
			mir_free(it);
	};

	void AddKeyWord(wchar_t *ptKeyWord)
	{
		wchar_t *plwrWord = _tcslwr_locale(mir_wstrdup(ptKeyWord));
		if (_pageKeyWords.getIndex(plwrWord) == -1)
			_pageKeyWords.insert(plwrWord);
		else
			mir_free(plwrWord);
	};

	BOOL ContainsString(wchar_t *data)
	{
		for (auto &it : _pageKeyWords)
			if (wcsstr(it, data))
				return TRUE;
		return FALSE;
	}
	static int PageSortFunc(const CPageKeywords* p1, const CPageKeywords* p2)
	{
		if (p1->_pageHashKey < p2->_pageHashKey) { return -1; }
		else if (p1->_pageHashKey > p2->_pageHashKey) { return 1; }
		return 0;
	}
};

class CPageList : public OBJLIST<CPageKeywords>
{
	CPageList();
public:
	CPageList(	int aincr, FTSortFunc afunc = CPageKeywords::PageSortFunc) : OBJLIST<CPageKeywords>(aincr, afunc) {};
	CPageKeywords	* operator[](PageHash key)
	{
		CPageKeywords keyToSearch(key);
		return this->find(&keyToSearch);
	}
	~CPageList() {};
};

#endif //M_OPTIONS_FILTERING_H
