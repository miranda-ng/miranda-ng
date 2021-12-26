#include "stdafx.h"
#include <locale.h>


wchar_t *_tcstolower(wchar_t *dst)
{
	if (dst == nullptr)
		return nullptr;
	
	SIZE_T dst_len = mir_wstrlen(dst);
	for (SIZE_T i = 0; i < dst_len; i ++)
		dst[i] = towlower(dst[i]);
	return dst;
}

wchar_t *_tcstoupper(wchar_t *dst)
{
	if (dst == nullptr)
		return nullptr;
	
	SIZE_T dst_len = mir_wstrlen(dst);
	for (SIZE_T i = 0; i < dst_len; i ++)
		dst[i] = towupper(dst[i]);
	return dst;
}

BOOL _isregex(wchar_t* strSearch)
{
	BOOL ret = FALSE;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	wchar_t *regex;
	wchar_t regex_parse[] = L"/(.*)/([igsm]*)";
	int ovector[9];

	if (strSearch == nullptr)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, nullptr);
	if (re == nullptr)
		return FALSE;
	regex = mir_wstrdup(strSearch);
	if (regex == nullptr)
		goto err_out;
	rc = pcre16_exec(re, nullptr, regex, (int)mir_wstrlen(regex), 0, 0, ovector, 9);
	if (rc == 3)
		ret = TRUE;
	mir_free(regex);
err_out:
	pcre16_free(re);

	return ret;
}

BOOL _isvalidregex(wchar_t* strSearch)
{
	BOOL ret = FALSE;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	wchar_t *regex, *regexp, *mod;
	int opts = 0;
	wchar_t regex_parse[] = L"/(.*)/([igsm]*)";
	int ovector[9];

	if (strSearch == nullptr)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, nullptr);
	if (re == nullptr)
		return FALSE;
	regex = mir_wstrdup(strSearch);
	if (regex == nullptr) {
		pcre16_free(re);
		return FALSE;
	}
	rc = pcre16_exec(re, nullptr, regex, (int)mir_wstrlen(regex), 0, 0, ovector, 9);
	pcre16_free(re);
	if (rc != 3)
		goto err_out;
	regexp = regex + ovector[2];
	regexp[ovector[3] - ovector[2]] = 0;
	mod = regex + ovector[4];
	mod[ovector[5] - ovector[4]] = 0;

	if (wcsstr(mod, L"i"))
		opts |= PCRE_CASELESS;
	if (wcsstr(mod, L"m"))
		opts |= PCRE_MULTILINE;
	if (wcsstr(mod, L"s"))
		opts |= PCRE_DOTALL;

	re = pcre16_compile(regexp, opts, &error, &erroroffs, nullptr);
	if (re != nullptr) {
		pcre16_free(re);
		ret = TRUE;
	}
err_out:
	mir_free(regex);

	return ret;
}

BOOL _regmatch(wchar_t* str, wchar_t* strSearch)
{
	BOOL ret = FALSE;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	wchar_t *regex, *regexp, *data = nullptr, *mod;
	int opts = 0;
	wchar_t regex_parse[] = L"^/(.*)/([igsm]*)";
	int ovector[9];

	if (str == nullptr || strSearch == nullptr)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, nullptr);
	if (re == nullptr)
		return FALSE; // [TODO] and log some error
	regex = mir_wstrdup(strSearch);
	if (regex == nullptr) {
		pcre16_free(re);
		return FALSE;
	}
	rc = pcre16_exec(re, nullptr, regex, (int)mir_wstrlen(regex), 0, 0, ovector, 9);
	pcre16_free(re);
	if (rc != 3)
		goto err_out; // [TODO] and log some error (better check for valid regex on options save)
	regexp = regex + ovector[2];
	regexp[ovector[3] - ovector[2]] = 0;
	mod = regex + ovector[4];
	mod[ovector[5] - ovector[4]] = 0;

	data = mir_wstrdup(str);
	if (data == nullptr)
		goto err_out;
	if (wcsstr(mod, L"i"))
		opts |= PCRE_CASELESS;
	if (wcsstr(mod, L"m"))
		opts |= PCRE_MULTILINE;
	if (wcsstr(mod, L"s"))
		opts |= PCRE_DOTALL;

	re = pcre16_compile(regexp, opts, &error, &erroroffs, nullptr);
	if (re == nullptr)
		goto err_out;
	rc = pcre16_exec(re, nullptr, data, (int)mir_wstrlen(data), 0, 0, nullptr, 0);
	pcre16_free(re);
	if (rc >= 0)
		ret = TRUE;
