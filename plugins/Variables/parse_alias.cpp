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
#include "parse_alias.h"

static CRITICAL_SECTION csAliasRegister;
static ALIASREGISTER *ar = NULL;
static unsigned int arCount = 0;

static ALIASREGISTER *searchAliasRegister(TCHAR *szAlias) {

	ALIASREGISTER *res;
	unsigned int i;

	res = NULL;
	if ( (szAlias == NULL) || (_tcslen(szAlias) == 0) ) {
		return NULL;
	}
	EnterCriticalSection(&csAliasRegister);
	for (i=0;i<arCount;i++) {
		if (!_tcscmp(ar[i].szAlias, szAlias)) {
			/* TODO: make a copy here? */
			res = &ar[i];
			LeaveCriticalSection(&csAliasRegister);
			return res;
		}
	}
	LeaveCriticalSection(&csAliasRegister);
	
	return NULL;
}

static TCHAR *replaceArguments(TCHAR *res, TCHAR *tArg, TCHAR *rArg) {

	unsigned int cur, ecur;

	if (_tcslen(tArg) == 0)
		return res;

	cur = ecur = 0;
	while (*(res+cur) != _T('\0')) {
		if ( (*(res+cur) == _T('(')) || (*(res+cur) == _T(',')) ) {
			ecur = ++cur;
			while ( (*(res+ecur) != _T(')')) && (*(res+ecur) != _T(',')) ) {
				ecur++;
			}
			if ( ((signed int)_tcslen(tArg) == (ecur-cur)) && (!_tcsncmp(tArg, res+cur, _tcslen(tArg))) ) {
				if (_tcslen(rArg) > _tcslen(tArg)) {
					res = ( TCHAR* )realloc(res, (_tcslen(res) + (_tcslen(rArg)-_tcslen(tArg)) + 1)*sizeof(TCHAR));
					if (res == NULL)
						return NULL;
				}
				MoveMemory(res+ecur+(_tcslen(rArg)-_tcslen(tArg)), res+ecur, (_tcslen(res+ecur)+1)*sizeof(TCHAR));
				_tcsncpy(res+cur, rArg, _tcslen(rArg));
			}
		}
		cur++;
	}

	return res;
}

static TCHAR *parseTranslateAlias(ARGUMENTSINFO *ai) {

	unsigned int i;
	TCHAR *res;
	ALIASREGISTER *areg;
	
	areg = searchAliasRegister(ai->targv[0]);
	if ( (areg == NULL) || (areg->argc != ai->argc-1) ) {
		return NULL;
	}
	res = _tcsdup(areg->szTranslation);
	for (i=0;i<areg->argc;i++) {
		res = replaceArguments(res, areg->argv[i], ai->targv[i+1]);
		if (res == NULL) {
			return NULL;
		}
	}

	return res;
}

