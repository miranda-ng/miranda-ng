/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"
#include <m_imgsrvc.h>
#include <stdio.h>

// Match AVS clist thumbnail edge (~128); square crop avoids mixed aspect ratios in the contact list.
static constexpr int MAX_AVATAR_SIDE = 128;

static void sttNormalizeAvatarToSquare(const wchar_t *wszPath)
{
	HBITMAP hb = Image_Load(wszPath, 0);
	if (hb == nullptr)
		return;

	HBITMAP sq = Image_Resize(hb, RESIZEBITMAP_MAKE_SQUARE, MAX_AVATAR_SIDE, MAX_AVATAR_SIDE);
	if (sq == nullptr) {
		DeleteObject(hb);
		return;
	}
	if (sq != hb)
		DeleteObject(hb);

	wchar_t wszDir[MAX_PATH], wszTmp[MAX_PATH];
	if (!GetTempPathW(_countof(wszDir), wszDir) || !GetTempFileNameW(wszDir, L"mx", 0, wszTmp)) {
		DeleteObject(sq);
		return;
	}

	IMGSRVC_INFO ii = {};
	ii.cbSize = sizeof(ii);
	ii.pwszName = wszTmp;
	ii.hbm = sq;
	ii.dwMask = IMGI_HBITMAP;
	ii.fif = FIF_UNKNOWN;

	const wchar_t *ext = wcsrchr(wszPath, L'.');
	int saveFlags = 0;
	if (ext && (!_wcsicmp(ext, L".jpg") || !_wcsicmp(ext, L".jpeg")))
		saveFlags = JPEG_QUALITYGOOD;

	const int ok = Image_Save(&ii, saveFlags);
	DeleteObject(sq);

	if (!ok) {
		DeleteFileW(wszTmp);
		return;
	}

	if (CopyFileW(wszTmp, wszPath, FALSE))
		DeleteFileW(wszTmp);
	else if (!MoveFileExW(wszTmp, wszPath, MOVEFILE_REPLACE_EXISTING))
		DeleteFileW(wszTmp);
}

static bool sttHeaderContainsI(const char *pszHaystack, const char *pszNeedle)
{
	if (pszHaystack == nullptr || pszNeedle == nullptr || pszNeedle[0] == 0)
		return false;
	for (const char *p = pszHaystack; *p; p++)
		if (!_strnicmp(p, pszNeedle, mir_strlen(pszNeedle)))
			return true;
	return false;
}

static bool sttContentTypeSaysNonImage(const char *pszCt)
{
	if (pszCt == nullptr)
		return false;
	if (sttHeaderContainsI(pszCt, "text/html"))
		return true;
	if (sttHeaderContainsI(pszCt, "application/json"))
		return true;
	if (sttHeaderContainsI(pszCt, "text/plain"))
		return true;
	return false;
}

static bool sttBodyLooksLikeHtmlDocument(const char *p, size_t cb)
{
	size_t i = 0;
	if (cb >= 3 && (uint8_t)p[0] == 0xEF && (uint8_t)p[1] == 0xBB && (uint8_t)p[2] == 0xBF) {
		i = 3;
	}
	while (i < cb && (unsigned char)p[i] <= ' ')
		i++;
	if (i >= cb || p[i] != '<')
		return false;
	if (cb - i >= 9 && !_strnicmp(p + i, "<!DOCTYPE", 9))
		return true;
	if (cb - i >= 5 && !_strnicmp(p + i, "<html", 5))
		return true;
	if (cb - i >= 5 && !_strnicmp(p + i, "<head", 5))
		return true;
	if (cb - i >= 5 && !_strnicmp(p + i, "<body", 5))
		return true;
	if (cb - i >= 7 && !_strnicmp(p + i, "<script", 7))
		return true;
	return false;
}

static bool sttBufferIsKnownRasterImage(const void *p, size_t cb)
{
	if (p == nullptr || cb < 12)
		return false;
	int fmt = ProtoGetBufferFormat(p, nullptr);
	return fmt == PA_FORMAT_PNG || fmt == PA_FORMAT_JPEG || fmt == PA_FORMAT_GIF || fmt == PA_FORMAT_BMP || fmt == PA_FORMAT_WEBP;
}

