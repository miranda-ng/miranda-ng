/*
Copyright (c) 2026 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.
*/

#include "stdafx.h"
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

static INT_PTR CALLBACK MaxAccMgrProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK MaxOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

struct CMaxMsgAckCtx
{
	CMaxProto *pProto = nullptr;
	MCONTACT hContact = 0;
	int result = ACKRESULT_FAILED;
	int hProcess = 0;
	CMStringA msgId;
	CMStringW errText;
};

struct CMaxFileSendCtx
{
	CMaxProto *pProto = nullptr;
	MCONTACT hContact = 0;
	CMStringW wszDesc;
	std::vector<CMStringW> files;
	int iCurrent = 0;
	CMStringW wszLastError;
};

typedef CProtoDlgBase<CMaxProto> CMaxDlgBase;

class CMaxQRDlg : public CMaxDlgBase
{
	bool m_bSucceeded = false;

public:
	CMaxQRDlg(CMaxProto *ppro) :
		CMaxDlgBase(ppro, IDD_QRLOGIN)
	{
		ppro->m_pQRDlg = this;
	}

	void OnDestroy() override
	{
		m_proto->m_pQRDlg = nullptr;
	}

	void SetData(const CMStringA &str)
	{
		auto *pQR = QRcode_encodeString(str, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
		if (pQR == nullptr)
			return;

		HWND hwndRc = GetDlgItem(m_hwnd, IDC_QRPIC);
		RECT rc;
		GetClientRect(hwndRc, &rc);

		int scale = 8;
		int rowLen = pQR->width * scale * 3;
		if (rowLen % 4)
			rowLen = (rowLen / 4 + 1) * 4;
		int dataLen = rowLen * pQR->width * scale;

		mir_ptr<BYTE> pData((BYTE *)mir_alloc(dataLen));
		if (pData == nullptr) {
			QRcode_free(pQR);
			return;
		}

		memset(pData, 0xFF, dataLen);
		const BYTE *s = pQR->data;
		for (int y = 0; y < pQR->width; y++) {
			BYTE *d = pData.get() + rowLen * y * scale;
			for (int x = 0; x < pQR->width; x++) {
				if (*s & 1) {
					for (int i = 0; i < scale; i++) {
						for (int j = 0; j < scale; j++) {
							d[j * 3 + i * rowLen] = 0;
							d[1 + j * 3 + i * rowLen] = 0;
							d[2 + j * 3 + i * rowLen] = 0;
						}
					}
				}
				d += scale * 3;
				s++;
			}
		}

		BITMAPFILEHEADER fih = {};
		fih.bfType = 0x4d42;
		fih.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataLen;
		fih.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER bih = {};
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = pQR->width * scale;
		bih.biHeight = -bih.biWidth;
		bih.biPlanes = 1;
		bih.biBitCount = 24;
		bih.biCompression = BI_RGB;

		wchar_t wszTempPath[MAX_PATH], wszTempFile[MAX_PATH];
		GetTempPathW(_countof(wszTempPath), wszTempPath);
		GetTempFileNameW(wszTempPath, L"mx_", TRUE, wszTempFile);
		FILE *f = _wfopen(wszTempFile, L"wb");
		if (f != nullptr) {
			fwrite(&fih, sizeof(BITMAPFILEHEADER), 1, f);
			fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, f);
			fwrite(pData, sizeof(unsigned char), dataLen, f);
			fclose(f);

			SendMessage(hwndRc, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Image_Load(wszTempFile));
			DeleteFileW(wszTempFile);
		}

		QRcode_free(pQR);
	}

	void SetSuccess()
	{
		m_bSucceeded = true;
	}
};

static INT_PTR CALLBACK LaunchQrDialog(void *param)
{
	auto *ppro = (CMaxProto *)param;
	if (ppro == nullptr)
		return 0;

	if (auto *pDlg = ppro->m_pQRDlg) {
		SetForegroundWindow(pDlg->GetHwnd());
		SetActiveWindow(pDlg->GetHwnd());
	}
	else {
		ppro->m_pQRDlg = new CMaxQRDlg(ppro);
		ppro->m_pQRDlg->Show();
	}
	return 0;
}

static INT_PTR CALLBACK CloseQrDialogSync(void *param)
{
	auto *ppro = (CMaxProto *)param;
	if (ppro != nullptr && ppro->m_pQRDlg != nullptr)
		ppro->m_pQRDlg->Close();
	return 0;
}

static bool sttHasNonAscii(const wchar_t *pwsz)
{
	if (pwsz == nullptr)
		return false;
	for (const wchar_t *p = pwsz; *p; ++p)
		if (*p > 0x7F)
			return true;
	return false;
}

static const wchar_t *sttBaseNameW(const wchar_t *pwszPath)
{
	if (pwszPath == nullptr)
		return L"file.bin";
	const wchar_t *p1 = wcsrchr(pwszPath, L'\\');
	const wchar_t *p2 = wcsrchr(pwszPath, L'/');
	const wchar_t *p = (p1 && p2) ? (p1 > p2 ? p1 : p2) : (p1 ? p1 : p2);
	return (p && p[1]) ? (p + 1) : pwszPath;
}

static CMStringW sttDirNameW(const wchar_t *pwszPath)
{
	if (pwszPath == nullptr || pwszPath[0] == 0)
		return CMStringW();
	const wchar_t *p1 = wcsrchr(pwszPath, L'\\');
	const wchar_t *p2 = wcsrchr(pwszPath, L'/');
	const wchar_t *p = (p1 && p2) ? (p1 > p2 ? p1 : p2) : (p1 ? p1 : p2);
	if (p == nullptr)
		return CMStringW();
	return CMStringW(pwszPath, (int)(p - pwszPath));
}

static bool sttIsImageFileForCompress(const wchar_t *pwszPath)
{
	if (pwszPath == nullptr || pwszPath[0] == 0)
		return false;
	const wchar_t *pExt = wcsrchr(pwszPath, L'.');
	if (pExt == nullptr || pExt[1] == 0)
		return false;
	++pExt;
	return mir_wstrcmpi(pExt, L"jpg") == 0 || mir_wstrcmpi(pExt, L"jpeg") == 0 || mir_wstrcmpi(pExt, L"png") == 0
		|| mir_wstrcmpi(pExt, L"webp") == 0 || mir_wstrcmpi(pExt, L"bmp") == 0 || mir_wstrcmpi(pExt, L"gif") == 0
		|| mir_wstrcmpi(pExt, L"heic") == 0 || mir_wstrcmpi(pExt, L"heif") == 0 || mir_wstrcmpi(pExt, L"tif") == 0
		|| mir_wstrcmpi(pExt, L"tiff") == 0;
}

static bool sttBuildMultipartBody(const wchar_t *pwszPath, const char *szFieldName, CMStringA &boundary, MBinBuffer &body)
{
	if (!pwszPath || !*pwszPath)
		return false;
	if (szFieldName == nullptr || szFieldName[0] == 0)
		szFieldName = "file";

	FILE *in = _wfopen(pwszPath, L"rb");
	if (in == nullptr)
		return false;

	fseek(in, 0, SEEK_END);
	long cbFile = ftell(in);
	fseek(in, 0, SEEK_SET);
	if (cbFile <= 0) {
		fclose(in);
		return false;
	}

	mir_ptr<uint8_t> fileData((uint8_t *)mir_alloc((size_t)cbFile));
	size_t cbRead = fread(fileData, 1, (size_t)cbFile, in);
	fclose(in);
	if (cbRead != (size_t)cbFile)
		return false;

	boundary.Format("----MaxBoundary%08X%08X", GetTickCount(), (unsigned)time(nullptr));

	ptrA fileNameUtf(mir_utf8encodeW(sttBaseNameW(pwszPath)));
	const char *szFileName = (fileNameUtf != nullptr && fileNameUtf[0] != 0) ? fileNameUtf.get() : "file.bin";

	CMStringA head;
	head.AppendFormat("--%s\r\n", boundary.c_str());
	head.AppendFormat("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n", szFieldName, szFileName);
	head.Append("Content-Type: application/octet-stream\r\n\r\n");

	CMStringA tail;
	tail.AppendFormat("\r\n--%s--\r\n", boundary.c_str());

	body.append(head.c_str(), head.GetLength());
	body.append(fileData, cbRead);
	body.append(tail.c_str(), tail.GetLength());
	return true;
}

static bool sttExtractPhotoTokenFromUploadJson(const CMStringA &jsonText, CMStringA &outPhotoToken)
{
	outPhotoToken.Empty();
	if (jsonText.IsEmpty())
		return false;

	JSONNode root = JSONNode::parse(jsonText.c_str());
	if (!root)
		return false;

	const JSONNode &directToken = root["token"];
	if (directToken.type() == JSON_STRING && !directToken.as_string().empty()) {
		outPhotoToken = directToken.as_string().c_str();
		return true;
	}

	const JSONNode &photos = root["photos"];
	if (photos.type() != JSON_NODE || photos.size() == 0)
		return false;

	for (auto it = photos.begin(); it != photos.end(); ++it) {
		const JSONNode &item = *it;
		if (item.type() != JSON_NODE)
			continue;
		const JSONNode &t = item["token"];
		if (t.type() == JSON_STRING && !t.as_string().empty()) {
			outPhotoToken = t.as_string().c_str();
			return true;
		}
	}

	return false;
}

