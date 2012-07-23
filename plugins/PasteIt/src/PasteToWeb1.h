/*
Paste It plugin
Copyright (C) 2011 Krzysztof Kral

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
#include "pastetoweb.h"
class PasteToWeb1 :
	public PasteToWeb
{
protected:
	virtual void SendToServer(std::wstring str, std::wstring fileName, std::wstring format);
	static PasteFormat formats[];
	static PasteFormat defFormats[];
public:
	PasteToWeb1();
	virtual ~PasteToWeb1();
	virtual TCHAR* GetName()
	{
		return _T("pastebin.com");
	}
	virtual void ConfigureSettings();
	virtual std::list<PasteFormat> GetFormats();
	virtual std::list<PasteFormat> GetDefFormats();
	std::wstring GetUserKey(std::wstring& user, std::wstring& password);
};

