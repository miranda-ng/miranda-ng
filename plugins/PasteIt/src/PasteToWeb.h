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

struct PasteFormat
{
	std::wstring id;
	std::wstring name;
};

class PasteToWeb
{
protected:
	virtual void SendToServer(std::wstring str, std::wstring fileName, std::wstring format) = 0;
	wchar_t* SendToWeb(char* url, std::map<std::string, std::string>& headers, std::wstring content);
	PasteToWeb();
	TCHAR bufErr[1024];
public:
	virtual ~PasteToWeb();
	char szFileLink[256];
	TCHAR* error;
	void FromClipboard();
	void FromFile(std::wstring file);
	virtual TCHAR* GetName() = 0;
	virtual void ConfigureSettings() = 0;
	int pageIndex;
	virtual std::list<PasteFormat> GetFormats() = 0;
	virtual std::list<PasteFormat> GetDefFormats() = 0;
	static const int pages = 2;
};

