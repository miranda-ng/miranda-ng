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

#include "Interface.h"
#include "EncryptionManager.h"
#include <tchar.h>

uint32_t CEncryptionManager::CipherListRefCount = 0;
CEncryptionManager::TCipherList* CEncryptionManager::CipherList = NULL;

static const uint32_t cFileBlockMask = 0xfffff000;

void CEncryptionManager::LoadCipherList()
{
	if (CipherList)
		return;

	CipherList = new TCipherList;
	CipherListRefCount++;

	WIN32_FIND_DATA search;
	TCHAR  path[MAX_PATH * 8];
	GetModuleFileName(NULL, path, sizeof(path));
	TCHAR * file = _tcsrchr(path, '\\');
	if (!file)
		file = path;

	_tcscpy_s(file, sizeof(path) / sizeof(path[0]) - (file - path), _T("\\plugins\\encryption\\*.dll"));
	file += 20;

	HANDLE hfinder = FindFirstFile(path, &search);
	if (hfinder != INVALID_HANDLE_VALUE)
	{
		TCipherItem item;
		TCipherInfo* (__cdecl *CipherInfoProc)(void *);
		do {
			_tcscpy_s(file, sizeof(path) / sizeof(path[0]) - (file - path), search.cFileName);
			HMODULE hmod = LoadLibrary(path);
			if (hmod)
			{
				CipherInfoProc = (TCipherInfo*(__cdecl*)(void*)) GetProcAddress(hmod, "CipherInfo");
				if (CipherInfoProc)
				{
					TCipherInfo* info = CipherInfoProc(NULL);
					if (info && (info->cbSize == sizeof(TCipherInfo)) && (CipherList->find(info->ID) == CipherList->end()))
					{
						item.ID          = info->ID;
						item.Name        = _wcsdup(info->Name);
						item.Description = _wcsdup(info->Description);
						item.FilePath    = _tcsdup(path);
						item.FileName    = item.FilePath + (file - path);
						
						CipherList->insert(std::make_pair(item.ID, item));
					} 
				}

				FreeLibrary(hmod);
				
			}
		} while (FindNextFile(hfinder, &search));

		FindClose(hfinder);
	}
}

CEncryptionManager::CEncryptionManager()
{
	m_Ciphers[CURRENT].Cipher = NULL;
	m_Ciphers[OLD].Cipher = NULL;
	m_Changing = false;
	m_ChangingProcess = 0;

	LoadCipherList();
}
CEncryptionManager::~CEncryptionManager()
{
	delete m_Ciphers[CURRENT].Cipher;
	m_Ciphers[CURRENT].Cipher = NULL;
	delete m_Ciphers[OLD].Cipher;
	m_Ciphers[OLD].Cipher = NULL;

	CipherListRefCount--;
	if (!CipherListRefCount)
	{
		TCipherList::iterator i = CipherList->begin();
		while (i != CipherList->end())
		{
			free(i->second.Description);
			free(i->second.Name);
			free(i->second.FilePath);
			// do not free Filename... it's a substring of FilePath
			++i;
		}

		delete CipherList;
		CipherList = NULL;
	}
}

bool CEncryptionManager::InitEncryption(TFileEncryption & Enc)
{
	if (Enc.ConversionProcess)
	{
		m_Changing = true;
		m_ChangingProcess = Enc.ConversionProcess;
	}

	for (int c = (int)CURRENT; c < (int)COUNT; c++)
	{
		TCipherList::iterator i = CipherList->find(Enc.CipherID);
		if (i != CipherList->end())
		{
			m_Ciphers[c].CipherDLL = LoadLibrary(i->second.FilePath);
			if (m_Ciphers[c].CipherDLL)
			{
				TCipherInfo* (__cdecl *cipherinfoproc)(void *);
				cipherinfoproc = (TCipherInfo*(__cdecl*)(void*)) GetProcAddress(m_Ciphers[c].CipherDLL, "CipherInfo");
				if (cipherinfoproc)
				{
					TCipherInfo* info = cipherinfoproc(NULL);
					if (info && (info->cbSize == sizeof(TCipherInfo)))
						m_Ciphers[c].Cipher = new CCipher(info->Create());

				}

				if (!m_Ciphers[c].Cipher)
				{
					FreeLibrary(m_Ciphers[c].CipherDLL);
					m_Ciphers[c].CipherDLL = NULL;
				}
			}
		}
	}

	return true;
}

