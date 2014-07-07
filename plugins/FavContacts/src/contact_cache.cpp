#include "headers.h"

static int CompareItem(const CContactCache::TContactInfo *p1, const CContactCache::TContactInfo *p2)
{
	if (p1->rate > p2->rate) return -1;
	if (p1->rate < p2->rate) return 1;
	return 0;
}

CContactCache::CContactCache() :
	m_cache(50, CompareItem)
{
	InitializeCriticalSection(&m_cs);

	int(__cdecl CContactCache::*pfn)(WPARAM, LPARAM);
	pfn = &CContactCache::OnDbEventAdded;
	HookEventObj(ME_DB_EVENT_ADDED, *(MIRANDAHOOKOBJ *)&pfn, this);

	Rebuild();
}

CContactCache::~CContactCache()
{
	for (int i = 0; i < m_cache.getCount(); i++)
		delete m_cache[i];

	DeleteCriticalSection(&m_cs);
}

int __cdecl CContactCache::OnDbEventAdded(WPARAM hContact, LPARAM lParam)
{
	HANDLE hEvent = (HANDLE)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get(hEvent, &dbei);
	if (dbei.eventType != EVENTTYPE_MESSAGE)
		return 0;

	float weight = GetEventWeight(time(NULL) - dbei.timestamp);
	float q = GetTimeWeight(time(NULL) - m_lastUpdate);
	m_lastUpdate = time(NULL);
	if (!weight)
		return 0;

	TContactInfo *pFound = NULL;
	mir_cslock lck(m_cs);
	for (int i = m_cache.getCount()-1; i >= 0; i--) {
		TContactInfo *p = m_cache[i];
		p->rate *= q;
		if (p->hContact == hContact) {
			p->rate += weight;
			pFound = p;
			m_cache.remove(i); // reinsert to maintain the sort order
		}
	}

	if (!pFound) {
		pFound = new TContactInfo;
		pFound->hContact = hContact;
		pFound->rate = weight;
	}
	m_cache.insert(pFound);
	return 0;
}

float CContactCache::GetEventWeight(unsigned long age)
{
	const float ceil = 1000.f;
	const float floor = 0.0001f;
	const int depth = 60 * 60 * 24 * 30;
	if (age > depth) return 0;
	return exp(log(ceil) - age * (log(ceil) - log(floor)) / depth);
}

float CContactCache::GetTimeWeight(unsigned long age)
{
	const float ceil = 1000.f;
	const float floor = 0.0001f;
	const int depth = 60 * 60 * 24 * 30;
	if (age > depth) return 0;
	return exp(age * (log(ceil) - log(floor)) / depth);
}

void CContactCache::Rebuild()
{
	unsigned long timestamp = time(NULL);
	m_lastUpdate = time(NULL);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		TContactInfo *info = new TContactInfo;
		info->hContact = hContact;
		info->rate = 0;

		for (HANDLE hEvent = db_event_last(hContact); hEvent; hEvent = db_event_prev(hContact, hEvent)) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			if (!db_event_get(hEvent, &dbei)) {
				if (float weight = GetEventWeight(timestamp - dbei.timestamp)) {
					if (dbei.eventType == EVENTTYPE_MESSAGE)
						info->rate += weight;
				}
				else break;
			}
		}

		m_cache.insert(info);
	}
}

MCONTACT CContactCache::get(int rate)
{
	if (rate >= 0 && rate < m_cache.getCount())
		return m_cache[rate]->hContact;
	return NULL;
}

float CContactCache::getWeight(int rate)
{
	if (rate >= 0 && rate < m_cache.getCount())
		return m_cache[rate]->rate;
	return -1;
}

static bool AppendInfo(TCHAR *buf, int size, MCONTACT hContact, int info)
{
	CONTACTINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	ci.dwFlag = info;
	ci.dwFlag |= CNF_UNICODE;

	bool ret = false;

	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci) && (ci.type == CNFT_ASCIIZ) && ci.pszVal) {
		if (*ci.pszVal && (lstrlen(ci.pszVal) < size - 2)) {
			lstrcpy(buf, ci.pszVal);
			ret = true;
		}
		mir_free(ci.pszVal);
	}

	return ret;
}

void CContactCache::TContactInfo::LoadInfo()
{
	if (infoLoaded) return;
	TCHAR *p = info;

	p[0] = p[1] = 0;

	static const int items[] = {
		CNF_FIRSTNAME, CNF_LASTNAME, CNF_NICK, CNF_CUSTOMNICK, CNF_EMAIL, CNF_CITY, CNF_STATE,
		CNF_COUNTRY, CNF_PHONE, CNF_HOMEPAGE, CNF_ABOUT, CNF_UNIQUEID, CNF_MYNOTES, CNF_STREET,
		CNF_CONAME, CNF_CODEPT, CNF_COCITY, CNF_COSTATE, CNF_COSTREET, CNF_COCOUNTRY
	};

	for (int i = 0; i < SIZEOF(items); ++i)
		if (AppendInfo(p, SIZEOF(info) - (p - info), hContact, items[i]))
			p += lstrlen(p) + 1;

	*p = 0;

	infoLoaded = true;
}

TCHAR *nb_stristr(TCHAR *str, TCHAR *substr)
{
	if (!substr || !*substr) return str;
	if (!str || !*str) return NULL;

	TCHAR *str_up = NEWTSTR_ALLOCA(str);
	TCHAR *substr_up = NEWTSTR_ALLOCA(substr);

	CharUpperBuff(str_up, lstrlen(str_up));
	CharUpperBuff(substr_up, lstrlen(substr_up));

	TCHAR *p = _tcsstr(str_up, substr_up);
	return p ? (str + (p - str_up)) : NULL;
}

bool CContactCache::filter(int rate, TCHAR *str)
{
	if (!str || !*str)
		return true;
	m_cache[rate]->LoadInfo();

	HKL kbdLayoutActive = GetKeyboardLayout(GetCurrentThreadId());
	HKL kbdLayouts[10];
	int nKbdLayouts = GetKeyboardLayoutList(SIZEOF(kbdLayouts), kbdLayouts);

	TCHAR buf[256];
	BYTE keyState[256] = { 0 };

	for (int iLayout = 0; iLayout < nKbdLayouts; ++iLayout) {
		if (kbdLayoutActive == kbdLayouts[iLayout])
			lstrcpy(buf, str);
		else {
			int i;
			for (i = 0; str[i]; ++i) {
				UINT vk = VkKeyScanEx(str[i], kbdLayoutActive);
				UINT scan = MapVirtualKeyEx(vk, 0, kbdLayoutActive);
				ToUnicodeEx(vk, scan, keyState, buf + i, SIZEOF(buf) - i, 0, kbdLayouts[iLayout]);
			}
			buf[i] = 0;
		}

		for (TCHAR *p = m_cache[rate]->info; p && *p; p = p + lstrlen(p) + 1)
			if (nb_stristr(p, buf))
				return true;
	}

	return false;
}
