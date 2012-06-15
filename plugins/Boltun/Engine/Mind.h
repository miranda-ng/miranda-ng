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

#include <string>
#include <vector>
#include <map>
#include <set>
#include "WordsList.h"
#include <tchar.h>
#include "tstring.h"


class Mind;

typedef struct
{
	friend class Mind;
	std::vector<tstring> initial;
	std::set<tstring> question;
	std::set<tstring> special;
	std::vector<tstring> escape;
	std::vector<tstring> failure;
	std::vector<tstring> repeats;
    unsigned int maxSmileLen;
	std::set<tstring> smiles;
	std::multimap<WordsList, tstring> keywords;
	std::multimap<WordsList, tstring> qkeywords;
	std::multimap<tstring, tstring> widelyUsed;
	std::multimap<WordsList, tstring> specialEscapes;
	std::multimap<WordsList, tstring> qspecialEscapes;
	std::multimap<tstring, tstring> study;
	std::map<tstring, tstring> aliases;
	std::map<tstring, std::vector<tstring>> raliases;
private:
	int referenceCount;
} MindData;

class Mind
{
private:
	MindData *data;
	bool fileTypeMark;	

	std::vector<tstring> Parse(tstring s);
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
	void Load(tstring filename);
	void Save(tstring filename) const;
	void LoadSmiles(tstring filename);
	void LoadSmiles(void* smiles, size_t size);
};

#endif
