#include "common.h"

TOX_USERSTATUS CToxProto::MirandaToToxStatus(int status)
{
	TOX_USERSTATUS userstatus;
	switch (status)
	{
	case ID_STATUS_ONLINE:
		userstatus = TOX_USERSTATUS_NONE;
		break;
	case ID_STATUS_AWAY:
		userstatus = TOX_USERSTATUS_AWAY;
		break;
	case ID_STATUS_OCCUPIED:
		userstatus = TOX_USERSTATUS_BUSY;
		break;
	default:
		userstatus = TOX_USERSTATUS_INVALID;
		break;
	}
	return userstatus;
}

int CToxProto::ToxToMirandaStatus(TOX_USERSTATUS userstatus)
{
	int status;
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
	default:
		status = ID_STATUS_OFFLINE;
		break;
	}
	return status;
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

void CToxProto::RaiseAuthRequestEvent(DWORD timestamp, const char* toxId, const char* reason)
{
	MCONTACT hContact = this->AddContact(toxId);

	/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), sid(ASCIIZ), reason(ASCIIZ)*/
	DWORD cbBlob = (DWORD)
		(sizeof(DWORD) * 2 +
		strlen(toxId) +
		strlen(reason) +
		5);

	PBYTE pBlob, pCurBlob;
	pCurBlob = pBlob = (PBYTE)mir_calloc(cbBlob);

	*((PDWORD)pCurBlob) = 0;
	pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact;
	pCurBlob += sizeof(DWORD);
	pCurBlob += 3;
	strcpy((char *)pCurBlob, toxId);
	pCurBlob += strlen(toxId) + 1;
	strcpy((char *)pCurBlob, reason);

	AddDbEvent(hContact, EVENTTYPE_AUTHREQUEST, timestamp, DBEF_UTF, cbBlob, pBlob);
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
	std::stringstream ss;
	ss << std::hex << std::uppercase;
	for (uint32_t i = 0; i < data.size(); i++)
	{
		ss << (int)data[i];
	}
	return ss.str();
}

int CToxProto::LoadToxData(const char *path)
{
	FILE *hFile = fopen(path, "rb");

	if (hFile)
	{
		fseek(hFile, 0, SEEK_END);
		size_t size = ftell(hFile);
		rewind(hFile);

		uint8_t *data = (uint8_t*)mir_alloc(size);

		if (fread(data, sizeof(uint8_t), size, hFile) != size)
		{
			mir_free(data);
			//fputs("[!] could not read data file!\n", stderr);
			fclose(hFile);
			return 0;
		}

		tox_load(tox, data, size);

		mir_free(data);

		if (fclose(hFile) < 0)
		{
			//perror("[!] fclose failed");
			/* we got it open and the expected data read... let it be ok */
			/* return 0; */
		}

		return 1;
	}

	return 0;
}

int CToxProto::SaveToxData(const char *path)
{
	FILE *hFile = fopen(path, "wb");

	if (!hFile)
	{
		//perror("[!] load_key");
		return 0;
	}

	int res = 1;
	uint32_t size = tox_size(tox);
	uint8_t *data = (uint8_t*)mir_alloc(size);

	tox_save(tox, data);

	if (fwrite(data, sizeof(uint8_t), size, hFile) != size)
	{
		mir_free(data);
		//fputs("[!] could not write data file (1)!", stderr);
		res = 0;
	}

	mir_free(data);

	if (fclose(hFile) < 0)
	{
		//perror("[!] could not write data file (2)");
		res = 0;
	}

	return res;
}
