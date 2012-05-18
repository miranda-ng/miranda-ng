/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "variables.h"
#include "parse_str.h"

static TCHAR *parseCaps(ARGUMENTSINFO *ai) {

	TCHAR *cur, *res;

	if (ai->argc != 2)
		return NULL;

	res = _tcsdup(ai->targv[1]);
	cur = res;
	CharLower(res);
	*cur = (TCHAR)CharUpper((LPTSTR)*cur);
	cur++;
	while (*cur != _T('\0')) {
		if ( (*cur == _T(' ')) && (*(cur+1) != _T('\0')) ) {
			cur++;
			if (IsCharLower(*cur))
				*cur = (TCHAR)CharUpper((LPTSTR)*cur);
		}
		else {
			cur++;
			if (IsCharUpper(*cur))
				*cur = (TCHAR)CharLower((LPTSTR)*cur);
		}
	}
	return res;
}

static TCHAR *parseCaps2(ARGUMENTSINFO *ai) {

	TCHAR *cur, *res;

	if (ai->argc != 2)
		return NULL;

	res = _tcsdup(ai->targv[1]);
	cur = res;
	*cur = (TCHAR)CharUpper((LPTSTR)*cur);
	cur++;
	while (*cur != _T('\0')) {
		if ( (*cur == _T(' ')) && (*(cur+1) != _T('\0')) ) {
			cur++;
			if (IsCharLower(*cur))
				*cur = (TCHAR)CharUpper((LPTSTR)*cur);
		}
		else cur++;
	}
	return res;
}

static TCHAR *parseCrlf(ARGUMENTSINFO *ai) {

	ai->flags |= AIF_DONTPARSE;
	
	return _tcsdup(_T("\r\n"));
}

