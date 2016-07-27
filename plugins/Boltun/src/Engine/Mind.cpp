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

typedef vector<wstring> string_vec;
typedef multimap<wstring, wstring> string_mmap;

Mind::Mind()
{
	data = new MindData();
	data->referenceCount = 1;
	data->maxSmileLen = 0;
}

Mind::~Mind()
{
	if (--data->referenceCount == 0)
		delete data;
}

Mind::Mind(const Mind& mind)
{
	mind.data->referenceCount++;
	data = mind.data;
}

const MindData *Mind::GetData() const
{
	return data;
}

Mind& Mind::operator= (const Mind& mind)
{
	if (--data->referenceCount == 0)
		delete data;
	mind.data->referenceCount++;
	data = mind.data;
	return *this;
}

inline void format(wstring& s)
{
	int pos = (int)s.length() - 1;
	if (s[pos] == '\r')
		s.resize(pos);
}

void toLowerStr(wchar_t* ch)
{
	CharLower(ch);
}

vector<wstring> Mind::Parse(wstring s)
{
	int len = (int)s.length() - 1;
	vector <wstring> res;
	while (len != -1 && iswspace(s[len]))
		len--;
	if (len < 0)
		return res;
	s.resize(len);
	int it = 0;
	while (it != len) {
		while (it != len && iswspace(s[it]))
			it++;
		if (it == len)
			break;
		int start = it;
		while (it != len && !iswspace(s[it]))
			it++;
		res.push_back(s.substr(start, it - start));
	}
	return res;
}

void Mind::Load(wstring filename)
{
	basic_ifstream<wchar_t, char_traits<wchar_t> > file;

	locale ulocale(locale(), new MyCodeCvt);
	file.imbue(ulocale);

	file.open(filename.c_str(), ios_base::in | ios_base::binary);
	wstring s1, st;
	wchar_t *c, *co = NULL;
	size_t count;
	int error = 0;
	int line = 1;

	bool start = true;

	try {
		while (file.good()) {
			getline(file, st);
			if (st.empty())
				break;
			line++;

			if (start) {
				if (st[0] == 65279) {
					st.erase(0, 1);
					fileTypeMark = true;
				}
				else
					fileTypeMark = false;
				start = false;
			}

			format(st);
			count = st.length();
			c = co = new wchar_t[count + 1];
			mir_wstrcpy(c, st.c_str());
			size_t pos = 0;
			while (pos < count && iswspace(*c)) {
				++pos;
				++c;
			}
			count -= pos;
			if (count > 2) {
				switch (*c) {
				case '(':
					if (c[count - 1] != ')')
						abort();
					if (file.eof())
						throw error;
					getline(file, s1);
					line++;
					format(s1);
					++c;
					count -= 2;
					c[count] = '\0';
					toLowerStr(c);
					{
						WordsList l(c);
						if (!l.IsEmpty()) {
							if (l.IsQuestion())
								data->qkeywords.insert(make_pair(l, s1));
							else
								data->keywords.insert(make_pair(l, s1));
						}
					}
					break;

				case '{':
					if (c[count - 1] != '}')
						abort();
					if (file.eof())
						throw error;
					getline(file, s1);
					line++;
					format(s1);
					++c;
					count -= 2;
					c[count] = '\0';
					toLowerStr(c);
					{
						WordsList l(c);
						if (!l.IsEmpty())
							if (l.IsQuestion())
								data->qspecialEscapes.insert(make_pair(l, s1));
							else
								data->specialEscapes.insert(make_pair(l, s1));
					}
					break;

				case '[':
					if (c[count - 1] != ']')
						throw error;
					if (file.eof())
						throw error;
					getline(file, s1);
					line++;
					format(s1);
					++c;
					count -= 2;
					c[count] = '\0';
					toLowerStr(c);
					data->widelyUsed.insert(make_pair(c, s1));
					break;

				case '<':
					if (c[count - 1] != '>')
						throw error;
					if (file.eof())
						throw error;
					getline(file, s1);
					line++;
					format(s1);
					++c;
					count -= 2;
					c[count] = '\0';
					if (mir_wstrcmp(c, L"QUESTION") == 0) {
						toLowerStr(c);
						data->question.insert(s1);
					}
					else if (mir_wstrcmp(c, L"IGNORED") == 0) {
						toLowerStr(c);
						data->special.insert(s1);
					}
					else if (mir_wstrcmp(c, L"ESCAPE") == 0) {
						data->escape.push_back(s1);
					}
					else if (mir_wstrcmp(c, L"FAILURE") == 0) {
						data->failure.push_back(s1);
					}
					else if (mir_wstrcmp(c, L"REPEAT") == 0) {
						data->repeats.push_back(s1);
					}
					else {
						if (mir_wstrcmp(c, L"INITIAL") != 0)
							throw error;
						data->initial.push_back(s1);
					}
					break;

				case '@':
					if (file.eof())
						throw error;

					getline(file, s1);
					line++;
					format(s1);
					++c;
					count -= 1;
					toLowerStr(c);
					{
						std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
						vector<wstring> strs = Parse(s1);
						data->raliases.insert(make_pair(s1, strs));
						for (vector<wstring>::const_iterator it = strs.begin(); it != strs.end(); ++it)
							data->aliases.insert(make_pair(*it, s1));
					}
					break;

				default:
					if (file.eof())
						throw error;
					getline(file, s1);
					line++;
					format(s1);
					toLowerStr(c);
					data->study.insert(make_pair(c, s1));
				}
			}
			else if (count) {
				if (file.eof())
					throw error;
				getline(file, s1);
				line++;
				format(s1);
				data->study.insert(make_pair(c, s1));
			}
		}
		if (!file.eof())
			throw error;

		delete co;
	}
	catch (...) {
		throw CorruptedMind(line);
	}
}

