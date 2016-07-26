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

#include "../stdafx.h"

using namespace std;

WordsList::WordsList(const wstring &data/*, bool allowUnstrict*/)
{
	Parse(data/*, allowUnstrict*/);
}

WordsList::operator wstring() const
{
	wstring res;
	//if (unstrict)
	//	res = L"~";
	set<wstring>::const_iterator it = words.begin();
	if (!words.empty())
		while (true)
		{
			res += *it;
			++it;
			if (it != words.end())
				res += L" ";
			else
				break;
		}
	res += isQuestion ? L"?" : L".";
	return res;
}

WordsList& WordsList::operator= (const wstring& s)
{
	Parse(s);
	return *this;
};

void WordsList::Parse(wstring s/*, bool allowUnstrict*/)
{
	isQuestion = false;
	/*if (allowUnstrict && s.length() && s[0] == '~')
	{
	s = s.substr(1, s.npos);
	unstrict = true;
	}
	else
	unstrict = false;*/
	int len = (int)s.length() - 1;
	while (len != -1 && iswspace(s[len]))
		len--;
	if (len < 0)
		return;
	if (s[len] == '?')
		isQuestion = true;
	else
		if (s[len] != '.')
			return;
	s.resize(len);
	int it = 0;
	while (it != len)
	{
		while (it != len && iswspace(s[it]))
			it++;
		if (it == len)
			break;
		int start = it;
		while (it != len && !iswspace(s[it]))
			it++;
		words.insert(s.substr(start, it - start));
	}
}

bool WordsList::MatchesAll(const vector<wstring>& s/*, bool& WasStrict*/, float& priority) const
{
	std::set<wstring> temp;
	//WasStrict = true;
	for (vector<wstring>::const_iterator it = s.begin(); it != s.end(); ++it)
	{
		/*		if (words.find(*it) == words.end())
					if (unstrict)
					{
					WasStrict = false;
					continue;
					}
					else
					return false;
					temp.insert((*it));*/
		if (words.find(*it) != words.end())
			temp.insert((*it));
	}
	if (temp.size() != words.size())
		return false;
	priority = words.size() * words.size() / (float)s.size();
	return temp.size() == words.size();
}

bool WordsList::MatchesAny(const vector<wstring>& s) const
{
	for (vector<wstring>::const_iterator it = s.begin(); it != s.end(); ++it)
		if (words.find(*it) != words.end())
			return true;
	return false;
}

vector<wstring> WordsList::ConsistsOf(const set<wstring>& list) const
{
	vector<wstring> res;
	for (set<wstring>::const_iterator it = words.begin(); it != words.end(); ++it)
		if (list.find(*it) == list.end())
			res.push_back(*it);
	return res;
}

vector<wstring> WordsList::DoesntIncludeAny(const set<wstring>& list) const
{
	vector<wstring> res;
	for (set<wstring>::const_iterator it = words.begin(); it != words.end(); ++it)
		if (list.find(*it) != list.end())
			res.push_back(*it);
	return res;
}

bool WordsList::operator<(const WordsList& value) const
{
	return (wstring)*this < (wstring)value;
}

bool WordsList::operator!=(const WordsList& value) const
{
	return (wstring)*this != (wstring)value;
}

bool WordsList::operator==(const WordsList& value) const
{
	return (wstring)*this == (wstring)value;
}

size_t WordsList::Size() const
{
	return words.size();
}

bool WordsList::IsQuestion() const
{
	return isQuestion;
}

bool WordsList::IsEmpty() const
{
	return words.size() == 0;
}