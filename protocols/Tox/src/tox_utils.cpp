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

std::vector<uint8_t> HexStringToData(const std::string hex)
{
	std::stringstream ss;
	std::vector<uint8_t> data;

	size_t count = hex.length() / 2;
	for (size_t i = 0; i < count; i++)
	{
		uint8_t temp;
		ss << std::hex << hex.substr(i * 2, 2);
		ss >> temp;
		data.push_back(temp);
	}

	return data;
}

std::string CToxProto::DataToHexString(const std::vector<uint8_t> data)
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

#define FRADDR_TOSTR_CHUNK_LEN 8 

void CToxProto::fraddr_to_str(uint8_t *id_bin, char *id_str)
{
	uint32_t i, delta = 0, pos_extra, sum_extra = 0;

	for (i = 0; i < TOX_FRIEND_ADDRESS_SIZE; i++) {
		sprintf(&id_str[2 * i + delta], "%02hhX", id_bin[i]);

		if ((i + 1) == TOX_CLIENT_ID_SIZE)
			pos_extra = 2 * (i + 1) + delta;

		if (i >= TOX_CLIENT_ID_SIZE)
			sum_extra |= id_bin[i];

		if (!((i + 1) % FRADDR_TOSTR_CHUNK_LEN)) {
			id_str[2 * (i + 1) + delta] = ' ';
			delta++;
		}
	}

	id_str[2 * i + delta] = 0;

	if (!sum_extra)
		id_str[pos_extra] = 0;
}

void CToxProto::get_id(Tox *m, char *data)
{
	int offset = strlen(data);
	uint8_t address[TOX_FRIEND_ADDRESS_SIZE];
	tox_get_address(m, address);
	fraddr_to_str(address, data + offset);
}
