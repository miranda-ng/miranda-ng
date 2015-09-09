#ifndef _GLOBAL_H_
#define _GLOBAL_H_

static LIST<void> arMonitoredWindows(3, PtrKeySortT);

static IconItem Icons[] = 
{
	{ LPGEN("Unread message icon"), "unread_icon", IDI_UNREAD },
	{ LPGEN("Read message icon"), "read_icon", IDI_READ },
	{ LPGEN("Failed sending icon"), "fail_icon", IDI_FAIL },
	{ LPGEN("Sending message icon"), "nosent_icon", IDI_NOSENT },
	{ LPGEN("Unread clist extra icon"), "clist_unread_icon", IDI_EXTRA },
};

enum SRMM_ICON_TYPE
{
	ICON_HIDDEN,
	ICON_READ,
	ICON_UNREAD,
	ICON_FAILED,
	ICON_NOSENT
};

#define FLAG_CONTAINS(x,y)      ((x & y) == y)

#define DBKEY_MESSAGE_READ_TIME "LastMsgReadTime"
#define DBKEY_MESSAGE_READ_TIME_TYPE "LastMsgReadTimeType"

__forceinline bool CheckProtoSupport(const char *szProto)
{	return ((szProto != NULL) ? FLAG_CONTAINS(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0), PF4_READNOTIFY) : false);
}

void InitServices();
void DestroyServices();
INT_PTR UpdateService(WPARAM, LPARAM);

int IconsUpdate(WPARAM hContact, LONGLONG readtime);
void SetSRMMIcon(MCONTACT hContact, SRMM_ICON_TYPE type, time_t time = 0);
LONGLONG GetLastSentMessageTime(MCONTACT hContact);
int OnModulesLoaded(WPARAM, LPARAM);
bool HasUnread(MCONTACT hContact);
int ExtraIconsApply(WPARAM hContact, LPARAM);
void InitClistExtraIcon();

#endif //_GLOBAL_H_