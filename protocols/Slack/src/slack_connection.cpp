#include "stdafx.h"

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

	HttpRequest *request = new HttpRequest(HttpMethod::HttpPost, SLACK_API_URL "/oauth.access");
	request->Headers
		<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded;charset=utf-8");
	request->Content
		<< CHAR_VALUE("client_id", SLACK_CLIENT_ID)
		<< CHAR_VALUE("client_secret", SLACK_CLIENT_SECRET)
		<< ENCODED_VALUE("code", oauth.GetAuthCode())
		<< ENCODED_VALUE("redirect_uri", SLACK_REDIRECT_URL);

	PushRequest(request, &CSlackProto::OnAuthorize);
}

void CSlackProto::OnAuthorize(JSONNode &root)
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
	if (!teamName.IsEmpty() > 0 && !Clist_GroupExists(teamName))
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