err_out:
	mir_free(regex);
	mir_free(data);

	return ret;
}

int get_response_id(const wchar_t* strvar)
{
	int ret = 0;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	wchar_t *_str, *_strvar;
	wchar_t regex[] = L"^%response([#-_]([0-9]+))?%$";
	int ovector[9];

	if (strvar == nullptr)
		return 0;
	re = pcre16_compile(regex, 0, &error, &erroroffs, nullptr);
	if (re == nullptr)
		return 0; // [TODO] and log some error
	_strvar = mir_wstrdup(strvar);
	if (_strvar == nullptr) {
		pcre16_free(re);
		return 0;
	}
	rc = pcre16_exec(re, nullptr, _strvar, (int)mir_wstrlen(_strvar), 0, 0, ovector, 9);
	pcre16_free(re);
	if (rc < 0) {
		ret = -1;
	} else if (rc == 3) {
		_str = _strvar + ovector[4];
		_str[ovector[5] - ovector[4]] = 0;
		ret = _wtoi(_str);
	}
	mir_free(_strvar);

	return ret;
}

int get_response_num(const wchar_t *str)
{
	int i = 0;
	wchar_t *tmp, *strc;
	
	if (str == nullptr)
		return 0;
	strc = mir_wstrdup(str);
	if (strc == nullptr)
		return 0;
	tmp = wcstok(strc, L"\r\n");
	while (tmp) {
		i ++;
		tmp = wcstok(nullptr, L"\r\n"); /* Move next. */
	}
	mir_free(strc);

	return i;
}

wchar_t* get_response(wchar_t* dst, unsigned int dstlen, int num)
{
	int i = 0;
	wchar_t *tmp, *src;

	if (dst == nullptr || dstlen == 0 || num < 0)
		return dst;
	src = (wchar_t*)mir_alloc(MAX_BUFFER_LENGTH * sizeof(wchar_t));
	if (src == nullptr)
		goto err_out;
	_getOptS(src, MAX_BUFFER_LENGTH, "Response", defaultResponse);
	tmp = wcstok(src, L"\r\n");
	while (tmp) {
		if (i == num) {
			mir_wstrcpy(dst, tmp);
			mir_free(src);
			return dst;
		}
		i ++;
		tmp = wcstok(nullptr, L"\r\n"); /* Move next. */
	}
	mir_free(src);
err_out:
	dst[0] = 0;
	return dst;
}

wchar_t* _tcsstr_cc(wchar_t* str, wchar_t* strSearch, BOOL cc)
{
	wchar_t *ret = nullptr, *_str = nullptr, *_strSearch = nullptr;

	if (cc)
		return wcsstr(str, strSearch);

	_str = mir_wstrdup(str);
	if (_str == nullptr)
		goto err_out;
	_strSearch = mir_wstrdup(strSearch);
	if (_strSearch == nullptr)
		goto err_out;
	ret = wcsstr(_tcstolower(_str), _tcstolower(_strSearch));
	if (ret != nullptr)
		ret = ((ret - _str) + str);
err_out:
	mir_free(_str);
	mir_free(_strSearch);

	return ret;
}

BOOL Contains(wchar_t* dst, wchar_t* src) // Checks for occurence of substring from src in dst
{
	BOOL ret = FALSE;
	wchar_t *tsrc = nullptr, *tdst = nullptr, *token, *token_end;
	SIZE_T dst_len;

	if (dst == nullptr || src == nullptr)
		return FALSE;
	tsrc = mir_wstrdup(src);
	if (tsrc == nullptr)
		goto err_out;
	tdst = mir_wstrdup(dst);
	if (tdst == nullptr)
		goto err_out;
	tdst = _tcstoupper(tdst);
	dst_len = mir_wstrlen(tdst);
	token = wcstok(tsrc, L",");
	while (token) {
		token_end = (token + mir_wstrlen(token));
		while (!wcsncmp(token, L" ", 1)) { /* Skeep spaces at start. */
			token ++;
		}
		/* Skeep spaces at end. */
		while (token > token_end && wcschr((token_end - 1), ' ')) {
			token_end --;
			token_end[0] = 0;
		}
		/* Compare. */
		if (dst_len == (token_end - token) &&
		    0 == memcmp(tdst, _tcstoupper(token), (dst_len * sizeof(wchar_t)))) {
			ret = TRUE;
			break;		
		}
		token = wcstok(nullptr, L","); /* Move next. */
	}
err_out:
	mir_free(tsrc);
	mir_free(tdst);
	return ret;
}

