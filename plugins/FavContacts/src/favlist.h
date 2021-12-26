#ifndef favlist_h__
#define favlist_h__

struct TContactInfo
{
private:
	MCONTACT hContact;
	uint32_t status;
	wchar_t *name;
	wchar_t *group;
	bool bManual;
	float fRate;

public:
	TContactInfo(MCONTACT _hContact, bool _bManual, float _fRate = 0) :
		hContact(_hContact),
		bManual(_bManual),
		fRate(_fRate)
	{
		name = mir_wstrdup(Clist_GetContactDisplayName(hContact));

		if (g_Options.bUseGroups)
			group = db_get_wsa(hContact, "CList", "Group", TranslateT("<no group>"));
		else
			group = mir_wstrdup(TranslateT("Favorite Contacts"));

		status = db_get_w(hContact, Proto_GetBaseAccountName(hContact), "Status", ID_STATUS_OFFLINE);
	}

	~TContactInfo()
	{
		mir_free(name);
		mir_free(group);
	}

	MCONTACT getHandle() const
	{
		return hContact;
	}

	wchar_t *getGroup() const
	{
		return group;
	}

	static int cmp(const TContactInfo *p1, const TContactInfo *p2)
	{
		if (p1->bManual && !p2->bManual) return -1;
		if (!p1->bManual && p2->bManual) return 1;

		if (!p1->bManual)
		{
			if (p1->fRate > p2->fRate) return -1;
			if (p1->fRate < p2->fRate) return 1;
		}

		int res = 0;
		if (res = mir_wstrcmp(p1->group, p2->group)) return res;
		if (res = mir_wstrcmp(p1->name, p2->name)) return res;
		return 0;
	}
};

class TFavContacts : public LIST < TContactInfo >
{
private:
	int nGroups;
	wchar_t *prevGroup;
	MDatabaseCommon *db;

	int addContact(MCONTACT hContact, bool bManual)
	{
		DBCachedContact *cc = db->getCache()->GetCachedContact(hContact);
		if (cc == nullptr)
			return 0;

		if (db_mc_isEnabled()) {
			if (cc->IsSub()) // skip subcontacts if MC is enabled
				return 0;
		}
		else if (cc->IsMeta()) // skip metacontacts if MC is not enabled
			return 0;

		TContactInfo *info = new TContactInfo(hContact, bManual);
		insert(info);
		wchar_t *group = info->getGroup();
		if (prevGroup && mir_wstrcmp(prevGroup, group))
			++nGroups;
		prevGroup = group;
		return 1;
	}

public:
	TFavContacts() : LIST<TContactInfo>(5, TContactInfo::cmp)
	{
		db = db_get_current();
	}

	~TFavContacts()
	{
		for (auto &it : *this)
			delete it;
	}

	__forceinline int groupCount() const { return nGroups; }

	void build()
	{
		prevGroup = nullptr;

		nGroups = 1;

		for (auto &hContact : Contacts())
			if (g_plugin.getByte(hContact, "IsFavourite", 0))
				addContact(hContact, true);

		int nRecent = 0;
		for (int i = 0; nRecent < g_Options.wMaxRecent; ++i) {
			MCONTACT hContact = g_contactCache->get(i);
			if (!hContact)
				break;
			
			if (!g_plugin.getByte(hContact, "IsFavourite", 0))
				nRecent += addContact(hContact, false);
		}
	}
};

#endif // favlist_h__
