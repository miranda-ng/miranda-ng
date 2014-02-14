/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#include <stdint.h>
#include "sigslot.h"

#define __INTERFACE_ONLY__
#include "Cipher.h"
#undef __INTERFACE_ONLY__

#include "SHA256.h"
#include "Interface.h"
//#include "Thread.h"
#include <map>
#include <windows.h>

static const uint32_t cEncryptionChangingFlag = 0x80000000;

#pragma pack(push, 1)

typedef struct TFileEncryption {
	uint32_t CipherID;
	uint32_t CipherOldID;
	uint32_t ConversionProcess;
	uint8_t  SHA[32];
	uint8_t  SHAOld[32];
	uint32_t Reserved[2];
} TFileEncryption, *PFileEncryption;

#pragma pack(pop)

typedef struct TEncryption {
	uint32_t CipherID;
	wchar_t * Password;
} TEncryption, *PEncryption;

class CEncryptionManager
{
public:
	CEncryptionManager();
	~CEncryptionManager();

	typedef struct {
		TCHAR * FilePath;
		TCHAR * FileName;
		uint32_t ID;
		wchar_t * Name;
		wchar_t * Description;
	} TCipherItem;
	typedef std::map<uint32_t, TCipherItem> TCipherList;
	
	static TCipherList* CipherList;	// = NULL; see cpp
	static uint32_t CipherListRefCount;	// = 0; see cpp
	static void LoadCipherList();

	bool InitEncryption(TFileEncryption & Enc);

	bool AlignData(uint32_t ID, uint32_t & Start, uint32_t & End);
	uint32_t AlignSize(uint32_t ID, uint32_t Size);
	bool IsEncrypted(uint32_t ID);
	void Encrypt(void* Data, uint32_t DataLength, uint32_t ID, uint32_t StartByte);
	void Decrypt(void* Data, uint32_t DataLength, uint32_t ID, uint32_t StartByte);

	bool CanChangeCipher();
	bool ChangeCipher(TEncryption & Encryption);
private:
	bool m_Changing;
	uint32_t m_ChangingProcess;

	typedef enum TUsedCiphers {
		CURRENT = 0, 
		OLD = 1,
		COUNT = 2
	} TUsedCiphers;

	struct
	{
		CCipher * Cipher;
		HMODULE CipherDLL;
	} m_Ciphers[COUNT];

};
