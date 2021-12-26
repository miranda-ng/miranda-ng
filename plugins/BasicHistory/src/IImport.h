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

class IImport
{
protected:
	std::istream* stream;
public:
	enum ImportType
	{
		Binary,
		Dat
	};

	struct ExternalMessage
	{
		uint32_t timestamp;
		uint16_t eventType;
		uint16_t flags;
		std::wstring message;
	};

	void SetStream(std::istream *str)
	{
		stream = str;
	}

	virtual const wchar_t* GetExt() = 0;
	virtual int IsContactInFile(const std::vector<MCONTACT>& contacts) = 0;
	virtual bool GetEventList(std::vector<ExternalMessage>& eventList) = 0;

	virtual ~IImport()
	{
	}
};

