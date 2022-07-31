/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

bool DBGetContactSettingAsString(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	buff[0] = 0;

	if (!szModuleName || !szSettingName)
		return false;

	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		switch (dbv.type) {
		case DBVT_BYTE:
			_itow(dbv.bVal, buff, 10);
			break;
		case DBVT_WORD:
			_ltow(dbv.wVal, buff, 10);
			break;
		case DBVT_DWORD:
			_ltow(dbv.dVal, buff, 10);
			break;
		case DBVT_ASCIIZ:
			if (dbv.pszVal) a2t(dbv.pszVal, buff, bufflen);
			buff[bufflen - 1] = 0;
			break;
		case DBVT_UTF8:
			if (dbv.pszVal) utf2t(dbv.pszVal, buff, bufflen);
			buff[bufflen - 1] = 0;
			break;

		case DBVT_WCHAR:
			if (dbv.pwszVal) wcsncpy(buff, dbv.pwszVal, bufflen);
			buff[bufflen - 1] = 0;
			break;
		}

		db_free(&dbv);
	}

	return buff[0] ? true : false;
}

bool CheckContactType(MCONTACT hContact, const DISPLAYITEM &di)
{
	if (di.type == DIT_ALL)
		return true;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto) {
		if (Contact::IsGroupChat(hContact, szProto))
			return di.type == DIT_CHATS;
		else
			return di.type == DIT_CONTACTS;
	}

	return false;
}

void StripBBCodesInPlace(wchar_t *ptszText)
{
	if (!g_plugin.getByte("StripBBCodes", 1))
		return;

	if (ptszText == nullptr)
		return;

	size_t iRead = 0, iWrite = 0;
	size_t iLen = mir_wstrlen(ptszText);

	while (iRead <= iLen) { // copy terminating null too
		while (iRead <= iLen && ptszText[iRead] != '[') {
			if (ptszText[iRead] != ptszText[iWrite]) ptszText[iWrite] = ptszText[iRead];
			iRead++; iWrite++;
		}

		if (iRead > iLen)
			break;

		if (iLen - iRead >= 3 && (wcsnicmp(ptszText + iRead, L"[b]", 3) == 0 || wcsnicmp(ptszText + iRead, L"[i]", 3) == 0))
			iRead += 3;
		else if (iLen - iRead >= 4 && (wcsnicmp(ptszText + iRead, L"[/b]", 4) == 0 || wcsnicmp(ptszText + iRead, L"[/i]", 4) == 0))
			iRead += 4;
		else if (iLen - iRead >= 6 && (wcsnicmp(ptszText + iRead, L"[color", 6) == 0)) {
			while (iRead < iLen && ptszText[iRead] != ']') iRead++;
			iRead++;// skip the ']'
		}
		else if (iLen - iRead >= 8 && (wcsnicmp(ptszText + iRead, L"[/color]", 8) == 0))
			iRead += 8;
		else if (iLen - iRead >= 5 && (wcsnicmp(ptszText + iRead, L"[size", 5) == 0)) {
			while (iRead < iLen && ptszText[iRead] != ']') iRead++;
			iRead++;// skip the ']'
		}
		else if (iLen - iRead >= 7 && (wcsnicmp(ptszText + iRead, L"[/size]", 7) == 0))
			iRead += 7;
		else {
			if (ptszText[iRead] != ptszText[iWrite]) ptszText[iWrite] = ptszText[iRead];
			iRead++; iWrite++;
		}
	}
}

uint32_t LastMessageTimestamp(MCONTACT hContact, bool received)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT) == received)
			return dbei.timestamp;
	}

	return 0;
}

void FormatTimestamp(uint32_t ts, char *szFormat, wchar_t *buff, int bufflen)
{
	wchar_t swzForm[16];
	a2t(szFormat, swzForm, 16);
	TimeZone_ToStringT(ts, swzForm, buff, bufflen);
}

