/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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

int CVkProto::SetStatus(int iNewStatus)
{
	debugLogA("CVkProto::SetStatus iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d m_hWorkerThread = %d",
		iNewStatus, m_iStatus, m_iDesiredStatus, m_hWorkerThread == nullptr ? 0 : 1);

	mir_cslock lck(m_csSetStatus);

	if (m_iDesiredStatus == iNewStatus || iNewStatus == ID_STATUS_IDLE)
		return 0;

	int oldStatus = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (IsOnline()) {
			SetServerStatus(ID_STATUS_OFFLINE);
			debugLogA("CVkProto::SetStatus ShutdownSession");
			ShutdownSession();
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		debugLogA("CVkProto::SetStatus (1) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d",
			iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
	}
	else if (m_hWorkerThread == nullptr && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		debugLogA("CVkProto::SetStatus (2) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d",
			iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
		m_hWorkerThread = ForkThreadEx(&CVkProto::WorkerThread, nullptr, nullptr);
	}
	else if (IsOnline()) {
		debugLogA("CVkProto::SetStatus (3) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d",
			iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
		SetServerStatus(iNewStatus);
	}
	else {
		debugLogA("CVkProto::SetStatus (4) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d",
			iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		if (!IsStatusConnecting(m_iStatus))
			m_iDesiredStatus = m_iStatus;
		debugLogA("CVkProto::SetStatus (5) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d",
			iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
	}

	debugLogA("CVkProto::SetStatus (ret) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d",
		iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
	return 0;
}

void CVkProto::SetServerStatus(int iNewStatus)
{
	debugLogA("CVkProto::SetServerStatus %d %d", iNewStatus, m_iStatus);
	if (!IsOnline() || iNewStatus < ID_STATUS_OFFLINE)
		return;

	int iOldStatus = m_iStatus;
	CMStringW oldStatusMsg(db_get_wsm(0, m_szModuleName, "OldStatusMsg"));
	ptrW pwszListeningToMsg(db_get_wsa(0, m_szModuleName, "ListeningTo"));

	if (iNewStatus == ID_STATUS_OFFLINE) {
		m_bNeedSendOnline = false;
		if (!IsEmpty(pwszListeningToMsg) && m_bSetBroadcast) {
			RetrieveStatusMsg(oldStatusMsg);
			m_bSetBroadcast = false;
		}
		m_iStatus = ID_STATUS_OFFLINE;
		if (iOldStatus != ID_STATUS_OFFLINE && iOldStatus != ID_STATUS_INVISIBLE)
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOffline.json", true, &CVkProto::OnReceiveSmth));
	}
	else if (iNewStatus != ID_STATUS_INVISIBLE) {
		m_bNeedSendOnline = true;
		if (iOldStatus == ID_STATUS_ONLINE)
			return;
		m_iStatus = ID_STATUS_ONLINE;
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOnline.json", true, &CVkProto::OnReceiveSmth));
	}
	else {
		m_bNeedSendOnline = false;
		if (!IsEmpty(pwszListeningToMsg) && m_bSetBroadcast) {
			RetrieveStatusMsg(oldStatusMsg);
			m_bSetBroadcast = false;
		}
		m_iStatus = ID_STATUS_INVISIBLE;
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOffline.json", true, &CVkProto::OnReceiveSmth));
	}

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR __cdecl CVkProto::SvcSetStatusMsg(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcSetStatusMsg");
	if (!IsOnline())
		return 1;

	MsgPopup(TranslateT("Loading status message from vk.com.\nThis may take some time."), TranslateT("Waiting..."));

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/status.get.json", true, &CVkProto::OnReceiveStatusMsg));

	return 0;
}

void CVkProto::OnReceiveStatusMsg(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveStatusMsg %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	OnReceiveStatus(reply, pReq);

	ptrW pwszOldStatusMsg(db_get_wsa(0, m_szModuleName, "OldStatusMsg"));
	CMStringW wszOldStatusMsg(pwszOldStatusMsg);

	ENTER_STRING pForm = {};
	pForm.type = ESF_MULTILINE;
	pForm.caption = TranslateT("Enter new status message");
	pForm.ptszInitVal = pwszOldStatusMsg;
	pForm.szModuleName = m_szModuleName;
	pForm.szDataPrefix = "statusmsgform_";

	if (!EnterString(&pForm))
		return;

	CMStringW wszNewStatusMsg(ptrW(pForm.ptszResult));
	if (wszOldStatusMsg == wszNewStatusMsg)
		return;

	RetrieveStatusMsg(wszNewStatusMsg);
	setWString("OldStatusMsg", pwszOldStatusMsg);
}

void CVkProto::OnReceiveStatus(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveStatus %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;
	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	const JSONNode &jnAudio = jnResponse["audio"];
	if (!jnAudio) {
		CMStringW wszStatusText(jnResponse["text"].as_mstring());
		if (wszStatusText[0] != wchar_t(9835))
			setWString("OldStatusMsg", wszStatusText);
	}
}

void CVkProto::RetrieveStatusMsg(const CMStringW &StatusMsg)
{
	debugLogA("CVkProto::RetrieveStatusMsg");
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/status.set.json", true, &CVkProto::OnReceiveSmth)
		<< WCHAR_PARAM("text", StatusMsg));
}

void CVkProto::RetrieveStatusMusic(const CMStringW &StatusMsg)
{
	debugLogA("CVkProto::RetrieveStatusMusic");
	if (!IsOnline() || m_iStatus == ID_STATUS_INVISIBLE || m_vkOptions.iMusicSendMetod == MusicSendMetod::sendNone)
		return;

	CMStringW wszOldStatusMsg(db_get_wsa(0, m_szModuleName, "OldStatusMsg"));
	if (StatusMsg.IsEmpty()) {
		CMStringW code;
		if (m_vkOptions.iMusicSendMetod == MusicSendMetod::sendBroadcastOnly)
			code = "API.audio.setBroadcast();return null;";
		else {
			CMStringW codeformat("API.status.set({text:\"%s\"});return null;");
			code.AppendFormat(codeformat, wszOldStatusMsg.c_str());
		}
		m_bSetBroadcast = false;
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveStatus)
			<< WCHAR_PARAM("code", code));
	}
	else {
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.RetrieveStatusMusic", true, &CVkProto::OnReceiveStatus)
			<< WCHAR_PARAM("statusmsg", StatusMsg)
			<< INT_PARAM("func_v", (int)(m_vkOptions.iMusicSendMetod))
		);
		m_bSetBroadcast = true;
	}
}

INT_PTR __cdecl CVkProto::SvcSetListeningTo(WPARAM, LPARAM lParam)
{
	debugLogA("CVkProto::SvcSetListeningTo");
	if (m_vkOptions.iMusicSendMetod == MusicSendMetod::sendNone)
		return 1;

	LISTENINGTOINFO *pliInfo = (LISTENINGTOINFO*)lParam;
	CMStringW wszListeningTo;
	if (pliInfo == nullptr || pliInfo->cbSize != sizeof(LISTENINGTOINFO))
		db_unset(0, m_szModuleName, "ListeningTo");
	else if (pliInfo->dwFlags & LTI_UNICODE) {
		if (ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT))
			wszListeningTo = ptrW((LPWSTR)CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM)L"%artist% - %title%", (LPARAM)pliInfo));
		else
			wszListeningTo.Format(L"%s - %s",
				pliInfo->ptszArtist ? pliInfo->ptszArtist : L"",
				pliInfo->ptszTitle ? pliInfo->ptszTitle : L"");
		setWString("ListeningTo", wszListeningTo);
	}
	RetrieveStatusMusic(wszListeningTo);
	return 0;
}

HANDLE CVkProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&CVkProto::GetAwayMsgThread, (void *)hContact);
	return (HANDLE)1;
}

void CVkProto::GetAwayMsgThread(void* p)
{
	Sleep(100);

	MCONTACT hContact = (DWORD_PTR)p;
	ptrW wszStatus(db_get_wsa(hContact, "CList", "StatusMsg"));
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, wszStatus);
}
