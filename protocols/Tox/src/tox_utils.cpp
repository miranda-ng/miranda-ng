#include "common.h"

char *CToxProto::HexToBinString(const char *hex_string)
{
	size_t len = strlen(hex_string);
	char *val = (char*)mir_alloc(len);

	size_t i;

	for (i = 0; i < len; ++i, hex_string += 2)
		sscanf(hex_string, "%2hhx", &val[i]);

	return val;
}

char *CToxProto::BinToHexString(uint8_t *bin_string)
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