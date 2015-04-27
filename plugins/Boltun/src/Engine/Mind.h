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
	std::vector<std::tstring> initial;
	std::set<std::tstring> question;
	std::set<std::tstring> special;
	std::vector<std::tstring> escape;
	std::vector<std::tstring> failure;
	std::vector<std::tstring> repeats;
	unsigned int maxSmileLen;
	std::set<std::tstring> smiles;
	std::multimap<WordsList, std::tstring> keywords;
	std::multimap<WordsList, std::tstring> qkeywords;
	std::multimap<std::tstring, std::tstring> widelyUsed;
	std::multimap<WordsList, std::tstring> specialEscapes;
	std::multimap<WordsList, std::tstring> qspecialEscapes;
	std::multimap<std::tstring, std::tstring> study;
	std::map<std::tstring, std::tstring> aliases;
	std::map<std::tstring, std::vector<std::tstring>> raliases;
private:
	int referenceCount;
} MindData;

class Mind
{
private:
	MindData *data;
	bool fileTypeMark;

	std::vector<std::tstring> Parse(std::tstring s);
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
	void Load(std::tstring filename);
	void Save(std::tstring filename) const;
	void LoadSmiles(std::tstring filename);
	void LoadSmiles(void* smiles, size_t size);
};

#endif
