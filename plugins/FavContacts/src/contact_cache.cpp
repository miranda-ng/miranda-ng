#include "stdafx.h"

static int CompareItem(const CContactCache::TContactInfo *p1, const CContactCache::TContactInfo *p2)
{
	if (p1->rate > p2->rate) return -1;
	if (p1->rate < p2->rate) return 1;
	return 0;
}

CContactCache::CContactCache() :
	m_cache(50, CompareItem)
{
	int(__cdecl CContactCache::*pfn)(WPARAM, LPARAM);
	pfn = &CContactCache::OnDbEventAdded;
	HookEventObj(ME_DB_EVENT_ADDED, *(MIRANDAHOOKOBJ *)&pfn, this);

	Rebuild();
}

CContactCache::~CContactCache()
{
	for (auto &it : m_cache)
		delete it;
}

int __cdecl CContactCache::OnDbEventAdded(WPARAM hContact, LPARAM hEvent)
{
	DBEVENTINFO dbei = {};
	db_event_get(hEvent, &dbei);
	if (dbei.eventType != EVENTTYPE_MESSAGE)
		return 0;

	float weight = GetEventWeight(time(0) - dbei.timestamp);
	float q = GetTimeWeight(time(0) - m_lastUpdate);
	m_lastUpdate = time(0);
	if (!weight)
		return 0;

	TContactInfo *pFound = nullptr;
	mir_cslock lck(m_cs);

	for (auto &it : m_cache.rev_iter()) {
		it->rate *= q;
		if (it->hContact == hContact) {
			it->rate += weight;
			pFound = m_cache.removeItem(&it); // reinsert to maintain the sort order
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
	unsigned long timestamp = time(0);
	m_lastUpdate = time(0);

	for (auto &hContact : Contacts()) {
		TContactInfo *info = new TContactInfo;
		info->hContact = hContact;
		info->rate = 0;

		DB::ECPTR cursor(DB::EventsRev(hContact));
		while (MEVENT hEvent = cursor.FetchNext()) {
			DBEVENTINFO dbei = {};
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

static bool AppendInfo(wchar_t *buf, int size, MCONTACT hContact, int info)
{
	ptrW str(Contact::GetInfo(info, hContact));
	if (str != NULL) {
		mir_wstrncpy(buf, str, size);
		return true;
	}

	return false;
}

void CContactCache::TContactInfo::LoadInfo()
{
	if (infoLoaded) return;
	wchar_t *p = info;

	p[0] = p[1] = 0;

	static const int items[] = {
		CNF_FIRSTNAME, CNF_LASTNAME, CNF_NICK, CNF_CUSTOMNICK, CNF_EMAIL, CNF_CITY, CNF_STATE,
		CNF_COUNTRY, CNF_PHONE, CNF_HOMEPAGE, CNF_ABOUT, CNF_UNIQUEID, CNF_MYNOTES, CNF_STREET,
		CNF_CONAME, CNF_CODEPT, CNF_COCITY, CNF_COSTATE, CNF_COSTREET, CNF_COCOUNTRY
	};

	for (int i = 0; i < _countof(items); ++i)
		if (AppendInfo(p, _countof(info) - (p - info), hContact, items[i]))
			p += mir_wstrlen(p) + 1;

	*p = 0;

	infoLoaded = true;
}

wchar_t *nb_stristr(wchar_t *str, wchar_t *substr)
{
	if (!substr || !*substr) return str;
	if (!str || !*str) return nullptr;

	wchar_t *str_up = NEWWSTR_ALLOCA(str);
	wchar_t *substr_up = NEWWSTR_ALLOCA(substr);

	CharUpperBuff(str_up, (uint32_t)mir_wstrlen(str_up));
	CharUpperBuff(substr_up, (uint32_t)mir_wstrlen(substr_up));

	wchar_t *p = wcsstr(str_up, substr_up);
	return p ? (str + (p - str_up)) : nullptr;
}

bool CContactCache::filter(int rate, wchar_t *str)
{
	if (!str || !*str)
		return true;
	m_cache[rate]->LoadInfo();

	HKL kbdLayoutActive = GetKeyboardLayout(GetCurrentThreadId());
	HKL kbdLayouts[10];
	int nKbdLayouts = GetKeyboardLayoutList(_countof(kbdLayouts), kbdLayouts);

	wchar_t buf[256];
	uint8_t keyState[256] = { 0 };

	for (int iLayout = 0; iLayout < nKbdLayouts; ++iLayout) {
		if (kbdLayoutActive == kbdLayouts[iLayout])
			wcsncpy_s(buf, str, _TRUNCATE);
		else {
			int i;
			for (i = 0; str[i]; ++i) {
				UINT vk = VkKeyScanEx(str[i], kbdLayoutActive);
				UINT scan = MapVirtualKeyEx(vk, 0, kbdLayoutActive);
				ToUnicodeEx(vk, scan, keyState, buf + i, _countof(buf) - i, 0, kbdLayouts[iLayout]);
			}
			buf[i] = 0;
		}

		for (wchar_t *p = m_cache[rate]->info; p && *p; p = p + mir_wstrlen(p) + 1)
			if (nb_stristr(p, buf))
				return true;
	}

	return false;
}
