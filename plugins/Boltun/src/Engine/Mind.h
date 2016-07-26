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

#ifndef MindH
#define MindH

#include <vector>
#include <map>
#include <set>
#include "WordsList.h"

class Mind;

typedef struct
{
	friend class Mind;
	std::vector<std::wstring> initial;
	std::set<std::wstring> question;
	std::set<std::wstring> special;
	std::vector<std::wstring> escape;
	std::vector<std::wstring> failure;
	std::vector<std::wstring> repeats;
	unsigned int maxSmileLen;
	std::set<std::wstring> smiles;
	std::multimap<WordsList, std::wstring> keywords;
	std::multimap<WordsList, std::wstring> qkeywords;
	std::multimap<std::wstring, std::wstring> widelyUsed;
	std::multimap<WordsList, std::wstring> specialEscapes;
	std::multimap<WordsList, std::wstring> qspecialEscapes;
	std::multimap<std::wstring, std::wstring> study;
	std::map<std::wstring, std::wstring> aliases;
	std::map<std::wstring, std::vector<std::wstring>> raliases;
private:
	int referenceCount;
} MindData;

class Mind
{
private:
	MindData *data;
	bool fileTypeMark;

	std::vector<std::wstring> Parse(std::wstring s);
public:
	Mind();
	~Mind();
	Mind(const Mind& mind);
	Mind& operator= (const Mind& mind);

	class CorruptedMind
	{
	public:
		int line;
		CorruptedMind(int aline)
			: line(aline)
		{
		};
	};

	const MindData *GetData() const;
	void Load(std::wstring filename);
	void Save(std::wstring filename) const;
	void LoadSmiles(std::wstring filename);
	void LoadSmiles(void* smiles, size_t size);
};

#endif
