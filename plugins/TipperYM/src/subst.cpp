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

#include "common.h"

bool DBGetContactSettingAsString(MCONTACT hContact, const char *szModuleName, const char *szSettingName, TCHAR *buff, int bufflen)
{
	DBVARIANT dbv;
	buff[0] = 0;

	if (!szModuleName || !szSettingName)
		return false;

	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		switch (dbv.type) {
		case DBVT_BYTE:
			_itot(dbv.bVal, buff, 10);
			break;
		case DBVT_WORD:
			_ltot(dbv.wVal, buff, 10);
			break;
		case DBVT_DWORD:
			_ltot(dbv.dVal, buff, 10);
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

	char *szProto = GetContactProto(hContact);	
	if (szProto) {
		if (db_get_b(hContact, szProto, "ChatRoom", 0) != 0)
			return di.type == DIT_CHATS;
		else
			return di.type == DIT_CONTACTS;
	}

	return false;
}

void StripBBCodesInPlace(TCHAR *swzText)
{
	if (!db_get_b(0, MODULE, "StripBBCodes", 1))
		return;

	if (swzText == 0)
		return;

	size_t iRead = 0, iWrite = 0;
	size_t iLen = _tcslen(swzText);

	while(iRead <= iLen) { // copy terminating null too
		while (iRead <= iLen && swzText[iRead] != '[') {
			if (swzText[iRead] != swzText[iWrite]) swzText[iWrite] = swzText[iRead];
			iRead++; iWrite++;
		}

		if (iRead > iLen)
			break;

		if (iLen - iRead >= 3 && (_tcsnicmp(swzText + iRead, _T("[b]"), 3) == 0 || _tcsnicmp(swzText + iRead, _T("[i]"), 3) == 0))
			iRead += 3;
		else if (iLen - iRead >= 4 && (_tcsnicmp(swzText + iRead, _T("[/b]"), 4) == 0 || _tcsnicmp(swzText + iRead, _T("[/i]"), 4) == 0))
			iRead += 4;
		else if (iLen - iRead >= 6 && (_tcsnicmp(swzText + iRead, _T("[color"), 6) == 0)) {
			while (iRead < iLen && swzText[iRead] != ']') iRead++;
			iRead++;// skip the ']'
		}
		else if (iLen - iRead >= 8 && (_tcsnicmp(swzText + iRead, _T("[/color]"), 8) == 0))
			iRead += 8;
		else if (iLen - iRead >= 5 && (_tcsnicmp(swzText + iRead, _T("[size"), 5) == 0)) {
			while (iRead < iLen && swzText[iRead] != ']') iRead++;
			iRead++;// skip the ']'
		}
		else if (iLen - iRead >= 7 && (_tcsnicmp(swzText + iRead, _T("[/size]"), 7) == 0))
			iRead += 7;
		else {
			if (swzText[iRead] != swzText[iWrite]) swzText[iWrite] = swzText[iRead];
			iRead++; iWrite++;
		}
	}
}

DWORD LastMessageTimestamp(MCONTACT hContact, bool received)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDbEvent, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT) == received)
			return dbei.timestamp;
	}

	return 0;
}

void FormatTimestamp(DWORD ts, char *szFormat, TCHAR *buff, int bufflen)
{
	TCHAR swzForm[16];
	DBTIMETOSTRINGT dbt = {0};
	dbt.cbDest = bufflen;
	dbt.szDest = buff;
	a2t(szFormat, swzForm, 16);
	dbt.szFormat = swzForm;
	CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ts, (LPARAM)&dbt);
}

