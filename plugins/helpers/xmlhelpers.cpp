#include "commonheaders.h"
#include "gen_helpers.h"

char *XMLEncodeString(TCHAR *tszSrc) {

	int cur, len;
	TCHAR *tszEsc, *tszRep;
	char *szRes;

	if (tszSrc == NULL) {
		return NULL;
	}
	tszEsc = _tcsdup(tszSrc);
	cur = len = 0;
	while (tszEsc[cur] != _T('\0')) {
		tszRep = NULL;
		switch (tszEsc[cur]) {
		case _T('<'):
			len = 1;
			tszRep = _T("&lt;");
			break;
		case _T('>'):
			len = 1;
			tszRep = _T("&gt;");
			break;
		case _T('&'):
			len = 1;
			tszRep = _T("&amp;");
			break;
		case _T('\''):
			len = 1;
			tszRep = _T("&apos;");
			break;
		case _T('"'):
			len = 1;
			tszRep = _T("&quot;");
			break;
		case _T('\r'):
			if (tszEsc[cur+1] == _T('\n')) {
				len = 2;
				tszRep = _T("&#xA;");
			}
			break;
		case _T('\n'):
			len = 1;
			tszRep = _T("&#xA;");
			break;
		}
		// tszRep > len !!!
		if (tszRep != NULL) {
			tszEsc = realloc(tszEsc, (_tcslen(tszEsc) + _tcslen(tszRep) - len + 1)*sizeof(TCHAR));
			MoveMemory(&tszEsc[cur+_tcslen(tszRep) - len], &tszEsc[cur], _tcslen(&tszEsc[cur]) + 1);
			CopyMemory(&tszEsc[cur], tszRep, _tcslen(tszRep)*sizeof(TCHAR));
			cur += _tcslen(tszRep);
		}
		else {
			cur++;
		}
	}
#ifdef UNICODE
	szRes = Utf8EncodeUcs2(tszEsc);
#else
	szRes = Utf8Encode(tszEsc);
#endif
	free(tszEsc);

	return szRes;		
}

char *XMLEncodeStringA(char *szSrc) {

	int cur, len;
	char *szEsc, *szRep;

	if (szSrc == NULL) {
		return NULL;
	}
	szEsc = _strdup(szSrc);
	cur = len = 0;
	while (szEsc[cur] != '\0') {
		szRep = NULL;
		switch (szEsc[cur]) {
		case '<':
			len = 1;
			szRep = "&lt;";
			break;
		case '>':
			len = 1;
			szRep = "&gt;";
			break;
		case '&':
			len = 1;
			szRep = "&amp;";
			break;
		case '\'':
			len = 1;
			szRep = "&apos;";
			break;
		case '"':
			len = 1;
			szRep = "&quot;";
			break;
		case '\r':
			if (szEsc[cur+1] == '\n') {
				len = 2;
				szRep = "&#xA;";
			}
			break;
		case '\n':
			len = 1;
			szRep = "&#xA;";
			break;
		}
		if (szRep != NULL) {
			szEsc = realloc(szEsc, (strlen(szEsc) + strlen(szRep) - len + 1));
			MoveMemory(&szEsc[cur+strlen(szRep) - len], &szEsc[cur], strlen(&szEsc[cur]) + 1);
			CopyMemory(&szEsc[cur], szRep, strlen(szRep));
			cur+=strlen(szRep);
		}
		else {
			cur++;
		}
	}

	return szEsc;
}

TCHAR *DecodeXMLString(char *szSrc) {

	int cur;
	TCHAR *tszEsc, *tszRep, *tszTrans;
	char *szEsc;
#ifdef UNICODE
	wchar_t *wszEsc;
#endif

	if (szSrc == NULL) {
		return NULL;
	}
	szEsc = _strdup(szSrc);
#ifdef UNICODE
	Utf8Decode(szEsc, &wszEsc);
	tszEsc = wszEsc;
	free(szEsc);
#else
	Utf8Decode(szEsc, NULL);
	tszEsc = szEsc;
#endif
	cur= 0;
	while (tszEsc[cur] != _T('\0')) {
		tszRep = NULL;
		if (!_tcsncmp(&tszEsc[cur], _T("&lt;"), 4)) {
			tszRep = _T("&lt;");
			tszTrans = _T("<");
		}
		else if (!_tcsncmp(&tszEsc[cur], _T("&gt;"), 4)) {
			tszRep = _T("&gt;");
			tszTrans = _T(">");
		}
		else if (!_tcsncmp(&tszEsc[cur], _T("&amp;"), 5)) {
			tszRep = _T("&amp;");
			tszTrans = _T("&");
		}
		else if (!_tcsncmp(&tszEsc[cur], _T("&apos;"), 6)) {
			tszRep = _T("&apos;");
			tszTrans = _T("\'");
		}
		else if (!_tcsncmp(&tszEsc[cur], _T("&quot;"), 6)) {
			tszRep = _T("&quot;");
			tszTrans = _T("&");
		}
		else if (!_tcsncmp(&tszEsc[cur], _T("&#xA;"), 5)) {
			tszRep = _T("&#xA;");
			tszTrans = _T("\r\n");
		}
		if (tszRep != NULL) {
			tszEsc = realloc(tszEsc, (_tcslen(tszEsc) - _tcslen(tszRep) + _tcslen(tszTrans) + 1)*sizeof(TCHAR));
			MoveMemory(&tszEsc[cur+_tcslen(tszTrans)], &tszEsc[cur+_tcslen(tszRep)], _tcslen(&tszEsc[cur+_tcslen(tszRep)]) + 1);
			CopyMemory(&tszEsc[cur], tszTrans, _tcslen(tszTrans)*sizeof(TCHAR));
			cur += _tcslen(tszTrans);
		}
		else {
			cur++;
		}
	}

	return tszEsc;
}

