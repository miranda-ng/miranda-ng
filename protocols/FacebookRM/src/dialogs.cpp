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

static BOOL LoadDBCheckState(FacebookProto* ppro, HWND hwnd, int idCtrl, const char* szSetting, BYTE bDef)
{
	BOOL state = ppro->getByte(szSetting, bDef);
	CheckDlgButton(hwnd, idCtrl, state);
	return state;
}

static BOOL StoreDBCheckState(FacebookProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	BOOL state = IsDlgButtonChecked(hwnd, idCtrl);
	ppro->setByte(szSetting, (BYTE)state);
	return state;
}

INT_PTR CALLBACK FBAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

	switch (message)
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);

		DBVARIANT dbv;
		if (!db_get_s(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,&dbv))
		{
			SetDlgItemTextA(hwnd,IDC_UN,dbv.pszVal);
			db_free(&dbv);
		}

		if (!db_get_s(0,proto->ModuleName(),FACEBOOK_KEY_PASS,&dbv))
		{
			CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
				reinterpret_cast<LPARAM>(dbv.pszVal));
			SetDlgItemTextA(hwnd,IDC_PW,dbv.pszVal);
			db_free(&dbv);
		}

		if (!proto->isOffline()) {
			SendMessage(GetDlgItem(hwnd,IDC_UN),EM_SETREADONLY,1,0);
			SendMessage(GetDlgItem(hwnd,IDC_PW),EM_SETREADONLY,1,0); }

		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wparam) == IDC_NEWACCOUNTLINK)
		{
			proto->OpenUrl(std::string(FACEBOOK_URL_HOMEPAGE));
			return TRUE;
		}

		if (HIWORD(wparam) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus())
		{
			switch(LOWORD(wparam))
			{
			case IDC_UN:
			case IDC_PW:
				SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			char str[128];

			GetDlgItemTextA(hwnd,IDC_UN,str,sizeof(str));
			db_set_s(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,str);

			GetDlgItemTextA(hwnd,IDC_PW,str,sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING,sizeof(str),reinterpret_cast<LPARAM>(str));
			db_set_s(0,proto->ModuleName(),FACEBOOK_KEY_PASS,str);

			return TRUE;
		}
		break;

	}

	return FALSE;
}

void RefreshPrivacy(HWND hwnd, post_status_data *data)
{
	SendDlgItemMessage(hwnd, IDC_PRIVACY, CB_RESETCONTENT, 0, 0);
	int wall_id = SendDlgItemMessage(hwnd, IDC_WALL, CB_GETCURSEL, 0, 0);
	if (data->walls[wall_id]->user_id == data->proto->facy.self_.user_id) {
		for (size_t i = 0; i < SIZEOF(privacy_types); i++)
			SendDlgItemMessageA(hwnd, IDC_PRIVACY, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(Translate(privacy_types[i].name)));
	} else {
		SendDlgItemMessage(hwnd, IDC_PRIVACY, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(TranslateT("Default")));
	}
	SendDlgItemMessage(hwnd, IDC_PRIVACY, CB_SETCURSEL, 0, 0);
	SendDlgItemMessage(hwnd, IDC_PRIVACY, CB_SETCURSEL, data->proto->getByte(FACEBOOK_KEY_PRIVACY_TYPE, 0), 0);
}

