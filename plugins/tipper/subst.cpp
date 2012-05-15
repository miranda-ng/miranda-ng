#include "common.h"
#include "subst.h"
#include "str_utils.h"

void StripBBCodesInPlace(TCHAR *text) {
	if(!DBGetContactSettingByte(0, MODULE, "StripBBCodes", 1))
		return;

	if(text == 0) return;

	int read = 0, write = 0;
	int len = (int)_tcslen(text);

	while(read <= len) { // copy terminating null too
		while(read <= len && text[read] != '[') {
			if(text[read] != text[write]) text[write] = text[read];
			read++; write++;
		}
		if(read > len) break;

		if(len - read >= 3 && (_tcsnicmp(text + read, _T("[b]"), 3) == 0 || _tcsnicmp(text + read, _T("[i]"), 3) == 0))
			read += 3;
		else if(len - read >= 4 && (_tcsnicmp(text + read, _T("[/b]"), 4) == 0 || _tcsnicmp(text + read, _T("[/i]"), 4) == 0))
			read += 4;
		else if(len - read >= 6 && (_tcsnicmp(text + read, _T("[color"), 6) == 0)) {
			while(read < len && text[read] != ']') read++; 
			read++;// skip the ']'
		} else if(len - read >= 8 && (_tcsnicmp(text + read, _T("[/color]"), 8) == 0))
			read += 8;
		else if(len - read >= 5 && (_tcsnicmp(text + read, _T("[size"), 5) == 0)) {
			while(read < len && text[read] != ']') read++; 
			read++;// skip the ']'
		} else if(len - read >= 7 && (_tcsnicmp(text + read, _T("[/size]"), 7) == 0))
			read += 7;
		else {
			if(text[read] != text[write]) text[write] = text[read];
			read++; write++;
		}
	}
}

DWORD last_message_timestamp(HANDLE hContact) {
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	HANDLE hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0);
	while(hDbEvent) {
		dbei.cbBlob = 0;
		CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei);
		if(dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
			break;
		}
		hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hDbEvent, 0);
	}

	if(hDbEvent) return dbei.timestamp;

	return 0;
}

void format_timestamp(DWORD ts, char *format, TCHAR *buff, int bufflen) {
	TCHAR form[16];
	DBTIMETOSTRINGT dbt = {0};
	dbt.cbDest = bufflen;
	dbt.szDest = buff;
	a2t(format, form, 16);
	dbt.szFormat = form;
	CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ts, (LPARAM)&dbt);
}

bool uid(HANDLE hContact, char *proto, TCHAR *buff, int bufflen) {
	CONTACTINFO ci;
	ci.cbSize = sizeof(CONTACTINFO); 
	ci.hContact = hContact;
	// pass in proto so we can get uid when hContact == 0 (i.e. our own uid for a given proto)
	ci.szProto = proto;//(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0);
	ci.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
	if(!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		switch(ci.type) {
			case CNFT_BYTE:
				_ltot(ci.bVal, buff, 10);
				break; 
			case CNFT_WORD:
				_ltot(ci.wVal, buff, 10);
				break;
			case CNFT_DWORD:
				_ltot(ci.dVal, buff, 10);
				break;
			case CNFT_ASCIIZ:
				_tcsncpy(buff, ci.pszVal, bufflen);
				buff[bufflen-1] = 0;
				break;
			default: 
				return false;
		}
		return true;
	}
	return false;
}

bool uid_name(char *szProto, TCHAR *buff, int bufflen) {
	if (szProto){
		char *szUniqueId = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
		if(szUniqueId) {
			a2t(szUniqueId, buff, bufflen);
			return true;
		}       
	}
	return false;
}

