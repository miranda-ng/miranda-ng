/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-15 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"

HWND FacebookProto::NotifyEvent(TCHAR* title, TCHAR* info, MCONTACT contact, DWORD flags, std::string *url, std::string *notification_id)
{
	if (title == NULL || info == NULL)
		return NULL;

	char name[256];

	switch (flags)
	{
	case FACEBOOK_EVENT_CLIENT:
		if (!getByte(FACEBOOK_KEY_EVENT_CLIENT_ENABLE, DEFAULT_EVENT_CLIENT_ENABLE))
			return NULL;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Client");
		flags |= NIIF_WARNING;
		break;

	case FACEBOOK_EVENT_NEWSFEED:
		if (!getByte(FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE))
			return NULL;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Newsfeed");
		SkinPlaySound("NewsFeed");
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_NOTIFICATION:
		if (!getByte(FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
			return NULL;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Notification");
		SkinPlaySound("Notification");
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_OTHER:
		if (!getByte(FACEBOOK_KEY_EVENT_OTHER_ENABLE, DEFAULT_EVENT_OTHER_ENABLE))
			return NULL;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Other");
		SkinPlaySound("OtherEvent");
		flags |= NIIF_INFO;
		break;
	}

	if (!getByte(FACEBOOK_KEY_SYSTRAY_NOTIFY, DEFAULT_SYSTRAY_NOTIFY))
	{
		if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {

			// TODO: if popup with particular ID is already showed, just update his content
			// ... but f***ed up Popup Classes won't allow it now - they need to return hPopupWindow somehow
			/* if (popup exists) {
				if (PUChangeTextT(hWndPopup, info) > 0) // success
				return;
				}*/

			POPUPDATACLASS pd = { sizeof(pd) };
			pd.ptszTitle = title;
			pd.ptszText = info;
			pd.pszClassName = name;
			pd.hContact = contact;

			if (url != NULL || notification_id != NULL) {
				popup_data *data = new popup_data(this);
				if (url != NULL)
					data->url = *url;
				if (notification_id != NULL)
					data->notification_id = *notification_id;
				pd.PluginData = data;
			}

			return (HWND)CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&pd);
		}
	}
	else {
		if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY))
		{
			MIRANDASYSTRAYNOTIFY err;
			int niif_flags = flags;
			REMOVE_FLAG(niif_flags, FACEBOOK_EVENT_CLIENT |
				FACEBOOK_EVENT_NEWSFEED |
				FACEBOOK_EVENT_NOTIFICATION |
				FACEBOOK_EVENT_OTHER);
			err.szProto = m_szModuleName;
			err.cbSize = sizeof(err);
			err.dwInfoFlags = NIIF_INTERN_TCHAR | niif_flags;
			err.tszInfoTitle = title;
			err.tszInfo = info;
			err.uTimeout = 10000;
			if (CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)& err) == 0)
				return NULL;
		}
	}

	if (FLAG_CONTAINS(flags, FACEBOOK_EVENT_CLIENT))
		MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);

	return NULL;
}