bool Uid(MCONTACT hContact, char *szProto, wchar_t *buff, int bufflen)
{
	char *tmpProto = (hContact ? Proto_GetBaseAccountName(hContact) : szProto);
	if (tmpProto) {
		const char *szUid = Proto_GetUniqueId(tmpProto);
		if (szUid)
			return DBGetContactSettingAsString(hContact, tmpProto, szUid, buff, bufflen);
	}

	return false;
}

bool UidName(char *szProto, wchar_t *buff, int bufflen)
{
	if (szProto) {
		wchar_t *szUidName = (wchar_t *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
		if (szUidName && (INT_PTR)szUidName != CALLSERVICE_NOTFOUND) {
			wcsncpy_s(buff, bufflen, szUidName, _TRUNCATE);
			return true;
		}
	}
	return false;
}

wchar_t* GetLastMessageText(MCONTACT hContact, bool received)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT) == received) {
			dbei.pBlob = (uint8_t *)alloca(dbei.cbBlob);
			db_event_get(hDbEvent, &dbei);
			if (dbei.cbBlob == 0 || dbei.pBlob == nullptr)
				return nullptr;

			wchar_t *buff = DbEvent_GetTextW(&dbei, CP_ACP);
			wchar_t *swzMsg = mir_wstrdup(buff);
			mir_free(buff);

			StripBBCodesInPlace(swzMsg);
			return swzMsg;
		}
	}

	return nullptr;
}

bool CanRetrieveStatusMsg(MCONTACT hContact, char *szProto)
{
	if (opt.bGetNewStatusMsg) {
		int iFlags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0);
		uint16_t wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		if ((CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) && (iFlags & Proto_Status2Flag(wStatus))) {
			iFlags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & (PF1_VISLIST | PF1_INVISLIST);
			if (opt.bDisableIfInvisible && iFlags) {
				int iVisMode = db_get_w(hContact, szProto, "ApparentMode", 0);
				int wProtoStatus = Proto_GetStatus(szProto);
				if ((iVisMode == ID_STATUS_OFFLINE) || (wProtoStatus == ID_STATUS_INVISIBLE && iVisMode != ID_STATUS_ONLINE))
					return false;
				return true;
			}
			return true;
		}
	}

	return false;
}

wchar_t* GetStatusMessageText(MCONTACT hContact)
{
	wchar_t *swzMsg = nullptr;
	DBVARIANT dbv;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto) {
		if (!mir_strcmp(szProto, META_PROTO))
			hContact = db_mc_getMostOnline(hContact);
		else {
			uint16_t wStatus = Proto_GetStatus(szProto);
			if (wStatus == ID_STATUS_OFFLINE)
				return nullptr;

			if (!g_plugin.getWString(hContact, "TempStatusMsg", &dbv)) {
				if (mir_wstrlen(dbv.pwszVal) != 0)
					swzMsg = mir_wstrdup(dbv.pwszVal);
				db_free(&dbv);
			}
		}

		if (!swzMsg) {
			if (CanRetrieveStatusMsg(hContact, szProto))
				if (ProtoChainSend(hContact, PSS_GETAWAYMSG, 0, 0))
					return nullptr;

			if (!db_get_ws(hContact, "CList", "StatusMsg", &dbv)) {
				if (mir_wstrlen(dbv.pwszVal) != 0)
					swzMsg = mir_wstrdup(dbv.pwszVal);
				db_free(&dbv);
			}
		}
	}

	if (swzMsg)
		StripBBCodesInPlace(swzMsg);

	return swzMsg;
}

