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

struct ALIASREGISTER
{
	TCHAR *szAlias;
	unsigned int argc;
	TCHAR **argv;
	TCHAR *szTranslation;
};

static LIST<ALIASREGISTER> arAliases(5);
static CRITICAL_SECTION csAliasRegister;

static ALIASREGISTER *searchAliasRegister(TCHAR *szAlias)
{
	if (szAlias == NULL || *szAlias == 0)
		return NULL;

	mir_cslock lck(csAliasRegister);
	for (int i = 0; i < arAliases.getCount(); i++)
	if (!_tcscmp(arAliases[i]->szAlias, szAlias))
		return arAliases[i];

	return NULL;
}

static TCHAR *replaceArguments(TCHAR *res, TCHAR *tArg, TCHAR *rArg)
{
	if (_tcslen(tArg) == 0)
		return res;

	unsigned int cur = 0, ecur = 0;
	while (*(res + cur) != 0) {
		if ((*(res + cur) == '(') || (*(res + cur) == ',')) {
			ecur = ++cur;
			while ((*(res + ecur) != ')') && (*(res + ecur) != ','))
				ecur++;

			if (((signed int)_tcslen(tArg) == (ecur - cur)) && (!_tcsncmp(tArg, res + cur, _tcslen(tArg)))) {
				if (_tcslen(rArg) > _tcslen(tArg)) {
					res = (TCHAR*)mir_realloc(res, (_tcslen(res) + (_tcslen(rArg) - _tcslen(tArg)) + 1)*sizeof(TCHAR));
					if (res == NULL)
						return NULL;
				}
				memmove(res + ecur + (_tcslen(rArg) - _tcslen(tArg)), res + ecur, (_tcslen(res + ecur) + 1)*sizeof(TCHAR));
				_tcsncpy(res + cur, rArg, _tcslen(rArg));
			}
		}
		cur++;
	}

	return res;
}

static TCHAR *parseTranslateAlias(ARGUMENTSINFO *ai)
{
	ALIASREGISTER *areg = searchAliasRegister(ai->targv[0]);
	if (areg == NULL || areg->argc != ai->argc - 1)
		return NULL;

	TCHAR *res = mir_tstrdup(areg->szTranslation);
	for (unsigned i = 0; i < areg->argc; i++) {
		res = replaceArguments(res, areg->argv[i], ai->targv[i + 1]);
		if (res == NULL)
			return NULL;
	}

	return res;
}

static int addToAliasRegister(TCHAR *szAlias, unsigned int argc, TCHAR** argv, TCHAR *szTranslation)
{
	if (szAlias == NULL || szTranslation == NULL || _tcslen(szAlias) == 0)
		return -1;

	mir_cslock lck(csAliasRegister);
	for (int i = 0; i < arAliases.getCount(); i++) {
		ALIASREGISTER *p = arAliases[i];
		if (_tcscmp(p->szAlias, szAlias))
			continue;

		mir_free(p->szTranslation);
		p->szTranslation = mir_tstrdup(szTranslation);
		for (unsigned j = 0; j < p->argc; j++)
			mir_free(p->argv[j]);

		p->argc = argc;
		p->argv = (TCHAR**)mir_realloc(p->argv, argc * sizeof(TCHAR*));
		if (p->argv == NULL)
			return -1;

		for (unsigned j = 0; j < argc; j++) {
			if (argv[j] != NULL)
				p->argv[j] = mir_tstrdup(argv[j]);
			else
				p->argv[j] = NULL;
		}
		return 0;
	}

	ALIASREGISTER *p = new ALIASREGISTER;
	p->szAlias = mir_tstrdup(szAlias);
	p->szTranslation = mir_tstrdup(szTranslation);
	p->argc = argc;
	p->argv = (TCHAR**)mir_alloc(argc * sizeof(TCHAR*));
	if (p->argv == NULL)
		return -1;

	for (unsigned j = 0; j < p->argc; j++) {
		if (argv[j] != NULL)
			p->argv[j] = mir_tstrdup(argv[j]);
		else
			p->argv[j] = NULL;
	}
	arAliases.insert(p);
	return 0;
}

static TCHAR *parseAddAlias(ARGUMENTSINFO *ai)
{
	int res;
	char *szHelp, *szArgsA;

	if (ai->argc != 3)
		return NULL;

	TCHAR *cur = ai->targv[1];
	while (isValidTokenChar(*cur))
		cur++;

	ptrT alias(mir_tstrndup(ai->targv[1], cur - ai->targv[1]));

	int argc;
	TCHAR **argv;
	getArguments(cur, &argv, &argc);
	deRegisterToken(alias);
	addToAliasRegister(alias, argc, argv, ai->targv[2]);
	TCHAR *szArgs = NULL;
	for (int i = 0; i < argc; i++) {
		if (i == 0)
			szArgs = (TCHAR*)mir_calloc((_tcslen(argv[i]) + 2)*sizeof(TCHAR));
		else
			szArgs = (TCHAR*)mir_realloc(szArgs, (_tcslen(szArgs) + _tcslen(argv[i]) + 2)*sizeof(TCHAR));

		_tcscat(szArgs, argv[i]);
		if (i != argc - 1)
			_tcscat(szArgs, _T(","));
	}
	if (szArgs != NULL && argc > 0) {
		szArgsA = mir_t2a(szArgs);

		size_t size = 32 + strlen(szArgsA);
		szHelp = (char *)mir_alloc(size);
		memset(szHelp, '\0', 32 + strlen(szArgsA));
		mir_snprintf(szHelp, size, LPGEN("Alias")"\t(%s)\t"LPGEN("user defined"), szArgsA);
		res = registerIntToken(alias, parseTranslateAlias, TRF_FUNCTION | TRF_UNPARSEDARGS, szHelp);
		mir_free(szArgsA);
	}
	else {
		szHelp = (char*)mir_alloc(32);
		memset(szHelp, '\0', 32);
		mir_snprintf(szHelp, 32, LPGEN("Alias")"\t\t"LPGEN("user defined"));
		res = registerIntToken(alias, parseTranslateAlias, TRF_FIELD | TRF_UNPARSEDARGS, szHelp);
	}
	mir_free(szArgs);
	mir_free(szHelp);
	return (res == 0) ? mir_tstrdup(_T("")) : NULL;
}

void registerAliasTokens()
{
	registerIntToken(ADDALIAS, parseAddAlias, TRF_FUNCTION | TRF_UNPARSEDARGS, LPGEN("Variables")"\t(x,y)\t"LPGEN("stores y as alias named x"));//TRF_UNPARSEDARGS);
	InitializeCriticalSection(&csAliasRegister);
}

void unregisterAliasTokens()
{
	DeleteCriticalSection(&csAliasRegister);

	for (int i = 0; i < arAliases.getCount(); i++) {
		ALIASREGISTER *p = arAliases[i];
		for (unsigned j = 0; j < p->argc; j++)
			mir_free(p->argv[j]);
		mir_free(p->argv);
		mir_free(p->szAlias);
		mir_free(p->szTranslation);
		delete p;
	}
}