static bool sttHttpBodyIsImagePayload(MHttpResponse *pReply, const CMStringA &body)
{
	if (body.GetLength() < 32)
		return false;
	if (sttContentTypeSaysNonImage(pReply->FindHeader("Content-Type")))
		return false;
	const char *raw = body.c_str();
	size_t cb = (size_t)body.GetLength();
	if (sttBodyLooksLikeHtmlDocument(raw, cb))
		return false;
	return sttBufferIsKnownRasterImage(raw, cb);
}

static bool sttAvatarFileOnDiskIsRasterImage(const wchar_t *wszPath)
{
	FILE *f = _wfopen(wszPath, L"rb");
	if (f == nullptr)
		return false;
	uint8_t buf[32];
	size_t n = fread(buf, 1, sizeof(buf), f);
	fclose(f);
	return sttBufferIsKnownRasterImage(buf, n);
}

static bool sttLooksLikeHttpUrl(const CMStringA &s)
{
	return s.GetLength() >= 8 && (!_strnicmp(s, "https://", 8) || !_strnicmp(s, "http://", 7));
}

static bool sttTakeUrlField(const JSONNode &obj, const char *key, CMStringA &out)
{
	const JSONNode &n = obj[key];
	if (n.type() == JSON_NULL)
		return false;
	if (n.type() != JSON_STRING)
		return false;
	CMStringA s(n.as_string().c_str());
	s.Trim();
	if (!sttLooksLikeHttpUrl(s))
		return false;
	out = s;
	return true;
}

static CMStringA sttExtractAvatarUrlDepth(const JSONNode &c, int depth)
{
	if (depth > 6)
		return {};

	// Omit "link": in Max JSON it is a profile URL, not the image (avatars use baseUrl / baseRawUrl).
	static const char *flatKeys[] = {
		"baseRawUrl", "baseUrl",
		"full_avatar_url", "avatar_url", "fullAvatarUrl", "avatarUrl",
		"photoUrl", "photo_url", "imageUrl", "pictureUrl", "profileImageUrl",
		"iconUrl", "thumbnailUrl", "previewUrl", nullptr
	};

	const JSONNode &names = c["names"];
	if (names.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < names.size(); i++) {
			const JSONNode &el = names[i];
			if (el.type() != JSON_NODE)
				continue;
			for (int j = 0; flatKeys[j]; j++) {
				CMStringA u;
				if (sttTakeUrlField(el, flatKeys[j], u))
					return u;
			}
			CMStringA r = sttExtractAvatarUrlDepth(el, depth + 1);
			if (!r.IsEmpty())
				return r;
		}
	}

	for (int i = 0; flatKeys[i]; i++) {
		CMStringA u;
		if (sttTakeUrlField(c, flatKeys[i], u))
			return u;
	}

	const JSONNode &av = c["avatar"];
	if (av.type() == JSON_STRING) {
		CMStringA u(av.as_string().c_str());
		u.Trim();
		if (sttLooksLikeHttpUrl(u))
			return u;
	}
	if (av.type() == JSON_NODE) {
		CMStringA u;
		if (sttTakeUrlField(av, "url", u))
			return u;
		if (sttTakeUrlField(av, "fullUrl", u))
			return u;
	}

	const JSONNode &ph = c["photo"];
	if (ph.type() == JSON_NODE) {
		CMStringA u;
		if (sttTakeUrlField(ph, "url", u))
			return u;
	}

	const JSONNode &bc = c["baseContact"];
	if (bc.type() == JSON_NODE) {
		CMStringA r = sttExtractAvatarUrlDepth(bc, depth + 1);
		if (!r.IsEmpty())
			return r;
	}

	static const char *nestKeys[] = { "user", "profile", "contact", "person", nullptr };
	for (int k = 0; nestKeys[k]; k++) {
		const JSONNode &nn = c[nestKeys[k]];
		if (nn.type() == JSON_NODE) {
			CMStringA r = sttExtractAvatarUrlDepth(nn, depth + 1);
			if (!r.IsEmpty())
				return r;
		}
	}

	return {};
}

CMStringA CMaxProto::ExtractAvatarUrlFromJson(const JSONNode &c)
{
	return sttExtractAvatarUrlDepth(c, 0);
}

