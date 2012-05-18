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

static BOOL LoadDBCheckState(FacebookProto* ppro, HWND hwnd, int idCtrl, const char* szSetting, BYTE bDef)
{
	BOOL state = DBGetContactSettingByte(NULL, ppro->m_szModuleName, szSetting, bDef);
	CheckDlgButton(hwnd, idCtrl, state);
	return state;
}

static BOOL StoreDBCheckState(FacebookProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	BOOL state = IsDlgButtonChecked(hwnd, idCtrl);
	DBWriteContactSettingByte(NULL, ppro->m_szModuleName, szSetting, (BYTE)state);
	return state;
}

INT_PTR CALLBACK FBAccountProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto;

	switch ( message )
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWLP_USERDATA,lparam);

		DBVARIANT dbv;
		if( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,&dbv) )
		{
			SetDlgItemTextA(hwnd,IDC_UN,dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		if ( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_PASS,&dbv) )
		{
			CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
				reinterpret_cast<LPARAM>(dbv.pszVal));
			SetDlgItemTextA(hwnd,IDC_PW,dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		if (!proto->isOffline()) {
			SendMessage(GetDlgItem(hwnd,IDC_UN),EM_SETREADONLY,1,0);
			SendMessage(GetDlgItem(hwnd,IDC_PW),EM_SETREADONLY,1,0); }

		return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDC_NEWACCOUNTLINK )
		{
			CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>
				( FACEBOOK_URL_HOMEPAGE ) );
			return TRUE;
		}

		if ( HIWORD( wparam ) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus() )
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
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{
			proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWLP_USERDATA));
			char str[128];

			GetDlgItemTextA(hwnd,IDC_UN,str,sizeof(str));
			DBWriteContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,str);

			GetDlgItemTextA(hwnd,IDC_PW,str,sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING,sizeof(str),reinterpret_cast<LPARAM>(str));
			DBWriteContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_PASS,str);

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBMindProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto;

	switch(message)
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWLP_USERDATA,lparam);
		SendDlgItemMessage(hwnd,IDC_MINDMSG,EM_LIMITTEXT,FACEBOOK_MIND_LIMIT,0);

		DBVARIANT dbv = { DBVT_TCHAR };

		if (!DBGetContactSettingTString(NULL,proto->m_szModuleName,FACEBOOK_KEY_NAME,&dbv))
		{
			SetWindowText( hwnd, dbv.ptszVal );
			DBFreeVariant( &dbv );
		}
	}

	EnableWindow(GetDlgItem( hwnd, IDOK ), FALSE);
	return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDC_MINDMSG && HIWORD( wparam ) == EN_CHANGE )
		{
			size_t len = SendDlgItemMessage(hwnd,IDC_MINDMSG,WM_GETTEXTLENGTH,0,0);
			TCHAR str[4];
			_sntprintf( str, 4, TEXT( "%d" ), FACEBOOK_MIND_LIMIT-len );
			SetDlgItemText(hwnd,IDC_CHARACTERS,str);

			EnableWindow(GetDlgItem( hwnd, IDOK ), len > 0);

			return TRUE;
		}
		else if ( LOWORD( wparam ) == IDOK )
		{
			TCHAR mindMessage[FACEBOOK_MIND_LIMIT+1];
			proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWLP_USERDATA));

			GetDlgItemText(hwnd,IDC_MINDMSG,mindMessage,SIZEOF(mindMessage));
			ShowWindow(hwnd,SW_HIDE);

			char *narrow = mir_utf8encodeT(mindMessage);
			if (proto->last_status_msg_ != narrow) proto->last_status_msg_ = narrow;
			utils::mem::detract(narrow);

			//char *narrow = mir_t2a_cp(mindMessage,CP_UTF8);
			ForkThread(&FacebookProto::SetAwayMsgWorker, proto, NULL);

			EndDialog(hwnd, wparam); 
			return TRUE;
		}
		else if ( LOWORD( wparam ) == IDCANCEL )
		{
			EndDialog(hwnd, wparam);
			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWLP_USERDATA));

	switch ( message )
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWLP_USERDATA,lparam);

		DBVARIANT dbv;
		if( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,&dbv) )
		{
			SetDlgItemTextA(hwnd,IDC_UN,dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		if ( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_PASS,&dbv) )
		{
			CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,reinterpret_cast<LPARAM>(dbv.pszVal));
			SetDlgItemTextA(hwnd,IDC_PW,dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		if (!proto->isOffline())
	    {
			SendMessage(GetDlgItem(hwnd,IDC_UN),EM_SETREADONLY,TRUE,0);
			SendMessage(GetDlgItem(hwnd,IDC_PW),EM_SETREADONLY,TRUE,0);
		}

		SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, FACEBOOK_GROUP_NAME_LIMIT, 0);

		if( !DBGetContactSettingTString(0,proto->ModuleName(),FACEBOOK_KEY_DEF_GROUP,&dbv) )
		{
			SetDlgItemText(hwnd,IDC_GROUP,dbv.ptszVal);
			DBFreeVariant(&dbv);
		}

		LoadDBCheckState(proto, hwnd, IDC_SET_IGNORE_STATUS, FACEBOOK_KEY_DISABLE_STATUS_NOTIFY, DEFAULT_DISABLE_STATUS_NOTIFY);
		LoadDBCheckState(proto, hwnd, IDC_BIGGER_AVATARS, FACEBOOK_KEY_BIG_AVATARS, DEFAULT_BIG_AVATARS);
		LoadDBCheckState(proto, hwnd, IDC_LOAD_MOBILE, FACEBOOK_KEY_LOAD_MOBILE, DEFAULT_LOAD_MOBILE);

	} return TRUE;

	case WM_COMMAND: {
		if ( LOWORD( wparam ) == IDC_NEWACCOUNTLINK )
		{
			CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>
				( FACEBOOK_URL_HOMEPAGE ) );
			return TRUE;
		}

		if ( LOWORD( wparam ) == IDC_SECURE ) {			
			EnableWindow(GetDlgItem(hwnd, IDC_SECURE_CHANNEL), IsDlgButtonChecked(hwnd, IDC_SECURE));
		}		
		
		if ((LOWORD(wparam)==IDC_UN || LOWORD(wparam)==IDC_PW || LOWORD(wparam)==IDC_GROUP) &&
		    (HIWORD(wparam)!=EN_CHANGE || (HWND)lparam!=GetFocus()))
			return 0;
		else
			SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);

	} break;

	case WM_NOTIFY:
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{
			char str[128]; TCHAR tstr[128];

			GetDlgItemTextA(hwnd,IDC_UN,str,sizeof(str));
			DBWriteContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,str);

			GetDlgItemTextA(hwnd,IDC_PW,str,sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING,sizeof(str),reinterpret_cast<LPARAM>(str));
			DBWriteContactSettingString(NULL,proto->m_szModuleName,FACEBOOK_KEY_PASS,str);

			GetDlgItemText(hwnd,IDC_GROUP,tstr,sizeof(tstr));
			if ( lstrlen( tstr ) > 0 )
			{
				DBWriteContactSettingTString(NULL,proto->m_szModuleName,FACEBOOK_KEY_DEF_GROUP,tstr);
				CallService( MS_CLIST_GROUPCREATE, 0, (LPARAM)tstr );
			}
			else
				DBDeleteContactSetting(NULL,proto->m_szModuleName,FACEBOOK_KEY_DEF_GROUP);

			StoreDBCheckState(proto, hwnd, IDC_SET_IGNORE_STATUS, FACEBOOK_KEY_DISABLE_STATUS_NOTIFY);
			StoreDBCheckState(proto, hwnd, IDC_BIGGER_AVATARS, FACEBOOK_KEY_BIG_AVATARS);
			StoreDBCheckState(proto, hwnd, IDC_LOAD_MOBILE, FACEBOOK_KEY_LOAD_MOBILE);
			
			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBOptionsAdvancedProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWLP_USERDATA));

	switch ( message )
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWLP_USERDATA,lparam);

		LoadDBCheckState(proto, hwnd, IDC_SECURE, FACEBOOK_KEY_FORCE_HTTPS, DEFAULT_FORCE_HTTPS);
		LoadDBCheckState(proto, hwnd, IDC_SECURE_CHANNEL, FACEBOOK_KEY_FORCE_HTTPS_CHANNEL, DEFAULT_FORCE_HTTPS_CHANNEL);
		LoadDBCheckState(proto, hwnd, IDC_DISCONNECT_CHAT, FACEBOOK_KEY_DISCONNECT_CHAT, DEFAULT_DISCONNECT_CHAT);
		LoadDBCheckState(proto, hwnd, IDC_PARSE_UNREAD, FACEBOOK_KEY_PARSE_MESSAGES, DEFAULT_PARSE_MESSAGES);
		LoadDBCheckState(proto, hwnd, IDC_CLOSE_WINDOWS, FACEBOOK_KEY_CLOSE_WINDOWS_ENABLE, DEFAULT_CLOSE_WINDOWS_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_SET_STATUS, FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS);
		LoadDBCheckState(proto, hwnd, IDC_LOGGING, FACEBOOK_KEY_LOGGING_ENABLE, DEFAULT_LOGGING_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_MAP_STATUSES, FACEBOOK_KEY_MAP_STATUSES, DEFAULT_MAP_STATUSES);
		LoadDBCheckState(proto, hwnd, IDC_GROUPCHATS, FACEBOOK_KEY_ENABLE_GROUPCHATS, DEFAULT_ENABLE_GROUPCHATS);

		EnableWindow(GetDlgItem(hwnd, IDC_SECURE_CHANNEL), IsDlgButtonChecked(hwnd, IDC_SECURE));

	} return TRUE;

	case WM_COMMAND: {
		if ( LOWORD( wparam ) == IDC_SECURE ) {			
			EnableWindow(GetDlgItem(hwnd, IDC_SECURE_CHANNEL), IsDlgButtonChecked(hwnd, IDC_SECURE));
		}		
		
		if (LOWORD(wparam) == IDC_SECURE_CHANNEL && IsDlgButtonChecked(hwnd, IDC_SECURE_CHANNEL))
			MessageBox( hwnd, TranslateT("Note: Make sure you have disabled 'Validate SSL certificates' option in Network options to work properly."), proto->m_tszUserName, MB_OK );

		SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);

	} break;

	case WM_NOTIFY:
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{
			StoreDBCheckState(proto, hwnd, IDC_SECURE, FACEBOOK_KEY_FORCE_HTTPS);
			StoreDBCheckState(proto, hwnd, IDC_CLOSE_WINDOWS, FACEBOOK_KEY_CLOSE_WINDOWS_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_LOGGING, FACEBOOK_KEY_LOGGING_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_SECURE_CHANNEL, FACEBOOK_KEY_FORCE_HTTPS_CHANNEL);
			StoreDBCheckState(proto, hwnd, IDC_DISCONNECT_CHAT, FACEBOOK_KEY_DISCONNECT_CHAT);
			StoreDBCheckState(proto, hwnd, IDC_PARSE_UNREAD, FACEBOOK_KEY_PARSE_MESSAGES);
			StoreDBCheckState(proto, hwnd, IDC_MAP_STATUSES, FACEBOOK_KEY_MAP_STATUSES);
			StoreDBCheckState(proto, hwnd, IDC_GROUPCHATS, FACEBOOK_KEY_ENABLE_GROUPCHATS);
			
			BOOL setStatus = IsDlgButtonChecked(hwnd, IDC_SET_STATUS);
			BOOL setStatusOld = DBGetContactSettingByte(NULL, proto->m_szModuleName, FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS);
			if (setStatus != setStatusOld)
			{
				DBWriteContactSettingByte(NULL, proto->m_szModuleName, FACEBOOK_KEY_SET_MIRANDA_STATUS, setStatus);
				if (setStatus && proto->isOnline() && DBGetContactSettingByte(NULL, proto->m_szModuleName, FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS))
				{
					ForkThread(&FacebookProto::SetAwayMsgWorker, proto, NULL);
				}
			}

			return TRUE;
		}
		break;

	}

	return FALSE;
}