INT_PTR CALLBACK FBMindProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	post_status_data *data;

	switch(message)
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(GetIconHandle("mind")));

		data = reinterpret_cast<post_status_data*>(lparam);

		SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
		SendDlgItemMessage(hwnd, IDC_MINDMSG, EM_LIMITTEXT, FACEBOOK_MIND_LIMIT, 0);
		SendDlgItemMessage(hwnd, IDC_URL, EM_LIMITTEXT, 1024, 0);

		ptrT place = data->proto->getTStringA(FACEBOOK_KEY_PLACE);
		SetDlgItemText(hwnd, IDC_PLACE, place != NULL ? place : _T("Miranda NG"));

		for (std::vector<wall_data*>::size_type i = 0; i < data->walls.size(); i++)
			SendDlgItemMessage(hwnd, IDC_WALL, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(data->walls[i]->title));
		SendDlgItemMessage(hwnd, IDC_WALL, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage(hwnd, IDC_WALL, CB_SETCURSEL, data->proto->getByte(FACEBOOK_KEY_LAST_WALL, 0), 0);
		RefreshPrivacy(hwnd, data);

		ptrA name(data->proto->getStringA(FACEBOOK_KEY_NAME));
		if (name != NULL) {
			std::string firstname = name;
			std::string::size_type pos = firstname.find(" ");
			if (pos != std::string::npos)
				firstname = firstname.substr(0, pos);

			char title[100];
			mir_snprintf(title, SIZEOF(title), Translate("What's on your mind, %s?"), firstname.c_str());
			SetWindowTextA(hwnd, title);
		}
	}

	EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
	return TRUE;

	case WM_COMMAND:
		if (LOWORD(wparam) == IDC_WALL && HIWORD(wparam) == CBN_SELCHANGE)
		{
			data = reinterpret_cast<post_status_data*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));
			RefreshPrivacy(hwnd, data);

			// remember last choice, only when there are more options
			if (SendDlgItemMessage(hwnd, IDC_WALL, CB_GETCOUNT, 0, 0) > 1)
				data->proto->setByte(FACEBOOK_KEY_LAST_WALL, SendDlgItemMessage(hwnd, IDC_WALL, CB_GETCURSEL, 0, 0));
		}
		if (LOWORD(wparam) == IDC_PRIVACY && HIWORD(wparam) == CBN_SELCHANGE) {
			data = reinterpret_cast<post_status_data*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

			// remember last choice, only when there are more options
			if (SendDlgItemMessage(hwnd, IDC_PRIVACY, CB_GETCOUNT, 0, 0) > 1)
				data->proto->setByte(FACEBOOK_KEY_PRIVACY_TYPE, SendDlgItemMessage(hwnd, IDC_PRIVACY, CB_GETCURSEL, 0, 0));
		}
		else if (LOWORD(wparam) == IDC_MINDMSG && HIWORD(wparam) == EN_CHANGE)
		{
			size_t len = SendDlgItemMessage(hwnd,IDC_MINDMSG,WM_GETTEXTLENGTH,0,0);
			EnableWindow(GetDlgItem(hwnd, IDOK), len > 0);

			return TRUE;
		}
		else if (LOWORD(wparam) == IDOK)
		{
			data = reinterpret_cast<post_status_data*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

			TCHAR mindMessageT[FACEBOOK_MIND_LIMIT+1];
			TCHAR urlT[1024];
			TCHAR placeT[100];		

			GetDlgItemText(hwnd, IDC_MINDMSG, mindMessageT, SIZEOF(mindMessageT));
			GetDlgItemText(hwnd, IDC_PLACE, placeT, SIZEOF(placeT));
			GetDlgItemText(hwnd, IDC_URL, urlT, SIZEOF(urlT));
			ShowWindow(hwnd, SW_HIDE);

			ptrA place(mir_utf8encodeT(placeT));
			data->proto->setString(FACEBOOK_KEY_PLACE, place);

			int wall_id = SendDlgItemMessage(hwnd, IDC_WALL, CB_GETCURSEL, 0, 0);

			status_data *status = new status_data();
			status->user_id = data->walls[wall_id]->user_id;
			status->isPage = data->walls[wall_id]->isPage;
			status->privacy = privacy_types[SendDlgItemMessage(hwnd, IDC_PRIVACY, CB_GETCURSEL, 0, 0)].id;
			status->place = place;
			status->url = _T2A(urlT);

			char *narrow = mir_utf8encodeT(mindMessageT);		
			status->text = narrow;

			if (status->user_id == data->proto->facy.self_.user_id && data->proto->last_status_msg_ != narrow)
				data->proto->last_status_msg_ = narrow;

			mir_free(narrow);

			data->proto->ForkThread(&FacebookProto::SetAwayMsgWorker, status);

			EndDialog(hwnd, wparam);
			return TRUE;
		}
		else if (LOWORD(wparam) == IDCANCEL)
		{
			EndDialog(hwnd, wparam);
			return TRUE;
		}
		break;
	case WM_DESTROY:
		data = reinterpret_cast<post_status_data*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

		for(std::vector<wall_data*>::size_type i = 0; i < data->walls.size(); i++) {
			mir_free(data->walls[i]->title);
			delete data->walls[i];
		}

		delete data;
	}

	return FALSE;
}

