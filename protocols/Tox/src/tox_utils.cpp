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

uint8_t *HexStringToData(const char *hex_string)
{
	size_t legth = strlen(hex_string) / 2;
	uint8_t *data = (uint8_t*)mir_alloc(legth);

	for (size_t i = 0; i < legth; i++)
	{
		unsigned int val;
		sscanf(&hex_string[i * 2], "%2hhx", &val);
		data[i] = val;
	}

	return data;
}

char *CToxProto::DataToHexString(const uint8_t *bin_string)
{
	uint32_t i, delta = 0, pos_extra, sum_extra = 0;
	char *ret = (char*)mir_alloc(TOX_FRIEND_ADDRESS_SIZE);

	for (i = 0; i < TOX_FRIEND_ADDRESS_SIZE; i++)
	{
		sprintf(&ret[2 * i + delta], "%02hhX", bin_string[i]);

		if ((i + 1) == TOX_CLIENT_ID_SIZE)
			pos_extra = 2 * (i + 1) + delta;

		if (i >= TOX_CLIENT_ID_SIZE)
			sum_extra |= bin_string[i];

		/*if (!((i + 1) % FRADDR_TOSTR_CHUNK_LEN)) {
			id_str[2 * (i + 1) + delta] = ' ';
			delta++;
			}*/
	}

	ret[2 * i + delta] = 0;

	if (!sum_extra)
		ret[pos_extra] = 0;

	return ret;
}

#define FRADDR_TOSTR_CHUNK_LEN 8 

static void fraddr_to_str(uint8_t *id_bin, char *id_str)
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

void get_id(Tox *m, char *data)
{
	int offset = strlen(data);
	uint8_t address[TOX_FRIEND_ADDRESS_SIZE];
	tox_get_address(m, address);
	fraddr_to_str(address, data + offset);
}
