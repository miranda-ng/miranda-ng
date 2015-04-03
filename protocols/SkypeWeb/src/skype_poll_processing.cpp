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
		ptrA version(mir_t2a(ptrT(json_as_string(json_get(publicInfo, "skypeNameVersion")))));
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
				db_set_s(hContact, m_szModuleName, "MirVer", "Miranda NG");

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
	//ptrA composeTime(mir_t2a(ptrT(json_as_string(json_get(node, "composetime")))));
	TCHAR *composeTime = json_as_string (json_get(node, "composetime"));
	ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));
	time_t timeStamp = IsoToUnixTime(composeTime);//time(NULL); // it should be rewritten

	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.timestamp = timeStamp;
	recv.szMessage = content;
	debugLogA("Incoming message from %s", ContactUrlToName(from));
	if(IsMe(ContactUrlToName(from)))
		return; //it should be rewritten
	MCONTACT hContact = GetContact(ContactUrlToName(from));
	OnReceiveMessage(hContact, &recv);
}