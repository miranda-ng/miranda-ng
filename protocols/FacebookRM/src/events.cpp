/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-13 Robert Pösel

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

int FacebookProto::Log(const char *fmt,...)
{
	if (!getByte(FACEBOOK_KEY_LOGGING_ENABLE, 0))
		return EXIT_SUCCESS;

	va_list va;
	char text[65535];
	ScopedLock s(log_lock_);

	va_start(va,fmt);
	mir_vsnprintf(text,sizeof(text),fmt,va);
	va_end(va);

	// Write into network log
	CallService(MS_NETLIB_LOG, (WPARAM)m_hNetlibUser, (LPARAM)text);

	// Write into log file
	return utils::debug::log(m_szModuleName, text);
}

void FacebookProto::NotifyEvent(TCHAR* title, TCHAR* info, HANDLE contact, DWORD flags, std::string *url, std::string *notification_id)
{
	char name[256];

	switch (flags)
	{
	case FACEBOOK_EVENT_CLIENT:
		if (!getByte(FACEBOOK_KEY_EVENT_CLIENT_ENABLE, DEFAULT_EVENT_CLIENT_ENABLE))
			return;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Client");
		flags |= NIIF_WARNING;
		break;

	case FACEBOOK_EVENT_NEWSFEED:
		if (!getByte(FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE))
			return;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Newsfeed");
		SkinPlaySound("NewsFeed");
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_NOTIFICATION:
		if (!getByte(FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
			return;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Notification");
		SkinPlaySound("Notification");
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_OTHER:
		if (!getByte(FACEBOOK_KEY_EVENT_OTHER_ENABLE, DEFAULT_EVENT_OTHER_ENABLE))
			return;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Other");
		SkinPlaySound("OtherEvent");
		flags |= NIIF_INFO;
		break;
	}

	if (!getByte(FACEBOOK_KEY_SYSTRAY_NOTIFY,DEFAULT_SYSTRAY_NOTIFY))
	{
		if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
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

			if (CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&pd) == 0)
				return;
		}
	} else {
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
			if (CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & err) == 0)
				return;
		}
	}

	if (FLAG_CONTAINS(flags, FACEBOOK_EVENT_CLIENT))
		MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);
}