bool GetSysSubstText(MCONTACT hContact, wchar_t *swzRawSpec, wchar_t *buff, int bufflen)
{
	bool recv = false;

	if (!mir_wstrcmp(swzRawSpec, L"uid"))
		return Uid(hContact, nullptr, buff, bufflen);

	if (!mir_wstrcmp(swzRawSpec, L"proto")) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto) {
			a2t(szProto, buff, bufflen);
			return true;
		}
	}
	else if (!mir_wstrcmp(swzRawSpec, L"account")) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if ((INT_PTR)szProto == CALLSERVICE_NOTFOUND) {
			return GetSysSubstText(hContact, L"proto", buff, bufflen);
		}
		else if (szProto) {
			PROTOACCOUNT *pa = Proto_GetAccount(szProto);
			if (pa && pa->tszAccountName) {
				wcsncpy(buff, pa->tszAccountName, bufflen);
				return true;
			}
			else
				return GetSysSubstText(hContact, L"proto", buff, bufflen);
		}
	}
	else if (!mir_wstrcmp(swzRawSpec, L"time")) {
		if (!printDateTimeByContact(hContact, L"t", buff, bufflen, TZF_KNOWNONLY))
			return true;
	}
	else if (!mir_wstrcmp(swzRawSpec, L"uidname")) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		return UidName(szProto, buff, bufflen);
	}
	else if (!mir_wstrcmp(swzRawSpec, L"status_msg")) {
		wchar_t *swzMsg = GetStatusMessageText(hContact);
		if (swzMsg) {
			wcsncpy(buff, swzMsg, bufflen);
			mir_free(swzMsg);
			return true;
		}
	}
	else if ((recv = !mir_wstrcmp(swzRawSpec, L"last_msg")) || !mir_wstrcmp(swzRawSpec, L"last_msg_out")) {
		wchar_t *swzMsg = GetLastMessageText(hContact, recv);
		if (swzMsg) {
			wcsncpy(buff, swzMsg, bufflen);
			mir_free(swzMsg);
			return true;
		}
	}
	else if (!mir_wstrcmp(swzRawSpec, L"meta_subname")) {
		// get contact list name of active subcontact
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		if (!hSubContact)
			return false;

		wchar_t *swzNick = Clist_GetContactDisplayName(hSubContact);
		if (swzNick)
			wcsncpy(buff, swzNick, bufflen);
		return true;
	}
	else if (!mir_wstrcmp(swzRawSpec, L"meta_subuid")) {
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND)
			return false;
		return Uid(hSubContact, nullptr, buff, bufflen);
	}
	else if (!mir_wstrcmp(swzRawSpec, L"meta_subproto")) {
		// get protocol of active subcontact
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND)
			return false;
		return GetSysSubstText(hSubContact, L"account", buff, bufflen);
	}
	else if ((recv = !mir_wstrcmp(swzRawSpec, L"last_msg_time")) || !mir_wstrcmp(swzRawSpec, L"last_msg_out_time")) {
		uint32_t ts = LastMessageTimestamp(hContact, recv);
		if (ts == 0) return false;
		FormatTimestamp(ts, "t", buff, bufflen);
		return true;
	}
	else if ((recv = !mir_wstrcmp(swzRawSpec, L"last_msg_date")) || !mir_wstrcmp(swzRawSpec, L"last_msg_out_date")) {
		uint32_t ts = LastMessageTimestamp(hContact, recv);
		if (ts == 0) return false;
		FormatTimestamp(ts, "d", buff, bufflen);
		return true;
	}
	else if ((recv = !mir_wstrcmp(swzRawSpec, L"last_msg_reltime")) || !mir_wstrcmp(swzRawSpec, L"last_msg_out_reltime")) {
		uint32_t ts = LastMessageTimestamp(hContact, recv);
		if (ts == 0) return false;
		uint32_t t = (uint32_t)time(0);
		uint32_t diff = (t - ts);
		int d = (diff / 60 / 60 / 24);
		int h = (diff - d * 60 * 60 * 24) / 60 / 60;
		int m = (diff - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		if (d > 0) mir_snwprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
		else if (h > 0) mir_snwprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
		else mir_snwprintf(buff, bufflen, TranslateT("%dm"), m);
		return true;
	}
	else if (!mir_wstrcmp(swzRawSpec, L"msg_count_all") || !mir_wstrcmp(swzRawSpec, L"msg_count_out") || !mir_wstrcmp(swzRawSpec, L"msg_count_in")) {
		uint32_t dwCountOut, dwCountIn;
		uint32_t dwMetaCountOut = 0, dwMetaCountIn = 0;
		uint32_t dwLastTs, dwNewTs, dwRecountTs;
		uint32_t dwTime, dwDiff;
		int iNumber = 1;
		MCONTACT hTmpContact = hContact;

		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto && !mir_strcmp(szProto, META_PROTO)) {
			iNumber = db_mc_getSubCount(hContact);
			hTmpContact = db_mc_getSub(hContact, 0);
		}

		for (int i = 0; i < iNumber; i++) {
			if (i > 0)
				hTmpContact = db_mc_getSub(hContact, i);
			dwRecountTs = g_plugin.getDword(hTmpContact, "LastCountTS");
			dwTime = (uint32_t)time(0);
			dwDiff = (dwTime - dwRecountTs);
			if (dwDiff > (60 * 60 * 24 * 3)) {
				g_plugin.setDword(hTmpContact, "LastCountTS", dwTime);
				dwCountOut = dwCountIn = dwLastTs = 0;
			}
			else {
				dwCountOut = g_plugin.getDword(hTmpContact, "MsgCountOut");
				dwCountIn = g_plugin.getDword(hTmpContact, "MsgCountIn");
				dwLastTs = g_plugin.getDword(hTmpContact, "LastMsgTS");
			}

			dwNewTs = dwLastTs;

			MEVENT dbe = db_event_last(hTmpContact);
			while (dbe != NULL) {
				DBEVENTINFO dbei = {};
				if (!db_event_get(dbe, &dbei)) {
					if (dbei.eventType == EVENTTYPE_MESSAGE) {
						dwNewTs = max(dwNewTs, dbei.timestamp);
						if (dbei.timestamp > dwLastTs) {
							if (dbei.flags & DBEF_SENT) dwCountOut++;
							else dwCountIn++;
						}
						else break;
					}
				}
				dbe = db_event_prev(hTmpContact, dbe);
			}

			if (dwNewTs > dwLastTs) {
				g_plugin.setDword(hTmpContact, "MsgCountOut", dwCountOut);
				g_plugin.setDword(hTmpContact, "MsgCountIn", dwCountIn);
				g_plugin.setDword(hTmpContact, "LastMsgTS", dwNewTs);
			}

			dwMetaCountOut += dwCountOut;
			dwMetaCountIn += dwCountIn;
		}

		if (!mir_wstrcmp(swzRawSpec, L"msg_count_out"))
			mir_snwprintf(buff, bufflen, L"%d", dwMetaCountOut);
		else if (!mir_wstrcmp(swzRawSpec, L"msg_count_in"))
			mir_snwprintf(buff, bufflen, L"%d", dwMetaCountIn);
		else
			mir_snwprintf(buff, bufflen, L"%d", dwMetaCountOut + dwMetaCountIn);
		return true;
	}

	return false;
}