TCHAR *GetLastMessageText(HANDLE hContact) {
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);

	HANDLE hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0);
	while(hDbEvent) {
		dbei.cbBlob = 0;
		CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei);
		if(dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
			break;
		}
		hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hDbEvent, 0);
	}

	if(hDbEvent) {
		dbei.pBlob = (BYTE *)alloca(dbei.cbBlob);
		CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei);

		if(dbei.cbBlob == 0 || dbei.pBlob == 0) return 0;

		TCHAR *buf = DbGetEventTextT( &dbei, CP_ACP );
		TCHAR *msg = _tcsdup( buf );
		mir_free( buf );

		StripBBCodesInPlace(msg);
		return msg;
	}

	return 0;
}

TCHAR *GetStatusMessageText(HANDLE hContact) {
	TCHAR *ret = 0;
	DBVARIANT dbv;
	if(!DBGetContactSettingTString(hContact, MODULE, "TempStatusMsg", &dbv)) {
		if(dbv.type != DBVT_DELETED && dbv.ptszVal && dbv.ptszVal[0]) {
			ret = _tcsdup(dbv.ptszVal);
			StripBBCodesInPlace(ret);
		} else CallContactService(hContact, PSS_GETAWAYMSG, 0, 0);
		DBFreeVariant(&dbv);
	/*
	// removed - people can use e.g. %raw:CList/StatusMsg% for SMR
	} else if(!DBGetContactSettingTString(hContact, "CList", "StatusMsg", &dbv)) {
		if(_tcslen(dbv.ptszVal)) ret = _tcsdup(dbv.ptszVal);
		else CallContactService(hContact, PSS_GETAWAYMSG, 0, 0);
		DBFreeVariant(&dbv);
	*/
	} else
		CallContactService(hContact, PSS_GETAWAYMSG, 0, 0);

	return ret;
}

bool GetSysSubstText(HANDLE hContact, TCHAR *raw_spec, TCHAR *buff, int bufflen) {
	if (!_tcscmp(raw_spec, _T("uid"))) {
		return uid(hContact, 0, buff, bufflen);
	} else if (!_tcscmp(raw_spec, _T("proto"))) {
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, (WPARAM)hContact, 0);
		if ((INT_PTR)szProto == CALLSERVICE_NOTFOUND) {
			szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			if (szProto) {
				a2t(szProto, buff, bufflen);
				return true;
			}
		}
		else if (szProto) {
				PROTOACCOUNT *pa = ProtoGetAccount(szProto);
				_tcsncpy(buff, pa->tszAccountName, bufflen);
				return true;
		}
	} else if (!_tcscmp(raw_spec, _T("time"))) {
		if (tmi.printDateTime && !tmi.printDateTimeByContact(hContact, _T("t"), buff, bufflen, TZF_KNOWNONLY))
			return true;
	} else if (!_tcscmp(raw_spec, _T("uidname"))) {
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0); 
		return uid_name(szProto, buff, bufflen);
	} else if (!_tcscmp(raw_spec, _T("status_msg"))) {
		TCHAR *msg = GetStatusMessageText(hContact);
		if(msg) {
			_tcsncpy(buff, msg, bufflen);
			free(msg);
			return true;
		}
	} else if (!_tcscmp(raw_spec, _T("last_msg"))) {
		TCHAR *msg = GetLastMessageText(hContact);
		if(msg) {
			_tcsncpy(buff, msg, bufflen);
			free(msg);
			return true;
		}
	} else if (!_tcscmp(raw_spec, _T("meta_subname"))) {
		// get contact list name of active subcontact
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND) return false;

		TCHAR *stzCDN = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hSubContact, GCDNF_TCHAR);
		if(stzCDN) _tcsncpy(buff, stzCDN, bufflen);
		return true;
	} else if (!_tcscmp(raw_spec, _T("meta_subuid"))){
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND) return false;
		return uid(hSubContact, 0, buff, bufflen);
	} else if (!_tcscmp(raw_spec, _T("meta_subproto"))) {
		// get protocol of active subcontact
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if (!hSubContact || (INT_PTR)hSubContact == CALLSERVICE_NOTFOUND) return false;

		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, (WPARAM)hSubContact, 0);
		if ((INT_PTR)szProto == CALLSERVICE_NOTFOUND) {
			szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubContact, 0);
			if (szProto) {
				a2t(szProto, buff, bufflen);
				return true;
			}
		}
		else if (szProto) {
				PROTOACCOUNT *pa = ProtoGetAccount(szProto);
				_tcsncpy(buff, pa->tszAccountName, bufflen);
				return true;
		}
	} else if (!_tcscmp(raw_spec, _T("last_msg_time"))) {
		DWORD ts = last_message_timestamp(hContact);
		if(ts == 0) return false;

		format_timestamp(ts, "t", buff, bufflen);
		return true;
	} else if (!_tcscmp(raw_spec, _T("last_msg_date"))) {
		DWORD ts = last_message_timestamp(hContact);
		if(ts == 0) return false;

		format_timestamp(ts, "d", buff, bufflen);
		return true;
	} else if (!_tcscmp(raw_spec, _T("last_msg_reltime"))) {
		DWORD ts = last_message_timestamp(hContact);
		if(ts == 0) return false;

		DWORD t = (DWORD)time(0);
		DWORD diff = (t - ts);
		int d = (diff / 60 / 60 / 24);
		int h = (diff - d * 60 * 60 * 24) / 60 / 60;
		int m = (diff  - d * 60 * 60 * 24 - h * 60 * 60) / 60;
		if(d > 0) mir_sntprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
		else if(h > 0) mir_sntprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
		else mir_sntprintf(buff, bufflen, TranslateT("%dm"), m);
		
		return true;
	}
	return false;
}

