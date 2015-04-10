/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

bool CSkypeProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE && m_hPollingThread;
}

MEVENT CSkypeProto::AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
	return db_event_add(hContact, &dbei);
}

time_t CSkypeProto::IsoToUnixTime(const TCHAR *stamp)
{
	TCHAR date[9];
	int i, y;

	if (stamp == NULL)
		return 0;

	TCHAR *p = NEWTSTR_ALLOCA(stamp);

	// skip '-' chars
	int si = 0, sj = 0;
	while (true) {
		if (p[si] == _T('-'))
			si++;
		else if (!(p[sj++] = p[si++]))
			break;
	}

	// Get the date part
	for (i = 0; *p != '\0' && i < 8 && isdigit(*p); p++, i++)
		date[i] = *p;

	// Parse year
	if (i == 6) {
		// 2-digit year (1970-2069)
		y = (date[0] - '0') * 10 + (date[1] - '0');
		if (y < 70) y += 100;
	}
	else if (i == 8) {
		// 4-digit year
		y = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + date[3] - '0';
		y -= 1900;
	}
	else return 0;

	struct tm timestamp;
	timestamp.tm_year = y;

	// Parse month
	timestamp.tm_mon = (date[i - 4] - '0') * 10 + date[i - 3] - '0' - 1;

	// Parse date
	timestamp.tm_mday = (date[i - 2] - '0') * 10 + date[i - 1] - '0';

	// Skip any date/time delimiter
	for (; *p != '\0' && !isdigit(*p); p++);

	// Parse time
	if (_stscanf(p, _T("%d:%d:%d"), &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3)
		return (time_t)0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	time_t t = mktime(&timestamp);

	_tzset();
	t -= _timezone;
	return (t >= 0) ? t : 0;
}


bool CSkypeProto::IsMe(const char *skypeName)
{
	ptrA mySkypeName(getStringA(SKYPE_SETTINGS_ID));
	ptrA SelfEndpointName(getStringA("SelfEndpointName"));
	if (!mir_strcmp(skypeName, mySkypeName) || !mir_strcmp(skypeName, SelfEndpointName))
		return true;

	return false;
}

char *CSkypeProto::MirandaToSkypeStatus(int status)
{
	switch (status)
	{
	case ID_STATUS_AWAY:
		return "Away";

	case ID_STATUS_DND:
		return "Busy";

	case ID_STATUS_IDLE:
		return "Idle";

	case ID_STATUS_INVISIBLE:
		return "Hidden";
	}
	return "Online";
}

int CSkypeProto::SkypeToMirandaStatus(const char *status)
{
	if (!mir_strcmpi(status, "Online"))
		return ID_STATUS_ONLINE;	 
	else if (!mir_strcmpi(status, "Hidden"))
		return ID_STATUS_INVISIBLE;
	else if (!mir_strcmpi(status, "Away"))
		return ID_STATUS_AWAY;
	else if (!mir_strcmpi(status, "Idle"))
		return /*ID_STATUS_IDLE*/ID_STATUS_AWAY;
	else if (!mir_strcmpi(status, "Busy"))
		return ID_STATUS_DND;
	else 
		return ID_STATUS_OFFLINE;
}

void CSkypeProto::ShowNotification(const TCHAR *caption, const TCHAR *message, int flags, MCONTACT hContact)
{
	if (Miranda_Terminated())
		return;

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Skype_main");

		if (!PUAddPopupT(&ppd))
			return;
	}

	MessageBox(NULL, message, caption, MB_OK | flags);
}

void CSkypeProto::ShowNotification(const TCHAR *message, int flags, MCONTACT hContact)
{
	ShowNotification(_T(MODULE), message, flags, hContact);
}

bool CSkypeProto::IsFileExists(std::tstring path)
{
	return _taccess(path.c_str(), 0) == 0;
}

// url parsing

char *ParseUrl(const char *url, const char *token)
{
	const char *start = strstr(url, token);
	if (start == NULL)
		return NULL;
	start = start + mir_strlen(token);
	const char *end = strchr(start, '/');
	if (end == NULL)
		return mir_strdup(start);
	return mir_strndup(start, end - start);
}

char *CSkypeProto::ContactUrlToName(const char *url)
{
	return ParseUrl(url, "/8:");
}

char *CSkypeProto::SelfUrlToName(const char *url)
{
	return ParseUrl(url, "/1:");
}

char *CSkypeProto::ChatUrlToName(const char *url)
{
	return ParseUrl(url, "/19:");
}

char *CSkypeProto::GetServerFromUrl(const char *url)
{
	return ParseUrl(url, "://");
}