static bool sttUploadFileMultipartViaWinHttp(HNETLIBUSER hNlu, const char *szUrl, const char *szToken, const wchar_t *pwszPath, const char *szFieldName, CMStringA *pOutBody)
{
	if (!szUrl || !*szUrl || !pwszPath || !*pwszPath)
		return false;

	CMStringA boundary;
	MBinBuffer body;
	if (!sttBuildMultipartBody(pwszPath, szFieldName, boundary, body))
		return false;

	ptrW wszUrl(mir_utf8decodeW(szUrl));
	if (!wszUrl || !wszUrl[0])
		return false;

	URL_COMPONENTSW uc = {};
	uc.dwStructSize = sizeof(uc);
	wchar_t wszHost[256] = {};
	wchar_t wszPath[4096] = {};
	uc.lpszHostName = wszHost;
	uc.dwHostNameLength = _countof(wszHost);
	uc.lpszUrlPath = wszPath;
	uc.dwUrlPathLength = _countof(wszPath);
	uc.dwSchemeLength = (DWORD)-1;
	if (!WinHttpCrackUrl(wszUrl, 0, 0, &uc) || uc.nScheme != INTERNET_SCHEME_HTTPS) {
		Netlib_Logf(hNlu, "Max: upload winhttp bad url %s", szUrl);
		return false;
	}

	HINTERNET hSession = WinHttpOpen(L"MirandaMax/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession)
		return false;

	DWORD timeoutMs = 20000;
	WinHttpSetTimeouts(hSession, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

	HINTERNET hConnect = WinHttpConnect(hSession, wszHost, uc.nPort, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		return false;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wszPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return false;
	}

	const bool hasBearer = (szToken != nullptr && szToken[0] != 0);
	ptrW wszToken(hasBearer ? mir_utf8decodeW(szToken) : nullptr);
	ptrW wszBoundary(mir_utf8decodeW(boundary));
	CMStringW headers;
	if (hasBearer)
		headers.AppendFormat(L"Authorization: Bearer %s\r\n", (wszToken && wszToken[0]) ? wszToken.get() : L"");
	headers.Append(L"Accept: */*\r\n");
	headers.Append(L"Connection: close\r\n");
	headers.AppendFormat(L"Content-Type: multipart/form-data; boundary=%s\r\n", (wszBoundary && wszBoundary[0]) ? wszBoundary.get() : L"");

	Netlib_Logf(hNlu, "Max: upload winhttp begin host=%S path=%S bytes=%u", wszHost, wszPath, (unsigned)body.length());
	DWORD t0 = GetTickCount();

	BOOL ok = WinHttpSendRequest(hRequest, headers, (DWORD)-1, (LPVOID)body.data(), (DWORD)body.length(), (DWORD)body.length(), 0);
	if (!ok)
		goto LBL_Fail;
	ok = WinHttpReceiveResponse(hRequest, nullptr);
	if (!ok)
		goto LBL_Fail;

	DWORD statusCode = 0, cb = sizeof(statusCode);
	ok = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &cb, WINHTTP_NO_HEADER_INDEX);
	if (!ok)
		goto LBL_Fail;

	{
		CMStringA bodyHead;
		CMStringA fullBody;
		char buf[256];
		while (true) {
			DWORD avail = 0, read = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &avail) || avail == 0)
				break;
			DWORD want = (avail < (DWORD)sizeof(buf)) ? avail : (DWORD)sizeof(buf);
			if (!WinHttpReadData(hRequest, buf, want, &read) || read == 0)
				break;
			fullBody.Append(buf, read);
			if (bodyHead.GetLength() < 512)
				bodyHead.Append(buf, min((int)read, 512 - bodyHead.GetLength()));
		}
		if (pOutBody != nullptr)
			*pOutBody = fullBody;

		if (statusCode < 200 || statusCode >= 300) {
			bodyHead.Truncate(min(bodyHead.GetLength(), 512));
			Netlib_Logf(hNlu, "Max: upload winhttp HTTP %u (url=%s, body=%s)", (unsigned)statusCode, szUrl, bodyHead.IsEmpty() ? "(empty)" : bodyHead.c_str());
			goto LBL_Fail;
		}
		if (pOutBody != nullptr && !fullBody.IsEmpty()) {
			CMStringA head(fullBody, min(fullBody.GetLength(), 256));
			Netlib_Logf(hNlu, "Max: upload winhttp response head=%s", head.c_str());
		}
	}

	Netlib_Logf(hNlu, "Max: upload winhttp ok status=%u elapsed=%u", (unsigned)statusCode, (unsigned)(GetTickCount() - t0));
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return true;

LBL_Fail:
	Netlib_Logf(hNlu, "Max: upload winhttp failed gle=%u elapsed=%u", (unsigned)GetLastError(), (unsigned)(GetTickCount() - t0));
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return false;
}

static bool sttUploadFileMultipartViaNetlib(HNETLIBUSER hNlu, const char *szUrl, const char *szToken, const wchar_t *pwszPath, const char *szFieldName, CMStringA *pOutBody)
{
	if (hNlu == nullptr || !szUrl || !*szUrl || !pwszPath || !*pwszPath)
		return false;

	CMStringA boundary;
	MBinBuffer body;
	if (!sttBuildMultipartBody(pwszPath, szFieldName, boundary, body))
		return false;

	MHttpRequest req(REQUEST_POST);
	req.m_szUrl = szUrl;
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_SSL;
	req.timeout = 20000;
	if (szToken != nullptr && szToken[0] != 0)
		req.AddHeader("Authorization", CMStringA("Bearer ") + szToken);
	req.AddHeader("Accept", "*/*");
	req.AddHeader("Connection", "close");
	req.AddHeader("Content-Type", CMStringA("multipart/form-data; boundary=") + boundary);
	req.SetData(body.data(), body.length());
	Netlib_Logf(hNlu, "Max: upload netlib begin url=%s bytes=%u", szUrl, (unsigned)body.length());
	DWORD t0 = GetTickCount();

	NLHR_PTR reply(Netlib_HttpTransaction(hNlu, &req));
	if (!reply) {
		Netlib_Logf(hNlu, "Max: upload netlib no HTTP response (url=%s, elapsed=%u)", szUrl, (unsigned)(GetTickCount() - t0));
		return false;
	}

	if (reply->resultCode < 200 || reply->resultCode >= 300) {
		CMStringA bodyHead(reply->body, min(reply->body.GetLength(), 512));
		Netlib_Logf(hNlu, "Max: upload netlib HTTP %d (url=%s, body=%s, elapsed=%u)",
			reply->resultCode, szUrl, bodyHead.IsEmpty() ? "(empty)" : bodyHead.c_str(), (unsigned)(GetTickCount() - t0));
		return false;
	}

	if (pOutBody != nullptr)
		*pOutBody = reply->body;

	Netlib_Logf(hNlu, "Max: upload netlib ok status=%d elapsed=%u", reply->resultCode, (unsigned)(GetTickCount() - t0));
	return true;
}

static bool sttUploadFileMultipart(HNETLIBUSER hNlu, const char *szUrl, const char *szToken, const wchar_t *pwszPath, const char *szFieldName, CMStringA *pOutBody)
{
	if (pOutBody != nullptr)
		pOutBody->Empty();

	if (sttUploadFileMultipartViaWinHttp(hNlu, szUrl, szToken, pwszPath, szFieldName, pOutBody))
		return true;

	Netlib_Logf(hNlu, "Max: upload fallback to netlib url=%s", szUrl ? szUrl : "(null)");
	return sttUploadFileMultipartViaNetlib(hNlu, szUrl, szToken, pwszPath, szFieldName, pOutBody);
}

static bool sttIsAttachmentNotReadyError(const CMStringA &pendingResponse)
{
	if (pendingResponse.IsEmpty())
		return false;

	JSONNode resp = JSONNode::parse(pendingResponse.c_str());
	if (!resp)
		return false;

	const JSONNode &pl = resp["payload"];
	if (pl.type() != JSON_NODE)
		return false;

	const JSONNode &err = pl["error"];
	if (err.type() == JSON_STRING) {
		const char *szErr = err.as_string().c_str();
		if (!mir_strcmp(szErr, "attachment.not.ready") || !mir_strcmp(szErr, "attachment.processing"))
			return true;
	}

	const JSONNode &msg = pl["message"];
	if (msg.type() == JSON_STRING) {
		const char *szMsg = msg.as_string().c_str();
		if (strstr(szMsg, "attachment.file.not.processed") != nullptr)
			return true;
	}

	return false;
}

static int64_t sttGetFileSizeW(const wchar_t *pwszPath)
{
	if (pwszPath == nullptr || pwszPath[0] == 0)
		return 0;
	struct _stati64 st = {};
	if (_wstati64(pwszPath, &st) != 0)
		return 0;
	return st.st_size > 0 ? st.st_size : 0;
}

static void sttBroadcastFileProgress(CMaxFileSendCtx *ctx, int iCurrentFile, int64_t currentFileSize, int64_t currentFileProgress, int64_t totalBytes, int64_t totalProgress)
{
	if (ctx == nullptr)
		return;

	PROTOFILETRANSFERSTATUS fts = {};
	fts.hContact = ctx->hContact;
	fts.flags = PFTS_SENDING | PFTS_UNICODE;
	static thread_local std::vector<wchar_t*> s_files;
	s_files.clear();
	s_files.reserve(ctx->files.size() + 1);
	for (const auto &f : ctx->files)
		s_files.push_back((wchar_t*)f.c_str());
	s_files.push_back(nullptr);
	fts.pszFiles.w = s_files.data();
	fts.totalFiles = (int)ctx->files.size();
	fts.currentFileNumber = iCurrentFile;
	fts.totalBytes = totalBytes;
	fts.totalProgress = totalProgress;
	fts.currentFileSize = currentFileSize;
	fts.currentFileProgress = currentFileProgress;

	if (!ctx->files.empty()) {
		const wchar_t *curPath = ctx->files[(size_t)iCurrentFile].c_str();
		fts.szCurrentFile.w = (wchar_t *)sttBaseNameW(curPath);
		static thread_local CMStringW s_workDir;
		s_workDir = sttDirNameW(ctx->files[0].c_str());
		if (!s_workDir.IsEmpty())
			fts.szWorkingDir.w = (wchar_t *)s_workDir.c_str();
	}
	ctx->pProto->ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)ctx, (LPARAM)&fts);
}

