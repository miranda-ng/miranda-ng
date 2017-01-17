#include "stdafx.h"
#include <locale.h>


wchar_t *_tcstolower(wchar_t *dst)
{
	if (dst == NULL)
		return NULL;
	
	SIZE_T dst_len = mir_wstrlen(dst);
	for (SIZE_T i = 0; i < dst_len; i ++)
		dst[i] = towlower(dst[i]);
	return dst;
}

wchar_t *_tcstoupper(wchar_t *dst)
{
	if (dst == NULL)
		return NULL;
	
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

	if (strSearch == NULL)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE;
	regex = mir_wstrdup(strSearch);
	if (regex == NULL)
		goto err_out;
	rc = pcre16_exec(re, NULL, regex, (int)mir_wstrlen(regex), 0, 0, ovector, 9);
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

	if (strSearch == NULL)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE;
	regex = mir_wstrdup(strSearch);
	if (regex == NULL) {
		pcre16_free(re);
		return FALSE;
	}
	rc = pcre16_exec(re, NULL, regex, (int)mir_wstrlen(regex), 0, 0, ovector, 9);
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

	re = pcre16_compile(regexp, opts, &error, &erroroffs, NULL);
	if (re != NULL) {
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
	wchar_t *regex, *regexp, *data = NULL, *mod;
	int opts = 0;
	wchar_t regex_parse[] = L"^/(.*)/([igsm]*)";
	int ovector[9];

	if (str == NULL || strSearch == NULL)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE; // [TODO] and log some error
	regex = mir_wstrdup(strSearch);
	if (regex == NULL) {
		pcre16_free(re);
		return FALSE;
	}
	rc = pcre16_exec(re, NULL, regex, (int)mir_wstrlen(regex), 0, 0, ovector, 9);
	pcre16_free(re);
	if (rc != 3)
		goto err_out; // [TODO] and log some error (better check for valid regex on options save)
	regexp = regex + ovector[2];
	regexp[ovector[3] - ovector[2]] = 0;
	mod = regex + ovector[4];
	mod[ovector[5] - ovector[4]] = 0;

	data = mir_wstrdup(str);
	if (data == NULL)
		goto err_out;
	if (wcsstr(mod, L"i"))
		opts |= PCRE_CASELESS;
	if (wcsstr(mod, L"m"))
		opts |= PCRE_MULTILINE;
	if (wcsstr(mod, L"s"))
		opts |= PCRE_DOTALL;

	re = pcre16_compile(regexp, opts, &error, &erroroffs, NULL);
	if (re == NULL)
		goto err_out;
	rc = pcre16_exec(re, NULL, data, (int)mir_wstrlen(data), 0, 0, NULL, 0);
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

	if (strvar == NULL)
		return 0;
	re = pcre16_compile(regex, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return 0; // [TODO] and log some error
	_strvar = mir_wstrdup(strvar);
	if (_strvar == NULL) {
		pcre16_free(re);
		return 0;
	}
	rc = pcre16_exec(re, NULL, _strvar, (int)mir_wstrlen(_strvar), 0, 0, ovector, 9);
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
	
	if (str == NULL)
		return 0;
	strc = mir_wstrdup(str);
	if (strc == NULL)
		return 0;
	tmp = wcstok(strc, L"\r\n");
	while (tmp) {
		i ++;
		tmp = wcstok(NULL, L"\r\n"); /* Move next. */
	}
	mir_free(strc);

	return i;
}

wchar_t* get_response(wchar_t* dst, unsigned int dstlen, int num)
{
	int i = 0;
	wchar_t *tmp, *src;

	if (dst == NULL || dstlen == 0 || num < 0)
		return dst;
	src = (wchar_t*)mir_alloc(MAX_BUFFER_LENGTH * sizeof(wchar_t));
	if (src == NULL)
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
		tmp = wcstok(NULL, L"\r\n"); /* Move next. */
	}
	mir_free(src);
err_out:
	dst[0] = 0;
	return dst;
}

wchar_t* _tcsstr_cc(wchar_t* str, wchar_t* strSearch, BOOL cc)
{
	wchar_t *ret = NULL, *_str = NULL, *_strSearch = NULL;

	if (cc)
		return wcsstr(str, strSearch);

	_str = mir_wstrdup(str);
	if (_str == NULL)
		goto err_out;
	_strSearch = mir_wstrdup(strSearch);
	if (_strSearch == NULL)
		goto err_out;
	ret = wcsstr(_tcstolower(_str), _tcstolower(_strSearch));
	if (ret != NULL)
		ret = ((ret - _str) + str);
err_out:
	mir_free(_str);
	mir_free(_strSearch);

	return ret;
}

BOOL Contains(wchar_t* dst, wchar_t* src) // Checks for occurence of substring from src in dst
{
	BOOL ret = FALSE;
	wchar_t *tsrc = NULL, *tdst = NULL, *token, *token_end;
	SIZE_T dst_len;

	if (dst == NULL || src == NULL)
		return FALSE;
	tsrc = mir_wstrdup(src);
	if (tsrc == NULL)
		goto err_out;
	tdst = mir_wstrdup(dst);
	if (tdst == NULL)
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
		token = wcstok(NULL, L","); /* Move next. */
	}
err_out:
	mir_free(tsrc);
	mir_free(tdst);
	return ret;
}

