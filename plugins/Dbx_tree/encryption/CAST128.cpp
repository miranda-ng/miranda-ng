/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2008 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "CAST128.h"
#include "CAST128.inc"

const wchar_t * CAST128::Name()
{
	return cName;
}
const wchar_t * CAST128::Description()
{
	return cDescription;
}
const uint32_t  CAST128::BlockSizeBytes()
{
	return cBlockSizeBytes;
}
const bool      CAST128::IsStreamCipher()
{
	return cIsStreamCipher;
}

CAST128::CAST128()
{

}
CAST128::~CAST128()
{

}
CCipher::TCipherInterface* CAST128::Create() 
{
	return (new CAST128())->m_Interface;
}

void CAST128::SetKey(void* Key, uint32_t KeyLength)
{
	uint8_t k128[16] = "Mirandadbx_tree";
	int i = 0;
	uint8_t* k = (uint8_t*) Key;
	while (KeyLength > 0)
	{
		k128[i] = k128[i] ^ (*k);
		i = (i + 1) % 16;
		++k;
		--KeyLength;
	}
	CreateSubKeys(k128);
}
void CAST128::Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
{
	for (uint32_t i = 0; i <= Size - BlockSizeBytes(); i += BlockSizeBytes())
	{
		EncryptBlock((uint8_t*)Data + i);
		StartByte += BlockSizeBytes();
	}
}
void CAST128::Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
{
	for (uint32_t i = 0; i <= Size - BlockSizeBytes(); i += BlockSizeBytes())
	{
		DecryptBlock((uint8_t*)Data + i);
		StartByte += BlockSizeBytes();
	}
}

