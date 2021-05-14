#include "stdafx.h"

static IconItem Icons[] =
{
	{ LPGEN("Unread message icon"), "unread_icon", IDI_UNREAD },
	{ LPGEN("Read message icon"), "read_icon", IDI_READ },
	{ LPGEN("Failed sending icon"), "fail_icon", IDI_FAIL },
	{ LPGEN("Sending message icon"), "nosent_icon", IDI_NOSENT },
	{ LPGEN("Unread clist extra icon"), "clist_unread_icon", IDI_EXTRA },
};

static INT_PTR UpdateService(WPARAM hContact, LPARAM lParam)
{
	auto *p = FindContact(hContact);
	
	time_t currTime = time(0);
	if (currTime > p->dwLastReadTime) {
		p->dwLastReadTime = currTime;
		p->type = lParam;

		if (db_mc_isSub(hContact)) {
			p = FindContact(db_mc_getMeta(hContact));
			p->dwLastReadTime = currTime;
			p->type = lParam;
		}
		
		IconsUpdate(hContact);
	}
	return 0;
}

void InitServices()
{
	g_plugin.registerIcon(MODULENAME, Icons);

	CreateServiceFunction(MS_MESSAGESTATE_UPDATE, UpdateService);
}
