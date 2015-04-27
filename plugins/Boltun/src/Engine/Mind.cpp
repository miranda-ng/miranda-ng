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

#include "..\stdafx.h"

using namespace std;

typedef vector<tstring> string_vec;
typedef multimap<tstring, tstring> string_mmap;

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

inline void format(tstring& s)
{
	int pos = (int)s.length() - 1;
	if (s[pos] == _T('\r'))
		s.resize(pos);
}

void toLowerStr(TCHAR* ch)
{
	CharLower(ch);
}

vector<tstring> Mind::Parse(tstring s)
{
	int len = (int)s.length() - 1;
	vector <tstring> res;
	while (len != -1 && _istspace(s[len]))
		len--;
	if (len < 0)
		return res;
	s.resize(len);
	int it = 0;
	while (it != len)
	{
		while (it != len && _istspace(s[it]))
			it++;
		if (it == len)
			break;
		int start = it;
		while (it != len && !_istspace(s[it]))
			it++;
		res.push_back(s.substr(start, it - start));
	}
	return res;
}

void Mind::Load(tstring filename)
{
	basic_ifstream<TCHAR, char_traits<TCHAR> > file;

	locale ulocale(locale(), new MyCodeCvt);
	file.imbue(ulocale);

	file.open(filename.c_str(), ios_base::in | ios_base::binary);
	tstring s1, st;
	TCHAR *c, *co = NULL;
	size_t count;
	int error = 0;
	int line = 1;

	bool start = true;

	try
	{
		while (file.good())
		{
			getline(file, st);
			if (st.empty())
				break;
			line++;

			if (start)
			{
				if (st[0] == 65279)
				{
					st.erase(0, 1);
					fileTypeMark = true;
				}
				else
					fileTypeMark = false;
				start = false;
			}

			format(st);
			count = st.length();
			c = co = new TCHAR[count + 1];
			_tcscpy(c, st.c_str());
			size_t pos = 0;
			while (pos < count && _istspace(*c))
			{
				++pos;
				++c;
			}
			count -= pos;
			if (count > 2)
			{
				switch (*c)
				{
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
						if (!l.IsEmpty())
							if (l.IsQuestion())
								data->qkeywords.insert(make_pair(l, s1));
							else
								data->keywords.insert(make_pair(l, s1));
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
					if (_tcscmp(c, _T("QUESTION")) == 0)
					{
						toLowerStr(c);
						data->question.insert(s1);
					}
					else
						if (_tcscmp(c, _T("IGNORED")) == 0)
						{
							toLowerStr(c);
							data->special.insert(s1);
						}
						else
							if (_tcscmp(c, _T("ESCAPE")) == 0)
							{
								data->escape.push_back(s1);
							}
							else
								if (_tcscmp(c, _T("FAILURE")) == 0)
								{
									data->failure.push_back(s1);
								}
								else
									if (_tcscmp(c, _T("REPEAT")) == 0)
									{
										data->repeats.push_back(s1);
									}
									else
									{
										if (_tcscmp(c, _T("INITIAL")) != 0)
											throw error;
										data->initial.push_back(s1);
									}
					break;
				case '@':
				{
					if (file.eof())
						throw error;
					getline(file, s1);
					line++;
					format(s1);
					++c;
					count -= 1;
					toLowerStr(c);
					tstring sc(c);
					int count1 = (int)s1.length();
					TCHAR *c = new TCHAR[count1 + 1];
					_tcscpy(c, s1.c_str());
					CharLower(c);
					s1 = c;
					delete c;
					vector<tstring> strs = Parse(s1);
					data->raliases.insert(make_pair(sc, strs));
					for (vector<tstring>::const_iterator it = strs.begin(); it != strs.end(); ++it)
						data->aliases.insert(make_pair(*it, sc));
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
			else
				if (count)
				{
					if (file.eof())
						throw error;
					getline(file, s1);
					line++;
					format(s1);
					data->study.insert(make_pair(c, s1));
				}
		}
		if (!file.eof())
		{
			throw error;
		}
		delete co;
	}
	catch (...)
	{
		throw CorruptedMind(line);
		delete co;
	}
}

void Mind::Save(tstring filename) const
{
	basic_ofstream<TCHAR, char_traits<TCHAR> > file;

	locale ulocale(locale(), new MyCodeCvt);
	file.imbue(ulocale);

	file.open(filename.c_str(), ios_base::out | ios_base::binary);

	if (fileTypeMark)
		file << TCHAR(65279);

	for (string_mmap::iterator it = data->study.begin(); it != data->study.end(); ++it)
	{
		file << (*it).first << _T('\r') << endl;
		file << (*it).second << _T('\r') << endl;
	}
	for (multimap<WordsList, tstring>::iterator it = data->keywords.begin(); it != data->keywords.end(); ++it)
	{
		file << _T(" (") << (tstring)(*it).first << _T(")") << _T('\r') << endl;
		file << (*it).second << _T('\r') << endl;
	}
	for (multimap<WordsList, tstring>::iterator it = data->qkeywords.begin(); it != data->qkeywords.end(); ++it)
	{
		file << _T(" (") << (tstring)(*it).first << _T(")") << _T('\r') << endl;
		file << (*it).second << _T('\r') << endl;
	}
	for (multimap<WordsList, tstring>::iterator it = data->specialEscapes.begin(); it != data->specialEscapes.end(); ++it)
	{
		file << _T(" {") << (tstring)(*it).first << _T("}") << _T('\r') << endl;
		file << (*it).second << _T('\r') << endl;
	}
	for (multimap<WordsList, tstring>::iterator it = data->qspecialEscapes.begin(); it != data->qspecialEscapes.end(); ++it)
	{
		file << _T(" {") << (tstring)(*it).first << _T("}") << _T('\r') << endl;
		file << (*it).second << _T('\r') << endl;
	}
	for (string_mmap::iterator it = data->widelyUsed.begin(); it != data->widelyUsed.end(); ++it)
	{
		file << _T(" [") << (*it).first << _T("]") << _T('\r') << endl;
		file << (*it).second << _T('\r') << endl;
	}
	for (set<tstring>::iterator it = data->question.begin(); it != data->question.end(); ++it)
	{
		file << _T(" <QUESTION>") << _T('\r') << endl;
		file << (*it) << _T('\r') << endl;
	}
	for (set<tstring>::iterator it = data->special.begin(); it != data->special.end(); ++it)
	{
		file << _T(" <IGNORED>") << _T('\r') << endl;
		file << (*it) << _T('\r') << endl;
	}
	for (string_vec::iterator it = data->escape.begin(); it != data->escape.end(); ++it)
	{
		file << _T(" <ESCAPE>") << _T('\r') << endl;
		file << (*it) << _T('\r') << endl;
	}
	for (string_vec::iterator it = data->initial.begin(); it != data->initial.end(); ++it)
	{
		file << _T(" <INITIAL>") << _T('\r') << endl;
		file << (*it) << _T('\r') << endl;
	}
	for (string_vec::iterator it = data->failure.begin(); it != data->failure.end(); ++it)
	{
		file << _T(" <FAILURE>") << _T('\r') << endl;
		file << (*it) << _T('\r') << endl;
	}
	for (string_vec::iterator it = data->repeats.begin(); it != data->repeats.end(); ++it)
	{
		file << _T(" <REPEAT>") << _T('\r') << endl;
		file << (*it) << _T('\r') << endl;
	}
	for (map<tstring, vector<tstring>>::const_iterator it = data->raliases.begin(); it != data->raliases.end(); ++it)
	{
		tstring s;
		const vector<tstring>& v = (*it).second;
		bool first = true;
		for (vector<tstring>::const_iterator it1 = v.begin(); it1 != v.end(); ++it1)
		{
			if (first)
			{
				first = false;
				s = *it1;
			}
			else
			{
				s += _T(" ") + *it1;
			}
		}
		file << _T('@') << (*it).first << _T('\r') << endl;
		file << s << _T('\r') << endl;
	}
}

void Mind::LoadSmiles(tstring filename)
{
	basic_ifstream<TCHAR, char_traits<TCHAR> > file;
	file.open(filename.c_str());
	data->smiles.clear();
	tstring s;
	unsigned int l = 0;
	while (!file.eof())
	{
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
	TCHAR* buf = (TCHAR*)smiles;
	unsigned l = 0;
	TCHAR* end = buf + size;
	while (buf != end)
	{
		TCHAR *lend = buf;
		while (lend != end && *lend != _T('\r'))
			lend++;
		tstring s(buf, lend - buf);
		if ((unsigned)(lend - buf) > l)
			l = (int)s.length();
		data->smiles.insert(s);
		if (lend == end)
			break;
		buf = lend + 2;
	}
	data->maxSmileLen = l;
}
