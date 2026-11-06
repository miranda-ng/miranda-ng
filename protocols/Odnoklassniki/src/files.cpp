/*
Copyright (C) 2026 Miranda NG team

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

// Images are the only attachments: received photos are "offline files" (a url, downloaded
// when the user opens or saves them), and a file sent to a contact or chat must be a picture,
// which goes up through photosV2 and is attached to a message as UPLOADED_PHOTO.

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// receiving

void COdnoklassnikiProto::OnReceiveOfflineFile(DB::EventInfo &, DB::FILE_BLOB &blob)
{
	if (auto *p = (OkPhoto *)blob.getUserInfo()) {
		blob.setUrl(p->szUrl);
		delete p;
	}
}

static void __cdecl DownloadCallback(size_t iProgress, void *pParam)
{
	auto *ofd = (OFDTHREAD *)pParam;

	DBVARIANT dbv = { DBVT_DWORD };
	dbv.dVal = unsigned(iProgress);
	db_event_setJson(ofd->hDbEvent, "ft", &dbv);
}

void COdnoklassnikiProto::OfflineFileThread(void *param)
{
	auto *ofd = (OFDTHREAD *)param;

	DB::EventInfo dbei(ofd->hDbEvent);
	if (dbei && !strcmp(dbei.szModule, m_szModuleName) && dbei.eventType == EVENTTYPE_FILE) {
		DB::FILE_BLOB blob(dbei);

		if (!ofd->bCopy) {
			MHttpRequest req(REQUEST_GET);
			req.flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_REDIRECT | NLHRF_NOUSERAGENT;
			req.m_szUrl = blob.getUrl();
			req.AddHeader("User-Agent", OK_USER_AGENT);

			debugLogW(L"Saving to [%s]", ofd->wszPath.c_str());
			NLHR_PTR reply(Netlib_DownloadFile(m_hNetlibUser, &req, ofd->wszPath, DownloadCallback, ofd));
			if (reply && reply->resultCode == 200) {
				struct _stat st;
				_wstat(ofd->wszPath, &st);

				DBVARIANT dbv = { DBVT_DWORD };
				dbv.dVal = st.st_size;
				db_event_setJson(ofd->hDbEvent, "ft", &dbv);

				ofd->Finish();
			}
			else debugLogA("download failed: HTTP %d", reply ? reply->resultCode : 0);
		}
		else {
			ofd->wszPath.Empty();
			ofd->wszPath.Append(_A2T(blob.getUrl()));
			ofd->pCallback->Invoke(*ofd);
		}
	}

	delete ofd;
}

INT_PTR COdnoklassnikiProto::SvcOfflineFile(WPARAM param, LPARAM)
{
	ForkThread(&COdnoklassnikiProto::OfflineFileThread, (void *)param);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// sending

struct SendFileParam
{
	MCONTACT hContact;
	CMStringW wszFile, wszDescr;
};

static bool IsImage(const wchar_t *pwszFile)
{
	const wchar_t *ext = wcsrchr(pwszFile, '.');
	if (ext == nullptr)
		return false;

	static const wchar_t *exts[] = { L".jpg", L".jpeg", L".png", L".gif", L".bmp", L".webp" };
	for (auto &it : exts)
		if (!mir_wstrcmpi(ext, it))
			return true;
	return false;
}

void COdnoklassnikiProto::SendFileThread(void *param)
{
	auto *p = (SendFileParam *)param;
	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, p);

	CMStringA szToken, szError, cnvId(GetConversationId(p->hContact));
	bool bOk = false;
	if (cnvId.IsEmpty())
		debugLogA("no conversation for contact %d", p->hContact);
	else if (!UploadPhoto(p->wszFile, szToken))
		debugLogW(L"cannot upload %s", p->wszFile.c_str());
	else {
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, p);
		CMStringA szId = SendMessage(cnvId, T2Utf(p->wszDescr), szToken, szError);
		if (szId.IsEmpty())
			debugLogA("sending the photo failed: %s", szError.c_str());
		else {
			mir_cslock lck(m_csSentPhotos);
			m_sentPhotos.insert(szId);
			bOk = true;
		}
	}

	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, bOk ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, p);
	SetEvent(m_hPollEvent);
	delete p;
}

HANDLE COdnoklassnikiProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	if (!m_bOnline || ppszFiles == nullptr || ppszFiles[0] == nullptr)
		return nullptr;

	if (!IsImage(ppszFiles[0])) {
		MessageBoxW(nullptr, TranslateT("Odnoklassniki accepts only images: JPG, PNG, GIF, BMP or WEBP."), m_tszUserName, MB_OK | MB_ICONWARNING);
		return nullptr;
	}

	auto *p = new SendFileParam{ hContact, ppszFiles[0], szDescription ? szDescription : L"" };
	ForkThread(&COdnoklassnikiProto::SendFileThread, p);
	return p;
}
