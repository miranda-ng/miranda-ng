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

#include "stdafx.h"

struct TokenRegisterEntry
{
	TOKENREGISTEREX tr;
	DWORD nameHash;
};

static int CompareTokens(const TokenRegisterEntry* p1, const TokenRegisterEntry* p2)
{
	if (p1->nameHash == p2->nameHash)
		return 0;

	return (p1->nameHash < p2->nameHash) ? -1 : 1;
}

static LIST<TokenRegisterEntry> tokens(100, CompareTokens);

static mir_cs csRegister;

unsigned long int hashlittle(void *key, size_t length, unsigned long int initval);

static DWORD NameHashFunction(wchar_t *tszStr)
{
	return (DWORD)hashlittle(tszStr, mir_wstrlen(tszStr)*sizeof(wchar_t), 0);
}

static TokenRegisterEntry* FindTokenRegisterByName(wchar_t *name)
{
	TokenRegisterEntry temp;
	temp.nameHash = NameHashFunction(name);
	return tokens.find(&temp);
}

int registerIntToken(wchar_t *szToken, wchar_t *(*parseFunction)(ARGUMENTSINFO *ai), int extraFlags, char* szHelpText)
{
	TOKENREGISTEREX tr = { 0 };
	tr.cbSize = sizeof(tr);
	tr.flags = TRF_FREEMEM | TRF_TCHAR | TRF_PARSEFUNC | extraFlags;
	//tr.memType = TR_MEM_VARIABLES;
	tr.memType = TR_MEM_MIRANDA;
	tr.szHelpText = szHelpText;
	tr.tszTokenString = szToken;
	tr.parseFunctionT = parseFunction;

	return registerToken(0, (LPARAM)&tr);
}

int deRegisterToken(wchar_t *token)
{
	if (token == NULL)
		return -1;

	TokenRegisterEntry *tre;
	{
		mir_cslock lck(csRegister);
		tre = FindTokenRegisterByName(token);
		if (tre == NULL)
			return -1;

		tokens.remove(tre);
	}

	if (!(tre->tr.flags & TRF_PARSEFUNC) && tre->tr.szService != NULL)
		mir_free(tre->tr.szService);

	if (tre->tr.tszTokenString != NULL)
		mir_free(tre->tr.tszTokenString);

	if (tre->tr.szHelpText != NULL)
		mir_free(tre->tr.szHelpText);

	if ((tre->tr.flags & TRF_CLEANUP) && !(tre->tr.flags & TRF_CLEANUPFUNC) && tre->tr.szCleanupService != NULL)
		mir_free(tre->tr.szCleanupService);

	mir_free(tre);
	return 0;
}

INT_PTR registerToken(WPARAM, LPARAM lParam)
{
	DWORD hash;

	TOKENREGISTEREX *newVr = (TOKENREGISTEREX*)lParam;
	if (newVr == NULL || newVr->szTokenString == NULL || newVr->cbSize <= 0)
		return -1;

	if (newVr->flags & TRF_TCHAR) {
		deRegisterToken(newVr->tszTokenString);
		hash = NameHashFunction(newVr->tszTokenString);
	}
	else {
		WCHAR *wtoken = mir_a2u(newVr->szTokenString);
		deRegisterToken(wtoken);
		hash = NameHashFunction(wtoken);
		mir_free(wtoken);
	}

	TokenRegisterEntry *tre = (TokenRegisterEntry*)mir_alloc(sizeof(TokenRegisterEntry));
	if (tre == NULL)
		return -1;

	memcpy(&tre->tr, newVr, newVr->cbSize);
	tre->nameHash = hash;
	if (!mir_wstrcmp(newVr->tszTokenString, L"alias"))
		log_debugA("alias");

	if (!(newVr->flags & TRF_PARSEFUNC) && newVr->szService != NULL)
		tre->tr.szService = mir_strdup(newVr->szService);

	if (newVr->flags & TRF_TCHAR)
		tre->tr.tszTokenString = mir_wstrdup(newVr->tszTokenString);
	else
		tre->tr.tszTokenString = mir_a2u(newVr->szTokenString);

	if (newVr->szHelpText != NULL)
		tre->tr.szHelpText = mir_strdup(newVr->szHelpText);

	if ((newVr->flags & TRF_CLEANUP) && !(newVr->flags & TRF_CLEANUPFUNC) && newVr->szCleanupService != NULL)
		tre->tr.szCleanupService = mir_strdup(newVr->szCleanupService);

	mir_cslock lck(csRegister);
	tokens.insert(tre);
	return 0;
}