bool Uid(MCONTACT hContact, char *szProto, TCHAR *buff, int bufflen)
{
	char *tmpProto = (hContact) ? tmpProto = GetContactProto(hContact) : szProto;
	if (tmpProto) {
		char *szUid = (char*)CallProtoService(tmpProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (szUid && (INT_PTR)szUid != CALLSERVICE_NOTFOUND)
			return DBGetContactSettingAsString(hContact, tmpProto, szUid, buff, bufflen);
	}

	return false;
}

bool UidName(char *szProto, TCHAR *buff, int bufflen)
{
	if (szProto) {
		char *szUidName = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
		if (szUidName && (INT_PTR)szUidName != CALLSERVICE_NOTFOUND) {
			a2t(szUidName, buff, bufflen);
			return true;
		}
	}
	return false;
}

TCHAR *GetLastMessageText(MCONTACT hContact, bool received)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
		DBEVENTINFO dbei = {	sizeof(dbei) };
		db_event_get(hDbEvent, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT) == received) {
			dbei.pBlob = (BYTE *)alloca(dbei.cbBlob);
			db_event_get(hDbEvent, &dbei);
			if (dbei.cbBlob == 0 || dbei.pBlob == 0)
				return 0;

			TCHAR *buff = DbGetEventTextT( &dbei, CP_ACP );
			TCHAR *swzMsg = mir_tstrdup(buff);
			mir_free(buff);

			StripBBCodesInPlace(swzMsg);
			return swzMsg;
		}
	}

	return 0;
}

bool CanRetrieveStatusMsg(MCONTACT hContact, char *szProto)
{
	if (opt.bGetNewStatusMsg) {
		int iFlags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0);
		WORD wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		if ((CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) && (iFlags & Proto_Status2Flag(wStatus))) {
			iFlags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & (PF1_VISLIST | PF1_INVISLIST);
			if (opt.bDisableIfInvisible && iFlags) {
				int iVisMode = db_get_w(hContact, szProto, "ApparentMode", 0);
				int wProtoStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
				if ((iVisMode == ID_STATUS_OFFLINE) || (wProtoStatus == ID_STATUS_INVISIBLE && iVisMode != ID_STATUS_ONLINE))
					return false;
				return true;
			}
			return true;
		}
	}

	return false;
}

TCHAR* GetStatusMessageText(MCONTACT hContact)
{
	TCHAR *swzMsg = NULL;
	DBVARIANT dbv;

	char *szProto = GetContactProto(hContact);
	if (szProto) {
		if (!strcmp(szProto, META_PROTO))
			hContact = db_mc_getMostOnline(hContact);
		else {
			WORD wStatus = (int)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			if (wStatus == ID_STATUS_OFFLINE)
				return NULL;

			if (!db_get_ts(hContact, MODULE, "TempStatusMsg", &dbv)) {
				if (_tcslen(dbv.ptszVal) != 0)
					swzMsg = mir_tstrdup(dbv.ptszVal);
				db_free(&dbv);
			}
		}

		if (!swzMsg) {
			if (CanRetrieveStatusMsg(hContact, szProto))
				if (CallContactService(hContact, PSS_GETAWAYMSG, 0, 0))
					return NULL;

			if (!db_get_ts(hContact, "CList", "StatusMsg", &dbv)) {
				if (_tcslen(dbv.ptszVal) != 0)
					swzMsg = mir_tstrdup(dbv.ptszVal);
				db_free(&dbv);
			}
		}
	}

	if (swzMsg)
		StripBBCodesInPlace(swzMsg);

	return swzMsg;
}