INT_PTR CALLBACK FBOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

	switch (message)
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);

		DBVARIANT dbv;
		if (!db_get_s(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,&dbv))
		{
			SetDlgItemTextA(hwnd,IDC_UN,dbv.pszVal);
			db_free(&dbv);
		}

		if (!db_get_s(0,proto->ModuleName(),FACEBOOK_KEY_PASS,&dbv))
		{
			CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,reinterpret_cast<LPARAM>(dbv.pszVal));
			SetDlgItemTextA(hwnd,IDC_PW,dbv.pszVal);
			db_free(&dbv);
		}

		if (!proto->isOffline())
	    {
			SendMessage(GetDlgItem(hwnd,IDC_UN),EM_SETREADONLY,TRUE,0);
			SendMessage(GetDlgItem(hwnd,IDC_PW),EM_SETREADONLY,TRUE,0);
		}

		SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, FACEBOOK_GROUP_NAME_LIMIT, 0);

		if (!db_get_ts(0,proto->ModuleName(),FACEBOOK_KEY_DEF_GROUP,&dbv))
		{
			SetDlgItemText(hwnd,IDC_GROUP,dbv.ptszVal);
			db_free(&dbv);
		}

		LoadDBCheckState(proto, hwnd, IDC_SET_IGNORE_STATUS, FACEBOOK_KEY_DISABLE_STATUS_NOTIFY, DEFAULT_DISABLE_STATUS_NOTIFY);
		LoadDBCheckState(proto, hwnd, IDC_BIGGER_AVATARS, FACEBOOK_KEY_BIG_AVATARS, DEFAULT_BIG_AVATARS);
		LoadDBCheckState(proto, hwnd, IDC_LOAD_MOBILE, FACEBOOK_KEY_LOAD_MOBILE, DEFAULT_LOAD_MOBILE);

	} return TRUE;

	case WM_COMMAND:
	{
		if (LOWORD(wparam) == IDC_NEWACCOUNTLINK)
		{
			proto->OpenUrl(std::string(FACEBOOK_URL_HOMEPAGE));
			return TRUE;
		}

		if (LOWORD(wparam) == IDC_SECURE) {
			EnableWindow(GetDlgItem(hwnd, IDC_SECURE_CHANNEL), IsDlgButtonChecked(hwnd, IDC_SECURE));
		}

		if ((LOWORD(wparam)==IDC_UN || LOWORD(wparam)==IDC_PW || LOWORD(wparam)==IDC_GROUP) &&
		    (HIWORD(wparam)!=EN_CHANGE || (HWND)lparam!=GetFocus()))
			return 0;
		else
			SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);

	} break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			char str[128]; TCHAR tstr[128];

			GetDlgItemTextA(hwnd,IDC_UN,str,sizeof(str));
			db_set_s(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,str);

			GetDlgItemTextA(hwnd,IDC_PW,str,sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING,sizeof(str),reinterpret_cast<LPARAM>(str));
			proto->setString(FACEBOOK_KEY_PASS, str);

			GetDlgItemText(hwnd,IDC_GROUP,tstr,sizeof(tstr));
			if (lstrlen(tstr) > 0)
			{
				proto->setTString(FACEBOOK_KEY_DEF_GROUP, tstr);
				CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)tstr);
			}
			else proto->delSetting(FACEBOOK_KEY_DEF_GROUP);

			StoreDBCheckState(proto, hwnd, IDC_SET_IGNORE_STATUS, FACEBOOK_KEY_DISABLE_STATUS_NOTIFY);
			StoreDBCheckState(proto, hwnd, IDC_BIGGER_AVATARS, FACEBOOK_KEY_BIG_AVATARS);
			StoreDBCheckState(proto, hwnd, IDC_LOAD_MOBILE, FACEBOOK_KEY_LOAD_MOBILE);

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBOptionsAdvancedProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

	switch (message)
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);

		for(size_t i=0; i<SIZEOF(server_types); i++)
			SendDlgItemMessageA(hwnd, IDC_URL_SERVER, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(Translate(server_types[i].name)));
		SendDlgItemMessage(hwnd, IDC_URL_SERVER, CB_SETCURSEL, proto->getByte(FACEBOOK_KEY_SERVER_TYPE, 0), 0);

		LoadDBCheckState(proto, hwnd, IDC_SECURE, FACEBOOK_KEY_FORCE_HTTPS, DEFAULT_FORCE_HTTPS);
		LoadDBCheckState(proto, hwnd, IDC_SECURE_CHANNEL, FACEBOOK_KEY_FORCE_HTTPS_CHANNEL, DEFAULT_FORCE_HTTPS_CHANNEL);
		LoadDBCheckState(proto, hwnd, IDC_DISCONNECT_CHAT, FACEBOOK_KEY_DISCONNECT_CHAT, DEFAULT_DISCONNECT_CHAT);
		LoadDBCheckState(proto, hwnd, IDC_SET_STATUS, FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS);
		LoadDBCheckState(proto, hwnd, IDC_LOGGING, FACEBOOK_KEY_LOGGING_ENABLE, DEFAULT_LOGGING_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_MAP_STATUSES, FACEBOOK_KEY_MAP_STATUSES, DEFAULT_MAP_STATUSES);
		LoadDBCheckState(proto, hwnd, IDC_CUSTOM_SMILEYS, FACEBOOK_KEY_CUSTOM_SMILEYS, DEFAULT_CUSTOM_SMILEYS);
		LoadDBCheckState(proto, hwnd, IDC_USE_LOCAL_TIME, FACEBOOK_KEY_LOCAL_TIMESTAMP, DEFAULT_LOCAL_TIME);
		LoadDBCheckState(proto, hwnd, IDC_LOAD_PAGES, FACEBOOK_KEY_LOAD_PAGES, DEFAULT_LOAD_PAGES);

		EnableWindow(GetDlgItem(hwnd, IDC_SECURE_CHANNEL), IsDlgButtonChecked(hwnd, IDC_SECURE));

		return TRUE;
	}

	case WM_COMMAND: {
		if (LOWORD(wparam) == IDC_SECURE) {
			EnableWindow(GetDlgItem(hwnd, IDC_SECURE_CHANNEL), IsDlgButtonChecked(hwnd, IDC_SECURE));
		}

		if (LOWORD(wparam) == IDC_SECURE_CHANNEL && IsDlgButtonChecked(hwnd, IDC_SECURE_CHANNEL))
			MessageBox(hwnd, TranslateT("Note: Make sure you have disabled 'Validate SSL certificates' option in Network options to work properly."), proto->m_tszUserName, MB_OK);

		SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);

		break;
	}

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			proto->setByte(FACEBOOK_KEY_SERVER_TYPE, SendDlgItemMessage(hwnd, IDC_URL_SERVER, CB_GETCURSEL, 0, 0));

			StoreDBCheckState(proto, hwnd, IDC_SECURE, FACEBOOK_KEY_FORCE_HTTPS);
			StoreDBCheckState(proto, hwnd, IDC_LOGGING, FACEBOOK_KEY_LOGGING_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_SECURE_CHANNEL, FACEBOOK_KEY_FORCE_HTTPS_CHANNEL);
			StoreDBCheckState(proto, hwnd, IDC_DISCONNECT_CHAT, FACEBOOK_KEY_DISCONNECT_CHAT);
			StoreDBCheckState(proto, hwnd, IDC_MAP_STATUSES, FACEBOOK_KEY_MAP_STATUSES);
			StoreDBCheckState(proto, hwnd, IDC_CUSTOM_SMILEYS, FACEBOOK_KEY_CUSTOM_SMILEYS);
			StoreDBCheckState(proto, hwnd, IDC_USE_LOCAL_TIME, FACEBOOK_KEY_LOCAL_TIMESTAMP);
			StoreDBCheckState(proto, hwnd, IDC_LOAD_PAGES, FACEBOOK_KEY_LOAD_PAGES);

			BOOL setStatus = IsDlgButtonChecked(hwnd, IDC_SET_STATUS);
			BOOL setStatusOld = proto->getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS);
			if (setStatus != setStatusOld)
			{
				proto->setByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, setStatus);
				if (setStatus && proto->isOnline())
					proto->ForkThread(&FacebookProto::SetAwayMsgWorker, NULL);
			}

			return TRUE;
		}

		break;
	}

	return FALSE;
}


