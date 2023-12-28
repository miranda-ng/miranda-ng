/*

Import plugin for Miranda NG

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

static const char *pSettings[] =
{
	LPGEN("FirstName"),
	LPGEN("LastName"),
	LPGEN("e-mail"),
	LPGEN("Nick"),
	LPGEN("Age"),
	LPGEN("Gender"),
	LPGEN("City"),
	LPGEN("State"),
	LPGEN("Phone"),
	LPGEN("Homepage"),
	LPGEN("About")
};

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
	FILE *m_out = nullptr;
	bool m_bAppendOnly = false;

public:
	CDbxJson() :
		m_events(100),
		m_modules(10, CompareModules)
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
		HANDLE hFile = CreateFile(profile, GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
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

	int Create(const wchar_t *profile)
	{
		m_out = _wfopen(profile, L"wt");
		return (m_out == nullptr) ? EGROKPRF_CANTREAD : EGROKPRF_NOERROR;
	}

	STDMETHODIMP_(int) BeginExport() override
	{
		return 0;
	}

	STDMETHODIMP_(int) ExportContact(MCONTACT hContact) override
	{
		char *szProto = Proto_GetBaseAccountName(hContact);
		ptrW id(Contact::GetInfo(CNF_UNIQUEID, hContact, szProto));
		ptrW nick(Contact::GetInfo(CNF_DISPLAY, hContact, szProto));
		const char *uid = Proto_GetUniqueId(szProto);

		JSONNode pRoot, pInfo, pHist(JSON_ARRAY);
		pInfo.set_name("info");
		if (szProto)
			pInfo.push_back(JSONNode("proto", szProto));

		if (id != NULL)
			pInfo.push_back(JSONNode(uid, T2Utf(id).get()));

		for (auto &it : pSettings) {
			wchar_t *szValue = db_get_wsa(hContact, szProto, it);
			if (szValue)
				pInfo.push_back(JSONNode(it, T2Utf(szValue).get()));
			mir_free(szValue);
		}

		pRoot.push_back(pInfo);

		pHist.set_name("history");
		pRoot.push_back(pHist);

		fputs(pRoot.write_formatted().c_str(), m_out);
		fseek(m_out, -4, SEEK_CUR);
		return 0;
	}

	STDMETHODIMP_(int) ExportEvent(const DB::EventInfo &dbei) override
	{
		if (m_bAppendOnly) {
			fseek(m_out, -4, SEEK_END);
			fputs(",", m_out);
		}

		JSONNode pRoot2;
		pRoot2.push_back(JSONNode("type", dbei.eventType));

		char *szProto = Proto_GetBaseAccountName(dbei.hContact);
		if (mir_strcmp(dbei.szModule, szProto))
			pRoot2.push_back(JSONNode("module", dbei.szModule));

		pRoot2.push_back(JSONNode("timestamp", dbei.timestamp));

		wchar_t szTemp[500];
		TimeZone_PrintTimeStamp(UTC_TIME_HANDLE, dbei.timestamp, L"I", szTemp, _countof(szTemp), 0);
		pRoot2.push_back(JSONNode("isotime", T2Utf(szTemp).get()));

		std::string flags;
		if (dbei.flags & DBEF_SENT)
			flags += "m";
		if (dbei.flags & DBEF_READ)
			flags += "r";
		pRoot2.push_back(JSONNode("flags", flags));

		ptrW msg(DbEvent_GetTextW(&dbei));
		if (msg)
			pRoot2.push_back(JSONNode("body", T2Utf(msg).get()));

		fputs(pRoot2.write_formatted().c_str(), m_out);
		fputs("\n]}", m_out);

		m_bAppendOnly = true;
		return 0;
	}

	STDMETHODIMP_(int) EndExport() override
	{
		if (m_out)
			fclose(m_out);
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
	std::unique_ptr<CDbxJson> db(new CDbxJson());
	if	(db->Create(profile))
		return nullptr;

	db->Load();
	return db.release();
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