BOOL isOneDay(uint32_t timestamp1, uint32_t timestamp2)
{
	time_t t1, t2;
	int at1[3], at2[3];
	struct tm *tm;
	if (!timestamp1 || !timestamp2)
		return FALSE;
	t1 = (time_t)timestamp1;
	t2 = (time_t)timestamp2;
	tm = gmtime(&t1);
	at1[0] = tm->tm_mday;
	at1[1] = tm->tm_mon;
	at1[2] = tm->tm_year;
	tm = gmtime(&t2);
	at2[0] = tm->tm_mday;
	at2[1] = tm->tm_mon;
	at2[2] = tm->tm_year;
	if (memcmp(&at1, &at2, 3*sizeof(int)) == 0)
		return TRUE;
	return FALSE;
}

wchar_t* ReplaceVar(wchar_t *dst, unsigned int len, const wchar_t *var, const wchar_t *rvar)
{
	wchar_t *var_start;
	SIZE_T dst_len, var_len, rvar_len;

	if (dst == nullptr || var == nullptr || rvar == nullptr)
		return nullptr;
	dst_len = mir_wstrlen(dst);
	var_len = mir_wstrlen(var);
	rvar_len = mir_wstrlen(rvar);
	var_start = wcsstr(dst, var);
	while (var_start) {
		if (len < (dst_len + rvar_len - var_len + 1))
			return nullptr; /* Out of buf space. */
		memmove((var_start + rvar_len),
		    (var_start + var_len),
		    (((dst + dst_len + 1) - (var_start + var_len)) * sizeof(wchar_t)));
		if (var_len >= rvar_len) { /* Buf data size not changed or decreased. */
			dst_len -= (var_len - rvar_len);
		} else { /* Buf data size increased. */
			dst_len += (rvar_len - var_len);
		}
		memcpy(var_start, rvar, (rvar_len * sizeof(wchar_t)));
		var_start = wcsstr(dst, var); /* Move next. */
	}

	return dst;
}

wchar_t* ReplaceVars(wchar_t *dst, unsigned int len)
{
	return ReplaceVarsNum(dst, len, -1);
}

wchar_t* ReplaceVarsNum(wchar_t *dst, unsigned int len, int num)
{
	wchar_t response[MAX_BUFFER_LENGTH];
	int ret, i;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	wchar_t *_str, *tmp, **r = nullptr, **tr, *ttmp, *dstcopy;
	wchar_t regex[] = L"%response([#-_]([0-9]+))?%";
	int ovector[9];

	re = pcre16_compile(regex, 0, &error, &erroroffs, nullptr);
	if (re == nullptr)
		return FALSE; // [TODO] and log some error
	_getOptS(response, _countof(response), "Response", defaultResponse);	

	ttmp = wcstok(response, L"\r\n");
	for (i = 0; ttmp != nullptr; i ++) {
		tr = (wchar_t**)mir_realloc(r, ((i + 1) * sizeof(wchar_t*)));
		if (tr == nullptr)
			goto err_out;
		r = tr;
		r[i] = ttmp;
		ttmp = wcstok(nullptr, L"\r\n"); /* Move next. */
	}

	do {
		_str = mir_wstrdup(dst);
		dstcopy = mir_wstrdup(dst);
		if (_str == nullptr || dstcopy == nullptr) {
			mir_free(_str);
			mir_free(dstcopy);
			goto err_out;
		}
		rc = pcre16_exec(re, nullptr, _str, (int)mir_wstrlen(_str), 0, 0, ovector, 9);
		if (rc < 0) {
			ret = -1;
		} else if (rc == 3) {
			ttmp = dstcopy + ovector[0];
			ttmp[ovector[1] - ovector[0]] = 0;
			tmp = _str + ovector[4];
			tmp[ovector[5] - ovector[4]] = 0;
			ret = _wtoi(tmp);
		} else {
			ttmp = dstcopy + ovector[0];
			ttmp[ovector[1] - ovector[0]] = 0;
			ret = 0;
		}

		if (ret >= 0) {
			if (ret > i)
				ReplaceVar(dst, len, ttmp, r[0]);
			else if (ret == 0 && num > 0 && num < i)
				ReplaceVar(dst, len, ttmp, r[num]);
			else
				ReplaceVar(dst, len, ttmp, r[ret == 0 ? 0 : (ret - 1)]);
		}

		mir_free(_str);
		mir_free(dstcopy);
	} while (rc >= 0);
err_out:
	mir_free(r);
	pcre16_free(re);

	return dst;
} 

