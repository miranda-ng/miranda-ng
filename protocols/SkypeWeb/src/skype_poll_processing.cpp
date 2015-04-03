#include "common.h"

void CSkypeProto::ProcessEndpointPresenceRes(JSONNODE *node)
{
	debugLogA("CSkypeProto::ProcessEndpointPresenceRes");
	ptrA selfLink(mir_t2a(ptrT(json_as_string(json_get(node, "selfLink")))));
	char *skypename = ContactUrlToName(selfLink);
	if (skypename == NULL)
		return;
	MCONTACT hContact = GetContact(skypename);

	//"publicInfo":{"capabilities":"","typ":"11","skypeNameVersion":"0/7.1.0.105//","nodeInfo":"","version":"24"}
	JSONNODE *publicInfo = json_get(node, "publicInfo");
	if (publicInfo != NULL) 
	{
		ptrA skypeNameVersion(mir_t2a(ptrT(json_as_string(json_get(publicInfo, "skypeNameVersion")))));
		ptrA version(mir_t2a(ptrT(json_as_string(json_get(publicInfo, "version")))));
		ptrA typ(mir_t2a(ptrT(json_as_string(json_get(publicInfo, "typ")))));
		if (typ != NULL)
		{
			if (!mir_strcmpi(typ, "17")) //Android
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (Android)");
			else if (!mir_strcmpi(typ, "16")) //iOS
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (iOS)");
			else if (!mir_strcmpi(typ, "12")) //WinRT/Metro
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (WinRT)");
			else if (!mir_strcmpi(typ, "15")) //Winphone
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (WP)");
			else if (!mir_strcmpi(typ, "13")) //OSX
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (OSX)");
			else if (!mir_strcmpi(typ, "11")) //Windows
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (Windows)");
			else if (!mir_strcmpi(typ, "14")) //Linux
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (Linux)");
			else if (!mir_strcmpi(typ, "10"))//XBox ? skypeNameVersion 11/1.8.0.1006
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (XBOX)");
			else if (!mir_strcmpi(typ, "1")) //SkypeWeb
				db_set_s(hContact, m_szModuleName, "MirVer", "Skype (Web)");
			else if (!mir_strcmpi(typ, "125")) //Miranda
			{
				char ver[MAX_PATH];
				mir_snprintf(ver, SIZEOF(ver), "%s %s", skypeNameVersion, version);
				db_set_s(hContact, m_szModuleName, "MirVer", ver);
			}

		}

	}
}

void CSkypeProto::ProcessUserPresenceRes(JSONNODE *node)
{
	debugLogA("CSkypeProto::ProcessUserPresenceRes");

	ptrA selfLink(mir_t2a(ptrT(json_as_string(json_get(node, "selfLink")))));
	ptrA status(mir_t2a(ptrT(json_as_string(json_get(node, "status")))));
	char *skypename = ContactUrlToName(selfLink);
	if (skypename == NULL)
	{
		if (IsMe(SelfUrlToName(selfLink)))
		{
				int iNewStatus = SkypeToMirandaStatus(status);
				int old_status = m_iStatus;
				m_iDesiredStatus = iNewStatus;
				m_iStatus = iNewStatus;
				if (old_status != iNewStatus)
					ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, iNewStatus);
				return;
		}
		return;
	}
	MCONTACT hContact = GetContact(skypename);
	SetContactStatus(hContact, SkypeToMirandaStatus(status));
}

void CSkypeProto::ProcessNewMessageRes(JSONNODE *node)
{
	ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(node, "clientmessageid")))));
	ptrA skypeeditedid(mir_t2a(ptrT(json_as_string(json_get(node, "skypeeditedid")))));
	ptrA messagetype(mir_t2a(ptrT(json_as_string(json_get(node, "messagetype")))));
	ptrA from(mir_t2a(ptrT(json_as_string(json_get(node, "from")))));
	ptrA content(mir_t2a(ptrT(json_as_string(json_get(node, "content")))));
	TCHAR *composeTime = json_as_string (json_get(node, "composetime"));
	ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));
	time_t timeStamp = IsoToUnixTime(composeTime);
	char *convname;
	if (strstr(conversationLink, "/19:"))
		{
			const char *chatname, *topic;
			chatname = ContactUrlToName(conversationLink);
			convname = mir_strdup(chatname);
			return; //chats not supported
		}
	else if (strstr(conversationLink, "/8:"))
	{
		if (!mir_strcmpi(messagetype, "Control/Typing"))
		{
			MCONTACT hContact = GetContact(ContactUrlToName(from));
			CallService(MS_PROTO_CONTACTISTYPING, hContact, 5);
		}
		else if (!mir_strcmpi(messagetype, "Control/ClearTyping"))
		{
			return;
		}
		else if (!mir_strcmpi(messagetype, "Text") || !mir_strcmpi(messagetype, "RichText")) 
		{
			OnReceiveMessage(from, conversationLink, timeStamp, content);
		}
		else if (!mir_strcmpi(messagetype, "Event/SkypeVideoMessage"))
		{
			return; //not supported
		}
	}
}