bool GetSubstText(HANDLE hContact, const DisplaySubst &ds, TCHAR *buff, int bufflen) {
	TranslateFunc *tfunc = 0;
	for(int i = 0; i < num_tfuncs; i++) {
		if(translations[i].id == (DWORD)ds.translate_func_id) {
			tfunc = translations[i].tfunc;
			break;
		}
	}
	if(!tfunc) return false;

	switch(ds.type) {
		case DVT_DB:
			return tfunc(hContact, ds.module_name, ds.setting_name, buff, bufflen) != 0;
		case DVT_PROTODB:
			{
				char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
				if(proto) {
					return tfunc(hContact, proto, ds.setting_name, buff, bufflen) != 0;
				}
			}
			break;
	}
	return false;
}

bool GetRawSubstText(HANDLE hContact, char *raw_spec, TCHAR *buff, int bufflen) {
	int len = (int)strlen(raw_spec);
	for(int i = 0; i < len; i++) {
		if(raw_spec[i] == '/') {
			raw_spec[i] = 0;
			if(strlen(raw_spec) == 0) {
				char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
				if(proto)
					return translations[0].tfunc(hContact, proto, &raw_spec[i + 1], buff, bufflen) != 0;
				else
					return false;
			} else
				return translations[0].tfunc(hContact, raw_spec, &raw_spec[i + 1], buff, bufflen) != 0;
		}
	}
	return false;
}