void Mind::Save(wstring filename) const
{
	basic_ofstream<wchar_t, char_traits<wchar_t> > file;

	locale ulocale(locale(), new MyCodeCvt);
	file.imbue(ulocale);

	file.open(filename.c_str(), ios_base::out | ios_base::binary);

	if (fileTypeMark)
		file << wchar_t(65279);

	for (string_mmap::iterator it = data->study.begin(); it != data->study.end(); ++it) {
		file << (*it).first << '\r' << endl;
		file << (*it).second << '\r' << endl;
	}
	for (multimap<WordsList, wstring>::iterator it = data->keywords.begin(); it != data->keywords.end(); ++it) {
		file << L" (" << (wstring)(*it).first << L")" << '\r' << endl;
		file << (*it).second << '\r' << endl;
	}
	for (multimap<WordsList, wstring>::iterator it = data->qkeywords.begin(); it != data->qkeywords.end(); ++it) {
		file << L" (" << (wstring)(*it).first << L")" << '\r' << endl;
		file << (*it).second << '\r' << endl;
	}
	for (multimap<WordsList, wstring>::iterator it = data->specialEscapes.begin(); it != data->specialEscapes.end(); ++it) {
		file << L" {" << (wstring)(*it).first << L"}" << '\r' << endl;
		file << (*it).second << '\r' << endl;
	}
	for (multimap<WordsList, wstring>::iterator it = data->qspecialEscapes.begin(); it != data->qspecialEscapes.end(); ++it) {
		file << L" {" << (wstring)(*it).first << L"}" << '\r' << endl;
		file << (*it).second << '\r' << endl;
	}
	for (string_mmap::iterator it = data->widelyUsed.begin(); it != data->widelyUsed.end(); ++it) {
		file << L" [" << (*it).first << L"]" << '\r' << endl;
		file << (*it).second << '\r' << endl;
	}
	for (set<wstring>::iterator it = data->question.begin(); it != data->question.end(); ++it) {
		file << L" <QUESTION>" << '\r' << endl;
		file << (*it) << '\r' << endl;
	}
	for (set<wstring>::iterator it = data->special.begin(); it != data->special.end(); ++it) {
		file << L" <IGNORED>" << '\r' << endl;
		file << (*it) << '\r' << endl;
	}
	for (string_vec::iterator it = data->escape.begin(); it != data->escape.end(); ++it) {
		file << L" <ESCAPE>" << '\r' << endl;
		file << (*it) << '\r' << endl;
	}
	for (string_vec::iterator it = data->initial.begin(); it != data->initial.end(); ++it) {
		file << L" <INITIAL>" << '\r' << endl;
		file << (*it) << '\r' << endl;
	}
	for (string_vec::iterator it = data->failure.begin(); it != data->failure.end(); ++it) {
		file << L" <FAILURE>" << '\r' << endl;
		file << (*it) << '\r' << endl;
	}
	for (string_vec::iterator it = data->repeats.begin(); it != data->repeats.end(); ++it) {
		file << L" <REPEAT>" << '\r' << endl;
		file << (*it) << '\r' << endl;
	}
	for (map<wstring, vector<wstring>>::const_iterator it = data->raliases.begin(); it != data->raliases.end(); ++it) {
		wstring s;
		const vector<wstring>& v = (*it).second;
		bool first = true;
		for (vector<wstring>::const_iterator it1 = v.begin(); it1 != v.end(); ++it1) {
			if (first) {
				first = false;
				s = *it1;
			}
			else {
				s += L" " + *it1;
			}
		}
		file << '@' << (*it).first << '\r' << endl;
		file << s << '\r' << endl;
	}
}

void Mind::LoadSmiles(wstring filename)
{
	basic_ifstream<wchar_t, char_traits<wchar_t> > file;
	file.open(filename.c_str());
	data->smiles.clear();
	wstring s;
	unsigned int l = 0;
	while (!file.eof()) {
		getline(file, s);
		if (s.length() > l)
			l = (int)s.length();
		data->smiles.insert(s);
	}
	data->maxSmileLen = l;
}

void Mind::LoadSmiles(void *smiles, size_t size)
{
	data->smiles.clear();
	wchar_t* buf = (wchar_t*)smiles;
	unsigned l = 0;
	wchar_t* end = buf + size;
	while (buf != end) {
		wchar_t *lend = buf;
		while (lend != end && *lend != '\r')
			lend++;
		wstring s(buf, lend - buf);
		if ((unsigned)(lend - buf) > l)
			l = (int)s.length();
		data->smiles.insert(s);
		if (lend == end)
			break;
		buf = lend + 2;
	}
	data->maxSmileLen = l;
}