bool GetSubstText(MCONTACT hContact, const DISPLAYSUBST &ds, wchar_t *buff, int bufflen)
{
	TranslateFunc *transFunc = nullptr;
	for (int i = 0; i < iTransFuncsCount; i++)
		if (translations[i].id == (uint32_t)ds.iTranslateFuncId) {
			transFunc = translations[i].transFunc;
			break;
		}

	if (!transFunc)
		return false;

	switch (ds.type) {
	case DVT_DB:
		return transFunc(hContact, ds.szModuleName, ds.szSettingName, buff, bufflen) != nullptr;
	case DVT_PROTODB:
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto) {
			if (transFunc(hContact, szProto, ds.szSettingName, buff, bufflen) != nullptr)
				return true;
			return transFunc(hContact, "UserInfo", ds.szSettingName, buff, bufflen) != nullptr;
		}
		break;
	}
	return false;
}

bool GetRawSubstText(MCONTACT hContact, char *szRawSpec, wchar_t *buff, int bufflen)
{
	size_t lenght = mir_strlen(szRawSpec);
	for (size_t i = 0; i < lenght; i++) {
		if (szRawSpec[i] == '/') {
			szRawSpec[i] = 0;
			if (mir_strlen(szRawSpec) == 0) {
				char *szProto = Proto_GetBaseAccountName(hContact);
				if (szProto) {
					if (translations[0].transFunc(hContact, szProto, &szRawSpec[i + 1], buff, bufflen) != nullptr)
						return true;
					return translations[0].transFunc(hContact, "UserInfo", &szRawSpec[i + 1], buff, bufflen) != nullptr;
				}
				return false;
			}
			return translations[0].transFunc(hContact, szRawSpec, &szRawSpec[i + 1], buff, bufflen) != nullptr;
		}
	}
	return false;
}