char *DecodeXMLStringA(char *szSrc, BOOL bUtfDecode) {

	int cur;
	char *szEsc, *szRep, *szTrans;

	if (szSrc == NULL) {
		return NULL;
	}
	szEsc = _strdup(szSrc);
	if (bUtfDecode) {
		Utf8Decode(szEsc, NULL);
	}
	cur= 0;
	while (szEsc[cur] != '\0') {
		szRep = NULL;
		if (!strncmp(&szEsc[cur], "&lt;", 4)) {
			szRep = "&lt;";
			szTrans = "<";
		}
		else if (!strncmp(&szEsc[cur], "&gt;", 4)) {
			szRep = "&gt;";
			szTrans = ">";
		}
		else if (!strncmp(&szEsc[cur], "&amp;", 5)) {
			szRep = "&amp;";
			szTrans = "&";
		}
		else if (!strncmp(&szEsc[cur], "&apos;", 6)) {
			szRep = "&apos;";
			szTrans = "'";
		}
		else if (!strncmp(&szEsc[cur], "&quot;", 6)) {
			szRep = "&quot;";
			szTrans = "&";
		}
		else if (!strncmp(&szEsc[cur], "&#xA;", 5)) {
			szRep = "&#xA;";
			szTrans = "\r\n";
		}
		if (szRep != NULL) {
			szEsc = realloc(szEsc, (strlen(szEsc) - strlen(szRep) + strlen(szTrans) + 1));
			MoveMemory(&szEsc[cur+strlen(szTrans)], &szEsc[cur+strlen(szRep)], strlen(&szEsc[cur+strlen(szRep)]) + 1);
			CopyMemory(&szEsc[cur], szTrans, strlen(szTrans));
			cur += strlen(szTrans);
		}
		else {
			cur++;
		}
	}

	return szEsc;
}

void AppendXMLOpeningTag(char **szBuf, char *szTag, int *depth) {

	char *cur;

	*depth += 1;
	*szBuf = realloc(*szBuf, strlen(*szBuf) + strlen(szTag) + *depth + 4);
	cur = *szBuf+strlen(*szBuf);
	*cur = '\n';
	if (*depth > 0) {
		memset(cur+1, ' ', *depth);
	}
	*(cur+1+*depth) = '\0';
	sprintf(*szBuf+strlen(*szBuf), "<%s>", szTag);
}

void AppendXMLClosingTag(char **szBuf, char *szTag, int *depth) {

	char *cur;

	*depth -= 1;
	*szBuf = realloc(*szBuf, strlen(*szBuf) + strlen(szTag) + *depth + 5);
	cur = *szBuf+strlen(*szBuf);
	*cur = '\n';
	if (*depth > 0) {
		memset(cur+1, ' ', *depth);
	}
	*(cur+1+*depth) = '\0';
	sprintf(*szBuf+strlen(*szBuf), "</%s>", szTag);
}

void AppendXMLTag(char **szBuf, char *szTag, char *szAtts, int *depth) {

	char *cur;

	*depth += 1;
	if (szAtts != NULL) {
		*szBuf = realloc(*szBuf, strlen(*szBuf) + strlen(szTag) + strlen(szAtts) + *depth + 6);
	}
	else {
		*szBuf = realloc(*szBuf, strlen(*szBuf) + strlen(szTag) + *depth + 5);
	}
	cur = *szBuf+strlen(*szBuf);
	*cur = '\n';
	if (*depth > 0) {
		memset(cur+1, ' ', *depth);
	}
	*(cur+1+*depth) = '\0';
	if (szAtts != NULL) {
		sprintf(*szBuf+strlen(*szBuf), "<%s %s/>", szTag, szAtts);
	}
	else {
		sprintf(*szBuf+strlen(*szBuf), "<%s/>", szTag);
	}
	*depth -= 1;
}