static void sttIngestSentFileAck(CMaxProto *pProto, const char *szChatId, const CMStringA &pendingResponse, const std::vector<CMStringW> *pSentFiles = nullptr)
{
	if (pProto == nullptr || szChatId == nullptr || szChatId[0] == 0 || pendingResponse.IsEmpty())
		return;

	JSONNode resp = JSONNode::parse(pendingResponse.c_str());
	if (!resp)
		return;

	const JSONNode &pl = resp["payload"];
	const JSONNode &srcMsg = pl["message"];
	if (srcMsg.type() != JSON_NODE)
		return;

	JSONNode msg(srcMsg);
	if (pSentFiles != nullptr && !pSentFiles->empty()) {
		JSONNode &att = msg["attaches"];
		if (att.type() == JSON_ARRAY) {
			size_t photoIdx = 0;
			for (unsigned i = 0; i < att.size() && photoIdx < pSentFiles->size(); ++i) {
				JSONNode &a = att[(json_index_t)i];
				if (a.type() != JSON_NODE)
					continue;
				if (a["_type"].type() != JSON_STRING || mir_strcmpi(a["_type"].as_string().c_str(), "PHOTO"))
					continue;

				const wchar_t *pwszPath = (*pSentFiles)[photoIdx].c_str();
				const wchar_t *pwszBase = sttBaseNameW(pwszPath);
				if (a["name"].type() == JSON_NULL && pwszBase != nullptr && pwszBase[0] != 0) {
					ptrA nameUtf(mir_utf8encodeW(pwszBase));
					if (nameUtf != nullptr && nameUtf[0] != 0)
						a << CHAR_PARAM("name", nameUtf.get());
				}

				if (a["size"].type() == JSON_NULL) {
					int64_t sz = sttGetFileSizeW(pwszPath);
					if (sz > 0)
						a << INT64_PARAM("size", sz);
				}
				photoIdx++;
			}
		}
	}

	pProto->IngestMaxMessageJson(msg, szChatId);
}

/////////////////////////////////////////////////////////////////////////////////////////

CMaxProto::CMaxProto(const char *szModuleName, const wchar_t *ptszUserName) :
	PROTO<CMaxProto>(szModuleName, ptszUserName)
{
	m_hWaitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	HookProtoEvent(ME_OPT_INITIALISE, &CMaxProto::OnOptionsInit);
	HookProtoEvent(ME_LANGPACK_CHANGED, &CMaxProto::OnLangpackChanged);

	NETLIBUSER nlu = {};
	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	CMStringW descr;
	descr.Format(TranslateT("%s connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	m_hProtoIcon = g_plugin.getIconHandle(IDI_MAIN);

	if (getWStringA(DB_KEY_DEFAULT_GROUP) == nullptr)
		setWString(DB_KEY_DEFAULT_GROUP, L"Max");
	Clist_GroupCreate(0, GetDefaultGroupW());
	RegisterChatModule();

	CreateProtoService(PS_GETAVATARINFO, &CMaxProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CMaxProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CMaxProto::SvcGetMyAvatar);
	CreateProtoService(PS_OFFLINEFILE, &CMaxProto::SvcOfflineFile);
	CreateProtoService(PS_MENU_LOADHISTORY, &CMaxProto::SvcLoadServerHistory);
	CreateProtoService(PS_CAN_EMPTY_HISTORY, &CMaxProto::SvcCanEmptyHistory);
	CreateProtoService(PS_EMPTY_SRV_HISTORY, &CMaxProto::SvcEmptyServerHistory);
}

CMaxProto::~CMaxProto()
{
	DisconnectGateway();
	FreeWsInflater();
	if (m_hWaitEvent)
		CloseHandle(m_hWaitEvent);
}

int CMaxProto::OnLangpackChanged(WPARAM, LPARAM)
{
	debugLogA("Max: langpack changed event received");
	// If we're connected, reconnect WS so the next handshake uses the current locale mapping.
	// Do not change the visible status; just restart the gateway session.
	if (m_iStatus == ID_STATUS_OFFLINE)
		return 0;
	if (!HasLoginToken())
		return 0;

	const char *raw = Langpack_GetDefaultLocaleName();
	const char *want = (raw && (raw[0] == 'r' || raw[0] == 'R') && (raw[1] == 'u' || raw[1] == 'U')) ? "ru" : "en";
	if (!m_wsLocale.IsEmpty() && !mir_strcmpi(m_wsLocale.c_str(), want))
		return 0;

	debugLogA("Max: langpack changed, reconnecting gateway (locale=%s)", want);
	DisconnectGateway();

	m_bTerminated = false;
	m_hConnThread = ForkThreadEx(&CMaxProto::ConnectionWorker, nullptr, nullptr);
	return 0;
}

INT_PTR CMaxProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSG | PF1_CHAT | PF1_EXTSEARCH | PF1_ADDSEARCHRES | PF1_SERVERCLIST | PF1_FILE;

	case PFLAGNUM_2:
		return PF2_ONLINE;

	case PFLAGNUM_3:
		return PF2_ONLINE;

	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_AVATARS | PF4_SERVERMSGID | PF4_DELETEFORALL | PF4_SUPPORTTYPING | PF4_OFFLINEFILES | PF4_SERVERFORMATTING;

	case PFLAG_UNIQUEIDTEXT:
	{
		static wchar_t s_wszUid[96];
		mir_wstrcpy(s_wszUid, TranslateT("Max user ID"));
		return (INT_PTR)s_wszUid;
	}
	}
	return 0;
}

int CMaxProto::SetStatus(int iNewStatus)
{
	auto syncFavoritesPresence = [&](int protoStatus) {
		ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
		for (auto &hContact : AccContacts()) {
			if (isChatRoom(hContact))
				continue;
			if (IsMaxBotMirrorContact(hContact)) {
				setWord(hContact, "Status", (protoStatus == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
				continue;
			}
			if (myUid == nullptr || myUid[0] == 0)
				continue;
			ptrA uid(getStringA(hContact, DB_KEY_MAX_UID));
			if (uid == nullptr || uid[0] == 0 || mir_strcmp(uid, myUid))
				continue;
			setWord(hContact, "Status", (protoStatus == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
		}
	};

	if (iNewStatus == ID_STATUS_INVISIBLE)
		iNewStatus = ID_STATUS_ONLINE;
	if (iNewStatus != ID_STATUS_OFFLINE && iNewStatus != ID_STATUS_ONLINE)
		iNewStatus = ID_STATUS_ONLINE;

	int iOldStatus = m_iStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		DisconnectGateway();
		m_iStatus = ID_STATUS_OFFLINE;
		syncFavoritesPresence(m_iStatus);
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		return 0;
	}

	if (m_hConnThread)
		return 0;

	m_bTerminated = false;
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	m_hConnThread = ForkThreadEx(&CMaxProto::ConnectionWorker, nullptr, nullptr);
	if (!m_hConnThread)
		return 1;

	iOldStatus = m_iStatus;
	m_iStatus = iNewStatus;
	syncFavoritesPresence(m_iStatus);
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	return 0;
}

bool CMaxProto::JsonIndicatesMaxBot(const JSONNode &c) const
{
	const JSONNode &opts = c["options"];
	if (opts.type() != JSON_ARRAY)
		return false;
	for (unsigned i = 0; i < opts.size(); i++) {
		if (opts[i].type() != JSON_STRING)
			continue;
		CMStringA v(opts[i].as_string().c_str());
		v.MakeUpper();
		if (v == "BOT")
			return true;
	}
	return false;
}

bool CMaxProto::IsMaxBotMirrorContact(MCONTACT hContact)
{
	return hContact != 0 && getByte(hContact, DB_KEY_MAX_IS_BOT, 0) != 0;
}

int CMaxProto::SendMsg(MCONTACT hContact, MEVENT, const char *msg)
{
	if (hContact == 0 || msg == nullptr || msg[0] == 0)
		return 0;

	int hProcess = (int)InterlockedIncrement(&m_iSendMsgSeq);
	if (hProcess <= 0)
		hProcess = 1;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		auto *ctx = new CMaxMsgAckCtx;
		ctx->pProto = this;
		ctx->hContact = hContact;
		ctx->result = ACKRESULT_FAILED;
		ctx->hProcess = hProcess;
		ctx->errText = TranslateT("Gateway is not connected");
		ForkThread(&CMaxProto::MessageAckWorker, ctx);
		return hProcess;
	}

	CMStringA chatId(GetOrResolveDialogChatId(hContact));
	if (chatId.IsEmpty()) {
		auto *ctx = new CMaxMsgAckCtx;
		ctx->pProto = this;
		ctx->hContact = hContact;
		ctx->result = ACKRESULT_FAILED;
		ctx->hProcess = hProcess;
		ctx->errText = TranslateT("Missing chat id for this contact");
		ForkThread(&CMaxProto::MessageAckWorker, ctx);
		return hProcess;
	}

	CMStringA serverMsgId;
	if (!ApiSendMessage(m_pGateway, chatId.c_str(), msg, &serverMsgId)) {
		CMStringW err = FormatLastError();
		if (!err.IsEmpty())
			NotifyUser(TranslateT("Max"), err.c_str());
		auto *ctx = new CMaxMsgAckCtx;
		ctx->pProto = this;
		ctx->hContact = hContact;
		ctx->result = ACKRESULT_FAILED;
		ctx->hProcess = hProcess;
		ctx->errText = TranslateT("Message send failed");
		ForkThread(&CMaxProto::MessageAckWorker, ctx);
		return hProcess;
	}

	debugLogA("Max: send ok chat=%s id=%s", chatId.c_str(), serverMsgId.IsEmpty() ? "(none)" : serverMsgId.c_str());
	auto *ctx = new CMaxMsgAckCtx;
	ctx->pProto = this;
	ctx->hContact = hContact;
	ctx->result = ACKRESULT_SUCCESS;
	ctx->hProcess = hProcess;
	ctx->msgId = serverMsgId;
	ForkThread(&CMaxProto::MessageAckWorker, ctx);
	return hProcess;
}

HANDLE CMaxProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	debugLogA("Max: SendFile called h=%u desc=%S filesPtr=%p first=%S",
		(unsigned)hContact,
		(szDescription && szDescription[0]) ? szDescription : L"(empty)",
		ppszFiles,
		(ppszFiles && ppszFiles[0]) ? ppszFiles[0] : L"(null)");

	if (hContact == 0 || ppszFiles == nullptr || ppszFiles[0] == nullptr)
		return INVALID_HANDLE_VALUE;

	auto *ctx = new CMaxFileSendCtx;
	ctx->pProto = this;
	ctx->hContact = hContact;
	if (szDescription != nullptr)
		ctx->wszDesc = szDescription;

	for (int i = 0; ppszFiles[i] != nullptr; ++i) {
		if (ppszFiles[i][0] == 0)
			continue;
		ctx->files.push_back(CMStringW(ppszFiles[i]));
	}

	if (ctx->files.empty()) {
		debugLogA("Max: SendFile rejected (empty file list) h=%u", (unsigned)hContact);
		delete ctx;
		return INVALID_HANDLE_VALUE;
	}

	ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)ctx);
	ForkThread(&CMaxProto::FileSendWorker, ctx);
	return (HANDLE)ctx;
}

