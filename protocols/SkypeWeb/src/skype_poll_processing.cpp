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

void CSkypeProto::ProcessEndpointPresenceRes(const JSONNode &node)
{
	debugLogA("CSkypeProto::ProcessEndpointPresenceRes");
	std::string selfLink = node["selfLink"].as_string();
	ptrA skypename(ContactUrlToName(selfLink.c_str()));
	if (skypename == NULL)
		return;

	MCONTACT hContact = FindContact(skypename);
	if (hContact == NULL)
		return;

	//"publicInfo":{"capabilities":"","typ":"11","skypeNameVersion":"0/7.1.0.105//","nodeInfo":"","version":"24"}
	//"privateInfo": {"epname": "Skype"}
	const JSONNode &publicInfo = node["publicInfo"];
	const JSONNode &privateInfo = node["privateInfo"];
	CMStringA MirVer = "";
	if (publicInfo)
	{
		std::string skypeNameVersion = publicInfo["skypeNameVersion"].as_string();
		std::string version = publicInfo["version"].as_string();
		std::string typ = publicInfo["typ"].as_string();
		if (!typ.empty())
		{
			int iTyp = atoi(typ.c_str());
			switch (iTyp)
			{
			case 17:
				MirVer.Append("Skype (Android)");
				break;
			case 16:
				MirVer.Append("Skype (iOS)");
				break;
			case 12:
				MirVer.Append("Skype (WinRT)");
				break;
			case 15:
				MirVer.Append("Skype (WP)");
				break;
			case 13:
				MirVer.Append("Skype (OSX)");
				break;
			case 11:
				MirVer.Append("Skype (Windows)");
				break;
			case 14:
				MirVer.Append("Skype (Linux)");
				break;
			case 10:
				MirVer.Append("Skype (XBOX)");
				break;
			case 1:
				MirVer.Append("Skype (Web)");
				break;
			case 125:
				MirVer.Append("Miranda NG Skype");
				break;
			default:
				{
					if (!mir_strcmpi(typ.c_str(), "website"))
						MirVer.Append("Skype (Outlook)");
					else
						MirVer.Append("Skype (Unknown)");
					break;
				}
			}
			if (iTyp == 125)
				MirVer.AppendFormat(" %s", version);
			else
				MirVer.AppendFormat(" %s", ParseUrl(skypeNameVersion.c_str(), "/"));	
		}
	}
	if (privateInfo != NULL)
	{
		std::string epname = privateInfo["epname"].as_string();
		if (!epname.empty())
		{
			MirVer.AppendFormat(" [%s]", epname.c_str());
		}
	}
	db_set_s(hContact, m_szModuleName, "MirVer", MirVer);
}

void CSkypeProto::ProcessUserPresenceRes(const JSONNode &node)
{
	debugLogA("CSkypeProto::ProcessUserPresenceRes");

	std::string selfLink = node["selfLink"].as_string();
	std::string status = node["status"].as_string();
	ptrA skypename;

	if (selfLink.find("/8:") != std::string::npos)
	{
		skypename = ContactUrlToName(selfLink.c_str());
	}
	else if (selfLink.find("/1:") != std::string::npos)
	{
		skypename = SelfUrlToName(selfLink.c_str());
	}

	if (skypename != NULL)
	{
		if (IsMe(skypename))
		{
			int iNewStatus = SkypeToMirandaStatus(status.c_str());
			int old_status = m_iStatus;
			m_iDesiredStatus = iNewStatus;
			m_iStatus = iNewStatus;
			if (old_status != iNewStatus)
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, iNewStatus);
			return;
		}
		else
		{
			MCONTACT hContact = FindContact(skypename);
			if (hContact != NULL)
				SetContactStatus(hContact, SkypeToMirandaStatus(status.c_str()));
		}
	}
}

void CSkypeProto::ProcessNewMessageRes(const JSONNode &node)
{
	debugLogA("CSkypeProto::ProcessNewMessageRes");

	std::string conversationLink = node["conversationLink"].as_string();

	if (conversationLink.find("/8:") != std::string::npos)
		OnPrivateMessageEvent(node);
	else if (conversationLink.find("/19:") != std::string::npos)
		OnChatEvent(node);
}

void CSkypeProto::ProcessConversationUpdateRes(const JSONNode&)
{
	/*const JSONNode &lastMessage = node, "lastMessage");
	const JSONNode &properties = node, "properties");

	std::string convLink(mir_t2a(json_as_string(lastMessage, "conversationLink"))));
	std::string fromLink(mir_t2a(json_as_string(lastMessage, "from"))));

	if (strstr(convLink, "/8:") && IsMe(ContactUrlToName(fromLink)))
	{
		ptrA skypename(ContactUrlToName(convLink));
		MCONTACT hContact = FindContact(skypename);

		if (hContact != NULL)
		{
			CMStringA consumptionhorizon(mir_t2a(json_as_string(properties, "consumptionhorizon"))));

			int iStart = 0;
			CMStringA szToken1 = consumptionhorizon.Tokenize(";", iStart).Trim();

			if (iStart != -1)
			{
				CMStringA szToken2 = consumptionhorizon.Tokenize(";", iStart).Trim();

				szToken2.Truncate(szToken2.GetLength() - 3);

				time_t evttime = atoi(szToken2.GetBuffer());
				db_set_dw(hContact, m_szModuleName, "LastMsgReadTime", evttime);

				//SetSrmmReadStatus(hContact);
			}
		}
	}*/
}

void CSkypeProto::ProcessThreadUpdateRes(const JSONNode&)
{
}