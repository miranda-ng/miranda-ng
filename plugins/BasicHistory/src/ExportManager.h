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
#include "IExport.h"
#include "IImport.h"
class ExportManager : public EventList
{
private:
	IExport* exp;
	std::wstring file;
	HWND hwnd;
	bool oldOnTop;
protected:
	virtual void AddGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText, int ico);
public:
	ExportManager(HWND _hwnd, MCONTACT _hContact, int filter);

	void SetAutoExport(const std::wstring _file, int _deltaTime, DWORD _now)
	{
		file = _file;
		deltaTime = _deltaTime;
		now = _now;
	}

	void SetAutoImport(const std::wstring _file)
	{
		file = _file;
	}

	std::wstring GetFileName()
	{
		return file;
	}

	bool Export(IExport::ExportType type);
	void SetDeleteWithoutExportEvents(int _deltaTime, DWORD _now);
	void DeleteExportedEvents();
	int Import(IImport::ImportType type, const std::vector<MCONTACT>& contacts);
	bool Import(IImport::ImportType type, std::vector<IImport::ExternalMessage>& eventList, std::wstring* err = NULL, bool* differentContact = NULL, std::vector<MCONTACT>* contacts = NULL);
	static const TCHAR* GetExt(IImport::ImportType type);
};