void CAST128::CreateSubKeys(uint8_t* Key)
{
	union {
		uint8_t z[16];
		uint32_t i[4];
	} t;

	uint32_t* k;
	k = (uint32_t*) Key;

	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Km[0x0] = S5[t.z[0x8]] ^ S6[t.z[0x9]] ^ S7[t.z[0x7]] ^ S8[t.z[0x6]] ^ S5[t.z[0x2]];
	Km[0x1] = S5[t.z[0xA]] ^ S6[t.z[0xB]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S6[t.z[0x6]];
	Km[0x2] = S5[t.z[0xC]] ^ S6[t.z[0xD]] ^ S7[t.z[0x3]] ^ S8[t.z[0x2]] ^ S7[t.z[0x9]];
	Km[0x3] = S5[t.z[0xE]] ^ S6[t.z[0xF]] ^ S7[t.z[0x1]] ^ S8[t.z[0x0]] ^ S8[t.z[0xC]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Km[0x4] = S5[Key[0x3]] ^ S6[Key[0x2]] ^ S7[Key[0xC]] ^ S8[Key[0xD]] ^ S5[Key[0x8]];
	Km[0x5] = S5[Key[0x1]] ^ S6[Key[0x0]] ^ S7[Key[0xE]] ^ S8[Key[0xF]] ^ S6[Key[0xD]];
	Km[0x6] = S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x8]] ^ S8[Key[0x9]] ^ S7[Key[0x3]];
	Km[0x7] = S5[Key[0x5]] ^ S6[Key[0x4]] ^ S7[Key[0xA]] ^ S8[Key[0xB]] ^ S8[Key[0x7]];

	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Km[0x8] = S5[t.z[0x3]] ^ S6[t.z[0x2]] ^ S7[t.z[0xC]] ^ S8[t.z[0xD]] ^ S5[t.z[0x9]];
	Km[0x9] = S5[t.z[0x1]] ^ S6[t.z[0x0]] ^ S7[t.z[0xE]] ^ S8[t.z[0xF]] ^ S6[t.z[0xC]];
	Km[0xa] = S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x8]] ^ S8[t.z[0x9]] ^ S7[t.z[0x2]];
	Km[0xb] = S5[t.z[0x5]] ^ S6[t.z[0x4]] ^ S7[t.z[0xA]] ^ S8[t.z[0xB]] ^ S8[t.z[0x6]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Km[0xc] = S5[Key[0x8]] ^ S6[Key[0x9]] ^ S7[Key[0x7]] ^ S8[Key[0x6]] ^ S5[Key[0x3]];
	Km[0xd] = S5[Key[0xA]] ^ S6[Key[0xB]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S6[Key[0x7]];
	Km[0xe] = S5[Key[0xC]] ^ S6[Key[0xD]] ^ S7[Key[0x3]] ^ S8[Key[0x2]] ^ S7[Key[0x8]];
	Km[0xf] = S5[Key[0xE]] ^ S6[Key[0xF]] ^ S7[Key[0x1]] ^ S8[Key[0x0]] ^ S8[Key[0xD]];



	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Kr[0x0] = S5[t.z[0x8]] ^ S6[t.z[0x9]] ^ S7[t.z[0x7]] ^ S8[t.z[0x6]] ^ S5[t.z[0x2]];
	Kr[0x1] = S5[t.z[0xA]] ^ S6[t.z[0xB]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S6[t.z[0x6]];
	Kr[0x2] = S5[t.z[0xC]] ^ S6[t.z[0xD]] ^ S7[t.z[0x3]] ^ S8[t.z[0x2]] ^ S7[t.z[0x9]];
	Kr[0x3] = S5[t.z[0xE]] ^ S6[t.z[0xF]] ^ S7[t.z[0x1]] ^ S8[t.z[0x0]] ^ S8[t.z[0xC]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Kr[0x4] = S5[Key[0x3]] ^ S6[Key[0x2]] ^ S7[Key[0xC]] ^ S8[Key[0xD]] ^ S5[Key[0x8]];
	Kr[0x5] = S5[Key[0x1]] ^ S6[Key[0x0]] ^ S7[Key[0xE]] ^ S8[Key[0xF]] ^ S6[Key[0xD]];
	Kr[0x6] = S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x8]] ^ S8[Key[0x9]] ^ S7[Key[0x3]];
	Kr[0x7] = S5[Key[0x5]] ^ S6[Key[0x4]] ^ S7[Key[0xA]] ^ S8[Key[0xB]] ^ S8[Key[0x7]];

	t.i[0] = k[0] ^ S5[Key[0xD]] ^ S6[Key[0xF]] ^ S7[Key[0xC]] ^ S8[Key[0xE]] ^ S7[Key[0x8]];
	t.i[1] = k[2] ^ S5[t.z[0x0]] ^ S6[t.z[0x2]] ^ S7[t.z[0x1]] ^ S8[t.z[0x3]] ^ S8[Key[0xA]];
	t.i[2] = k[3] ^ S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x5]] ^ S8[t.z[0x4]] ^ S5[Key[0x9]];
	t.i[3] = k[1] ^ S5[t.z[0xA]] ^ S6[t.z[0x9]] ^ S7[t.z[0xB]] ^ S8[t.z[0x8]] ^ S6[Key[0xB]];

	Kr[0x8] = S5[t.z[0x3]] ^ S6[t.z[0x2]] ^ S7[t.z[0xC]] ^ S8[t.z[0xD]] ^ S5[t.z[0x9]];
	Kr[0x9] = S5[t.z[0x1]] ^ S6[t.z[0x0]] ^ S7[t.z[0xE]] ^ S8[t.z[0xF]] ^ S6[t.z[0xC]];
	Kr[0xa] = S5[t.z[0x7]] ^ S6[t.z[0x6]] ^ S7[t.z[0x8]] ^ S8[t.z[0x9]] ^ S7[t.z[0x2]];
	Kr[0xb] = S5[t.z[0x5]] ^ S6[t.z[0x4]] ^ S7[t.z[0xA]] ^ S8[t.z[0xB]] ^ S8[t.z[0x6]];

	k[0] = t.i[2] ^ S5[t.z[0x5]] ^ S6[t.z[0x7]] ^ S7[t.z[0x4]] ^ S8[t.z[0x6]] ^ S7[t.z[0x0]];
	k[1] = t.i[0] ^ S5[Key[0x0]] ^ S6[Key[0x2]] ^ S7[Key[0x1]] ^ S8[Key[0x3]] ^ S8[t.z[0x2]];
	k[2] = t.i[1] ^ S5[Key[0x7]] ^ S6[Key[0x6]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S5[t.z[0x1]];
	k[3] = t.i[3] ^ S5[Key[0xA]] ^ S6[Key[0x9]] ^ S7[Key[0xB]] ^ S8[Key[0x8]] ^ S6[t.z[0x3]];

	Kr[0xc] = S5[Key[0x8]] ^ S6[Key[0x9]] ^ S7[Key[0x7]] ^ S8[Key[0x6]] ^ S5[Key[0x3]];
	Kr[0xd] = S5[Key[0xA]] ^ S6[Key[0xB]] ^ S7[Key[0x5]] ^ S8[Key[0x4]] ^ S6[Key[0x7]];
	Kr[0xe] = S5[Key[0xC]] ^ S6[Key[0xD]] ^ S7[Key[0x3]] ^ S8[Key[0x2]] ^ S7[Key[0x8]];
	Kr[0xf] = S5[Key[0xE]] ^ S6[Key[0xF]] ^ S7[Key[0x1]] ^ S8[Key[0x0]] ^ S8[Key[0xD]];


	for (int i = 0; i < 16; i++)
		Kr[i] = Kr[i] & 0x0000001F;
}


