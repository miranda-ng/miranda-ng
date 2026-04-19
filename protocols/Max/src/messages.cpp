/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"

static CMStringA sttMsgJsonIdStr(const JSONNode &n)
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

static bool sttReadJsonInt(const JSONNode &n, int &out)
{
	if (n.type() == JSON_NUMBER) {
		out = (int)(n.as_float() + 0.5);
		return true;
	}

	if (n.type() == JSON_STRING) {
		const char *sz = n.as_string().c_str();
		if (sz == nullptr || sz[0] == 0)
			return false;

		out = atoi(sz);
		return true;
	}

	return false;
}

static bool sttBuildMirandaReactionsJson(const JSONNode &reactionInfo, JSONNode &out)
{
	if (reactionInfo.type() != JSON_NODE)
		return false;

	const JSONNode &counters = reactionInfo["counters"];
	const bool hasReactionInfo = (counters.type() == JSON_ARRAY
		|| reactionInfo["totalCount"].type() != JSON_NULL
		|| reactionInfo["yourReaction"].type() != JSON_NULL);
	if (!hasReactionInfo)
		return false;

	out = JSONNode(JSON_NODE);
	if (counters.type() != JSON_ARRAY)
		return true;

	for (unsigned i = 0; i < counters.size(); ++i) {
		const JSONNode &counter = counters[i];
		if (counter.type() != JSON_NODE)
			continue;

		CMStringA reaction = sttMsgJsonIdStr(counter["reaction"]);
		if (reaction.IsEmpty())
			continue;

		int count = 0;
		if (!sttReadJsonInt(counter["count"], count) || count <= 0)
			continue;

		auto it = out.find(reaction.c_str());
		if (it == out.end())
			out << INT_PARAM(reaction.c_str(), count);
		else
			(*it) = JSONNode(reaction.c_str(), (*it).as_int() + count);
	}

	return true;
}

static uint64_t sttPresenceTimeMs(const JSONNode &n)
{
	if (n.type() == JSON_NUMBER) {
		double v = n.as_float();
		if (v <= 0)
			return 0;
		// Server can return seconds or milliseconds.
		if (v < 20000000000.0)
			return (uint64_t)(v * 1000.0 + 0.5);
		return (uint64_t)(v + 0.5);
	}
	if (n.type() == JSON_STRING) {
		const char *s = n.as_string().c_str();
		if (s == nullptr || s[0] == 0)
			return 0;
		uint64_t v = _strtoui64(s, nullptr, 10);
		if (v == 0)
			return 0;
		if (v < 20000000000ull)
			return v * 1000ull;
		return v;
	}
	return 0;
}

static HANDLE sttGetReactionExtraIcon()
{
	static HANDLE hReactionExtra = ExtraIcon_RegisterIcolib("reaction", LPGEN("Reaction"), nullptr);
	return hReactionExtra;
}

static bool sttShouldNotifyReactionForEvent(const char *szModule, const char *szMsgId)
{
	if (szModule == nullptr || szMsgId == nullptr || szMsgId[0] == 0)
		return false;

	DB::EventInfo dbei(db_event_getById(szModule, szMsgId));
	if (!dbei || !dbei.bSent)
		return false;

	MessageWindowData mwd = {};
	if (dbei.hContact != 0 && Srmm_GetWindowData(dbei.hContact, mwd) == 0) {
		if ((mwd.uState & MSG_WINDOW_STATE_VISIBLE) && !(mwd.uState & MSG_WINDOW_STATE_ICONIC))
			return false;
	}

	return true;
}

void CMaxProto::SetMessageReactionsById(const char *szMsgId, JSONNode &reactions, const char *szSource, bool bNotify)
{
	if (szMsgId == nullptr || szMsgId[0] == 0)
		return;

	DB::EventInfo dbei(db_event_getById(m_szModuleName, szMsgId));
	if (!dbei) {
		debugLogA("Max: reactions skip source=%s msg=%s (event not found)", szSource ? szSource : "(unknown)", szMsgId);
		return;
	}

	if (bNotify)
		dbei.setReactions(reactions);
	else {
		auto &json = dbei.setJson();
		auto it = json.find("r");
		if (it != json.end())
			json.erase(it);

		if (!reactions.empty()) {
			reactions.set_name("r");
			json << reactions;
		}

		dbei.flushJson();
		db_event_setJson(dbei.getEvent(), dbei.pBlob);
	}

	if (dbei.hContact != 0) {
		MessageWindowData mwd = {};
		if (Srmm_GetWindowData(dbei.hContact, mwd) == 0 && mwd.pDlg != nullptr)
			mwd.pDlg->ScheduleRedrawLog();
	}

	debugLogA("Max: reactions updated source=%s msg=%s entries=%u", szSource ? szSource : "(unknown)", szMsgId, (unsigned)reactions.size());
}

static bool sttLoadStoredReactions(const char *szModule, const char *szMsgId, JSONNode &out)
{
	out = JSONNode(JSON_NODE);
	if (szModule == nullptr || szMsgId == nullptr || szMsgId[0] == 0)
		return false;

	DB::EventInfo dbei(db_event_getById(szModule, szMsgId));
	if (!dbei)
		return false;
	if (!(dbei.flags & DBEF_JSON) || dbei.pBlob == nullptr || dbei.pBlob[0] == 0)
		return false;

	JSONNode root = JSONNode::parse((const char *)dbei.pBlob);
	const JSONNode &reactions = root["r"];
	if (reactions.type() != JSON_NODE)
		return false;

	for (auto &it : reactions) {
		int count = 0;
		if (!sttReadJsonInt(it, count) || count <= 0)
			continue;

		out << INT_PARAM(it.name(), count);
	}

	return out.size() > 0;
}

static void sttEnsureReactionPresent(JSONNode &reactions, const char *emoji)
{
	if (emoji == nullptr || emoji[0] == 0)
		return;

	auto it = reactions.find(emoji);
	if (it == reactions.end())
		reactions << INT_PARAM(emoji, 1);
}

static int sttReactionInfoTotalCount(const JSONNode &reactionInfo)
{
	int total = 0;
	if (sttReadJsonInt(reactionInfo["totalCount"], total) && total >= 0)
		return total;

	const JSONNode &counters = reactionInfo["counters"];
	if (counters.type() != JSON_ARRAY)
		return 0;

	for (unsigned i = 0; i < counters.size(); ++i) {
		int count = 0;
		if (sttReadJsonInt(counters[i]["count"], count) && count > 0)
			total += count;
	}

	return total;
}

static bool sttShouldKeepReactionTracked(const JSONNode &reactionInfo)
{
	const int total = sttReactionInfoTotalCount(reactionInfo);
	if (total <= 0)
		return false;

	const JSONNode &yourReaction = reactionInfo["yourReaction"];
	const bool hasYourReaction = (yourReaction.type() == JSON_STRING && !yourReaction.as_string().empty());
	return total > (hasYourReaction ? 1 : 0);
}

