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

INT_PTR CSkypeProto::GetEventText(WPARAM, LPARAM lParam)
{
	DBEVENTGETTEXT *pEvent = (DBEVENTGETTEXT *)lParam;

	INT_PTR nRetVal = 0;

	ptrA pszText;
	
	switch (pEvent->dbei->eventType)
	{
	case SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE:
		{
			CMStringA text; 
			JSONNode jMsg = JSONNode::parse((char*)pEvent->dbei->pBlob);
			if (jMsg)
			{
				text.AppendFormat(Translate("Original message:\n\t%s\n"), jMsg["original_message"]["text"].as_string());
				JSONNode &jEdits = jMsg["edits"];
				for (auto it = jEdits.begin(); it != jEdits.end(); ++it)
				{
					const JSONNode &jEdit = *it;

					time_t time = jEdit["time"].as_int();
					tm* _tm = localtime(&time);
					char szTime[MAX_PATH];
					strftime(szTime, 0, "%X %x", _tm);

					text.AppendFormat(Translate("Edited at %s:\n\t%s\n"), szTime, jEdit["text"].as_string());
				}
				
			}
			else 
			{
#ifdef _DEBUG
				text = (char*)pEvent->dbei->pBlob;
#else
				text = Translate("Invalid data!");
#endif
			}

			pszText = mir_utf8decodeA(text);
			break;
		}

	case SKYPE_DB_EVENT_TYPE_CALL_INFO:
		{
			CMStringA text;
			HXML xml = xmlParseString(ptrT(mir_utf8decodeT((char*)pEvent->dbei->pBlob)), 0, _T("partlist"));
			if (xml != NULL)
			{
				ptrA type(mir_t2a(xmlGetAttrValue(xml, _T("type"))));
				bool bType = (!mir_strcmpi(type, "started")) ? 1 : 0;

				for (int i = 0; i < xmlGetChildCount(xml); i++)
				{
					HXML xmlPart = xmlGetNthChild(xml, _T("part"), i);
					if (xmlPart != NULL)
					{
						HXML xmlName = xmlGetChildByPath(xmlPart, _T("name"), 0);
						if (xmlName != NULL)
						{
							text.AppendFormat(Translate("%s %s this call.\n"), _T2A(xmlGetText(xmlName)), bType ? Translate("enters") : Translate("leaves"));
							xmlDestroyNode(xmlName);
						}
						xmlDestroyNode(xmlPart);
					}
				}
				xmlDestroyNode(xml);
			}
			pszText = mir_strdup(text);
			break;
		}
	case SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO:
		{
			CMStringA text;
			HXML xml = xmlParseString(ptrT(mir_utf8decodeT((char*)pEvent->dbei->pBlob)), 0, _T("files"));
			if (xml != NULL)
			{
				for (int i = 0; i < xmlGetChildCount(xml); i++)
				{
					size_t fileSize = 0;
					HXML xmlNode = xmlGetNthChild(xml, _T("file"), i);
					if (xmlNode != NULL)
					{
						fileSize = _ttoi(ptrT((TCHAR*)xmlGetAttrValue(xmlNode, _T("size"))));
						ptrA fileName(mir_utf8encodeT(ptrT((TCHAR*)xmlGetText(xmlNode))));
						if (fileName != NULL)
						{
							CMStringA msg(FORMAT, Translate("File transfer:\n\tFile name: %s\n\tSize: %d bytes"), fileName, fileSize);
							text.AppendFormat("%s\n", msg);
						}

						xmlDestroyNode(xmlNode);
					}
				}
				xmlDestroyNode(xml);
			}
			pszText = mir_strdup(text);
			break;
		}
	case SKYPE_DB_EVENT_TYPE_URIOBJ:
		{
			CMStringA text;
			HXML xml = xmlParseString(ptrT(mir_utf8decodeT((char*)pEvent->dbei->pBlob)), 0, _T("URIObject"));
			if (xml != NULL)
			{
				text.Append(_T2A(xmlGetText(xml)));
				xmlDestroyNode(xml);
			}
			pszText = mir_strdup(text);
			break;

		}
	case SKYPE_DB_EVENT_TYPE_INCOMING_CALL:
		{
			pszText = Translate("Incoming call.");
			break;
		}
	default:
		{
			pszText = mir_strdup((char*)pEvent->dbei->pBlob);
		}
	}


	if (pEvent->datatype == DBVT_TCHAR)
	{
		TCHAR *pwszText = _A2T(pszText);
		nRetVal = (INT_PTR)mir_tstrdup(pwszText);
	}
	else if (pEvent->datatype == DBVT_ASCIIZ)
		nRetVal = (INT_PTR)mir_strdup(pszText);

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
			icon = IcoLib_GetIconByHandle(GetIconHandle("inc_call"));
			break;
		}
	case SKYPE_DB_EVENT_TYPE_ACTION:
		{
			icon = IcoLib_GetIconByHandle(GetIconHandle("me_action"));
			break;
		}
	default:
		{
			icon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
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
	dbEventType.iconService = MODULE "/GetEventIcon";
	dbEventType.textService = MODULE "/GetEventText";

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE;
	dbEventType.descr = Translate("Edited message");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_CALL_INFO;
	dbEventType.descr = Translate("Call information");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO;
	dbEventType.descr = Translate("File transfer information");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_URIOBJ;
	dbEventType.descr = Translate("URI object");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
	dbEventType.descr = Translate("Incoming call");
	dbEventType.flags |= DETF_NONOTIFY;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}