static void sttDeleteCachedAvatarFilesForContact(CMaxProto *p, MCONTACT hContact)
{
	CMStringW path(p->GetAvatarPath());
	path.AppendChar(L'\\');

	CMStringA uid;
	if (hContact)
		uid = p->getMStringA(hContact, DB_KEY_MAX_UID);
	else {
		ptrA my(p->getStringA(DB_KEY_MY_MAX_ID));
		if (my != nullptr && my[0])
			uid = my.get();
	}
	if (uid.IsEmpty())
		uid = "me";

	path.Append(mir_utf8decodeW(uid.c_str()));

	static const wchar_t *pexts[] = { L".jpg", L".jpeg", L".png", L".webp", L".gif", L".bmp", nullptr };
	for (int i = 0; pexts[i]; i++) {
		CMStringW f(path);
		f += pexts[i];
		_wunlink(f);
	}
}

void CMaxProto::SyncContactAvatarFromJson(MCONTACT hContact, const JSONNode &c)
{
	CMStringA url = ExtractAvatarUrlFromJson(c);
	CMStringA old = getMStringA(hContact, DB_KEY_AVATAR_URL);

	if (url == old) {
		if (url.IsEmpty())
			return;
		wchar_t wszExisting[MAX_PATH];
		GetAvatarFileName(hContact, wszExisting, _countof(wszExisting));
		if (::_waccess(wszExisting, 0) == 0 && sttAvatarFileOnDiskIsRasterImage(wszExisting))
			return;
	}

	if (url.IsEmpty()) {
		if (!old.IsEmpty()) {
			delSetting(hContact, DB_KEY_AVATAR_URL);
			setByte(hContact, "NeedNewAvatar", 0);
			sttDeleteCachedAvatarFilesForContact(this, hContact);
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr);
		}
		return;
	}

	setString(hContact, DB_KEY_AVATAR_URL, url.c_str());
	setByte(hContact, "NeedNewAvatar", 1);

	// Download while session is active (do not use GetStatus() > OFFLINE: ID_STATUS_CONNECTING==1 is < OFFLINE==40071).
	if (HasLoginToken() && !m_bTerminated) {
		PROTO_AVATAR_INFORMATION ai = {};
		ai.hContact = hContact;
		GetAvatarFileName(hContact, ai.filename, _countof(ai.filename));
		if (DownloadAvatarToFile(hContact, url.c_str(), ai.filename, _countof(ai.filename))) {
			setByte(hContact, "NeedNewAvatar", 0);
			ai.format = ProtoGetAvatarFileFormat(ai.filename);
			if (ai.format == PA_FORMAT_UNKNOWN)
				ai.format = ProtoGetAvatarFormat(ai.filename);
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai);
			return;
		}
		debugLogA("Max: avatar download failed for h=%p url=%s", (void *)(INT_PTR)hContact, url.c_str());
	}

	db_set_b(hContact, "ContactPhoto", "NeedUpdate", 1);
	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, nullptr);
}

void CMaxProto::GetAvatarFileName(MCONTACT hContact, wchar_t *pwszDest, size_t cchDest)
{
	if (!pwszDest || cchDest == 0)
		return;

	CMStringW path(GetAvatarPath());
	path.AppendChar(L'\\');

	CMStringA uid;
	if (hContact)
		uid = getMStringA(hContact, DB_KEY_MAX_UID);
	else {
		ptrA my(getStringA(DB_KEY_MY_MAX_ID));
		if (my != nullptr && my[0])
			uid = my.get();
	}
	if (uid.IsEmpty())
		uid = "me";

	CMStringW wszExtW(L".jpg");
	ptrA szUrl(getStringA(hContact, DB_KEY_AVATAR_URL));
	const char *urlc = szUrl.get();
	if (urlc != nullptr && urlc[0]) {
		const char *q = strchr(urlc, '?');
		const char *end = q ? q : urlc + strlen(urlc);
		for (const char *p = end - 1; p >= urlc; --p) {
			if (*p == '/')
				break;
			if (*p == '.') {
				CMStringW wtmp(mir_utf8decodeW(p));
				if (!wtmp.IsEmpty() && wtmp.GetLength() <= 8)
					wszExtW = wtmp;
				break;
			}
		}
	}

	path.Append(mir_utf8decodeW(uid.c_str()));
	path += wszExtW;
	wcsncpy_s(pwszDest, cchDest, path, _TRUNCATE);
}