static MCONTACT sttFindReactionChatContact(CMaxProto *pProto, const JSONNode &chat)
{
	if (pProto == nullptr || chat.type() != JSON_NODE)
		return 0;

	CMStringA chatId = sttMsgJsonIdStr(chat["id"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(chat["cid"]);
	if (chatId.IsEmpty())
		return 0;

	return pProto->FindContactByDialogChatId(chatId.c_str());
}

static void sttLoadTrackedReactionMsgIds(CMaxProto *pProto, MCONTACT hContact, std::vector<CMStringA> &out)
{
	out.clear();
	if (pProto == nullptr || hContact == 0)
		return;

	CMStringA raw(pProto->getMStringA(hContact, DB_KEY_MAX_REACTION_MSGIDS));
	int len = raw.GetLength();
	int start = 0;
	while (start < len) {
		int sep = raw.Find('|', start);
		if (sep < 0)
			sep = len;

		CMStringA token(raw.Mid(start, sep - start));
		if (!token.IsEmpty())
			out.push_back(token);

		start = sep + 1;
	}
}

static void sttStoreTrackedReactionMsgIds(CMaxProto *pProto, MCONTACT hContact, const std::vector<CMStringA> &ids)
{
	if (pProto == nullptr || hContact == 0)
		return;

	if (ids.empty()) {
		pProto->delSetting(hContact, DB_KEY_MAX_REACTION_MSGIDS);
		return;
	}

	CMStringA raw;
	for (const auto &it : ids) {
		if (it.IsEmpty())
			continue;
		if (!raw.IsEmpty())
			raw.AppendChar('|');
		raw.Append(it);
	}

	if (raw.IsEmpty())
		pProto->delSetting(hContact, DB_KEY_MAX_REACTION_MSGIDS);
	else
		pProto->setString(hContact, DB_KEY_MAX_REACTION_MSGIDS, raw.c_str());
}

static void sttTrackReactionMsgId(CMaxProto *pProto, MCONTACT hContact, const char *szMsgId)
{
	if (pProto == nullptr || hContact == 0 || szMsgId == nullptr || szMsgId[0] == 0)
		return;

	std::vector<CMStringA> ids;
	sttLoadTrackedReactionMsgIds(pProto, hContact, ids);
	for (const auto &it : ids)
		if (it == szMsgId)
			return;

	ids.push_back(szMsgId);
	sttStoreTrackedReactionMsgIds(pProto, hContact, ids);
}

static void sttForgetReactionMsgId(CMaxProto *pProto, MCONTACT hContact, const char *szMsgId)
{
	if (pProto == nullptr || hContact == 0 || szMsgId == nullptr || szMsgId[0] == 0)
		return;

	std::vector<CMStringA> ids;
	sttLoadTrackedReactionMsgIds(pProto, hContact, ids);
	ids.erase(std::remove_if(ids.begin(), ids.end(),
		[&](const CMStringA &it) { return it == szMsgId; }), ids.end());
	sttStoreTrackedReactionMsgIds(pProto, hContact, ids);
}

static void sttClearTrackedReactionMsgIds(CMaxProto *pProto, MCONTACT hContact, const char *szModule, const char *szSource)
{
	if (pProto == nullptr || hContact == 0 || szModule == nullptr)
		return;

	std::vector<CMStringA> ids;
	sttLoadTrackedReactionMsgIds(pProto, hContact, ids);
	for (const auto &it : ids) {
		if (it.IsEmpty())
			continue;

		JSONNode empty(JSON_NODE);
		pProto->SetMessageReactionsById(it.c_str(), empty, szSource,
			sttShouldNotifyReactionForEvent(szModule, it.c_str()));
	}

	pProto->delSetting(hContact, DB_KEY_MAX_REACTION_MSGIDS);
}

void CMaxProto::ApplyMessageReactionsById(const char *szMsgId, const JSONNode &reactionInfo, const char *szSource, bool bNotify)
{
	JSONNode reactions(JSON_NODE);
	if (!sttBuildMirandaReactionsJson(reactionInfo, reactions))
		return;

	SetMessageReactionsById(szMsgId, reactions, szSource, bNotify);
}

static void sttApplyAuthoritativeMessageReactions(CMaxProto *pProto, const char *szMsgId, const JSONNode &reactionInfo, const char *szSource, bool bNotify)
{
	if (pProto == nullptr || szMsgId == nullptr || szMsgId[0] == 0)
		return;

	if (reactionInfo.type() != JSON_NODE)
		return;

	if (sttReactionInfoTotalCount(reactionInfo) <= 0) {
		JSONNode empty(JSON_NODE);
		pProto->SetMessageReactionsById(szMsgId, empty, szSource, bNotify);
		return;
	}

	pProto->ApplyMessageReactionsById(szMsgId, reactionInfo, szSource, bNotify);
}

void CMaxProto::ApplyPresenceToContact(MCONTACT hContact, const JSONNode &src)
{
	if (hContact == 0 || src.type() != JSON_NODE || isChatRoom(hContact))
		return;

	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	ptrA contactUid(getStringA(hContact, DB_KEY_MAX_UID));
	const bool isSelfContact = (myUid != nullptr && myUid[0] != 0 && contactUid != nullptr && contactUid[0] != 0
		&& !mir_strcmp(myUid, contactUid));

	// Favorites/self dialog should follow protocol state, not server presence snapshots.
	if (isSelfContact) {
		setWord(hContact, "Status", (GetStatus() == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
		return;
	}

	if (JsonIndicatesMaxBot(src))
		setByte(hContact, DB_KEY_MAX_IS_BOT, 1);

	// Bots: online while protocol is online (ignore server "last seen" style presence).
	if (IsMaxBotMirrorContact(hContact)) {
		setWord(hContact, "Status", (GetStatus() == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
		return;
	}

	bool hasOnline = false, isOnline = false;

	auto takeBool = [](const JSONNode &n, bool &has, bool &val) {
		if (n.type() == JSON_BOOL) {
			has = true;
			val = n.as_bool();
		}
	};
	takeBool(src["online"], hasOnline, isOnline);
	if (!hasOnline)
		takeBool(src["isOnline"], hasOnline, isOnline);
	if (!hasOnline && src["status"].type() == JSON_NUMBER) {
		hasOnline = true;
		isOnline = ((int)src["status"].as_float() == 1);
	}
	if (!hasOnline && src["status"].type() == JSON_STRING) {
		CMStringA s(src["status"].as_string().c_str());
		s.MakeLower();
		if (s == "online") {
			hasOnline = true;
			isOnline = true;
		}
		else if (s == "offline") {
			hasOnline = true;
			isOnline = false;
		}
	}

	const JSONNode &st = src["status"];
	if (st.type() == JSON_NODE) {
		takeBool(st["online"], hasOnline, isOnline);
		if (!hasOnline)
			takeBool(st["isOnline"], hasOnline, isOnline);
		if (!hasOnline && st["status"].type() == JSON_NUMBER) {
			hasOnline = true;
			isOnline = ((int)st["status"].as_float() == 1);
		}
	}

	uint64_t lastMs = 0;
	lastMs = sttPresenceTimeMs(src["lastSeen"]);
	if (lastMs == 0)
		lastMs = sttPresenceTimeMs(src["lastSeenAt"]);
	if (lastMs == 0)
		lastMs = sttPresenceTimeMs(src["seen"]);
	if (lastMs == 0 && st.type() == JSON_NODE) {
		lastMs = sttPresenceTimeMs(st["lastSeen"]);
		if (lastMs == 0)
			lastMs = sttPresenceTimeMs(st["lastSeenAt"]);
		if (lastMs == 0)
			lastMs = sttPresenceTimeMs(st["seen"]);
	}

	// Some presence updates indicate offline by sending only "seen" timestamp (without explicit status).
	if (!hasOnline && lastMs != 0) {
		hasOnline = true;
		isOnline = false;
	}

	if (hasOnline)
		setWord(hContact, "Status", isOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);

	if (lastMs != 0) {
		uint32_t sec = (uint32_t)(lastMs / 1000ull);
		setDword(hContact, "LastSeen", sec);
	}
}

static bool sttMessageElementTags(const char *szType, const wchar_t *&pOpen, const wchar_t *&pClose)
{
	if (szType == nullptr || szType[0] == 0)
		return false;

	CMStringA t(szType);
	t.MakeUpper();
	if (t == "STRONG") {
		pOpen = L"[b]";
		pClose = L"[/b]";
		return true;
	}
	// NewStory has no heading-size BBCode, so map HEADING to strong emphasis.
	if (t == "HEADING") {
		pOpen = L"[b]";
		pClose = L"[/b]";
		return true;
	}
	if (t == "EMPHASIZED") {
		pOpen = L"[i]";
		pClose = L"[/i]";
		return true;
	}
	if (t == "UNDERLINE") {
		pOpen = L"[u]";
		pClose = L"[/u]";
		return true;
	}
	if (t == "STRIKETHROUGH") {
		pOpen = L"[s]";
		pClose = L"[/s]";
		return true;
	}
	if (t == "CODE") {
		pOpen = L"[code]";
		pClose = L"[/code]";
		return true;
	}
	if (t == "MONOSPACED") {
		pOpen = L"[code]";
		pClose = L"[/code]";
		return true;
	}
	if (t == "QUOTE") {
		pOpen = L"[quote]";
		pClose = L"[/quote]";
		return true;
	}
	return false;
}

static CMStringA sttMessageBodyUtf8(const JSONNode &msg)
{
	const JSONNode &t = msg["text"];
	if (t.type() != JSON_STRING || t.as_string().empty())
		return CMStringA();
	CMStringA rawText(t.as_string().c_str());

	const JSONNode &elems = msg["elements"];
	if (elems.type() != JSON_ARRAY || elems.size() == 0)
		return rawText;

	ptrW wszText(mir_utf8decodeW(rawText.c_str()));
	if (wszText == nullptr || wszText[0] == 0)
		return rawText;

	CMStringW src(wszText), out;
	const int total = src.GetLength();
	int fallbackFrom = 0;
	bool bUsedFormatting = false;
	std::map<int, std::vector<const wchar_t*>> openTags, closeTags;

	for (unsigned i = 0; i < elems.size(); ++i) {
		const JSONNode &e = elems[(json_index_t)i];
		if (e.type() != JSON_NODE)
			continue;

		int from = fallbackFrom;
		if (e["from"].type() == JSON_NUMBER)
			from = (int)(e["from"].as_float() + 0.5);
		else if (e["from"].type() == JSON_STRING)
			from = atoi(e["from"].as_string().c_str());

		int len = 0;
		if (e["length"].type() == JSON_NUMBER)
			len = (int)(e["length"].as_float() + 0.5);
		else if (e["length"].type() == JSON_STRING)
			len = atoi(e["length"].as_string().c_str());
		if (len <= 0)
			continue;

		if (from < 0)
			from = 0;
		if (from >= total)
			continue;
		int to = from + len;
		if (to > total)
			to = total;
		if (to <= from)
			continue;

		const wchar_t *pOpen = nullptr, *pClose = nullptr;
		if (e["type"].type() == JSON_STRING) {
			CMStringA type(e["type"].as_string().c_str());
			type.MakeUpper();
			if (type == "LINK") {
				CMStringW openTag = L"[url]";
				const JSONNode &attrs = e["attributes"];
				if (attrs.type() == JSON_NODE && attrs["url"].type() == JSON_STRING && !attrs["url"].as_string().empty()) {
					ptrW wUrl(mir_utf8decodeW(attrs["url"].as_string().c_str()));
					if (wUrl != nullptr && wUrl[0] != 0) {
						openTag = L"[url=";
						openTag.Append(wUrl);
						openTag.Append(L"]");
					}
				}
				openTags[from].push_back(mir_wstrdup(openTag.c_str()));
				closeTags[to].insert(closeTags[to].begin(), mir_wstrdup(L"[/url]"));
				bUsedFormatting = true;
			}
			else if (sttMessageElementTags(type.c_str(), pOpen, pClose)) {
				openTags[from].push_back(pOpen);
				// close in reverse opening order for nested segments on same boundary
				closeTags[to].insert(closeTags[to].begin(), pClose);
				bUsedFormatting = true;
			}
		}

		if (e["from"].type() == JSON_NULL)
			fallbackFrom = to;
	}

	if (!bUsedFormatting)
		return rawText;

	for (int i = 0; i < total; ++i) {
		auto itOpen = openTags.find(i);
		if (itOpen != openTags.end()) {
			for (auto *tag : itOpen->second)
				out.Append(tag);
		}

		out.AppendChar(src[i]);

		auto itClose = closeTags.find(i + 1);
		if (itClose != closeTags.end()) {
			for (auto *tag : itClose->second)
				out.Append(tag);
		}
	}

	for (auto &it : openTags)
		for (auto *tag : it.second)
			if (tag != nullptr && (tag[0] == L'[' && wcsncmp(tag, L"[url", 4) == 0))
				mir_free((void*)tag);
	for (auto &it : closeTags)
		for (auto *tag : it.second)
			if (tag != nullptr && !mir_wstrcmp(tag, L"[/url]"))
				mir_free((void*)tag);

	ptrA utf(mir_utf8encodeW(out.c_str()));
	return (utf != nullptr && utf[0] != 0) ? CMStringA(utf) : rawText;
}

struct CMaxIncomingFile
{
	CMStringA fileId;
	CMStringW name;
	int64_t size = -1;
	CMStringA directUrl;
	bool isPhoto = false;
};

static bool sttTakeHttpUrl(const JSONNode &a, const char *key, CMStringA &out)
{
	const JSONNode &n = a[key];
	if (n.type() != JSON_STRING || n.as_string().empty())
		return false;
	CMStringA v(n.as_string().c_str());
	if (_strnicmp(v.c_str(), "https://", 8) && _strnicmp(v.c_str(), "http://", 7))
		return false;
	out = v;
	return true;
}

static CMStringA sttSelectPhotoBestUrl(const JSONNode &a)
{
	CMStringA url;
	// Prefer "raw/original" variants when present.
	if (sttTakeHttpUrl(a, "baseRawUrl", url)) return url;
	if (sttTakeHttpUrl(a, "rawUrl", url)) return url;
	if (sttTakeHttpUrl(a, "originalUrl", url)) return url;
	if (sttTakeHttpUrl(a, "downloadUrl", url)) return url;
	if (sttTakeHttpUrl(a, "url", url)) return url;
	if (sttTakeHttpUrl(a, "baseUrl", url)) return url;
	return CMStringA();
}

static CMStringW sttPhotoFileNameFromAttach(const JSONNode &a, const CMStringA &fileId)
{
	// Max PHOTO push usually has no original filename; derive a stable one from payload mime + photoId.
	CMStringW ext = L"jpg";
	if (a["previewData"].type() == JSON_STRING) {
		CMStringA pd(a["previewData"].as_string().c_str());
		const char *pfx = "data:image/";
		if (!_strnicmp(pd.c_str(), pfx, mir_strlen(pfx))) {
			const char *p = pd.c_str() + mir_strlen(pfx);
			const char *q = strchr(p, ';');
			if (q != nullptr && q > p) {
				CMStringA mimeSub;
				for (const char *t = p; t < q; ++t) {
					char c = (char)tolower((unsigned char)*t);
					if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
						mimeSub.AppendChar(c);
				}
				if (!mimeSub.IsEmpty()) {
					if (!mir_strcmp(mimeSub.c_str(), "jpeg"))
						ext = L"jpg";
					else
						ext = mir_utf8decodeW(mimeSub.c_str());
				}
			}
		}
	}

	CMStringW out;
	out.Format(L"photo_%S.%s", fileId.c_str(), ext.c_str());
	return out;
}

static bool sttStickerHeaderContainsI(const char *pszHaystack, const char *pszNeedle)
{
	if (pszHaystack == nullptr || pszNeedle == nullptr || pszNeedle[0] == 0)
		return false;
	for (const char *p = pszHaystack; *p; ++p)
		if (!_strnicmp(p, pszNeedle, mir_strlen(pszNeedle)))
			return true;
	return false;
}

static bool sttStickerContentTypeSaysNonImage(const char *pszCt)
{
	if (pszCt == nullptr)
		return false;
	return sttStickerHeaderContainsI(pszCt, "text/html")
		|| sttStickerHeaderContainsI(pszCt, "application/json")
		|| sttStickerHeaderContainsI(pszCt, "text/plain");
}

static bool sttStickerBodyLooksLikeHtmlDocument(const char *p, size_t cb)
{
	size_t i = 0;
	if (cb >= 3 && (uint8_t)p[0] == 0xEF && (uint8_t)p[1] == 0xBB && (uint8_t)p[2] == 0xBF)
		i = 3;

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

static bool sttStickerBufferIsKnownRasterImage(const void *p, size_t cb)
{
	if (p == nullptr || cb < 12)
		return false;

	int fmt = ProtoGetBufferFormat(p, nullptr);
	return fmt == PA_FORMAT_PNG || fmt == PA_FORMAT_JPEG || fmt == PA_FORMAT_GIF || fmt == PA_FORMAT_BMP || fmt == PA_FORMAT_WEBP;
}

static const wchar_t* sttStickerExtFromFormat(int fmt)
{
	switch (fmt) {
	case PA_FORMAT_PNG: return L"png";
	case PA_FORMAT_JPEG: return L"jpg";
	case PA_FORMAT_GIF: return L"gif";
	case PA_FORMAT_BMP: return L"bmp";
	case PA_FORMAT_WEBP: return L"webp";
	}
	return L"png";
}

static int sttStickerFormatFromBuffer(const void *p, size_t cb)
{
	if (p == nullptr || cb < 12)
		return PA_FORMAT_UNKNOWN;
	return ProtoGetBufferFormat(p, nullptr);
}

static bool sttStickerHttpBodyIsImagePayload(MHttpResponse *pReply, const CMStringA &body)
{
	if (pReply == nullptr || body.GetLength() < 32)
		return false;
	if (sttStickerContentTypeSaysNonImage(pReply->FindHeader("Content-Type")))
		return false;

	const char *raw = body.c_str();
	size_t cb = (size_t)body.GetLength();
	if (sttStickerBodyLooksLikeHtmlDocument(raw, cb))
		return false;

	return sttStickerBufferIsKnownRasterImage(raw, cb);
}

static bool sttStickerFileOnDiskIsRasterImage(const wchar_t *wszPath)
{
	FILE *f = _wfopen(wszPath, L"rb");
	if (f == nullptr)
		return false;

	char hdr[64] = {};
	size_t cb = fread(hdr, 1, sizeof(hdr), f);
	fclose(f);
	return sttStickerBufferIsKnownRasterImage(hdr, cb);
}

static CMStringW sttStickerActualExtFromFile(const wchar_t *wszPath)
{
	FILE *f = _wfopen(wszPath, L"rb");
	if (f == nullptr)
		return L"";

	char hdr[64] = {};
	size_t cb = fread(hdr, 1, sizeof(hdr), f);
	fclose(f);

	int fmt = sttStickerFormatFromBuffer(hdr, cb);
	if (fmt == PA_FORMAT_UNKNOWN)
		return L"";

	return sttStickerExtFromFormat(fmt);
}

static bool sttStickerFindCachedPath(const CMStringW &basePath, CMStringW &outPath)
{
	outPath.Empty();

	_wfinddata_t c_file = {};
	CMStringW mask(basePath);
	mask += L".*";
	INT_PTR hFile = _wfindfirst(mask, &c_file);
	if (hFile < 0)
		return false;

	do {
		if (c_file.name[0] == L'.')
			continue;

		CMStringW candidate(basePath);
		candidate += L".";
		candidate += c_file.name;
		const int slash = candidate.ReverseFind(L'\\');
		if (slash >= 0)
			candidate = basePath.Left(slash + 1) + c_file.name;

		if (sttStickerFileOnDiskIsRasterImage(candidate.c_str())) {
			outPath = candidate;
			_findclose(hFile);
			return true;
		}
	} while (_wfindnext(hFile, &c_file) == 0);

	_findclose(hFile);
	return false;
}

static bool sttDownloadStickerToFile(CMaxProto *pProto, const char *szUrl, const CMStringW &basePath, CMStringW &outPath)
{
	outPath.Empty();
	if (pProto == nullptr || szUrl == nullptr || szUrl[0] == 0 || basePath.IsEmpty())
		return false;

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

		NLHR_PTR pReply(Netlib_HttpTransaction(pProto->m_hNetlibUser, &req));
		if (pReply == nullptr || pReply->resultCode < 200 || pReply->resultCode >= 300)
			return false;
		if (!sttStickerHttpBodyIsImagePayload(pReply, pReply->body))
			return false;

		int fmt = sttStickerFormatFromBuffer(pReply->body.c_str(), (size_t)pReply->body.GetLength());
		CMStringW finalPath(basePath);
		finalPath += L".";
		finalPath += sttStickerExtFromFormat(fmt);

		if (CreatePathToFileW(finalPath.c_str()) != 0)
			return false;

		FILE *out = _wfopen(finalPath.c_str(), L"wb");
		if (out == nullptr)
			return false;

		size_t cb = (size_t)pReply->body.GetLength();
		size_t wr = fwrite(pReply->body.c_str(), 1, cb, out);
		fclose(out);
		if (wr != cb) {
			_wunlink(finalPath.c_str());
			return false;
		}

		if (!sttStickerFileOnDiskIsRasterImage(finalPath.c_str())) {
			_wunlink(finalPath.c_str());
			return false;
		}

		outPath = finalPath;
		return true;
	};

	ptrA tok(pProto->getStringA(DB_KEY_LOGIN_TOKEN));
	CMStringA bearer;
	if (tok != nullptr && tok[0] != 0)
		bearer.Format("Bearer %s", tok.get());

	if (tok != nullptr && tok[0] != 0) {
		if (tryOnce(szUrl, "Authorization", bearer.c_str()))
			return true;
		if (tryOnce(szUrl, "Authorization", tok.get()))
			return true;
		if (tryOnce(szUrl, "X-Auth-Token", tok.get()))
			return true;
	}

	if (tryOnce(szUrl, nullptr, nullptr))
		return true;

	return false;
}

static bool sttBuildStickerSmileyToken(CMaxProto *pProto, const JSONNode &a, CMStringA &outToken)
{
	outToken.Empty();
	if (pProto == nullptr || !ServiceExists(MS_SMILEYADD_REPLACESMILEYS))
		return false;

	CMStringA stickerId = sttMsgJsonIdStr(a["stickerId"]);
	if (stickerId.IsEmpty())
		return false;

	CMStringA url;
	if (!sttTakeHttpUrl(a, "url", url))
		return false;

	CMStringW tokenW;
	tokenW.Format(L"STK{%S}", stickerId.c_str());

	CMStringW stickerDir(pProto->GetAvatarPath());
	stickerDir += L"\\Stickers";
	CreateDirectoryTreeW(stickerDir);

	CMStringW stickerBasePath;
	stickerBasePath.Format(L"%s\\%s", stickerDir.c_str(), tokenW.c_str());

	CMStringW stickerPath;
	bool bReady = false;
	if (sttStickerFindCachedPath(stickerBasePath, stickerPath)) {
		CMStringW actualExt = sttStickerActualExtFromFile(stickerPath.c_str());
		int dot = stickerPath.ReverseFind(L'.');
		CMStringW currentExt = (dot >= 0) ? stickerPath.Mid(dot + 1) : L"";
		currentExt.MakeLower();
		if (!actualExt.IsEmpty() && currentExt != actualExt) {
			CMStringW correctedPath(stickerBasePath);
			correctedPath += L".";
			correctedPath += actualExt;
			_wunlink(correctedPath.c_str());
			if (_wrename(stickerPath.c_str(), correctedPath.c_str()) == 0)
				stickerPath = correctedPath;
		}
		bReady = true;
	}

	if (!bReady)
		bReady = sttDownloadStickerToFile(pProto, url.c_str(), stickerBasePath, stickerPath);

	if (!bReady) {
		CMStringA safeUrl(MaxRedactUrlForLog(url.c_str()));
		pProto->debugLogA("Max: sticker preview download failed sticker=%s url=%s", stickerId.c_str(), safeUrl.IsEmpty() ? "(empty)" : safeUrl.c_str());
		return false;
	}

	SmileyAdd_LoadContactSmileys(SMADD_FILE, pProto->m_szModuleName, stickerPath.c_str());
	outToken.Format("STK{%s}", stickerId.c_str());
	return true;
}

static void sttAppendIncomingStickers(CMaxProto *pProto, const JSONNode &msg, CMStringA &text)
{
	const JSONNode &att = msg["attaches"];
	if (pProto == nullptr || att.type() != JSON_ARRAY)
		return;

	for (unsigned i = 0; i < att.size(); ++i) {
		const JSONNode &a = att[(json_index_t)i];
		if (a.type() != JSON_NODE || a["_type"].type() != JSON_STRING)
			continue;
		if (mir_strcmpi(a["_type"].as_string().c_str(), "STICKER"))
			continue;

		CMStringA stickerText;
		if (!sttBuildStickerSmileyToken(pProto, a, stickerText)) {
			if (!ServiceExists(MS_SMILEYADD_REPLACESMILEYS))
				stickerText = TranslateU("SmileyAdd plugin required to support stickers");
			else {
				CMStringA stickerId = sttMsgJsonIdStr(a["stickerId"]);
				if (stickerId.IsEmpty())
					stickerText = TranslateU("Sticker");
				else
					stickerText.Format("%s #%s", TranslateU("Sticker"), stickerId.c_str());
			}
		}

		if (!text.IsEmpty())
			text.AppendChar('\n');
		text.Append(stickerText);
	}
}

static bool sttTextLooksLikeStickerPlaceholder(const wchar_t *wszText)
{
	if (wszText == nullptr || wszText[0] == 0)
		return false;

	const wchar_t *pNeedSmiley = L"SmileyAdd plugin required to support stickers";
	const wchar_t *pSticker = L"Sticker";
	if (!mir_wstrcmp(wszText, pNeedSmiley) || !mir_wstrcmp(wszText, pSticker))
		return true;
	if (!wcsncmp(wszText, L"Sticker #", 9))
		return true;
	if (wcsstr(wszText, L"\nSmileyAdd plugin required to support stickers") != nullptr)
		return true;
	if (wcsstr(wszText, L"\nSticker") != nullptr)
		return true;
	return false;
}

static bool sttTextContainsStickerMarker(const char *szUtf8)
{
	return szUtf8 != nullptr && strstr(szUtf8, "STK{") != nullptr;
}

static void sttUpgradeExistingStickerPlaceholder(CMaxProto *pProto, MEVENT hExisting, const CMStringA &newTextUtf8)
{
	if (pProto == nullptr || hExisting == 0 || newTextUtf8.IsEmpty() || !sttTextContainsStickerMarker(newTextUtf8.c_str()))
		return;

	DB::EventInfo dbei(hExisting);
	if (!dbei || dbei.eventType != EVENTTYPE_MESSAGE || !dbei.bUtf)
		return;

	ptrW wszCurrent(dbei.getText());
	if (!sttTextLooksLikeStickerPlaceholder(wszCurrent))
		return;

	if (dbei.pBlob != nullptr)
		mir_free(dbei.pBlob);

	dbei.cbBlob = (int)mir_strlen(newTextUtf8.c_str()) + 1;
	dbei.pBlob = mir_strdup(newTextUtf8.c_str());
	dbei.bEdited = true;
	db_event_edit(hExisting, &dbei, true);
	pProto->debugLogA("Max: upgraded sticker placeholder msg=%s", dbei.szId ? dbei.szId : "(no-id)");
}

static void sttCollectStickerTokens(const char *szUtf8, std::vector<CMStringA> &out)
{
	out.clear();
	if (szUtf8 == nullptr || szUtf8[0] == 0)
		return;

	const char *p = szUtf8;
	while ((p = strstr(p, "STK{")) != nullptr) {
		const char *q = strchr(p + 4, '}');
		if (q == nullptr)
			break;

		CMStringA token;
		token.Append(p, int(q - p + 1));
		if (!token.IsEmpty()) {
			bool bExists = false;
			for (const auto &it : out) {
				if (it == token) {
					bExists = true;
					break;
				}
			}
			if (!bExists)
				out.push_back(token);
		}

		p = q + 1;
	}
}

static bool sttAnyOtherEventUsesStickerToken(CMaxProto *pProto, MEVENT hSkipEvent, const char *szToken)
{
	if (pProto == nullptr || szToken == nullptr || szToken[0] == 0)
		return false;

	for (auto &hContact : pProto->AccContacts()) {
		for (MEVENT hEv = db_event_first(hContact); hEv; hEv = db_event_next(hContact, hEv)) {
			if (hEv == hSkipEvent)
				continue;

			DB::EventInfo dbei(hEv, false);
			if (!dbei || dbei.eventType != EVENTTYPE_MESSAGE || dbei.szModule == nullptr)
				continue;
			if (mir_strcmp(dbei.szModule, pProto->m_szModuleName))
				continue;
			if (dbei.pBlob != nullptr && strstr((const char*)dbei.pBlob, szToken) != nullptr)
				return true;
		}
	}

	return false;
}

static void sttDeleteStickerCacheFiles(CMaxProto *pProto, const char *szToken)
{
	if (pProto == nullptr || szToken == nullptr || szToken[0] == 0)
		return;

	CMStringW stickerDir(pProto->GetAvatarPath());
	stickerDir += L"\\Stickers";

	ptrW wszToken(mir_utf8decodeW(szToken));
	if (wszToken == nullptr || wszToken[0] == 0)
		return;

	CMStringW basePath;
	basePath.Format(L"%s\\%s", stickerDir.c_str(), wszToken.get());

	_wfinddata_t c_file = {};
	CMStringW mask(basePath);
	mask += L".*";
	INT_PTR hFile = _wfindfirst(mask, &c_file);
	if (hFile < 0)
		return;

	const int slash = basePath.ReverseFind(L'\\');
	const CMStringW parentDir = (slash >= 0) ? basePath.Left(slash + 1) : CMStringW();
	do {
		if (c_file.name[0] == L'.' || (c_file.attrib & _A_SUBDIR))
			continue;

		CMStringW candidate(parentDir);
		candidate += c_file.name;
		if (sttStickerFileOnDiskIsRasterImage(candidate.c_str())) {
			if (DeleteFileW(candidate.c_str()))
				pProto->debugLogA("Max: sticker cache deleted file=%S", candidate.c_str());
		}
	} while (_wfindnext(hFile, &c_file) == 0);

	_findclose(hFile);
}

void CMaxProto::CleanupStickerCacheForEvent(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return;

	DB::EventInfo dbei(hDbEvent, false);
	if (!dbei || dbei.eventType != EVENTTYPE_MESSAGE || dbei.szModule == nullptr)
		return;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return;
	if (dbei.pBlob == nullptr || !strstr((const char*)dbei.pBlob, "STK{"))
		return;

	std::vector<CMStringA> tokens;
	sttCollectStickerTokens((const char*)dbei.pBlob, tokens);
	for (const auto &token : tokens) {
		if (token.IsEmpty())
			continue;
		if (sttAnyOtherEventUsesStickerToken(this, hDbEvent, token.c_str()))
			continue;

		sttDeleteStickerCacheFiles(this, token.c_str());
	}
}

static void sttCollectIncomingFiles(const JSONNode &msg, std::vector<CMaxIncomingFile> &out)
{
	const JSONNode &att = msg["attaches"];
	if (att.type() != JSON_ARRAY)
		return;

	for (unsigned i = 0; i < att.size(); ++i) {
		const JSONNode &a = att[(json_index_t)i];
		if (a.type() != JSON_NODE)
			continue;
		CMaxIncomingFile f;
		CMStringA at;
		if (a["_type"].type() == JSON_STRING)
			at = a["_type"].as_string().c_str();
		at.MakeUpper();

		if (!mir_strcmp(at.c_str(), "FILE")) {
			f.fileId = sttMsgJsonIdStr(a["fileId"]);
			if (f.fileId.IsEmpty())
				continue;
			if (a["name"].type() == JSON_STRING && !a["name"].as_string().empty())
				f.name = mir_utf8decodeW(a["name"].as_string().c_str());
			if (f.name.IsEmpty())
				f.name = L"file";
			if (a["size"].type() == JSON_NUMBER)
				f.size = (int64_t)(a["size"].as_float() + 0.5);
			else if (a["size"].type() == JSON_STRING)
				f.size = _strtoi64(a["size"].as_string().c_str(), nullptr, 10);
			out.push_back(f);
			continue;
		}

		if (!mir_strcmp(at.c_str(), "PHOTO")) {
			f.fileId = sttMsgJsonIdStr(a["photoId"]);
			if (f.fileId.IsEmpty())
				f.fileId = "photo";
			if (a["name"].type() == JSON_STRING && !a["name"].as_string().empty())
				f.name = mir_utf8decodeW(a["name"].as_string().c_str());
			if (a["size"].type() == JSON_NUMBER)
				f.size = (int64_t)(a["size"].as_float() + 0.5);
			else if (a["size"].type() == JSON_STRING)
				f.size = _strtoi64(a["size"].as_string().c_str(), nullptr, 10);
			f.directUrl = sttSelectPhotoBestUrl(a);
			if (f.directUrl.IsEmpty())
				continue;
			if (f.name.IsEmpty())
				f.name = sttPhotoFileNameFromAttach(a, f.fileId);
			f.isPhoto = true;
			out.push_back(f);
			continue;
		}
	}
}

static bool sttJsonNodeContainsTypingOn(const JSONNode &n)
{
	CMStringA s;
	if (n["event"].type() == JSON_STRING)
		s = n["event"].as_string().c_str();
	else if (n["type"].type() == JSON_STRING)
		s = n["type"].as_string().c_str();
	else if (n["action"].type() == JSON_STRING)
		s = n["action"].as_string().c_str();
	if (s.IsEmpty())
		return false;
	s.MakeLower();
	return strstr(s, "typing") != nullptr && strstr(s, "clear") == nullptr && strstr(s, "stop") == nullptr && strstr(s, "off") == nullptr;
}

static bool sttJsonNodeContainsTypingOff(const JSONNode &n)
{
	CMStringA s;
	if (n["event"].type() == JSON_STRING)
		s = n["event"].as_string().c_str();
	else if (n["type"].type() == JSON_STRING)
		s = n["type"].as_string().c_str();
	else if (n["action"].type() == JSON_STRING)
		s = n["action"].as_string().c_str();
	if (s.IsEmpty())
		return false;
	s.MakeLower();
	return strstr(s, "clear") != nullptr || strstr(s, "stop") != nullptr || strstr(s, "off") != nullptr;
}

static int sttDetectTypingStateFromMessage(const JSONNode &msg)
{
	// -1 unknown, 0 off, 1 on
	if (msg["typing"].type() == JSON_BOOL)
		return msg["typing"].as_bool() ? 1 : 0;
	if (msg["isTyping"].type() == JSON_BOOL)
		return msg["isTyping"].as_bool() ? 1 : 0;

	const JSONNode &att = msg["attaches"];
	if (att.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < att.size(); ++i) {
			const JSONNode &a = att[i];
			if (a.type() != JSON_NODE)
				continue;
			if (a["_type"].type() == JSON_STRING) {
				CMStringA t(a["_type"].as_string().c_str());
				t.MakeLower();
				if (strstr(t, "control") == nullptr)
					continue;
			}
			if (sttJsonNodeContainsTypingOff(a))
				return 0;
			if (sttJsonNodeContainsTypingOn(a))
				return 1;
		}
	}

	const JSONNode &elems = msg["elements"];
	if (elems.type() == JSON_ARRAY) {
		for (unsigned i = 0; i < elems.size(); ++i) {
			const JSONNode &e = elems[i];
			if (e.type() != JSON_NODE)
				continue;
			if (sttJsonNodeContainsTypingOff(e))
				return 0;
			if (sttJsonNodeContainsTypingOn(e))
				return 1;
		}
	}

	if (sttJsonNodeContainsTypingOff(msg))
		return 0;
	if (sttJsonNodeContainsTypingOn(msg))
		return 1;
	return -1;
}

/// 1:1 dialog id from Max: chatId = myMaxUid XOR peerMaxUid (decimal string ids).
static CMStringA sttXorPeerUidFromDialog(const char *szMyUid, const char *szChatId)
{
	if (szMyUid == nullptr || szMyUid[0] == 0 || szChatId == nullptr || szChatId[0] == 0)
		return CMStringA();
	uint64_t a = _strtoui64(szMyUid, nullptr, 10);
	uint64_t c = _strtoui64(szChatId, nullptr, 10);
	if (c == 0)
		return CMStringA();
	uint64_t b = a ^ c;
	CMStringA out;
	out.Format("%llu", (unsigned long long)b);
	return out;
}

MCONTACT CMaxProto::ResolveContactForDialogMessage(const char *szChatId, const char *senderUid)
{
	if (szChatId == nullptr || szChatId[0] == 0)
		return 0;

	MCONTACT hContact = FindContactByDialogChatId(szChatId);
	if (hContact)
		return hContact;

	for (auto &hc : AccContacts()) {
		if (isChatRoom(hc))
			continue;
		CMStringA rid = GetOrResolveDialogChatId(hc, false);
		if (!rid.IsEmpty() && !mir_strcmp(rid, szChatId))
			return hc;
	}

	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	const bool fromSelf = (senderUid != nullptr && senderUid[0] != 0 && myUid != nullptr && myUid[0] != 0
		&& !mir_strcmp(senderUid, myUid));

	if (!fromSelf) {
		if (senderUid != nullptr && senderUid[0] != 0) {
			hContact = FindContactByMaxUid(senderUid);
			if (hContact)
				return hContact;
			return EnsureUserContact(senderUid, nullptr, nullptr, szChatId);
		}
		return 0;
	}

	CMStringA peerStr = sttXorPeerUidFromDialog(myUid, szChatId);
	if (peerStr.IsEmpty() || !mir_strcmp(peerStr, myUid))
		return 0;

	hContact = FindContactByMaxUid(peerStr.c_str());
	if (hContact)
		return hContact;
	return EnsureUserContact(peerStr.c_str(), nullptr, nullptr, szChatId);
}

MCONTACT CMaxProto::FindContactByDialogChatId(const char *szChatId)
{
	if (szChatId == nullptr || szChatId[0] == 0)
		return 0;

	for (auto &hContact : AccContacts()) {
		if (isChatRoom(hContact))
			continue;
		ptrA cid(getStringA(hContact, DB_KEY_MAX_CHATID));
		if (cid != nullptr && !mir_strcmp(cid, szChatId))
			return hContact;
	}
	return 0;
}

CMStringA CMaxProto::GetOrResolveDialogChatId(MCONTACT hContact, bool bPersistIfDerived)
{
	if (hContact == 0)
		return CMStringA();

	CMStringA chatId(getMStringA(hContact, DB_KEY_MAX_CHATID));
	if (!chatId.IsEmpty())
		return chatId;

	ptrA peer(getStringA(hContact, DB_KEY_MAX_UID));
	if (peer == nullptr || peer[0] == 0)
		return CMStringA();

	ptrA my(getStringA(DB_KEY_MY_MAX_ID));
	if (my == nullptr || my[0] == 0)
		return CMStringA();

	uint64_t a = _strtoui64(my, nullptr, 10);
	uint64_t b = _strtoui64(peer, nullptr, 10);
	if (b == 0)
		return CMStringA();

	CMStringA resolved;
	resolved.Format("%llu", (unsigned long long)(a ^ b));

	if (bPersistIfDerived && !resolved.IsEmpty())
		setString(hContact, DB_KEY_MAX_CHATID, resolved.c_str());

	return resolved;
}

static CMStringA sttForwardPreambleUtf8(CMaxProto *pProto, const JSONNode &embMsg, uint64_t fallbackTimeMs)
{
	wchar_t wszDate[100];
	wszDate[0] = 0;
	uint64_t tMs = 0;
	const JSONNode &t = embMsg["time"];
	if (t.type() == JSON_NUMBER)
		tMs = (uint64_t)(t.as_float() + 0.5);
	else if (t.type() == JSON_STRING)
		tMs = _strtoui64(t.as_string().c_str(), nullptr, 10);
	if (tMs == 0)
		tMs = fallbackTimeMs;

	mir_time tsSec = 0;
	if (tMs >= 1000000000000ULL)
		tsSec = (mir_time)(tMs / 1000);
	else if (tMs > 0)
		tsSec = (mir_time)tMs;
	if (tsSec != 0)
		TimeZone_PrintTimeStamp(LOCAL_TIME_HANDLE, tsSec, L"d t", wszDate, _countof(wszDate), 0);

	CMStringW wszAuthor;
	const JSONNode &contactNd = embMsg["contact"];
	if (contactNd.type() == JSON_NODE) {
		CMStringW fn, ln;
		pProto->FillNameFromMaxContactJson(contactNd, fn, ln);
		if (!fn.IsEmpty() || !ln.IsEmpty()) {
			wszAuthor = fn;
			if (!ln.IsEmpty()) {
				if (!wszAuthor.IsEmpty())
					wszAuthor.AppendChar(L' ');
				wszAuthor += ln;
			}
		}
	}
	if (wszAuthor.IsEmpty()) {
		CMStringA uid = sttMsgJsonIdStr(embMsg["sender"]);
		if (!uid.IsEmpty()) {
			MCONTACT h = pProto->FindContactByMaxUid(uid.c_str());
			if (h) {
				CMStringW fn = pProto->getMStringW(h, "FirstName");
				CMStringW ln = pProto->getMStringW(h, "LastName");
				if (!fn.IsEmpty() || !ln.IsEmpty()) {
					wszAuthor = fn;
					if (!ln.IsEmpty()) {
						if (!wszAuthor.IsEmpty())
							wszAuthor.AppendChar(L' ');
						wszAuthor += ln;
					}
				}
			}
			if (wszAuthor.IsEmpty())
				wszAuthor = mir_utf8decodeW(uid.c_str());
		}
	}
	if (wszAuthor.IsEmpty())
		wszAuthor = TranslateT("Unknown");

	ptrA dateUtf8(mir_utf8encodeW(wszDate[0] ? wszDate : L"?"));
	ptrA authUtf8(mir_utf8encodeW(wszAuthor.c_str()));
	const char *pszWrote = TranslateU("wrote");
	if (pszWrote == nullptr || pszWrote[0] == 0)
		pszWrote = "wrote";

	CMStringA headline;
	headline.Format(">%s %s %s\r\n", dateUtf8.get(), authUtf8.get(), pszWrote);
	return headline;
}

void CMaxProto::IngestMaxMessageJson(const JSONNode &msg, const char *szChatId, bool bMarkRead, bool bSyncReactionState)
{
	if (msg.type() != JSON_NODE || szChatId == nullptr || szChatId[0] == 0)
		return;

	const JSONNode &typeNd = msg["type"];
	if (typeNd.type() == JSON_STRING && mir_strcmpi(typeNd.as_string().c_str(), "USER"))
		return;

	CMStringA msgId = sttMsgJsonIdStr(msg["id"]);
	if (msgId.IsEmpty())
		return;

	const JSONNode &st = msg["status"];
	if (st.type() == JSON_STRING && !mir_strcmpi(st.as_string().c_str(), "REMOVED")) {
		MEVENT hEv = db_event_getById(m_szModuleName, msgId.c_str());
		if (hEv != 0) {
			CleanupStickerCacheForEvent(hEv);
			db_event_delete(hEv, CDF_FROM_SERVER);
			debugLogA("Max: server removed msg id=%s chat=%s", msgId.c_str(), szChatId);
		}
		return;
	}

	CMStringA text = sttMessageBodyUtf8(msg);
	std::vector<CMaxIncomingFile> files;
	sttCollectIncomingFiles(msg, files);
	sttAppendIncomingStickers(this, msg, text);

	MEVENT hExisting = db_event_getById(m_szModuleName, msgId.c_str());
	bool isEdited = false;
	if (st.type() == JSON_STRING && !mir_strcmpi(st.as_string().c_str(), "EDITED"))
		isEdited = true;
	else if (msg["updateTime"].type() != JSON_NULL)
		isEdited = true;

	// Existing server message id is normally a duplicate push.
	// Keep only explicit edits to update text in place.
	if (hExisting != 0 && !isEdited) {
		sttUpgradeExistingStickerPlaceholder(this, hExisting, text);
		if (bSyncReactionState)
			sttApplyAuthoritativeMessageReactions(this, msgId.c_str(), msg["reactionInfo"], "message-sync", false);
		else
			ApplyMessageReactionsById(msgId.c_str(), msg["reactionInfo"], "message", false);
		return;
	}

	CMStringA sender = sttMsgJsonIdStr(msg["sender"]);
	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	const bool fromSelf = (myUid != nullptr && sender == myUid);
	CMStringA replyId;
	const JSONNode &link = msg["link"];
	bool isForward = false;
	if (link.type() == JSON_NODE && link["type"].type() == JSON_STRING) {
		CMStringA lt(link["type"].as_string().c_str());
		lt.MakeUpper();
		if (lt == "REPLY") {
			replyId = sttMsgJsonIdStr(link["messageId"]);
			if (replyId.IsEmpty() && link["mid"].type() != JSON_NULL)
				replyId = sttMsgJsonIdStr(link["mid"]);
			if (replyId.IsEmpty() && link["message"].type() == JSON_NODE)
				replyId = sttMsgJsonIdStr(link["message"]["id"]);
		}
		else if (lt == "FORWARD") {
			isForward = true;
		}
	}

	uint64_t msgTimeMs = 0;
	{
		const JSONNode &tmsg = msg["time"];
		if (tmsg.type() == JSON_NUMBER)
			msgTimeMs = (uint64_t)(tmsg.as_float() + 0.5);
		else if (tmsg.type() == JSON_STRING)
			msgTimeMs = _strtoui64(tmsg.as_string().c_str(), nullptr, 10);
	}

	// Forwarded messages may have empty outer "text" and carry content in link.message.
	if (text.IsEmpty() && files.empty() && isForward && link.type() == JSON_NODE && link["message"].type() == JSON_NODE) {
		const JSONNode &emb = link["message"];
		CMStringA fwd = sttMessageBodyUtf8(emb);
		CMStringA pre = sttForwardPreambleUtf8(this, emb, msgTimeMs);
		if (!fwd.IsEmpty())
			text = pre + fwd;
		else
			text = pre + CMStringA(TranslateU("Forwarded message"));
	}
	// Some forward payloads contain only link.chatId/messageId without embedded message body.
	// Keep a visible placeholder instead of dropping the event as "empty".
	if (text.IsEmpty() && files.empty() && isForward)
		text = TranslateU("Forwarded message");
	if (text.IsEmpty() && files.empty())
		return;

	MCONTACT hContact = ResolveContactForDialogMessage(szChatId, sender.IsEmpty() ? nullptr : sender.c_str());
	if (!hContact)
		return;

	const uint64_t tMs = msgTimeMs;

	if (!text.IsEmpty()) {
		DB::EventInfo dbei;
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.bUtf = true;
		dbei.szId = msgId.c_str();
		if (isEdited)
			dbei.bEdited = true;
		if (tMs != 0) {
			dbei.bMsec = true;
			dbei.iTimestamp = tMs;
		}
		else
			dbei.iTimestamp = (uint64_t)time(nullptr);

		dbei.cbBlob = (int)mir_strlen(text.c_str());
		dbei.pBlob = mir_strdup(text.c_str());
		if (!replyId.IsEmpty())
			dbei.szReplyId = replyId.c_str();

		if (fromSelf) {
			dbei.bSent = true;
			dbei.bRead = true;
		}
		else if (bMarkRead)
			dbei.bRead = true;

		ProtoChainRecvMsg(hContact, dbei);
		debugLogA("Max: ingested msg id=%s chat=%s from=%s", msgId.c_str(), szChatId, sender.c_str());
	}

	if (hExisting != 0 || !text.IsEmpty()) {
		if (bSyncReactionState)
			sttApplyAuthoritativeMessageReactions(this, msgId.c_str(), msg["reactionInfo"], "message-sync", false);
		else
			ApplyMessageReactionsById(msgId.c_str(), msg["reactionInfo"], "message", false);
	}

	if (!files.empty()) {
		debugLogA("Max: files detected chat=%s msg=%s count=%u", szChatId, msgId.c_str(), (unsigned)files.size());
		for (auto &f : files) {
			CMStringA eventId;
			eventId.Format("%s:file:%s", msgId.c_str(), f.fileId.c_str());
			if (db_event_getById(m_szModuleName, eventId.c_str()) != 0)
				continue;

			CMStringA url;
			if (!f.directUrl.IsEmpty()) {
				url = f.directUrl;
				// For PHOTO, try to resolve server file URL (can provide original format in some accounts),
				// then fall back to directUrl from attach.
				if (f.isPhoto && WaitForGatewayReady() && m_pGateway != nullptr) {
					CMStringA altUrl;
					if (ApiGetFileDownloadUrl(m_pGateway, szChatId, msgId.c_str(), _strtoi64(f.fileId.c_str(), nullptr, 10), altUrl) && !altUrl.IsEmpty()) {
						debugLogA("Max: photo url replaced by opcode 88 chat=%s msg=%s file=%s", szChatId, msgId.c_str(), f.fileId.c_str());
						url = altUrl;
					}
				}
			}
			else {
				if (!WaitForGatewayReady() || m_pGateway == nullptr) {
					debugLogA("Max: file url unresolved (gateway offline) chat=%s msg=%s file=%s", szChatId, msgId.c_str(), f.fileId.c_str());
					continue;
				}
				if (!ApiGetFileDownloadUrl(m_pGateway, szChatId, msgId.c_str(), _strtoi64(f.fileId.c_str(), nullptr, 10), url) || url.IsEmpty()) {
					debugLogA("Max: file url unresolved chat=%s msg=%s file=%s", szChatId, msgId.c_str(), f.fileId.c_str());
					continue;
				}
			}

			DB::EventInfo fdbei;
			fdbei.eventType = EVENTTYPE_FILE;
			fdbei.bUtf = true;
			// Offline cloud file: avoid SRFile live-transfer dialog ("cannot start transfer") for incoming files.
			// For self-sent files keep it non-temporary to prevent auto-download into Received Files path.
			fdbei.bTemporary = !fromSelf;
			fdbei.szId = eventId.c_str();
			if (tMs != 0) {
				fdbei.bMsec = true;
				fdbei.iTimestamp = tMs;
			}
			else fdbei.iTimestamp = (uint64_t)time(nullptr);
			if (fromSelf) {
				fdbei.bSent = true;
				fdbei.bRead = true;
			}
			else if (bMarkRead)
				fdbei.bRead = true;

			ptrW wszName(mir_wstrdup(f.name.c_str()));
			DB::FILE_BLOB blob(wszName, nullptr);
			blob.setUrl(url.c_str());
			if (f.size > 0)
				blob.setSize(f.size);
			if (!isChatRoom(hContact))
				setString(hContact, DB_KEY_MAX_CHATID, szChatId);
			ProtoChainRecvFile(hContact, blob, fdbei);
			debugLogA("Max: ingested file chat=%s h=%u msg=%s file=%s", szChatId, (unsigned)hContact, msgId.c_str(), f.fileId.c_str());
		}
	}
}

uint64_t CMaxProto::GetLastLocalMessageTimeMs(MCONTACT hContact)
{
	if (hContact == 0)
		return 0;

	for (MEVENT hEv = db_event_last(hContact); hEv; hEv = db_event_prev(hContact, hEv)) {
		DB::EventInfo ei(hEv, false);
		if (!ei)
			continue;
		if (mir_strcmp(ei.szModule, m_szModuleName))
			continue;
		if (ei.eventType != EVENTTYPE_MESSAGE)
			continue;
		if (ei.bMsec)
			return (uint64_t)ei.iTimestamp;
		return (uint64_t)ei.iTimestamp * 1000;
	}
	return 0;
}

void CMaxProto::TryIngestNotifMessagePayload(const JSONNode &payload)
{
	const JSONNode &msg = payload["message"];
	if (msg.type() != JSON_NODE)
		return;

	CMStringA chatId = sttMsgJsonIdStr(payload["chatId"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(payload["cid"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(msg["cid"]);
	if (chatId.IsEmpty())
		return;

	int typingState = sttDetectTypingStateFromMessage(msg);
	if (typingState != -1) {
		CMStringA sender = sttMsgJsonIdStr(msg["sender"]);
		MCONTACT hContact = ResolveContactForDialogMessage(chatId.c_str(), sender.IsEmpty() ? nullptr : sender.c_str());
		if (hContact != 0) {
			ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
			if (sender.IsEmpty() || myUid == nullptr || myUid[0] == 0 || mir_strcmp(sender.c_str(), myUid))
				CallService(MS_PROTO_CONTACTISTYPING, hContact, typingState ? 10 : PROTOTYPE_CONTACTTYPING_OFF);
		}
	}

	QueueLiveNotifIngest(payload);
}

void CMaxProto::TryIngestMessageReactionsPayload(const JSONNode &payload, int opcode)
{
	if (payload.type() != JSON_NODE)
		return;

	CMStringA msgId = sttMsgJsonIdStr(payload["messageId"]);
	if (msgId.IsEmpty())
		msgId = sttMsgJsonIdStr(payload["mid"]);
	if (msgId.IsEmpty() && payload["message"].type() == JSON_NODE)
		msgId = sttMsgJsonIdStr(payload["message"]["id"]);

	const JSONNode &reactionInfo = (payload["reactionInfo"].type() == JSON_NODE) ? payload["reactionInfo"] : payload;
	if (opcode == 156 && reactionInfo.type() == JSON_NODE && reactionInfo.empty()) {
		JSONNode empty(JSON_NODE);
		SetMessageReactionsById(msgId.IsEmpty() ? nullptr : msgId.c_str(), empty, "push156-clear", false);
		return;
	}

	if (opcode == 155 && !msgId.IsEmpty()) {
		CMStringA chatId = sttMsgJsonIdStr(payload["chatId"]);
		if (chatId.IsEmpty())
			chatId = sttMsgJsonIdStr(payload["cid"]);

		MCONTACT hReactionContact = chatId.IsEmpty() ? 0 : FindContactByDialogChatId(chatId.c_str());
		const int totalCount = sttReactionInfoTotalCount(reactionInfo);
		debugLogA("Max: push155 chat=%s msg=%s total=%d hContact=%u", chatId.c_str(), msgId.c_str(), totalCount, (unsigned)hReactionContact);
		if (totalCount <= 0) {
			JSONNode empty(JSON_NODE);
			debugLogA("Max: push155 clear msg=%s", msgId.c_str());
			SetMessageReactionsById(msgId.c_str(), empty, "push155-clear", false);
			if (hReactionContact != 0)
				sttForgetReactionMsgId(this, hReactionContact, msgId.c_str());
			return;
		}

		const bool bNotify155 = sttShouldNotifyReactionForEvent(m_szModuleName, msgId.c_str());
		debugLogA("Max: push155 apply msg=%s notify=%d", msgId.c_str(), bNotify155 ? 1 : 0);
		ApplyMessageReactionsById(msgId.c_str(), reactionInfo, "push155", bNotify155);
		if (hReactionContact != 0) {
			if (sttShouldKeepReactionTracked(reactionInfo))
				sttTrackReactionMsgId(this, hReactionContact, msgId.c_str());
			else
				sttForgetReactionMsgId(this, hReactionContact, msgId.c_str());
		}
		return;
	}

	const bool bNotify = (opcode != 156) && sttShouldNotifyReactionForEvent(m_szModuleName, msgId.IsEmpty() ? nullptr : msgId.c_str());
	ApplyMessageReactionsById(msgId.IsEmpty() ? nullptr : msgId.c_str(), reactionInfo, "push", bNotify);

	const JSONNode &chat = payload["chat"];
	if (chat.type() != JSON_NODE)
		return;
	if (chat["type"].type() == JSON_STRING && mir_strcmpi(chat["type"].as_string().c_str(), "DIALOG"))
		return;
	if (opcode != 135 && chat["lastReaction"].type() == JSON_NULL && chat["lastReactedMessageId"].type() == JSON_NULL)
		return;

	CMStringA lastReaction = sttMsgJsonIdStr(chat["lastReaction"]);
	CMStringA reactedMsgId = sttMsgJsonIdStr(chat["lastReactedMessageId"]);
	MCONTACT hReactionContact = sttFindReactionChatContact(this, chat);
	if (!lastReaction.IsEmpty() && reactedMsgId.IsEmpty() && chat["lastMessage"].type() == JSON_NODE)
		reactedMsgId = sttMsgJsonIdStr(chat["lastMessage"]["id"]);

	JSONNode reactions(JSON_NODE);
	if (!lastReaction.IsEmpty()) {
		if (reactedMsgId.IsEmpty())
			return;

		sttLoadStoredReactions(m_szModuleName, reactedMsgId.c_str(), reactions);
		sttEnsureReactionPresent(reactions, lastReaction.c_str());
		SetMessageReactionsById(reactedMsgId.c_str(), reactions, (opcode == 135) ? "push135-chat" : "push-chat",
			sttShouldNotifyReactionForEvent(m_szModuleName, reactedMsgId.c_str()));
		if (hReactionContact != 0)
			sttTrackReactionMsgId(this, hReactionContact, reactedMsgId.c_str());
	}
	else if (opcode == 135 && !reactedMsgId.IsEmpty() && sttLoadStoredReactions(m_szModuleName, reactedMsgId.c_str(), reactions) && reactions.size() == 1) {
		JSONNode empty(JSON_NODE);
		SetMessageReactionsById(reactedMsgId.c_str(), empty, "push135-chat-clear",
			sttShouldNotifyReactionForEvent(m_szModuleName, reactedMsgId.c_str()));
		if (hReactionContact != 0)
			sttForgetReactionMsgId(this, hReactionContact, reactedMsgId.c_str());
	}
	else if (opcode == 135 && reactedMsgId.IsEmpty() && hReactionContact != 0)
		sttClearTrackedReactionMsgIds(this, hReactionContact, m_szModuleName, "push135-chat-clear");
}

void CMaxProto::TryIngestTypingPayload(const JSONNode &payload)
{
	if (payload.type() != JSON_NODE)
		return;

	CMStringA chatId = sttMsgJsonIdStr(payload["chatId"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(payload["cid"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(payload["message"]["cid"]);
	if (chatId.IsEmpty())
		chatId = sttMsgJsonIdStr(payload["chat"]["id"]);
	if (chatId.IsEmpty()) {
		debugLogA("Max: typing skip (no chatId)");
		return;
	}

	CMStringA sender = sttMsgJsonIdStr(payload["userId"]);
	if (sender.IsEmpty())
		sender = sttMsgJsonIdStr(payload["sender"]);
	if (sender.IsEmpty())
		sender = sttMsgJsonIdStr(payload["uid"]);
	if (sender.IsEmpty() && payload["typingUsers"].type() == JSON_ARRAY && payload["typingUsers"].size() > 0)
		sender = sttMsgJsonIdStr(payload["typingUsers"][(json_index_t)0]);
	if (sender.IsEmpty() && payload["userIds"].type() == JSON_ARRAY && payload["userIds"].size() > 0)
		sender = sttMsgJsonIdStr(payload["userIds"][(json_index_t)0]);

	int typingState = -1;
	if (payload["typing"].type() == JSON_BOOL)
		typingState = payload["typing"].as_bool() ? 1 : 0;
	else if (payload["isTyping"].type() == JSON_BOOL)
		typingState = payload["isTyping"].as_bool() ? 1 : 0;
	else if (payload["typingUsers"].type() == JSON_ARRAY)
		typingState = payload["typingUsers"].size() > 0 ? 1 : 0;
	else if (payload["userIds"].type() == JSON_ARRAY)
		typingState = payload["userIds"].size() > 0 ? 1 : 0;
	else if (payload["message"].type() == JSON_NODE)
		typingState = sttDetectTypingStateFromMessage(payload["message"]);
	else {
		const JSONNode &ev = payload["event"];
		if (ev.type() == JSON_STRING && !ev.as_string().empty()) {
			CMStringA s(ev.as_string().c_str());
			s.MakeLower();
			typingState = (strstr(s, "stop") != nullptr || strstr(s, "clear") != nullptr || strstr(s, "off") != nullptr) ? 0 : 1;
		}
	}
	// Max NOTIF_TYPING (opcode 129) may carry only {chatId,userId} without explicit typing flag.
	// But payloads like {"type":"PHOTO"} are media notifications and must not trigger typing.
	if (typingState == -1 && !sender.IsEmpty()) {
		bool bLooksTyping = true;
		if (payload["type"].type() == JSON_STRING) {
			CMStringA typeStr(payload["type"].as_string().c_str());
			typeStr.MakeLower();
			bLooksTyping = (strstr(typeStr, "typing") != nullptr);
		}
		if (bLooksTyping)
			typingState = 1;
	}
	if (typingState == -1) {
		debugLogA("Max: typing skip chat=%s (unknown state format)", chatId.c_str());
		return;
	}

	MCONTACT hContact = ResolveContactForDialogMessage(chatId.c_str(), sender.IsEmpty() ? nullptr : sender.c_str());
	if (hContact == 0) {
		debugLogA("Max: typing skip chat=%s from=%s (contact not resolved)", chatId.c_str(), sender.IsEmpty() ? "(unknown)" : sender.c_str());
		return;
	}

	ptrA myUid(getStringA(DB_KEY_MY_MAX_ID));
	if (!sender.IsEmpty() && myUid != nullptr && myUid[0] != 0 && !mir_strcmp(sender.c_str(), myUid)) {
		debugLogA("Max: typing skip chat=%s (self sender)", chatId.c_str());
		return;
	}

	CallService(MS_PROTO_CONTACTISTYPING, hContact, typingState ? 10 : PROTOTYPE_CONTACTTYPING_OFF);
	debugLogA("Max: typing chat=%s from=%s state=%s", chatId.c_str(), sender.IsEmpty() ? "(unknown)" : sender.c_str(), typingState ? "on" : "off");
}

void CMaxProto::TryIngestPresencePayload(const JSONNode &payload, int opcode)
{
	if (payload.type() != JSON_NODE)
		return;
	if (opcode != 132 && opcode != 35 && opcode != 159 && payload["presence"].type() == JSON_NULL
		&& payload["status"].type() == JSON_NULL && payload["online"].type() == JSON_NULL
		&& payload["lastSeen"].type() == JSON_NULL && payload["lastActivity"].type() == JSON_NULL
		&& payload["contact"].type() == JSON_NULL && payload["contacts"].type() == JSON_NULL)
		return;

	auto applyOne = [&](const JSONNode &n, const char *fallbackUid) {
		if (n.type() != JSON_NODE)
			return;
		CMStringA uid = sttMsgJsonIdStr(n["userId"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["id"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["uid"]);
		if (uid.IsEmpty())
			uid = sttMsgJsonIdStr(n["contactId"]);
		if (uid.IsEmpty() && n["user"].type() == JSON_NODE) {
			uid = sttMsgJsonIdStr(n["user"]["id"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["user"]["userId"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["user"]["uid"]);
		}
		if (uid.IsEmpty() && n["contact"].type() == JSON_NODE) {
			uid = sttMsgJsonIdStr(n["contact"]["id"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["contact"]["contactId"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["contact"]["userId"]);
			if (uid.IsEmpty())
				uid = sttMsgJsonIdStr(n["contact"]["uid"]);
		}
		if (uid.IsEmpty() && fallbackUid != nullptr && fallbackUid[0] != 0)
			uid = fallbackUid;
		if (uid.IsEmpty())
			return;
		MCONTACT h = FindContactByMaxUid(uid.c_str());
		if (!h) {
			debugLogA("Max: presence skip uid=%s (no local contact)", uid.c_str());
			return;
		}
		ApplyPresenceToContact(h, n);
		debugLogA("Max: presence applied uid=%s hContact=%u", uid.c_str(), (unsigned)h);
	};

	CMStringA payloadUid = sttMsgJsonIdStr(payload["userId"]);
	if (payloadUid.IsEmpty())
		payloadUid = sttMsgJsonIdStr(payload["id"]);
	if (payloadUid.IsEmpty())
		payloadUid = sttMsgJsonIdStr(payload["uid"]);
	if (payloadUid.IsEmpty())
		payloadUid = sttMsgJsonIdStr(payload["contactId"]);

	if (payload["presence"].type() == JSON_NODE)
		applyOne(payload["presence"], payloadUid.IsEmpty() ? nullptr : payloadUid.c_str());
	if (payload["presence"].type() == JSON_ARRAY)
		for (unsigned i = 0; i < payload["presence"].size(); ++i)
			applyOne(payload["presence"][i], nullptr);
	if (payload["contacts"].type() == JSON_ARRAY)
		for (unsigned i = 0; i < payload["contacts"].size(); ++i)
			applyOne(payload["contacts"][i], nullptr);
	if (payload["contacts"].type() == JSON_NODE)
		for (auto it = payload["contacts"].begin(); it != payload["contacts"].end(); ++it)
			applyOne(*it, nullptr);
	if (payload["contact"].type() == JSON_NODE)
		applyOne(payload["contact"], nullptr);
	if (payload["user"].type() == JSON_NODE)
		applyOne(payload["user"], nullptr);

	const bool hasNestedPresence = (payload["presence"].type() != JSON_NULL
		|| payload["contacts"].type() != JSON_NULL
		|| payload["contact"].type() != JSON_NULL
		|| payload["user"].type() != JSON_NULL);
	if (!hasNestedPresence)
		applyOne(payload, nullptr);
}

void CMaxProto::IngestChatHistoryPayload(const JSONNode &payload, const char *szChatId, bool bMarkRead)
{
	if (szChatId == nullptr || szChatId[0] == 0)
		return;

	const JSONNode *msgs = nullptr;
	if (payload["messages"].type() == JSON_ARRAY)
		msgs = &payload["messages"];
	else {
		const JSONNode &ch = payload["chat"];
		if (ch.type() == JSON_NODE && ch["messages"].type() == JSON_ARRAY)
			msgs = &ch["messages"];
	}

	if (msgs == nullptr || msgs->type() != JSON_ARRAY)
		return;

	unsigned n = (unsigned)msgs->size();
	debugLogA("Max: history chat=%s messages=%u", szChatId, n);
	for (unsigned i = 0; i < n; i++) {
		const JSONNode &msg = (*msgs)[i];
		CMStringA msgId = sttMsgJsonIdStr(msg["id"]);
		if (msgId.IsEmpty())
			continue;

		const JSONNode &st = msg["status"];
		if (st.type() == JSON_STRING && !mir_strcmpi(st.as_string().c_str(), "REMOVED")) {
			if (MEVENT hEv = db_event_getById(m_szModuleName, msgId.c_str())) {
				CleanupStickerCacheForEvent(hEv);
				db_event_delete(hEv, CDF_FROM_SERVER);
			}
			continue;
		}

		IngestMaxMessageJson(msg, szChatId, bMarkRead, true);
	}

	if (bMarkRead) {
		MCONTACT hContact = ResolveContactForDialogMessage(szChatId, nullptr);
		if (hContact != 0)
			ExtraIcon_Clear(sttGetReactionExtraIcon(), hContact);
	}
}
