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
	CMStringA skypename(UrlToSkypename(selfLink.c_str()));

	MCONTACT hContact = FindContact(skypename);
	if (hContact == NULL)
		return;

	const JSONNode &publicInfo = node["publicInfo"];
	const JSONNode &privateInfo = node["privateInfo"];
	CMStringA MirVer;
	if (publicInfo)
	{
		std::string skypeNameVersion = publicInfo["skypeNameVersion"].as_string();
		std::string version = publicInfo["version"].as_string();
		std::string typ = publicInfo["typ"].as_string();
		int iTyp = atoi(typ.c_str());
		switch (iTyp)
		{
		case 0:
		case 1:
			MirVer.AppendFormat("Skype (Web) %s", ParseUrl(version.c_str(), "/"));
			break;
		case 10:
			MirVer.AppendFormat("Skype (XBOX) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 17:
			MirVer.AppendFormat("Skype (Android) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 16:
			MirVer.AppendFormat("Skype (iOS) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 12:
			MirVer.AppendFormat("Skype (WinRT) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 15:
			MirVer.AppendFormat("Skype (WP) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 13:
			MirVer.AppendFormat("Skype (OSX) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
				break;
		case 11:
			MirVer.AppendFormat("Skype (Windows) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 14:
			MirVer.AppendFormat("Skype (Linux) %s", ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 125:
			MirVer.AppendFormat("Miranda NG Skype %s", version.c_str());
			break;
		default:
				MirVer.Append("Skype (Unknown)");
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
	CMStringA skypename = UrlToSkypename(selfLink.c_str());

	if (!skypename.IsEmpty())
	{
		if (IsMe(skypename))
		{
			int iNewStatus = SkypeToMirandaStatus(status.c_str());
			int old_status = m_iStatus;
			m_iDesiredStatus = iNewStatus;
			m_iStatus = iNewStatus;
			if (old_status != iNewStatus)
			{
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, iNewStatus);
			}
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
	debugLogA(__FUNCTION__);

	std::string conversationLink = node["conversationLink"].as_string();

	if (conversationLink.find("/8:") != std::string::npos)
		OnPrivateMessageEvent(node);
	else if (conversationLink.find("/19:") != std::string::npos)
		OnChatEvent(node);
}

void CSkypeProto::ProcessConversationUpdateRes(const JSONNode&){}
void CSkypeProto::ProcessThreadUpdateRes(const JSONNode&){}