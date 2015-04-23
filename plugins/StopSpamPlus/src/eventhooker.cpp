/* eventhooker.cpp
* Copyright (C) Miklashevsky Roman
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "headers.h"

namespace miranda
{
	namespace
	{
		std::list<EventHooker*> eventHookerList;
	}

	EventHooker::EventHooker(std::string name, MIRANDAHOOK fun) : name_(name), fun_(fun), handle_(0)
	{
		eventHookerList.push_back(this);
	}

	EventHooker::~EventHooker()
	{
		eventHookerList.remove(this);
	}

	void EventHooker::Hook()
	{
		handle_ = HookEvent(name_.c_str(), fun_);
	}

	void EventHooker::Unhook()
	{
		if (handle_)
		{
			UnhookEvent(handle_);
			handle_ = 0;
		}
	}

	void EventHooker::HookAll()
	{
		for (std::list<EventHooker*>::iterator it = eventHookerList.begin(); it != eventHookerList.end(); ++it)
		{
			(*it)->Hook();
		}
	}

	void EventHooker::UnhookAll()
	{
		for (std::list<EventHooker*>::iterator it = eventHookerList.begin(); it != eventHookerList.end(); ++it)
		{
			(*it)->Unhook();
		}
	}
}
