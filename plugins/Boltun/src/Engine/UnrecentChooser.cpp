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

UnRecentChooser::UnRecentChooser()
	:last(-1), minimum(-1), newItemsPrio(-1), maxOldPrio(-1)
{
}

void UnRecentChooser::AddChoice(tstring value, float prio)
{
	if (items.count(value) != 0)
	{
		int val = (int)items[value];
		oldItems.insert(make_pair(val, value));
		oldPrios.insert(make_pair(value, prio));
		if (minimum > val || minimum == -1)
			minimum = val;
		if (maxOldPrio < prio)
			maxOldPrio = prio;
	}
	else
	{
		if (prio > newItemsPrio)
		{
			newItems.push_back(value);
			newItemsPrio = prio;
		}
	}
}

tstring UnRecentChooser::Choose()
{
	tstring res;
	//Find answer
	if (newItemsPrio != -1)
	{
		int num = rand() % newItems.size();
		res = newItems[num];
	}
	else
		if (minimum == -1)
			res = _T("");
		else
		{
			float minprio = maxOldPrio / 1.5F;
			while (oldPrios[oldItems[minimum]] < minprio)
				minimum++;
			res = oldItems[minimum];
		}
	//Clean items
	minimum = -1;
	newItemsPrio = -1;
	maxOldPrio = -1;
	oldItems.clear();
	oldPrios.clear();
	newItems.clear();
	return res;
}

void UnRecentChooser::SaveChoice(tstring choice)
{
	//Add answer
	if (items.find(choice) != items.end())
	{
		for (vector<tstring>::iterator it = itemsList.begin(); it != itemsList.end(); ++it)
			if (*it == choice)
			{
				itemsList.erase(it);
				break;
			}
	}
	items[choice] = ++last;
	itemsList.push_back(choice);
	if (itemsList.size() > maxItems)
	{
		items.erase(*itemsList.begin());
		itemsList.erase(itemsList.begin());
	}
}