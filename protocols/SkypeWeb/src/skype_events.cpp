/*
Copyright (c) 2015-17 Miranda NG project (https://miranda-ng.org)

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
#define INVALID_DATA Translate("SkypeWeb error: Invalid data!")

INT_PTR CSkypeProto::GetEventText(WPARAM pEvent, LPARAM datatype)
{
	DBEVENTINFO *dbei = (DBEVENTINFO*)pEvent;

	CMStringA szText;

	BOOL bUseBB = db_get_b(NULL, dbei->szModule, "UseBBCodes", 1);
	switch (dbei->eventType) {
	case SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE:
		{
			JSONNode jMsg = JSONNode::parse((char*)dbei->pBlob);
			if (jMsg) {
				JSONNode &jOriginalMsg = jMsg["original_message"];
				szText.AppendFormat(bUseBB ? Translate("[b]Original message:[/b]\n%s\n") : Translate("Original message:\n%s\n"), mir_utf8decodeA(jOriginalMsg["text"].as_string().c_str()));
				JSONNode &jEdits = jMsg["edits"];
				for (auto it = jEdits.begin(); it != jEdits.end(); ++it) {
					const JSONNode &jEdit = *it;

					time_t time = jEdit["time"].as_int();
					char szTime[MAX_PATH];
					strftime(szTime, sizeof(szTime), "%X %x", localtime(&time));

					szText.AppendFormat(bUseBB ? Translate("[b]Edited at %s:[/b]\n%s\n") : Translate("Edited at %s:\n%s\n"), szTime, mir_utf8decodeA(jEdit["text"].as_string().c_str()));
				}

			}
			else {
				szText = INVALID_DATA;
			}
			break;
		}

	case SKYPE_DB_EVENT_TYPE_CALL_INFO:
		{
			HXML xml = xmlParseString(ptrW(mir_utf8decodeW((char*)dbei->pBlob)), 0, L"partlist");
			if (xml != NULL) {
				ptrA type(mir_u2a(xmlGetAttrValue(xml, L"type")));
				bool bType = (!mir_strcmpi(type, "started")) ? 1 : 0;
				time_t callDuration = 0;

				for (int i = 0; i < xmlGetChildCount(xml); i++) {
					HXML xmlPart = xmlGetNthChild(xml, L"part", i);
					if (xmlPart != NULL) {
						HXML xmlDuration = xmlGetChildByPath(xmlPart, L"duration", 0);

						if (xmlDuration != NULL) {
							callDuration = _wtol(xmlGetText(xmlDuration));
							break;
						}
					}
				}

				if (bType) {
					szText = Translate("Call");
				}
				else {
					if (callDuration == 0) {
						szText = Translate("Call missed");
					}
					else {
						char szTime[100];
						strftime(szTime, sizeof(szTime), "%X", gmtime(&callDuration));
						szText.Format(Translate("Call ended (%s)"), szTime);
					}
				}
				xmlDestroyNode(xml);
			}
			else {
				szText = INVALID_DATA;
			}
			break;
		}
	case SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO:
		{
			HXML xml = xmlParseString(ptrW(mir_utf8decodeW((char*)dbei->pBlob)), 0, L"files");
			if (xml != NULL) {
				for (int i = 0; i < xmlGetChildCount(xml); i++) {
					LONGLONG fileSize = 0;
					HXML xmlNode = xmlGetNthChild(xml, L"file", i);
					if (xmlNode != NULL) {
						fileSize = _wtol(xmlGetAttrValue(xmlNode, L"size"));
						char *fileName = _T2A(xmlGetText(xmlNode));
						if (fileName != NULL) {
							szText.AppendFormat(Translate("File transfer:\n\tFile name: %s \n\tSize: %lld bytes \n"), fileName, fileSize);
						}

					}
				}
				xmlDestroyNode(xml);
			}
			else {
				szText = INVALID_DATA;
			}
		}
		break;

	case SKYPE_DB_EVENT_TYPE_FILE:
	case SKYPE_DB_EVENT_TYPE_MOJI:
	case SKYPE_DB_EVENT_TYPE_URIOBJ:
		{
			HXML xml = xmlParseString(ptrW(mir_utf8decodeW((char*)dbei->pBlob)), 0, L"URIObject");
			if (xml != NULL) {
				//szText.Append(_T2A(xmlGetText(xml)));
				HXML xmlA = xmlGetChildByPath(xml, L"a", 0);
				if (xmlA != NULL) {
					szText += T2Utf(xmlGetAttrValue(xmlA, L"href"));
				}
				xmlDestroyNode(xml);
			}
			else {
				szText = INVALID_DATA;
			}
		}
		break;

	case SKYPE_DB_EVENT_TYPE_INCOMING_CALL:
		szText = Translate("Incoming call");
		break;

	case SKYPE_DB_EVENT_TYPE_UNKNOWN:
		szText.Format(Translate("Unknown event, please send this text for developer: \"%s\""), mir_utf8decodeA((char*)dbei->pBlob));
		break;

	default:
		szText = ptrA(mir_utf8decodeA((char*)dbei->pBlob));
	}

	return (datatype == DBVT_WCHAR) ? (INT_PTR)mir_a2u(szText) : (INT_PTR)szText.Detach();
}

INT_PTR CSkypeProto::EventGetIcon(WPARAM flags, LPARAM pEvent)
{
	DBEVENTINFO *dbei = (DBEVENTINFO*)pEvent;
	HICON icon = NULL;

	switch (dbei->eventType) {
	case SKYPE_DB_EVENT_TYPE_CALL_INFO:
	case SKYPE_DB_EVENT_TYPE_INCOMING_CALL:
		icon = GetIcon(IDI_CALL);
		break;

	case SKYPE_DB_EVENT_TYPE_ACTION:
		icon = GetIcon(IDI_ACTION_ME);
		break;

	case SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO:
		icon = Skin_LoadIcon(SKINICON_EVENT_FILE);
		break;

	case SKYPE_DB_EVENT_TYPE_URIOBJ:
		icon = Skin_LoadIcon(SKINICON_EVENT_URL);
		break;

	case SKYPE_DB_EVENT_TYPE_UNKNOWN:
		icon = Skin_LoadIcon(SKINICON_WARNING);
		break;

	default:
		icon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		break;
	}

	return (INT_PTR)((flags & LR_SHARED) ? icon : CopyIcon(icon));
}