bool GetSysSubstText(MCONTACT hContact, TCHAR *swzRawSpec, TCHAR *buff, int bufflen)
{
	bool recv = false;

	if (!_tcscmp(swzRawSpec, _T("uid")))
		return Uid(hContact, 0, buff, bufflen);

	if (!_tcscmp(swzRawSpec, _T("proto"))) {
		char *szProto = GetContactProto(hContact);
		if (szProto) {
			a2t(szProto, buff, bufflen);
			return true;
		}
	}
	else if (!_tcscmp(swzRawSpec, _T("account"))) {
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, hContact, 0);
		if ((INT_PTR)szProto == CALLSERVICE_NOTFOUND) {
			return GetSysSubstText(hContact, _T("proto"), buff, bufflen);
		}
		else if (szProto) {
			PROTOACCOUNT *pa = ProtoGetAccount(szProto);
			if (pa && pa->tszAccountName) {
				_tcsncpy(buff, pa->tszAccountName, bufflen);
				return true;
			}
			else
				return GetSysSubstText(hContact, _T("proto"), buff, bufflen);
		}
	}
	else if (!_tcscmp(swzRawSpec, _T("time"))) {
		if (tmi.printDateTime && !tmi.printDateTimeByContact(hContact, _T("t"), buff, bufflen, TZF_KNOWNONLY))
			return true;
	}
	else if (!_tcscmp(swzRawSpec, _T("uidname"))) {
		char *szProto = GetContactProto(hContact);
		return UidName(szProto, buff, bufflen);
	}
	else if (!_tcscmp(swzRawSpec, _T("status_msg"))) {
		TCHAR *swzMsg = GetStatusMessageText(hContact);
		if (swzMsg) {
			_tcsncpy(buff, swzMsg, bufflen);
			mir_free(swzMsg);
			return true;
		}
	}
	else if ((recv = !_tcscmp(swzRawSpec, _T("last_msg"))) || !_tcscmp(swzRawSpec, _T("last_msg_out"))) {
		TCHAR *swzMsg = GetLastMessageText(hContact, recv);
		if (swzMsg) {
			_tcsncpy(buff, swzMsg, bufflen);
			mir_free(swzMsg);
			return true;
		}
	}
	else if (!_tcscmp(swzRawSpec, _T("meta_subname"))) {
		// get contact list name of active subcontact
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		if (!hSubContact)
			return false;
		
		TCHAR *swzNick = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hSubContact, GCDNF_TCHAR);
		if (swzNick) _tcsncpy(buff, swzNick, bufflen);
		return true;
	}
	else if (!_tcscmp(swzRawSpec, _T("meta_subuid"))) {
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND)
			return false;
		return Uid(hSubContact, 0, buff, bufflen);
	}
	else if (!_tcscmp(swzRawSpec, _T("meta_subproto"))) {
		// get protocol of active subcontact
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND)
			return false;
		return GetSysSubstText(hSubContact, _T("account"), buff, bufflen);
	}
	else if ((recv = !_tcscmp(swzRawSpec, _T("last_msg_time"))) || !_tcscmp(swzRawSpec, _T("last_msg_out_time"))) {
		DWORD ts = LastMessageTimestamp(hContact, recv);
		if (ts == 0) return false;
		FormatTimestamp(ts, "t", buff, bufflen);
		return true;
	}
	else if ((recv = !_tcscmp(swzRawSpec, _T("last_msg_date"))) || !_tcscmp(swzRawSpec, _T("last_msg_out_date"))) {
		DWORD ts = LastMessageTimestamp(hContact, recv);
		if (ts == 0) return false;
		FormatTimestamp(ts, "d", buff, bufflen);
		return true;
	}
	else if ((recv = !_tcscmp(swzRawSpec, _T("last_msg_reltime"))) || !_tcscmp(swzRawSpec, _T("last_msg_out_reltime"))) {
		DWORD ts = LastMessageTimestamp(hContact, recv);
		if (ts == 0) return false;
		DWORD t = (DWORD)time(0);
		DWORD diff = (t - ts);
		int d = (diff / 60 / 60 / 24);
		int h = (diff - d * 60 * 60 * 24) / 60 / 60;
		int m = (diff - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		if (d > 0) mir_sntprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
		else if (h > 0) mir_sntprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
		else mir_sntprintf(buff, bufflen, TranslateT("%dm"), m);
		return true;
	}
	else if (!_tcscmp(swzRawSpec, _T("msg_count_all")) || !_tcscmp(swzRawSpec, _T("msg_count_out")) || !_tcscmp(swzRawSpec, _T("msg_count_in"))) {
		DWORD dwCountOut, dwCountIn;
		DWORD dwMetaCountOut = 0, dwMetaCountIn = 0;
		DWORD dwLastTs, dwNewTs, dwRecountTs;
		DWORD dwTime, dwDiff;
		int iNumber = 1;
		MCONTACT hTmpContact = hContact;

		char *szProto = GetContactProto(hContact);
		if (szProto && !strcmp(szProto, META_PROTO)) {
			iNumber = db_mc_getSubCount(hContact);
			hTmpContact = db_mc_getSub(hContact, 0);
		}

		for (int i = 0; i < iNumber; i++) {
			if (i > 0)
				hTmpContact = db_mc_getSub(hContact, i);
			dwRecountTs = db_get_dw(hTmpContact, MODULE, "LastCountTS", 0);
			dwTime = (DWORD)time(0);
			dwDiff = (dwTime - dwRecountTs);
			if (dwDiff > (60 * 60 * 24 * 3)) {
				db_set_dw(hTmpContact, MODULE, "LastCountTS", dwTime);
				dwCountOut = dwCountIn = dwLastTs = 0;
			}
			else {
				dwCountOut = db_get_dw(hTmpContact, MODULE, "MsgCountOut", 0);
				dwCountIn = db_get_dw(hTmpContact, MODULE, "MsgCountIn", 0);
				dwLastTs = db_get_dw(hTmpContact, MODULE, "LastMsgTS", 0);
			}

			dwNewTs = dwLastTs;

			MEVENT dbe = db_event_last(hTmpContact);
			while (dbe != NULL) {
				DBEVENTINFO dbei = { sizeof(dbei) };
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
				db_set_dw(hTmpContact, MODULE, "MsgCountOut", dwCountOut);
				db_set_dw(hTmpContact, MODULE, "MsgCountIn", dwCountIn);
				db_set_dw(hTmpContact, MODULE, "LastMsgTS", dwNewTs);
			}

			dwMetaCountOut += dwCountOut;
			dwMetaCountIn += dwCountIn;
		}

		if (!_tcscmp(swzRawSpec, _T("msg_count_out")))
			mir_sntprintf(buff, bufflen, _T("%d"), dwMetaCountOut);
		else if (!_tcscmp(swzRawSpec, _T("msg_count_in")))
			mir_sntprintf(buff, bufflen, _T("%d"), dwMetaCountIn);
		else
			mir_sntprintf(buff, bufflen, _T("%d"), dwMetaCountOut + dwMetaCountIn);
		return true;
	}

	return false;
}

