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
#include "IImport.h"

#define MAXSELECTSTR 8184

class ComparatorInterface
{
public:
	virtual bool Compare(const bool isMe, const std::wstring& message, wchar_t *strFind) = 0;
};

class HistoryEventList
{
public:
	struct EventData
	{
		bool isMe;
		uint16_t eventType;
		uint32_t timestamp;
	};
	struct EventIndex
	{
		union
		{
			MEVENT hEvent;
			int exIdx;
		};
		bool isExternal;
	};

private:
	std::map<int, bool> m_filterMap;
	bool m_onlyInFilter;
	bool m_onlyOutFilter;
	int  m_defFilter;
	std::wstring m_filterName;
	std::vector<IImport::ExternalMessage> m_importedMessages;
	uint32_t m_oldBlobSize;

	struct EventTempIndex
	{
		union
		{
			MEVENT hEvent;
			int exIdx;
		};
		bool isExternal;
		uint32_t timestamp;
	};

	struct ImportDiscData
	{
		IImport::ImportType type;
		std::wstring file;
	};

	static std::map<MCONTACT, ImportDiscData> m_contactFileMap;
	static std::wstring m_contactFileDir;

	bool CanShowHistory(DBEVENTINFO* dbei);
	bool CanShowHistory(const IImport::ExternalMessage& message);
	void InitFilters();
	void InitNames();
	void AddGroup(const EventIndex& ev);
	void GetTempList(std::list<EventTempIndex>& tempList, bool noFilter, bool noExt, MCONTACT _hContact);
	void ImportMessages(const std::vector<IImport::ExternalMessage>& messages);

protected:
	wchar_t m_contactName[256];
	wchar_t m_myName[256];
	bool  m_isWnd;
	bool  m_isFlat;
	int   m_deltaTime;
	uint32_t m_now;
	DBEVENTINFO m_dbei;
	
	virtual void AddGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText, int ico) = 0;
	bool GetEventIcon(bool isMe, int eventType, int &id);
	void DeleteEvent(const EventIndex& ev)
	{
		if (!ev.isExternal)
			db_event_delete(ev.hEvent);
	}

	void RebuildGroup(int selected);

public:
	HistoryEventList();
	HistoryEventList(MCONTACT _hContact, int filter);
	~HistoryEventList();
	
	HWND m_hWnd;
	MCONTACT m_hContact;
	std::vector<std::deque<EventIndex> > m_eventList;
	bool m_useImportedMessages;

	static void Init();
	void SetDefFilter(int filter);
	int GetFilterNr();
	std::wstring GetFilterName();
	void RefreshEventList();
	bool SearchInContact(MCONTACT hContact, wchar_t *strFind, ComparatorInterface* compFun);
	std::wstring GetContactName();
	std::wstring GetMyName();
	std::wstring GetProtocolName();
	std::wstring GetMyId();
	std::wstring GetContactId();
	std::string GetBaseProtocol();
	void MargeMessages(const std::vector<IImport::ExternalMessage>& messages);
	static void AddImporter(MCONTACT hContact, IImport::ImportType type, const std::wstring& file);
	static int GetContactMessageNumber(MCONTACT hContact);
	static bool IsImportedHistory(MCONTACT hContact);
	static void DeleteImporter(MCONTACT hContact);
	static void GetObjectDescription( DBEVENTINFO *dbei, wchar_t* str, int cbStr );
	bool GetEventData(const EventIndex& ev, EventData& data);
	void GetExtEventDBei(const EventIndex& ev);
	HICON GetEventCoreIcon(const EventIndex& ev);
	void GetEventMessage(const EventIndex& ev, wchar_t* message) // must be allocated with MAXSELECTSTR len
	{
		if (!ev.isExternal)
			GetObjectDescription(&m_dbei, message, MAXSELECTSTR);
		else
			wcscpy_s(message, MAXSELECTSTR, m_importedMessages[ev.exIdx].message.c_str());
	}
	void GetEventMessage(const EventIndex& ev, wchar_t* message, int strLen)
	{
		if (!ev.isExternal)
			GetObjectDescription(&m_dbei, message, strLen);
		else
		{
			std::wstring& meg = m_importedMessages[ev.exIdx].message;
			if ((int)meg.size() >= strLen)
			{
				memcpy_s(message, strLen * sizeof(wchar_t), meg.c_str(), (strLen - 1) * sizeof(wchar_t));
				message[strLen - 1] = 0;
			}
			else
			{
				wcscpy_s(message, strLen,  meg.c_str());
			}
		}
	}
};

