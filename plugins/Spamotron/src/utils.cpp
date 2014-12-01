#include "common.h"
#include <locale.h>


TCHAR *_tcstolower(TCHAR *dst)
{
	if (dst == NULL)
		return NULL;
	setlocale(LC_ALL, "");
	SIZE_T dst_len = _tcslen(dst);
	for (SIZE_T i = 0; i < dst_len; i ++)
		dst[i] = _totlower(dst[i]);
	return dst;
}

TCHAR *_tcstoupper(TCHAR *dst)
{
	if (dst == NULL)
		return NULL;
	setlocale(LC_ALL, "");
	SIZE_T dst_len = _tcslen(dst);
	for (SIZE_T i = 0; i < dst_len; i ++)
		dst[i] = _totupper(dst[i]);
	return dst;
}

BOOL _isregex(TCHAR* strSearch)
{
	BOOL ret = FALSE;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *regex;
	TCHAR regex_parse[] = _T("/(.*)/([igsm]*)");
	int ovector[9];

	if (strSearch == NULL)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE;
	regex = mir_tstrdup(strSearch);
	if (regex == NULL)
		goto err_out;
	rc = pcre16_exec(re, NULL, regex, _tcslen(regex), 0, 0, ovector, 9);
	if (rc == 3)
		ret = TRUE;
	mir_free(regex);
err_out:
	pcre16_free(re);

	return ret;
}

