/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

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
	if ( !getByte( FACEBOOK_KEY_LOGGING_ENABLE, 0 ) )
		return EXIT_SUCCESS;

	va_list va;
	char text[65535];
	ScopedLock s(log_lock_);

	va_start(va,fmt);
	mir_vsnprintf(text,sizeof(text),fmt,va);
	va_end(va);

	return utils::debug::log( m_szModuleName, text );
}

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_COMMAND:
	{
		//Get the plugin data (we need the PopUp service to do it)
		TCHAR* data = (TCHAR*)PUGetPluginData(hwnd);
		if (data != NULL)
		{
			std::string url = mir_t2a_cp(data,CP_UTF8);
			if ( url.substr(0,4) != "http" )
				url = FACEBOOK_URL_HOMEPAGE + url; // make absolute url

			CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) url.c_str() );
		}

		// After a click, destroy popup
		PUDeletePopUp(hwnd);
		} break;

	case WM_CONTEXTMENU: 
		PUDeletePopUp(hwnd);
		break;

	case UM_FREEPLUGINDATA:
	{
		// After close, free
		TCHAR* url = (TCHAR*)PUGetPluginData(hwnd);
		if (url != NULL)
			mir_free(url);
	} return FALSE;

	default:
		break; 
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
};

void FacebookProto::NotifyEvent(TCHAR* title, TCHAR* info, HANDLE contact, DWORD flags, TCHAR* szUrl)
{
	int ret; int timeout; COLORREF colorBack = 0; COLORREF colorText = 0;

	switch ( flags )
	{
	case FACEBOOK_EVENT_CLIENT:
		if ( !getByte( FACEBOOK_KEY_EVENT_CLIENT_ENABLE, DEFAULT_EVENT_CLIENT_ENABLE ) )
			goto exit;
		if ( !getByte( FACEBOOK_KEY_EVENT_CLIENT_DEFAULT, 0 ) )
		{
			colorBack = getDword( FACEBOOK_KEY_EVENT_CLIENT_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_CLIENT_COLTEXT, DEFAULT_EVENT_COLTEXT );
		}
		timeout = getDword( FACEBOOK_KEY_EVENT_CLIENT_TIMEOUT, 0 );
		flags |= NIIF_WARNING;
		break;

	case FACEBOOK_EVENT_NEWSFEED:
		if ( !getByte( FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE ) )
			goto exit;
		if ( !getByte( FACEBOOK_KEY_EVENT_FEEDS_DEFAULT, 0 ) )
		{
			colorBack = getDword( FACEBOOK_KEY_EVENT_FEEDS_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_FEEDS_COLTEXT, DEFAULT_EVENT_COLTEXT );
		}
		timeout = getDword( FACEBOOK_KEY_EVENT_FEEDS_TIMEOUT, 0 );
		SkinPlaySound( "NewsFeed" );
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_NOTIFICATION:
		if ( !getByte( FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE ) )
			goto exit;
		if ( !getByte( FACEBOOK_KEY_EVENT_NOTIFICATIONS_DEFAULT, 0 ) )
		{
			colorBack = getDword( FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLTEXT, DEFAULT_EVENT_COLTEXT );
		}
		timeout = getDword( FACEBOOK_KEY_EVENT_NOTIFICATIONS_TIMEOUT, 0 );
		SkinPlaySound( "Notification" );
		flags |= NIIF_INFO;
		break;

	case FACEBOOK_EVENT_OTHER:
		if ( !getByte( FACEBOOK_KEY_EVENT_OTHER_ENABLE, DEFAULT_EVENT_OTHER_ENABLE ) )
			goto exit;
		if ( !getByte( FACEBOOK_KEY_EVENT_OTHER_DEFAULT, 0 ) )
		{
			colorBack = getDword( FACEBOOK_KEY_EVENT_OTHER_COLBACK, DEFAULT_EVENT_COLBACK );
			colorText = getDword( FACEBOOK_KEY_EVENT_OTHER_COLTEXT, DEFAULT_EVENT_COLTEXT );
		}
		timeout = getDword( FACEBOOK_KEY_EVENT_OTHER_TIMEOUT, 0 );
		SkinPlaySound( "OtherEvent" );
		flags |= NIIF_INFO;
		break;
	}

	if ( !getByte(FACEBOOK_KEY_SYSTRAY_NOTIFY,DEFAULT_SYSTRAY_NOTIFY) )
	{
		if (ServiceExists(MS_POPUP_ADDPOPUP))
		{
			POPUPDATAT pd;
			pd.colorBack = colorBack;
			pd.colorText = colorText;
			pd.iSeconds = timeout;
			pd.lchContact = contact;
			pd.lchIcon = GetIcon(1); // TODO: Icon test
			pd.PluginData = szUrl;
			pd.PluginWindowProc = (WNDPROC)PopupDlgProc;
			lstrcpy(pd.lptzContactName, title);
			lstrcpy(pd.lptzText, info);
			ret = PUAddPopUpT(&pd);

			if (ret == 0)
				return;
		}
	} else {
		if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY))
		{
			MIRANDASYSTRAYNOTIFY err;
			int niif_flags = flags;
			REMOVE_FLAG( niif_flags, FACEBOOK_EVENT_CLIENT |
			                         FACEBOOK_EVENT_NEWSFEED |
			                         FACEBOOK_EVENT_NOTIFICATION |
			                         FACEBOOK_EVENT_OTHER );
			err.szProto = m_szModuleName;
			err.cbSize = sizeof(err);
			err.dwInfoFlags = NIIF_INTERN_TCHAR | niif_flags;
			err.tszInfoTitle = title;
			err.tszInfo = info;
			err.uTimeout = 1000 * timeout;
			ret = CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & err);

			if (ret == 0)
				goto exit;
		}
	}

	if (FLAG_CONTAINS(flags, FACEBOOK_EVENT_CLIENT))
		MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);

exit:
	if (szUrl != NULL)
		mir_free(szUrl);
}
