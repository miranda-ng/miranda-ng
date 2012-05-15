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
#include "subst.h"
#include "str_utils.h"
#include "popwin.h"


int ProtoServiceExists(const char *szModule, const char *szService) 
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}

bool DBGetContactSettingAsString(HANDLE hContact, const char *szModuleName, const char *szSettingName, TCHAR *buff, int bufflen) 
{
	DBVARIANT dbv;
	buff[0] = 0;

	if (!szModuleName || !szSettingName)
		return false;

	if (!DBGetContactSetting(hContact, szModuleName, szSettingName, &dbv))
	{
		switch(dbv.type) 
		{
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
#ifdef _UNICODE
		case DBVT_WCHAR:
			if (dbv.pwszVal) wcsncpy(buff, dbv.pwszVal, bufflen);
			buff[bufflen - 1] = 0;
			break;
#endif
		}

		DBFreeVariant(&dbv);
	}

	return buff[0] ? true : false;
}

void StripBBCodesInPlace(TCHAR *swzText) 
{
	if (!DBGetContactSettingByte(0, MODULE, "StripBBCodes", 1))
		return;

	if (swzText == 0) 
		return;

	size_t iRead = 0, iWrite = 0;
	size_t iLen = _tcslen(swzText);

	while(iRead <= iLen)  // copy terminating null too
	{
		while(iRead <= iLen && swzText[iRead] != '[') 
		{
			if (swzText[iRead] != swzText[iWrite]) swzText[iWrite] = swzText[iRead];
			iRead++; iWrite++;
		}

		if (iRead > iLen)
			break;

		if (iLen - iRead >= 3 && (_tcsnicmp(swzText + iRead, _T("[b]"), 3) == 0 || _tcsnicmp(swzText + iRead, _T("[i]"), 3) == 0))
			iRead += 3;
		else if (iLen - iRead >= 4 && (_tcsnicmp(swzText + iRead, _T("[/b]"), 4) == 0 || _tcsnicmp(swzText + iRead, _T("[/i]"), 4) == 0))
			iRead += 4;
		else if (iLen - iRead >= 6 && (_tcsnicmp(swzText + iRead, _T("[color"), 6) == 0)) 
		{
			while(iRead < iLen && swzText[iRead] != ']') iRead++; 
			iRead++;// skip the ']'
		} 
		else if (iLen - iRead >= 8 && (_tcsnicmp(swzText + iRead, _T("[/color]"), 8) == 0))
			iRead += 8;
		else if (iLen - iRead >= 5 && (_tcsnicmp(swzText + iRead, _T("[size"), 5) == 0))
		{
			while(iRead < iLen && swzText[iRead] != ']') iRead++; 
			iRead++;// skip the ']'
		} 
		else if (iLen - iRead >= 7 && (_tcsnicmp(swzText + iRead, _T("[/size]"), 7) == 0))
			iRead += 7;
		else 
		{
			if (swzText[iRead] != swzText[iWrite]) swzText[iWrite] = swzText[iRead];
			iRead++; iWrite++;
		}
	}
}

DWORD LastMessageTimestamp(HANDLE hContact) 
{
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	HANDLE hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0);
	while (hDbEvent) 
	{
		dbei.cbBlob = 0;
		CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT))
			break;
		
		hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hDbEvent, 0);
	}

	if (hDbEvent) 
		return dbei.timestamp;

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

bool Uid(HANDLE hContact, char *szProto, TCHAR *buff, int bufflen) 
{
	char *tmpProto = NULL;

	if (hContact) tmpProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact, 0);
	else tmpProto = szProto;

	if (tmpProto) 
	{
		char *szUid = (char *)CallProtoService(tmpProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (szUid && (INT_PTR)szUid != CALLSERVICE_NOTFOUND)
			return DBGetContactSettingAsString(hContact, tmpProto, szUid, buff, bufflen);
	}

	return false;
}

bool UidName(char *szProto, TCHAR *buff, int bufflen) 
{
	if (szProto)
	{
		char *szUidName = (char *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
		if (szUidName && (INT_PTR)szUidName != CALLSERVICE_NOTFOUND)
		{
			a2t(szUidName, buff, bufflen);
			return true;
		}       
	}
	return false;
}

TCHAR *GetLastMessageText(HANDLE hContact) 
{
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);

	HANDLE hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0);
	while (hDbEvent) 
	{
		dbei.cbBlob = 0;
		CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT))
			break;
		
		hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hDbEvent, 0);
	}

	if (hDbEvent) 
	{
		dbei.pBlob = (BYTE *)alloca(dbei.cbBlob);
		CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei);

		if (dbei.cbBlob == 0 || dbei.pBlob == 0) 
			return 0;

		TCHAR *buff = DbGetEventTextT( &dbei, CP_ACP );
		TCHAR *swzMsg = mir_tstrdup(buff);
		mir_free(buff);

		StripBBCodesInPlace(swzMsg);
		return swzMsg;
	}

	return 0;
}

