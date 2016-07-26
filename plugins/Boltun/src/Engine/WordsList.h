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

class WordsList
{
	WordsList();
	void Parse(std::wstring s/*, bool allowUnstrict = false*/);
	//bool unstrict;
#ifdef _DEBUG
public:
#endif
	std::set<std::wstring> words;
	bool isQuestion;
public:
	WordsList(const std::wstring &data/*, bool allowUnstrict = false*/);
	operator std::wstring() const;
	bool MatchesAll(const std::vector<std::wstring>& s/*, bool& WasStrict*/, float& priority) const;
	bool MatchesAny(const std::vector<std::wstring>& s) const;
	std::vector<std::wstring> ConsistsOf(const std::set<std::wstring>& list) const;
	std::vector<std::wstring> DoesntIncludeAny(const std::set<std::wstring>& list) const;
	WordsList& operator= (const std::wstring& s);
	bool operator<(const WordsList& value) const;
	bool operator==(const WordsList& value) const;
	bool operator!=(const WordsList& value) const;
	bool IsQuestion() const;
	bool IsEmpty() const;
	size_t Size() const;
};

#endif
