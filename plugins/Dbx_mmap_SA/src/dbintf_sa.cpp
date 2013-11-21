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

#include "commonheaders.h"

#define NeedBytes(n)   if(bytesRemaining<(n)) pBlob=(PBYTE)DBRead(ofsBlobPtr,(n),&bytesRemaining)
#define MoveAlong(n)   {int x=n; pBlob+=(x); ofsBlobPtr+=(x); bytesRemaining-=(x);}
#define VLT(n) ((n==DBVT_UTF8)?DBVT_ASCIIZ:n)

DBSignature dbSignatureSecured = {"Miranda ICQ SD",0x1A};
DBSignature dbSignatureNonSecured = {"Miranda ICQ SA",0x1A};

CDbxMmapSA::CDbxMmapSA(const TCHAR* tszFileName) :
	CDb3Mmap(tszFileName)
{
}

int CDbxMmapSA::Load(bool bSkipInit)
{
	if (CDb3Base::Load(bSkipInit) != ERROR_SUCCESS)
		return 1;

	if ( CheckDbHeaders())
		return 1;

	if (!bSkipInit) {
		TCHAR* p = _tcsrchr(m_tszProfileName, '\\');
		if (!p)
			return 1;

		if (m_bEncoding && !CheckPassword( LOWORD(m_dbHeader.version), p+1))
			return 1;

		InitDialogs();
	}

	return 0;
}

int CDbxMmapSA::CheckDbHeaders()
{
	if ( memcmp(m_dbHeader.signature, &dbSignatureSecured, sizeof(m_dbHeader.signature)) == 0)
		m_bEncoding = true;
	else if ( memcmp(m_dbHeader.signature, &dbSignatureNonSecured, sizeof(m_dbHeader.signature)) == 0)
		m_bEncoding = false;
	else {
		m_bEncoding = false;
		if ( memcmp(m_dbHeader.signature,&dbSignature,sizeof(m_dbHeader.signature)))
			return EGROKPRF_UNKHEADER;
		if ( LOWORD(m_dbHeader.version) != 0x0700)
			return EGROKPRF_VERNEWER;
	}
	if (m_dbHeader.ofsUser == 0)
		return EGROKPRF_DAMAGED;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static DWORD __inline GetSettingValueLength(PBYTE pSetting)
{
	if(pSetting[0]&DBVTF_VARIABLELENGTH) return 2+*(PWORD)(pSetting+1);
	return pSetting[0];
}

void CDbxMmapSA::EncodeContactSettings(HANDLE hContact)
{
	if (!hContact)
		hContact = (HANDLE)m_dbHeader.ofsUser;

	DBContact *contact = (DBContact *)DBRead((DWORD)hContact, sizeof(DBContact), NULL);
	if (contact->ofsFirstSettings) {
		DBContactSettings *setting = (struct DBContactSettings *)DBRead(contact->ofsFirstSettings, sizeof(struct DBContactSettings), NULL);
		DWORD offset = contact->ofsFirstSettings;
		while( true ) {
			DWORD ofsBlobPtr;
			PBYTE pBlob;
			int bytesRemaining;
			DWORD len;

			ofsBlobPtr = offset + offsetof(struct DBContactSettings,blob);
			pBlob = (PBYTE)DBRead(ofsBlobPtr,1,&bytesRemaining);
			while(pBlob[0]) {
				NeedBytes(1);
				NeedBytes(1+pBlob[0]);
				MoveAlong(1+pBlob[0]);

				NeedBytes(5);

				switch(pBlob[0]) {
				case DBVT_DELETED: break;
				case DBVT_BYTE: break;
				case DBVT_WORD:
					CryptoEngine->EncryptMem(pBlob+1, 2, key);
					break;

				case DBVT_DWORD:
					CryptoEngine->EncryptMem(pBlob+1, 4, key);
					break;

				case DBVT_UTF8:
				case DBVT_ASCIIZ:
				case DBVT_BLOB:
					NeedBytes(3+*(PWORD)(pBlob+1));
					len = *(PWORD)(pBlob+1);

					CryptoEngine->EncryptMem(pBlob+3, len, key);
					break;
				}
				NeedBytes(3);
				MoveAlong(1+GetSettingValueLength(pBlob));
				NeedBytes(1);
			}

			if (!setting->ofsNext)
				break;

			offset = setting->ofsNext;
			setting = (struct DBContactSettings *)DBRead(setting->ofsNext, sizeof(struct DBContactSettings), NULL);
		}
	}
}

void CDbxMmapSA::DecodeContactSettings(HANDLE hContact)
{
	if (!hContact)
		hContact = (HANDLE)m_dbHeader.ofsUser;

	DBContact *contact = (DBContact *)DBRead((DWORD)hContact, sizeof(DBContact), NULL);
	if (contact->ofsFirstSettings){
		DBContactSettings *setting = (struct DBContactSettings *)DBRead(contact->ofsFirstSettings, sizeof(struct DBContactSettings), NULL);
		DWORD offset = contact->ofsFirstSettings;
		while (true) {
			DWORD ofsBlobPtr;
			PBYTE pBlob;
			int bytesRemaining;
			DWORD len;
			ofsBlobPtr = offset + offsetof(struct DBContactSettings,blob);
			pBlob = (PBYTE)DBRead(ofsBlobPtr,1,&bytesRemaining);
			while(pBlob[0]) {
				NeedBytes(1);
				NeedBytes(1+pBlob[0]);
				//CopyMemory(szSetting,pBlob+1,pBlob[0]); szSetting[pBlob[0]] = 0;
				MoveAlong(1+pBlob[0]);

				NeedBytes(5);

				switch(pBlob[0]) {
				case DBVT_DELETED: break;
				case DBVT_BYTE: break;
				case DBVT_WORD:
					CryptoEngine->DecryptMem(pBlob+1, 2, key);
					break;

				case DBVT_DWORD:
					CryptoEngine->DecryptMem(pBlob+1, 4, key);
					break;

				case DBVT_UTF8:
				case DBVT_ASCIIZ:
				case DBVT_BLOB:
					NeedBytes(3+*(PWORD)(pBlob+1));
					len = *(PWORD)(pBlob+1);

					CryptoEngine->DecryptMem(pBlob+3, len, key);
					break;
				}
				NeedBytes(3);
				MoveAlong(1+GetSettingValueLength(pBlob));
				NeedBytes(1);
			}

			if (!setting->ofsNext)
				break;
			
			offset = setting->ofsNext;
			setting = (struct DBContactSettings *)DBRead(setting->ofsNext, sizeof(struct DBContactSettings), NULL);
		}
	}
}

void CDbxMmapSA::EncodeEvent(HANDLE hEvent)
{
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hEvent,sizeof(DBEvent),NULL);
	if (dbe->signature = DBEVENT_SIGNATURE)
		CryptoEngine->EncryptMem(DBRead((DWORD)hEvent + offsetof(DBEvent,blob), dbe->cbBlob, NULL), dbe->cbBlob, key);
}