bool CanRetrieveStatusMsg(HANDLE hContact, char *szProto)
{
	if (opt.bGetNewStatusMsg) 
	{
		int iFlags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0);
		WORD wStatus = DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		if ((CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) && (iFlags & Proto_Status2Flag(wStatus)))
		{
			iFlags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & (PF1_VISLIST | PF1_INVISLIST);
			if (opt.bDisableIfInvisible && iFlags) 
			{
				int iVisMode = DBGetContactSettingWord(hContact, szProto, "ApparentMode", 0); 
				int wProtoStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
				if ((iVisMode == ID_STATUS_OFFLINE) || (wProtoStatus == ID_STATUS_INVISIBLE && iVisMode != ID_STATUS_ONLINE))
					return false;
				else
					return true;
			} 
			else
			{
				return true;
			}
		} 
		else
		{
			return false;
		}
	}

	return false;
}

TCHAR *GetStatusMessageText(HANDLE hContact) 
{
	TCHAR *swzMsg = 0;
	DBVARIANT dbv;

	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (szProto) 
	{
		if (!strcmp(szProto, szMetaModuleName))
		{
			hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		}
		else 
		{	
			WORD wStatus = (int)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			if (wStatus == ID_STATUS_OFFLINE) 
				return NULL;

			if (!DBGetContactSettingTString(hContact, MODULE, "TempStatusMsg", &dbv)) 
			{
				if (_tcslen(dbv.ptszVal) != 0)
					swzMsg = mir_tstrdup(dbv.ptszVal);
				DBFreeVariant(&dbv);
			}
		}

		if (!swzMsg) 
		{
			if (CanRetrieveStatusMsg(hContact, szProto))
			{
				if (CallContactService(hContact, PSS_GETAWAYMSG, 0, 0))
					return NULL;
			}

			if (!DBGetContactSettingTString(hContact, "CList", "StatusMsg", &dbv)) 
			{
				if (dbv.ptszVal && _tcslen(dbv.ptszVal) != 0)
					swzMsg = mir_tstrdup(dbv.ptszVal);
				DBFreeVariant(&dbv);
			}
		}
	}

	if (swzMsg) 
		StripBBCodesInPlace(swzMsg);

	return swzMsg;
}

