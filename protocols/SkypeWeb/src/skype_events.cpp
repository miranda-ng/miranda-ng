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

INT_PTR CSkypeProto::EventGetIcon(WPARAM wParam, LPARAM lParam) // it not work , ????
{
	DBEVENTINFO* dbei = (DBEVENTINFO*)lParam;
	HICON icon = NULL;
	
	switch (dbei->eventType)
	{
	case SKYPE_DB_EVENT_TYPE_INCOMING_CALL:
		{
			icon = Skin_GetIconByHandle(GetIconHandle("inc_call"));
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

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
	dbEventType.descr = Translate("Incoming call");
	dbEventType.iconService = MODULE"/GetEventIcon";
	dbEventType.textService = MODULE"/GetCallText";
	dbEventType.flags |= DETF_NONOTIFY;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}

INT_PTR CSkypeProto::GetCallEventText(WPARAM, LPARAM lParam)
{
	DBEVENTGETTEXT *pEvent = (DBEVENTGETTEXT *)lParam;

	INT_PTR nRetVal = 0;
	char *pszText = Translate("Incoming call");


	if (pEvent->datatype == DBVT_TCHAR) 
	{
		TCHAR *pwszText = _A2T(pszText);
		nRetVal = (INT_PTR)mir_wstrdup(pwszText);
	}

	else if (pEvent->datatype == DBVT_ASCIIZ)
		nRetVal = (INT_PTR)mir_strdup(Translate(pszText));

	return nRetVal;
}