bool ApplySubst(MCONTACT hContact, const wchar_t *swzSource, bool parseTipperVarsFirst, wchar_t *swzDest, size_t iDestLen)
{
	// hack - allow empty strings before passing to variables (note - zero length strings return false after this)
	if (swzDest && swzSource && (*swzSource == 0)) {
		swzDest[0] = 0;
		return true;
	}

	// pass to variables plugin if available
	wchar_t *swzVarSrc = (parseTipperVarsFirst ? mir_wstrdup(swzSource) : variables_parsedup((wchar_t *)swzSource, nullptr, hContact));

	size_t iSourceLen = mir_wstrlen(swzVarSrc);
	size_t si = 0, di = 0, v = 0;

	wchar_t swzVName[LABEL_LEN], swzRep[VALUE_LEN], swzAlt[VALUE_LEN];
	while (si < iSourceLen && di < iDestLen - 1) {
		if (swzVarSrc[si] == '%') {
			si++;
			v = 0;
			while (si < iSourceLen && v < LABEL_LEN - 1) {
				if (swzVarSrc[si] == '%')
					break;

				swzVName[v] = swzVarSrc[si];
				v++; si++;
			}

			if (v == 0)  // bSubst len is 0 - just a % symbol
				swzDest[di] = '%';
			else if (si < iSourceLen) // we found end %
			{
				swzVName[v] = 0;

				bool bAltSubst = false, bSubst = false;

				// apply only to specific protocols
				wchar_t *p = wcsrchr(swzVName, '^'); // use last '^', so if you want a ^ in swzAlt text, you can just put a '^' on the end
				if (p) {
					*p = 0;
					p++;
					if (*p) {
						char *cp = Proto_GetBaseAccountName(hContact);
						if (cp != nullptr) {
							PROTOACCOUNT *acc = Proto_GetAccount(cp);
							if (acc != nullptr) {
								cp = acc->szProtoName;
							}
						}

						if (cp == nullptr)
							goto empty;

						bool negate = false;
						if (*p == '!') {
							p++;
							if (*p == 0) goto error;
							negate = true;
						}

						char sproto[256];
						bool spec = false;
						int len;

						wchar_t *last = wcsrchr(p, ',');
						if (!last) last = p;

						while (p <= last + 1) {
							len = (int)wcscspn(p, L",");
							t2a(p, sproto, len);
							sproto[len] = 0;
							p += len + 1;

							if (_stricmp(cp, sproto) == 0) {
								spec = true;
								break;
							}
						}

						if (negate ? spec : !spec)
							goto empty;
					}
				}

				// get alternate text, if bSubst fails
				swzAlt[0] = 0;
				p = wcschr(swzVName, '|'); // use first '|' - so you can use the '|' symbol in swzAlt text
				if (p) {
					*p = 0; // clip swzAlt from swzVName
					p++;
					if (mir_wstrlen(p) > 4 && wcsncmp(p, L"raw:", 4) == 0) { // raw db substitution
						char raw_spec[LABEL_LEN];
						p += 4;
						t2a(p, raw_spec, LABEL_LEN);
						GetRawSubstText(hContact, raw_spec, swzAlt, VALUE_LEN);
					}
					else if (mir_wstrlen(p) > 4 && wcsncmp(p, L"sys:", 4) == 0) { // 'system' substitution
						p += 4;
						GetSysSubstText(hContact, p, swzAlt, VALUE_LEN);
					}
					else {
						// see if we can find the bSubst
						DSListNode *ds_node = opt.dsList;
						while (ds_node) {
							if (mir_wstrcmp(ds_node->ds.swzName, p) == 0)
								break;

							ds_node = ds_node->next;
						}

						if (ds_node)
							GetSubstText(hContact, ds_node->ds, swzAlt, VALUE_LEN);
						else {
							wcsncpy(swzAlt, p, VALUE_LEN);
							bAltSubst = true;
						}
					}
					swzAlt[VALUE_LEN - 1] = 0;
					if (mir_wstrlen(swzAlt) != 0)
						bAltSubst = true;
				}

				// get bSubst text
				if (v > 4 && wcsncmp(swzVName, L"raw:", 4) == 0) // raw db substitution
				{
					char raw_spec[LABEL_LEN];
					t2a(&swzVName[4], raw_spec, LABEL_LEN);
					bSubst = GetRawSubstText(hContact, raw_spec, swzRep, VALUE_LEN);
				}
				else if (v > 4 && wcsncmp(swzVName, L"sys:", 4) == 0) // 'system' substitution
				{
					bSubst = GetSysSubstText(hContact, &swzVName[4], swzRep, VALUE_LEN);
				}
				else {
					// see if we can find the bSubst
					DSListNode *ds_node = opt.dsList;
					while (ds_node) {
						if (mir_wstrcmp(ds_node->ds.swzName, swzVName) == 0)
							break;

						ds_node = ds_node->next;
					}

					if (!ds_node)
						goto error; // no such bSubst

					bSubst = GetSubstText(hContact, ds_node->ds, swzRep, VALUE_LEN);
				}

				if (bSubst) {
					size_t rep_len = mir_wstrlen(swzRep);
					wcsncpy(&swzDest[di], swzRep, min(rep_len, iDestLen - di));
					di += rep_len - 1; // -1 because we inc at bottom of loop
				}
				else if (bAltSubst) {
					size_t alt_len = mir_wstrlen(swzAlt);
					wcsncpy(&swzDest[di], swzAlt, min(alt_len, iDestLen - di));
					di += alt_len - 1; // -1 because we inc at bottom of loop
				}
				else goto empty; // empty value
			}
			else // no end % - error
				goto error;
		}
		else swzDest[di] = swzVarSrc[si];

		si++;
		di++;
	}

	mir_free(swzVarSrc);
	swzDest[di] = 0;

	if (parseTipperVarsFirst) {
		swzVarSrc = variables_parsedup(swzDest, nullptr, hContact);
		wcsncpy(swzDest, swzVarSrc, iDestLen);
		mir_free(swzVarSrc);
	}


	// check for a 'blank' string - just spaces etc
	for (si = 0; si <= di; si++) {
		if (swzDest[si] != 0 && swzDest[si] != ' ' && swzDest[si] != '\t' && swzDest[si] != '\r' && swzDest[si] != '\n')
			return true;
	}

	return false;

empty:
	mir_free(swzVarSrc);
	return false;

error:
	swzDest[0] = '*';
	swzDest[1] = 0;
	mir_free(swzVarSrc);
	return true;
}

