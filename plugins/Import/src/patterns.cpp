/*

Import plugin for Miranda NG

Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org)

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

	std::auto_ptr<CImportPattern> pNew(new CImportPattern());
	pNew->wszName = buf;
	pNew->iType = GetPrivateProfileIntW(L"General", L"Type", 1, pwszFileName);

	if (GetPrivateProfileStringW(L"General", L"DefaultExtension", L"", buf, _countof(buf), pwszFileName))
		pNew->wszExt = buf;

	if (GetPrivateProfileStringW(L"General", L"Charset", L"", buf, _countof(buf), pwszFileName)) {
		if (!wcsicmp(buf, L"ANSI"))
			pNew->iCodePage = GetPrivateProfileIntW(L"General", L"Codepage", CP_ACP, pwszFileName);
		else if (!wcsicmp(buf, L"UCS2"))
			pNew->iCodePage = 1200;
	}
	else return;

	pNew->iUseHeader = GetPrivateProfileIntW(L"General", L"UseHeader", 0, pwszFileName);
	pNew->iUsePreMsg = GetPrivateProfileIntW(L"General", L"UsePreMsg", 0, pwszFileName);
	pNew->iUseFilename = GetPrivateProfileIntW(L"General", L"UseFileName", 0, pwszFileName);

	// [Message] section
	int erroffset;
	const char *err;
	if (GetPrivateProfileStringW(L"Message", L"Pattern", L"", buf, _countof(buf), pwszFileName)) {
		if ((pNew->regMessage.pattern = pcre16_compile(buf, 0, &err, &erroffset, nullptr)) == nullptr)
			return;
		pNew->regMessage.extra = pcre16_study(pNew->regMessage.pattern, 0, &err);
	}
	else return;

	if (GetPrivateProfileStringW(L"Message", L"In", L"", buf, _countof(buf), pwszFileName))
		pNew->wszIncoming = buf;
	if (GetPrivateProfileStringW(L"Message", L"Out", L"", buf, _countof(buf), pwszFileName))
		pNew->wszOutgoing = buf;

	pNew->iDirection = GetPrivateProfileIntW(L"General", L"Direction", 0, pwszFileName);
	pNew->iDay = GetPrivateProfileIntW(L"General", L"Day", 0, pwszFileName);
	pNew->iMonth = GetPrivateProfileIntW(L"General", L"Month", 0, pwszFileName);
	pNew->iYear = GetPrivateProfileIntW(L"General", L"Year", 0, pwszFileName);
	pNew->iHours = GetPrivateProfileIntW(L"General", L"Hours", 0, pwszFileName);
	pNew->iMinutes = GetPrivateProfileIntW(L"General", L"Minutes", 0, pwszFileName);
	pNew->iSeconds = GetPrivateProfileIntW(L"General", L"Seconds", 0, pwszFileName);

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
	FILE *m_hFile;

	std::vector<DWORD> m_events;
	std::vector<DWORD>::iterator m_curr;

public:
	CDbxPattern()
	{}

	~CDbxPattern()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
			::CloseHandle(m_hFile);
	}

	void Load()
	{
		wchar_t wszLine[2048];
		do {
			m_events.push_back(ftell(m_hFile));
		}
		while (fgetws(wszLine, _countof(wszLine), m_hFile));

		fseek(m_hFile, 0, SEEK_SET);
	}

	int Open(const wchar_t *profile)
	{
		m_hFile = _wfopen(profile, L"rb");
		return (m_hFile == nullptr) ? EGROKPRF_CANTREAD : EGROKPRF_NOERROR;
	}
	
	// mcontacts format always store history for one contact only
	STDMETHODIMP_(LONG) GetContactCount(void) override
	{
		return 1;
	}

	STDMETHODIMP_(LONG) GetEventCount(MCONTACT) override
	{
		return (LONG)m_events.size();
	}

	STDMETHODIMP_(BOOL) GetEvent(MEVENT dwOffset, DBEVENTINFO *dbei) override
	{
		fseek(m_hFile, dwOffset, SEEK_SET);

		wchar_t wszLine[2048];
		fgetws(wszLine, _countof(wszLine), m_hFile);
		return 1;
	}

	STDMETHODIMP_(MEVENT) FindFirstEvent(MCONTACT) override
	{
		m_curr = m_events.begin();
		return *m_curr;
	}

	STDMETHODIMP_(MEVENT) FindNextEvent(MCONTACT, MEVENT) override
	{
		if (m_curr == m_events.end())
			return 0;

		++m_curr;
		return *m_curr;
	}

	STDMETHODIMP_(MEVENT) FindLastEvent(MCONTACT) override
	{
		m_curr = m_events.end();
		return *m_curr;
	}

	STDMETHODIMP_(MEVENT) FindPrevEvent(MCONTACT, MEVENT) override
	{
		if (m_curr == m_events.begin())
			return 0;

		--m_curr;
		return *m_curr;
	}
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
	std::auto_ptr<CDbxPattern> db(new CDbxPattern());
	if (db->Open(profile))
		return nullptr;

	db->Load();
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
