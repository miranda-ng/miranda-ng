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

#pragma once

#include "Cipher.h"

class ARC4 : public CCipher
{
private:	
	uint8_t x;
	uint8_t y;
	uint8_t State[256];

	uint8_t Backx;
	uint8_t Backy;
	uint8_t BackState[256];

	uint8_t Stream();

  static const wchar_t* cName;
  static const wchar_t* cDescription;
	static const uint32_t cBlockSizeBytes = 1;
	static const bool     cIsStreamCipher = true;

public:
	const wchar_t * __cdecl Name();
	const wchar_t * __cdecl Description();
	const uint32_t  __cdecl BlockSizeBytes();
	const bool      __cdecl IsStreamCipher();

	static const TCipherInfo cCipherInfo;

	ARC4();
	~ARC4();
	static CCipher::TCipherInterface* __cdecl Create();

	void __cdecl SetKey(void* Key, uint32_t KeyLength);
	void __cdecl Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);
	void __cdecl Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);

};

const wchar_t* ARC4::cName          = L"ARC4";
const wchar_t* ARC4::cDescription   = L"Streamcipher - 8bit step, fast, Ron Rivest 1987";
const TCipherInfo ARC4::cCipherInfo = {sizeof(TCipherInfo), 'ARC4', cName, cDescription, &ARC4::Create};
