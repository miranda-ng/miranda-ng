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

#include "StdAfx.h"
#include "TxtExport.h"
#define EXP_FILE (*stream)

TxtExport::~TxtExport()
{
}

void TxtExport::WriteHeader(const std::wstring&, const std::wstring &filterName, const std::wstring &myName, const std::wstring &myId, const std::wstring &name1, const std::wstring &proto1, const std::wstring &id1, const std::string&, const std::wstring&)
{
	wchar_t* start = TranslateT("###");
	EXP_FILE << start << "\n" << start << L" " << TranslateT("History Log") << L"\n";
	EXP_FILE << start << L" " << myName;
	if (proto1.length() || myId.length())
		EXP_FILE << L" (" << proto1 << L": " << myId << L") - ";
	else
		EXP_FILE << L" - ";

	EXP_FILE << name1;
	if (proto1.length() || id1.length())
		EXP_FILE << L" (" << proto1 << L": " << id1 << L")\n";
	else
		EXP_FILE << L"\n";

	EXP_FILE << start << L" " << TranslateT("Filter:") << L" " << filterName << L"\n" << start << L"\n";
}

void TxtExport::WriteFooter()
{
}

void TxtExport::WriteGroup(bool, const std::wstring&, const std::wstring&, const std::wstring&)
{
}

void TxtExport::WriteMessage(bool, const std::wstring &longDate, const std::wstring&, const std::wstring &user, const std::wstring &message, const DBEVENTINFO&)
{
	EXP_FILE << "\n[" << longDate << "] " << user << ":\n" << message << "\n";
}
