/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "EventList.h"

class SearchContext : public EventList
{
public:
	HWND editWindow;
	HWND findWindow;
	HWND toolbarWindow;
	HWND listWindow;
	int selected;

	virtual void SelectEventGroup(int sel) = 0;
	virtual MCONTACT GetNextContact(MCONTACT hContact, int adder) = 0;
	virtual void SelectContact(MCONTACT _hContact) = 0;

	struct MessageData
	{
		MessageData(const std::wstring& _description, int _startPos, int _endPos, bool _isMe, DWORD _timestamp)
			:description(_description)
		{
			startPos = _startPos;
			endPos = _endPos;
			isMe = _isMe;
			timestamp = _timestamp;
		}

		std::wstring description;
		int startPos;
		int endPos;
		bool isMe;
		DWORD timestamp;
	};

	std::vector<MessageData> currentGroup;
};