bool GetSubstText(MCONTACT hContact, const DISPLAYSUBST &ds, TCHAR *buff, int bufflen)
{
	TranslateFunc *transFunc = 0;
	for (int i = 0; i < iTransFuncsCount; i++)
		if (translations[i].id == (DWORD)ds.iTranslateFuncId) {
			transFunc = translations[i].transFunc;
			break;
		}

	if (!transFunc)
		return false;

	switch (ds.type) {
	case DVT_DB:
		return transFunc(hContact, ds.szModuleName, ds.szSettingName, buff, bufflen) != 0;
	case DVT_PROTODB:
		char *szProto = GetContactProto(hContact);
		if (szProto) {
			if (transFunc(hContact, szProto, ds.szSettingName, buff, bufflen) != 0)
				return true;
			return transFunc(hContact, "UserInfo", ds.szSettingName, buff, bufflen) != 0;
		}
		break;
	}
	return false;
}

bool GetRawSubstText(MCONTACT hContact, char *szRawSpec, TCHAR *buff, int bufflen)
{
	size_t lenght = strlen(szRawSpec);
	for (size_t i = 0; i < lenght; i++) {
		if (szRawSpec[i] == '/') {
			szRawSpec[i] = 0;
			if (strlen(szRawSpec) == 0) {
				char *szProto = GetContactProto(hContact);
				if (szProto) {
					if (translations[0].transFunc(hContact, szProto, &szRawSpec[i + 1], buff, bufflen) != 0)
						return true;
					return translations[0].transFunc(hContact, "UserInfo", &szRawSpec[i + 1], buff, bufflen) != 0;
				}
				return false;
			}
			return translations[0].transFunc(hContact, szRawSpec, &szRawSpec[i + 1], buff, bufflen) != 0;
		}
	}
	return false;
}