INT_PTR CALLBACK FBEventsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

	switch(message)
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);

		for(size_t i=0; i<SIZEOF(feed_types); i++)
		{
			SendDlgItemMessageA(hwnd,IDC_FEED_TYPE,CB_INSERTSTRING,i,
				reinterpret_cast<LPARAM>(Translate(feed_types[i].name)));
		}
		SendDlgItemMessage(hwnd, IDC_FEED_TYPE, CB_SETCURSEL, proto->getByte(FACEBOOK_KEY_FEED_TYPE, 0), 0);
		LoadDBCheckState(proto, hwnd, IDC_SYSTRAY_NOTIFY, FACEBOOK_KEY_SYSTRAY_NOTIFY, DEFAULT_SYSTRAY_NOTIFY);

		LoadDBCheckState(proto, hwnd, IDC_NOTIFICATIONS_ENABLE, FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_FEEDS_ENABLE, FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_CLIENT_ENABLE, FACEBOOK_KEY_EVENT_CLIENT_ENABLE, DEFAULT_EVENT_CLIENT_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_OTHER_ENABLE, FACEBOOK_KEY_EVENT_OTHER_ENABLE, DEFAULT_EVENT_OTHER_ENABLE);

	} return TRUE;

	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case IDC_PREVIEW:
		{
			TCHAR protoName[255];
			lstrcpy(protoName, proto->m_tszUserName);
			proto->NotifyEvent(protoName, TranslateT("Sample event"), NULL, FACEBOOK_EVENT_CLIENT);
			proto->NotifyEvent(protoName, TranslateT("Sample request"), NULL, FACEBOOK_EVENT_OTHER);
			proto->NotifyEvent(protoName, TranslateT("Sample newsfeed"), NULL, FACEBOOK_EVENT_NEWSFEED);
			proto->NotifyEvent(protoName, TranslateT("Sample notification"), NULL, FACEBOOK_EVENT_NOTIFICATION);
		} break;
		}

		if ((LOWORD(wparam)==IDC_PREVIEW || (HWND)lparam!=GetFocus()))
			return 0;
		else
			SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);

	} return TRUE;

	case WM_NOTIFY:
	{
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			proto->setByte(FACEBOOK_KEY_FEED_TYPE, SendDlgItemMessage(hwnd, IDC_FEED_TYPE, CB_GETCURSEL, 0, 0));

			StoreDBCheckState(proto, hwnd, IDC_SYSTRAY_NOTIFY, FACEBOOK_KEY_SYSTRAY_NOTIFY);

			StoreDBCheckState(proto, hwnd, IDC_NOTIFICATIONS_ENABLE, FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_FEEDS_ENABLE, FACEBOOK_KEY_EVENT_FEEDS_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_OTHER_ENABLE, FACEBOOK_KEY_EVENT_OTHER_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_CLIENT_ENABLE, FACEBOOK_KEY_EVENT_CLIENT_ENABLE);
		}
	} return TRUE;

	}

	return FALSE;
}
