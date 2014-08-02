#include "common.h"
#include <locale.h>

TCHAR *_tcstolower(TCHAR *dst)
{
	unsigned int i = 0;
	setlocale(LC_ALL, "");
	if (!dst)
		return NULL;
	for (i = 0; i < _tcslen(dst); i++)
		dst[i] = _totlower(dst[i]);
	return dst;
}

TCHAR *_tcstoupper(TCHAR *dst)
{
	unsigned int i = 0;
	setlocale(LC_ALL, "");
	if (!dst)
		return NULL;
	for (i = 0; i < _tcslen(dst); i++)
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
	
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (!re)
		return FALSE;
	
	regex = mir_tstrdup(strSearch);
	rc = pcre16_exec(re, NULL, regex, (int)lstrlen(regex), 0, 0, ovector, 9);
	if (rc == 3)
		ret = TRUE;
	
	if (re)
		pcre16_free(re);
	if (regex)
		mir_free(regex);
	
	return ret;
}

BOOL _isvalidregex(TCHAR* strSearch)
{
	BOOL ret;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *regex, *regexp, *mod;
	int opts = 0;
	TCHAR regex_parse[] = _T("/(.*)/([igsm]*)");
	int ovector[9];
	
	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (!re)
		return FALSE;

	regex = mir_tstrdup(strSearch);
	rc = pcre16_exec(re, NULL, regex, (int)lstrlen(regex), 0, 0, ovector, 9);
	pcre16_free(re);

	if (rc == 3) {
		regexp = regex + ovector[2];
		regexp[ovector[3]-ovector[2]] = 0;
		mod = regex + ovector[4];
		mod[ovector[5]-ovector[4]] = 0;

		if (_tcsstr(mod, _T("i")))
			opts |= PCRE_CASELESS;
		if (_tcsstr(mod, _T("m")))
			opts |= PCRE_MULTILINE;
		if (_tcsstr(mod, _T("s")))
			opts |= PCRE_DOTALL;

		re = pcre16_compile(regexp, opts, &error, &erroroffs, NULL);
		ret = (re) ? TRUE : FALSE;
	}

	if (re)
		pcre16_free(re);
	if (regex)
		mir_free(regex);
	
	return ret;
}

BOOL _regmatch(TCHAR* str, TCHAR* strSearch)
{
	BOOL ret;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *regex, *regexp, *data, *mod;
	int opts = 0;
	TCHAR regex_parse[] = _T("^/(.*)/([igsm]*)");
	int ovector[9];

	re = pcre16_compile(regex_parse, 0, &error, &erroroffs, NULL);
	if (!re)
		return FALSE; // [TODO] and log some error

	regex = mir_tstrdup(strSearch);
	rc = pcre16_exec(re, NULL, regex, (int)lstrlen(regex), 0, 0, ovector, 9);
	if (rc != 3) {
		mir_free(regex);
		return FALSE; // [TODO] and log some error (better check for valid regex on options save)
	}

	regexp = regex + ovector[2];
	regexp[ovector[3]-ovector[2]] = 0;
	mod = regex + ovector[4];
	mod[ovector[5]-ovector[4]] = 0;
	pcre16_free(re);

	data = mir_tstrdup(str);

	if (_tcsstr(mod, _T("i")))
		opts |= PCRE_CASELESS;
	if (_tcsstr(mod, _T("m")))
		opts |= PCRE_MULTILINE;
	if (_tcsstr(mod, _T("s")))
		opts |= PCRE_DOTALL;

	re = pcre16_compile(regexp, opts, &error, &erroroffs, NULL);
	if (!re) {
		mir_free(regex);
		mir_free(data);
		return FALSE;
	}
	
	rc = pcre16_exec(re, NULL, data, (int)lstrlen(data), 0, 0, NULL, 0);
	if (rc < 0) {
		ret = FALSE;
	} else {
		ret = TRUE;
	}

	if (re)
		pcre16_free(re);
	if (regex)
		mir_free(regex);
	if (data)
		mir_free(data);
	
	return ret;
}

int get_response_id(const TCHAR* strvar)
{
	int ret;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *_str, *_strvar;
	int opts = 0;
	TCHAR regex[] = _T("^%response([#-_]([0-9]+))?%$");
	int ovector[9];

	re = pcre16_compile(regex, 0, &error, &erroroffs, NULL);
	if (!re)
		return FALSE; // [TODO] and log some error
	
	_strvar = mir_tstrdup(strvar);
	rc = pcre16_exec(re, NULL, _strvar, (int)lstrlen(_strvar), 0, 0, ovector, 9);
	if (rc < 0) {
		ret = -1;
	} else if (rc == 3) {
		_str = _strvar + ovector[4];
		_str[ovector[5]-ovector[4]] = 0;
		ret = _ttoi(_str);
	} else
		ret = 0;

	if (re)
		pcre16_free(re);
	if (_strvar)
		mir_free(_strvar);
	
	return ret;
}

