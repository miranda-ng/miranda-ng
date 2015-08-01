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
#define INVALID_DATA Translate("Invalid data!")

INT_PTR CSkypeProto::GetEventText(WPARAM, LPARAM lParam)
{
	DBEVENTGETTEXT *pEvent = (DBEVENTGETTEXT *)lParam;

	CMStringA szText; 

	switch (pEvent->dbei->eventType)
	{
	case SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE:
		{
			
			JSONNode jMsg = JSONNode::parse((char*)pEvent->dbei->pBlob);
			if (jMsg)
			{
				JSONNode &jOriginalMsg = jMsg["original_message"];
				szText.AppendFormat(Translate("Original message:\n\t%s\n"), mir_utf8decodeA(jOriginalMsg["text"].as_string().c_str()));
				JSONNode &jEdits = jMsg["edits"];
				for (auto it = jEdits.begin(); it != jEdits.end(); ++it)
				{
					const JSONNode &jEdit = *it;

					time_t time = jEdit["time"].as_int();
					char szTime[MAX_PATH];
					strftime(szTime, sizeof(szTime), "%X %x", localtime(&time));

					szText.AppendFormat(Translate("Edited at %s:\n\t%s\n"), szTime, mir_utf8decodeA(jEdit["text"].as_string().c_str()));
				}
				
			}
			else 
			{
				szText = INVALID_DATA;
			}
			break;
		}

	case SKYPE_DB_EVENT_TYPE_CALL_INFO:
		{
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
						HXML xmlDuration = xmlGetChildByPath(xmlPart, _T("duration"), 0);
						time_t callDuration = 0;
						if (xmlDuration != NULL)
						{
							callDuration = _ttol(ptrT((TCHAR*)xmlGetText(xmlDuration)));
							xmlDestroyNode(xmlDuration);
						}
						if (xmlName != NULL)
						{
							char szTime[MAX_PATH];
							strftime(szTime, sizeof(szTime), "%X", gmtime(&callDuration));

							szText.AppendFormat(Translate("%s %s this call (%s).\n"), _T2A(xmlGetText(xmlName)), bType ? Translate("enters") : Translate("leaves"), szTime);
							xmlDestroyNode(xmlName);
						}
						xmlDestroyNode(xmlPart);
					}
				}
				xmlDestroyNode(xml);
			}
			else
			{
				szText = INVALID_DATA;
			}
			break;
		}
	case SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO:
		{
			HXML xml = xmlParseString(ptrT(mir_utf8decodeT((char*)pEvent->dbei->pBlob)), 0, _T("files"));
			if (xml != NULL)
			{
				for (int i = 0; i < xmlGetChildCount(xml); i++)
				{
					long fileSize = 0;
					HXML xmlNode = xmlGetNthChild(xml, _T("file"), i);
					if (xmlNode != NULL)
					{
						fileSize = _ttol(ptrT((TCHAR*)xmlGetAttrValue(xmlNode, _T("size"))));
						ptrA fileName(mir_utf8encodeT(ptrT((TCHAR*)xmlGetText(xmlNode))));
						if (fileName != NULL)
						{
							CMStringA msg(FORMAT, Translate("File transfer:\n\tFile name: %s\n\tSize: %d bytes"), fileName, fileSize);
							szText.AppendFormat("%s\n", msg);
						}

						xmlDestroyNode(xmlNode);
					}
				}
				xmlDestroyNode(xml);
			}
			else
			{
				szText = INVALID_DATA;
			}
			break;
		}
	case SKYPE_DB_EVENT_TYPE_URIOBJ:
		{
			HXML xml = xmlParseString(ptrT(mir_utf8decodeT((char*)pEvent->dbei->pBlob)), 0, _T("URIObject"));
			if (xml != NULL)
			{
				szText.Append(_T2A(xmlGetText(xml)));
				xmlDestroyNode(xml);
			}
			else 
			{
				szText = INVALID_DATA;
			}
			break;

		}
	case SKYPE_DB_EVENT_TYPE_INCOMING_CALL:
		{
			szText = Translate("Incoming call");
			break;
		}
	case SKYPE_DB_EVENT_TYPE_UNKNOWN:
		{
			szText.Format(Translate("Unknown event, please send this text for developer: \"%s\""), mir_utf8decodeA((char*)pEvent->dbei->pBlob));
			break;
		}
	default:
		{
			szText = (char*)pEvent->dbei->pBlob;
		}
	}

	switch(pEvent->datatype)
	{
	case DBVT_TCHAR:
		{
			return (INT_PTR)mir_a2t(szText);
		}
	case DBVT_ASCIIZ:
		{
			return (INT_PTR)szText.Detach();
		}
	case DBVT_UTF8:
		{
			return (INT_PTR)mir_utf8encode(szText);
		}
	default:
		{
			return NULL;
		}
	}
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
	case SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO:
		{
			icon = Skin_LoadIcon(SKINICON_EVENT_FILE);
			break;
		}
	case SKYPE_DB_EVENT_TYPE_URIOBJ:
		{
			icon = Skin_LoadIcon(SKINICON_EVENT_URL);
			break;
		}
	case SKYPE_DB_EVENT_TYPE_UNKNOWN:
		{
			icon = Skin_LoadIcon(SKINICON_WARNING);
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
	db_set_resident(m_szModuleName, "LastAuthRequestTime");

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

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_UNKNOWN;
	dbEventType.descr = Translate("Unknown event");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
	dbEventType.descr = Translate("Incoming call");
	dbEventType.flags |= DETF_NONOTIFY;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}

int CSkypeProto::ProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	debugLogA(__FUNCTION__);
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPENING)
		SetSrmmReadStatus(event->hContact);

	return 0;
}
