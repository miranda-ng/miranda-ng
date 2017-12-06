#ifndef _GLOBAL_H_
#define _GLOBAL_H_

enum SRMM_ICON_TYPE
{
	ICON_HIDDEN = -1,
	ICON_UNREAD,
	ICON_READ,
	ICON_FAILED,
	ICON_NOSENT,
	ICON_EXTRA,
};

#define DBKEY_MESSAGE_READ_TIME "LastMsgReadTime"
#define DBKEY_MESSAGE_READ_TIME_TYPE "LastMsgReadTimeType"

__forceinline bool CheckProtoSupport(const char *szProto)
{	return (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_READNOTIFY) != 0;
}

void InitServices();
INT_PTR UpdateService(WPARAM, LPARAM);

int IconsUpdate(MCONTACT);
void SetSRMMIcon(MCONTACT hContact, SRMM_ICON_TYPE type, time_t time = 0);
time_t GetLastSentMessageTime(MCONTACT hContact);
int OnModulesLoaded(WPARAM, LPARAM);
bool HasUnread(MCONTACT hContact);
int ExtraIconsApply(WPARAM hContact, LPARAM);
void InitClistExtraIcon();

#endif //_GLOBAL_H_