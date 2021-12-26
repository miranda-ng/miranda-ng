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
#include "DatExport.h"
#include "EventList.h"

#define EXP_FILE (*((std::ofstream*)IExport::stream))
#define IMP_FILE (*((std::ifstream*)IImport::stream))

std::wstring GetProtocolName(MCONTACT hContact);
std::wstring GetContactId(MCONTACT hContact);

#pragma pack(push, 1)

struct MCHeader
{
	unsigned char signature[2];
	unsigned int version;
	unsigned int dataSize;
};

#pragma pack(pop)

typedef struct
{
	int cbSize;       //size of the structure in bytes
	uint32_t szModule;	  //pointer to name of the module that 'owns' this
							 //event, ie the one that is in control of the data format
	uint32_t timestamp;  //seconds since 00:00, 01/01/1970. Gives us times until
							//2106 unless you use the standard C library which is
					  //signed and can only do until 2038. In GMT.
	uint32_t flags;	  //the omnipresent flags
	uint16_t eventType;	  //module-defined event type field
	uint32_t cbBlob;	  //size of pBlob in bytes
	uint32_t pBlob;	  //pointer to buffer containing module-defined event data
} DBEVENTINFO86;


DatExport::~DatExport()
{
}

int DatExport::WriteString(const std::wstring &str)
{
	int conv = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length() + 1, nullptr, 0, nullptr, nullptr);
	if (conv > (int)memBuf.size())
		memBuf.resize(conv);

	conv = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length() + 1, (char*)memBuf.c_str(), conv, nullptr, nullptr);
	return conv;
}

void DatExport::WriteHeader(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::string&, const std::wstring&)
{
	MCHeader header;
	memset(&header, 0, sizeof(MCHeader));
	memcpy(header.signature, "HB", 2);
	header.version = -1;
	header.dataSize = 0;
	dataSize = 0;
	EXP_FILE.write((char*)&header, sizeof(MCHeader));
}

void DatExport::WriteFooter()
{
	size_t pos = EXP_FILE.tellp();
	EXP_FILE.seekp(offsetof(MCHeader, dataSize), std::ios_base::beg);
	EXP_FILE.write((char*)&dataSize, sizeof(dataSize));
	EXP_FILE.seekp(pos, std::ios_base::beg);
	memBuf.resize(0);
	memBuf.shrink_to_fit();
}

void DatExport::WriteGroup(bool, const std::wstring&, const std::wstring&, const std::wstring&)
{
}

void DatExport::WriteMessage(bool, const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring &message, const DBEVENTINFO& dbei)
{
	DBEVENTINFO86 header;
	header.cbSize = sizeof(DBEVENTINFO86);
	header.eventType = dbei.eventType;
	header.flags = dbei.flags & (~(0x800));
	header.timestamp = dbei.timestamp;
	header.szModule = 0;
	header.pBlob = 0;
	if (dbei.flags & 0x800) {
		//Imported
		header.flags |= DBEF_UTF;
		header.cbBlob = WriteString(message);
		EXP_FILE.write((char*)&header, header.cbSize);
		EXP_FILE.write(memBuf.c_str(), header.cbBlob);
	}
	else {
		//Internal
		header.cbBlob = dbei.cbBlob;
		EXP_FILE.write((char*)&header, header.cbSize);
		EXP_FILE.write((char*)dbei.pBlob, header.cbBlob);
	}

	dataSize += header.cbSize + header.cbBlob;
}

bool ReadHeader(MCHeader& header, std::istream* stream)
{
	stream->read((char*)&header, sizeof(MCHeader));
	if (!stream->good())
		return false;

	if (memcmp(header.signature, "HB", 2) != 0)
		return false;

	return true;
}

int DatExport::IsContactInFile(const std::vector<MCONTACT>& contacts)
{
	MCHeader header;
	if (!ReadHeader(header, IImport::stream))
		return -2;

	if (contacts.size() == 1)
		hContact = contacts[0];

	IMP_FILE.seekg(0, std::ios_base::beg);
	return -3;
}

bool DatExport::GetEventList(std::vector<IImport::ExternalMessage>& eventList)
{
	MCHeader header;
	if (!ReadHeader(header, IImport::stream))
		return false;

	dataSize = header.dataSize;
	DBEVENTINFO86 messageHeader;
	DBEVENTINFO info = {};
	info.szModule = Proto_GetBaseAccountName(hContact);
	wchar_t _str[MAXSELECTSTR + 8]; // for safety reason
	std::multimap<uint32_t, IImport::ExternalMessage> sortedEvents;
	while (dataSize > 0) {
		messageHeader.cbSize = 0;
		IMP_FILE.read((char*)&messageHeader, sizeof(DBEVENTINFO86));
		if (!IMP_FILE.good())
			return false;

		if (messageHeader.cbSize < sizeof(DBEVENTINFO86))
			return false;

		if (messageHeader.cbSize > sizeof(DBEVENTINFO86))
			IMP_FILE.seekg(messageHeader.cbSize - sizeof(DBEVENTINFO86), std::ios_base::cur);

		IImport::ExternalMessage exMsg;
		exMsg.eventType = messageHeader.eventType;
		exMsg.flags = messageHeader.flags;
		exMsg.timestamp = messageHeader.timestamp;
		if (messageHeader.cbBlob > memBuf.size())
			memBuf.resize(messageHeader.cbBlob);

		IMP_FILE.read((char*)memBuf.c_str(), messageHeader.cbBlob);
		if (!IMP_FILE.good())
			return false;

		info.eventType = messageHeader.eventType;
		info.flags = messageHeader.flags;
		info.timestamp = messageHeader.timestamp;
		info.cbBlob = messageHeader.cbBlob;
		info.pBlob = (uint8_t*)memBuf.c_str();
		HistoryEventList::GetObjectDescription(&info, _str, MAXSELECTSTR);
		exMsg.message = _str;
		sortedEvents.insert(std::pair<uint32_t, IImport::ExternalMessage>(messageHeader.timestamp, exMsg));
		dataSize -= messageHeader.cbSize + messageHeader.cbBlob;
	}

	memBuf.resize(0);
	memBuf.shrink_to_fit();

	for (std::multimap<uint32_t, IImport::ExternalMessage>::iterator it = sortedEvents.begin(); it != sortedEvents.end(); ++it)
		eventList.push_back(it->second);

	return true;
}