bool GetLabelText(MCONTACT hContact, const DISPLAYITEM &di, wchar_t *buff, size_t bufflen)
{
	return ApplySubst(hContact, di.swzLabel, false, buff, bufflen);
}

bool GetValueText(MCONTACT hContact, const DISPLAYITEM &di, wchar_t *buff, size_t bufflen)
{
	return ApplySubst(hContact, di.swzValue, di.bParseTipperVarsFirst, buff, bufflen);
}

void TruncateString(wchar_t *ptszText)
{
	if (ptszText && opt.iLimitCharCount > 3) {
		if ((int)mir_wstrlen(ptszText) > opt.iLimitCharCount) {
			ptszText[opt.iLimitCharCount - 3] = 0;
			mir_wstrcat(ptszText, L"...");
		}
	}
}

wchar_t* GetProtoStatusMessage(char *szProto, uint16_t wStatus)
{
	if (!szProto || wStatus == ID_STATUS_OFFLINE)
		return nullptr;

	// check if protocol supports status message for status
	int flags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0);
	if (!(flags & Proto_Status2Flag(wStatus)))
		return nullptr;

	wchar_t *ptszText = (wchar_t *)CallProtoService(szProto, PS_GETMYAWAYMSG, 0, SGMA_UNICODE);
	if ((INT_PTR)ptszText == CALLSERVICE_NOTFOUND)
		ptszText = (wchar_t *)CallService(MS_AWAYMSG_GETSTATUSMSGW, wStatus, (LPARAM)szProto);

	else if (ptszText == nullptr) {
		// try to use service without SGMA_TCHAR
		char *tmpMsg = (char *)CallProtoService(szProto, PS_GETMYAWAYMSG, 0, 0);
		if (tmpMsg && (INT_PTR)tmpMsg != CALLSERVICE_NOTFOUND) {
			ptszText = mir_a2u(tmpMsg);
			mir_free(tmpMsg);
		}
	}


	if (ptszText && !ptszText[0]) {
		mir_free(ptszText);
		ptszText = nullptr;
	}

	if (ptszText && opt.bLimitMsg)
		TruncateString(ptszText);

	return ptszText;
}