INT_PTR CALLBACK FBEventsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWLP_USERDATA));

	switch(message) 
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWLP_USERDATA,lparam);		

		for(size_t i=0; i<SIZEOF(feed_types); i++)
		{
			SendDlgItemMessageA(hwnd,IDC_FEED_TYPE,CB_INSERTSTRING,i,
				reinterpret_cast<LPARAM>(Translate(feed_types[i].name)));
		}
		SendDlgItemMessage(hwnd, IDC_FEED_TYPE, CB_SETCURSEL, DBGetContactSettingByte(NULL, proto->m_szModuleName, FACEBOOK_KEY_FEED_TYPE, 0), 0);
		LoadDBCheckState(proto, hwnd, IDC_SYSTRAY_NOTIFY, FACEBOOK_KEY_SYSTRAY_NOTIFY, DEFAULT_SYSTRAY_NOTIFY);

		LoadDBCheckState(proto, hwnd, IDC_NOTIFICATIONS_ENABLE, FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_FEEDS_ENABLE, FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_CLIENT_ENABLE, FACEBOOK_KEY_EVENT_CLIENT_ENABLE, DEFAULT_EVENT_CLIENT_ENABLE);
		LoadDBCheckState(proto, hwnd, IDC_OTHER_ENABLE, FACEBOOK_KEY_EVENT_OTHER_ENABLE, DEFAULT_EVENT_OTHER_ENABLE);

		SendDlgItemMessage(hwnd, IDC_COLBACK, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLBACK,DEFAULT_EVENT_COLBACK));
		SendDlgItemMessage(hwnd, IDC_COLTEXT, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLTEXT,DEFAULT_EVENT_COLTEXT));
		SetDlgItemInt(hwnd, IDC_TIMEOUT,DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_NOTIFICATIONS_TIMEOUT, 0),TRUE);
		SendDlgItemMessage(hwnd, IDC_COLBACK2, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_FEEDS_COLBACK,DEFAULT_EVENT_COLBACK));
		SendDlgItemMessage(hwnd, IDC_COLTEXT2, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_FEEDS_COLTEXT,DEFAULT_EVENT_COLTEXT));
		SetDlgItemInt(hwnd, IDC_TIMEOUT2,DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_FEEDS_TIMEOUT, 0),TRUE);
		SendDlgItemMessage(hwnd, IDC_COLBACK3, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_OTHER_COLBACK,DEFAULT_EVENT_COLBACK));
		SendDlgItemMessage(hwnd, IDC_COLTEXT3, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_OTHER_COLTEXT,DEFAULT_EVENT_COLTEXT));
		SetDlgItemInt(hwnd, IDC_TIMEOUT3,DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_OTHER_TIMEOUT, 0),TRUE);
		SendDlgItemMessage(hwnd, IDC_COLBACK4, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_CLIENT_COLBACK,DEFAULT_EVENT_COLBACK));
		SendDlgItemMessage(hwnd, IDC_COLTEXT4, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_CLIENT_COLTEXT,DEFAULT_EVENT_COLTEXT));
		SetDlgItemInt(hwnd, IDC_TIMEOUT4,DBGetContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_CLIENT_TIMEOUT, 0),TRUE);

		LoadDBCheckState(proto, hwnd, IDC_NOTIFICATIONS_DEFAULT, FACEBOOK_KEY_EVENT_NOTIFICATIONS_DEFAULT, 0);
		LoadDBCheckState(proto, hwnd, IDC_FEEDS_DEFAULT, FACEBOOK_KEY_EVENT_FEEDS_DEFAULT, 0);
		LoadDBCheckState(proto, hwnd, IDC_CLIENT_DEFAULT, FACEBOOK_KEY_EVENT_CLIENT_DEFAULT, 0);
		LoadDBCheckState(proto, hwnd, IDC_OTHER_DEFAULT, FACEBOOK_KEY_EVENT_OTHER_DEFAULT, 0);

		SendDlgItemMessage(hwnd, IDC_TIMEOUT, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hwnd, IDC_TIMEOUT_SPIN, UDM_SETRANGE32, -1, 100);
		SendDlgItemMessage(hwnd, IDC_TIMEOUT2, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hwnd, IDC_TIMEOUT_SPIN2, UDM_SETRANGE32, -1, 100);
		SendDlgItemMessage(hwnd, IDC_TIMEOUT3, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hwnd, IDC_TIMEOUT_SPIN3, UDM_SETRANGE32, -1, 100);
		SendDlgItemMessage(hwnd, IDC_TIMEOUT4, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hwnd, IDC_TIMEOUT_SPIN4, UDM_SETRANGE32, -1, 100);

	} return TRUE;

	case WM_COMMAND: {
		switch ( LOWORD( wparam ) )
		{
		case IDC_PREVIEW:
		{
			TCHAR protoName[255];
			lstrcpy( protoName, proto->m_tszUserName );
			proto->NotifyEvent( protoName, TranslateT("Sample event"), NULL, FACEBOOK_EVENT_CLIENT ); 
			proto->NotifyEvent( protoName, TranslateT("Sample request"), NULL, FACEBOOK_EVENT_OTHER ); 
			proto->NotifyEvent( protoName, TranslateT("Sample newsfeed"), NULL, FACEBOOK_EVENT_NEWSFEED ); 
			proto->NotifyEvent( protoName, TranslateT("Sample notification"), NULL, FACEBOOK_EVENT_NOTIFICATION ); 
		} break;

		case IDC_COLTEXT:
		case IDC_COLBACK:
		case IDC_COLTEXT2:
		case IDC_COLBACK2:
		case IDC_COLTEXT3:
		case IDC_COLBACK3:
		case IDC_COLTEXT4:
		case IDC_COLBACK4:
			SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
			// TODO: Required? There's a catching clause below
		}

		if ((LOWORD(wparam)==IDC_PREVIEW || (HWND)lparam!=GetFocus()))
			return 0;
		else
			SendMessage(GetParent(hwnd),PSM_CHANGED,0,0); }

		return TRUE;

	case WM_NOTIFY:
	{
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{			
			DBWriteContactSettingByte(NULL, proto->m_szModuleName, FACEBOOK_KEY_FEED_TYPE, SendDlgItemMessage(hwnd, IDC_FEED_TYPE, CB_GETCURSEL, 0, 0));

			StoreDBCheckState(proto, hwnd, IDC_SYSTRAY_NOTIFY, FACEBOOK_KEY_SYSTRAY_NOTIFY);

			StoreDBCheckState(proto, hwnd, IDC_NOTIFICATIONS_ENABLE, FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_FEEDS_ENABLE, FACEBOOK_KEY_EVENT_FEEDS_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_OTHER_ENABLE, FACEBOOK_KEY_EVENT_OTHER_ENABLE);
			StoreDBCheckState(proto, hwnd, IDC_CLIENT_ENABLE, FACEBOOK_KEY_EVENT_CLIENT_ENABLE);

			StoreDBCheckState(proto, hwnd, IDC_NOTIFICATIONS_DEFAULT, FACEBOOK_KEY_EVENT_NOTIFICATIONS_DEFAULT);
			StoreDBCheckState(proto, hwnd, IDC_FEEDS_DEFAULT, FACEBOOK_KEY_EVENT_FEEDS_DEFAULT);
			StoreDBCheckState(proto, hwnd, IDC_OTHER_DEFAULT, FACEBOOK_KEY_EVENT_OTHER_DEFAULT);
			StoreDBCheckState(proto, hwnd, IDC_CLIENT_DEFAULT, FACEBOOK_KEY_EVENT_CLIENT_DEFAULT);

			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLBACK, SendDlgItemMessage(hwnd,IDC_COLBACK,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLTEXT, SendDlgItemMessage(hwnd,IDC_COLTEXT,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_NOTIFICATIONS_TIMEOUT, GetDlgItemInt(hwnd,IDC_TIMEOUT,NULL,TRUE));
			
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_FEEDS_COLBACK, SendDlgItemMessage(hwnd,IDC_COLBACK2,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_FEEDS_COLTEXT, SendDlgItemMessage(hwnd,IDC_COLTEXT2,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_FEEDS_TIMEOUT, GetDlgItemInt(hwnd,IDC_TIMEOUT2,NULL,TRUE));

			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_OTHER_COLBACK, SendDlgItemMessage(hwnd,IDC_COLBACK3,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_OTHER_COLTEXT, SendDlgItemMessage(hwnd,IDC_COLTEXT3,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_OTHER_TIMEOUT, GetDlgItemInt(hwnd,IDC_TIMEOUT3,NULL,TRUE));

			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_CLIENT_COLBACK, SendDlgItemMessage(hwnd,IDC_COLBACK4,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_CLIENT_COLTEXT, SendDlgItemMessage(hwnd,IDC_COLTEXT4,CPM_GETCOLOUR,0,0));
			DBWriteContactSettingDword(NULL, proto->m_szModuleName, FACEBOOK_KEY_EVENT_CLIENT_TIMEOUT, GetDlgItemInt(hwnd,IDC_TIMEOUT4,NULL,TRUE));
		}
	}
	return TRUE;

	}

	return FALSE;
}
