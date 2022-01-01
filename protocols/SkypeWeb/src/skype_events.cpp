/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

INT_PTR CSkypeProto::GetEventText(WPARAM pEvent, LPARAM datatype)
{
	DBEVENTINFO *dbei = (DBEVENTINFO*)pEvent;

	CMStringA szText = Translate("SkypeWeb error: Invalid data!");

	BOOL bUseBB = db_get_b(0, dbei->szModule, "UseBBCodes", 1);
	switch (dbei->eventType) {
	case SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE:
		{
			JSONNode jMsg = JSONNode::parse((char*)dbei->pBlob);
			if (jMsg) {
				JSONNode &jOriginalMsg = jMsg["original_message"];
				szText.AppendFormat(bUseBB ? Translate("[b]Original message:[/b]\n%s\n") : Translate("Original message:\n%s\n"), mir_utf8decodeA(jOriginalMsg["text"].as_string().c_str()));
				JSONNode &jEdits = jMsg["edits"];
				for (auto &it : jEdits) {
					time_t time = it["time"].as_int();
					char szTime[MAX_PATH];
					strftime(szTime, sizeof(szTime), "%X %x", localtime(&time));

					szText.AppendFormat(bUseBB ? Translate("[b]Edited at %s:[/b]\n%s\n") : Translate("Edited at %s:\n%s\n"), szTime, mir_utf8decodeA(it["text"].as_string().c_str()));
				}
			}
		}
		break;

	case SKYPE_DB_EVENT_TYPE_CALL_INFO:
		{
			TiXmlDocument doc;
			if (0 != doc.Parse((char*)dbei->pBlob))
				break;

			if (auto *pRoot = doc.FirstChildElement("partlist")) {
				bool bType = pRoot->IntAttribute("started") ? 1 : 0;

				time_t callDuration = 0;
				for (auto *it : TiXmlFilter(pRoot, "part")) {
					auto *xmlDuration = it->FirstChildElement("duration");
					if (xmlDuration != nullptr) {
						callDuration = atoi(xmlDuration->GetText());
						break;
					}
				}

				if (bType)
					szText = Translate("Call");
				else if (callDuration == 0)
					szText = Translate("Call missed");
				else {
					char szTime[100];
					strftime(szTime, sizeof(szTime), "%X", gmtime(&callDuration));
					szText.Format(Translate("Call ended (%s)"), szTime);
				}
			}
		}
		break;

	case SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO:
		{
			TiXmlDocument doc;
			if (0 != doc.Parse((char*)dbei->pBlob))
				break;

			if (auto *pRoot = doc.FirstChildElement("files")) {
				for (auto *it : TiXmlFilter(pRoot, "file")) {
					LONGLONG fileSize = it->Int64Attribute("size");
					const char *fileName = it->GetText();
					if (fileName != nullptr)
						szText.AppendFormat(Translate("File transfer:\n\tFile name: %s \n\tSize: %lld bytes \n"), fileName, fileSize);
				}
			}
		}
		break;

	case SKYPE_DB_EVENT_TYPE_FILE:
	case SKYPE_DB_EVENT_TYPE_MOJI:
	case SKYPE_DB_EVENT_TYPE_URIOBJ:
		{
			TiXmlDocument doc;
			if (0 != doc.Parse((char*)dbei->pBlob))
				break;

			szText.Empty();
			if (auto *pRoot = doc.FirstChildElement("URIObject")) {
				if (auto *xmlA = pRoot->FirstChildElement("a"))
					szText += xmlA->Attribute("href");
				if (auto *xmlThumb = pRoot->Attribute("url_thumbnail")) {
					szText.AppendFormat("\r\n%s: %s", TranslateU("Preview"), xmlThumb);
					
					CMStringA szUrl(xmlThumb);
					int iCount = szUrl.Replace("/views/imgt1_anim", "/views/imgpsh_fullsize_anim");
					if (!iCount)
						iCount = szUrl.Replace("/views/imgt1", "/views/imgpsh_fullsize_anim");
					if (iCount)
						szText.AppendFormat("\r\n%s: %s", TranslateU("Full image"), szUrl.c_str());
				}
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
	HICON icon = nullptr;

	switch (dbei->eventType) {
	case SKYPE_DB_EVENT_TYPE_CALL_INFO:
	case SKYPE_DB_EVENT_TYPE_INCOMING_CALL:
		icon = g_plugin.getIcon(IDI_CALL);
		break;

	case SKYPE_DB_EVENT_TYPE_ACTION:
		icon = g_plugin.getIcon(IDI_ACTION_ME);
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