int _notify(MCONTACT hContact, uint8_t type, wchar_t *message, wchar_t *origmessage)
{
	char *tmp, *tmporig;
	wchar_t msg[MAX_BUFFER_LENGTH];
	mir_snwprintf(msg, message, Clist_GetContactDisplayName(hContact));

	if (g_plugin.getByte("LogActions", defaultLogActions)) {
		tmp = mir_u2a(msg);
		tmporig = mir_u2a(origmessage);
		LogToSystemHistory(tmp, origmessage ? tmporig : nullptr);
		mir_free(tmp);
		mir_free(tmporig);
	}

	if (_NOTIFYP) {
		if (type == POPUP_BLOCKED) {
			if (g_plugin.getByte("NotifyPopupBlocked", defaultNotifyPopupBlocked))
				ShowPopup(hContact, type, nullptr, msg);
		} else if (type == POPUP_APPROVED) {
			if (g_plugin.getByte("NotifyPopupApproved", defaultNotifyPopupApproved))
				ShowPopup(hContact, type, nullptr, msg);
		} else if (type == POPUP_CHALLENGE) {
			if (g_plugin.getByte("NotifyPopupChallenge", defaultNotifyPopupChallenge))
				ShowPopup(hContact, type, nullptr, msg);
		} else {
			ShowPopup(hContact, type, nullptr, msg);
		}
	}
	return 0;
}
#define DOT(a) (a[mir_strlen(a)-1] == 46) ? "" : "."
int LogToSystemHistory(char *message, char *origmessage)
{
	char msg[MAX_BUFFER_LENGTH];
	time_t tm;

	if (message == nullptr)
		return 0;

	DBEVENTINFO dbei = {};
	dbei.timestamp = time(&tm);
	dbei.szModule = MODULENAME;
	dbei.pBlob = (uint8_t*)msg;
	if (origmessage)
		dbei.cbBlob = (1 + mir_snprintf(msg, "%s: %s%s %s: %s", MODULENAME, message, DOT(message), Translate("Their message was"), origmessage));
	else 
		dbei.cbBlob = (1 + mir_snprintf(msg, "%s: %s%s", MODULENAME, message, DOT(message)));
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_READ;
	db_event_add(NULL, &dbei);
	return 0;
}

void MarkUnread(MCONTACT hContact)
{
	// We're not actually marking anything. We just pushing saved events to the database from a temporary location
	DBVARIANT _dbv = {0};
	uint8_t *pos;
	
	if (hContact == NULL)
		return;
	
	if (db_get(hContact, MODULENAME, "LastMsgEvents", &_dbv) == 0) {
		pos = _dbv.pbVal;
		while (pos - _dbv.pbVal < _dbv.cpbVal) {
			DBEVENTINFO dbei = {};
			memcpy(&dbei.eventType, pos, sizeof(uint16_t)); pos += sizeof(uint16_t);
			memcpy(&dbei.flags, pos, sizeof(uint32_t)); pos += sizeof(uint32_t);
			memcpy(&dbei.timestamp, pos, sizeof(uint32_t)); pos += sizeof(uint32_t);

			dbei.szModule = (char*)malloc(mir_strlen((const char*)pos)+1);
			mir_strcpy((char*)dbei.szModule, (const char*)pos);
			pos += mir_strlen((const char*)pos)+1;

			memcpy(&dbei.cbBlob, pos, sizeof(uint32_t)); pos += sizeof(uint32_t);
			dbei.pBlob = (uint8_t*)malloc(dbei.cbBlob);
			memcpy(dbei.pBlob, pos, dbei.cbBlob);
			pos += dbei.cbBlob;
			db_event_add(hContact,&dbei);
		}
		db_free(&_dbv);
		g_plugin.delSetting(hContact, "LastMsgEvents");
	}
}