void __cdecl CMaxProto::FileSendWorker(void *param)
{
	std::unique_ptr<CMaxFileSendCtx> ctx((CMaxFileSendCtx *)param);
	if (!ctx || ctx->pProto != this || ctx->hContact == 0)
		return;

	CMStringW workDir = sttDirNameW(ctx->files.empty() ? nullptr : ctx->files[0].c_str());
	debugLogA("Max: file send start h=%u files=%u compress=%d",
		(unsigned)ctx->hContact, (unsigned)ctx->files.size(),
		getByte("CompressFiles", 1) != 0 ? 1 : 0);

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ctx.get());
		return;
	}

	CMStringA chatId(GetOrResolveDialogChatId(ctx->hContact));
	if (chatId.IsEmpty()) {
		ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ctx.get());
		return;
	}

	int64_t totalBytes = 0;
	for (const auto &it : ctx->files)
		totalBytes += sttGetFileSizeW(it.c_str());
	int64_t totalProgress = 0;

	const bool bCompressAll = getByte("CompressFiles", 1) != 0;
	bool bBatchPhotos = bCompressAll && ctx->files.size() > 1;
	if (bBatchPhotos) {
		for (const auto &it : ctx->files) {
			if (!sttIsImageFileForCompress(it.c_str())) {
				bBatchPhotos = false;
				break;
			}
		}
	}

	if (bBatchPhotos) {
		std::vector<CMStringA> photoTokens;
		photoTokens.reserve(ctx->files.size());
		ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)ctx.get());

		for (size_t i = 0; i < ctx->files.size(); ++i) {
			ctx->iCurrent = (int)i;
			const wchar_t *pwszFile = ctx->files[i].c_str();
			const int64_t cbFile = sttGetFileSizeW(pwszFile);
			if (_waccess(pwszFile, 0) != 0) {
				bBatchPhotos = false;
				break;
			}
			// Keep transfer UI informed (avoids "No data transferred" and default placeholder paths).
			sttBroadcastFileProgress(ctx.get(), (int)i, cbFile, 0, totalBytes, totalProgress);

			CMStringA uploadUrl, uploadToken;
			int64_t fileId = 0;
			if (!ApiRequestPhotoUpload(m_pGateway, uploadUrl, uploadToken, fileId)) {
				bBatchPhotos = false;
				break;
			}

			CMStringA uploadReplyBody;
			if (!sttUploadFileMultipart(m_hNetlibUser, uploadUrl.c_str(), nullptr, pwszFile, "data", &uploadReplyBody)) {
				bBatchPhotos = false;
				break;
			}

			CMStringA photoTokenFromUpload;
			if (!sttExtractPhotoTokenFromUploadJson(uploadReplyBody, photoTokenFromUpload)) {
				bBatchPhotos = false;
				break;
			}
			photoTokens.push_back(photoTokenFromUpload);

			totalProgress += cbFile;
			sttBroadcastFileProgress(ctx.get(), (int)i, cbFile, cbFile, totalBytes, totalProgress);
		}

		if (bBatchPhotos) {
			ptrA descUtf(!ctx->wszDesc.IsEmpty() ? mir_utf8encodeW(ctx->wszDesc) : nullptr);
			CMStringA serverMsgId;
			if (ApiSendMultiPhotoMessage(m_pGateway, chatId.c_str(), photoTokens, descUtf != nullptr ? descUtf.get() : nullptr, &serverMsgId)) {
				sttIngestSentFileAck(this, chatId.c_str(), m_szPendingResponse, &ctx->files);
				debugLogA("Max: multi-photo batch sent chat=%s count=%u msg=%s", chatId.c_str(),
					(unsigned)photoTokens.size(), serverMsgId.IsEmpty() ? "(none)" : serverMsgId.c_str());
				ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)ctx.get());
				return;
			}
		}
		// fallback to legacy per-file pipeline if batch mode failed
		totalProgress = 0;
	}

	for (size_t i = 0; i < ctx->files.size(); ++i) {
		ctx->iCurrent = (int)i;
		const wchar_t *pwszFile = ctx->files[i].c_str();
		const int64_t cbFile = sttGetFileSizeW(pwszFile);
		if (_waccess(pwszFile, 0) != 0) {
			ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ctx.get());
			return;
		}
		sttBroadcastFileProgress(ctx.get(), (int)i, cbFile, 0, totalBytes, totalProgress);

		const bool bCompress = bCompressAll;
		bool bWantPhoto = bCompress && sttIsImageFileForCompress(pwszFile);

		CMStringA uploadUrl, uploadToken;
		int64_t fileId = 0;
		bool usedPhotoUpload = false;
		if (bWantPhoto) {
			if (ApiRequestPhotoUpload(m_pGateway, uploadUrl, uploadToken, fileId))
				usedPhotoUpload = true;
			else
				debugLogA("Max: photo upload slot request failed, fallback to file upload");
		}
		if (!usedPhotoUpload) {
			if (!ApiRequestFileUpload(m_pGateway, uploadUrl, uploadToken, fileId)) {
				ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ctx.get());
				return;
			}
		}

		ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)ctx.get());
		CMStringA uploadReplyBody;
		const char *formField = usedPhotoUpload ? "data" : "file";
		const char *authToken = (usedPhotoUpload ? nullptr : uploadToken.c_str());
		if (!sttUploadFileMultipart(m_hNetlibUser, uploadUrl.c_str(), authToken, pwszFile, formField, &uploadReplyBody)) {
			ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ctx.get());
			return;
		}
		CMStringA photoTokenFromUpload;
		if (usedPhotoUpload) {
			if (sttExtractPhotoTokenFromUploadJson(uploadReplyBody, photoTokenFromUpload))
				debugLogA("Max: photo token extracted from upload response");
			else {
				CMStringA head(uploadReplyBody, min(uploadReplyBody.GetLength(), 256));
				debugLogA("Max: photo token NOT extracted (upload body head=%s)", head.IsEmpty() ? "(empty)" : head.c_str());
			}
		}

		const bool addText = (!ctx->wszDesc.IsEmpty() && i == 0);
		ptrA descUtf(addText ? mir_utf8encodeW(ctx->wszDesc) : nullptr);
		CMStringA serverMsgId;
		bool bUsePhotoAttach = usedPhotoUpload && !photoTokenFromUpload.IsEmpty();
		bool bSent = false;
		for (int retry = 0; retry < 8; ++retry) {
			if (ApiSendFileMessage(m_pGateway, chatId.c_str(), fileId, bUsePhotoAttach,
				bUsePhotoAttach ? photoTokenFromUpload.c_str() : nullptr,
				descUtf != nullptr ? descUtf.get() : nullptr, &serverMsgId)) {
				bSent = true;
				break;
			}

			// PHOTO attach may be unsupported on some backend versions; fallback to FILE.
			if (bUsePhotoAttach) {
				debugLogA("Max: photo attach rejected, fallback to file attach chat=%s fileId=%lld", chatId.c_str(), (long long)fileId);
				bUsePhotoAttach = false;
				// If we used PHOTO_UPLOAD and PHOTO attach still rejected, retrying with FILE attach requires FILE_UPLOAD token+id.
				// Fall back to FILE_UPLOAD pipeline.
				uploadUrl.Empty();
				uploadToken.Empty();
				fileId = 0;
				if (!ApiRequestFileUpload(m_pGateway, uploadUrl, uploadToken, fileId))
					break;
				if (!sttUploadFileMultipart(m_hNetlibUser, uploadUrl.c_str(), uploadToken.c_str(), pwszFile, "file", nullptr))
					break;
				continue;
			}

			// Upload endpoint can return HTTP 200 before backend processing finishes.
			// Retry attach send when server explicitly says attachment is not ready yet.
			if (!sttIsAttachmentNotReadyError(m_szPendingResponse))
				break;

			DWORD waitMs = 250 + retry * 200;
			debugLogA("Max: file attach not ready chat=%s fileId=%lld retry=%d wait=%u", chatId.c_str(), (long long)fileId, retry + 1, (unsigned)waitMs);
			Sleep(waitMs);
		}

		if (!bSent) {
			ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ctx.get());
			return;
		}

		totalProgress += cbFile;
		sttBroadcastFileProgress(ctx.get(), (int)i, cbFile, cbFile, totalBytes, totalProgress);
		std::vector<CMStringW> sentOne;
		sentOne.push_back(ctx->files[i]);
		sttIngestSentFileAck(this, chatId.c_str(), m_szPendingResponse, &sentOne);

		debugLogA("Max: file sent chat=%s local=%S fileId=%lld mode=%s msg=%s", chatId.c_str(), pwszFile, (long long)fileId,
			bUsePhotoAttach ? "PHOTO" : "FILE", serverMsgId.IsEmpty() ? "(none)" : serverMsgId.c_str());
		if (i + 1 < ctx->files.size())
			ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)ctx.get());
	}

	ProtoBroadcastAck(ctx->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)ctx.get());
}

int CMaxProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact == 0 || (type != PROTOTYPE_SELFTYPING_ON && type != PROTOTYPE_SELFTYPING_OFF))
		return 0;
	if (!WaitForGatewayReady() || m_pGateway == nullptr)
		return 0;

	CMStringA chatId = GetOrResolveDialogChatId(hContact, false);
	if (chatId.IsEmpty())
		return 0;

	ApiSendTyping(m_pGateway, chatId.c_str(), type == PROTOTYPE_SELFTYPING_ON);
	return 0;
}

