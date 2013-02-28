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

#ifndef UnRecentChooserH
#define UnRecentChooserH

class UnRecentChooser
{
	std::map<std::tstring, size_t> items;
	std::vector<std::tstring> itemsList;
	std::vector<std::tstring> newItems;
	float newItemsPrio, maxOldPrio;
	std::map<size_t, std::tstring> oldItems;
	std::map<std::tstring, float> oldPrios;
	size_t last;
	int minimum;
	static const size_t maxItems = 100;
public:
	UnRecentChooser();
	void AddChoice(std::tstring value, float prio = 1.0);
	std::tstring Choose();
	void SaveChoice(std::tstring choice);
};

#endif /* UnRecentChooserH */