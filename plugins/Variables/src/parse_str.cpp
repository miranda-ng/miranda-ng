/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is mir_free software; you can redistribute it and/or modify
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

static TCHAR *parseCaps(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	TCHAR *cur = res;
	CharLower(res);
	*cur = (TCHAR)CharUpper((LPTSTR)*cur);
	cur++;
	while (*cur != _T('\0')) {
		if ((*cur == _T(' ')) && (*(cur+1) != _T('\0'))) {
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

static TCHAR *parseCaps2(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	TCHAR *cur = res;
	*cur = (TCHAR)CharUpper((LPTSTR)*cur);
	cur++;
	while (*cur != _T('\0')) {
		if ((*cur == _T(' ')) && (*(cur+1) != _T('\0'))) {
			cur++;
			if (IsCharLower(*cur))
				*cur = (TCHAR)CharUpper((LPTSTR)*cur);
		}
		else cur++;
	}
	return res;
}

static TCHAR *parseCrlf(ARGUMENTSINFO *ai)
{
	ai->flags |= AIF_DONTPARSE;
	return mir_tstrdup(_T("\r\n"));
}

static TCHAR *parseEolToCrlf(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	TCHAR *cur = res;
	do {
		cur = _tcschr(cur, _T('\n'));
		if ((cur == NULL) || ((cur > res) && (*(cur-1) == '\r')))
			continue;
		
		log_debug(cur);
		int loc = cur - res;
		res = (TCHAR*)mir_realloc(res, (_tcslen(res)+2)*sizeof(TCHAR));
		cur = res + loc;
		MoveMemory(cur+2, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
		CopyMemory(cur, _T("\r\n"), 2*sizeof(TCHAR));
		cur += 2;
	}
		while (cur != NULL);

	return res;
}

static TCHAR *parseFixeol(ARGUMENTSINFO *ai)
{
	TCHAR *szReplacement;
	if (ai->argc == 2)
		szReplacement = _T("(...)");
	else if (ai->argc == 3)
		szReplacement = ai->targv[2];
	else
		return NULL;

	TCHAR *cur = ai->targv[1];
	while ( (_tcscmp(cur, _T("\r\n"))) && (*cur != _T('\n')) && (*cur != _T('\0')))
		cur++;

	if (*cur == '\0')
		return mir_tstrdup(ai->targv[1]);

	cur--;
	TCHAR *res = (TCHAR*)mir_alloc((cur-ai->targv[1] + _tcslen(szReplacement) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return res;

	ZeroMemory(res, ((cur - ai->targv[1]) + 1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1], cur-ai->targv[1]);
	_tcscat(res, szReplacement);
	return res;
}

static TCHAR *parseFixeol2(ARGUMENTSINFO *ai)
{
	TCHAR *szReplacement;
	switch( ai->argc ) {
		case 2:	szReplacement = _T(" ");	break;
		case 3:  szReplacement = ai->targv[2];  break;
		default: return NULL;
	}

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	for (size_t pos=0; pos < _tcslen(res); pos++ ) {
		TCHAR *cur = res+pos;
		TCHAR *szEol = NULL;
		if (!_tcsncmp(cur, _T("\r\n"), _tcslen(_T("\r\n"))))
			szEol = _T("\r\n");

		if (*cur == _T('\n'))
			szEol = _T("\n");

		if (szEol != NULL) {
			if ( _tcslen(szReplacement) > _tcslen(szEol)) {
				res = (TCHAR*)mir_realloc(res, (_tcslen(res) + _tcslen(szReplacement) - _tcslen(szEol) + 1)*sizeof(TCHAR));
				cur = res+pos;
			}
			MoveMemory(cur+_tcslen(szReplacement), cur+_tcslen(szEol), (_tcslen(cur+_tcslen(szEol))+1)*sizeof(TCHAR));
			CopyMemory(cur, szReplacement, _tcslen(szReplacement)*sizeof(TCHAR));
			pos += _tcslen(szReplacement) - 1;
		}
	}
	return (TCHAR*)mir_realloc(res, (_tcslen(res)+1)*sizeof(TCHAR));
}

static TCHAR *parseInsert(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return NULL;

	unsigned int pos = ttoi(ai->targv[3]);
	if (pos > _tcslen(ai->targv[1]))
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((_tcslen(ai->targv[1]) + _tcslen(ai->targv[2]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (_tcslen(ai->targv[1]) + _tcslen(ai->targv[2]) + 1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1], pos);
	_tcscpy(res + pos, ai->targv[2]);
	_tcscpy(res+pos+_tcslen(ai->targv[2]), ai->targv[1]+pos);
	return res;
}

static TCHAR *parseLeft(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	int len = ttoi(ai->targv[2]);
	if (len < 0)
		return NULL;

	len = min(len, (signed int)_tcslen(ai->targv[1]));
	TCHAR *res = (TCHAR*)mir_alloc((len + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (len+1)*sizeof(TCHAR));
	_tcsncpy(res, ai->targv[1], len);
	return res;
}

static TCHAR *parseLen(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	return itot( _tcslen( ai->targv[1] ));
}

static TCHAR *parseLineCount(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	int count = 1;
	TCHAR *cur = ai->targv[1];
	while (cur < (ai->targv[1] + _tcslen(ai->targv[1]))) {
		if (!_tcsncmp(cur, _T("\r\n"), 2)) {
			count += 1;
			cur++;
		}
		else if (*cur == _T('\n'))
			count++;

		cur++;
	}

	return itot(count);
}

static TCHAR *parseLower(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	if (res == NULL)
		return NULL;
	
	return CharLower(res);
}

static TCHAR *parseLongest(ARGUMENTSINFO *ai)
{
	if (ai->argc < 2)
		return NULL;

	unsigned int iLong = 1;
	for (unsigned int i=2; i < ai->argc; i++)
		if ( _tcslen(ai->targv[i]) > _tcslen(ai->targv[iLong]))
			iLong = i;

	return mir_tstrdup( ai->targv[iLong] );
}

static TCHAR *parseNoOp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;
	
	return mir_tstrdup( ai->targv[1] );
}

static TCHAR *parsePad(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch( ai->argc ) {
		case 3:  padchar = _T(' ');  break;
		case 4:  padchar = *ai->targv[3];  break;
		default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	unsigned int addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	TCHAR *cur = res;
	for ( unsigned int i=0; i < addcount; i++ )
		*cur++ = padchar;

	_tcscat(res, ai->targv[1]);
	return res;
}
	
static TCHAR *parsePadright(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch (ai->argc ) {
		case 3: padchar = _T(' ');  break;
		case 4: padchar = *ai->targv[3]; break;
		default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	unsigned int addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (addcount + _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
	_tcscpy(res, ai->targv[1]);
	TCHAR *cur = res + _tcslen(ai->targv[1]);
	for (unsigned int i=0; i < addcount; i++)
		*cur++ = padchar;
	
	return res;
}

static TCHAR *parsePadcut(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch( ai->argc ) {
		case 3: padchar = _T(' ');   break;
		case 4: padchar = *ai->targv[3]; break;
		default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	int addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((padding + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (padding + 1)*sizeof(TCHAR));
	TCHAR *cur = res;
	for (int i=0; i < addcount; i++)
		*cur++ = padchar;

	if (padding > addcount)
		_tcsncpy(res+addcount, ai->targv[1], padding-addcount);
		
	return res;
}

static TCHAR *parsePadcutright(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch( ai->argc ) {
		case 3:  padchar = _T(' ');  break;
		case 4:  padchar = *ai->targv[3]; break;
		default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	int addcount = max(padding - (signed int)_tcslen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((padding + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (padding + 1)*sizeof(TCHAR));
	TCHAR *cur = res + padding - addcount;
	for (int i=0; i < addcount; i++ )
		*cur++ = padchar;

	if (padding > addcount )
		_tcsncpy(res, ai->targv[1], padding-addcount);

	return res;
}

static TCHAR *parseRepeat(ARGUMENTSINFO *ai)
{
	TCHAR *res;
	unsigned int i, count;

	if (ai->argc != 3) {
		return NULL;
	}
	count = ttoi(ai->targv[2]);
	if (count < 0) {
		return NULL;
	}
	res = (TCHAR*)mir_alloc((count * _tcslen(ai->targv[1]) + 1)*sizeof(TCHAR));
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
	if ((ai->argc < 4) || (ai->argc%2 != 0)) {
		return NULL;
	}
	TCHAR *cur;

	size_t pos = 0;
	TCHAR *res = mir_tstrdup(ai->targv[1]);
	for (size_t i=2;i<ai->argc;i+=2) {
		if ( _tcslen(ai->targv[i]) == 0) {
			continue;
		}
		for (pos=0;pos<_tcslen(res);pos++) {
			cur = res+pos;
			if (!_tcsncmp(cur, ai->targv[i], _tcslen(ai->targv[i]))) {
				if ( _tcslen(ai->targv[i+1]) > _tcslen(ai->targv[i])) {
					res = (TCHAR*)mir_realloc(res, (_tcslen(res) + _tcslen(ai->targv[i+1]) - _tcslen(ai->targv[i]) + 1)*sizeof(TCHAR));
					cur = res+pos;
				}
				MoveMemory(cur+_tcslen(ai->targv[i+1]), cur+_tcslen(ai->targv[i]), (_tcslen(cur+_tcslen(ai->targv[i]))+1)*sizeof(TCHAR));
				CopyMemory(cur, ai->targv[i+1], _tcslen(ai->targv[i+1])*sizeof(TCHAR));
				pos += _tcslen(ai->targv[i+1]) - 1;
			}
		}
		res = (TCHAR*)mir_realloc(res, (_tcslen(res)+1)*sizeof(TCHAR));
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
	res = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR));
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
	if ( _tcslen(ai->targv[1]) == 0) {
	
		return mir_tstrdup(ai->targv[1]);
	}
	move = ttoi(ai->targv[3])%_tcslen(ai->targv[1]);
	display = ttoi(ai->targv[2]);
	if (display > _tcslen(ai->targv[1])) {
		display = _tcslen(ai->targv[1]);
	}
	res = (TCHAR*)mir_alloc((2*_tcslen(ai->targv[1])+1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (2*_tcslen(ai->targv[1])+1)*sizeof(TCHAR));
	_tcscpy(res, ai->targv[1]);
	_tcscat(res, ai->targv[1]);
	MoveMemory(res, res+move, (_tcslen(res+move)+1)*sizeof(TCHAR));
	*(res + display) = _T('\0');	
	res = (TCHAR*)mir_realloc(res, (_tcslen(res)+1)*sizeof(TCHAR));
	
	return res;
}			

static TCHAR *parseShortest(ARGUMENTSINFO *ai) {

	unsigned int i, iShort;

	if (ai->argc <= 1) {
		return NULL;
	}
	iShort = 1;
	for (i=2;i<ai->argc;i++) {
		if ( _tcslen(ai->targv[i]) < _tcslen(ai->targv[iShort])) {
			iShort = i;
		}
	}

	return mir_tstrdup(ai->targv[iShort]);
}

static TCHAR *parseStrchr(ARGUMENTSINFO *ai) {

	TCHAR *c;
	char *szVal[34];
	
	if (ai->argc != 3) {
		return NULL;
	}
	ZeroMemory(szVal, sizeof(szVal));
	c = _tcschr(ai->targv[1], *ai->targv[2]);
	if ((c == NULL) || (*c == _T('\0'))) {
		return mir_tstrdup(_T("0"));
	}

	return itot(c-ai->targv[1]+1);
}

static TCHAR *parseStrcmp(ARGUMENTSINFO *ai) {

	if (ai->argc != 3) {
		return NULL;
	}
	if ( _tcscmp(ai->targv[1], ai->targv[2])) {
		ai->flags |= AIF_FALSE;
	}

	return mir_tstrdup(_T(""));
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

	return mir_tstrdup(_T(""));
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
	if ( _tcsncmp(ai->targv[1], ai->targv[2], n)) {
		ai->flags |= AIF_FALSE;
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseStricmp(ARGUMENTSINFO *ai) {

	if (ai->argc != 3) {
		return NULL;
	}

	if ( _tcsicmp(ai->targv[1], ai->targv[2])) {
		ai->flags |= AIF_FALSE;
	}

	return mir_tstrdup(_T(""));
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
	if ( _tcsnicmp(ai->targv[1], ai->targv[2], n)) {
		ai->flags |= AIF_FALSE;
	}

	return mir_tstrdup(_T(""));
}
	
static TCHAR *parseStrrchr(ARGUMENTSINFO *ai) {

	TCHAR *c;
	
	if (ai->argc != 3) {
		return NULL;
	}
	c = _tcsrchr(ai->targv[1], *ai->targv[2]);
	if ((c == NULL) || (*c == _T('\0'))) {
		return mir_tstrdup(_T("0"));
	}

	return itot(c-ai->targv[1]+1);
}

static TCHAR *parseStrstr(ARGUMENTSINFO *ai) {

	TCHAR *c;
	
	if (ai->argc != 3) {
		return NULL;
	}
	c = _tcsstr(ai->targv[1], ai->targv[2]);
	if ((c == NULL) || (*c == _T('\0'))) {
		return mir_tstrdup(_T("0"));
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
	res = (TCHAR*)mir_alloc((to-from+1)*sizeof(TCHAR));
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
	if ((n > (signed int)ai->argc-2) || (n <= 0)) {
		return NULL;
	}
	
	return mir_tstrdup(ai->targv[n+1]);
}

static TCHAR *parseSwitch(ARGUMENTSINFO *ai) {

	unsigned int i;

	if (ai->argc%2 != 0) {
		return NULL;
	}
	for (i=2;i<ai->argc;i+=2) {
		if (!_tcscmp(ai->targv[1], ai->targv[i])) {
			return mir_tstrdup(ai->targv[i+1]);
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
	while ( (*ecur == _T(' ')) && (ecur > ai->targv[1])) {
		ecur--;
	}
	if (scur >= ecur) {
		return mir_tstrdup(_T(""));
	}
	res = (TCHAR*)mir_alloc((ecur-scur+2)*sizeof(TCHAR));
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
	if ((ai->argc == 2) && (_tcslen(ai->targv[1]) > 0)) {
		count = ttoi(ai->targv[1]);
	}
	if (count < 0) {
		return NULL;
	}
	res = (TCHAR*)mir_alloc((count+1)*sizeof(TCHAR));
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
	res = mir_tstrdup(ai->targv[1]);
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
	while ( (count < w) && (scur < szString+_tcslen(szString))) {
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
	while ( (*ecur != _T(' ')) && (*ecur != _T('\0'))) {
		ecur++;
	}
	res = (TCHAR*)mir_alloc((ecur-scur+1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (ecur-scur+1)*sizeof(TCHAR));
	_tcsncpy(res, scur, ecur-scur);

	return res;
}

static TCHAR *parseWord(ARGUMENTSINFO *ai)
{
	int i, from, to;
	TCHAR *res, *szWord;

	if (ai->argc < 3 || ai->argc > 4 )
		return NULL;

	res = NULL;
	from = ttoi(ai->targv[2]);
	if (ai->argc == 4) {
		if ( _tcslen(ai->targv[3]) > 0)
			to = ttoi(ai->targv[3]);
		else
			to = 100000; // rework
	}
	else to = from;

	if ((from == 0) || (to == 0) || (from > to))
		return NULL;

	for (i=from;i<=to;i++) {
		szWord = getNthWord(ai->targv[1], i);
		if (szWord == NULL)
			return res;

		if (res != NULL) {
			res = (TCHAR*)mir_realloc(res, (_tcslen(res) + _tcslen(szWord) + 2)*sizeof(TCHAR));
			if (res != NULL) {
				_tcscat(res, _T(" "));
				_tcscat(res, szWord);
			}
		}
		else res = mir_tstrdup(szWord);

		mir_free(szWord);
	}

	return res;
}

static TCHAR *parseExtratext(ARGUMENTSINFO *ai)
{
	if (ai->argc > 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	if (ai->fi->szExtraText != NULL)
		return mir_tstrdup(ai->fi->tszExtraText);

	return NULL;
}

int registerStrTokens() {

	registerIntToken(_T(CAPS), parseCaps, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts each first letter of a word to uppercase, all others to lowercase"));
	registerIntToken(_T(CAPS2), parseCaps2, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts each first letter of a word to uppercase"));
	registerIntToken(_T(CRLF), parseCrlf, TRF_FUNCTION, LPGEN("String Functions")"\t()\t"LPGEN("inserts 'end of line' character"));
	registerIntToken(_T(EXTRATEXT), parseExtratext, TRF_FIELD, LPGEN("String Functions")"\t"LPGEN("depends on calling plugin"));
	registerIntToken(_T(EOL2CRLF), parseEolToCrlf, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("Replace all occurrences of \\n (Unix) by \\r\\n (Windows)"));
	registerIntToken(_T(FIXEOL), parseFixeol, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("cuts x after the first line and appends y (y is optional)"));
	registerIntToken(_T(FIXEOL2), parseFixeol2, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("replaces all end of line characters by y (y is optional)"));
	registerIntToken(_T(INSERT), parseInsert, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("inserts string y at position z in string x"));
	registerIntToken(_T(LEFT), parseLeft, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("trims x to length y, keeping first y characters"));
	registerIntToken(_T(LEN), parseLen, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("length of x"));
	registerIntToken(_T(LINECOUNT), parseLineCount, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("the number of lines in string x"));
	registerIntToken(_T(LONGEST), parseLongest, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("the longest string of the arguments"));
	registerIntToken(_T(LOWER), parseLower, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts x to lowercase"));
	registerIntToken(_T(NOOP), parseNoOp, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("no operation, x as given"));
	registerIntToken(_T(PAD), parsePad, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y prepending character z (z is optional)"));
	registerIntToken(_T(PADRIGHT), parsePadright, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y appending character z (z is optional)"));
	registerIntToken(_T(PADCUT), parsePadcut, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y prepending character z, or cut if x is longer (z is optional)"));
	registerIntToken(_T(PADCUTRIGHT), parsePadcutright, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y appending character z, or cut if x is longer (z is optional)"));
	registerIntToken(_T(REPEAT), parseRepeat, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("repeats x y times"));
	registerIntToken(_T(REPLACE), parseReplace, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z,...)\t"LPGEN("replace all occurrences of y in x with z, multiple y and z arguments allowed"));
	registerIntToken(_T(RIGHT), parseRight, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("trims x to length y, keeping last y characters"));
	registerIntToken(_T(SCROLL), parseScroll, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("moves string x, z characters to the left and trims it to y characters"));
	registerIntToken(_T(STRCMP), parseStrcmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("TRUE if x equals y"));
	registerIntToken(_T(STRMCMP), parseStrmcmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("TRUE if x equals any of the following arguments"));
	registerIntToken(_T(STRNCMP), parseStrncmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("TRUE if the first z characters of x equal y"));
	registerIntToken(_T(STRICMP), parseStricmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("TRUE if x equals y, ignoring case"));
	registerIntToken(_T(STRNICMP), parseStrnicmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("TRUE if the first z characters of x equal y, ignoring case"));
	registerIntToken(_T(SHORTEST), parseShortest, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("the shortest string of the arguments"));
	registerIntToken(_T(STRCHR), parseStrchr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("location of first occurrence of character y in string x"));
	registerIntToken(_T(STRRCHR), parseStrrchr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("location of last occurrence of character y in string x"));
	registerIntToken(_T(STRSTR), parseStrstr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("location of first occurrence of string y in x"));
	registerIntToken(_T(SUBSTR), parseSubstr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("substring of x starting from position y to z"));
	registerIntToken(_T(SELECT), parseSelect, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("the xth string of the arguments"));
	registerIntToken(_T(SWITCH), parseSwitch, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z,...)\t"LPGEN("z if y equals x, multiple y and z arguments allowed"));
	registerIntToken(_T(TRIM), parseTrim, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("removes white spaces in before and after x"));
	registerIntToken(_T(TAB), parseTab, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("inserts x tab characters (x is optional)"));
	registerIntToken(_T(UPPER), parseUpper, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts x to upper case"));
	registerIntToken(_T(WORD), parseWord, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("words (separated by white spaces) number y to z from string x (z is optional)"));

	return 0;
}
