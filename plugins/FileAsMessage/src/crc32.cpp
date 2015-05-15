#include "main.h"

const ulong CRCPoly = 0xEDB88320;
ulong CRC32Table[256];

void InitCRC32()
{
	for (UINT32 i = 0; i < 256; i++)
	{
		UINT32 r = i;
		for (int j = 0; j < 8; j++)
			if (r & 1)
				r = (r >> 1) ^ CRCPoly;
			else
				r >>= 1;
		CRC32Table[i] = r;
	}
}

const ulong INITCRC = -1L;

inline ulong UpdateCRC32(uchar val, ulong crc)
{
	return CRC32Table[(uchar)crc^val] ^ (crc >> 8);
}

ulong memcrc32(uchar *ptr, int size, ulong crc)
{
	while (size--) crc = UpdateCRC32(*ptr++, crc);
	return crc;
}
