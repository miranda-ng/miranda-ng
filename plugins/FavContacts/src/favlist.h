#ifndef favlist_h__
#define favlist_h__

struct TContactInfo
{
private:
	MCONTACT hContact;
	DWORD status;
	TCHAR *name;
	TCHAR *group;
	bool bManual;
	float fRate;

public:
	TContactInfo(MCONTACT _hContact, bool _bManual, float _fRate = 0) :
		hContact(_hContact),
		bManual(_bManual),
		fRate(_fRate)
	{
		name = mir_tstrdup(pcli->pfnGetContactDisplayName(hContact, 0));

		if (g_Options.bUseGroups) {
			if ((group = db_get_tsa(hContact, "CList", "Group")) == NULL)
				group = mir_tstrdup(TranslateT("<no group>"));
		}
		else group = mir_tstrdup(TranslateT("Favorite Contacts"));

		status = db_get_w(hContact, GetContactProto(hContact), "Status", ID_STATUS_OFFLINE);
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

	TCHAR *getGroup() const
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
		if (res = mir_tstrcmp(p1->group, p2->group)) return res;
		if (res = mir_tstrcmp(p1->name, p2->name)) return res;
		return 0;
	}
};

class TFavContacts : public LIST < TContactInfo >
{
private:
	int nGroups;
	TCHAR *prevGroup;
	MIDatabase *db;

	int addContact(MCONTACT hContact, bool bManual)
	{
		DBCachedContact *cc = db->m_cache->GetCachedContact(hContact);
		if (cc == NULL)
			return 0;

		if (db_mc_isEnabled()) {
			if (cc->IsSub()) // skip subcontacts if MC is enabled
				return 0;
		}
		else if (cc->IsMeta()) // skip metacontacts if MC is not enabled
			return 0;

		TContactInfo *info = new TContactInfo(hContact, bManual);
		insert(info);
		TCHAR *group = info->getGroup();
		if (prevGroup && mir_tstrcmp(prevGroup, group))
			++nGroups;
		prevGroup = group;
		return 1;
	}

public:
	TFavContacts() : LIST<TContactInfo>(5, TContactInfo::cmp)
	{
		db = GetCurrentDatabase();
	}

	~TFavContacts()
	{
		for (int i = 0; i < this->getCount(); ++i)
			delete (*this)[i];
	}

	__forceinline int groupCount() const { return nGroups; }

	void build()
	{
		prevGroup = NULL;

		nGroups = 1;

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			if (db_get_b(hContact, "FavContacts", "IsFavourite", 0))
				addContact(hContact, true);

		int nRecent = 0;
		for (int i = 0; nRecent < g_Options.wMaxRecent; ++i) {
			MCONTACT hContact = g_contactCache->get(i);
			if (!hContact)
				break;
			
			if (!db_get_b(hContact, "FavContacts", "IsFavourite", 0))
				nRecent += addContact(hContact, false);
		}
	}
};

#endif // favlist_h__
