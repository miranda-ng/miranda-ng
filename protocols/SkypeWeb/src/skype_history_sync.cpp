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

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["messages"].as_array();

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	bool markAllAsUnread = getBool("MarkMesUnread", false);

	if (totalCount >= 99 || conversations.size() >= 99)
		PushRequest(new GetHistoryOnUrlRequest(syncState.c_str(), RegToken), &CSkypeProto::OnGetServerHistory);

	for (int i = (int)conversations.size(); i >= 0; i--)
	{
		const JSONNode &message = conversations.at(i);

		std::string clientMsgId = message["clientmessageid"].as_string();
		std::string skypeEditedId = message["skypeeditedid"].as_string();
		std::string messageType = message["messagetype"].as_string();
		std::string from = message["from"].as_string();
		std::string content = message["content"].as_string();
		std::string conversationLink = message["conversationLink"].as_string();
		int emoteOffset = message["skypeemoteoffset"].as_int();
		time_t timestamp = IsoToUnixTime(message["composetime"].as_string().c_str());
		ptrA skypename(ContactUrlToName(from.c_str()));

		bool isEdited = message["skypeeditedid"];

		MCONTACT hContact = FindContact(ptrA(ContactUrlToName(conversationLink.c_str())));
			  
		if (timestamp > db_get_dw(hContact, m_szModuleName, "LastMsgTime", 0))
			db_set_dw(hContact, m_szModuleName, "LastMsgTime", (DWORD)timestamp);

		int flags = DBEF_UTF;

		if (!markAllAsUnread)
			flags |= DBEF_READ;

		if (IsMe(skypename))
			flags |= DBEF_SENT;

		if (strstr(conversationLink.c_str(), "/8:"))
		{
			if (!mir_strcmpi(messageType.c_str(), "Text") || !mir_strcmpi(messageType.c_str(), "RichText"))
			{
				ptrA message(RemoveHtml(content.c_str()));
				MEVENT dbevent = GetMessageFromDb(hContact, skypeEditedId.c_str());

				if (isEdited && dbevent != NULL)
				{
					DBEVENTINFO dbei = { sizeof(dbei) };
					CMStringA msg;

					dbei.cbBlob = db_event_getBlobSize(dbevent);
					mir_ptr<BYTE> blob((PBYTE)mir_alloc(dbei.cbBlob));
					dbei.pBlob = blob;

					db_event_get(dbevent, &dbei);
					time_t dbEventTimestamp = dbei.timestamp;

					ptrA dbMsgText(NEWSTR_ALLOCA((char *)dbei.pBlob));

					TCHAR time[64];
					_locale_t locale = _create_locale(LC_ALL, "");
					_tcsftime_l(time, sizeof(time), L"%X %x", localtime(&timestamp), locale);
					_free_locale(locale);

					msg.AppendFormat("%s\n%s %s:\n%s", dbMsgText, Translate("Edited at"), _T2A(time), message);
					db_event_delete(hContact, dbevent);
					AddMessageToDb(hContact, dbEventTimestamp, flags, clientMsgId.c_str(), msg.GetBuffer(), emoteOffset);
				}
				else AddMessageToDb(hContact, timestamp, flags, clientMsgId.c_str(), message, emoteOffset);
			}
			else if (!mir_strcmpi(messageType.c_str(), "Event/Call"))
			{
				//content=<partlist type="ended" alt=""><part identity="username"><name>user name</name><duration>6</duration></part>
				//<part identity="echo123"><name>Echo / Sound Test Service</name><duration>6</duration></part></partlist>

				//content=<partlist type="started" alt=""><part identity="username"><name>user name</name></part></partlist>
				int iType = 3, iDuration = 0;
				ptrA skypename(ContactUrlToName(from.c_str()));
				HXML xml = xi.parseString(ptrT(mir_a2t(content.c_str())), 0, _T("partlist"));
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
				AddCallInfoToDb(hContact, timestamp, flags, clientMsgId.c_str(), text.GetBuffer());
			}
			else if (!mir_strcmpi(messageType.c_str(), "RichText/Files"))
			{
				//content=<files alt="отправил (-а) файл &quot;run.bat&quot;"><file size="97" index="0" tid="4197760077">run.bat</file></files>
				HXML xml = xi.parseString(ptrT(mir_a2t(content.c_str())), 0, _T("files"));
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
						AddMessageToDb(hContact, timestamp, flags, clientMsgId.c_str(), msg.GetBuffer());
					}
				}
			}
			else if (!mir_strcmpi(messageType.c_str(), "RichText/UriObject"))
			{
				//content=<URIObject type="Picture.1" uri="https://api.asm.skype.com/v1//objects/0-weu-d1-262f0a1ee256d03b8e4b8360d9208834" url_thumbnail="https://api.asm.skype.com/v1//objects/0-weu-d1-262f0a1ee256d03b8e4b8360d9208834/views/imgt1"><Title></Title><Description></Description>Для просмотра этого общего фото перейдите по ссылке: https://api.asm.skype.com/s/i?0-weu-d1-262f0a1ee256d03b8e4b8360d9208834<meta type="photo" originalName="ysd7ZE4BqOg.jpg"/><OriginalName v="ysd7ZE4BqOg.jpg"/></URIObject>
				HXML xml = xi.parseString(ptrT(mir_a2t(content.c_str())), 0, _T("URIObject"));
				if (xml != NULL)
				{
					ptrA url(mir_t2a(xi.getAttrValue(xml, L"uri")));
					ptrA object(ParseUrl(url, "/objects/"));

					CMStringA data(FORMAT, "%s: https://api.asm.skype.com/s/i?%s", Translate("Image"), object);

					AddMessageToDb(hContact, timestamp, flags, clientMsgId.c_str(), data.GetBuffer());
				}
			} //Picture
		}
		else if (conversationLink.find("/19:") != -1)
		{
			ptrA chatname(ChatUrlToName(conversationLink.c_str()));
			if (!mir_strcmpi(messageType.c_str(), "Text") || !mir_strcmpi(messageType.c_str(), "RichText"))
				AddMessageToChat(_A2T(chatname), _A2T(skypename), content.c_str(), emoteOffset != NULL, emoteOffset, timestamp, true);
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

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["conversations"].as_array();

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	if (totalCount >= 99 || conversations.size() >= 99)
		PushRequest(new SyncHistoryFirstRequest(syncState.c_str(), RegToken), &CSkypeProto::OnSyncHistory);

	for (size_t i = 0; i < conversations.size(); i++)
	{
		const JSONNode &conversation = conversations.at(i);
		const JSONNode &lastMessage = conversation["lastMessage"];
		if (!lastMessage)
			continue;

		std::string conversationLink = lastMessage["conversationLink"].as_string();
		time_t composeTime(IsoToUnixTime(lastMessage["composetime"].as_string().c_str()));

		if (conversationLink.find("/8:") != -1)
		{
			ptrA skypename(ContactUrlToName(conversationLink.c_str()));
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