static TCHAR *parseEolToCrlf(ARGUMENTSINFO *ai) {

	int loc;
	TCHAR *cur, *res;

	if (ai->argc != 2) {
		return NULL;
	}
	res = _tcsdup(ai->targv[1]);
	cur = res;
	do {
		cur = _tcschr(cur, _T('\n'));
		if ( (cur == NULL) || ((cur > res) && (*(cur-1) == '\r')) ) {
			continue;
		}
		log_debug(cur);
		loc = cur - res;
		res = ( TCHAR* )realloc(res, (_tcslen(res)+2)*sizeof(TCHAR));
		cur = res + loc;
		MoveMemory(cur+2, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
		CopyMemory(cur, _T("\r\n"), 2*sizeof(TCHAR));
		cur += 2;
	} while (cur != NULL);

	return res;
}

static TCHAR *parseFixeol(ARGUMENTSINFO *ai) {

	TCHAR *cur, *szReplacement, *res;

	if (ai->argc == 2) {
		szReplacement = _T("(...)");
	}
	else if (ai->argc == 3) {
		szReplacement = ai->targv[2];
	}
	else {
		return NULL;
	}
	cur = ai->targv[1];
	while ( (_tcscmp(cur, _T("\r\n"))) && (*cur != _T('\n')) && (*cur != _T('\0')) ) {
		cur++;
	}
	if (*cur == '\0') {
		return _tcsdup(ai->targv[1]);
	}
	cur--;
	res = ( TCHAR* )malloc((cur-ai->targv[1] + _tcslen(szReplacement) + 1)*sizeof(TCHAR));
	if (res == NULL) {
		return res;
	}
	ZeroMemory(res, ((cur - ai->targv[1]) + 1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1], cur-ai->targv[1]);
	_tcscat(res, szReplacement);
	
	return res;
}

static TCHAR *parseFixeol2(ARGUMENTSINFO *ai) {

	TCHAR *res, *cur, *szEol, *szReplacement;

	unsigned int pos = 0;
	res = _tcsdup(ai->targv[1]);
	switch( ai->argc ) {
		case 2:	szReplacement = _T(" ");	break;
		case 3:  szReplacement = ai->targv[2];  break;
		default: return NULL;
	}

	for ( pos=0; pos < _tcslen(res); pos++ ) {
		cur = res+pos;
		szEol = NULL;
		if (!_tcsncmp(cur, _T("\r\n"), _tcslen(_T("\r\n")))) {
			szEol = _T("\r\n");
		}
		if (*cur == _T('\n')) {
			szEol = _T("\n");
		}
		if (szEol != NULL) {
			if (_tcslen(szReplacement) > _tcslen(szEol)) {
				res = ( TCHAR* )realloc(res, (_tcslen(res) + _tcslen(szReplacement) - _tcslen(szEol) + 1)*sizeof(TCHAR));
				cur = res+pos;
			}
			MoveMemory(cur+_tcslen(szReplacement), cur+_tcslen(szEol), (_tcslen(cur+_tcslen(szEol))+1)*sizeof(TCHAR));
			CopyMemory(cur, szReplacement, _tcslen(szReplacement)*sizeof(TCHAR));
			pos += _tcslen(szReplacement) - 1;
		}
	}
	res = ( TCHAR* )realloc(res, (_tcslen(res)+1)*sizeof(TCHAR));
	
	return res;
}

static TCHAR *parseInsert(ARGUMENTSINFO *ai) {

	TCHAR *res;
	unsigned int pos;

	if (ai->argc != 4) {
		return NULL;
	}

	pos = ttoi(ai->targv[3]);
	if (pos > _tcslen(ai->targv[1])) {
		return NULL;
	}
	res = ( TCHAR* )malloc((_tcslen(ai->targv[1]) + _tcslen(ai->targv[2]) + 1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (_tcslen(ai->targv[1]) + _tcslen(ai->targv[2]) + 1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1], pos);
	_tcscpy(res + pos, ai->targv[2]);
	_tcscpy(res+pos+_tcslen(ai->targv[2]), ai->targv[1]+pos);

	return res;
}

static TCHAR *parseLeft(ARGUMENTSINFO *ai) {

	int len;
	TCHAR *res;

	if (ai->argc != 3) {
		return NULL;
	}
	len = ttoi(ai->targv[2]);
	if (len < 0) {
		return NULL;
	}
	len = min(len, (signed int)_tcslen(ai->targv[1]));
	res = ( TCHAR* )malloc((len + 1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (len+1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1], len);

	return res;
}

static TCHAR *parseLen(ARGUMENTSINFO *ai) {

	int len;

	if (ai->argc != 2) {
		return NULL;
	}
	len = _tcslen(ai->targv[1]);
	
	return itot(len);
}

static TCHAR *parseLineCount(ARGUMENTSINFO *ai) {

	int count;
	TCHAR *cur;

	if (ai->argc != 2) {
		return NULL;
	}
	count = 1;
	cur = ai->targv[1];
	while (cur < (ai->targv[1] + _tcslen(ai->targv[1]))) {
		if (!_tcsncmp(cur, _T("\r\n"), 2)) {
			count += 1;
			cur++;
		}
		else if (*cur == _T('\n')) {
			count += 1;
		}
		cur++;
	}

	return itot(count);
}

static TCHAR *parseLower(ARGUMENTSINFO *ai) {

	TCHAR *res;

	if (ai->argc != 2) {
		return NULL;
	}
	res = _tcsdup(ai->targv[1]);
	if (res == NULL) {
		return NULL;
	}
	
	return CharLower(res);
}

static TCHAR *parseLongest(ARGUMENTSINFO *ai) {

	unsigned int i, iLong;

	if (ai->argc < 2) {
		return NULL;
	}
	iLong = 1;
	for (i=2;i<ai->argc;i++) {
		if (_tcslen(ai->targv[i]) > _tcslen(ai->targv[iLong])) {
			iLong = i;
		}
	}
	return _tcsdup(ai->targv[iLong]);
}

static TCHAR *parseNoOp(ARGUMENTSINFO *ai) {

	if (ai->argc != 2) {
		return NULL;
	}
	
	return _tcsdup(ai->targv[1]);
}

static TCHAR *parsePad(ARGUMENTSINFO *ai) {

	unsigned int addcount, i;
	int padding;
	TCHAR *res, padchar, *cur;

	switch( ai->argc ) {
		case 3:  padchar = _T(' ');  break;
		case 4:  padchar = *ai->targv[3];  break;
		default: return NULL;
	}

	padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	res = ( TCHAR* )malloc((addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	cur = res;
	for ( i=0; i < addcount; i++ )
		*cur++ = padchar;

	_tcscat(res, ai->targv[1]);
	return res;
}
	
static TCHAR *parsePadright(ARGUMENTSINFO *ai) {

	unsigned int addcount, i;
	int padding;
	TCHAR *res, padchar, *cur;

	switch (ai->argc ) {
		case 3: padchar = _T(' ');  break;
		case 4: padchar = *ai->targv[3]; break;
		default: return NULL;
	}

	padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	res = ( TCHAR* )malloc((addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	_tcscpy(res, ai->targv[1]);
	cur = res + _tcslen(ai->targv[1]);
	for (i=0;i<addcount;i++)
		*cur++ = padchar;
	
	return res;
}

static TCHAR *parsePadcut(ARGUMENTSINFO *ai) {

	int padding, addcount, i;
	TCHAR *res, padchar, *cur;

	switch( ai->argc ) {
		case 3: padchar = _T(' ');   break;
		case 4: padchar = *ai->targv[3]; break;
		default: return NULL;
	}

	padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	res = ( TCHAR* )malloc((padding + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (padding + 1)*sizeof(TCHAR));
	cur = res;
	for (i=0;i<addcount;i++)
		*cur++ = padchar;

	if (padding > addcount)
		_tcsncpy(res+addcount, ai->targv[1], padding-addcount);
		
	return res;
}

static TCHAR *parsePadcutright(ARGUMENTSINFO *ai) {

	int padding, addcount, i;
	TCHAR *res, padchar, *cur;

	switch( ai->argc ) {
		case 3:  padchar = _T(' ');  break;
		case 4:  padchar = *ai->targv[3]; break;
		default: return NULL;
	}

	padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	res = ( TCHAR* )malloc((padding + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (padding + 1)*sizeof(TCHAR));
	cur = res + padding - addcount;
	for (i=0; i < addcount; i++ )
		*cur++ = padchar;

	if ( padding > addcount )
		_tcsncpy(res, ai->targv[1], padding-addcount);

	return res;
}

static TCHAR *parseRepeat(ARGUMENTSINFO *ai) {

	TCHAR *res;
	unsigned int i, count;

	if (ai->argc != 3) {
		return NULL;
	}
	count = ttoi(ai->targv[2]);
	if (count < 0) {
		return NULL;
	}
	res = ( TCHAR* )malloc((count * _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (count * _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	for (i=0;i<count;i++) {
		_tcscat(res, ai->targv[1]);
	}

	return res;
}

static TCHAR *parseReplace(ARGUMENTSINFO *ai) {

	TCHAR *res, *cur;
	unsigned int i, pos;

	if ( (ai->argc < 4) || (ai->argc%2 != 0) ) {
		return NULL;
	}
	pos = 0;
	res = _tcsdup(ai->targv[1]);
	for (i=2;i<ai->argc;i+=2) {
		if (_tcslen(ai->targv[i]) == 0) {
			continue;
		}
		for (pos=0;pos<_tcslen(res);pos++) {
			cur = res+pos;
			if (!_tcsncmp(cur, ai->targv[i], _tcslen(ai->targv[i]))) {
				if (_tcslen(ai->targv[i+1]) > _tcslen(ai->targv[i])) {
					res = ( TCHAR* )realloc(res, (_tcslen(res) + _tcslen(ai->targv[i+1]) - _tcslen(ai->targv[i]) + 1)*sizeof(TCHAR));
					cur = res+pos;
				}
				MoveMemory(cur+_tcslen(ai->targv[i+1]), cur+_tcslen(ai->targv[i]), (_tcslen(cur+_tcslen(ai->targv[i]))+1)*sizeof(TCHAR));
				CopyMemory(cur, ai->targv[i+1], _tcslen(ai->targv[i+1])*sizeof(TCHAR));
				pos += _tcslen(ai->targv[i+1]) - 1;
			}
		}
		res = ( TCHAR* )realloc(res, (_tcslen(res)+1)*sizeof(TCHAR));
	}

	return res;
}

static TCHAR *parseRight(ARGUMENTSINFO *ai) {

	int len;
	TCHAR *res;

	if (ai->argc != 3) {
		return NULL;
	}
	len = ttoi(ai->targv[2]);
	if (len < 0) {
		return NULL;
	}
	len = min(len, (signed int)_tcslen(ai->targv[1]));
	res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (len+1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1]+_tcslen(ai->targv[1])-len, len);

	return res;
}

/*
	string, display size, scroll amount 
*/
static TCHAR *parseScroll(ARGUMENTSINFO *ai) {

	unsigned int display, move;
	TCHAR *res;

	if (ai->argc != 4) {
		return NULL;
	}
	if (_tcslen(ai->targv[1]) == 0) {
	
		return _tcsdup(ai->targv[1]);
	}
	move = ttoi(ai->targv[3])%_tcslen(ai->targv[1]);
	display = ttoi(ai->targv[2]);
	if (display > _tcslen(ai->targv[1])) {
		display = _tcslen(ai->targv[1]);
	}
	res = ( TCHAR* )malloc((2*_tcslen(ai->targv[1])+1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (2*_tcslen(ai->targv[1])+1)*sizeof(TCHAR));
	_tcscpy(res, ai->targv[1]);
	_tcscat(res, ai->targv[1]);
	MoveMemory(res, res+move, (_tcslen(res+move)+1)*sizeof(TCHAR));
	*(res + display) = _T('\0');	
	res = ( TCHAR* )realloc(res, (_tcslen(res)+1)*sizeof(TCHAR));
	
	return res;
}			

static TCHAR *parseShortest(ARGUMENTSINFO *ai) {

	unsigned int i, iShort;

	if (ai->argc <= 1) {
		return NULL;
	}
	iShort = 1;
	for (i=2;i<ai->argc;i++) {
		if (_tcslen(ai->targv[i]) < _tcslen(ai->targv[iShort])) {
			iShort = i;
		}
	}

	return _tcsdup(ai->targv[iShort]);
}

static TCHAR *parseStrchr(ARGUMENTSINFO *ai) {

	TCHAR *c;
	char *szVal[34];
	
	if (ai->argc != 3) {
		return NULL;
	}
	ZeroMemory(szVal, sizeof(szVal));
	c = _tcschr(ai->targv[1], *ai->targv[2]);
	if ( (c == NULL) || (*c == _T('\0')) ) {
		return _tcsdup(_T("0"));
	}

	return itot(c-ai->targv[1]+1);
}

static TCHAR *parseStrcmp(ARGUMENTSINFO *ai) {

	if (ai->argc != 3) {
		return NULL;
	}
	if (_tcscmp(ai->targv[1], ai->targv[2])) {
		ai->flags |= AIF_FALSE;
	}

	return _tcsdup(_T(""));
}

static TCHAR *parseStrmcmp(ARGUMENTSINFO *ai) {

	unsigned int i;

	if (ai->argc < 3) {
		return NULL;
	}
	ai->flags |= AIF_FALSE;
	for (i=2;i<ai->argc;i++) {
		if (!_tcscmp(ai->targv[1], ai->targv[i])) {
			ai->flags &= ~AIF_FALSE;
			break;
		}
	}

	return _tcsdup(_T(""));
}

static TCHAR *parseStrncmp(ARGUMENTSINFO *ai) {

	int n;

	if (ai->argc != 4) {
		return NULL;
	}
	n = ttoi(ai->targv[3]);
	if (n <= 0) {
		return NULL;
	}
	if (_tcsncmp(ai->targv[1], ai->targv[2], n)) {
		ai->flags |= AIF_FALSE;
	}

	return _tcsdup(_T(""));
}

static TCHAR *parseStricmp(ARGUMENTSINFO *ai) {

	if (ai->argc != 3) {
		return NULL;
	}

	if (_tcsicmp(ai->targv[1], ai->targv[2])) {
		ai->flags |= AIF_FALSE;
	}

	return _tcsdup(_T(""));
}

static TCHAR *parseStrnicmp(ARGUMENTSINFO *ai) {

	int n;

	if (ai->argc != 4) {
		return NULL;
	}
	n = ttoi(ai->targv[3]);
	if (n <= 0) {
		return NULL;
	}
	if (_tcsnicmp(ai->targv[1], ai->targv[2], n)) {
		ai->flags |= AIF_FALSE;
	}

	return _tcsdup(_T(""));
}
	
static TCHAR *parseStrrchr(ARGUMENTSINFO *ai) {

	TCHAR *c;
	
	if (ai->argc != 3) {
		return NULL;
	}
	c = _tcsrchr(ai->targv[1], *ai->targv[2]);
	if ( (c == NULL) || (*c == _T('\0')) ) {
		return _tcsdup(_T("0"));
	}

	return itot(c-ai->targv[1]+1);
}

static TCHAR *parseStrstr(ARGUMENTSINFO *ai) {

	TCHAR *c;
	
	if (ai->argc != 3) {
		return NULL;
	}
	c = _tcsstr(ai->targv[1], ai->targv[2]);
	if ( (c == NULL) || (*c == _T('\0')) ) {
		return _tcsdup(_T("0"));
	}

	return itot(c-ai->targv[1]+1);
}

static TCHAR *parseSubstr(ARGUMENTSINFO *ai) {

	int from, to;
	TCHAR *res;

	if (ai->argc < 3) {
		return NULL;
	}
	from = max(ttoi(ai->targv[2])-1, 0);
	if (ai->argc > 3) {
		to = min(ttoi(ai->targv[3]), (signed int)_tcslen(ai->targv[1]));
	}
	else {
		to = _tcslen(ai->targv[1]);
	}
	if (to < from) {
		return NULL;
	}
	res = ( TCHAR* )malloc((to-from+1)*sizeof(TCHAR));
	ZeroMemory(res, (to-from+1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1]+from, to-from);

	return res;
}

static TCHAR *parseSelect(ARGUMENTSINFO *ai) {

	int n;

	if (ai->argc <= 1) {
		return NULL;
	}
	n = ttoi(ai->targv[1]);
	if ( (n > (signed int)ai->argc-2) || (n <= 0) ) {
		return NULL;
	}
	
	return _tcsdup(ai->targv[n+1]);
}

static TCHAR *parseSwitch(ARGUMENTSINFO *ai) {

	unsigned int i;

	if (ai->argc%2 != 0) {
		return NULL;
	}
	for (i=2;i<ai->argc;i+=2) {
		if (!_tcscmp(ai->targv[1], ai->targv[i])) {
			return _tcsdup(ai->targv[i+1]);
		}
	}
	return NULL;
}

static TCHAR *parseTrim(ARGUMENTSINFO *ai) {

	TCHAR *scur, *ecur, *res;

	if (ai->argc != 2) {
		return NULL;
	}
	scur = ai->targv[1];
	while (*scur == _T(' ')) {
		scur++;
	}
	ecur = ai->targv[1] + _tcslen(ai->targv[1])-1;
	while ( (*ecur == _T(' ')) && (ecur > ai->targv[1]) ) {
		ecur--;
	}
	if (scur >= ecur) {
		return _tcsdup(_T(""));
	}
	res = ( TCHAR* )malloc((ecur-scur+2)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (ecur-scur+2)*sizeof(TCHAR));
	_tcsncpy(res, scur, ecur-scur+1);

	return res;
}

static TCHAR *parseTab(ARGUMENTSINFO *ai) {

	int count, i;
	TCHAR *res, *cur;

	count = 1;
	if ( (ai->argc == 2) && (_tcslen(ai->targv[1]) > 0) ) {
		count = ttoi(ai->targv[1]);
	}
	if (count < 0) {
		return NULL;
	}
	res = ( TCHAR* )malloc((count+1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	memset(res, _T('\0'), (count+1)*sizeof(TCHAR));
	cur = res;
	for (i=0;i<count;i++) {
		*cur++ = _T('\t');
	}
	
	return res;
}

static TCHAR *parseUpper(ARGUMENTSINFO *ai) {

	TCHAR *res;

	if (ai->argc != 2) {
		return NULL;
	}
	res = _tcsdup(ai->targv[1]);
	if (res == NULL) {
		return NULL;
	}
	
	return CharUpper(res);
}

static TCHAR *getNthWord(TCHAR *szString, int w) {

	int count;
	TCHAR *res, *scur, *ecur;

	if (szString == NULL) {
		return NULL;
	}
	count = 0;
	scur = szString;
	while (*scur == _T(' ')) {
		scur++;
	}
	count+=1;
	while ( (count < w) && (scur < szString+_tcslen(szString)) ) {
		if (*scur == _T(' ')) {
			while (*scur == _T(' ')) {
				scur++;
			}
			count+=1;
		}
		if (count < w) {
			scur++;
		}
	}
	if (count != w) {
		return NULL;
	}
	ecur = scur;
	while ( (*ecur != _T(' ')) && (*ecur != _T('\0')) ) {
		ecur++;
	}
	res = ( TCHAR* )malloc((ecur-scur+1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (ecur-scur+1)*sizeof(TCHAR));
	_tcsncpy(res, scur, ecur-scur);

	return res;
}

static TCHAR *parseWord(ARGUMENTSINFO *ai) {

	int i, from, to;
	TCHAR *res, *szWord;

	if ( ai->argc < 3 || ai->argc > 4 )
		return NULL;

	res = NULL;
	from = ttoi(ai->targv[2]);
	if (ai->argc == 4) {
		if (_tcslen(ai->targv[3]) > 0)
			to = ttoi(ai->targv[3]);
		else
			to = 100000; // rework
	}
	else to = from;

	if ( (from == 0) || (to == 0) || (from > to) )
		return NULL;

	for (i=from;i<=to;i++) {
		szWord = getNthWord(ai->targv[1], i);
		if (szWord == NULL)
			return res;

		if (res != NULL) {
			res = ( TCHAR* )realloc(res, (_tcslen(res) + _tcslen(szWord) + 2)*sizeof(TCHAR));
			if (res != NULL) {
				_tcscat(res, _T(" "));
				_tcscat(res, szWord);
			}
		}
		else res = _tcsdup(szWord);

		free(szWord);
	}

	return res;
}

static TCHAR *parseExtratext(ARGUMENTSINFO *ai)
{
	if (ai->argc > 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	if (ai->fi->szExtraText != NULL)
		return _tcsdup(ai->fi->tszExtraText);

	return NULL;
}

int registerStrTokens() {

	registerIntToken(_T(CAPS), parseCaps, TRF_FUNCTION, "String Functions\t(x)\tconverts each first letter of a word to uppercase, all others to lowercase");
	registerIntToken(_T(CAPS2), parseCaps2, TRF_FUNCTION, "String Functions\t(x)\tconverts each first letter of a word to uppercase");
	registerIntToken(_T(CRLF), parseCrlf, TRF_FUNCTION, "String Functions\t()\tinserts 'end of line' character");
	registerIntToken(_T(EXTRATEXT), parseExtratext, TRF_FIELD, "String Functions\tdepends on calling plugin");
	registerIntToken(_T(EOL2CRLF), parseEolToCrlf, TRF_FUNCTION, "String Functions\t(x)\tReplace all occurrences of \\n (Unix) by \\r\\n (Windows)");
	registerIntToken(_T(FIXEOL), parseFixeol, TRF_FUNCTION, "String Functions\t(x,y)\tcuts x after the first line and appends y (y is optional)");
	registerIntToken(_T(FIXEOL2), parseFixeol2, TRF_FUNCTION, "String Functions\t(x,y)\treplaces all end of line characters by y (y is optional)");
	registerIntToken(_T(INSERT), parseInsert, TRF_FUNCTION, "String Functions\t(x,y,z)\tinserts string y at position z in string x");
	registerIntToken(_T(LEFT), parseLeft, TRF_FUNCTION, "String Functions\t(x,y)\ttrims x to length y, keeping first y characters");
	registerIntToken(_T(LEN), parseLen, TRF_FUNCTION, "String Functions\t(x)\tlength of x");
	registerIntToken(_T(LINECOUNT), parseLineCount, TRF_FUNCTION, "String Functions\t(x)\tthe number of lines in string x");
	registerIntToken(_T(LONGEST), parseLongest, TRF_FUNCTION, "String Functions\t(x,y,...)\tthe longest string of the arguments");
	registerIntToken(_T(LOWER), parseLower, TRF_FUNCTION, "String Functions\t(x)\tconverts x to lowercase");
	registerIntToken(_T(NOOP), parseNoOp, TRF_FUNCTION, "String Functions\t(x)\tno operation, x as given");
	registerIntToken(_T(PAD), parsePad, TRF_FUNCTION, "String Functions\t(x,y,z)\tpads x to length y prepending character z (z is optional)");
	registerIntToken(_T(PADRIGHT), parsePadright, TRF_FUNCTION, "String Functions\t(x,y,z)\tpads x to length y appending character z (z is optional)");
	registerIntToken(_T(PADCUT), parsePadcut, TRF_FUNCTION, "String Functions\t(x,y,z)\tpads x to length y prepending character z, or cut if x is longer (z is optional)");
	registerIntToken(_T(PADCUTRIGHT), parsePadcutright, TRF_FUNCTION, "String Functions\t(x,y,z)\tpads x to length y appending character z, or cut if x is longer (z is optional)");
	registerIntToken(_T(REPEAT), parseRepeat, TRF_FUNCTION, "String Functions\t(x,y)\trepeats x y times");
	registerIntToken(_T(REPLACE), parseReplace, TRF_FUNCTION, "String Functions\t(x,y,z,...)\treplace all occurrences of y in x with z, multiple y and z arguments allowed");
	registerIntToken(_T(RIGHT), parseRight, TRF_FUNCTION, "String Functions\t(x,y)\ttrims x to length y, keeping last y characters");
	registerIntToken(_T(SCROLL), parseScroll, TRF_FUNCTION, "String Functions\t(x,y,z)\tmoves string x, z characters to the left and trims it to y characters");
	registerIntToken(_T(STRCMP), parseStrcmp, TRF_FUNCTION, "String Functions\t(x,y)\tTRUE if x equals y");
	registerIntToken(_T(STRMCMP), parseStrmcmp, TRF_FUNCTION, "String Functions\t(x,y,...)\tTRUE if x equals any of the following arguments");
	registerIntToken(_T(STRNCMP), parseStrncmp, TRF_FUNCTION, "String Functions\t(x,y,z)\tTRUE if the first z characters of x equal y");
	registerIntToken(_T(STRICMP), parseStricmp, TRF_FUNCTION, "String Functions\t(x,y)\tTRUE if x equals y, ignoring case");
	registerIntToken(_T(STRNICMP), parseStrnicmp, TRF_FUNCTION, "String Functions\t(x,y)\tTRUE if the first z characters of x equal y, ignoring case");
	registerIntToken(_T(SHORTEST), parseShortest, TRF_FUNCTION, "String Functions\t(x,y,...)\tthe shortest string of the arguments");
	registerIntToken(_T(STRCHR), parseStrchr, TRF_FUNCTION, "String Functions\t(x,y)\tlocation of first occurrence of character y in string x");
	registerIntToken(_T(STRRCHR), parseStrrchr, TRF_FUNCTION, "String Functions\t(x,y)\tlocation of last occurrence of character y in string x");
	registerIntToken(_T(STRSTR), parseStrstr, TRF_FUNCTION, "String Functions\t(x,y)\tlocation of first occurrence of string y in x");
	registerIntToken(_T(SUBSTR), parseSubstr, TRF_FUNCTION, "String Functions\t(x,y,z)\tsubstring of x starting from position y to z");
	registerIntToken(_T(SELECT), parseSelect, TRF_FUNCTION, "String Functions\t(x,y,...)\tthe xth string of the arguments");
	registerIntToken(_T(SWITCH), parseSwitch, TRF_FUNCTION, "String Functions\t(x,y,z,...)\tz if y equals x, multiple y and z arguments allowed");
	registerIntToken(_T(TRIM), parseTrim, TRF_FUNCTION, "String Functions\t(x)\tremoves white spaces in before and after x");
	registerIntToken(_T(TAB), parseTab, TRF_FUNCTION, "String Functions\t(x)\tinserts x tab characters (x is optional)");
	registerIntToken(_T(UPPER), parseUpper, TRF_FUNCTION, "String Functions\t(x)\tconverts x to upper case");
	registerIntToken(_T(WORD), parseWord, TRF_FUNCTION, "String Functions\t(x,y,z)\twords (separated by white spaces) number y to z from string x (z is optional)");

	return 0;
}
