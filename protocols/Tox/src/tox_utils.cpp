#include "common.h"

uint8_t *CToxProto::HexToBinString(char *hex_string)
{
	// byte is represented by exactly 2 hex digits, so lenth of binary string
	// is half of that of the hex one. only hex string with even length valid.
	// the more proper implementation would be to check if strlen(hex_string)
	// is odd and return error code if it is. we assume strlen is even.
	// if it's not then the last byte just won't be written in 'ret'.
	int length = strlen(hex_string) / 2;
	uint8_t *ret = (uint8_t*)mir_alloc(length);
	char *pos = hex_string;

	for (int i = 0; i < length; i++, pos += 2)
	{
		//sscanf(pos, "%2hhx", &ret[i]);
		uint8_t byteval;
		sscanf(pos, "%2hhx", &byteval);
		ret[i] = byteval;
	}

	return ret;
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