#include "stdafx.h"

bool CSlackProto::IsOnline()
{
	return !isTerminated && m_iStatus > ID_STATUS_OFFLINE;
}

void CSlackProto::Login()
{
	ptrA token(getStringA("TokenSecret"));
	if (mir_strlen(token))
	{
		PushRequest(new GetUserListRequest(token), &CSlackProto::OnGotUserList);
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);		
		return;
	}

	CSlackOAuth oauth(this);
	if (!oauth.DoModal())
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	PushRequest(new OAuhtAccessRequest(oauth.GetAuthCode()), &CSlackProto::OnAuthorize);
}

void CSlackProto::OnAuthorize(JSONNode &root, void*)
{
	if (!root)
	{
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, NULL);
		return;
	}

	bool isOk = root["ok"].as_bool();
	if (!isOk)
	{
		SetStatus(ID_STATUS_OFFLINE);
		json_string error = root["error"].as_string();
		debugLogA(__FUNCTION__": %s", error);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
		return;
	}

	json_string token = root["access_token"].as_string();
	setString("TokenSecret", token.c_str());

	json_string userId = root["user_id"].as_string();
	setString("UserId", userId.c_str());

	CMStringW teamName = root["team_name"].as_mstring();
	setWString("TeamName", teamName);
	if (!teamName.IsEmpty() && !Clist_GroupExists(teamName))
		Clist_GroupCreate(0, teamName);

	json_string teamId = root["team_id"].as_string();
	setString("TeamId", userId.c_str());

	PushRequest(new GetUserListRequest(token.c_str()), &CSlackProto::OnGotUserList);

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);
}

void CSlackProto::LogOut()
{
	isTerminated = true;
	if (hRequestQueueThread)
		SetEvent(hRequestsQueueEvent);
}