BOOL isOneDay(DWORD timestamp1, DWORD timestamp2)
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

	if (dst == NULL || var == NULL || rvar == NULL)
		return NULL;
	dst_len = mir_wstrlen(dst);
	var_len = mir_wstrlen(var);
	rvar_len = mir_wstrlen(rvar);
	var_start = wcsstr(dst, var);
	while (var_start) {
		if (len < (dst_len + rvar_len - var_len + 1))
			return NULL; /* Out of buf space. */
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
	wchar_t *_str, *tmp, **r = NULL, **tr, *ttmp, *dstcopy;
	wchar_t regex[] = L"%response([#-_]([0-9]+))?%";
	int ovector[9];

	re = pcre16_compile(regex, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE; // [TODO] and log some error
	_getOptS(response, _countof(response), "Response", defaultResponse);	

	ttmp = wcstok(response, L"\r\n");
	for (i = 0; ttmp != NULL; i ++) {
		tr = (wchar_t**)mir_realloc(r, ((i + 1) * sizeof(wchar_t*)));
		if (tr == NULL)
			goto err_out;
		r = tr;
		r[i] = ttmp;
		ttmp = wcstok(NULL, L"\r\n"); /* Move next. */
	}

	do {
		_str = mir_wstrdup(dst);
		dstcopy = mir_wstrdup(dst);
		if (_str == NULL || dstcopy == NULL) {
			mir_free(_str);
			mir_free(dstcopy);
			goto err_out;
		}
		rc = pcre16_exec(re, NULL, _str, (int)mir_wstrlen(_str), 0, 0, ovector, 9);
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

int _notify(MCONTACT hContact, BYTE type, wchar_t *message, wchar_t *origmessage)
{
	char *tmp, *tmporig;
	wchar_t msg[MAX_BUFFER_LENGTH];
	mir_snwprintf(msg, message, pcli->pfnGetContactDisplayName(hContact, 0));

	if (_getOptB("LogActions", defaultLogActions)) {
		tmp = mir_u2a(msg);
		tmporig = mir_u2a(origmessage);
		LogToSystemHistory(tmp, origmessage ? tmporig : NULL);
		mir_free(tmp);
		mir_free(tmporig);
	}

	if (_NOTIFYP) {
		if (type == POPUP_BLOCKED) {
			if (_getOptB("NotifyPopupBlocked", defaultNotifyPopupBlocked))
				ShowPopup(hContact, type, NULL, msg);
		} else if (type == POPUP_APPROVED) {
			if (_getOptB("NotifyPopupApproved", defaultNotifyPopupApproved))
				ShowPopup(hContact, type, NULL, msg);
		} else if (type == POPUP_CHALLENGE) {
			if (_getOptB("NotifyPopupChallenge", defaultNotifyPopupChallenge))
				ShowPopup(hContact, type, NULL, msg);
		} else {
			ShowPopup(hContact, type, NULL, msg);
		}
	}
	return 0;
}
#define DOT(a) (a[mir_strlen(a)-1] == 46) ? "" : "."
int LogToSystemHistory(char *message, char *origmessage)
{
	char msg[MAX_BUFFER_LENGTH];
	time_t tm;

	if (message == NULL)
		return 0;

	DBEVENTINFO dbei = {};
	dbei.timestamp = time(&tm);
	dbei.szModule = PLUGIN_NAME;
	dbei.pBlob = (PBYTE)msg;
	if (origmessage)
		dbei.cbBlob = (1 + mir_snprintf(msg, "%s: %s%s %s: %s", PLUGIN_NAME, message, DOT(message), Translate("Their message was"), origmessage));
	else 
		dbei.cbBlob = (1 + mir_snprintf(msg, "%s: %s%s", PLUGIN_NAME, message, DOT(message)));
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_READ;
	db_event_add(NULL, &dbei);
	return 0;
}

void MarkUnread(MCONTACT hContact)
{
	// We're not actually marking anything. We just pushing saved events to the database from a temporary location
	DBVARIANT _dbv = {0};
	PBYTE pos;
	
	if (hContact == NULL)
		return;
	
	if (db_get(hContact, PLUGIN_NAME, "LastMsgEvents", &_dbv) == 0) {
		pos = _dbv.pbVal;
		while (pos - _dbv.pbVal < _dbv.cpbVal) {
			DBEVENTINFO _dbei = {};
			memcpy(&_dbei.eventType, pos, sizeof(WORD)); pos += sizeof(WORD);
			memcpy(&_dbei.flags, pos, sizeof(DWORD)); pos += sizeof(DWORD);
			memcpy(&_dbei.timestamp, pos, sizeof(DWORD)); pos += sizeof(DWORD);

			_dbei.szModule = (char*)malloc(mir_strlen((const char*)pos)+1);
			mir_strcpy(_dbei.szModule, (const char*)pos);
			pos += mir_strlen((const char*)pos)+1;

			memcpy(&_dbei.cbBlob, pos, sizeof(DWORD)); pos += sizeof(DWORD);
			_dbei.pBlob = (PBYTE)malloc(_dbei.cbBlob);
			memcpy(_dbei.pBlob, pos, _dbei.cbBlob);
			pos += _dbei.cbBlob;

			db_event_add(hContact,&_dbei);
		}
		db_free(&_dbv);
		db_unset(hContact, PLUGIN_NAME, "LastMsgEvents");
	}
}