bool ApplySubst(MCONTACT hContact, const TCHAR *swzSource, bool parseTipperVarsFirst, TCHAR *swzDest, int iDestLen)
{
	// hack - allow empty strings before passing to variables (note - zero length strings return false after this)
	if (swzDest && swzSource && _tcslen(swzSource) == 0) {
		swzDest[0] = 0;
		return true;
	}

	// pass to variables plugin if available
	TCHAR *swzVarSrc;
	if (parseTipperVarsFirst)
		swzVarSrc = mir_tstrdup(swzSource);
	else
		swzVarSrc = variables_parsedup((TCHAR *)swzSource, 0, hContact);

	size_t iSourceLen = _tcslen(swzVarSrc);
	size_t si = 0, di = 0, v = 0;

	TCHAR swzVName[LABEL_LEN];
	TCHAR swzRep[VALUE_LEN], swzAlt[VALUE_LEN];
	while (si < iSourceLen && di < (size_t)iDestLen - 1) {
		if (swzVarSrc[si] == _T('%')) {
			si++;
			v = 0;
			while (si < iSourceLen && v < LABEL_LEN) {
				if (swzVarSrc[si] == _T('%'))
					break;

				swzVName[v] = swzVarSrc[si];
				v++; si++;
			}

			if (v == 0)  // bSubst len is 0 - just a % symbol
				swzDest[di] = _T('%');
			else if (si < iSourceLen) // we found end %
			{
				swzVName[v] = 0;

				bool bAltSubst = false;
				bool bSubst = false;

				// apply only to specific protocols
				TCHAR *p = _tcsrchr(swzVName, _T('^')); // use last '^', so if you want a ^ in swzAlt text, you can just put a '^' on the end
				if (p) {
					*p = 0;
					p++;
					if (*p) {
						char *cp = GetContactProto(hContact);
						if (cp != NULL) {
							PROTOACCOUNT *acc = ProtoGetAccount(cp);
							if (acc != NULL) {
								cp = acc->szProtoName;
							}
						}

						if (cp == NULL)
							goto empty;

						bool negate = false;
						if (*p == _T('!')) {
							p++;
							if (*p == 0) goto error;
							negate = true;
						}

						char sproto[256];
						bool spec = false;
						int len;

						TCHAR *last = _tcsrchr(p, _T(','));
						if (!last) last = p;

						while (p <= last + 1) {
							len = (int)_tcscspn(p, _T(","));
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
				p = _tcschr(swzVName, _T('|')); // use first '|' - so you can use the '|' symbol in swzAlt text
				if (p) {
					*p = 0; // clip swzAlt from swzVName
					p++;
					if (_tcslen(p) > 4 && _tcsncmp(p, _T("raw:"), 4) == 0) { // raw db substitution
						char raw_spec[LABEL_LEN];
						p += 4;
						t2a(p, raw_spec, LABEL_LEN);
						GetRawSubstText(hContact, raw_spec, swzAlt, VALUE_LEN);
					}
					else if (_tcslen(p) > 4 && _tcsncmp(p, _T("sys:"), 4) == 0) { // 'system' substitution
						p += 4;
						GetSysSubstText(hContact, p, swzAlt, VALUE_LEN);
					}
					else {
						// see if we can find the bSubst
						DSListNode *ds_node = opt.dsList;
						while (ds_node) {
							if (_tcscmp(ds_node->ds.swzName, p) == 0)
								break;

							ds_node = ds_node->next;
						}

						if (ds_node)
							GetSubstText(hContact, ds_node->ds, swzAlt, VALUE_LEN);
						else {
							_tcsncpy(swzAlt, p, VALUE_LEN);
							bAltSubst = true;
						}
					}
					swzAlt[VALUE_LEN - 1] = 0;
					if (_tcslen(swzAlt) != 0)
						bAltSubst = true;
				}

				// get bSubst text
				if (v > 4 && _tcsncmp(swzVName, _T("raw:"), 4) == 0) // raw db substitution
				{
					char raw_spec[LABEL_LEN];
					t2a(&swzVName[4], raw_spec, LABEL_LEN);
					bSubst = GetRawSubstText(hContact, raw_spec, swzRep, VALUE_LEN);
				}
				else if (v > 4 && _tcsncmp(swzVName, _T("sys:"), 4) == 0) // 'system' substitution
				{
					bSubst = GetSysSubstText(hContact, &swzVName[4], swzRep, VALUE_LEN);
				}
				else {
					// see if we can find the bSubst
					DSListNode *ds_node = opt.dsList;
					while (ds_node) {
						if (_tcscmp(ds_node->ds.swzName, swzVName) == 0)
							break;

						ds_node = ds_node->next;
					}

					if (!ds_node)
						goto error; // no such bSubst

					bSubst = GetSubstText(hContact, ds_node->ds, swzRep, VALUE_LEN);
				}

				if (bSubst) {
					size_t rep_len = _tcslen(swzRep);
					_tcsncpy(&swzDest[di], swzRep, min(rep_len, iDestLen - di));
					di += rep_len - 1; // -1 because we inc at bottom of loop
				}
				else if (bAltSubst) {
					size_t alt_len = _tcslen(swzAlt);
					_tcsncpy(&swzDest[di], swzAlt, min(alt_len, iDestLen - di));
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
		swzVarSrc = variables_parsedup((TCHAR *)swzDest, 0, hContact);
		_tcscpy(swzDest, swzVarSrc);
		mir_free(swzVarSrc);
	}


	// check for a 'blank' string - just spaces etc
	for (si = 0; si <= di; si++) {
		if (swzDest[si] != 0 && swzDest[si] != _T(' ') && swzDest[si] != _T('\t') && swzDest[si] != _T('\r') && swzDest[si] != _T('\n'))
			return true;
	}

	return false;

empty:
	mir_free(swzVarSrc);
	return false;

error:
	swzDest[0] = _T('*');
	swzDest[1] = 0;
	mir_free(swzVarSrc);
	return true;
}

bool GetLabelText(MCONTACT hContact, const DISPLAYITEM &di, TCHAR *buff, int bufflen)
{
	return ApplySubst(hContact, di.swzLabel, false, buff, bufflen);
}

bool GetValueText(MCONTACT hContact, const DISPLAYITEM &di, TCHAR *buff, int bufflen)
{
	return ApplySubst(hContact, di.swzValue, di.bParseTipperVarsFirst, buff, bufflen);
}

void TruncateString(TCHAR *swzText)
{
	if (swzText && opt.iLimitCharCount > 3) {
		if ((int)_tcslen(swzText) > opt.iLimitCharCount) {
			swzText[opt.iLimitCharCount - 3] = 0;
			_tcscat(swzText, _T("..."));
		}
	}
}

TCHAR *GetProtoStatusMessage(char *szProto, WORD wStatus)
{
	if (!szProto || wStatus == ID_STATUS_OFFLINE)
		return NULL;

	// check if protocol supports status message for status
	int flags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0);
	if (!(flags & Proto_Status2Flag(wStatus)))
		return NULL;

	TCHAR *swzText = (TCHAR *)CallProtoService(szProto, PS_GETMYAWAYMSG, 0, SGMA_TCHAR);
	if ((INT_PTR)swzText == CALLSERVICE_NOTFOUND)
		swzText = (TCHAR *)CallService(MS_AWAYMSG_GETSTATUSMSGT, wStatus, (LPARAM)szProto);

	else if (swzText == NULL) {
		// try to use service without SGMA_TCHAR
		char *tmpMsg = (char *)CallProtoService(szProto, PS_GETMYAWAYMSG, 0, 0);
		if (tmpMsg && (INT_PTR)tmpMsg != CALLSERVICE_NOTFOUND) {
			swzText = mir_a2t(tmpMsg);
			mir_free(tmpMsg);
		}
	}


	if (swzText && !swzText[0]) {
		mir_free(swzText);
		swzText = NULL;
	}

	if (swzText && opt.bLimitMsg)
		TruncateString(swzText);

	return swzText;
}

TCHAR *GetProtoExtraStatusTitle(char *szProto)
{
	DBVARIANT dbv;
	TCHAR *swzText = NULL;

	if (!szProto)
		return NULL;

	if (!db_get_ts(0, szProto, "XStatusName", &dbv)) {
		if (_tcslen(dbv.ptszVal) != 0)
			swzText = mir_tstrdup(dbv.ptszVal);
		db_free(&dbv);
	}

	if (!swzText) {
		TCHAR buff[256];
		if (EmptyXStatusToDefaultName(0, szProto, 0, buff, 256))
			swzText = mir_tstrdup(buff);
	}

	if (opt.bLimitMsg)
		TruncateString(swzText);

	return swzText;
}

TCHAR *GetProtoExtraStatusMessage(char *szProto)
{
	if (!szProto)
		return NULL;

	TCHAR *swzText = NULL;
	DBVARIANT dbv;
	if (!db_get_ts(0, szProto, "XStatusMsg", &dbv)) {
		if (_tcslen(dbv.ptszVal) != 0)
			swzText = mir_tstrdup(dbv.ptszVal);
		db_free(&dbv);

		if (ServiceExists(MS_VARS_FORMATSTRING)) {
			MCONTACT hContact = db_find_first();
			char *proto = GetContactProto(hContact);
			while (!proto) {
				hContact = db_find_next(hContact);
				if (hContact)
					proto = GetContactProto(hContact);
				else {
					hContact = NULL;
					break;
				}
			}

			TCHAR *tszParsed = variables_parse(swzText, NULL, hContact);
			if (tszParsed)
				replaceStrT(swzText, tszParsed);
		}
	}

	if (opt.bLimitMsg)
		TruncateString(swzText);

	return swzText;
}

TCHAR *GetListeningTo(char *szProto)
{
	DBVARIANT dbv;
	TCHAR *swzText = NULL;

	if (!szProto)
		return NULL;

	if (!db_get_ts(0, szProto, "ListeningTo", &dbv)) {
		if (_tcslen(dbv.ptszVal) != 0)
			swzText = mir_tstrdup(dbv.ptszVal);
		db_free(&dbv);
	}

	if (opt.bLimitMsg)
		TruncateString(swzText);

	return swzText;
}

TCHAR *GetJabberAdvStatusText(char *szProto, const char *szSlot, const char *szValue)
{
	DBVARIANT dbv;
	TCHAR *swzText = NULL;

	if (!szProto)
		return NULL;

	char szSetting[128];
	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/%s", szProto, szSlot, szValue);
	if (!db_get_ts(0, "AdvStatus", szSetting, &dbv)) {
		if (_tcslen(dbv.ptszVal) != 0)
			swzText = mir_tstrdup(dbv.ptszVal);
		db_free(&dbv);
	}

	if (opt.bLimitMsg)
		TruncateString(swzText);

	return swzText;
}

HICON GetJabberActivityIcon(MCONTACT hContact, char *szProto)
{
	DBVARIANT dbv;
	HICON hIcon = NULL;

	if (!szProto)
		return NULL;

	char szSetting[128];
	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/%s", szProto, "activity", "icon");
	if (!db_get_s(hContact, "AdvStatus", szSetting, &dbv)) {
		hIcon = Skin_GetIcon(dbv.pszVal);
		db_free(&dbv);
	}

	return hIcon;
}