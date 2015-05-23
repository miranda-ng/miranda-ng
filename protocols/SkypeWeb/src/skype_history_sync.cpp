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

/* HISTORY SYNC */

void CSkypeProto::OnGetServerHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *metadata = json_get(root, "_metadata");
	JSONNODE *conversations = json_as_array(json_get(root, "messages"));

	int totalCount = json_as_int(json_get(metadata, "totalCount"));
	ptrA syncState(mir_t2a(ptrT(json_as_string(json_get(metadata, "syncState")))));

	bool markAllAsUnread = getBool("MarkMesUnread", false);

	if (totalCount >= 99 || json_size(conversations) >= 99)
		PushRequest(new GetHistoryOnUrlRequest(syncState, RegToken), &CSkypeProto::OnGetServerHistory);

	for (int i = json_size(conversations); i >= 0; i--)
	{
		JSONNODE *message = json_at(conversations, i);

		ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(message, "clientmessageid")))));
		ptrA skypeEditedId(mir_t2a(ptrT(json_as_string(json_get(message, "skypeeditedid")))));
		ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(message, "messagetype")))));
		ptrA from(mir_t2a(ptrT(json_as_string(json_get(message, "from")))));
		ptrA content(mir_t2a(ptrT(json_as_string(json_get(message, "content")))));
		ptrT composeTime(json_as_string(json_get(message, "composetime")));
		ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(message, "conversationLink")))));
		int emoteOffset = atoi(ptrA(mir_t2a(ptrT(json_as_string(json_get(message, "skypeemoteoffset"))))));
		time_t timestamp = IsoToUnixTime(composeTime);
		ptrA skypename(ContactUrlToName(from));

		bool isEdited = (json_get(message, "skypeeditedid") != NULL);

		MCONTACT hContact = FindContact(ptrA(ContactUrlToName(conversationLink)));


		if (timestamp > db_get_dw(hContact, m_szModuleName, "LastMsgTime", 0))
			db_set_dw(hContact, m_szModuleName, "LastMsgTime", (DWORD)timestamp);

		int flags = DBEF_UTF;

		if (!markAllAsUnread)
			flags |= DBEF_READ;

		if (IsMe(skypename))
			flags |= DBEF_SENT;

		if (conversationLink != NULL && strstr(conversationLink, "/8:"))
		{
			if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
			{

				ptrA message(RemoveHtml(content));
				MEVENT dbevent = GetMessageFromDb(hContact, skypeEditedId);

				if (isEdited && dbevent != NULL)
				{
					DBEVENTINFO dbei = { sizeof(dbei) };
					CMStringA msg;

					dbei.cbBlob = db_event_getBlobSize(dbevent);
					mir_ptr<BYTE> blob((PBYTE)mir_alloc(dbei.cbBlob));
					dbei.pBlob = blob;

					db_event_get(dbevent, &dbei);
					time_t dbEventTimestamp = dbei.timestamp;
					ptrA dbMsgText((char *)mir_alloc(dbei.cbBlob));

					mir_strcpy(dbMsgText, (char*)dbei.pBlob);

					TCHAR time[64];
					_locale_t locale = _create_locale(LC_ALL, "");
					_tcsftime_l(time, sizeof(time), L"%X %x", localtime(&timestamp), locale);
					_free_locale(locale);

					msg.AppendFormat("%s\n%s %s:\n%s", dbMsgText, Translate("Edited at"), _T2A(time), message);
					db_event_delete(hContact, dbevent);
					AddMessageToDb(hContact, dbEventTimestamp, flags, clientMsgId, msg.GetBuffer(), emoteOffset);
				}
				else
					AddMessageToDb(hContact, timestamp, flags, clientMsgId, message, emoteOffset);
			}
			else if (!mir_strcmpi(messageType, "Event/Call"))
			{
				//content=<partlist type="ended" alt=""><part identity="username"><name>user name</name><duration>6</duration></part>
				//<part identity="echo123"><name>Echo / Sound Test Service</name><duration>6</duration></part></partlist>

				//content=<partlist type="started" alt=""><part identity="username"><name>user name</name></part></partlist>
				int iType = 3, iDuration = 0;
				ptrA skypename(ContactUrlToName(from));
				HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("partlist"));
				if (xml != NULL)
				{

					ptrA type(mir_t2a(xi.getAttrValue(xml, _T("type"))));

					if (!mir_strcmpi(type, "ended")) iType = 0;
					else if (!mir_strcmpi(type, "started")) iType = 1;
					HXML xmlNode = xi.getChildByPath(xml, _T("part"), 0);
					HXML duration = xmlNode == NULL ? NULL : xi.getChildByPath(xmlNode, _T("duration"), 0);
					iDuration = duration != NULL ? atoi(mir_t2a(xi.getText(duration))) : NULL;

					xi.destroyNode(xml);
				}
				CMStringA text = "";
				if (iType == 1)
					text.Append(Translate("Call started"));
				else if (iType == 0)
				{
					CMStringA chours = "", cmins = "", csec = "";
					int hours = 0, mins = 0, sec = 0;
					if (iDuration != NULL)
					{
						hours = iDuration / 3600;
						mins = iDuration / 60;
						sec = iDuration % 60;
					}
					else
						hours = mins = sec = 0;

					chours.AppendFormat(hours < 10 ? "0%d" : "%d", hours);
					cmins.AppendFormat(mins < 10 ? "0%d" : "%d", mins);
					csec.AppendFormat(sec < 10 ? "0%d" : "%d", sec);
					text.AppendFormat("%s\n%s: %s:%s:%s", Translate("Call ended"), Translate("Duration"), chours, cmins, csec);
				}
				AddCallInfoToDb(hContact, timestamp, flags, clientMsgId, text.GetBuffer());
			}
			else if (!mir_strcmpi(messageType, "RichText/Files"))
			{
				//content=<files alt="отправил (-а) файл &quot;run.bat&quot;"><file size="97" index="0" tid="4197760077">run.bat</file></files>
				HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("files"));
				if (xml != NULL)
				{
					for (int i = 0; i < xi.getChildCount(xml); i++)
					{
						int fileSize; CMStringA msg = "";
						HXML xmlNode = xi.getNthChild(xml, L"file", i);
						if (xmlNode == NULL)
							break;
						fileSize = atoi(_T2A(xi.getAttrValue(xmlNode, L"size")));
						ptrA fileName(mir_t2a(xi.getText(xmlNode)));
						if (fileName == NULL || fileSize == NULL)
							continue;

						msg.Empty();
						msg.AppendFormat("%s:\n\t%s: %s\n\t%s: %d %s", Translate("File transfer"), Translate("File name"), fileName, Translate("Size"), fileSize, Translate("bytes"));
						AddMessageToDb(hContact, timestamp, flags, clientMsgId, msg.GetBuffer());

					}
				}
			}
			else if (!mir_strcmpi(messageType, "RichText/UriObject"))
			{
				//content=<URIObject type="Picture.1" uri="https://api.asm.skype.com/v1//objects/0-weu-d1-262f0a1ee256d03b8e4b8360d9208834" url_thumbnail="https://api.asm.skype.com/v1//objects/0-weu-d1-262f0a1ee256d03b8e4b8360d9208834/views/imgt1"><Title></Title><Description></Description>Для просмотра этого общего фото перейдите по ссылке: https://api.asm.skype.com/s/i?0-weu-d1-262f0a1ee256d03b8e4b8360d9208834<meta type="photo" originalName="ysd7ZE4BqOg.jpg"/><OriginalName v="ysd7ZE4BqOg.jpg"/></URIObject>
				HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("URIObject"));
				if (xml != NULL)
				{
					ptrA url(mir_t2a(xi.getAttrValue(xml, L"uri")));
					ptrA object(ParseUrl(url, "/objects/"));

					CMStringA data(FORMAT, "%s: https://api.asm.skype.com/s/i?%s", Translate("Image"), object);

					AddMessageToDb(hContact, timestamp, flags, clientMsgId, data.GetBuffer());
				}
			} //Picture
		}
		else if (conversationLink != NULL && strstr(conversationLink, "/19:"))
		{
			ptrA chatname(ChatUrlToName(conversationLink));
			if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
			{
				AddMessageToChat(_A2T(chatname), _A2T(skypename), content, emoteOffset != NULL, emoteOffset, timestamp, true);
			}
		}
	}
}