bool CMaxProto::DownloadAvatarToFile(MCONTACT hContact, const char *szUrl, wchar_t *wszPath, size_t cchPath)
{
	if (szUrl == nullptr || szUrl[0] == 0 || wszPath == nullptr || cchPath == 0)
		return false;

	CreateDirectoryTreeW(GetAvatarPath());

	GetAvatarFileName(hContact, wszPath, cchPath);

	ptrA tok(getStringA(DB_KEY_LOGIN_TOKEN));
	if (tok != nullptr && tok[0] && !m_bAvatarWebPrimed) {
		m_bAvatarWebPrimed = true;
		MHttpRequest warm(REQUEST_GET);
		warm.flags = NLHRF_NODUMP | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
		warm.m_szUrl = "https://web.max.ru/";
		warm.AddHeader("User-Agent", MAX_HTTP_USER_AGENT);
		warm.AddHeader("Accept", "text/html,application/xhtml+xml;q=0.9,*/*;q=0.8");
		CMStringA wb;
		wb.Format("Bearer %s", tok.get());
		warm.AddHeader("Authorization", wb.c_str());
		Netlib_HttpTransaction(m_hNetlibUser, &warm);
	}

	auto tryOnce = [&](const char *pszReqUrl, const char *pszAuthName, const char *pszAuthVal) -> bool {
		if (pszReqUrl == nullptr || pszReqUrl[0] == 0)
			return false;

		MHttpRequest req(REQUEST_GET);
		req.flags = NLHRF_NODUMP | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
		req.m_szUrl = pszReqUrl;
		req.AddHeader("Origin", MAX_HTTP_ORIGIN_HEADER);
		req.AddHeader("Referer", MAX_HTTP_ORIGIN_HEADER "/");
		req.AddHeader("User-Agent", MAX_HTTP_USER_AGENT);
		req.AddHeader("Accept", "image/avif,image/webp,image/apng,image/*,*/*;q=0.8");
		req.AddHeader("Sec-Fetch-Dest", "image");
		req.AddHeader("Sec-Fetch-Mode", "no-cors");
		req.AddHeader("Sec-Fetch-Site", "cross-site");
		if (pszAuthName != nullptr && pszAuthVal != nullptr && pszAuthVal[0] != 0)
			req.AddHeader(pszAuthName, pszAuthVal);

		NLHR_PTR pReply(Netlib_HttpTransaction(m_hNetlibUser, &req));
		if (pReply == nullptr)
			return false;
		if (pReply->resultCode < 200 || pReply->resultCode >= 300) {
			debugLogA("Max: avatar HTTP %d url=%s", pReply->resultCode, pszReqUrl);
			return false;
		}
		if (pReply->body.IsEmpty())
			return false;
		if (!sttHttpBodyIsImagePayload(pReply, pReply->body))
			return false;

		if (CreatePathToFileW(wszPath) != 0)
			return false;

		FILE *out = _wfopen(wszPath, L"wb");
		if (out == nullptr)
			return false;
		size_t cb = (size_t)pReply->body.GetLength();
		size_t wr = fwrite(pReply->body.c_str(), 1, cb, out);
		fclose(out);
		if (wr != cb) {
			_wunlink(wszPath);
			return false;
		}
		sttNormalizeAvatarToSquare(wszPath);
		return true;
	};

	CMStringA bearer;
	if (tok != nullptr && tok[0])
		bearer.Format("Bearer %s", tok.get());

	// i.oneme.ru often returns HTML without session; web.max.ru uses the same LOGIN token as WebSocket opcode 19.
	if (tok != nullptr && tok[0]) {
		if (tryOnce(szUrl, "Authorization", bearer.c_str()))
			return true;
		if (tryOnce(szUrl, "Authorization", tok.get()))
			return true;
		if (tryOnce(szUrl, "X-Auth-Token", tok.get()))
			return true;
	}

	if (tryOnce(szUrl, nullptr, nullptr))
		return true;

	if (tok != nullptr && tok[0]) {
		CMStringA u(szUrl);
		u.Append(strchr(szUrl, '?') ? "&" : "?");
		u.Append("access_token=");
		u += mir_urlEncode(tok);

		if (tryOnce(u.c_str(), nullptr, nullptr))
			return true;

		u = szUrl;
		u.Append(strchr(szUrl, '?') ? "&" : "?");
		u.Append("token=");
		u += mir_urlEncode(tok);
		if (tryOnce(u.c_str(), nullptr, nullptr))
			return true;
	}

	debugLogA("Max: avatar download failed (all strategies returned non-image or HTTP error) baseUrl=%s", szUrl);
	return false;
}