bool ApplySubst(HANDLE hContact, const TCHAR *source, TCHAR *dest, int dest_len) {
	// hack - allow empty strings before passing to variables (note - zero length strings return false after this)
	if(dest && source &&_tcslen(source) == 0) {
		dest[0] = 0;
		return true;
	}

	// pass to variables plugin if available
	TCHAR *var_src = variables_parsedup((TCHAR *)source, 0, hContact);
	//TCHAR *var_src = wcsdup(source); // disable variables
	int source_len = (int)_tcslen(var_src);

	int si = 0, di = 0, v = 0;
	TCHAR vname[LABEL_LEN];
	TCHAR rep[VALUE_LEN], alt[VALUE_LEN];
	while(si < source_len && di < dest_len - 1) {
		if(var_src[si] == _T('%')) {
			si++;
			v = 0;
			while(si < source_len && v < LABEL_LEN) {
				if(var_src[si] == _T('%')) {					
					// two %'s in a row in variable name disabled: e.g. %a%%b% - this is ambiguous]
					//if(si + 1 < source_len && var_src[si + 1] == _T('%')) { 
					//	si++; // skip first %, allow following code to add the second one to the variable name
					//} else
						break;
				}
				vname[v] = var_src[si];
				v++; si++;
			}
			if(v == 0) { // subst len is 0 - just a % symbol
				dest[di] = _T('%');
			} else if(si < source_len) { // we found end %
				vname[v] = 0;

				bool alt_subst = false;
				bool subst = false;

				// apply only to specific protocol
				TCHAR *p = _tcsrchr(vname, _T('^')); // use last '^', so if you want a ^ in alt text, you can just put a '^' on the end
				if(p) {
					*p = 0;
					p++;
					if(*p) {
						bool negate = false;
						if(*p == _T('!')) {
							p++;
							if(*p == 0) goto error;
							negate = true;
						}

						char sproto[256], *cp;
						t2a(p, sproto, 256);
						cp = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
						if(cp == 0 || (negate ? stricmp(cp, sproto) == 0 : stricmp(cp, sproto) != 0))
							goto empty;
					}
				}

				// get alternate text, if subst fails
				alt[0] = 0;
				p = _tcschr(vname, _T('|')); // use first '|' - so you can use the '|' symbol in alt text
				if(p) {
					*p = 0; // clip alt from vname
					alt_subst = true;

					p++;
					_tcsncpy(alt, p, VALUE_LEN);
					alt[VALUE_LEN - 1] = 0;
				}
			
				// get subst text
				if(v > 4 && _tcsncmp(vname, _T("raw:"), 4) == 0) { // raw db substitution
					char raw_spec[LABEL_LEN];
					t2a(&vname[4], raw_spec, LABEL_LEN);
					subst = GetRawSubstText(hContact, raw_spec, rep, VALUE_LEN);
				} else if(v > 4 && _tcsncmp(vname, _T("sys:"), 4) == 0) { // 'system' substitution
					subst = GetSysSubstText(hContact, &vname[4], rep, VALUE_LEN);
				} else {
					// see if we can find the subst
					DSListNode *ds_node = options.ds_list;
					while(ds_node) {
						if(_tcscmp(ds_node->ds.name, vname) == 0)
							break;
						
						ds_node = ds_node->next;
					}
					if(!ds_node) goto error; // no such subst
				
					subst = GetSubstText(hContact, ds_node->ds, rep, VALUE_LEN);
				}

				if(subst) {
					int rep_len = (int)_tcslen(rep);
					_tcsncpy(&dest[di], rep, min(rep_len, dest_len - di));
					di += rep_len - 1; // -1 because we inc at bottom of loop
				} else if(alt_subst) {
					int alt_len = (int)_tcslen(alt);
					_tcsncpy(&dest[di], alt, min(alt_len, dest_len - di));
					di += alt_len - 1; // -1 because we inc at bottom of loop
				} else
					goto empty; // empty value

			} else // no end % - error
				goto error;

		} else {
			dest[di] = var_src[si];
		}

		si++;
		di++;
	}

	free(var_src);
	dest[di] = 0;

	// check for a 'blank' string - just spaces etc
	for(si = 0; si <= di; si++) {
		if(dest[si] != 0 && dest[si] != _T(' ') && dest[si] != _T('\t') && dest[si] != _T('\r') && dest[si] != _T('\n'))
			return true;
	}

	return false;

empty:
	free(var_src);
	return false;

error:
	dest[0] = _T('*');
	dest[1] = 0;
	free(var_src);
	return true;
}

bool GetLabelText(HANDLE hContact, const DisplayItem &di, TCHAR *buff, int bufflen) {
	return ApplySubst(hContact, di.label, buff, bufflen);

}

bool GetValueText(HANDLE hContact, const DisplayItem &di, TCHAR *buff, int bufflen) {
	return ApplySubst(hContact, di.value, buff, bufflen);
}