void CMaxProto::OnEventEdited(MCONTACT hContact, MEVENT, const DBEVENTINFO &dbei)
{
	if (hContact == 0 || dbei.szId == nullptr || dbei.pBlob == nullptr)
		return;
	if (!(dbei.flags & DBEF_SENT))
		return;

	CMStringA chatId(GetOrResolveDialogChatId(hContact));
	if (chatId.IsEmpty())
		return;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		debugLogA("Max: edit skipped (gateway not connected) chat=%s msg=%s", chatId.c_str(), dbei.szId);
		return;
	}

	if (!ApiEditMessage(m_pGateway, chatId.c_str(), dbei.szId, dbei.pBlob))
		debugLogA("Max: edit failed chat=%s msg=%s", chatId.c_str(), dbei.szId);
	else
		debugLogA("Max: edit ok chat=%s msg=%s", chatId.c_str(), dbei.szId);
}

void CMaxProto::OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags)
{
	if (hContact == 0 || !(flags & CDF_DEL_HISTORY))
		return;

	DB::EventInfo dbei(hDbEvent, false);
	if (!dbei || mir_strcmp(dbei.szModule, m_szModuleName))
		return;
	if (dbei.eventType != EVENTTYPE_MESSAGE && dbei.eventType != EVENTTYPE_FILE)
		return;
	if (dbei.szId == nullptr || dbei.szId[0] == 0)
		return;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		debugLogA("Max: delete msg skipped (gateway not connected) id=%s", dbei.szId);
		return;
	}

	CMStringA chatId;
	if (isChatRoom(hContact)) {
		SESSION_INFO *si = Chat_Find(hContact, m_szModuleName);
		if (si == nullptr || si->ptszID == nullptr || si->ptszID[0] == 0)
			return;
		ptrA cidUtf(mir_u2a(si->ptszID));
		if (cidUtf == nullptr || cidUtf[0] == 0)
			return;
		chatId = cidUtf.get();
	}
	else
		chatId = GetOrResolveDialogChatId(hContact, false);

	if (chatId.IsEmpty()) {
		debugLogA("Max: delete msg skipped (no chat id) msg=%s", dbei.szId);
		return;
	}

	const bool forEveryone = (flags & CDF_FOR_EVERYONE) != 0;
	const bool forMe = !forEveryone;
	CMStringA serverMsgId(dbei.szId);
	if (dbei.eventType == EVENTTYPE_FILE) {
		// File events are stored as "<msgId>:file:<fileId>", but server delete API expects original message id.
		const char *pSep = strstr(serverMsgId.c_str(), ":file:");
		if (pSep != nullptr)
			serverMsgId.Truncate((int)(pSep - serverMsgId.c_str()));
	}
	if (serverMsgId.IsEmpty())
		return;

	if (!ApiDeleteMessages(m_pGateway, chatId.c_str(), serverMsgId.c_str(), forMe))
		debugLogA("Max: delete msg API failed chat=%s id=%s forMe=%d", chatId.c_str(), serverMsgId.c_str(), forMe ? 1 : 0);
	else
		debugLogA("Max: delete msg ok chat=%s id=%s forMe=%d", chatId.c_str(), serverMsgId.c_str(), forMe ? 1 : 0);
}

void CMaxProto::OnSendOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &, void *)
{
	// SRFile creates a synthetic local "sent file" event (e.g. "3 files").
	// Max receives canonical sent attachments from server echo (opcode 64), so keep local synthetic event out of history.
	dbei.bTemporary = true;
}

void __cdecl CMaxProto::MessageAckWorker(void *param)
{
	std::unique_ptr<CMaxMsgAckCtx> ctx((CMaxMsgAckCtx *)param);
	if (!ctx || ctx->pProto == nullptr)
		return;

	// Give SRMM time to register msgQueue entry after SendMsg returns.
	Sleep(120);
	if (ctx->result == ACKRESULT_SUCCESS)
		ctx->pProto->ProtoBroadcastAck(ctx->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ctx->hProcess,
			ctx->msgId.IsEmpty() ? 0 : (LPARAM)ctx->msgId.c_str());
	else
		ctx->pProto->ProtoBroadcastAck(ctx->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)ctx->hProcess,
			(LPARAM)ctx->errText.c_str());
}

struct CMaxPhoneSearchCtx
{
	CMaxProto *p = nullptr;
	wchar_t *wszPhone = nullptr;
	/// Must match the handle returned from SearchBasic (Find/Add matches ACKDATA::hProcess to it).
	HANDLE hSearch = nullptr;
};

static CMStringA MaxJsonIdStrLocal(const JSONNode &n)
{
	if (n.type() == JSON_NULL)
		return "";
	if (n.type() == JSON_NUMBER) {
		CMStringA s;
		s.Format("%.0f", n.as_float());
		return s;
	}
	return CMStringA(n.as_string().c_str());
}

static bool MaxNormalizePhoneWtoUtf8(const wchar_t *in, CMStringA &outUtf8)
{
	if (in == nullptr || in[0] == 0)
		return false;

	CMStringW digits;
	bool hadLeadingPlus = false;
	for (const wchar_t *p = in; *p; ++p) {
		if ((*p == L'+' || *p == 0xFF0B) && digits.IsEmpty()) { // fullwidth plus
			hadLeadingPlus = true;
			continue;
		}
		if (*p >= L'0' && *p <= L'9')
			digits.AppendChar(*p);
	}

	if (digits.IsEmpty())
		return false;

	CMStringW normalized;
	if (hadLeadingPlus)
		normalized.Format(L"+%s", digits.c_str());
	else if (digits.GetLength() == 11 && digits[0] == L'8') {
		CMStringW tail8 = digits.Mid(1);
		normalized.Format(L"+7%s", tail8.c_str());
	}
	else if (digits.GetLength() == 10)
		normalized.Format(L"+7%s", digits.c_str());
	else
		normalized.Format(L"+%s", digits.c_str());

	ptrA utf(mir_utf8encodeW(normalized));
	if (utf == nullptr || utf[0] == 0)
		return false;
	outUtf8 = utf.get();
	return true;
}

void __cdecl CMaxProto::PhoneSearchWorker(void *param)
{
	auto *ctx = (CMaxPhoneSearchCtx *)param;
	if (ctx == nullptr)
		return;

	CMaxProto *p = ctx->p;
	const HANDLE hSearch = ctx->hSearch;
	ptrW wszPhone(ctx->wszPhone);
	mir_free(ctx);

	if (p == nullptr)
		return;

	CMStringA phoneUtf8;
	if (!MaxNormalizePhoneWtoUtf8(wszPhone, phoneUtf8)) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	if (!p->WaitForGatewayReady() || p->m_pGateway == nullptr) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	JSONNode contact(JSON_NULL);
	if (!p->ApiSearchByPhone(p->m_pGateway, phoneUtf8.c_str(), contact)) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	if (contact.type() != JSON_NODE || contact.size() == 0) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch, 0);
		return;
	}

	CMStringA uid = MaxJsonIdStrLocal(contact["id"]);
	if (uid.IsEmpty()) uid = MaxJsonIdStrLocal(contact["contactId"]);
	if (uid.IsEmpty()) uid = MaxJsonIdStrLocal(contact["userId"]);
	if (uid.IsEmpty()) {
		p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, hSearch, 0);
		return;
	}

	CMStringW fn, ln;
	p->FillNameFromMaxContactJson(contact, fn, ln);

	CMStringW nick;
	if (!fn.IsEmpty() || !ln.IsEmpty()) {
		nick = fn;
		if (!ln.IsEmpty()) {
			if (!nick.IsEmpty()) nick += L' ';
			nick += ln;
		}
	}
	if (nick.IsEmpty())
		nick.Format(L"User %S", uid.c_str());

	PROTOSEARCHRESULT psr = {};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;

	CMStringW uidw;
	uidw.Format(L"%S", uid.c_str());
	psr.id.w = mir_wstrdup(uidw);
	psr.firstName.w = mir_wstrdup(fn.c_str());
	psr.lastName.w = mir_wstrdup(ln.c_str());
	psr.nick.w = mir_wstrdup(nick.c_str());
	psr.email.w = mir_wstrdup(L"");

	p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, hSearch, (LPARAM)&psr);
	p->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch, 0);

	mir_free(psr.id.w);
	mir_free(psr.firstName.w);
	mir_free(psr.lastName.w);
	mir_free(psr.nick.w);
	mir_free(psr.email.w);
}

HANDLE CMaxProto::SearchBasic(const wchar_t *id)
{
	if (id == nullptr || id[0] == 0)
		return nullptr;
	if (m_iStatus == ID_STATUS_OFFLINE)
		return nullptr;

	auto *ctx = (CMaxPhoneSearchCtx *)mir_alloc(sizeof(CMaxPhoneSearchCtx));
	if (ctx == nullptr)
		return nullptr;
	ctx->p = this;
	ctx->wszPhone = mir_wstrdup(id);
	if (ctx->wszPhone == nullptr) {
		mir_free(ctx);
		return nullptr;
	}
	ctx->hSearch = (HANDLE)1;
	ForkThread(&CMaxProto::PhoneSearchWorker, ctx);
	return ctx->hSearch;
}

static INT_PTR CALLBACK MaxSearchDlgProc(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SetDlgItemTextW(GetParent(hwndDlg), 1408 /* IDC_BYCUSTOM */, TranslateT("Phone number"));
		return TRUE;
	}
	return FALSE;
}

HWND CMaxProto::CreateExtendedSearchUI(HWND owner)
{
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SEARCHUI), owner, MaxSearchDlgProc, (LPARAM)this);
}

HANDLE CMaxProto::SearchAdvanced(HWND owner)
{
	if (owner == nullptr)
		return nullptr;

	wchar_t wszPhone[128];
	GetDlgItemTextW(owner, 0, wszPhone, _countof(wszPhone));
	if (wszPhone[0] == 0)
		return nullptr;

	return SearchBasic(wszPhone);
}

