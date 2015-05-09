/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

INT_PTR CSkypeProto::GetCallEventText(WPARAM, LPARAM lParam)
{
	DBEVENTGETTEXT *pEvent = (DBEVENTGETTEXT *)lParam;

	INT_PTR nRetVal = 0;
	char *pszText = Translate("Incoming call");

	if (pEvent->datatype == DBVT_TCHAR) 
	{
		TCHAR *pwszText = _A2T(pszText);
		nRetVal = (INT_PTR)mir_tstrdup(pwszText);
	}

	else if (pEvent->datatype == DBVT_ASCIIZ)
		nRetVal = (INT_PTR)mir_strdup(Translate(pszText));

	return nRetVal;
}

INT_PTR CSkypeProto::EventGetIcon(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO* dbei = (DBEVENTINFO*)lParam;
	HICON icon = NULL;
	
	switch (dbei->eventType)
	{
	case SKYPE_DB_EVENT_TYPE_CALL_INFO:
	case SKYPE_DB_EVENT_TYPE_INCOMING_CALL:
		{
			icon = Skin_GetIconByHandle(GetIconHandle("inc_call"));
			break;
		}
	case SKYPE_DB_EVENT_TYPE_ACTION:
		{
			icon = Skin_GetIconByHandle(GetIconHandle("me_action"));
			break;
		}
	default:
		{
			icon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			break;
		}
	}
	
	return (INT_PTR)((wParam & LR_SHARED) ? icon : CopyIcon(icon));
}

void CSkypeProto::InitDBEvents()
{
	// custom event
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;
	dbEventType.iconService = MODULE"/GetEventIcon";

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_CALL_INFO;
	dbEventType.descr = Translate("Call information.");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
	dbEventType.descr = Translate("Incoming call");
	dbEventType.textService = MODULE"/GetCallText";
	dbEventType.flags |= DETF_NONOTIFY;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}

void CSkypeProto::InitPopups()
{
	TCHAR desc[256];
	char name[256];
	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;

	mir_sntprintf(desc, SIZEOF(desc), _T("%s %s"), m_tszUserName, TranslateT("Calls"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Call");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle("inc_call"));
	ppc.colorBack = RGB(255, 255, 255); 
	ppc.colorText = RGB(0, 0, 0); 
	ppc.iSeconds = 30;
	ppc.PluginWindowProc = PopupDlgProcCall;
	m_hPopupClassCall = Popup_RegisterClass(&ppc);

	mir_sntprintf(desc, SIZEOF(desc), _T("%s %s"), m_tszUserName, TranslateT("Notifications"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Notification");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle("notify"));
	ppc.colorBack = RGB(255, 255, 255); 
	ppc.colorText = RGB(0, 0, 0); 
	ppc.iSeconds = 5;
	m_hPopupClassNotify = Popup_RegisterClass(&ppc);

	mir_sntprintf(desc, SIZEOF(desc), _T("%s %s"), m_tszUserName, TranslateT("Errors"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Error");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle("error"));
	ppc.colorBack = RGB(255, 255, 255); 
	ppc.colorText = RGB(0, 0, 0); 
	ppc.iSeconds = -1;
	m_hPopupClassNotify = Popup_RegisterClass(&ppc);
}

/*int CSkypeProto::ProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	debugLogA(__FUNCTION__);
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPENING)
		SetSrmmReadStatus(event->hContact);

	return 0;
}*/