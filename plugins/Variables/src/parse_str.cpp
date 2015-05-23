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

static TCHAR *parseCaps(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	TCHAR *cur = res;
	CharLower(res);
	*cur = (TCHAR)CharUpper((LPTSTR)*cur);
	cur++;
	while (*cur != 0) {
		if ((*cur == ' ') && (*(cur + 1) != 0)) {
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
	while (*cur != 0) {
		if ((*cur == ' ') && (*(cur + 1) != 0)) {
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
		cur = _tcschr(cur, '\n');
		if ((cur == NULL) || ((cur > res) && (*(cur - 1) == '\r')))
			continue;

		log_debug(cur);
		int loc = cur - res;
		res = (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + 2)*sizeof(TCHAR));
		cur = res + loc;
		memmove(cur + 2, cur + 1, (mir_tstrlen(cur + 1) + 1)*sizeof(TCHAR));
		memcpy(cur, _T("\r\n"), 2 * sizeof(TCHAR));
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
	while (mir_tstrcmp(cur, _T("\r\n")) && *cur != '\n' && *cur != 0)
		cur++;

	if (*cur == '\0')
		return mir_tstrdup(ai->targv[1]);

	cur--;
	TCHAR *res = (TCHAR*)mir_alloc((cur - ai->targv[1] + mir_tstrlen(szReplacement) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return res;

	memset(res, 0, (((cur - ai->targv[1]) + 1) * sizeof(TCHAR)));
	_tcsncpy(res, ai->targv[1], cur - ai->targv[1]);
	mir_tstrcat(res, szReplacement);
	return res;
}

static TCHAR *parseFixeol2(ARGUMENTSINFO *ai)
{
	TCHAR *szReplacement;
	switch (ai->argc) {
	case 2:	szReplacement = _T(" ");	break;
	case 3:  szReplacement = ai->targv[2];  break;
	default: return NULL;
	}

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	for (size_t pos = 0; pos < mir_tstrlen(res); pos++) {
		TCHAR *cur = res + pos;
		TCHAR *szEol = NULL;
		if (!_tcsncmp(cur, _T("\r\n"), mir_tstrlen(_T("\r\n"))))
			szEol = _T("\r\n");

		if (*cur == '\n')
			szEol = _T("\n");

		if (szEol != NULL) {
			if (mir_tstrlen(szReplacement) > mir_tstrlen(szEol)) {
				res = (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + mir_tstrlen(szReplacement) - mir_tstrlen(szEol) + 1)*sizeof(TCHAR));
				cur = res + pos;
			}
			memmove(cur + mir_tstrlen(szReplacement), cur + mir_tstrlen(szEol), (mir_tstrlen(cur + mir_tstrlen(szEol)) + 1)*sizeof(TCHAR));
			memcpy(cur, szReplacement, mir_tstrlen(szReplacement)*sizeof(TCHAR));
			pos += mir_tstrlen(szReplacement) - 1;
		}
	}
	return (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + 1)*sizeof(TCHAR));
}

static TCHAR *parseInsert(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return NULL;

	unsigned int pos = ttoi(ai->targv[3]);
	if (pos > mir_tstrlen(ai->targv[1]))
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((mir_tstrlen(ai->targv[1]) + mir_tstrlen(ai->targv[2]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((mir_tstrlen(ai->targv[1]) + mir_tstrlen(ai->targv[2]) + 1) * sizeof(TCHAR)));
	_tcsncpy(res, ai->targv[1], pos);
	mir_tstrcpy(res + pos, ai->targv[2]);
	mir_tstrcpy(res + pos + mir_tstrlen(ai->targv[2]), ai->targv[1] + pos);
	return res;
}

static TCHAR *parseLeft(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	int len = ttoi(ai->targv[2]);
	if (len < 0)
		return NULL;

	len = min(len, (signed int)mir_tstrlen(ai->targv[1]));
	TCHAR *res = (TCHAR*)mir_alloc((len + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((len + 1) * sizeof(TCHAR)));
	_tcsncpy(res, ai->targv[1], len);
	return res;
}

static TCHAR *parseLen(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	return itot((int)mir_tstrlen(ai->targv[1]));
}

static TCHAR *parseLineCount(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	int count = 1;
	TCHAR *cur = ai->targv[1];
	while (cur < (ai->targv[1] + mir_tstrlen(ai->targv[1]))) {
		if (!_tcsncmp(cur, _T("\r\n"), 2)) {
			count++;
			cur++;
		}
		else if (*cur == '\n')
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
	for (unsigned int i = 2; i < ai->argc; i++)
	if (mir_tstrlen(ai->targv[i]) > mir_tstrlen(ai->targv[iLong]))
		iLong = i;

	return mir_tstrdup(ai->targv[iLong]);
}

static TCHAR *parseNoOp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	return mir_tstrdup(ai->targv[1]);
}

static TCHAR *parsePad(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch (ai->argc) {
	case 3:  padchar = ' ';  break;
	case 4:  padchar = *ai->targv[3];  break;
	default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	unsigned int addcount = max(padding - (signed int)mir_tstrlen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((addcount + mir_tstrlen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((addcount + mir_tstrlen(ai->targv[1]) + 1) * sizeof(TCHAR)));
	TCHAR *cur = res;
	for (unsigned int i = 0; i < addcount; i++)
		*cur++ = padchar;

	mir_tstrcat(res, ai->targv[1]);
	return res;
}

static TCHAR *parsePadright(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch (ai->argc) {
	case 3: padchar = ' ';  break;
	case 4: padchar = *ai->targv[3]; break;
	default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	unsigned int addcount = max(padding - (signed int)mir_tstrlen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((addcount + mir_tstrlen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((addcount + mir_tstrlen(ai->targv[1]) + 1) * sizeof(TCHAR)));
	mir_tstrcpy(res, ai->targv[1]);
	TCHAR *cur = res + mir_tstrlen(ai->targv[1]);
	for (unsigned int i = 0; i < addcount; i++)
		*cur++ = padchar;

	return res;
}

static TCHAR *parsePadcut(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch (ai->argc) {
	case 3: padchar = ' ';   break;
	case 4: padchar = *ai->targv[3]; break;
	default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	int addcount = max(padding - (signed int)mir_tstrlen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((padding + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((padding + 1) * sizeof(TCHAR)));
	TCHAR *cur = res;
	for (int i = 0; i < addcount; i++)
		*cur++ = padchar;

	if (padding > addcount)
		_tcsncpy(res + addcount, ai->targv[1], padding - addcount);

	return res;
}

static TCHAR *parsePadcutright(ARGUMENTSINFO *ai)
{
	TCHAR padchar;
	switch (ai->argc) {
	case 3:  padchar = ' ';  break;
	case 4:  padchar = *ai->targv[3]; break;
	default: return NULL;
	}

	int padding = ttoi(ai->targv[2]);
	if (padding < 0)
		return NULL;

	int addcount = max(padding - (signed int)mir_tstrlen(ai->targv[1]), 0);
	TCHAR *res = (TCHAR*)mir_alloc((padding + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((padding + 1) * sizeof(TCHAR)));
	TCHAR *cur = res + padding - addcount;
	for (int i = 0; i < addcount; i++)
		*cur++ = padchar;

	if (padding > addcount)
		_tcsncpy(res, ai->targv[1], padding - addcount);

	return res;
}

static TCHAR *parseRepeat(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	int count = ttoi(ai->targv[2]);
	if (count < 0)
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((count * mir_tstrlen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((count * mir_tstrlen(ai->targv[1]) + 1) * sizeof(TCHAR)));
	for (int i = 0; i < count; i++)
		mir_tstrcat(res, ai->targv[1]);

	return res;
}

static TCHAR *parseReplace(ARGUMENTSINFO *ai)
{
	if ((ai->argc < 4) || (ai->argc % 2 != 0))
		return NULL;

	TCHAR *cur;

	size_t pos = 0;
	TCHAR *res = mir_tstrdup(ai->targv[1]);
	for (size_t i = 2; i < ai->argc; i += 2) {
		if (mir_tstrlen(ai->targv[i]) == 0)
			continue;

		for (pos = 0; pos<mir_tstrlen(res); pos++) {
			cur = res + pos;
			if (!_tcsncmp(cur, ai->targv[i], mir_tstrlen(ai->targv[i]))) {
				if (mir_tstrlen(ai->targv[i + 1]) > mir_tstrlen(ai->targv[i])) {
					res = (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + mir_tstrlen(ai->targv[i + 1]) - mir_tstrlen(ai->targv[i]) + 1)*sizeof(TCHAR));
					cur = res + pos;
				}
				memmove(cur + mir_tstrlen(ai->targv[i + 1]), cur + mir_tstrlen(ai->targv[i]), (mir_tstrlen(cur + mir_tstrlen(ai->targv[i])) + 1)*sizeof(TCHAR));
				memcpy(cur, ai->targv[i + 1], mir_tstrlen(ai->targv[i + 1])*sizeof(TCHAR));
				pos += mir_tstrlen(ai->targv[i + 1]) - 1;
			}
		}
		res = (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + 1)*sizeof(TCHAR));
	}

	return res;
}

static TCHAR *parseRight(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	int len = ttoi(ai->targv[2]);
	if (len < 0)
		return NULL;

	len = min(len, (signed int)mir_tstrlen(ai->targv[1]));
	TCHAR *res = (TCHAR*)mir_alloc((len + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((len + 1)*sizeof(TCHAR)));
	_tcsncpy(res, ai->targv[1] + mir_tstrlen(ai->targv[1]) - len, len);
	return res;
}

/*
	string, display size, scroll amount
	*/
static TCHAR *parseScroll(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return NULL;

	if (mir_tstrlen(ai->targv[1]) == 0)
		return mir_tstrdup(ai->targv[1]);

	size_t move = ttoi(ai->targv[3]) % mir_tstrlen(ai->targv[1]);
	size_t display = ttoi(ai->targv[2]);
	if (display > mir_tstrlen(ai->targv[1]))
		display = (unsigned)mir_tstrlen(ai->targv[1]);

	TCHAR *res = (TCHAR*)mir_alloc((2 * mir_tstrlen(ai->targv[1]) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((2 * mir_tstrlen(ai->targv[1]) + 1) * sizeof(TCHAR)));
	mir_tstrcpy(res, ai->targv[1]);
	mir_tstrcat(res, ai->targv[1]);
	memmove(res, res + move, (mir_tstrlen(res + move) + 1)*sizeof(TCHAR));
	*(res + display) = 0;
	res = (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + 1)*sizeof(TCHAR));

	return res;
}

static TCHAR *parseShortest(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return NULL;

	int iShort = 1;
	for (unsigned i = 2; i < ai->argc; i++)
	if (mir_tstrlen(ai->targv[i]) < mir_tstrlen(ai->targv[iShort]))
		iShort = i;

	return mir_tstrdup(ai->targv[iShort]);
}

static TCHAR *parseStrchr(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	char *szVal[34];
	memset(szVal, 0, sizeof(szVal));
	TCHAR *c = _tcschr(ai->targv[1], *ai->targv[2]);
	if (c == NULL || *c == 0)
		return mir_tstrdup(_T("0"));

	return itot(c - ai->targv[1] + 1);
}

static TCHAR *parseStrcmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if (mir_tstrcmp(ai->targv[1], ai->targv[2]))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseStrmcmp(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return NULL;

	ai->flags |= AIF_FALSE;
	for (unsigned i = 2; i < ai->argc; i++) {
		if (!mir_tstrcmp(ai->targv[1], ai->targv[i])) {
			ai->flags &= ~AIF_FALSE;
			break;
		}
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseStrncmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return NULL;

	int n = ttoi(ai->targv[3]);
	if (n <= 0)
		return NULL;

	if (_tcsncmp(ai->targv[1], ai->targv[2], n))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseStricmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if (mir_tstrcmpi(ai->targv[1], ai->targv[2]))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseStrnicmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return NULL;

	int n = ttoi(ai->targv[3]);
	if (n <= 0)
		return NULL;

	if (_tcsnicmp(ai->targv[1], ai->targv[2], n))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseStrrchr(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	TCHAR *c = _tcsrchr(ai->targv[1], *ai->targv[2]);
	if ((c == NULL) || (*c == 0))
		return mir_tstrdup(_T("0"));

	return itot(c - ai->targv[1] + 1);
}

static TCHAR *parseStrstr(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	TCHAR *c = _tcsstr(ai->targv[1], ai->targv[2]);
	if ((c == NULL) || (*c == 0))
		return mir_tstrdup(_T("0"));

	return itot(c - ai->targv[1] + 1);
}

static TCHAR *parseSubstr(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return NULL;

	int to, from = max(ttoi(ai->targv[2]) - 1, 0);
	if (ai->argc > 3)
		to = min(ttoi(ai->targv[3]), (int)mir_tstrlen(ai->targv[1]));
	else
		to = (int)mir_tstrlen(ai->targv[1]);

	if (to < from)
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((to - from + 1)*sizeof(TCHAR));
	memset(res, 0, ((to - from + 1) * sizeof(TCHAR)));
	_tcsncpy(res, ai->targv[1] + from, to - from);
	return res;
}

static TCHAR *parseSelect(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return NULL;

	int n = ttoi(ai->targv[1]);
	if ((n > (signed int)ai->argc - 2) || n <= 0)
		return NULL;

	return mir_tstrdup(ai->targv[n + 1]);
}

static TCHAR *parseSwitch(ARGUMENTSINFO *ai)
{
	if (ai->argc % 2 != 0)
		return NULL;

	for (unsigned i = 2; i < ai->argc; i += 2)
	if (!mir_tstrcmp(ai->targv[1], ai->targv[i]))
		return mir_tstrdup(ai->targv[i + 1]);

	return NULL;
}

static TCHAR *parseTrim(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *scur = ai->targv[1];
	while (*scur == ' ')
		scur++;

	TCHAR *ecur = ai->targv[1] + mir_tstrlen(ai->targv[1]) - 1;
	while ((*ecur == ' ') && (ecur > ai->targv[1]))
		ecur--;

	if (scur >= ecur)
		return mir_tstrdup(_T(""));

	TCHAR *res = (TCHAR*)mir_alloc((ecur - scur + 2)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((ecur - scur + 2) * sizeof(TCHAR)));
	_tcsncpy(res, scur, ecur - scur + 1);

	return res;
}

static TCHAR *parseTab(ARGUMENTSINFO *ai)
{
	int count = 1;
	if ((ai->argc == 2) && (mir_tstrlen(ai->targv[1]) > 0))
		count = ttoi(ai->targv[1]);

	if (count < 0)
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((count + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, (count + 1)*sizeof(TCHAR));
	TCHAR *cur = res;
	for (int i = 0; i < count; i++)
		*cur++ = '\t';

	return res;
}

static TCHAR *parseUpper(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *res = mir_tstrdup(ai->targv[1]);
	if (res == NULL)
		return NULL;

	return CharUpper(res);
}

static TCHAR *getNthWord(TCHAR *szString, int w)
{
	if (szString == NULL)
		return NULL;

	int count = 0;
	TCHAR *scur = szString;
	while (*scur == ' ')
		scur++;

	count++;
	while ((count < w) && (scur < szString + mir_tstrlen(szString))) {
		if (*scur == ' ') {
			while (*scur == ' ')
				scur++;

			count++;
		}
		if (count < w)
			scur++;
	}
	if (count != w)
		return NULL;

	TCHAR *ecur = scur;
	while ((*ecur != ' ') && (*ecur != 0))
		ecur++;

	TCHAR *res = (TCHAR*)mir_alloc((ecur - scur + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((ecur - scur + 1) * sizeof(TCHAR)));
	_tcsncpy(res, scur, ecur - scur);
	return res;
}

static TCHAR *parseWord(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3 || ai->argc > 4)
		return NULL;

	TCHAR *res = NULL;
	int to, from = ttoi(ai->targv[2]);
	if (ai->argc == 4) {
		if (mir_tstrlen(ai->targv[3]) > 0)
			to = ttoi(ai->targv[3]);
		else
			to = 100000; // rework
	}
	else to = from;

	if ((from == 0) || (to == 0) || (from > to))
		return NULL;

	for (int i = from; i <= to; i++) {
		TCHAR *szWord = getNthWord(ai->targv[1], i);
		if (szWord == NULL)
			return res;

		if (res != NULL) {
			TCHAR *pres = (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + mir_tstrlen(szWord) + 2)*sizeof(TCHAR));
			if (pres != NULL) {
				res = pres;
				mir_tstrcat(res, _T(" "));
				mir_tstrcat(res, szWord);
			}
			mir_free(szWord);
		}
		else res = szWord;
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

void registerStrTokens()
{
	registerIntToken(MIR_CAPS, parseCaps, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts each first letter of a word to uppercase, all others to lowercase"));
	registerIntToken(MIR_CAPS2, parseCaps2, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts each first letter of a word to uppercase"));
	registerIntToken(MIR_CRLF, parseCrlf, TRF_FUNCTION, LPGEN("String Functions")"\t()\t"LPGEN("inserts 'end of line' character"));
	registerIntToken(MIR_EXTRATEXT, parseExtratext, TRF_FIELD, LPGEN("String Functions")"\t"LPGEN("depends on calling plugin"));
	registerIntToken(MIR_EOL2CRLF, parseEolToCrlf, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("replace all occurrences of \\n (Unix) by \\r\\n (Windows)"));
	registerIntToken(MIR_FIXEOL, parseFixeol, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("cuts x after the first line and appends y (y is optional)"));
	registerIntToken(MIR_FIXEOL2, parseFixeol2, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("replaces all end of line characters by y (y is optional)"));
	registerIntToken(MIR_INSERT, parseInsert, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("inserts string y at position z in string x"));
	registerIntToken(MIR_LEFT, parseLeft, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("trims x to length y, keeping first y characters"));
	registerIntToken(MIR_LEN, parseLen, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("length of x"));
	registerIntToken(MIR_LINECOUNT, parseLineCount, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("the number of lines in string x"));
	registerIntToken(MIR_LONGEST, parseLongest, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("the longest string of the arguments"));
	registerIntToken(MIR_LOWER, parseLower, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts x to lowercase"));
	registerIntToken(MIR_NOOP, parseNoOp, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("no operation, x as given"));
	registerIntToken(MIR_PAD, parsePad, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y prepending character z (z is optional)"));
	registerIntToken(MIR_PADRIGHT, parsePadright, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y appending character z (z is optional)"));
	registerIntToken(MIR_PADCUT, parsePadcut, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y prepending character z, or cut if x is longer (z is optional)"));
	registerIntToken(MIR_PADCUTRIGHT, parsePadcutright, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("pads x to length y appending character z, or cut if x is longer (z is optional)"));
	registerIntToken(MIR_REPEAT, parseRepeat, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("repeats x y times"));
	registerIntToken(MIR_REPLACE, parseReplace, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z,...)\t"LPGEN("replace all occurrences of y in x with z, multiple y and z arguments allowed"));
	registerIntToken(MIR_RIGHT, parseRight, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("trims x to length y, keeping last y characters"));
	registerIntToken(MIR_SCROLL, parseScroll, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("moves string x, z characters to the left and trims it to y characters"));
	registerIntToken(MIR_STRCMP, parseStrcmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("TRUE if x equals y"));
	registerIntToken(MIR_STRMCMP, parseStrmcmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("TRUE if x equals any of the following arguments"));
	registerIntToken(MIR_STRNCMP, parseStrncmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("TRUE if the first z characters of x equal y"));
	registerIntToken(MIR_STRICMP, parseStricmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("TRUE if x equals y, ignoring case"));
	registerIntToken(MIR_STRNICMP, parseStrnicmp, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("TRUE if the first z characters of x equal y, ignoring case"));
	registerIntToken(MIR_SHORTEST, parseShortest, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("the shortest string of the arguments"));
	registerIntToken(MIR_STRCHR, parseStrchr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("location of first occurrence of character y in string x"));
	registerIntToken(MIR_STRRCHR, parseStrrchr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("location of last occurrence of character y in string x"));
	registerIntToken(MIR_STRSTR, parseStrstr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y)\t"LPGEN("location of first occurrence of string y in x"));
	registerIntToken(MIR_SUBSTR, parseSubstr, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("substring of x starting from position y to z"));
	registerIntToken(MIR_SELECT, parseSelect, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,...)\t"LPGEN("the xth string of the arguments"));
	registerIntToken(MIR_SWITCH, parseSwitch, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z,...)\t"LPGEN("z if y equals x, multiple y and z arguments allowed"));
	registerIntToken(MIR_TRIM, parseTrim, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("removes white spaces in before and after x"));
	registerIntToken(MIR_TAB, parseTab, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("inserts x tab characters (x is optional)"));
	registerIntToken(MIR_UPPER, parseUpper, TRF_FUNCTION, LPGEN("String Functions")"\t(x)\t"LPGEN("converts x to upper case"));
	registerIntToken(MIR_WORD, parseWord, TRF_FUNCTION, LPGEN("String Functions")"\t(x,y,z)\t"LPGEN("words (separated by white spaces) number y to z from string x (z is optional)"));
}