MCONTACT CMaxProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr == nullptr || psr->cbSize < (int)sizeof(PROTOSEARCHRESULT))
		return 0;
	if (!(psr->flags & PSR_UNICODE) || psr->id.w == nullptr || psr->id.w[0] == 0)
		return 0;

	ptrA uidUtf(mir_utf8encodeW(psr->id.w));
	if (uidUtf == nullptr || uidUtf[0] == 0)
		return 0;

	CMStringA chatId;
	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	if (my != nullptr && my[0]) {
		uint64_t a = _strtoui64(my, nullptr, 10);
		uint64_t b = _strtoui64(uidUtf, nullptr, 10);
		if (b != 0) {
			unsigned long long x = (unsigned long long)(a ^ b);
			chatId.Format("%llu", x);
		}
	}

	const wchar_t *pfn = (psr->firstName.w && psr->firstName.w[0]) ? psr->firstName.w : L"";
	const wchar_t *pln = (psr->lastName.w && psr->lastName.w[0]) ? psr->lastName.w : L"";

	MCONTACT hContact = EnsureUserContact(uidUtf, pfn, pln, chatId.IsEmpty() ? nullptr : chatId.c_str());
	if (hContact == 0)
		return 0;

	setByte(hContact, DB_KEY_MAX_PEER_ORIGIN, MAX_PEER_ORIGIN_CONTACTS);

	if (flags & PALF_TEMPORARY)
		Contact::RemoveFromList(hContact);

	if (WaitForGatewayReady() && m_pGateway != nullptr) {
		if (!ApiAddContactOnServer(m_pGateway, uidUtf))
			debugLogA("Max: AddToList server opcode 34 failed or timed out uid=%s", uidUtf.get());
	}

	return hContact;
}

void CMaxProto::OnModulesLoaded()
{
	HookProtoEvent(ME_USERINFO_INITIALISE, &CMaxProto::OnUserInfoInit);
}

bool CMaxProto::ContactNeedsServerDisplayFetch(MCONTACT hContact)
{
	if (hContact == 0 || isChatRoom(hContact))
		return false;
	CMStringW fn = getMStringW(hContact, "FirstName");
	CMStringW ln = getMStringW(hContact, "LastName");
	if (!fn.IsEmpty() && fn.GetLength() >= 5 && !_wcsnicmp(fn.c_str(), L"User ", 5))
		return true;
	return fn.IsEmpty() && ln.IsEmpty();
}

namespace
{
	struct CMaxLiveNotifCtx
	{
		CMaxProto *pProto = nullptr;
		JSONNode *pPayload = nullptr;
	};
}

void CMaxProto::QueueLiveNotifIngest(const JSONNode &payload)
{
	auto *ctx = new CMaxLiveNotifCtx();
	if (ctx == nullptr)
		return;
	ctx->pProto = this;
	ctx->pPayload = new JSONNode(payload);
	if (ctx->pPayload == nullptr) {
		delete ctx;
		return;
	}
	ForkThread(&CMaxProto::LiveNotifIngestWorker, ctx);
}

void __cdecl CMaxProto::LiveNotifIngestWorker(void *param)
{
	auto *ctx = (CMaxLiveNotifCtx *)param;
	if (ctx == nullptr || ctx->pProto == nullptr || ctx->pPayload == nullptr) {
		if (ctx) {
			delete ctx->pPayload;
			delete ctx;
		}
		return;
	}

	CMaxProto *p = ctx->pProto;
	JSONNode *pPl = ctx->pPayload;
	ctx->pPayload = nullptr;
	delete ctx;

	const JSONNode &payload = *pPl;

	p->SyncLiveDialogFromPushPayload(payload);

	const JSONNode &msg = payload["message"];
	if (msg.type() != JSON_NODE) {
		delete pPl;
		return;
	}

	CMStringA sender;
	{
		const JSONNode &sn = msg["sender"];
		if (sn.type() == JSON_NULL)
			sender.Empty();
		else if (sn.type() == JSON_NUMBER) {
			sender.Format("%.0f", sn.as_float());
		}
		else
			sender = sn.as_string().c_str();
	}

	CMStringA chatId;
	{
		const JSONNode &cn = payload["chatId"];
		if (cn.type() == JSON_NUMBER)
			chatId.Format("%.0f", cn.as_float());
		else if (cn.type() == JSON_STRING)
			chatId = cn.as_string().c_str();
	}
	if (chatId.IsEmpty())
		chatId = MaxJsonIdStrLocal(payload["cid"]);
	if (chatId.IsEmpty())
		chatId = MaxJsonIdStrLocal(msg["cid"]);
	if (chatId.IsEmpty()) {
		debugLogA("Max: live push skip (no usable chatId) sender=%s msg=%s",
			sender.IsEmpty() ? "(unknown)" : sender.c_str(),
			MaxJsonIdStrLocal(msg["id"]).c_str());
		delete pPl;
		return;
	}

	if (!p->m_bTerminated && p->WaitForGatewayReady() && p->m_pGateway != nullptr) {
		MCONTACT hPeer = p->ResolveContactForDialogMessage(chatId.c_str(), sender.IsEmpty() ? nullptr : sender.c_str());
		if (hPeer && p->ContactNeedsServerDisplayFetch(hPeer)) {
			ptrA uid(getStringA(hPeer, DB_KEY_MAX_UID));
			if (uid != nullptr && uid[0]) {
				CMStringA one[1] = { uid.get() };
				p->ApiFetchContactsBatch(p->m_pGateway, one, 1, false);
			}
		}
	}

	p->IngestMaxMessageJson(msg, chatId.c_str());
	delete pPl;
}

void __cdecl CMaxProto::LoadHistoryWorker(void *param)
{
	MCONTACT hContact = (MCONTACT)(UINT_PTR)param;
	if (hContact == 0)
		return;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot load server history: gateway is not connected."));
		return;
	}

	CMStringA chatId(GetOrResolveDialogChatId(hContact));
	if (chatId.IsEmpty()) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot load server history: chat id is missing for this contact."));
		return;
	}

	int totalLoaded = 0;
	int64_t fromMs = (int64_t)time(nullptr) * 1000;
	uint64_t prevOldest = 0;

	const int kBatch = 100;
	const DWORD kInterPageDelayMs = 200;
	const int kMaxRetry = 4;
	int page = 0;
	bool reachedBottom = false;
	bool abortedByError = false;
	const char *stopReason = "unknown";

	while (!m_bTerminated) {
		int pageMsgCount = 0;
		uint64_t pageOldest = 0;
		if (!ApiFetchChatMessages(m_pGateway, chatId.c_str(), fromMs, 0, kBatch, true, &pageMsgCount, &pageOldest)) {
			CMStringW err = FormatLastError();
			ptrA err8(mir_utf8encodeW(err));
			bool throttled = false;
			if (!err.IsEmpty()) {
				CMStringW low(err);
				low.MakeLower();
				if (low.Find(L"limit") >= 0 || low.Find(L"too many") >= 0 || low.Find(L"rate") >= 0)
					throttled = true;
			}

			bool okAfterRetry = false;
			for (int retry = 1; retry <= kMaxRetry && !m_bTerminated; ++retry) {
				DWORD waitMs = throttled ? (DWORD)(1000 * (1 << (retry - 1))) : (DWORD)(600 * retry);
				debugLogA("Max: load history retry chat=%s page=%d retry=%d/%d wait=%u err=%s",
					chatId.c_str(), page, retry, kMaxRetry, (unsigned)waitMs,
					(err8 != nullptr && err8[0]) ? err8.get() : "(empty)");
				InterruptibleSleepMs(waitMs);

				pageMsgCount = 0;
				pageOldest = 0;
				if (ApiFetchChatMessages(m_pGateway, chatId.c_str(), fromMs, 0, kBatch, true, &pageMsgCount, &pageOldest)) {
					okAfterRetry = true;
					break;
				}

				err = FormatLastError();
				err8 = mir_utf8encodeW(err);
			}

			if (!okAfterRetry) {
				abortedByError = true;
				stopReason = throttled ? "server-throttle" : "server-error";
				debugLogA("Max: load history aborted chat=%s page=%d reason=%s err=%s",
					chatId.c_str(), page, stopReason,
					(err8 != nullptr && err8[0]) ? err8.get() : "(empty)");
				break;
			}
		}

		if (pageMsgCount < 0) {
			abortedByError = true;
			stopReason = "bad-payload";
			debugLogA("Max: load history aborted chat=%s page=%d reason=bad-payload", chatId.c_str(), page);
			break;
		}

		if (pageMsgCount == 0) {
			reachedBottom = true;
			stopReason = "empty-page";
			break;
		}

		totalLoaded += pageMsgCount;

		if (pageOldest == 0) {
			abortedByError = true;
			stopReason = "missing-time";
			debugLogA("Max: load history aborted chat=%s page=%d reason=missing-time", chatId.c_str(), page);
			break;
		}

		if (pageOldest == prevOldest) {
			reachedBottom = true;
			stopReason = "stuck-oldest";
			break;
		}

		prevOldest = pageOldest;
		if (pageOldest <= 1) {
			reachedBottom = true;
			stopReason = "reached-zero";
			break;
		}

		if (pageMsgCount < kBatch) {
			reachedBottom = true;
			stopReason = "short-page";
			break;
		}

		fromMs = (int64_t)(pageOldest - 1);
		page++;
		InterruptibleSleepMs(kInterPageDelayMs);
	}

	if (m_bTerminated && !reachedBottom && !abortedByError)
		stopReason = "terminated";

	if (abortedByError && page == 0) {
		if (!mir_strcmp(stopReason, "server-error") || !mir_strcmp(stopReason, "bad-json")
		    || !mir_strcmp(stopReason, "bad-payload"))
			RemoveLocalPeerIfChatOnly(hContact);
	}

	debugLogA("Max: load history done chat=%s fetched=%d reason=%s", chatId.c_str(), totalLoaded, stopReason);
	if (reachedBottom)
		History::FinishLoad(hContact);
}

INT_PTR CMaxProto::SvcLoadServerHistory(WPARAM hContact, LPARAM)
{
	if ((MCONTACT)hContact == 0)
		return 1;

	ForkThread(&CMaxProto::LoadHistoryWorker, (void *)(UINT_PTR)hContact);
	return 0;
}