inline void CAST128::EncryptBlock(uint8_t *Block)
{
	uint32_t l, r, tmp;
	union {
		uint8_t byte[4];
		uint32_t block;
	} t;

	l = ((uint32_t*)Block)[0];
	r = ((uint32_t*)Block)[1];

	for (int i = 0; i < 16; i++)
	{
		if ((i % 3) == 0)
		{
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] ^ S2[t.byte[1]]) - S3[t.byte[2]]) + S4[t.byte[3]];
		} else if ((i % 3) == 1)
		{
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] - S2[t.byte[1]]) + S3[t.byte[2]]) ^ S4[t.byte[3]];
		} else {
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] + S2[t.byte[1]]) ^ S3[t.byte[2]]) - S4[t.byte[3]];
		}

		tmp = r;
		r = l ^ t.block;
		l = tmp;
	}

	((uint32_t*)Block)[0] = l;
	((uint32_t*)Block)[1] = r;
}



inline void CAST128::DecryptBlock(uint8_t *Block)
{
	uint32_t l, r, tmp;
	union {
		uint8_t byte[4];
		uint32_t block;
	} t;

	r = ((uint32_t*)Block)[0];
	l = ((uint32_t*)Block)[1];

	for (int i = 15; i >= 0; i--)
	{
		if ((i % 3) == 0)
		{
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] ^ S2[t.byte[1]]) - S3[t.byte[2]]) + S4[t.byte[3]];
		} else if ((i % 3) == 1)
		{
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] - S2[t.byte[1]]) + S3[t.byte[2]]) ^ S4[t.byte[3]];
		} else {
			t.block = (t.block << Kr[i]) | (t.block >> (32 - Kr[i]));
			t.block = ((S1[t.byte[0]] + S2[t.byte[1]]) ^ S3[t.byte[2]]) - S4[t.byte[3]];
		}

		tmp = r;
		r = l ^ t.block;
		l = tmp;
	}

	((uint32_t*)Block)[0] = r;
	((uint32_t*)Block)[1] = l;
}

extern "C" __declspec(dllexport) const TCipherInfo* CipherInfo(void * Reserved)
{
	return &CAST128::cCipherInfo;
}
