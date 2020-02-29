#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/////////////////////////////////////////////////////////////////////////////////////////

struct ContactData
{
	ContactData(MCONTACT _p1) :
		hContact(_p1)
	{}

	MCONTACT hContact;

	int type = -1, bHidden = true;
	__time64_t dwLastReadTime = 0, dwLastSentTime = 0;

	void __forceinline setSent(__time64_t _time)
	{
		dwLastSentTime = _time;
		type = 0;
	}
};

extern OBJLIST<ContactData> g_arContacts;

ContactData* FindContact(MCONTACT);

/////////////////////////////////////////////////////////////////////////////////////////


__forceinline bool CheckProtoSupport(const char *szProto)
{	return (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_READNOTIFY) != 0;
}

void InitServices();

void IconsUpdate(MCONTACT);
void SetSRMMIcon(MCONTACT hContact, int type, time_t time = 0);
time_t GetLastSentMessageTime(MCONTACT hContact);
int OnModulesLoaded(WPARAM, LPARAM);
bool HasUnread(MCONTACT hContact);

#endif //_GLOBAL_H_