INT_PTR CSkypeProto::GetContactHistory(WPARAM hContact, LPARAM)
{
	PushRequest(new GetHistoryRequest(RegToken, ptrA(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID)), 100, false, 0, Server), &CSkypeProto::OnGetServerHistory);
	return 0;
}

void CSkypeProto::OnSyncHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;
	JSONROOT root(response->pData);

	if (root == NULL)
		return;

	JSONNODE *metadata = json_get(root, "_metadata");
	JSONNODE *conversations = json_as_array(json_get(root, "conversations"));

	int totalCount = json_as_int(json_get(metadata, "totalCount"));
	ptrA syncState(mir_t2a(ptrT(json_as_string(json_get(metadata, "syncState")))));

	if (totalCount >= 99 || json_size(conversations) >= 99)
		PushRequest(new SyncHistoryFirstRequest(syncState, RegToken), &CSkypeProto::OnSyncHistory);

	for (size_t i = 0; i < json_size(conversations); i++)
	{
		JSONNODE *conversation = json_at(conversations, i);
		JSONNODE *lastMessage = json_get(conversation, "lastMessage");
		if (json_empty(lastMessage))
			continue;

		char *conversationLink = mir_t2a(json_as_string(json_get(lastMessage, "conversationLink")));
		time_t composeTime(IsoToUnixTime(ptrT(json_as_string(json_get(lastMessage, "composetime")))));

		ptrA skypename;

		if (conversationLink != NULL && strstr(conversationLink, "/8:"))
		{
			skypename = ContactUrlToName(conversationLink);
			MCONTACT hContact = FindContact(skypename);
			if (hContact == NULL)
				continue;

			if (db_get_dw(hContact, m_szModuleName, "LastMsgTime", 0) < composeTime)
			{
				PushRequest(new GetHistoryRequest(RegToken, skypename, 100, false, 0, Server), &CSkypeProto::OnGetServerHistory);
				HistorySynced = true;
			}
		}
	}
}