wchar_t* GetProtoExtraStatusTitle(char *szProto)
{
	if (!szProto)
		return nullptr;

	wchar_t *ptszText = db_get_wsa(0, szProto, "XStatusName");
	if (!ptszText) {
		wchar_t buff[256];
		if (EmptyXStatusToDefaultName(0, szProto, nullptr, buff, 256))
			ptszText = mir_wstrdup(buff);
	}

	if (opt.bLimitMsg)
		TruncateString(ptszText);

	return ptszText;
}

wchar_t* GetProtoExtraStatusMessage(char *szProto)
{
	if (!szProto)
		return nullptr;

	wchar_t *ptszText = db_get_wsa(0, szProto, "XStatusMsg");
	if (ptszText == nullptr)
		return nullptr;

	if (ServiceExists(MS_VARS_FORMATSTRING)) {
		MCONTACT hContact = db_find_first();
		char *proto = Proto_GetBaseAccountName(hContact);
		while (!proto) {
			hContact = db_find_next(hContact);
			if (hContact)
				proto = Proto_GetBaseAccountName(hContact);
			else {
				hContact = NULL;
				break;
			}
		}

		wchar_t *tszParsed = variables_parse(ptszText, nullptr, hContact);
		if (tszParsed) {
			mir_free(ptszText);
			ptszText = tszParsed;
		}
	}

	if (opt.bLimitMsg)
		TruncateString(ptszText);

	return ptszText;
}

wchar_t* GetListeningTo(char *szProto)
{
	if (!szProto)
		return nullptr;

	wchar_t *ptszText = db_get_wsa(0, szProto, "ListeningTo");
	if (opt.bLimitMsg)
		TruncateString(ptszText);

	return ptszText;
}

wchar_t* GetJabberAdvStatusText(char *szProto, const char *szSlot, const char *szValue)
{
	if (!szProto)
		return nullptr;

	char szSetting[128];
	mir_snprintf(szSetting, "%s/%s/%s", szProto, szSlot, szValue);
	wchar_t *ptszText = db_get_wsa(0, "AdvStatus", szSetting);
	if (opt.bLimitMsg)
		TruncateString(ptszText);

	return ptszText;
}

HICON GetJabberActivityIcon(MCONTACT hContact, char *szProto)
{
	if (!szProto)
		return nullptr;

	char szSetting[128];
	mir_snprintf(szSetting, "%s/%s/%s", szProto, "activity", "icon");
	ptrA szIcon(db_get_sa(hContact, "AdvStatus", szSetting));
	return (szIcon != NULL) ? IcoLib_GetIcon(szIcon) : nullptr;
}
