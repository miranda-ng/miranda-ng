#include "common.h"

TOX_USERSTATUS CToxProto::MirandaToToxStatus(int status)
{
	TOX_USERSTATUS userstatus = TOX_USERSTATUS_NONE;
	switch (status)
	{
	case ID_STATUS_AWAY:
		userstatus = TOX_USERSTATUS_AWAY;
		break;
	case ID_STATUS_OCCUPIED:
		userstatus = TOX_USERSTATUS_BUSY;
		break;
	}
	return userstatus;
}

int CToxProto::ToxToMirandaStatus(TOX_USERSTATUS userstatus)
{
	int status = ID_STATUS_OFFLINE;
	switch (userstatus)
	{
	case TOX_USERSTATUS_NONE:
		status = ID_STATUS_ONLINE;
		break;
	case TOX_USERSTATUS_AWAY:
		status = ID_STATUS_AWAY;
		break;
	case TOX_USERSTATUS_BUSY:
		status = ID_STATUS_OCCUPIED;
		break;
	}
	return status;
}

void CToxProto::ShowNotification(const TCHAR *caption, const TCHAR *message, int flags, MCONTACT hContact)
{
	if (Miranda_Terminated())
	{
		return;
	}

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Tox_main");

		if (!PUAddPopupT(&ppd))
			return;
	}

	MessageBox(NULL, message, caption, MB_OK | flags);
}

void CToxProto::ShowNotification(const TCHAR *message, int flags, MCONTACT hContact)
{
	ShowNotification(_T(MODULE), message, flags, hContact);
}

MEVENT CToxProto::AddDbEvent(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;

	return db_event_add(hContact, &dbei);
}

std::vector<uint8_t> CToxProto::HexStringToData(std::string hex)
{
	std::stringstream ss;
	std::vector<uint8_t> data;

	size_t count = hex.length() / 2;
	for (size_t i = 0; i < count; i++)
	{
		unsigned byte;
		std::istringstream hex_byte(hex.substr(i * 2, 2));
		hex_byte >> std::hex >> byte;
		data.push_back(static_cast<unsigned char>(byte));
	}

	return data;
}

std::string CToxProto::DataToHexString(std::vector<uint8_t> data)
{
	std::ostringstream oss;
	oss << std::hex << std::uppercase << std::setfill('0');
	for (size_t i = 0; i < data.size(); i++)
	{
		oss << std::setw(2) << static_cast<int>(data[i]);
	}
	return oss.str();
}

std::string  CToxProto::ToxAddressToId(std::string address)
{
	if (address.length() > TOX_CLIENT_ID_SIZE * 2)
	{
		address.erase(address.begin() + TOX_CLIENT_ID_SIZE * 2, address.end());
	}
	return address;
}

bool CToxProto::IsFileExists(std::tstring path)
{
	//return ::GetFileAttributes(fileName) != DWORD(-1)
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(path.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		FindClose(hFind);
		return true;
	}
	return false;
}