/*

Import plugin for Miranda NG

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

#include <memory>
#include <vector>

void CMPlugin::LoadPatterns()
{
	wchar_t wszPath[MAX_PATH], wszFullPath[MAX_PATH];
	GetModuleFileNameW(m_hInst, wszPath, _countof(wszPath));
	if (auto *p = wcsrchr(wszPath, '\\'))
		*p = 0;
	
	mir_snwprintf(wszFullPath, L"%s\\Import\\*.ini", wszPath);

	WIN32_FIND_DATAW fd;
	HANDLE hFind = FindFirstFileW(wszFullPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		mir_snwprintf(wszFullPath, L"%s\\Import\\%s", wszPath, fd.cFileName);
		LoadPattern(wszFullPath);
	}
	while (FindNextFileW(hFind, &fd) != 0);
}

void CMPlugin::LoadPattern(const wchar_t *pwszFileName)
{
	// [General] section
	wchar_t buf[1024];
	if (!GetPrivateProfileStringW(L"General", L"Name", L"", buf, _countof(buf), pwszFileName))
		return;

	std::unique_ptr<CImportPattern> pNew(new CImportPattern());
	pNew->wszName = buf;
	pNew->iType = GetPrivateProfileIntW(L"General", L"Type", 1, pwszFileName);

	if (GetPrivateProfileStringW(L"General", L"DefaultExtension", L"", buf, _countof(buf), pwszFileName))
		pNew->wszExt = buf;

	if (pNew->iType == 1) {
		if (GetPrivateProfileStringW(L"General", L"Charset", L"", buf, _countof(buf), pwszFileName)) {
			if (!wcsicmp(buf, L"ANSI"))
				pNew->iCodePage = GetPrivateProfileIntW(L"General", L"Codepage", CP_ACP, pwszFileName);
			else if (!wcsicmp(buf, L"UCS2"))
				pNew->iCodePage = 1200;
		}
		else return;
	}

	pNew->iUseHeader = GetPrivateProfileIntW(L"General", L"UseHeader", 0, pwszFileName);
	pNew->iUsePreMsg = GetPrivateProfileIntW(L"General", L"UsePreMsg", 0, pwszFileName);
	pNew->iUseFilename = GetPrivateProfileIntW(L"General", L"UseFileName", 0, pwszFileName);

	// [Message] section
	int erroffset;
	const char *err;
	if (pNew->iType == 1) {
		if (GetPrivateProfileStringW(L"Message", L"Pattern", L"", buf, _countof(buf), pwszFileName)) {
			if ((pNew->regMessage.pattern = pcre16_compile(buf, PCRE_MULTILINE, &err, &erroffset, nullptr)) == nullptr)
				return;
			pNew->regMessage.extra = pcre16_study(pNew->regMessage.pattern, 0, &err);
		}
		else return;

		if (GetPrivateProfileStringW(L"Message", L"In", L"", buf, _countof(buf), pwszFileName))
			pNew->wszIncoming = buf;
		if (GetPrivateProfileStringW(L"Message", L"Out", L"", buf, _countof(buf), pwszFileName))
			pNew->wszOutgoing = buf;

		pNew->iDirection = GetPrivateProfileIntW(L"Message", L"Direction", 0, pwszFileName);
		pNew->iDay = GetPrivateProfileIntW(L"Message", L"Day", 0, pwszFileName);
		pNew->iMonth = GetPrivateProfileIntW(L"Message", L"Month", 0, pwszFileName);
		pNew->iYear = GetPrivateProfileIntW(L"Message", L"Year", 0, pwszFileName);
		pNew->iHours = GetPrivateProfileIntW(L"Message", L"Hours", 0, pwszFileName);
		pNew->iMinutes = GetPrivateProfileIntW(L"Message", L"Minutes", 0, pwszFileName);
		pNew->iSeconds = GetPrivateProfileIntW(L"Message", L"Seconds", 0, pwszFileName);
	}

	if (pNew->iUseHeader) {
		if (GetPrivateProfileStringW(L"Header", L"Pattern", L"", buf, _countof(buf), pwszFileName)) {
			if ((pNew->regHeader.pattern = pcre16_compile(buf, PCRE_MULTILINE, &err, &erroffset, nullptr)) == nullptr)
				return;
			pNew->regHeader.extra = pcre16_study(pNew->regMessage.pattern, 0, &err);
		}
		else return;

		pNew->iHdrIncoming = GetPrivateProfileIntW(L"Header", L"In", 0, pwszFileName);
		pNew->iHdrOutgoing = GetPrivateProfileIntW(L"Header", L"Out", 0, pwszFileName);
		pNew->iHdrInNick = GetPrivateProfileIntW(L"Header", L"InNick", 0, pwszFileName);
		pNew->iHdrOutNick = GetPrivateProfileIntW(L"Header", L"OutNick", 0, pwszFileName);
		pNew->iHdrInUID = GetPrivateProfileIntW(L"Header", L"InUID", 0, pwszFileName);
		pNew->iHdrOutUID = GetPrivateProfileIntW(L"Header", L"OutUID", 0, pwszFileName);
	}

	if (pNew->iUsePreMsg) {
		pNew->preRN = GetPrivateProfileIntW(L"PreMessage", L"PreRN", -1, pwszFileName);
		pNew->preSP = GetPrivateProfileIntW(L"PreMessage", L"PreSP", 0, pwszFileName);
		pNew->afterRN = GetPrivateProfileIntW(L"PreMessage", L"AfterRN", -1, pwszFileName);
		pNew->afterSP = GetPrivateProfileIntW(L"PreMessage", L"AfterSP", 0, pwszFileName);
	}

	if (pNew->iUseFilename) {
		if (!GetPrivateProfileStringW(L"FileName", L"Pattern", L"", buf, _countof(buf), pwszFileName))
			return;
		if ((pNew->regFilename.pattern = pcre16_compile(buf, 0, &err, &erroffset, nullptr)) == nullptr)
			return;
		pNew->regFilename.extra = pcre16_study(pNew->regFilename.pattern, 0, &err);

		pNew->iInNick = GetPrivateProfileIntW(L"FileName", L"InNick", 0, pwszFileName);
		pNew->iInUID = GetPrivateProfileIntW(L"FileName", L"InUID", 0, pwszFileName);
		pNew->iOutNick = GetPrivateProfileIntW(L"FileName", L"OutNick", 0, pwszFileName);
		pNew->iOutUID = GetPrivateProfileIntW(L"FileName", L"OutUID", 0, pwszFileName);
	}

	m_patterns.insert(pNew.release());
}

/////////////////////////////////////////////////////////////////////////////////////////
// pattern-based database driver

class CDbxPattern : public MDatabaseReadonly, public MZeroedObject
{
	typedef MDatabaseReadonly CSuper;

	CMStringW m_buf, m_folder;
	MCONTACT m_hCurrContact = INVALID_CONTACT_ID;
	HANDLE m_hFile = INVALID_HANDLE_VALUE, m_hMap = 0;
	const uint8_t *m_pFile = 0;
	int m_iFileVersion = 0, m_iMsgHeaderSize = 0;

	std::vector<DWORD> m_events;
	std::vector<CMStringW> m_files;

	bool CheckContact(MCONTACT hContact)
	{
		if (hContact != m_hCurrContact) {
			m_hCurrContact = hContact;
			if (!Load(m_files[hContact - 1]))
				return false;
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// QHF file format

	bool LoadBinaryFile(const uint8_t *pFile, uint32_t iSize)
	{
		if (memicmp(pFile, "QHF", 3)) {
			AddMessage(LPGENW("Invalid file header"));
			return false;
		}

		m_iFileVersion = pFile[3];
							
		uint32_t fsz = RLInteger(&pFile[4]); pFile += 0x2C;
		uint32_t UIDLen = RLWord(pFile); pFile += 2;
		char *UIDStr = (char*)_alloca(UIDLen + 2);
		if (m_iFileVersion <= 2)
			strncpy_s(UIDStr, UIDLen + 2, (char *)pFile, UIDLen);
		else
			strncpy_s(UIDStr, UIDLen + 2, (char *)pFile, UIDLen + 1);
		pFile += UIDLen;

		uint32_t NickLen = RLWord(pFile); pFile += 2;
		char *NickStr = (char*)_alloca(NickLen + 2);
		if (m_iFileVersion <= 2)
			strncpy_s(NickStr, NickLen + 2, (char*)pFile, NickLen);
		else
			strncpy_s(NickStr, NickLen + 2, (char*)pFile, NickLen + 1);
		pFile += NickLen;

		DBCONTACTWRITESETTING dbcws = {};
		dbcws.szModule = "Pattern";
		dbcws.value.type = DBVT_UTF8;

		dbcws.szSetting = "ID";
		dbcws.value.pszVal = UIDStr;
		WriteContactSetting(m_hCurrContact, &dbcws);

		dbcws.szSetting = "Nick";
		dbcws.value.pszVal = NickStr;
		WriteContactSetting(m_hCurrContact, &dbcws);

		uint32_t iHeaderSize = 0x30 + NickLen + UIDLen;
		if (fsz != iSize - iHeaderSize)
			fsz = iSize - iHeaderSize;

		m_iMsgHeaderSize = (m_iFileVersion >= 3) ? 0x23 : 0x21;
		for (uint32_t i = 0; i < fsz; i += m_iMsgHeaderSize) {
			m_events.push_back(i + iHeaderSize);
			i += RLWord(&pFile[i + m_iMsgHeaderSize - 2]);
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Text file format, parsed by regexps

	bool LoadTextFile(const uint8_t *pFile)
	{
		auto *pPattern = g_pBatch->m_pPattern;

		switch (pPattern->iCodePage) {
		case CP_UTF8:
			m_buf = mir_utf8decodeW((char*)pFile);
			break;
		case 1200:
			m_buf = mir_wstrdup((wchar_t*)pFile);
			break;
		default:
			m_buf = mir_a2u_cp((char*)pFile, pPattern->iCodePage);
			break;
		}


		// smth went wrong or empty file
		if (m_buf.IsEmpty())
			return false;

		int iOffset = 0;
		if (m_buf[0] == 0xFEFF)
			m_buf.Delete(0);

		if (pPattern->iUseHeader) {
			int offsets[99];
			int nMatch = pcre16_exec(pPattern->regHeader.pattern, pPattern->regHeader.extra, m_buf, m_buf.GetLength(), iOffset, PCRE_NEWLINE_ANYCRLF, offsets, _countof(offsets));
			if (nMatch <= 0) {
				AddMessage(LPGENW("Cannot parse file header, skipping file"));
				return false;
			}

			const wchar_t **substrings;
			if (pcre16_get_substring_list(m_buf, offsets, nMatch, &substrings) >= 0) {
				if (pPattern->iUseHeader & 1) {
					pPattern->wszIncoming = substrings[pPattern->iHdrIncoming];
					pPattern->wszOutgoing = substrings[pPattern->iHdrOutgoing];
				}

				if (pPattern->iUseHeader & 2) {
					DBCONTACTWRITESETTING dbcws = {};
					dbcws.szModule = "Pattern";
					dbcws.value.type = DBVT_WCHAR;

					if (pPattern->iInUID && substrings[pPattern->iHdrInUID]) {
						dbcws.szSetting = "ID";
						dbcws.value.pwszVal = (wchar_t *)substrings[pPattern->iHdrInUID];
						WriteContactSetting(m_hCurrContact, &dbcws);
					}

					if (pPattern->iInNick && substrings[pPattern->iHdrInNick]) {
						dbcws.szSetting = "Nick";
						dbcws.value.pwszVal = (wchar_t *)substrings[pPattern->iHdrInNick];
						WriteContactSetting(m_hCurrContact, &dbcws);
					}
				}
			}

			iOffset = offsets[1];
		}

		while (true) {
			int offsets[99];
			int nMatch = pcre16_exec(pPattern->regMessage.pattern, pPattern->regMessage.extra, m_buf, m_buf.GetLength(), iOffset, PCRE_NEWLINE_ANYCRLF, offsets, _countof(offsets));
			if (nMatch <= 0)
				break;

			m_events.push_back(offsets[0]);
			iOffset = offsets[1];
		}
		return true;
	}

public:
	CDbxPattern()
	{}

	~CDbxPattern()
	{
		Close();
	}

	void Close()
	{
		if (m_pFile != nullptr)
			::UnmapViewOfFile(m_pFile);

		if (m_hMap != nullptr)
			::CloseHandle(m_hMap);

		if (m_hFile != INVALID_HANDLE_VALUE)
			::CloseHandle(m_hFile);
	}

	bool Load(const wchar_t *pwszFileName)
	{
		m_buf.Empty();
		m_events.clear();
		Close();

		AddMessage(LPGENW("Loading file '%s'..."), pwszFileName);

		m_hFile = ::CreateFileW(pwszFileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, 0);
		if (m_hFile == INVALID_HANDLE_VALUE) {
			AddMessage(LPGENW("Failed to open file <%s> for import: %d"), pwszFileName, GetLastError());
			return false;
		}

		uint32_t cbLen = ::GetFileSize(m_hFile, 0);
		m_hMap = ::CreateFileMappingW(m_hFile, nullptr, PAGE_READONLY, 0, 0, L"ImportMapfile");
		if (m_hMap == nullptr) {
			AddMessage(LPGENW("Failed to mmap file <%s> for import: %d"), pwszFileName, GetLastError());
			return false;
		}

		m_pFile = (const uint8_t*)::MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0);
		if (m_pFile == nullptr) {
			AddMessage(LPGENW("Failed to map view of file <%s> for import: %d"), pwszFileName, GetLastError());
			return false;
		}

		if (g_pBatch->m_pPattern->iType == 1) // text file
			return LoadTextFile(m_pFile);
		return LoadBinaryFile(m_pFile, cbLen);
	}

	int Open(const wchar_t *profile)
	{
		CMStringW wszBaseFolder(profile);
		auto *pPattern = g_pBatch->m_pPattern;

		// create a mask for loading multiple data files for a folder
		DWORD dwAttr = GetFileAttributesW(profile);
		if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) {
			wszBaseFolder = profile;
			m_folder.AppendFormat(L"%s\\*.%s", profile, pPattern->wszExt.c_str());
		}
		else {
			int i = wszBaseFolder.ReverseFind('\\');
			if (i != -1)
				wszBaseFolder = wszBaseFolder.Left(i);
			m_folder = profile;
		}

		int hContact = 1;
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(m_folder, &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				// find all subfolders except "." and ".."
				if (!mir_wstrcmp(fd.cFileName, L".") || !mir_wstrcmp(fd.cFileName, L".."))
					continue;

				CMStringW wszFullName(wszBaseFolder + L"\\" + fd.cFileName);
				m_files.push_back(wszFullName);

				auto *cc = m_cache->AddContactToCache(hContact);
				cc->szProto = "Pattern";

				// we try to restore user id from the file name
				if (pPattern->iUseFilename) {
					int offsets[100];
					int nMatch = pcre16_exec(pPattern->regFilename.pattern, pPattern->regFilename.extra, wszFullName, wszFullName.GetLength(), 0, 0, offsets, _countof(offsets));
					if (nMatch > 0) {
						const wchar_t **substrings;
						if (pcre16_get_substring_list(wszFullName, offsets, nMatch, &substrings) >= 0) {
							DBCONTACTWRITESETTING dbcws = {};
							dbcws.szModule = cc->szProto;
							dbcws.value.type = DBVT_WCHAR;

							if (pPattern->iInUID && substrings[pPattern->iInUID]) {
								dbcws.szSetting = "ID";
								dbcws.value.pwszVal = (wchar_t*)substrings[pPattern->iInUID];
								WriteContactSetting(hContact, &dbcws);
							}

							if (pPattern->iInNick && substrings[pPattern->iInNick]) {
								dbcws.szSetting = "Nick";
								dbcws.value.pwszVal = (wchar_t*)substrings[pPattern->iInNick];
								WriteContactSetting(hContact, &dbcws);
							}

							pcre16_free_substring_list(substrings);
						}
					}
				}			
				hContact++;
			}
				while (FindNextFile(hFind, &fd));

			FindClose(hFind);
		}

		if (m_files.empty())
			return EGROKPRF_CANTREAD;

		return EGROKPRF_NOERROR;
	}
	
	// patterns file always stores history for the single contact only
	STDMETHODIMP_(int) GetBlobSize(MEVENT idx) override
	{
		if (m_events.size() == 0 || idx < 1 || idx > m_events.size())
			return 0;

		if (g_pBatch->m_pPattern->iType == 1) {
			int iStart = m_events[idx-1], iEnd = (idx == m_events.size()) ? m_buf.GetLength() : m_events[idx];
			CMStringW msg = m_buf.Mid(iStart, iEnd - iStart);
			return (LONG)mir_strlen(ptrA(mir_utf8encodeW(msg))) + 1;
		}

		if (m_pFile == nullptr)
			return 0;

		const uint8_t *pMsg = m_pFile + m_events[idx-1];
		return RLWord(&pMsg[m_iMsgHeaderSize - 2]) + 1;
	}
	
	STDMETHODIMP_(int) GetContactCount(void) override
	{
		return (int)m_files.size();
	}

	STDMETHODIMP_(MCONTACT) FindFirstContact(const char *szProto) override
	{
		MCONTACT ret = CSuper::FindFirstContact(szProto);
		if (ret != 0)
			if (!CheckContact(ret))
				return 0;
		return ret;
	}
	
	STDMETHODIMP_(MCONTACT) FindNextContact(MCONTACT contactID, const char *szProto) override
	{
		MCONTACT ret = CSuper::FindNextContact(contactID, szProto);
		if (ret != 0)
			if (!CheckContact(ret))
				return 0;
		return ret;
	}

	STDMETHODIMP_(int) GetEventCount(MCONTACT) override
	{
		return (int)m_events.size();
	}

	STDMETHODIMP_(BOOL) GetContactSettingWorker(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT* dbv, int isStatic)
	{
		if (szSetting == nullptr || szModule == nullptr)
			return 1;

		DBCachedContact *cc = nullptr;
		if (hContact) {
			cc = m_cache->GetCachedContact(hContact);
			if (cc == nullptr)
				return 1;
		}

		size_t settingNameLen = strlen(szSetting);
		size_t moduleNameLen = strlen(szModule);
		char* szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

		DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(hContact, szCachedSettingName, 0);
		if (pCachedValue != nullptr) {
			if (pCachedValue->type == DBVT_ASCIIZ || pCachedValue->type == DBVT_UTF8) {
				int cbOrigLen = dbv->cchVal;
				char *cbOrigPtr = dbv->pszVal;
				memcpy(dbv, pCachedValue, sizeof(DBVARIANT));
				if (isStatic) {
					int cbLen = 0;
					if (pCachedValue->pszVal != nullptr)
						cbLen = (int)strlen(pCachedValue->pszVal);

					cbOrigLen--;
					dbv->pszVal = cbOrigPtr;
					if (cbLen < cbOrigLen)
						cbOrigLen = cbLen;
					memcpy(dbv->pszVal, pCachedValue->pszVal, cbOrigLen);
					dbv->pszVal[cbOrigLen] = 0;
					dbv->cchVal = cbLen;
				}
				else {
					dbv->pszVal = (char*)mir_alloc(strlen(pCachedValue->pszVal) + 1);
					strcpy(dbv->pszVal, pCachedValue->pszVal);
				}
			}
			else memcpy(dbv, pCachedValue, sizeof(DBVARIANT));

			return (pCachedValue->type == DBVT_DELETED) ? 1 : 0;
		}

		return 1;
	}

	STDMETHODIMP_(BOOL) WriteContactSetting(MCONTACT hContact, DBCONTACTWRITESETTING *dbcws)
	{
		if (dbcws == nullptr || dbcws->szSetting == nullptr || dbcws->szModule == nullptr)
			return 1;

		if (hContact) {
			DBCachedContact* cc = m_cache->GetCachedContact(hContact);
			if (cc == nullptr)
				return 1;
		}

		DBCONTACTWRITESETTING dbcwWork = *dbcws;
		if (dbcwWork.value.type == DBVT_WCHAR) {
			T2Utf value(dbcwWork.value.pwszVal);
			dbcwWork.value.pszVal = NEWSTR_ALLOCA(value);
			dbcwWork.value.type = DBVT_UTF8;
			dbcwWork.value.cchVal = (WORD)strlen(dbcwWork.value.pszVal);
		}

		char* cachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, mir_strlen(dbcwWork.szModule), mir_strlen(dbcwWork.szSetting));
		DBVARIANT* cachedValue = m_cache->GetCachedValuePtr(hContact, cachedSettingName, 1);
		if (cachedValue != nullptr)
			m_cache->SetCachedVariant(&dbcwWork.value, cachedValue);

		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////

	static int str2int(const wchar_t* str)
	{
		if (str == nullptr || *str == 0)
			return 0;

		return _wtoi(str);
	}

	int getBinaryEvent(MEVENT idx, DBEVENTINFO *dbei)
	{
		if (m_pFile == nullptr)
			return 1;

		const uint8_t *pMsg = m_pFile + m_events[idx-1];

		dbei->eventType = EVENTTYPE_MESSAGE;
		dbei->flags = DBEF_READ | DBEF_UTF;
		if (pMsg[0x1A] != 0)
			dbei->flags |= DBEF_SENT;
		dbei->timestamp = RLInteger(&pMsg[0x12]);
		dbei->timestamp -= TimeZone_ToLocal(dbei->timestamp) - dbei->timestamp; // deduct time zone offset from timestamp
		dbei->cbBlob = RLWord(&pMsg[m_iMsgHeaderSize - 2]);
		dbei->pBlob = (PBYTE)mir_alloc(dbei->cbBlob + 1);
		memcpy(dbei->pBlob, pMsg + m_iMsgHeaderSize, dbei->cbBlob);
		if (m_iFileVersion != 1)
			for (int i = 0; i < dbei->cbBlob; i++) {
				dbei->pBlob[i] += i+1;
				dbei->pBlob[i] = 255 - dbei->pBlob[i];
			}

		dbei->pBlob[dbei->cbBlob] = 0;
		return 0;
	}

	int getTextEvent(MEVENT idx, DBEVENTINFO *dbei)
	{
		auto *pPattern = g_pBatch->m_pPattern;

		int offsets[99];
		int nMatch = pcre16_exec(pPattern->regMessage.pattern, pPattern->regMessage.extra, m_buf, m_buf.GetLength(), m_events[idx-1], PCRE_NEWLINE_ANYCRLF, offsets, _countof(offsets));
		if (nMatch <= 0)
			return 1;

		dbei->eventType = EVENTTYPE_MESSAGE;
		dbei->flags = DBEF_READ | DBEF_UTF;

		int h1 = offsets[1], h2 = (idx == m_events.size()) ? m_buf.GetLength() : m_events[idx];
		int prn = -1, arn = -1;
		if (pPattern->iUsePreMsg)
			prn = pPattern->preRN, arn = pPattern->afterRN;

		if (prn != 0) {
			int i = 0;
			while (m_buf[h1] == '\r' && m_buf[h1 + 1] == '\n' && i < prn)
				h1 += 2, i++;
		}

		if (arn != 0) {
			int i = 0;
			while (m_buf[h2 - 2] == '\r' && m_buf[h2 - 1] == '\n' && i < arn)
				h2 -= 2, i++;
		}

		if (dbei->cbBlob) {
			CMStringW wszBody = m_buf.Mid(h1, h2 - h1).Trim();
			if (!wszBody.IsEmpty()) {
				ptrA tmp(mir_utf8encodeW(wszBody));
				int copySize = min(dbei->cbBlob - 1, (int)mir_strlen(tmp));
				memcpy(dbei->pBlob, tmp, copySize);
				dbei->pBlob[copySize] = 0;
				dbei->cbBlob = copySize;
			}
			else dbei->cbBlob = 0;
		}

		const wchar_t **substrings;
		if (pcre16_get_substring_list(m_buf, offsets, nMatch, &substrings) >= 0) {
			struct tm st = {};
			st.tm_year = str2int(substrings[pPattern->iYear]);
			if (st.tm_year > 1900)
				st.tm_year -= 1900;
			st.tm_mon = str2int(substrings[pPattern->iMonth]) - 1;
			st.tm_mday = str2int(substrings[pPattern->iDay]);
			st.tm_hour = str2int(substrings[pPattern->iHours]);
			st.tm_min = str2int(substrings[pPattern->iMinutes]);
			st.tm_sec = (pPattern->iSeconds) ? str2int(substrings[pPattern->iSeconds]) : 0;
			dbei->timestamp = mktime(&st);

			if (pPattern->iDirection)
				if (pPattern->wszOutgoing == substrings[pPattern->iDirection])
					dbei->flags |= DBEF_SENT;


			pcre16_free_substring_list(substrings);
		}
		return 0;
	}

	STDMETHODIMP_(BOOL) GetEvent(MEVENT idx, DBEVENTINFO *dbei) override
	{
		if (dbei == nullptr || m_events.size() == 0 || idx < 1 || idx > m_events.size())
			return 1;

		if (g_pBatch->m_pPattern->iType == 1)
			return getTextEvent(idx, dbei);

		return getBinaryEvent(idx, dbei);
	}

	STDMETHODIMP_(MEVENT) FindFirstEvent(MCONTACT hContact) override
	{
		// no system history
		if (hContact == 0)
			return 0;

		if (!CheckContact(hContact))
			return 0;

		return m_events.size() > 0 ? 1 : 0;
	}

	STDMETHODIMP_(MEVENT) FindNextEvent(MCONTACT, MEVENT idx) override
	{
		if (idx >= m_events.size())
			return 0;

		return idx + 1;
	}

	STDMETHODIMP_(MEVENT) FindLastEvent(MCONTACT hContact) override
	{
		// no system history
		if (hContact == 0)
			return 0;

		if (!CheckContact(hContact))
			return 0;

		return m_events.size() > 0 ? (MEVENT)m_events.size() : 0;
	}

	STDMETHODIMP_(MEVENT) FindPrevEvent(MCONTACT, MEVENT idx) override
	{
		return (idx >= 1) ? idx-1 : 0;
	}

	STDMETHODIMP_(DATABASELINK *) GetDriver();
};

/////////////////////////////////////////////////////////////////////////////////////////
// database link functions

static int pattern_makeDatabase(const wchar_t*)
{
	return 1;
}

static int pattern_grokHeader(const wchar_t *profile)
{
	return CDbxPattern().Open(profile);
}

static MDatabaseCommon* pattern_load(const wchar_t *profile, BOOL)
{
	std::unique_ptr<CDbxPattern> db(new CDbxPattern());
	if (db->Open(profile))
		return nullptr;

	return db.release();
}

DATABASELINK g_patternDbLink =
{
	0,
	"pattern",
	L"Pattern-based file driver",
	pattern_makeDatabase,
	pattern_grokHeader,
	pattern_load
};

STDMETHODIMP_(DATABASELINK *) CDbxPattern::GetDriver()
{
	return &g_patternDbLink;
}
