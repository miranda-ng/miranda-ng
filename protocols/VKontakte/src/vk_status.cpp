/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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
	debugLogA("CVkProto::SetStatus iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d m_hWorkerThread = %d", iNewStatus, m_iStatus, m_iDesiredStatus, m_hWorkerThread == NULL ? 0 : 1);
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
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		debugLogA("CVkProto::SetStatus (1) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d", iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
	}
	else if (m_hWorkerThread == NULL && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		debugLogA("CVkProto::SetStatus (2) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d", iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
		m_hWorkerThread = ForkThreadEx(&CVkProto::WorkerThread, 0, NULL);
	}
	else if (IsOnline()) {
		debugLogA("CVkProto::SetStatus (3) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d", iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
		SetServerStatus(iNewStatus);
	}
	else {
		debugLogA("CVkProto::SetStatus (4) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d", iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		if (!IsStatusConnecting(m_iStatus))
			m_iDesiredStatus = m_iStatus;
		debugLogA("CVkProto::SetStatus (5) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d", iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
	}

	debugLogA("CVkProto::SetStatus (ret) iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d oldStatus = %d", iNewStatus, m_iStatus, m_iDesiredStatus, oldStatus);
	return 0;
}

void CVkProto::SetServerStatus(int iNewStatus)
{
	debugLogA("CVkProto::SetServerStatus %d %d", iNewStatus, m_iStatus);
	if (!IsOnline() || iNewStatus < ID_STATUS_OFFLINE)
		return;

	int iOldStatus = m_iStatus;
	CMString oldStatusMsg = ptrT(db_get_tsa(NULL, m_szModuleName, "OldStatusMsg"));
	ptrT ptszListeningToMsg(db_get_tsa(NULL, m_szModuleName, "ListeningTo"));

	if (iNewStatus == ID_STATUS_OFFLINE) {
		m_bNeedSendOnline = false;
		if (!IsEmpty(ptszListeningToMsg) && m_bSetBroadcast) {
			RetrieveStatusMsg(oldStatusMsg);
			m_bSetBroadcast = false;
		}
		m_iStatus = ID_STATUS_OFFLINE;
		if (iOldStatus != ID_STATUS_OFFLINE && iOldStatus != ID_STATUS_INVISIBLE)
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOffline.json", true, &CVkProto::OnReceiveSmth)
			<< VER_API);
	}
	else if (iNewStatus != ID_STATUS_INVISIBLE) {
		m_bNeedSendOnline = true;
		if (iOldStatus == ID_STATUS_ONLINE)
			return;
		m_iStatus = ID_STATUS_ONLINE;
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOnline.json", true, &CVkProto::OnReceiveSmth)
			<< VER_API);
	}
	else {
		m_bNeedSendOnline = false;
		if (!IsEmpty(ptszListeningToMsg) && m_bSetBroadcast) {
			RetrieveStatusMsg(oldStatusMsg);
			m_bSetBroadcast = false;
		}
		m_iStatus = ID_STATUS_INVISIBLE;
		if (iOldStatus == ID_STATUS_ONLINE)
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOffline.json", true, &CVkProto::OnReceiveSmth)
			<< VER_API);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR __cdecl CVkProto::SvcSetStatusMsg(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcSetStatusMsg");
	if (!IsOnline())
		return 1;

	MsgPopup(NULL, TranslateT("Loading status message from vk.com.\nThis may take some time."), TranslateT("Waiting..."));

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/status.get.json", true, &CVkProto::OnReceiveStatusMsg)
		<< VER_API);

	return 0;
}

void CVkProto::OnReceiveStatusMsg(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveStatusMsg %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	OnReceiveStatus(reply, pReq);

	ptrT ptszOldStatusMsg(db_get_tsa(NULL, m_szModuleName, "OldStatusMsg"));
	CMString tszOldStatusMsg(ptszOldStatusMsg);

	ENTER_STRING pForm = { sizeof(pForm) };
	pForm.type = ESF_MULTILINE;
	pForm.caption = TranslateT("Enter new status message");
	pForm.ptszInitVal = ptszOldStatusMsg;
	pForm.szModuleName = m_szModuleName;
	pForm.szDataPrefix = "statusmsgform_";

	if (!EnterString(&pForm))
		return;

	CMString tszNewStatusMsg(ptrT(pForm.ptszResult));
	if (tszOldStatusMsg == tszNewStatusMsg)
		return;

	RetrieveStatusMsg(tszNewStatusMsg);
	setTString("OldStatusMsg", ptszOldStatusMsg);
}

void CVkProto::OnReceiveStatus(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveStatus %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;
	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	JSONNODE *pAudio = json_get(pResponse, "audio");
	if (pAudio == NULL) {
		ptrT ptszStatusText(json_as_string(json_get(pResponse, "text")));
		if (ptszStatusText[0] != TCHAR(9835))
			setTString("OldStatusMsg", ptszStatusText);
	}
}

void CVkProto::RetrieveStatusMsg(const CMString &StatusMsg)
{
	debugLogA("CVkProto::RetrieveStatusMsg");
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/status.set.json", true, &CVkProto::OnReceiveSmth)
		<< TCHAR_PARAM("text", StatusMsg)
		<< VER_API);
}

