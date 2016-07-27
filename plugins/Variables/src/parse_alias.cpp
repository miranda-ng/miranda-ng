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

#include "stdafx.h"

struct ALIASREGISTER
{
	wchar_t *szAlias;
	unsigned int argc;
	wchar_t **argv;
	wchar_t *szTranslation;
};

static LIST<ALIASREGISTER> arAliases(5);
static mir_cs csAliasRegister;

static ALIASREGISTER* searchAliasRegister(wchar_t *szAlias)
{
	if (szAlias == NULL || *szAlias == 0)
		return NULL;

	mir_cslock lck(csAliasRegister);
	for (int i = 0; i < arAliases.getCount(); i++)
	if (!mir_wstrcmp(arAliases[i]->szAlias, szAlias))
		return arAliases[i];

	return NULL;
}

static wchar_t *replaceArguments(wchar_t *res, wchar_t *tArg, wchar_t *rArg)
{
	if (mir_wstrlen(tArg) == 0)
		return res;

	unsigned int cur = 0, ecur = 0;
	while (*(res + cur) != 0) {
		if ((*(res + cur) == '(') || (*(res + cur) == ',')) {
			ecur = ++cur;
			while ((*(res + ecur) != ')') && (*(res + ecur) != ','))
				ecur++;

			if (((signed int)mir_wstrlen(tArg) == (ecur - cur)) && (!wcsncmp(tArg, res + cur, mir_wstrlen(tArg)))) {
				if (mir_wstrlen(rArg) > mir_wstrlen(tArg)) {
					res = (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + (mir_wstrlen(rArg) - mir_wstrlen(tArg)) + 1)*sizeof(wchar_t));
					if (res == NULL)
						return NULL;
				}
				memmove(res + ecur + (mir_wstrlen(rArg) - mir_wstrlen(tArg)), res + ecur, (mir_wstrlen(res + ecur) + 1)*sizeof(wchar_t));
				wcsncpy(res + cur, rArg, mir_wstrlen(rArg));
			}
		}
		cur++;
	}

	return res;
}

static wchar_t *parseTranslateAlias(ARGUMENTSINFO *ai)
{
	ALIASREGISTER *areg = searchAliasRegister(ai->targv[0]);
	if (areg == NULL || areg->argc != ai->argc - 1)
		return NULL;

	wchar_t *res = mir_wstrdup(areg->szTranslation);
	for (unsigned i = 0; i < areg->argc; i++) {
		res = replaceArguments(res, areg->argv[i], ai->targv[i + 1]);
		if (res == NULL)
			return NULL;
	}

	return res;
}

static int addToAliasRegister(wchar_t *szAlias, unsigned int argc, wchar_t** argv, wchar_t *szTranslation)
{
	if (szAlias == NULL || szTranslation == NULL || mir_wstrlen(szAlias) == 0)
		return -1;

	mir_cslock lck(csAliasRegister);
	for (int i = 0; i < arAliases.getCount(); i++) {
		ALIASREGISTER *p = arAliases[i];
		if (mir_wstrcmp(p->szAlias, szAlias))
			continue;

		mir_free(p->szTranslation);
		p->szTranslation = mir_wstrdup(szTranslation);
		for (unsigned j = 0; j < p->argc; j++)
			mir_free(p->argv[j]);

		p->argc = argc;
		p->argv = (wchar_t**)mir_realloc(p->argv, argc * sizeof(wchar_t*));
		if (p->argv == NULL)
			return -1;

		for (unsigned j = 0; j < argc; j++) {
			if (argv[j] != NULL)
				p->argv[j] = mir_wstrdup(argv[j]);
			else
				p->argv[j] = NULL;
		}
		return 0;
	}
	wchar_t **pargv = (wchar_t**)mir_alloc(argc * sizeof(wchar_t*));
	if (pargv == NULL)
		return -1;

	ALIASREGISTER *p = new ALIASREGISTER;
	p->szAlias = mir_wstrdup(szAlias);
	p->szTranslation = mir_wstrdup(szTranslation);
	p->argc = argc;
	p->argv = pargv;

	for (unsigned j = 0; j < p->argc; j++) {
		if (argv[j] != NULL)
			p->argv[j] = mir_wstrdup(argv[j]);
		else
			p->argv[j] = NULL;
	}
	arAliases.insert(p);
	return 0;
}

static wchar_t *parseAddAlias(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	char *szHelp, *szArgsA;
	wchar_t *cur = ai->targv[1];
	while (isValidTokenChar(*cur))
		cur++;

	ptrW alias(mir_wstrndup(ai->targv[1], cur - ai->targv[1]));

	TArgList argv;
	getArguments(cur, argv);

	deRegisterToken(alias);
	addToAliasRegister(alias, argv.getCount(), argv.getArray(), ai->targv[2]);
	wchar_t *szArgs = NULL;
	for (int i = 0; i < argv.getCount(); i++) {
		if (i == 0)
			szArgs = (wchar_t*)mir_calloc((mir_wstrlen(argv[i]) + 2)*sizeof(wchar_t));
		else
			szArgs = (wchar_t*)mir_realloc(szArgs, (mir_wstrlen(szArgs) + mir_wstrlen(argv[i]) + 2)*sizeof(wchar_t));

		mir_wstrcat(szArgs, argv[i]);
		if (i != argv.getCount() - 1)
			mir_wstrcat(szArgs, L",");
	}
	int res;
	if (szArgs != NULL && argv.getCount() > 0) {
		szArgsA = mir_u2a(szArgs);

		size_t size = 32 + mir_strlen(szArgsA);
		szHelp = (char *)mir_alloc(size);
		memset(szHelp, '\0', 32 + mir_strlen(szArgsA));
		mir_snprintf(szHelp, size, LPGEN("Alias") "\t(%s)\t" LPGEN("user defined"), szArgsA);
		res = registerIntToken(alias, parseTranslateAlias, TRF_FUNCTION | TRF_UNPARSEDARGS, szHelp);
		mir_free(szArgsA);
	}
	else {
		szHelp = (char*)mir_alloc(32);
		memset(szHelp, '\0', 32);
		mir_snprintf(szHelp, 32, LPGEN("Alias") "\t\t" LPGEN("user defined"));
		res = registerIntToken(alias, parseTranslateAlias, TRF_FIELD | TRF_UNPARSEDARGS, szHelp);
	}
	mir_free(szArgs);
	mir_free(szHelp);
	argv.destroy();
	return (res == 0) ? mir_wstrdup(L"") : NULL;
}

void registerAliasTokens()
{
	registerIntToken(ADDALIAS, parseAddAlias, TRF_FUNCTION | TRF_UNPARSEDARGS, LPGEN("Variables") "\t(x,y)\t" LPGEN("stores y as alias named x"));//TRF_UNPARSEDARGS);
}

void unregisterAliasTokens()
{
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
