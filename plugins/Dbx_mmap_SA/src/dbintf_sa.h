/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-13 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include <m_db_int.h>

#include "..\Db3x_mmap\src\dbintf.h"

struct CDbxMmapSA : public CDb3Base
{
	CDbxMmapSA(const TCHAR* tszFileName);

	int CheckPassword(WORD checkWord, TCHAR *szDBName);
	int CheckDbHeaders(void);

	void ChangePwd();
	void EncryptDB();
	void DecryptDB();
	void RecryptDB();

	int Load(bool bSkipInit);

	bool m_bEncoding, bEncProcess;

protected:
	virtual	void EncodeCopyMemory(void *dst, void *src, size_t size);
	virtual	void DecodeCopyMemory(void *dst, void *src, size_t size);
	virtual	void EncodeDBWrite(DWORD ofs, void *src, int size);
	virtual	void DecodeDBWrite(DWORD ofs, void *src, int size);

	virtual	int  WorkInitialCheckHeaders(void);

protected:
	int  CheckProto(HANDLE hContact, const char *proto);

	void EncoderInit();
	void EncodeContactEvents(HANDLE hContact);
	void EncodeEvent(HANDLE hEvent);
	void DecodeEvent(HANDLE hEvent);
	void DecodeContactEvents(HANDLE hContact);

	void DecodeContactSettings(HANDLE hContact);
	void EncodeContactSettings(HANDLE hContact);

	void WritePlainHeader();
	void WriteCryptHeader();

	void EncodeAll();
	void DecodeAll();

public:
	char  encryptKey[255];
	size_t encryptKeyLength;

private:
	void InitDialogs(void);
};