void CVkProto::RetrieveStatusMusic(const CMString &StatusMsg)
{
	debugLogA("CVkProto::RetrieveStatusMusic");
	if (!IsOnline() || m_iStatus == ID_STATUS_INVISIBLE || m_iMusicSendMetod == sendNone)
		return;

	CMString code;
	ptrT ptszOldStatusMsg(db_get_tsa(0, m_szModuleName, "OldStatusMsg"));
	if (StatusMsg.IsEmpty()) {
		if (m_iMusicSendMetod == sendBroadcastOnly)
			code = "API.audio.setBroadcast();return null;";
		else {
			CMString codeformat("API.status.set({text:\"%s\"});return null;");
			code.AppendFormat(codeformat, ptszOldStatusMsg);
		}
		m_bSetBroadcast = false;
	}
	else {
		if (m_iMusicSendMetod == sendBroadcastOnly) {
			CMString codeformat("var StatusMsg=\"%s\";var CntLmt=100;var OldMsg=API.status.get();"
				"var Tracks=API.audio.search({\"q\":StatusMsg,\"count\":CntLmt,\"search_own\":1});"
				"var Cnt=Tracks.count;if(Cnt>CntLmt){Cnt=CntLmt;}"
				"if(Cnt==0){API.audio.setBroadcast();}"
				"else{var i=0;var j=0;var Track=\" \";"
				"while(i<Cnt){Track=Tracks.items[i].artist+\" - \"+Tracks.items[i].title;if(Track==StatusMsg){j=i;}i=i+1;}"
				"Track=Tracks.items[j].owner_id+\"_\"+Tracks.items[j].id;API.audio.setBroadcast({\"audio\":Track});"
				"};return OldMsg;");
			code.AppendFormat(codeformat, StatusMsg);
		}
		else if (m_iMusicSendMetod == sendStatusOnly) {
			CMString codeformat("var StatusMsg=\"&#9835; %s\";var OldMsg=API.status.get();"
				"API.status.set({\"text\":StatusMsg});"
				"return OldMsg;");
			code.AppendFormat(codeformat, StatusMsg);
		}
		else if (m_iMusicSendMetod == sendBroadcastAndStatus) {
			CMString codeformat("var StatusMsg=\"%s\";var CntLmt=100;var Track=\" \";var OldMsg=API.status.get();"
				"var Tracks=API.audio.search({\"q\":StatusMsg,\"count\":CntLmt,\"search_own\":1});"
				"var Cnt=Tracks.count;if(Cnt>CntLmt){Cnt=CntLmt;}"
				"if(Cnt==0){Track=\"&#9835; \"+StatusMsg;API.status.set({\"text\":Track});}"
				"else{var i=0;var j=-1;"
				"while(i<Cnt){Track=Tracks.items[i].artist+\" - \"+Tracks.items[i].title;if(Track==StatusMsg){j=i;}i=i+1;}"
				"if(j==-1){Track=\"&#9835; \"+StatusMsg;API.status.set({\"text\":Track});}else{"
				"Track=Tracks.items[j].owner_id+\"_\"+Tracks.items[j].id;};API.audio.setBroadcast({\"audio\":Track});"
				"};return OldMsg;");
			code.AppendFormat(codeformat, StatusMsg);
		}
		m_bSetBroadcast = true;
	}
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveStatus)
		<< TCHAR_PARAM("code", code)
		<< VER_API);
}

INT_PTR __cdecl CVkProto::SvcSetListeningTo(WPARAM, LPARAM lParam)
{
	debugLogA("CVkProto::SvcSetListeningTo");
	if (m_iMusicSendMetod == sendNone)
		return 1;

	LISTENINGTOINFO *pliInfo = (LISTENINGTOINFO*)lParam;
	CMString tszListeningTo;
	if (pliInfo == NULL || pliInfo->cbSize != sizeof(LISTENINGTOINFO))
		db_unset(NULL, m_szModuleName, "ListeningTo");
	else if (pliInfo->dwFlags & LTI_UNICODE) {
		if (ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT))
			tszListeningTo = ptrT((LPWSTR)CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM)_T("%artist% - %title%"), (LPARAM)pliInfo));
		else
			tszListeningTo.Format(_T("%s - %s"),
			pliInfo->ptszArtist ? pliInfo->ptszArtist : _T(""),
			pliInfo->ptszTitle ? pliInfo->ptszTitle : _T(""));
		setTString("ListeningTo", tszListeningTo);
	}
	RetrieveStatusMusic(tszListeningTo);
	return 0;
}