int get_response_num(const TCHAR *str)
{
	int i = 0;
	TCHAR *tmp, *strc = NULL;
	strc = (TCHAR*)malloc((_tcslen(str)+1)*sizeof(TCHAR));
	if (strc != NULL) {
		_tcscpy(strc, str);
		tmp = _tcstok(strc, L"\r\n");
		while(tmp) {
			i++;
			tmp = _tcstok(NULL, L"\r\n");
		}
		free(strc);
	}
	return i;
}

TCHAR* get_response(TCHAR* dst, unsigned int dstlen, int num)
{
	int i = 0;
	TCHAR *tmp, *src = NULL;
	if (num < 0)
		return dst;
	src = (TCHAR*)malloc(MAX_BUFFER_LENGTH * sizeof(TCHAR));
	if (src != NULL) {
		_getOptS(src, MAX_BUFFER_LENGTH, "Response", defaultResponse);
		_tcscpy(src, src);
		tmp = _tcstok(src, L"\r\n");
		while (tmp) {
			if (i == num) {
				_tcscpy(dst, tmp);
				free(src);
				return dst;
			}
			i++;
			tmp = _tcstok(NULL, L"\r\n");
		}
		free(src);
	}
	return dst;
}

TCHAR* _tcsstr_cc(TCHAR* str, TCHAR* strSearch, BOOL cc)
{
	if (cc)
		return _tcsstr(str, strSearch);
	else {
		TCHAR *ret;
		TCHAR *_str = (TCHAR*)malloc((_tcslen(str)+1)*sizeof(TCHAR));
		TCHAR *_strSearch = (TCHAR*)malloc((_tcslen(strSearch)+1)*sizeof(TCHAR));
		_tcscpy(_str, str);
		_tcscpy(_strSearch, strSearch);
		ret = _tcsstr(_tcstolower(_str), _tcstolower(_strSearch));
		if (ret != NULL)
			ret = (ret-_str) + str;
		free(_str);
		free(_strSearch);
		return ret;
	}
}

BOOL Contains(TCHAR* dst, TCHAR* src) // Checks for occurence of substring from src in dst
{
	int i = 0, n = 0, value = 0;
	TCHAR *tsrc = (TCHAR *)malloc((_tcslen(src)+1)*sizeof(TCHAR));
	TCHAR *tdst = (TCHAR *)malloc((_tcslen(dst)+1)*sizeof(TCHAR));
	TCHAR **tokens = NULL;
	TCHAR *token = NULL;
	_tcscpy(tdst, dst);
	_tcscpy(tsrc, src);
	token = _tcstok(tsrc, _T(","));
	while (token) {
		tokens = (TCHAR **) realloc(tokens, (n+1)*sizeof(TCHAR *));
		tokens[n] = (TCHAR *)malloc((_tcslen(token)+1)*sizeof(TCHAR));
		while(!_tcsncmp(token, _T(" "), 1)) {++token;}
		while(_tcschr(token+_tcslen(token)-1, _T(' '))) { 
			token[_tcslen(token)-1] = _T('\0'); 
		}
		_tcscpy(tokens[n], token);
		token = _tcstok(NULL, _T(","));
		++n;
	}
	for (i = 0; i < n; i++) {
		
		if (_tcsstr(_tcstoupper(tdst), _tcstoupper(tokens[i]))) {
			value = 1;
			break;
		}
	}
	free(tsrc);
	if (tokens) {
		for(i = 0; i < n; i++)
			free(tokens[i]);
		free(tokens);
	}
	return value;
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
	TCHAR *_rvar = (TCHAR*)malloc((_tcslen(rvar)+1)*sizeof(TCHAR));
	TCHAR *__rvar = NULL;
	TCHAR *var_start, *tmp = NULL;
	TCHAR *src = NULL;
	int response_id = get_response_id(var);
	
	src = (TCHAR*)malloc((_tcslen(dst)+1)*sizeof(TCHAR));
	_tcscpy(_rvar, rvar);
	__rvar = _rvar;
/*
	if (response_id >= 0) {
		__rvar = _tcstok(_rvar, L"\r\n");
		while(response_id--) {
			__rvar = _tcstok(NULL, L"\r\n");
		}
	} else {
		__rvar = _rvar;
	}
*/	
	if (!dst)
		return NULL;
	_tcscpy(src, dst);
	var_start = _tcsstr(dst, var);
	while (var_start) {
		tmp = (TCHAR*)realloc(tmp, sizeof(TCHAR)*(_tcslen(dst) + _tcslen(__rvar) - _tcslen(var) + 1));
		_tcsncpy(tmp, dst, var_start - dst);
		tmp[var_start - dst] = 0;
		_tcscat(tmp, __rvar);
		_tcscat(tmp, var_start + _tcslen(var));
		if (len < (_tcslen(src)+1)*sizeof(TCHAR)) {
			free(tmp);
			return NULL;
		}
		_tcscpy(dst, tmp);
		var_start = _tcsstr(dst, var);
	}
	if (tmp)
		free(tmp);
	if (_rvar)
		free(_rvar);
	if (src)
		free(src);
	return dst;
}