static void __cdecl MaxOfflineDownloadProgress(size_t iProgress, void *pParam)
{
	auto *ofd = (OFDTHREAD *)pParam;
	DBVARIANT dbv = { DBVT_DWORD };
	dbv.dVal = (uint32_t)iProgress;
	db_event_setJson(ofd->hDbEvent, "ft", &dbv);
}

void __cdecl CMaxProto::OfflineFileWorker(void *param)
{
	auto *ofd = (OFDTHREAD *)param;
	if (ofd == nullptr) return;

	DB::EventInfo dbei(ofd->hDbEvent);
	if (dbei && !mir_strcmp(dbei.szModule, m_szModuleName) && dbei.eventType == EVENTTYPE_FILE) {
		DB::FILE_BLOB blob(dbei);
		const char *url = blob.getUrl();
		if (url != nullptr && url[0] != 0) {
			if (ofd->bCopy) {
				ofd->wszPath = Utf2T(url).get();
				ofd->pCallback->Invoke(*ofd);
			}
			else {
				MHttpRequest req(REQUEST_GET);
				req.flags = NLHRF_HTTP11 | NLHRF_SSL;
				req.m_szUrl = url;

				NLHR_PTR reply(Netlib_DownloadFile(m_hNetlibUser, &req, ofd->wszPath, MaxOfflineDownloadProgress, ofd));
				if (reply && reply->resultCode == 200) {
					struct _stat st = {};
					int stOk = _wstat(ofd->wszPath, &st);
					int64_t finalSize = (stOk == 0) ? (int64_t)st.st_size : 0;

					CMStringW localPath(ofd->wszPath);
					// NewStory [img=file://] may fail on non-ASCII paths; mirror file into protocol preview cache.
					if (sttHasNonAscii(localPath)) {
						CMStringW previewDir = GetPreviewPath();
						previewDir += L"\\offline";
						CreateDirectoryTreeW(previewDir);

						CMStringW ext = L".bin";
						int dotSrc = localPath.ReverseFind('.');
						int slashSrc = localPath.ReverseFind('\\');
						if (dotSrc != -1 && (slashSrc == -1 || dotSrc > slashSrc))
							ext = localPath.Mid(dotSrc);

						CMStringW mirror;
						mirror.Format(L"%s\\max_ev_%u%s", previewDir.c_str(), (unsigned)ofd->hDbEvent, ext.c_str());
						CMStringW unique = mirror;
						for (int i = 1; _waccess(unique, 0) == 0 && i < 1000; ++i) {
							int dot = mirror.ReverseFind('.');
							if (dot == -1)
								unique = mirror + CMStringW(FORMAT, L"_%d", i);
							else
								unique = mirror.Left(dot) + CMStringW(FORMAT, L"_%d", i) + mirror.Mid(dot);
						}
						if (CopyFileW(localPath, unique, true)) {
							localPath = unique;
							ofd->wszPath = localPath.c_str();
							struct _stat st2 = {};
							if (_wstat(localPath, &st2) == 0 && st2.st_size > 0)
								finalSize = (int64_t)st2.st_size;
							debugLogA("Max: offline file mirrored for preview path=%S", localPath.c_str());
						}
					}

					// Mark offline file as fully downloaded in DB event:
					// 1) save local path (lf), 2) set fs/ft to the same final size.
					DB::EventInfo ev(ofd->hDbEvent);
					if (ev && !mir_strcmp(ev.szModule, m_szModuleName) && ev.eventType == EVENTTYPE_FILE) {
						DB::FILE_BLOB finBlob(ev);
						if (finalSize <= 0) {
							// _wstat may occasionally race/fail on some systems; fall back to transfer counters.
							finalSize = finBlob.getTransferred();
							if (finalSize <= 0)
								finalSize = finBlob.getSize();
						}
						if (finalSize <= 0) {
							if (const char *cl = reply->FindHeader("Content-Length"))
								finalSize = _strtoi64(cl, nullptr, 10);
						}
						finBlob.setLocalName(localPath);
						if (finalSize > 0)
							finBlob.complete(finalSize);
						finBlob.write(ev);
						db_event_edit(ofd->hDbEvent, &ev, true);

						// Force critical fields directly to avoid losing them in concurrent updates.
						DBVARIANT vPath = { DBVT_WCHAR };
						vPath.pwszVal = localPath.GetBuffer();
						db_event_setJson(ofd->hDbEvent, "lf", &vPath);
						if (finalSize > 0) {
							DBVARIANT vSize = { DBVT_DWORD };
							vSize.dVal = (uint32_t)finalSize;
							db_event_setJson(ofd->hDbEvent, "fs", &vSize);
							db_event_setJson(ofd->hDbEvent, "ft", &vSize);
						}
						debugLogA("Max: offline file finalized ev=%u size=%lld path=%S", (unsigned)ofd->hDbEvent, (long long)finalSize, localPath.c_str());

						// Read back the event as NewStory does, to verify what is actually persisted.
						DB::EventInfo chk(ofd->hDbEvent);
						if (chk && !mir_strcmp(chk.szModule, m_szModuleName) && chk.eventType == EVENTTYPE_FILE) {
							DB::FILE_BLOB rb(chk);
							const wchar_t *pLf = rb.getLocalName();
							const wchar_t *pName = rb.getName();
							int fmt = (pLf && pLf[0]) ? ProtoGetAvatarFileFormat(pLf) : PA_FORMAT_UNKNOWN;
							debugLogA("Max: offline file verify ev=%u fs=%lld ft=%lld done=%d fmt=%d lf=%S name=%S",
								(unsigned)ofd->hDbEvent,
								(long long)rb.getSize(),
								(long long)rb.getTransferred(),
								rb.isCompleted() ? 1 : 0,
								fmt,
								(pLf && pLf[0]) ? pLf : L"(empty)",
								(pName && pName[0]) ? pName : L"(empty)");
						}
					}
					ofd->Finish();
				}
			}
		}
	}

	delete ofd;
}

INT_PTR CMaxProto::SvcOfflineFile(WPARAM wParam, LPARAM)
{
	ForkThread(&CMaxProto::OfflineFileWorker, (void *)wParam);
	return 0;
}

INT_PTR CMaxProto::SvcCanEmptyHistory(WPARAM hContact, LPARAM)
{
	MCONTACT h = (MCONTACT)hContact;
	if (h == 0)
		return 0;

	if (isChatRoom(h)) {
		SESSION_INFO *si = Chat_Find(h, m_szModuleName);
		return (si != nullptr && si->ptszID != nullptr && si->ptszID[0] != 0) ? 1 : 0;
	}

	CMStringA chatId = GetOrResolveDialogChatId(h, false);
	return chatId.IsEmpty() ? 0 : 1;
}

INT_PTR CMaxProto::SvcEmptyServerHistory(WPARAM hContact, LPARAM lParam)
{
	MCONTACT h = (MCONTACT)hContact;
	if (h == 0 || (lParam & CDF_DEL_HISTORY) == 0)
		return 0;

	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot remove server chat: not connected."));
		return 0;
	}

	if (isChatRoom(h)) {
		SESSION_INFO *si = Chat_Find(h, m_szModuleName);
		if (si == nullptr || si->ptszID == nullptr || si->ptszID[0] == 0)
			return 0;
		ptrA chatUtf(mir_u2a(si->ptszID));
		if (chatUtf == nullptr)
			return 0;
		if (ApiChatLeave(m_pGateway, chatUtf)) {
			debugLogA("Max: empty history — server leave (opcode 58) chat=%s", chatUtf.get());
			delSetting(h, DB_KEY_MAX_CHATID);
		}
		else
			NotifyUser(TranslateT("Max"), TranslateT("Could not leave the chat on the server."));
		return 0;
	}

	CMStringA chatId = GetOrResolveDialogChatId(h, false);
	if (chatId.IsEmpty()) {
		NotifyUser(TranslateT("Max"), TranslateT("Cannot remove server chat: dialog id is unknown."));
		return 0;
	}

	if (ApiDeleteServerDialog(m_pGateway, chatId.c_str())) {
		debugLogA("Max: empty history — server dialog delete (opcode 52) chat=%s", chatId.c_str());
		delSetting(h, DB_KEY_MAX_CHATID);
	}
	else
		NotifyUser(TranslateT("Max"), TranslateT("Could not delete the dialog on the server."));
	return 0;
}

void CMaxProto::OnShutdown(void)
{
	DisconnectGateway();
}

bool CMaxProto::OnContactDeleted(MCONTACT hContact, uint32_t flags)
{
	if (!(flags & CDF_DEL_CONTACT))
		return true;
	if (hContact == 0)
		return true;

	if (!WaitForGatewayReady() || m_pGateway == nullptr)
		return true;

	if (isChatRoom(hContact)) {
		if (SESSION_INFO *si = Chat_Find(hContact, m_szModuleName))
			if (si->ptszID != nullptr && si->ptszID[0]) {
				ptrA chatUtf(mir_u2a(si->ptszID));
				if (chatUtf && ApiChatLeave(m_pGateway, chatUtf))
					debugLogA("Max: server left group/channel (opcode 58) id=%s", chatUtf.get());
				else if (chatUtf)
					debugLogA("Max: server leave group failed id=%s", chatUtf.get());
			}
		return true;
	}

	CMStringA chatId = GetOrResolveDialogChatId(hContact, false);
	if (chatId.IsEmpty()) {
		debugLogA("Max: OnContactDeleted (server) skipped — no chat id h=%u", (unsigned)hContact);
		return true;
	}

	ptrA uid(getStringA(hContact, DB_KEY_MAX_UID));
	// Always try opcode 34 REMOVE when we have uid: MaxPeerOrigin can be CHATONLY while the peer is still
	// in the server address book (official client). REMOVE is cheap; SendJsonAndWait(..., true) tolerates errors.
	if (uid != nullptr && uid[0]) {
		if (ApiRemoveContactFromServer(m_pGateway, uid))
			debugLogA("Max: removed uid=%s from server address book (opcode 34)", uid.get());
		else
			debugLogA("Max: opcode 34 REMOVE finished with error/timeout uid=%s (dialog delete still attempted)", uid.get());
	}

	if (ApiDeleteServerDialog(m_pGateway, chatId.c_str()))
		debugLogA("Max: deleted dialog on server (opcode 52) chatId=%s", chatId.c_str());
	else
		debugLogA("Max: delete dialog on server failed chatId=%s", chatId.c_str());

	return true;
}