void CDbxMmapSA::DecodeEvent(HANDLE hEvent)
{
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hEvent,sizeof(DBEvent),NULL);
	if (dbe->signature = DBEVENT_SIGNATURE)
		CryptoEngine->DecryptMem(DBRead((DWORD)hEvent + offsetof(DBEvent,blob), dbe->cbBlob, NULL), dbe->cbBlob, key);
}

void CDbxMmapSA::EncodeContactEvents(HANDLE hContact)
{
	HANDLE hEvent = FindFirstEvent(hContact);
	while (hEvent != 0) {
		EncodeEvent(hEvent);
		hEvent = FindNextEvent(hEvent);
	}
}

void CDbxMmapSA::DecodeContactEvents(HANDLE hContact)
{
	HANDLE hEvent = FindFirstEvent(hContact);
	while (hEvent != 0) {
		DecodeEvent(hEvent);
		hEvent = FindNextEvent(hEvent);
	}
}

int CDbxMmapSA::WorkInitialCheckHeaders(void)
{
	if (m_bEncoding) {
		cb->pfnAddLogMessage(STATUS_SUCCESS,TranslateT("Database is Secured MMAP database"));

		TCHAR* p = _tcsrchr(m_tszProfileName, '\\');
		if (!p)
			return ERROR_BAD_FORMAT;

		if (!CheckPassword( LOWORD(m_dbHeader.version), p+1)) {
			cb->pfnAddLogMessage(STATUS_FATAL,TranslateT("You are not authorized for access to Database"));
			return ERROR_BAD_FORMAT;
		}

		cb->pfnAddLogMessage(STATUS_SUCCESS,TranslateT("Secured MMAP: authorization successful"));
	}

	if ( LOWORD(m_dbHeader.version) != 0x0700 && !m_bEncoding) {
		cb->pfnAddLogMessage(STATUS_FATAL,TranslateT("Database is marked as belonging to an unknown version of Miranda"));
		return ERROR_BAD_FORMAT;
	}

	return ERROR_SUCCESS;
}