char *GetSettingType(BYTE type) {

	switch (type) {
	case DBVT_BYTE:
		return "BYTE";
	case DBVT_WORD:
		return "WORD";
	case DBVT_DWORD:
		return "DWORD";
	case DBVT_ASCIIZ:
		return "ASCIIZ";
	case DBVT_BLOB:
		return "BLOB";
	case DBVT_UTF8:
		return "UTF8";
	case DBVT_WCHAR:
		return "WCHAR";
	}
	return "UNKNOWN";
}

void AppendXMLTagTString(char **szBuf, char *szTag, TCHAR *tszVal, int *depth) {

	char *szVal, *szAtts, *szType;

	szVal = XMLEncodeString(tszVal);
#ifdef UNICODE
	szType = GetSettingType(DBVT_WCHAR);
#else
	szType = GetSettingType(DBVT_ASCIIZ);
#endif
	szAtts = malloc(strlen(szVal) + strlen(szType) + 17);
	sprintf(szAtts, "type=\"%s\" value=\"%s\"", szType, szVal);
	AppendXMLTag(szBuf, szTag, szAtts, depth);
	free(szAtts);
	free(szVal);
}

void AppendXMLTagString(char **szBuf, char *szTag, char *szVal, int *depth) {

	char *szEnc, *szAtts, *szType;

	szEnc = XMLEncodeStringA(szVal);
	szType = GetSettingType(DBVT_ASCIIZ);
	szAtts = malloc(strlen(szEnc) + strlen(szType) + 17);
	sprintf(szAtts, "type=\"%s\" value=\"%s\"", szType, szEnc);
	AppendXMLTag(szBuf, szTag, szAtts, depth);
	free(szAtts);
	free(szEnc);
}

void AppendXMLTagUtfString(char **szBuf, char *szTag, char *szVal, int *depth) {

	char *szEnc, *szAtts, *szType;

	szEnc = XMLEncodeStringA(szVal);
	szType = GetSettingType(DBVT_UTF8);
	szAtts = malloc(strlen(szEnc) + strlen(szType) + 17);
	sprintf(szAtts, "type=\"%s\" value=\"%s\"", szType, szEnc);
	AppendXMLTag(szBuf, szTag, szAtts, depth);
	free(szAtts);
	free(szEnc);
}

void AppendXMLTagByte(char **szBuf, char *szTag, BYTE bVal, int *depth) {

	char szVal[64], *szAtts, *szType;

	mir_snprintf(szVal, sizeof(szVal), "0x%x", bVal);
	szType = GetSettingType(DBVT_BYTE);
	szAtts = malloc(strlen(szVal) + strlen(szType) + 17);
	sprintf(szAtts, "type=\"%s\" value=\"%s\"", szType, szVal);
	AppendXMLTag(szBuf, szTag, szAtts, depth);
	free(szAtts);
}

void AppendXMLTagWord(char **szBuf, char *szTag, WORD wVal, int *depth) {

	char szVal[64], *szAtts, *szType;

	mir_snprintf(szVal, sizeof(szVal), "0x%x", wVal);
	szType = GetSettingType(DBVT_WORD);
	szAtts = malloc(strlen(szVal) + strlen(szType) + 17);
	sprintf(szAtts, "type=\"%s\" value=\"%s\"", szType, szVal);
	AppendXMLTag(szBuf, szTag, szAtts, depth);
	free(szAtts);
}

void AppendXMLTagDword(char **szBuf, char *szTag, DWORD dVal, int *depth) {

	char szVal[64], *szAtts, *szType;

	mir_snprintf(szVal, sizeof(szVal), "0x%x", dVal);
	szType = GetSettingType(DBVT_DWORD);
	szAtts = malloc(strlen(szVal) + strlen(szType) + 17);
	sprintf(szAtts, "type=\"%s\" value=\"%s\"", szType, szVal);
	AppendXMLTag(szBuf, szTag, szAtts, depth);
	free(szAtts);
}

void AppendXMLTagBlob(char **szBuf, char *szTag, BYTE *pBlob, int cbBlob, int *depth) {

	int maxLen;
	char *szType, *szVal, *szAtts;
	NETLIBBASE64 nlb;

	maxLen = Netlib_GetBase64EncodedBufferSize(cbBlob);
	szVal = malloc(maxLen + 1);
	ZeroMemory(&nlb, sizeof(NETLIBBASE64));
	nlb.cbDecoded = cbBlob;
	nlb.pbDecoded = pBlob;
	nlb.cchEncoded = maxLen;
	nlb.pszEncoded = szVal;
	CallService(MS_NETLIB_BASE64ENCODE, 0, (LPARAM)&nlb);
	szType = GetSettingType(DBVT_BLOB);
	szAtts = malloc(strlen(szVal) + strlen(szType) + 17);
	sprintf(szAtts, "type=\"%s\" value=\"%s\"", szType, szVal);
	AppendXMLTag(szBuf, szTag, szAtts, depth);
	free(szAtts);
}