BOOL _isvalidregex(TCHAR* strSearch)
{
	BOOL ret = FALSE;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *regex, *regexp, *mod;
	int opts = 0;
	TCHAR regex_parse[] = _T("/(.*)/([igsm]*)");
	int ovector[9];

	if (strSearch == NULL)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE;
	regex = mir_tstrdup(strSearch);
	if (regex == NULL) {
		pcre16_free(re);
		return FALSE;
	}
	rc = pcre16_exec(re, NULL, regex, _tcslen(regex), 0, 0, ovector, 9);
	pcre16_free(re);
	if (rc != 3)
		goto err_out;
	regexp = regex + ovector[2];
	regexp[ovector[3] - ovector[2]] = 0;
	mod = regex + ovector[4];
	mod[ovector[5] - ovector[4]] = 0;

	if (_tcsstr(mod, _T("i")))
		opts |= PCRE_CASELESS;
	if (_tcsstr(mod, _T("m")))
		opts |= PCRE_MULTILINE;
	if (_tcsstr(mod, _T("s")))
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

BOOL _regmatch(TCHAR* str, TCHAR* strSearch)
{
	BOOL ret = FALSE;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *regex, *regexp, *data = NULL, *mod;
	int opts = 0;
	TCHAR regex_parse[] = _T("^/(.*)/([igsm]*)");
	int ovector[9];

	if (str == NULL || strSearch == NULL)
		return FALSE;
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE; // [TODO] and log some error
	regex = mir_tstrdup(strSearch);
	if (regex == NULL) {
		pcre16_free(re);
		return FALSE;
	}
	rc = pcre16_exec(re, NULL, regex, _tcslen(regex), 0, 0, ovector, 9);
	pcre16_free(re);
	if (rc != 3)
		goto err_out; // [TODO] and log some error (better check for valid regex on options save)
	regexp = regex + ovector[2];
	regexp[ovector[3] - ovector[2]] = 0;
	mod = regex + ovector[4];
	mod[ovector[5] - ovector[4]] = 0;

	data = mir_tstrdup(str);
	if (data == NULL)
		goto err_out;
	if (_tcsstr(mod, _T("i")))
		opts |= PCRE_CASELESS;
	if (_tcsstr(mod, _T("m")))
		opts |= PCRE_MULTILINE;
	if (_tcsstr(mod, _T("s")))
		opts |= PCRE_DOTALL;

	re = pcre16_compile(regexp, opts, &error, &erroroffs, NULL);
	if (re == NULL)
		goto err_out;
	rc = pcre16_exec(re, NULL, data, _tcslen(data), 0, 0, NULL, 0);
	pcre16_free(re);
	if (rc >= 0)
		ret = TRUE;
err_out:
	mir_free(regex);
	mir_free(data);

	return ret;
}

int get_response_id(const TCHAR* strvar)
{
	int ret = 0;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *_str, *_strvar;
	int opts = 0;
	TCHAR regex[] = _T("^%response([#-_]([0-9]+))?%$");
	int ovector[9];

	if (strvar == NULL)
		return 0;
	re = pcre16_compile(regex, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return 0; // [TODO] and log some error
	_strvar = mir_tstrdup(strvar);
	if (_strvar == NULL) {
		pcre16_free(re);
		return 0;
	}
	rc = pcre16_exec(re, NULL, _strvar, _tcslen(_strvar), 0, 0, ovector, 9);
	pcre16_free(re);
	if (rc < 0) {
		ret = -1;
	} else if (rc == 3) {
		_str = _strvar + ovector[4];
		_str[ovector[5] - ovector[4]] = 0;
		ret = _ttoi(_str);
	}
	mir_free(_strvar);

	return ret;
}

int get_response_num(const TCHAR *str)
{
	int i = 0;
	TCHAR *tmp, *strc;
	
	if (str == NULL)
		return 0;
	strc = mir_tstrdup(str);
	if (strc == NULL)
		return 0;
	tmp = _tcstok(strc, _T("\r\n"));
	while (tmp) {
		i ++;
		tmp = _tcstok(NULL, _T("\r\n")); /* Move next. */
	}
	mir_free(strc);

	return i;
}

TCHAR* get_response(TCHAR* dst, unsigned int dstlen, int num)
{
	int i = 0;
	TCHAR *tmp, *src;

	if (dst == NULL || dstlen == 0 || num < 0)
		return dst;
	src = (TCHAR*)mir_alloc(MAX_BUFFER_LENGTH * sizeof(TCHAR));
	if (src == NULL)
		goto err_out;
	_getOptS(src, MAX_BUFFER_LENGTH, "Response", defaultResponse);
	tmp = _tcstok(src, _T("\r\n"));
	while (tmp) {
		if (i == num) {
			_tcscpy(dst, tmp);
			mir_free(src);
			return dst;
		}
		i ++;
		tmp = _tcstok(NULL, _T("\r\n")); /* Move next. */
	}
	mir_free(src);
err_out:
	dst[0] = 0;
	return dst;
}

TCHAR* _tcsstr_cc(TCHAR* str, TCHAR* strSearch, BOOL cc)
{
	TCHAR *ret = NULL, *_str = NULL, *_strSearch = NULL;

	if (cc)
		return _tcsstr(str, strSearch);

	_str = mir_tstrdup(str);
	if (_str == NULL)
		goto err_out;
	_strSearch = mir_tstrdup(strSearch);
	if (_strSearch == NULL)
		goto err_out;
	ret = _tcsstr(_tcstolower(_str), _tcstolower(_strSearch));
	if (ret != NULL)
		ret = ((ret - _str) + str);
err_out:
	mir_free(_str);
	mir_free(_strSearch);

	return ret;
}

BOOL Contains(TCHAR* dst, TCHAR* src) // Checks for occurence of substring from src in dst
{
	BOOL ret = FALSE;
	TCHAR *tsrc = NULL, *tdst = NULL, *token, *token_end;
	SIZE_T dst_len;

	if (dst == NULL || src == NULL)
		return FALSE;
	tsrc = mir_tstrdup(src);
	if (tsrc == NULL)
		goto err_out;
	tdst = mir_tstrdup(dst);
	if (tdst == NULL)
		goto err_out;
	tdst = _tcstoupper(tdst);
	dst_len = _tcslen(tdst);
	token = _tcstok(tsrc, _T(","));
	while (token) {
		token_end = (token + _tcslen(token));
		while (!_tcsncmp(token, _T(" "), 1)) { /* Skeep spaces at start. */
			token ++;
		}
		/* Skeep spaces at end. */
		while (token > token_end && _tcschr((token_end - 1), _T(' '))) {
			token_end --;
			token_end[0] = 0;
		}
		/* Compare. */
		if (dst_len == (token_end - token) &&
		    0 == memcmp(tdst, _tcstoupper(token), (dst_len * sizeof(TCHAR)))) {
			ret = TRUE;
			break;		
		}
		token = _tcstok(NULL, _T(",")); /* Move next. */
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

TCHAR* ReplaceVar(TCHAR *dst, unsigned int len, const TCHAR *var, const TCHAR *rvar)
{
	TCHAR *var_start;
	SIZE_T dst_len, var_len, rvar_len;

	if (dst == NULL || var == NULL || rvar == NULL)
		return NULL;
	dst_len = _tcslen(dst);
	var_len = _tcslen(var);
	rvar_len = _tcslen(rvar);
	var_start = _tcsstr(dst, var);
	while (var_start) {
		if (len < (dst_len + rvar_len - var_len + 1))
			return NULL; /* Out of buf space. */
		memmove((var_start + rvar_len),
		    (var_start + var_len),
		    (((dst + dst_len + 1) - (var_start + var_len)) * sizeof(TCHAR)));
		if (var_len >= rvar_len) { /* Buf data size not changed or decreased. */
			dst_len -= (var_len - rvar_len);
		} else { /* Buf data size increased. */
			dst_len += (rvar_len - var_len);
		}
		memcpy(var_start, rvar, (rvar_len * sizeof(TCHAR)));
		var_start = _tcsstr(dst, var); /* Move next. */
	}

	return dst;
}

TCHAR* ReplaceVars(TCHAR *dst, unsigned int len)
{
	return ReplaceVarsNum(dst, len, -1);
}

TCHAR* ReplaceVarsNum(TCHAR *dst, unsigned int len, int num)
{
	TCHAR response[MAX_BUFFER_LENGTH];
	int ret, i;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *_str, *tmp, **r = NULL, **tr, *ttmp, *dstcopy;
	TCHAR regex[] = _T("%response([#-_]([0-9]+))?%");
	int ovector[9];

	re = pcre16_compile(regex, 0, &error, &erroroffs, NULL);
	if (re == NULL)
		return FALSE; // [TODO] and log some error
	_getOptS(response, SIZEOF(response), "Response", defaultResponse);	

	ttmp = _tcstok(response, _T("\r\n"));
	for (i = 0; ttmp != NULL; i ++) {
		tr = (TCHAR**)mir_realloc(r, ((i + 1) * sizeof(TCHAR*)));
		if (tr == NULL)
			goto err_out;
		r = tr;
		r[i] = ttmp;
		ttmp = _tcstok(NULL, _T("\r\n")); /* Move next. */
	}

	do {
		_str = mir_tstrdup(dst);
		dstcopy = mir_tstrdup(dst);
		if (_str == NULL || dstcopy == NULL) {
			mir_free(_str);
			mir_free(dstcopy);
			goto err_out;
		}
		rc = pcre16_exec(re, NULL, _str, _tcslen(_str), 0, 0, ovector, 9);
		if (rc < 0) {
			ret = -1;
		} else if (rc == 3) {
			ttmp = dstcopy + ovector[0];
			ttmp[ovector[1] - ovector[0]] = 0;
			tmp = _str + ovector[4];
			tmp[ovector[5] - ovector[4]] = 0;
			ret = _ttoi(tmp);
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

int _notify(MCONTACT hContact, BYTE type, TCHAR *message, TCHAR *origmessage)
{
	char *tmp, *tmporig;
	TCHAR msg[MAX_BUFFER_LENGTH];
	mir_sntprintf(msg, MAX_BUFFER_LENGTH, message, CONTACT_NAME(hContact));

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
#define DOT(a) (a[strlen(a)-1] == 46) ? "" : "."
int LogToSystemHistory(char *message, char *origmessage)
{
	char msg[MAX_BUFFER_LENGTH];
	time_t tm;
	DBEVENTINFO dbei;

	if (message == NULL)
		return 0;
	dbei.cbSize = sizeof(DBEVENTINFO);
	dbei.timestamp = time(&tm);
	dbei.szModule = PLUGIN_NAME;
	dbei.pBlob = (PBYTE)msg;
	if (origmessage)
		dbei.cbBlob = (1 + mir_snprintf(msg, MAX_BUFFER_LENGTH, "%s: %s%s %s: %s", PLUGIN_NAME, message, DOT(message), Translate("Their message was"), origmessage));
	else 
		dbei.cbBlob = (1 + mir_snprintf(msg, MAX_BUFFER_LENGTH, "%s: %s%s", PLUGIN_NAME, message, DOT(message)));
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
			DBEVENTINFO _dbei;
			memset(&_dbei, 0, sizeof(_dbei));
			_dbei.cbSize = sizeof(_dbei);

			memcpy(&_dbei.eventType, pos, sizeof(WORD)); pos += sizeof(WORD);
			memcpy(&_dbei.flags, pos, sizeof(DWORD)); pos += sizeof(DWORD);
			memcpy(&_dbei.timestamp, pos, sizeof(DWORD)); pos += sizeof(DWORD);

			_dbei.szModule = (char*)malloc(strlen((const char*)pos)+1);
			strcpy(_dbei.szModule, (const char*)pos);
			pos += strlen((const char*)pos)+1;

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