TCHAR* ReplaceVars(TCHAR *dst, unsigned int len)
{
	return ReplaceVarsNum(dst, len, -1);
}

TCHAR* ReplaceVarsNum(TCHAR *dst, unsigned int len, int num)
{
	TCHAR response[2048];
	int ret, i = 1;
	pcre16 *re;
	const char *error;
	int erroroffs, rc;
	TCHAR *_str, *tmp;
	TCHAR **r;
	TCHAR *ttmp, *dstcopy;
	int opts = 0;
	TCHAR regex[] = _T("%response([#-_]([0-9]+))?%");
	int ovector[9];

	re = pcre16_compile(regex, 0, &error, &erroroffs, NULL);
	if (!re)
		return FALSE; // [TODO] and log some error
	
	_getOptS(response, 2048, "Response", defaultResponse);	

	r = (TCHAR**)malloc(sizeof(char*));
	ttmp = _tcstok(response, L"\r\n");
	r[0] = ttmp;
	while(ttmp) {
		ttmp = _tcstok(NULL, L"\r\n");
		if (ttmp != NULL) {
			r = (TCHAR**)realloc(r, (i+1)*sizeof(TCHAR*));
			r[i++] = ttmp;
		}
	}

	do {
		_str = mir_tstrdup(dst);
		dstcopy = (TCHAR*)malloc((_tcslen(dst)+1)*sizeof(TCHAR));
		_tcscpy(dstcopy, dst);
		
		rc = pcre16_exec(re, NULL, _str, (int)lstrlen(_str), 0, 0, ovector, 9);
		if (rc < 0) {
			ret = -1;
		} else if (rc == 3) {
			ttmp = dstcopy + ovector[0];
			ttmp[ovector[1]-ovector[0]] = 0;
			tmp = _str + ovector[4];
			tmp[ovector[5]-ovector[4]] = 0;
			ret = _ttoi(tmp);
		} else {
			ttmp = dstcopy + ovector[0];
			ttmp[ovector[1]-ovector[0]] = 0;
			ret = 0;
		}

		if (ret >= 0) {
			if (ret > i)
				ReplaceVar(dst, len, ttmp, r[0]);
			else if (ret == 0 && num > 0 && num < i)
				ReplaceVar(dst, len, ttmp, r[num]);
			else
				ReplaceVar(dst, len, ttmp, r[ret == 0 ? 0 : ret-1]);
		}

		if (_str)
			mir_free(_str);
		if (dstcopy)
			free(dstcopy);
	} while (rc >= 0);

	if (re)
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
	char *msg = (char*)malloc(MAX_BUFFER_LENGTH);
	time_t tm;
	DBEVENTINFO dbei;
	dbei.cbSize = sizeof(DBEVENTINFO);
	dbei.timestamp = time(&tm);
	dbei.szModule = PLUGIN_NAME;
	if (origmessage)
		mir_snprintf(msg, MAX_BUFFER_LENGTH, "%s: %s%s %s: %s", PLUGIN_NAME, message, DOT(message), Translate("Their message was"), origmessage);
	else 
		mir_snprintf(msg, MAX_BUFFER_LENGTH, "%s: %s%s", PLUGIN_NAME, message, DOT(message));
	dbei.pBlob = (PBYTE)msg;
	dbei.cbBlob = (DWORD)strlen(msg)+1;
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
			ZeroMemory(&_dbei, sizeof(_dbei));
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
