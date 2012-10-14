//***********************************************************
//	Copyright © 2008 Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#ifndef WordsListH
#define WordsListH

#include <set>
#include <vector>
#include <string>
#include "tchar.h"
#include "tstring.h"

class WordsList
{
	WordsList();
	void Parse(tstring s/*, bool allowUnstrict = false*/);
	//bool unstrict;
#ifdef _DEBUG
public:
#endif
	std::set<tstring> words;
	bool isQuestion;
public:
	WordsList(const tstring &data/*, bool allowUnstrict = false*/);
	operator tstring() const;
	bool MatchesAll(const std::vector<tstring>& s/*, bool& WasStrict*/, float& priority) const;
	bool MatchesAny(const std::vector<tstring>& s) const;
	std::vector<tstring> ConsistsOf(const std::set<tstring>& list) const;
	std::vector<tstring> DoesntIncludeAny(const std::set<tstring>& list) const;
	WordsList& operator= (const tstring& s);
	bool operator<(const WordsList& value) const;
	bool operator==(const WordsList& value) const;
	bool operator!=(const WordsList& value) const;
	bool IsQuestion() const;
	bool IsEmpty() const;
	size_t Size() const;
};

#endif
