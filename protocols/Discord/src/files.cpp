/*
Copyright © 2016-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#pragma comment(lib, "opus.lib")

/////////////////////////////////////////////////////////////////////////////////////////
// Receiving files

static void __cdecl DownloadCallack(size_t iProgress, void *pParam)
{
	auto *ofd = (OFDTHREAD *)pParam;

	DBVARIANT dbv = { DBVT_DWORD };
	dbv.dVal = unsigned(iProgress);
	db_event_setJson(ofd->hDbEvent, "ft", &dbv);
}

void CDiscordProto::OfflineFileThread(void *param)
{
	auto *ofd = (OFDTHREAD *)param;

	DB::EventInfo dbei(ofd->hDbEvent);
	if (m_bOnline && dbei && !strcmp(dbei.szModule, m_szModuleName) && dbei.eventType == EVENTTYPE_FILE) {
		DB::FILE_BLOB blob(dbei);

		if (!ofd->bCopy) {
			MHttpRequest nlhr(REQUEST_GET);
			nlhr.flags = NLHRF_HTTP11 | NLHRF_SSL;
			nlhr.m_szUrl = blob.getUrl();
			if (!m_szFileCookie.IsEmpty())
				nlhr.AddHeader("Cookie", m_szFileCookie);

			debugLogW(L"Saving to [%s]", ofd->wszPath.c_str());
			NLHR_PTR reply(Netlib_DownloadFile(m_hNetlibUser, &nlhr, ofd->wszPath, DownloadCallack, ofd));
			if (reply && reply->resultCode == 200) {
				if (m_szFileCookie.IsEmpty())
					m_szFileCookie = reply->GetCookies();

				struct _stat st;
				_wstat(ofd->wszPath, &st);

				DBVARIANT dbv = { DBVT_DWORD };
				dbv.dVal = st.st_size;
				db_event_setJson(ofd->hDbEvent, "ft", &dbv);

				ofd->Finish();
			}
		}
		else {
			ofd->wszPath.Empty();
			ofd->wszPath.Append(_A2T(blob.getUrl()));
			ofd->pCallback->Invoke(*ofd);
		}
	}

	delete ofd;
}

INT_PTR CDiscordProto::SvcOfflineFile(WPARAM param, LPARAM)
{
	ForkThread((MyThreadFunc)&CDiscordProto::OfflineFileThread, (void *)param);
	return 0;
}

void CDiscordProto::OnReceiveOfflineFile(DB::EventInfo&, DB::FILE_BLOB &blob)
{
	if (auto *ft = (CDiscordAttachment *)blob.getUserInfo()) {
		blob.setUrl(ft->szUrl.GetBuffer());
		blob.setSize(ft->iFileSize);
		delete ft;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sending files

struct SendFileThreadParam
{
	MCONTACT hContact;
	CMStringW wszDescr, wszFileName;

	SendFileThreadParam(MCONTACT _p1, LPCWSTR _p2, LPCWSTR _p3) :
		hContact(_p1),
		wszFileName(_p2),
		wszDescr(_p3)
	{}
};

void CDiscordProto::SendFileThread(void *param)
{
	SendFileThreadParam *p = (SendFileThreadParam *)param;

	FILE *in = _wfopen(p->wszFileName, L"rb");
	if (in == nullptr) {
		debugLogA("cannot open file %S for reading", p->wszFileName.c_str());
LBL_Error:
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, param);
		delete p;
		return;
	}

	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, param);

	char szRandom[16], szRandomText[33];
	Utils_GetRandom(szRandom, _countof(szRandom));
	bin2hex(szRandom, _countof(szRandom), szRandomText);
	CMStringA szBoundary(FORMAT, "----Boundary%s", szRandomText);

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", getId(p->hContact, DB_KEY_CHANNELID));
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveFile);
	pReq->AddHeader("Content-Type", CMStringA("multipart/form-data; boundary=" + szBoundary));
	pReq->AddHeader("Accept", "*/*");

	szBoundary.Insert(0, "--");

	CMStringA szBody;
	szBody.Append(szBoundary + "\r\n");
	szBody.Append("Content-Disposition: form-data; name=\"content\"\r\n\r\n");
	szBody.Append(ptrA(mir_utf8encodeW(p->wszDescr)));
	szBody.Append("\r\n");

	szBody.Append(szBoundary + "\r\n");
	szBody.Append("Content-Disposition: form-data; name=\"tts\"\r\n\r\nfalse\r\n");

	wchar_t *pFileName = wcsrchr(p->wszFileName.GetBuffer(), '\\');
	if (pFileName != nullptr)
		pFileName++;
	else
		pFileName = p->wszFileName.GetBuffer();

	szBody.Append(szBoundary + "\r\n");
	szBody.AppendFormat("Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n", ptrA(mir_utf8encodeW(pFileName)).get());
	szBody.AppendFormat("Content-Type: %S\r\n", ProtoGetAvatarMimeType(ProtoGetAvatarFileFormat(p->wszFileName)));
	szBody.Append("\r\n");

	size_t cbBytes = filelength(fileno(in));

	szBoundary.Insert(0, "\r\n");
	szBoundary.Append("--\r\n");
	pReq->m_szParam.Truncate(int(szBody.GetLength() + szBoundary.GetLength() + cbBytes));

	memcpy(pReq->m_szParam.GetBuffer(), szBody.c_str(), szBody.GetLength());
	size_t cbRead = fread(pReq->m_szParam.GetBuffer() + szBody.GetLength(), 1, cbBytes, in);
	fclose(in);
	if (cbBytes != cbRead) {
		debugLogA("cannot read file %S: %d bytes read instead of %d", p->wszFileName.c_str(), cbRead, cbBytes);
		delete pReq;
		goto LBL_Error;
	}

	memcpy(pReq->m_szParam.GetBuffer() + szBody.GetLength() + cbBytes, szBoundary, szBoundary.GetLength());
	pReq->pUserInfo = p;
	Push(pReq);

	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, param);
}

void CDiscordProto::OnReceiveFile(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	SendFileThreadParam *p = (SendFileThreadParam *)pReq->pUserInfo;
	if (pReply->resultCode != 200) {
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, p);
		debugLogA("CDiscordProto::SendFile failed: %d", pReply->resultCode);
	}
	else {
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, p);
		debugLogA("CDiscordProto::SendFile succeeded");
	}

	delete p;
}

HANDLE CDiscordProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	SnowFlake id = getId(hContact, DB_KEY_CHANNELID);
	if (id == 0)
		return nullptr;

	// we don't wanna block the main thread, right?
	SendFileThreadParam *param = new SendFileThreadParam(hContact, ppszFiles[0], szDescription);
	ForkThread(&CDiscordProto::SendFileThread, param);
	return param;
}