bool CEncryptionManager::AlignData(uint32_t ID, uint32_t & Start, uint32_t & End)
{
	if (m_Ciphers[CURRENT].Cipher && (!m_Changing || (ID < m_ChangingProcess)))
	{
		if (m_Ciphers[CURRENT].Cipher->IsStreamCipher())
		{
			Start = 0;
			End = End - End % m_Ciphers[CURRENT].Cipher->BlockSizeBytes() + m_Ciphers[CURRENT].Cipher->BlockSizeBytes();
		} else {
			Start = Start - Start % m_Ciphers[CURRENT].Cipher->BlockSizeBytes();
			if (End % m_Ciphers[CURRENT].Cipher->BlockSizeBytes())
				End = End - End % m_Ciphers[CURRENT].Cipher->BlockSizeBytes() + m_Ciphers[CURRENT].Cipher->BlockSizeBytes();
		}

		return true;
	} else if (m_Ciphers[OLD].Cipher && m_Changing && (ID >= m_ChangingProcess))
	{
		if (m_Ciphers[OLD].Cipher->IsStreamCipher())
		{
			Start = 0;
			End = End - End % m_Ciphers[OLD].Cipher->BlockSizeBytes() + m_Ciphers[OLD].Cipher->BlockSizeBytes();
		} else {
			Start = Start - Start % m_Ciphers[OLD].Cipher->BlockSizeBytes();
			if (End % m_Ciphers[OLD].Cipher->BlockSizeBytes())
				End = End - End % m_Ciphers[OLD].Cipher->BlockSizeBytes() + m_Ciphers[OLD].Cipher->BlockSizeBytes();
		}

		return true;
	}

	return false;
}
uint32_t CEncryptionManager::AlignSize(uint32_t ID, uint32_t Size)
{
	if (m_Ciphers[CURRENT].Cipher && (!m_Changing || (ID < m_ChangingProcess)))
	{
		if (Size % m_Ciphers[CURRENT].Cipher->BlockSizeBytes())
			return Size - Size % m_Ciphers[CURRENT].Cipher->BlockSizeBytes() + m_Ciphers[CURRENT].Cipher->BlockSizeBytes();

	} else if (m_Ciphers[OLD].Cipher && m_Changing && (ID >= m_ChangingProcess))
	{
		if (Size % m_Ciphers[OLD].Cipher->BlockSizeBytes())
			return Size - Size % m_Ciphers[OLD].Cipher->BlockSizeBytes() + m_Ciphers[OLD].Cipher->BlockSizeBytes();

	}

	return Size;
}

bool CEncryptionManager::IsEncrypted(uint32_t ID)
{
	return (m_Ciphers[CURRENT].Cipher && (!m_Changing || (ID < m_ChangingProcess))) ||
	       (m_Ciphers[OLD].Cipher && m_Changing && (ID >= m_ChangingProcess));
}

void CEncryptionManager::Encrypt(void* Data, uint32_t DataLength, uint32_t ID, uint32_t StartByte)
{
	if (m_Ciphers[CURRENT].Cipher && (!m_Changing || (ID < m_ChangingProcess)))
	{
		m_Ciphers[CURRENT].Cipher->Encrypt(Data, DataLength, ID, StartByte);
	} else if (m_Ciphers[OLD].Cipher && m_Changing && (ID >= m_ChangingProcess))
	{
		m_Ciphers[OLD].Cipher->Encrypt(Data, DataLength, ID, StartByte);
	}
}
void CEncryptionManager::Decrypt(void* Data, uint32_t DataLength, uint32_t ID, uint32_t StartByte)
{
	if (m_Ciphers[CURRENT].Cipher && (!m_Changing || (ID < m_ChangingProcess)))
	{
		m_Ciphers[CURRENT].Cipher->Decrypt(Data, DataLength, ID, StartByte);
	} else if (m_Ciphers[OLD].Cipher && m_Changing && (ID >= m_ChangingProcess))
	{
		m_Ciphers[OLD].Cipher->Decrypt(Data, DataLength, ID, StartByte);
	}
}

bool CEncryptionManager::CanChangeCipher()
{
	return false;
}
bool CEncryptionManager::ChangeCipher(TEncryption & Encryption)
{
	return false;
}
