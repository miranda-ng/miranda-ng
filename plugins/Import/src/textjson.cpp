/*

Import plugin for Miranda NG

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

#include <m_json.h>

static int json_makeDatabase(const wchar_t*)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JSON text driver, read-only

static int CompareModules(const char *p1, const char *p2)
{
	return mir_strcmp(p1, p2);
}

class CDbxJson : public MDatabaseExport, public MZeroedObject
{
	JSONNode *m_root = nullptr;
	LIST<JSONNode> m_events;
	LIST<char> m_modules;
	bool m_bAppend = true;
	HANDLE m_out = INVALID_HANDLE_VALUE;
	ptrW m_wszFileName;
	CMStringA m_szId, m_szReplyId, m_szUserId;

public:
	CDbxJson(const wchar_t *pwszFileName = nullptr) :
		m_events(100),
		m_modules(10, CompareModules),
		m_wszFileName(mir_wstrdup(pwszFileName))
	{}

	~CDbxJson()
	{
		if (m_root != nullptr)
			json_delete(m_root);

		for (auto &it : m_modules)
			mir_free(it);
	}

	void Load()
	{
		// json operates with the only contact with pseudo id=1
		m_cache->AddContactToCache(1);
	}

	int Open(const wchar_t *profile)
	{
		HANDLE hFile = CreateFileW(profile, GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
			return EGROKPRF_CANTREAD;

		DWORD dwSize = GetFileSize(hFile, nullptr), dwRead;
		ptrA szFile((char *)mir_alloc(dwSize + 1));
		BOOL r = ReadFile(hFile, szFile, dwSize, &dwRead, nullptr);
		CloseHandle(hFile);
		if (!r)
			return EGROKPRF_CANTREAD;

		szFile[dwSize] = 0;
		if ((m_root = json_parse(szFile)) == nullptr)
			return EGROKPRF_DAMAGED;

		for (auto &it : m_root->at("history"))
			m_events.insert(&it);

		return EGROKPRF_NOERROR;
	}

	// mcontacts format always store history for one contact only
	STDMETHODIMP_(int) GetContactCount(void) override
	{
		return 1;
	}

	STDMETHODIMP_(int) GetEventCount(MCONTACT) override
	{
		return m_events.getCount();
	}

	STDMETHODIMP_(BOOL) GetEvent(MEVENT iEvent, DBEVENTINFO *dbei) override
	{
		JSONNode *pNode = m_events[iEvent - 1];
		if (pNode == nullptr)
			return 1;

		auto &node = *pNode;
		dbei->eventType = node["type"].as_int();

		dbei->timestamp = 0;
		std::string szTime = node["time"].as_string();
		if (!szTime.empty()) {
			char c;
			struct tm st = {};
			int res = sscanf(szTime.c_str(), "%4d%c%2d%c%2d %2d:%2d:%2d", &st.tm_year, &c, &st.tm_mon, &c, &st.tm_mday, &st.tm_hour, &st.tm_min, &st.tm_sec);
			if (res == 8) {
				st.tm_mon--;
				st.tm_year -= 1900;
				time_t tm = mktime(&st);
				if (tm != -1)
					dbei->timestamp = tm;
			}
		}
		else {
			szTime = node["isotime"].as_string();
			if (!szTime.empty()) {
				struct tm st = {};
				int res = sscanf(szTime.c_str(), "%4d-%2d-%2dT%2d:%2d:%2dZ", &st.tm_year, &st.tm_mon, &st.tm_mday, &st.tm_hour, &st.tm_min, &st.tm_sec);
				if (res == 6) {
					st.tm_mon--;
					st.tm_year -= 1900;
					time_t tm = _mkgmtime(&st);
					if (tm != -1)
						dbei->timestamp = tm;
				}
			}
		}

		if (dbei->timestamp == 0)
			dbei->timestamp = node["timeStamp"].as_int();

		dbei->flags = 0;
		std::string szFlags = node["flags"].as_string();
		for (auto &c : szFlags)
			switch (c) {
			case 'm': dbei->flags |= DBEF_SENT; break;
			case 'r': dbei->flags |= DBEF_READ; break;
			}

		std::string szModule = node["module"].as_string();
		if (!szModule.empty()) {
			dbei->szModule = m_modules.find((char *)szModule.c_str());
			if (dbei->szModule == nullptr) {
				dbei->szModule = mir_strdup(szModule.c_str());
				m_modules.insert((char *)dbei->szModule);
			}
		}

		m_szId = node["server_id"].as_mstring();
		dbei->szId = (m_szId.IsEmpty()) ? nullptr : m_szId.c_str();

		m_szReplyId = node["reply_id"].as_mstring();
		dbei->szReplyId = (m_szReplyId.IsEmpty()) ? nullptr : m_szReplyId.c_str();
			
		m_szUserId = node["reply_id"].as_mstring();
		dbei->szUserId = (!m_szUserId.IsEmpty()) ? nullptr : m_szUserId.c_str();

		if (dbei->eventType == EVENTTYPE_FILE) {
			dbei->flags |= DBEF_UTF;

			DB::FILE_BLOB blob(node["file"].as_mstring(), node["descr"].as_mstring());
			if (auto &url = node["url"])
				blob.setUrl(url.as_string().c_str());
			if (auto &file = node["localFile"])
				blob.setLocalName(file.as_mstring());
			if (auto &transferred = node["transferred"])
				blob.complete(transferred.as_int());
			if (auto &size = node["size"])
				blob.setSize(size.as_int());

			blob.write(*(DB::EventInfo *)dbei);
		}
		else {
			std::string szBody = node["body"].as_string();
			if (!szBody.empty()) {
				int offset;
				switch (dbei->eventType) {
				case EVENTTYPE_ADDED:
					offset = sizeof(uint32_t);
					break;

				case EVENTTYPE_AUTHREQUEST:
					offset = sizeof(uint32_t) * 2;
					break;

				default:
					offset = 0;
				}

				dbei->flags |= DBEF_UTF;
				dbei->cbBlob = (uint32_t)szBody.size() + offset;
				dbei->pBlob = (char *)mir_calloc(dbei->cbBlob + 1);
				memcpy(dbei->pBlob + offset, szBody.c_str(), szBody.size());
				dbei->pBlob[dbei->cbBlob] = 0;
			}
		}

		return 0;
	}

	STDMETHODIMP_(MEVENT) FindFirstEvent(MCONTACT) override
	{
		return 1;
	}

	STDMETHODIMP_(MEVENT) FindNextEvent(MCONTACT, MEVENT iEvent) override
	{
		if ((int)iEvent >= m_events.getCount())
			return 0;

		return iEvent + 1;
	}

	STDMETHODIMP_(MEVENT) FindLastEvent(MCONTACT) override
	{
		int numEvents = m_events.getCount();
		return numEvents ? numEvents - 1 : 0;
	}

	STDMETHODIMP_(MEVENT) FindPrevEvent(MCONTACT, MEVENT iEvent) override
	{
		if (iEvent <= 1)
			return 0;

		return iEvent - 1;
	}

	STDMETHODIMP_(DATABASELINK *) GetDriver();

	//////////////////////////////////////////////////////////////////////////////////////
	// Export interface

	int Create()
	{
		m_out = CreateFileW(m_wszFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (m_out == INVALID_HANDLE_VALUE)
			return EGROKPRF_CANTREAD;
		
		SetFilePointer(m_out, -3, 0, FILE_END);
		return EGROKPRF_NOERROR;
	}

	STDMETHODIMP_(int) BeginExport() override
	{
		if (m_out == INVALID_HANDLE_VALUE)
			Create();

		return GetFileSize(m_out, 0) != 0;
	}

	static int sttEnumSettings(const char *szSetting, void *param)
	{
		auto *pArray = (OBJLIST<char>*)param;
		pArray->insert(newStr(szSetting));
		return 1;
	}

	STDMETHODIMP_(int) ExportContact(MCONTACT hContact) override
	{
		char *szProto = Proto_GetBaseAccountName(hContact);
		ptrW id(Contact::GetInfo(CNF_UNIQUEID, hContact, szProto));
		ptrW nick(Contact::GetInfo(CNF_DISPLAY, hContact, szProto));

		JSONNode pRoot, pInfo, pHist(JSON_ARRAY);
		pInfo.set_name("info");
		if (szProto)
			pInfo.push_back(JSONNode("proto", szProto));

		OBJLIST<char> arSettings(50);
		db_enum_settings(hContact, &sttEnumSettings, szProto, &arSettings);

		for (auto &it : arSettings) {
			DBVARIANT dbv;
			if (db_get_s(hContact, szProto, it, &dbv, 0))
				continue;

			JSONNode S; S.set_name(it); S << INT_PARAM("type", dbv.type);

			switch (dbv.type) {
			case DBVT_BYTE: S << INT_PARAM("value", dbv.bVal); break;
			case DBVT_WORD: S << INT_PARAM("value", dbv.wVal); break;
			case DBVT_DWORD: S << INT_PARAM("value", dbv.dVal); break;
			case DBVT_ASCIIZ:
			case DBVT_UTF8: S << CHAR_PARAM("value", dbv.pszVal); break;
			case DBVT_WCHAR: S << WCHAR_PARAM("value", dbv.pwszVal); break;
			case DBVT_BLOB:
				{
					CMStringA buf;
					buf.Truncate(dbv.cchVal * 2);
					bin2hex(dbv.pbVal, dbv.cpbVal, buf.GetBuffer());
					S << CHAR_PARAM("value", buf);
				}
				break;

			default:
				continue;
			}
			pInfo << S;
		}

		pRoot.push_back(pInfo);

		pHist.set_name("history");
		pRoot.push_back(pHist);

		DWORD dwWritten;
		auto szOut = pRoot.write_formatted();
		WriteFile(m_out, szOut.c_str(), (DWORD)szOut.size(), &dwWritten, 0);

		m_bAppend = false;
		SetFilePointer(m_out, -3, 0, FILE_END);
		return 0;
	}

	STDMETHODIMP_(int) ExportEvent(const DB::EventInfo &dbei) override
	{
		if (m_bAppend) {
			SetFilePointer(m_out, -3, 0, FILE_END);

			DWORD dwWritten;
			WriteFile(m_out, ",", 1, &dwWritten, 0);
		}

		JSONNode pRoot;
		pRoot.push_back(JSONNode("type", dbei.eventType));

		char *szProto = Proto_GetBaseAccountName(dbei.hContact);
		if (mir_strcmp(dbei.szModule, szProto))
			pRoot.push_back(JSONNode("module", dbei.szModule));

		pRoot.push_back(JSONNode("timestamp", dbei.timestamp));

		wchar_t szTemp[500];
		TimeZone_PrintTimeStamp(UTC_TIME_HANDLE, dbei.timestamp, L"I", szTemp, _countof(szTemp), 0);
		pRoot.push_back(JSONNode("isotime", T2Utf(szTemp).get()));

		std::string flags;
		if (dbei.flags & DBEF_SENT)
			flags += "m";
		if (dbei.flags & DBEF_READ)
			flags += "r";
		pRoot.push_back(JSONNode("flags", flags));

		ptrW msg(dbei.getText());
		if (msg)
			pRoot.push_back(JSONNode("body", T2Utf(msg).get()));

		if (dbei.szId)
			pRoot.push_back(JSONNode("server_id", dbei.szId));
		if (dbei.szUserId)
			pRoot.push_back(JSONNode("user_id", dbei.szUserId));
		if (dbei.szReplyId)
			pRoot.push_back(JSONNode("reply_id", dbei.szReplyId));

		DWORD dwWritten;
		auto szOut = pRoot.write_formatted();
		WriteFile(m_out, szOut.c_str(), (DWORD)szOut.size(), &dwWritten, 0);
		WriteFile(m_out, "\n]}", 3, &dwWritten, 0);
		m_bAppend = true;
		return 0;
	}

	STDMETHODIMP_(int) EndExport() override
	{
		if (m_out)
			CloseHandle(m_out);
		return 0;
	}
};

static int json_grokHeader(const wchar_t *profile)
{
	return CDbxJson().Open(profile);
}

static MDatabaseCommon* json_load(const wchar_t *profile, BOOL)
{
	std::unique_ptr<CDbxJson> db(new CDbxJson());
	if (db->Open(profile))
		return nullptr;

	db->Load();
	return db.release();
}

static MDatabaseExport *json_export(const wchar_t *profile)
{
	auto *pDB = new CDbxJson(profile);
	pDB->Load();
	return pDB;
}

static DATABASELINK dblink =
{
	MDB_CAPS_EXPORT,
	"JSON",
	L"JSON text file driver",
	json_makeDatabase,
	json_grokHeader,
	json_load,
	json_export
};

STDMETHODIMP_(DATABASELINK *) CDbxJson::GetDriver()
{
	return &g_patternDbLink;
}

void RegisterJson()
{
	RegisterDatabasePlugin(&dblink);
}

void UnregisterJson()
{
	UnregisterDatabasePlugin(&dblink);
}
