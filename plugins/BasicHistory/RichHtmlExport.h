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
#include "IExport.h"
class RichHtmlExport :
	public IExport
{
private:
	int groupId;
	std::wstring folder;
	std::wstring folderName;
	std::string baseProto;
	stdext::hash_set<std::wstring> smileys;
	std::wstring ReplaceSmileys(bool isMe, const std::wstring &msg, bool &isUrl);
public:
	virtual const TCHAR* GetExt()
	{
		return _T("html");
	}
	
	virtual void WriteHeader(const std::wstring &fileName, const std::wstring &filterName, const std::wstring &myName, const std::wstring &myId, const std::wstring &name1, const std::wstring &proto1, const std::wstring &id1, const std::string& baseProto1, const std::wstring& encoding);
	virtual void WriteFooter();
	virtual void WriteGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText);
	virtual void WriteMessage(bool isMe, const std::wstring &longDate, const std::wstring &shortDate, const std::wstring &user, const std::wstring &message, const DBEVENTINFO& dbei);

	virtual ~RichHtmlExport();
};