static int addToAliasRegister(TCHAR *szAlias, unsigned int argc, TCHAR** argv, TCHAR *szTranslation) {

	unsigned int i, j;

	if ( szAlias == NULL || szTranslation == NULL || _tcslen(szAlias) == 0 )
		return -1;

	EnterCriticalSection(&csAliasRegister);
	for (i=0;i<arCount;i++) {
		if (!_tcscmp(ar[i].szAlias, szAlias)) {
			free(ar[i].szTranslation);
			ar[i].szTranslation = _tcsdup(szTranslation);
			for (j=0;j<ar[i].argc;j++) {
				if (ar[i].argv[j] != NULL) {
					free(ar[i].argv[j]);
				}
			}
			ar[i].argc = argc;
			ar[i].argv = ( TCHAR** )realloc(ar[i].argv, argc * sizeof(TCHAR *));
			if (ar[i].argv == NULL) {
				LeaveCriticalSection(&csAliasRegister);
				return -1;
			}
			for (j=0;j<argc;j++) {
				if (argv[j] != NULL)
					ar[i].argv[j] = _tcsdup(argv[j]);
				else
					ar[i].argv[j] = NULL;
			}
			LeaveCriticalSection(&csAliasRegister);
			return 0;
		}
	}
	ar = ( ALIASREGISTER* )realloc(ar, (arCount+1)*sizeof(ALIASREGISTER));
	if (ar == NULL) {
		LeaveCriticalSection(&csAliasRegister);
		return -1;
	}
	ar[arCount].szAlias = _tcsdup(szAlias);
	ar[arCount].szTranslation = _tcsdup(szTranslation);
	ar[arCount].argc = argc;
	ar[arCount].argv = ( TCHAR** )malloc(argc * sizeof(TCHAR *));
	if (ar[arCount].argv == NULL) {
		LeaveCriticalSection(&csAliasRegister);
		return -1;
	}
	for (j=0;j<ar[arCount].argc;j++) {
		if (argv[j] != NULL)
			ar[arCount].argv[j] = _tcsdup(argv[j]);
		else
			ar[arCount].argv[j] = NULL;
	}
	arCount += 1;
	LeaveCriticalSection(&csAliasRegister);

	return 0;
}

static TCHAR *parseAddAlias(ARGUMENTSINFO *ai) {

	int res;
	int argc, i;
	TCHAR *cur, *alias, **argv, *szArgs;
	char *szHelp, *szArgsA;

	if (ai->argc != 3)
		return NULL;

	cur = ai->targv[1];
	while (isValidTokenChar(*cur))
		cur++;

	alias = ( TCHAR* )calloc(((cur-ai->targv[1])+1), sizeof(TCHAR));
	if (alias == NULL)
		return NULL;

	_tcsncpy(alias, ai->targv[1], (cur-ai->targv[1]));
	getArguments(cur, &argv, &argc);
	deRegisterToken(alias);
	addToAliasRegister(alias, argc, argv, ai->targv[2]);
	szArgs = NULL;
	for (i=0;i<argc;i++) {
		if (i == 0)
			szArgs = ( TCHAR* )calloc( _tcslen(argv[i])+2, sizeof(TCHAR));
		else
			szArgs = ( TCHAR* )realloc(szArgs, (_tcslen(szArgs) + _tcslen(argv[i]) + 2)*sizeof(TCHAR));

		_tcscat(szArgs, argv[i]);
		if (i != argc-1)
			_tcscat(szArgs, _T(","));
	}
	if ( (szArgs != NULL) && (argc > 0) ) {
#ifdef UNICODE
		szArgsA = u2a(szArgs);
#else
		szArgsA = _strdup(szArgs);
#endif
		szHelp = ( char* )malloc(32 + strlen(szArgsA));
		memset(szHelp, '\0', 32 + strlen(szArgsA));
		sprintf(szHelp, "Alias\t(%s)\tuser defined", szArgsA);
		res = registerIntToken(alias, parseTranslateAlias, TRF_FUNCTION|TRF_UNPARSEDARGS, szHelp);
		free(szArgsA);
	}
	else {
		szHelp = ( char* )malloc(32);
		memset(szHelp, '\0', 32);
		sprintf(szHelp, "Alias\t\tuser defined");
		res = registerIntToken(alias, parseTranslateAlias, TRF_FIELD|TRF_UNPARSEDARGS, szHelp);
	}
	free(szArgs);
	free(szHelp);
	
	return res==0?_tcsdup(_T("")):NULL;
}

int registerAliasTokens()
{
	registerIntToken(_T(ADDALIAS), parseAddAlias, TRF_FUNCTION|TRF_UNPARSEDARGS, "Variables\t(x,y)\tstores y as alias named x");//TRF_UNPARSEDARGS);
	InitializeCriticalSection(&csAliasRegister);
	return 0;
}

void unregisterAliasTokens()
{
	DeleteCriticalSection(&csAliasRegister);
}

