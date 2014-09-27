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

void CToxProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
{
	if (Miranda_Terminated())
		return;

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAW ppd = { 0 };
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Tox_main");

		if (!PUAddPopupW(&ppd))
			return;
	}

	MessageBoxW(NULL, message, caption, MB_OK | flags);
}

void CToxProto::ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	ShowNotification(TranslateT(MODULE), message, flags, hContact);
}

HANDLE CToxProto::AddDbEvent(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
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

void CToxProto::LoadToxData()
{
	std::tstring toxProfilePath = GetToxProfilePath();
	FILE *hFile = _wfopen(toxProfilePath.c_str(), _T("rb"));
	if (!hFile)
	{
		debugLogA("CToxProto::LoadToxData: could not open tox profile");
		return;
	}

	fseek(hFile, 0, SEEK_END);
	uint32_t size = ftell(hFile);
	rewind(hFile);

	uint8_t *data = (uint8_t*)mir_alloc(size);
	if (fread(data, sizeof(uint8_t), size, hFile) != size)
	{
		debugLogA("CToxProto::LoadToxData: could not read tox profile");
		fclose(hFile);
		mir_free(data);
		return;
	}

	if (tox_is_data_encrypted(data))
	{
		ptrT password(getTStringA("Password"));
		char *password_utf8 = mir_utf8encodeW(password);
		if (tox_encrypted_load(tox, data, size, (uint8_t*)password_utf8, strlen(password_utf8)) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not decrypt tox profile");
		}
		mir_free(password_utf8);
	}
	else
	{
		if (tox_load(tox, data, size) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not load tox profile");
		}
	}

	mir_free(data);
	fclose(hFile);
}

void CToxProto::SaveToxData()
{
	std::tstring toxProfilePath = GetToxProfilePath();
	FILE *hFile = _wfopen(toxProfilePath.c_str(), _T("wb"));
	if (!hFile)
	{
		debugLogA("CToxProto::LoadToxData: could not open tox profile");
		return;
	}

	uint32_t size = tox_encrypted_size(tox);
	uint8_t *data = (uint8_t*)mir_alloc(size);
	ptrT password(getTStringA("Password"));
	if (password && _tcslen(password))
	{
		char *password_utf8 = mir_utf8encodeW(password);
		if (tox_encrypted_save(tox, data, (uint8_t*)password_utf8, strlen(password_utf8)) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not encrypt tox profile");
			mir_free(password_utf8);
			mir_free(data);
			fclose(hFile);
			return;
		}
		mir_free(password_utf8);
	}
	else
	{
		tox_save(tox, data);
	}

	if (fwrite(data, sizeof(uint8_t), size, hFile) != size)
	{
		debugLogA("CToxProto::LoadToxData: could not write tox profile");
	}

	mir_free(data);
	fclose(hFile);
}
