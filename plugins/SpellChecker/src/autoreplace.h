/* 
Copyright (C) 2009-2010 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __AUTOREPLACE_H__
# define __AUTOREPLACE_H__

struct AutoReplacement
{
	std::wstring replace;
	BOOL useVariables;

	AutoReplacement();
	AutoReplacement(const wchar_t *replace, BOOL useVariables);
};

struct Dictionary;

class AutoReplaceMap 
{
private:
	wchar_t m_filename[1024];
	Dictionary *m_dict;
	map<std::wstring, AutoReplacement> m_replacements;

	void loadAutoReplaceMap();
	void writeAutoReplaceMap();

public:
	AutoReplaceMap(wchar_t *filename, Dictionary *dict);

	wchar_t* filterText(const wchar_t *find);
	BOOL isWordChar(wchar_t c);

	/// Return an auto replacement to a word or NULL if none exists.
	/// You have to free the item.
	CMStringW autoReplace(const wchar_t *word);

	/// Add a word to the list of auto-replaced words
	void add(const wchar_t *from, const wchar_t *to, BOOL useVariables = FALSE);

	/// Make a copy of the auto replace map
	void copyMap(map<std::wstring, AutoReplacement> *replacements);

	/// Make a copy of the auto replace map
	void setMap(const map<std::wstring, AutoReplacement> &replacements);
};


	
#endif // __AUTOREPLACE_H__