void CSkypeProto::InitPopups()
{
	TCHAR desc[256];
	char name[256];
	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;

	mir_sntprintf(desc, _T("%s %s"), m_tszUserName, TranslateT("Calls"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Call");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("inc_call"));
	ppc.colorBack = RGB(255, 255, 255);
	ppc.colorText = RGB(0, 0, 0);
	ppc.iSeconds = 30;
	ppc.PluginWindowProc = PopupDlgProcCall;
	m_hPopupClassCall = Popup_RegisterClass(&ppc);

	mir_sntprintf(desc, _T("%s %s"), m_tszUserName, TranslateT("Notifications"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Notification");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("notify"));
	ppc.colorBack = RGB(255, 255, 255);
	ppc.colorText = RGB(0, 0, 0);
	ppc.iSeconds = 5;
	m_hPopupClassNotify = Popup_RegisterClass(&ppc);

	mir_sntprintf(desc, _T("%s %s"), m_tszUserName, TranslateT("Errors"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("error"));
	ppc.colorBack = RGB(255, 255, 255);
	ppc.colorText = RGB(0, 0, 0);
	ppc.iSeconds = -1;
	m_hPopupClassNotify = Popup_RegisterClass(&ppc);
}

int CSkypeProto::ProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	debugLogA(__FUNCTION__);
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPENING)
		SetSrmmReadStatus(event->hContact);

	return 0;
}

//Timers

mir_cs timerLock;
mir_cs CSkypeProto::accountsLock;

void CSkypeProto::ProcessTimer()
{
	if (IsOnline())
	{
		PushRequest(new GetContactListRequest(m_szTokenSecret), &CSkypeProto::LoadContactList);
		SendPresence(false);
		if (!m_hTrouterThread)
			SendRequest(new CreateTrouterRequest(), &CSkypeProto::OnCreateTrouter);
	}
}

static VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	mir_cslock lck(CSkypeProto::accountsLock);
	for (int i = 0; i < Accounts.getCount(); i++)
	{
		Accounts[i]->ProcessTimer();
	}
}

void CSkypeProto::SkypeSetTimer(void*)
{
	mir_cslock lck(timerLock);
	if (!CSkypeProto::m_timer)
		CSkypeProto::m_timer = SetTimer(NULL, 0, 600000, TimerProc);
}

void CSkypeProto::SkypeUnsetTimer(void*)
{
	mir_cslock lck(timerLock);
	if (CSkypeProto::m_timer)
		KillTimer(NULL, CSkypeProto::m_timer);
	CSkypeProto::m_timer = 0;
}