void CMaxProto::DisconnectGateway()
{
	const bool hadSession = (m_hConnThread != nullptr || m_hWsRunThread != nullptr || m_pGateway != nullptr);
	if (hadSession) {
		debugLogA("Connection terminated, exiting");
		debugLogA("CMaxProto::OnLoggedOut");
	}

	m_bTerminated = true;
	m_bAvatarWebPrimed = false;
	CloseQrDialog(false);
	if (m_pGateway)
		m_pGateway->terminate();

	if (m_hWsRunThread) {
		WaitForSingleObject(m_hWsRunThread, 15000);
		CloseHandle(m_hWsRunThread);
		m_hWsRunThread = nullptr;
	}

	if (m_hConnThread) {
		WaitForSingleObject(m_hConnThread, 15000);
		CloseHandle(m_hConnThread);
		m_hConnThread = nullptr;
	}
	m_pGateway = nullptr;
	m_wsRun.ws = nullptr;
	m_bGatewayConnected = false;
	FreeWsInflater();
	ResetServerContactBookCache();
}

MWindow CMaxProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ACCMGRUI), hwndParent, MaxAccMgrProc, (LPARAM)this);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK MaxAccMgrProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMaxProto *ppro = (CMaxProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		ppro = (CMaxProto *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		Window_SetIcon_IcoLib(hwndDlg, ppro->m_hProtoIcon);
		SetDlgItemTextW(hwndDlg, IDC_GROUPNAME, ppro->GetDefaultGroupW());
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_GROUPNAME)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			wchar_t wgrp[128];
			GetDlgItemTextW(hwndDlg, IDC_GROUPNAME, wgrp, _countof(wgrp));
			if (wgrp[0]) {
				ppro->setWString(DB_KEY_DEFAULT_GROUP, wgrp);
				Clist_GroupCreate(0, wgrp);
			}
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK MaxOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMaxProto *ppro = (CMaxProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		ppro = (CMaxProto *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SetDlgItemTextW(hwndDlg, IDC_GROUPNAME, ppro->GetDefaultGroupW());
		CheckDlgButton(hwndDlg, IDC_COMPRESS_FILES, ppro->getByte("CompressFiles", 1) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		if ((HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_GROUPNAME)
			|| (LOWORD(wParam) == IDC_COMPRESS_FILES && HIWORD(wParam) == BN_CLICKED))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			wchar_t wgrp[128];
			GetDlgItemTextW(hwndDlg, IDC_GROUPNAME, wgrp, _countof(wgrp));
			if (wgrp[0]) {
				ppro->setWString(DB_KEY_DEFAULT_GROUP, wgrp);
				Clist_GroupCreate(0, wgrp);
			}
			ppro->setByte("CompressFiles", IsDlgButtonChecked(hwndDlg, IDC_COMPRESS_FILES) ? 1 : 0);
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CMaxProto::SaveMyProfile(const wchar_t *pwszFirstName, const wchar_t *pwszLastName, const wchar_t *pwszBio)
{
	if (!WaitForGatewayReady() || m_pGateway == nullptr) {
		NotifyUser(TranslateT("Max"), TranslateT("You must be online to save your profile."));
		return false;
	}

	const wchar_t *fnIn = (pwszFirstName != nullptr) ? pwszFirstName : L"";
	CMStringW fnTrim(fnIn);
	fnTrim.Trim();
	if (fnTrim.IsEmpty()) {
		NotifyUser(TranslateT("Max"), TranslateT("First name cannot be empty."));
		return false;
	}

	const wchar_t *lnIn = (pwszLastName != nullptr) ? pwszLastName : L"";
	CMStringW lnTrim(lnIn);
	lnTrim.Trim();

	const wchar_t *bioIn = (pwszBio != nullptr) ? pwszBio : L"";

	ptrA fn8(mir_utf8encodeW(fnTrim.c_str()));
	ptrA ln8;
	if (!lnTrim.IsEmpty())
		ln8 = mir_utf8encodeW(lnTrim.c_str());
	ptrA bio8(mir_utf8encodeW(bioIn));

	const char *pLast = (ln8 != nullptr && ln8[0]) ? ln8.get() : nullptr;
	if (!ApiUpdateMyProfile(m_pGateway, fn8, pLast, bio8)) {
		CMStringW err = FormatLastError();
		if (!err.IsEmpty())
			NotifyUser(TranslateT("Max"), err.c_str());
		else
			NotifyUser(TranslateT("Max"), TranslateT("Could not save your profile on the server."));
		return false;
	}

	setWString("FirstName", fnTrim.c_str());
	if (lnTrim.IsEmpty())
		delSetting("LastName");
	else
		setWString("LastName", lnTrim.c_str());

	if (bioIn[0] == 0)
		delSetting("About");
	else
		setWString("About", bioIn);
	return true;
}

CMStringW CMaxProto::FormatLastError()
{
	if (m_szPendingResponse.IsEmpty())
		return L"";
	JSONNode root = JSONNode::parse(m_szPendingResponse.c_str());
	if (!root)
		return L"";
	const JSONNode &pl = root["payload"];
	if (pl["localizedMessage"].type() == JSON_STRING && !pl["localizedMessage"].as_string().empty()) {
		ptrW w(mir_utf8decodeW(pl["localizedMessage"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	if (pl["title"].type() == JSON_STRING && !pl["title"].as_string().empty()) {
		ptrW w(mir_utf8decodeW(pl["title"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	const JSONNode &er = pl["error"];
	if (er.type() == JSON_STRING) {
		CMStringA ecode(er.as_string().c_str());
		if (!mir_strcmp(ecode.c_str(), "error.limit.violate"))
			return TranslateT("Too many requests. Please wait before trying again.");
		if (!mir_strcmp(ecode.c_str(), "login.cred"))
			return TranslateT("Server rejected web sync credentials for this token (login.cred).");

		ptrW w(mir_utf8decodeW(ecode.c_str()));
		return CMStringW((const wchar_t*)w);
	}
	if (er.type() == JSON_NODE && er["message"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(er["message"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	if (pl["message"].type() != JSON_NULL) {
		ptrW w(mir_utf8decodeW(pl["message"].as_string().c_str()));
		return CMStringW((const wchar_t *)w);
	}
	return L"";
}

void CMaxProto::ShowQrCode(const CMStringA &qrText)
{
	if (Miranda_IsTerminated() || qrText.IsEmpty())
		return;

	CallFunctionSync(LaunchQrDialog, this);
	if (m_pQRDlg != nullptr)
		m_pQRDlg->SetData(qrText);
}

void CMaxProto::CloseQrDialog(bool bSuccess)
{
	if (Miranda_IsTerminated())
		return;
	if (m_pQRDlg == nullptr)
		return;
	if (bSuccess)
		m_pQRDlg->SetSuccess();
	CallFunctionSync(CloseQrDialogSync, this);
}

void CMaxProto::NotifyUser(const wchar_t *title, const wchar_t *text)
{
	if (Miranda_IsTerminated())
		return;

	// Always mirror user-facing text to netlog (popups may be disabled).
	ptrA title8(mir_utf8encodeW(title ? title : L""));
	ptrA text8(mir_utf8encodeW(text ? text : L""));
	debugLogA("Max: %s — %s",
		(title8 && title8[0]) ? title8.get() : "(no title)",
		(text8 && text8[0]) ? text8.get() : "(no text)");

	if (Popup_Enabled()) {
		POPUPDATAW ppd;
		ppd.lchContact = NULL;
		wcsncpy_s(ppd.lpwzContactName, title, _TRUNCATE);
		wcsncpy_s(ppd.lpwzText, text, _TRUNCATE);
		ppd.lchIcon = g_plugin.getIcon(IDI_MAIN);
		if (PUAddPopupW(&ppd))
			return;
	}
}

int CMaxProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.szTab.w = LPGENW("Account");
	odp.dwInitParam = (LPARAM)this;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSUI);
	odp.pfnDlgProc = MaxOptionsProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

bool CMaxProto::HasLoginToken()
{
	ptrA t(getStringA(DB_KEY_LOGIN_TOKEN));
	return t != nullptr && t[0] != 0;
}

bool CMaxProto::WaitForGatewayReady()
{
	if (Miranda_IsTerminated())
		return false;

	// Do not wait from inside ConnectionWorker itself: it would deadlock before it sets m_bGatewayConnected=true.
	if (m_dwConnThreadId != 0 && m_dwConnThreadId == GetCurrentThreadId()) {
		const bool ok = (m_pGateway != nullptr);
		debugLogA("Max: WaitForGatewayReady (conn thread) ok=%d gw=%p gwConnected=%d",
			ok ? 1 : 0, m_pGateway, m_bGatewayConnected ? 1 : 0);
		return ok;
	}

	debugLogA("Max: WaitForGatewayReady enter status=%d connThread=%p gw=%p gwConnected=%d",
		m_iStatus, m_hConnThread, m_pGateway, m_bGatewayConnected ? 1 : 0);

	if (m_hConnThread) {
		if (WaitForSingleObject(m_hConnThread, 0) == WAIT_OBJECT_0) {
			CloseHandle(m_hConnThread);
			m_hConnThread = nullptr;
		}
	}

	if (!m_hConnThread) {
		debugLogA("Max: WaitForGatewayReady starting connection (SetStatus online)");
		SetStatus(ID_STATUS_ONLINE);
	}

	for (int i = 0; i < 300; i++) {
		if (Miranda_IsTerminated())
			return false;
		if (m_bGatewayConnected && m_pGateway != nullptr)
			return true;
		Sleep(100);
	}

	const bool ok = m_bGatewayConnected && m_pGateway != nullptr;
	debugLogA("Max: WaitForGatewayReady exit ok=%d status=%d connThread=%p gw=%p gwConnected=%d",
		ok ? 1 : 0, m_iStatus, m_hConnThread, m_pGateway, m_bGatewayConnected ? 1 : 0);
	return ok;
}