INT_PTR __cdecl CMaxProto::SvcGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	auto *pai = (PROTO_AVATAR_INFORMATION *)lParam;
	if (pai == nullptr)
		return GAIR_NOAVATAR;

	MCONTACT h = pai->hContact;

	ptrA szUrl(getStringA(h, DB_KEY_AVATAR_URL));
	if (szUrl == nullptr || szUrl[0] == 0)
		return GAIR_NOAVATAR;

	GetAvatarFileName(h, pai->filename, _countof(pai->filename));
	pai->format = ProtoGetAvatarFormat(pai->filename);

	const bool force = (wParam & GAIF_FORCE) != 0;
	const bool onDisk = (::_waccess(pai->filename, 0) == 0);
	const bool needNew = getByte(h, "NeedNewAvatar") != 0;
	const bool diskOk = onDisk && sttAvatarFileOnDiskIsRasterImage(pai->filename);

	if (diskOk && !force && !needNew) {
		pai->format = ProtoGetAvatarFileFormat(pai->filename);
		if (pai->format == PA_FORMAT_UNKNOWN)
			pai->format = ProtoGetAvatarFormat(pai->filename);
		return GAIR_SUCCESS;
	}

	if (onDisk && !diskOk)
		_wunlink(pai->filename);

	// Only real offline blocks fetch (CONNECTING=1 is numerically <= OFFLINE with wrong semantics if we used <=).
	if (GetStatus() == ID_STATUS_OFFLINE)
		return (onDisk && diskOk) ? GAIR_SUCCESS : GAIR_NOAVATAR;

	if (!DownloadAvatarToFile(h, szUrl, pai->filename, _countof(pai->filename))) {
		pai->format = ProtoGetAvatarFormat(pai->filename);
		return (::_waccess(pai->filename, 0) != 0 || !sttAvatarFileOnDiskIsRasterImage(pai->filename)) ? GAIR_NOAVATAR : GAIR_SUCCESS;
	}

	setByte(h, "NeedNewAvatar", 0);
	pai->format = ProtoGetAvatarFileFormat(pai->filename);
	if (pai->format == PA_FORMAT_UNKNOWN)
		pai->format = ProtoGetAvatarFormat(pai->filename);
	return GAIR_SUCCESS;
}

INT_PTR __cdecl CMaxProto::SvcGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		if (lParam) {
			((POINT *)lParam)->x = 512;
			((POINT *)lParam)->y = 512;
		}
		return 0;

	case AF_PROPORTION:
		return PIP_SQUARE;

	case AF_FORMATSUPPORTED:
		switch ((int)lParam) {
		case PA_FORMAT_PNG:
		case PA_FORMAT_JPEG:
		case PA_FORMAT_GIF:
		case PA_FORMAT_WEBP:
			return 1;
		}
		return 0;

	case AF_ENABLED:
	case AF_DONTNEEDDELAYS:
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;

	case AF_FETCHIFPROTONOTVISIBLE:
		return 0;
	}
	return 0;
}

INT_PTR __cdecl CMaxProto::SvcGetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = 0;
	if (SvcGetAvatarInfo(0, (LPARAM)&ai) != GAIR_SUCCESS)
		return 1;

	wchar_t *buf = (wchar_t *)wParam;
	int size = (int)lParam;
	if (buf == nullptr || size <= 0)
		return 1;

	wcsncpy_s(buf, size, ai.filename, _TRUNCATE);
	return 0;
}