TOKENREGISTEREX *searchRegister(wchar_t *tvar, int type)
{
	if (tvar == NULL)
		return 0;

	mir_cslock lck(csRegister);
	TokenRegisterEntry *tre = FindTokenRegisterByName(tvar);
	if (tre == NULL || (type != 0 && (tre->tr.flags & (TRF_FIELD | TRF_FUNCTION)) != 0 && !(tre->tr.flags & type)))
		return NULL;

	return &tre->tr;
}

wchar_t *parseFromRegister(ARGUMENTSINFO *ai)
{
	if (ai == NULL || ai->argc == 0 || ai->targv[0] == NULL)
		return NULL;

	INT_PTR callRes = 0;
	wchar_t *res = NULL;

	mir_cslock lck(csRegister);

	/* note the following limitation: you cannot add/remove tokens during a call from a different thread */
	TOKENREGISTEREX *thisVr = searchRegister(ai->targv[0], 0);
	if (thisVr == NULL)
		return NULL;

	TOKENREGISTEREX trCopy = *thisVr;

	// ai contains WCHARs, convert to chars because the tr doesn't support WCHARs
	if (!(thisVr->flags & TRF_TCHAR)) {
		// unicode variables calls a non-unicode plugin
		ARGUMENTSINFO cAi;
		memcpy(&cAi, ai, sizeof(ARGUMENTSINFO));
		cAi.argv = (char**)mir_alloc(ai->argc*sizeof(char *));
		for (unsigned j = 0; j < ai->argc; j++)
			cAi.argv[j] = mir_u2a(ai->targv[j]);

		if (thisVr->flags & TRF_PARSEFUNC)
			callRes = (INT_PTR)thisVr->parseFunction(&cAi);
		else if (thisVr->szService != NULL)
			callRes = CallService(thisVr->szService, 0, (LPARAM)&cAi);

		for (unsigned j = 0; j < cAi.argc; j++)
			mir_free(cAi.argv[j]);

		if ((char *)callRes != NULL)
			res = mir_a2u((char*)callRes);
	}
	else {
		// unicode variables calls unicode plugin
		if (thisVr->flags & TRF_PARSEFUNC)
			callRes = (INT_PTR)thisVr->parseFunctionT(ai);
		else if (thisVr->szService != NULL)
			callRes = CallService(thisVr->szService, 0, (LPARAM)ai);

		if ((wchar_t*)callRes != NULL)
			res = mir_wstrdup((wchar_t*)callRes);
	}

	if (callRes != NULL) {
		if (trCopy.flags & TRF_CLEANUP) {
			if (trCopy.flags & TRF_CLEANUPFUNC)
				trCopy.cleanupFunctionT((wchar_t*)callRes);
			else if (trCopy.szCleanupService != NULL)
				CallService(trCopy.szCleanupService, 0, (LPARAM)callRes);
		}
		if ((trCopy.flags & TRF_FREEMEM) && trCopy.memType == TR_MEM_MIRANDA)
			mir_free((void*)callRes);
	}
	return res;
}

TOKENREGISTEREX* getTokenRegister(int i)
{
	mir_cslock lck(csRegister);
	return (i >= tokens.getCount() || i < 0) ? NULL : &tokens[i]->tr;
}

int getTokenRegisterCount()
{
	mir_cslock lck(csRegister);
	return tokens.getCount();
}

/////////////////////////////////////////////////////////////////////////////////////////

int initTokenRegister()
{
	return 0;
}

int deinitTokenRegister()
{
	for (int i = 0; i < tokens.getCount(); i++) {
		TokenRegisterEntry *tre = tokens[i];
		if (!(tre->tr.flags & TRF_PARSEFUNC) && tre->tr.szService != NULL)
			mir_free(tre->tr.szService);

		if (tre->tr.tszTokenString != NULL)
			mir_free(tre->tr.tszTokenString);

		if (tre->tr.szHelpText != NULL)
			mir_free(tre->tr.szHelpText);

		if ((tre->tr.flags & TRF_CLEANUP) && !(tre->tr.flags & TRF_CLEANUPFUNC) && tre->tr.szCleanupService != NULL)
			mir_free(tre->tr.szCleanupService);

		mir_free(tre);
	}
	tokens.destroy();

	return 0;
}