bool GetSysSubstText(HANDLE hContact, TCHAR *swzRawSpec, TCHAR *buff, int bufflen) 
{
	if (!_tcscmp(swzRawSpec, _T("uid")))
	{
		return Uid(hContact, 0, buff, bufflen);
	} 
	else if (!_tcscmp(swzRawSpec, _T("proto"))) 
	{
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (szProto)
		{
			a2t(szProto, buff, bufflen);
			return true;
		}
	} 
	else if (!_tcscmp(swzRawSpec, _T("account"))) 
	{
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, (WPARAM)hContact, 0);
		if ((INT_PTR)szProto == CALLSERVICE_NOTFOUND) 
		{
			return GetSysSubstText(hContact, _T("proto"), buff, bufflen);
		}
		else if (szProto) 
		{
			PROTOACCOUNT *pa = ProtoGetAccount(szProto);
			if (pa && pa->tszAccountName)
			{
				_tcsncpy(buff, pa->tszAccountName, bufflen);
				return true;
			}
			else
				return GetSysSubstText(hContact, _T("proto"), buff, bufflen);
		}
	} 
	else if (!_tcscmp(swzRawSpec, _T("time"))) 
	{
		if (tmi.printDateTime && !tmi.printDateTimeByContact(hContact, _T("t"), buff, bufflen, TZF_KNOWNONLY))
			return true;
	}
	else if (!_tcscmp(swzRawSpec, _T("uidname")))
	{
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0); 
		return UidName(szProto, buff, bufflen);
	}
	else if (!_tcscmp(swzRawSpec, _T("status_msg"))) 
	{
		TCHAR *swzMsg = GetStatusMessageText(hContact);
		if (swzMsg)
		{
			_tcsncpy(buff, swzMsg, bufflen);
			mir_free(swzMsg);
			return true;
		}
	} 
	else if (!_tcscmp(swzRawSpec, _T("last_msg")))
	{
		TCHAR *swzMsg = GetLastMessageText(hContact);
		if (swzMsg)
		{
			_tcsncpy(buff, swzMsg, bufflen);
			mir_free(swzMsg);
			return true;
		}
	} 
	else if (!_tcscmp(swzRawSpec, _T("meta_subname"))) 
	{
		// get contact list name of active subcontact
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND) return false;
		TCHAR *swzNick = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hSubContact, GCDNF_TCHAR);
		if (swzNick) _tcsncpy(buff, swzNick, bufflen);
		return true;
	} 
	else if (!_tcscmp(swzRawSpec, _T("meta_subuid")))
	{
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND) return false;
		return Uid(hSubContact, 0, buff, bufflen);
	} 
	else if (!_tcscmp(swzRawSpec, _T("meta_subproto")))
	{
		// get protocol of active subcontact
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND) 
			return false;
		return GetSysSubstText(hSubContact, _T("account"), buff, bufflen);
	} 
	else if (!_tcscmp(swzRawSpec, _T("last_msg_time"))) 
	{
		DWORD ts = LastMessageTimestamp(hContact);
		if (ts == 0) return false;
		FormatTimestamp(ts, "t", buff, bufflen);
		return true;
	} 
	else if (!_tcscmp(swzRawSpec, _T("last_msg_date"))) 
	{
		DWORD ts = LastMessageTimestamp(hContact);
		if (ts == 0) return false;
		FormatTimestamp(ts, "d", buff, bufflen);
		return true;
	} 
	else if (!_tcscmp(swzRawSpec, _T("last_msg_reltime")))
	{
		DWORD ts = LastMessageTimestamp(hContact);
		if (ts == 0) return false;
		DWORD t = (DWORD)time(0);
		DWORD diff = (t - ts);
		int d = (diff / 60 / 60 / 24);
		int h = (diff - d * 60 * 60 * 24) / 60 / 60;
		int m = (diff  - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		if (d > 0) mir_sntprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
		else if (h > 0) mir_sntprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
		else mir_sntprintf(buff, bufflen, TranslateT("%dm"), m);
		return true;
	} 
	else if (!_tcscmp(swzRawSpec, _T("msg_count_all")) || !_tcscmp(swzRawSpec, _T("msg_count_out")) || !_tcscmp(swzRawSpec, _T("msg_count_in")))
	{
		DWORD dwCountOut, dwCountIn;
		DWORD dwMetaCountOut = 0, dwMetaCountIn = 0;
		DWORD dwLastTs, dwNewTs, dwRecountTs;
		DWORD dwTime, dwDiff;
		int iNumber = 1;
		HANDLE hTmpContact = hContact;

		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (szProto && !strcmp(szProto, szMetaModuleName))
		{
			iNumber = CallService(MS_MC_GETNUMCONTACTS, (WPARAM)hContact, 0);
			hTmpContact = (HANDLE)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hContact, 0);
		}

		for (int i = 0; i < iNumber; i++)
		{
			if (i > 0) hTmpContact = (HANDLE)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hContact, i);
			dwRecountTs = DBGetContactSettingDword(hTmpContact, MODULE, "LastCountTS", 0);
			dwTime = (DWORD)time(0);
			dwDiff = (dwTime - dwRecountTs);
			if (dwDiff > (60 * 60 * 24 * 3))
			{
				DBWriteContactSettingDword(hTmpContact, MODULE, "LastCountTS", dwTime);
				dwCountOut = dwCountIn = dwLastTs = 0;		
			} 
			else 
			{
				dwCountOut = DBGetContactSettingDword(hTmpContact, MODULE, "MsgCountOut", 0);
				dwCountIn = DBGetContactSettingDword(hTmpContact, MODULE, "MsgCountIn", 0);
				dwLastTs = DBGetContactSettingDword(hTmpContact, MODULE, "LastMsgTS", 0);
			}

			dwNewTs = dwLastTs;

			HANDLE dbe = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hTmpContact, 0);
			while (dbe != NULL) 
			{
				DBEVENTINFO dbei = {0};
				dbei.cbSize = sizeof(dbei);
				if (!CallService(MS_DB_EVENT_GET, (WPARAM)dbe, (LPARAM)&dbei))
				{
					if (dbei.eventType == EVENTTYPE_MESSAGE) 
					{
						dwNewTs = max(dwNewTs, dbei.timestamp);
						if (dbei.timestamp > dwLastTs) 
						{
							if (dbei.flags & DBEF_SENT) dwCountOut++;
							else dwCountIn++;
						} 
						else 
							break;
					}
				}
				dbe = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)dbe, 0);
			}

			if (dwNewTs > dwLastTs)
			{
				DBWriteContactSettingDword(hTmpContact, MODULE, "MsgCountOut", dwCountOut);
				DBWriteContactSettingDword(hTmpContact, MODULE, "MsgCountIn", dwCountIn);
				DBWriteContactSettingDword(hTmpContact, MODULE, "LastMsgTS", dwNewTs);
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

bool GetSubstText(HANDLE hContact, const DISPLAYSUBST &ds, TCHAR *buff, int bufflen) 
{
	TranslateFunc *transFunc = 0;
	for (int i = 0; i < iTransFuncsCount; i++)
	{
		if (translations[i].id == (DWORD)ds.iTranslateFuncId) 
		{
			transFunc = translations[i].transFunc;
			break;
		}
	}

	if (!transFunc) 
		return false;

	switch (ds.type) 
	{
		case DVT_DB:
			return transFunc(hContact, ds.szModuleName, ds.szSettingName, buff, bufflen) != 0;
		case DVT_PROTODB:
		{
			char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			if (szProto)
			{
				if (transFunc(hContact, szProto, ds.szSettingName, buff, bufflen) != 0)
					return true;
				else
					return transFunc(hContact, "UserInfo", ds.szSettingName, buff, bufflen) != 0;
			}
			break;
		}			
	}
	return false;
}

bool GetRawSubstText(HANDLE hContact, char *szRawSpec, TCHAR *buff, int bufflen) 
{
	size_t lenght = strlen(szRawSpec);
	for (size_t i = 0; i < lenght; i++) 
	{
		if (szRawSpec[i] == '/')
		{
			szRawSpec[i] = 0;
			if (strlen(szRawSpec) == 0)
			{
				char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
				if (szProto)
				{
					if (translations[0].transFunc(hContact, szProto, &szRawSpec[i + 1], buff, bufflen) != 0)
						return true;
					else
						return translations[0].transFunc(hContact, "UserInfo", &szRawSpec[i + 1], buff, bufflen) != 0;
				} 
				else
				{
					return false;
				}
			}
			else
			{
				return translations[0].transFunc(hContact, szRawSpec, &szRawSpec[i + 1], buff, bufflen) != 0;
			}
		}
	}
	return false;
}

bool ApplySubst(HANDLE hContact, const TCHAR *swzSource, bool parseTipperVarsFirst, TCHAR *swzDest, int iDestLen) 
{
	// hack - allow empty strings before passing to variables (note - zero length strings return false after this)
	if (swzDest && swzSource && _tcslen(swzSource) == 0) 
	{
		swzDest[0] = 0;
		return true;
	}

	// pass to variables plugin if available
	TCHAR *swzVarSrc;
	if (parseTipperVarsFirst == false)
		swzVarSrc = variables_parsedup((TCHAR *)swzSource, 0, hContact);
	else
		swzVarSrc = _tcsdup(swzSource);

	size_t iSourceLen = _tcslen(swzVarSrc);
	size_t si = 0, di = 0, v = 0;

	TCHAR swzVName[LABEL_LEN];
	TCHAR swzRep[VALUE_LEN], swzAlt[VALUE_LEN];
	while (si < iSourceLen && di < (size_t)iDestLen - 1) 
	{
		if (swzVarSrc[si] == _T('%'))
		{
			si++;
			v = 0;
			while (si < iSourceLen && v < LABEL_LEN) 
			{
				if (swzVarSrc[si] == _T('%'))
				{					
					// two %'s in a row in variable name disabled: e.g. %a%%b% - this is atbbguous]
					//if (si + 1 < iSourceLen && swzVarSrc[si + 1] == _T('%')) { 
					//	si++; // skip first %, allow following code to add the second one to the variable name
					//} else
						break;
				}
				swzVName[v] = swzVarSrc[si];
				v++; si++;
			}

			if (v == 0)  // bSubst len is 0 - just a % symbol
			{
				swzDest[di] = _T('%');
			} 
			else if (si < iSourceLen) // we found end %
			{
				swzVName[v] = 0;

				bool bAltSubst = false;
				bool bSubst = false;

				// apply only to specific protocols
				TCHAR *p = _tcsrchr(swzVName, _T('^')); // use last '^', so if you want a ^ in swzAlt text, you can just put a '^' on the end
				if (p) 
				{
					*p = 0;
					p++;
					if (*p) 
					{
						char *cp = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
						if (cp != NULL)
						{
							PROTOACCOUNT *acc = ProtoGetAccount(cp);
							if (acc != NULL)
							{
								cp = acc->szProtoName;
							}
						}
						
						if (cp == NULL)
							goto empty;

						bool negate = false;
						if (*p == _T('!')) 
						{
							p++;
							if (*p == 0) goto error;
							negate = true;
						}

						char sproto[256];
						bool spec = false;
						int len;

						TCHAR *last = _tcsrchr(p, _T(','));
						if (!last) last = p;

						while (p <= last + 1)
						{
							len = (int)_tcscspn(p, _T(","));
							t2a(p, sproto, len);
							sproto[len] = 0;
							p += len + 1;

							if (_stricmp(cp, sproto) == 0)
							{
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
				if (p) 
				{
					*p = 0; // clip swzAlt from swzVName
					p++;	
					if (_tcslen(p) > 4 && _tcsncmp(p, _T("raw:"), 4) == 0) // raw db substitution
					{ 
						char raw_spec[LABEL_LEN];
						p += 4;
						t2a(p, raw_spec, LABEL_LEN);
						GetRawSubstText(hContact, raw_spec, swzAlt, VALUE_LEN);
					} 
					else if (_tcslen(p) > 4 && _tcsncmp(p, _T("sys:"), 4) == 0) // 'system' substitution
					{ 
						p += 4;
						GetSysSubstText(hContact, p, swzAlt, VALUE_LEN);
					} 
					else
					{
						// see if we can find the bSubst
						DSListNode *ds_node = opt.dsList;
						while(ds_node) 
						{
							if (_tcscmp(ds_node->ds.swzName, p) == 0)
								break;
				
							ds_node = ds_node->next;
						}

						if (ds_node)
						{
							GetSubstText(hContact, ds_node->ds, swzAlt, VALUE_LEN);
						}
						else 
						{
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
				else
				{
					// see if we can find the bSubst
					DSListNode *ds_node = opt.dsList;
					while(ds_node)
					{
						if (_tcscmp(ds_node->ds.swzName, swzVName) == 0)
							break;
						
						ds_node = ds_node->next;
					}

					if (!ds_node) 
						goto error; // no such bSubst
				
					bSubst = GetSubstText(hContact, ds_node->ds, swzRep, VALUE_LEN);
				}

				if (bSubst)
				{
					size_t rep_len = _tcslen(swzRep);
					_tcsncpy(&swzDest[di], swzRep, min(rep_len, iDestLen - di));
					di += rep_len - 1; // -1 because we inc at bottom of loop
				}
				else if (bAltSubst)
				{
					size_t alt_len = _tcslen(swzAlt);
					_tcsncpy(&swzDest[di], swzAlt, min(alt_len, iDestLen - di));
					di += alt_len - 1; // -1 because we inc at bottom of loop
				} 
				else
				{
					goto empty; // empty value
				}

			} 
			else // no end % - error
			{
				goto error;
			}
		} 
		else
		{
			swzDest[di] = swzVarSrc[si];
		}

		si++;
		di++;
	}

	free(swzVarSrc);
	swzDest[di] = 0;

	if (parseTipperVarsFirst)
	{
		swzVarSrc = variables_parsedup((TCHAR *)swzDest, 0, hContact);
		_tcscpy(swzDest, swzVarSrc);
		free(swzVarSrc);
	}


	// check for a 'blank' string - just spaces etc
	for (si = 0; si <= di; si++) 
	{
		if (swzDest[si] != 0 && swzDest[si] != _T(' ') && swzDest[si] != _T('\t') && swzDest[si] != _T('\r') && swzDest[si] != _T('\n'))
			return true;
	}

	return false;

empty:
	free(swzVarSrc);
	return false;

error:
	swzDest[0] = _T('*');
	swzDest[1] = 0;
	free(swzVarSrc);
	return true;
}

bool GetLabelText(HANDLE hContact, const DISPLAYITEM &di, TCHAR *buff, int bufflen) 
{
	return ApplySubst(hContact, di.swzLabel, false, buff, bufflen);
}

bool GetValueText(HANDLE hContact, const DISPLAYITEM &di, TCHAR *buff, int bufflen) 
{
	return ApplySubst(hContact, di.swzValue, di.bParseTipperVarsFirst, buff, bufflen);
}

void TruncateString(TCHAR *swzText) 
{
	if (swzText && opt.iLimitCharCount > 3) 
	{
		if ((int)_tcslen(swzText) > opt.iLimitCharCount) 
		{
			swzText[opt.iLimitCharCount - 3] = 0;
			_tcscat(swzText, _T("..."));
		}
	}
}

TCHAR *GetProtoStatusMessage(char *szProto, WORD wStatus) 
{
	TCHAR *swzText = NULL;
	char *tmpMsg = NULL;

	if (!szProto || wStatus == ID_STATUS_OFFLINE) 
		return NULL;

	// check if protocol supports status message for status
	int flags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0);
	if (!(flags & Proto_Status2Flag(wStatus)))
		return NULL;
	
	swzText = (TCHAR *)CallProtoService(szProto, PS_GETMYAWAYMSG, 0, SGMA_TCHAR);
	if ((INT_PTR)swzText != CALLSERVICE_NOTFOUND) 
	{
		if (swzText == NULL)
			tmpMsg = (char *)CallProtoService(szProto, PS_GETMYAWAYMSG, 0, 0);
	}
	else
	{
		tmpMsg = (char *)CallService(MS_AWAYMSG_GETSTATUSMSG, wStatus, 0);
		swzText = NULL;
	}

	if (tmpMsg && (INT_PTR)tmpMsg != CALLSERVICE_NOTFOUND)
	{
		swzText = mir_a2t(tmpMsg);
		mir_free(tmpMsg);
	}
	
	if (swzText && !swzText[0])
	{
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

	if (!DBGetContactSettingTString(0, szProto, "XStatusName", &dbv)) 
	{
		if (_tcslen(dbv.ptszVal) != 0) 
			swzText = mir_tstrdup(dbv.ptszVal);
		DBFreeVariant(&dbv);
	} 

	if (!swzText) 
	{
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
	DBVARIANT dbv;
	TCHAR *swzText = NULL;

	if (!szProto) 
		return NULL;

	if (!DBGetContactSettingTString(0, szProto, "XStatusMsg", &dbv)) 
	{
		if (_tcslen(dbv.ptszVal) != 0)
			swzText = mir_tstrdup(dbv.ptszVal);
		DBFreeVariant(&dbv);

		if (ServiceExists(MS_VARS_FORMATSTRING))
		{		
			HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
			char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			while(!proto)
			{
				hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
				if (hContact) 
				{
					proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
				}
				else 
				{
					hContact = NULL;
					break;
				}
			}

			TCHAR *tszParsed = (TCHAR *)variables_parse(swzText, NULL, hContact);
			if (tszParsed) 
			{
				mir_free(swzText);
				swzText = mir_tstrdup(tszParsed);
				variables_free(tszParsed);
			}
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

	if (!DBGetContactSettingTString(0, szProto, "ListeningTo", &dbv))
	{
		if (_tcslen(dbv.ptszVal) != 0)
			swzText = mir_tstrdup(dbv.ptszVal);
		DBFreeVariant(&dbv);
	}

	if (opt.bLimitMsg)
		TruncateString(swzText); 

	return swzText;
}

TCHAR *GetJabberAdvStatusText(char *szProto, const char *szSlot, const char *szValue) 
{
	DBVARIANT dbv;
	TCHAR *swzText = NULL;
	char szSetting[128];

	if (!szProto) 
		return NULL;

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/%s", szProto, szSlot, szValue);
	if (!DBGetContactSettingTString(0, "AdvStatus", szSetting, &dbv))
	{
		if (_tcslen(dbv.ptszVal) != 0)
			swzText = mir_tstrdup(dbv.ptszVal);
		DBFreeVariant(&dbv);
	}

	if (opt.bLimitMsg)
		TruncateString(swzText); 

	return swzText;
}

HICON GetJabberActivityIcon(HANDLE hContact, char *szProto)  
{	
	DBVARIANT dbv;
	HICON hIcon = NULL;
	char szSetting[128];

	if (!szProto) 
		return NULL;

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/%s", szProto, "activity", "icon");
	if (!DBGetContactSettingString(hContact, "AdvStatus", szSetting, &dbv)) 
	{
		hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	return hIcon;
}