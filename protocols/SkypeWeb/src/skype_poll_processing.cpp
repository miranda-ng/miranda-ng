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

void CSkypeProto::ProcessEndpointPresenceRes(JSONNODE *node)
{
	debugLogA("CSkypeProto::ProcessEndpointPresenceRes");
	ptrA selfLink(mir_t2a(ptrT(json_as_string(json_get(node, "selfLink")))));
	ptrA skypename(ContactUrlToName(selfLink));
	if (skypename == NULL)
		return;

	MCONTACT hContact = FindContact(skypename);
	if (hContact == NULL)
		return;

	//"publicInfo":{"capabilities":"","typ":"11","skypeNameVersion":"0/7.1.0.105//","nodeInfo":"","version":"24"}
	//"privateInfo": {"epname": "Skype"}
	JSONNODE *publicInfo = json_get(node, "publicInfo");
	JSONNODE *privateInfo = json_get(node, "privateInfo");
	CMStringA MirVer = "";
	if (publicInfo != NULL)
	{
		ptrA skypeNameVersion(mir_t2a(ptrT(json_as_string(json_get(publicInfo, "skypeNameVersion")))));
		ptrA version(mir_t2a(ptrT(json_as_string(json_get(publicInfo, "version")))));
		ptrA typ(mir_t2a(ptrT(json_as_string(json_get(publicInfo, "typ")))));
		if (typ != NULL)
		{
			int iTyp = atoi(typ);
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
					if (!mir_strcmpi(typ, "website"))
						MirVer.Append("Skype (Outlook)");
					else
						MirVer.Append("Skype (Unknown)");
					break;
				}
			}
			if (iTyp == 125)
				MirVer.AppendFormat(" %s", version);
			else
				MirVer.AppendFormat(" %s", ParseUrl(skypeNameVersion, "/"));	
		}
	}
	if (privateInfo != NULL)
	{
		ptrA epname(mir_t2a(ptrT(json_as_string(json_get(privateInfo, "epname")))));
		if (epname != NULL && *epname)
		{
			MirVer.AppendFormat(" [%s]", epname);
		}
	}
	db_set_s(hContact, m_szModuleName, "MirVer", MirVer);
}

void CSkypeProto::ProcessUserPresenceRes(JSONNODE *node)
{
	debugLogA("CSkypeProto::ProcessUserPresenceRes");

	ptrA selfLink(mir_t2a(ptrT(json_as_string(json_get(node, "selfLink")))));
	ptrA status(mir_t2a(ptrT(json_as_string(json_get(node, "status")))));
	ptrA skypename;

	if (strstr(selfLink, "/8:"))
	{
		skypename = ContactUrlToName(selfLink);
	}
	else if (strstr(selfLink, "/1:"))
	{
		skypename = SelfUrlToName(selfLink);
	}

	if (skypename != NULL)
	{
		if (IsMe(skypename))
		{
			int iNewStatus = SkypeToMirandaStatus(status);
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
				SetContactStatus(hContact, SkypeToMirandaStatus(status));
		}
	}
}

void CSkypeProto::ProcessNewMessageRes(JSONNODE *node)
{
	debugLogA("CSkypeProto::ProcessNewMessageRes");

	ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));

	if (strstr(conversationLink, "/8:"))
		OnPrivateMessageEvent(node);
	else if (strstr(conversationLink, "/19:"))
		OnChatEvent(node);
}

void CSkypeProto::ProcessConversationUpdateRes(JSONNODE *node)
{
	/*JSONNODE *lastMessage = json_get(node, "lastMessage");
	JSONNODE *properties = json_get(node, "properties");

	ptrA convLink(mir_t2a(json_as_string(json_get(lastMessage, "conversationLink"))));
	ptrA fromLink(mir_t2a(json_as_string(json_get(lastMessage, "from"))));

	if (strstr(convLink, "/8:") && IsMe(ContactUrlToName(fromLink)))
	{
		ptrA skypename(ContactUrlToName(convLink));
		MCONTACT hContact = FindContact(skypename);

		if (hContact != NULL)
		{
			CMStringA consumptionhorizon(mir_t2a(json_as_string(json_get(properties, "consumptionhorizon"))));

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

void CSkypeProto::ProcessThreadUpdateRes(JSONNODE *node)
{
	return;
}