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
	uint32_t nameHash;
};

static int CompareTokens(const TokenRegisterEntry* p1, const TokenRegisterEntry* p2)
{
	if (p1->nameHash == p2->nameHash)
		return 0;

	return (p1->nameHash < p2->nameHash) ? -1 : 1;
}

static LIST<TokenRegisterEntry> tokens(100, CompareTokens);

static mir_cs csRegister;

static TokenRegisterEntry* FindTokenRegisterByName(wchar_t *name)
{
	TokenRegisterEntry temp;
	temp.nameHash = mir_hashstrW(name);
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
	tr.szTokenString.w = szToken;
	tr.parseFunctionW = parseFunction;

	return registerToken(0, (LPARAM)&tr);
}

int deRegisterToken(wchar_t *token)
{
	if (token == nullptr)
		return -1;

	TokenRegisterEntry *tre;
	{
		mir_cslock lck(csRegister);
		tre = FindTokenRegisterByName(token);
		if (tre == nullptr)
			return -1;

		tokens.remove(tre);
	}

	if (!(tre->tr.flags & TRF_PARSEFUNC) && tre->tr.szService != nullptr)
		mir_free(tre->tr.szService);

	if (tre->tr.szTokenString.w != nullptr)
		mir_free(tre->tr.szTokenString.w);

	if (tre->tr.szHelpText != nullptr)
		mir_free(tre->tr.szHelpText);

	if ((tre->tr.flags & TRF_CLEANUP) && !(tre->tr.flags & TRF_CLEANUPFUNC) && tre->tr.szCleanupService != nullptr)
		mir_free(tre->tr.szCleanupService);

	mir_free(tre);
	return 0;
}

INT_PTR registerToken(WPARAM, LPARAM lParam)
{
	uint32_t hash;

	TOKENREGISTEREX *newVr = (TOKENREGISTEREX*)lParam;
	if (newVr == nullptr || newVr->szTokenString.w == nullptr || newVr->cbSize <= 0)
		return -1;

	if (newVr->flags & TRF_TCHAR) {
		deRegisterToken(newVr->szTokenString.w);
		hash = mir_hashstrW(newVr->szTokenString.w);
	}
	else {
		wchar_t *wtoken = mir_a2u(newVr->szTokenString.a);
		deRegisterToken(wtoken);
		hash = mir_hashstrW(wtoken);
		mir_free(wtoken);
	}

	TokenRegisterEntry *tre = (TokenRegisterEntry*)mir_alloc(sizeof(TokenRegisterEntry));
	if (tre == nullptr)
		return -1;

	memcpy(&tre->tr, newVr, newVr->cbSize);
	tre->nameHash = hash;
	if (!mir_wstrcmp(newVr->szTokenString.w, L"alias"))
		log_debug(0, "alias");

	if (!(newVr->flags & TRF_PARSEFUNC) && newVr->szService != nullptr)
		tre->tr.szService = mir_strdup(newVr->szService);

	if (newVr->flags & TRF_TCHAR)
		tre->tr.szTokenString.w = mir_wstrdup(newVr->szTokenString.w);
	else
		tre->tr.szTokenString.w = mir_a2u(newVr->szTokenString.a);

	if (newVr->szHelpText != nullptr)
		tre->tr.szHelpText = mir_strdup(newVr->szHelpText);

	if ((newVr->flags & TRF_CLEANUP) && !(newVr->flags & TRF_CLEANUPFUNC) && newVr->szCleanupService != nullptr)
		tre->tr.szCleanupService = mir_strdup(newVr->szCleanupService);

	mir_cslock lck(csRegister);
	tokens.insert(tre);
	return 0;
}

TOKENREGISTEREX *searchRegister(wchar_t *tvar, int type)
{
	if (tvar == nullptr)
		return nullptr;

	mir_cslock lck(csRegister);
	TokenRegisterEntry *tre = FindTokenRegisterByName(tvar);
	if (tre == nullptr || (type != 0 && (tre->tr.flags & (TRF_FIELD | TRF_FUNCTION)) != 0 && !(tre->tr.flags & type)))
		return nullptr;

	return &tre->tr;
}

wchar_t *parseFromRegister(ARGUMENTSINFO *ai)
{
	if (ai == nullptr || ai->argc == 0 || ai->argv.w[0] == nullptr)
		return nullptr;

	INT_PTR callRes = 0;
	wchar_t *res = nullptr;

	mir_cslock lck(csRegister);

	/* note the following limitation: you cannot add/remove tokens during a call from a different thread */
	TOKENREGISTEREX *thisVr = searchRegister(ai->argv.w[0], 0);
	if (thisVr == nullptr)
		return nullptr;

	TOKENREGISTEREX trCopy = *thisVr;

	// ai contains WCHARs, convert to chars because the tr doesn't support WCHARs
	if (!(thisVr->flags & TRF_TCHAR)) {
		// unicode variables calls a non-unicode plugin
		ARGUMENTSINFO cAi;
		memcpy(&cAi, ai, sizeof(ARGUMENTSINFO));
		cAi.argv.a = (char**)mir_alloc(ai->argc*sizeof(char *));
		for (unsigned j = 0; j < ai->argc; j++)
			cAi.argv.a[j] = mir_u2a(ai->argv.w[j]);

		if (thisVr->flags & TRF_PARSEFUNC)
			callRes = (INT_PTR)thisVr->parseFunction(&cAi);
		else if (thisVr->szService != nullptr) {
			callRes = CallService(thisVr->szService, 0, (LPARAM)&cAi);
			if (callRes == CALLSERVICE_NOTFOUND)
				callRes = 0;
		}

		for (unsigned j = 0; j < cAi.argc; j++)
			mir_free(cAi.argv.a[j]);

		if ((char *)callRes != nullptr)
			res = mir_a2u((char*)callRes);
	}
	else {
		// unicode variables calls unicode plugin
		if (thisVr->flags & TRF_PARSEFUNC)
			callRes = (INT_PTR)thisVr->parseFunctionW(ai);
		else if (thisVr->szService != nullptr) {
			callRes = CallService(thisVr->szService, 0, (LPARAM)ai);
			if (callRes == CALLSERVICE_NOTFOUND)
				callRes = 0;
		}

		if ((wchar_t*)callRes != nullptr)
			res = mir_wstrdup((wchar_t*)callRes);
	}

	if (callRes != NULL) {
		if (trCopy.flags & TRF_CLEANUP) {
			if (trCopy.flags & TRF_CLEANUPFUNC)
				trCopy.cleanupFunctionW((wchar_t*)callRes);
			else if (trCopy.szCleanupService != nullptr)
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
	return (i >= tokens.getCount() || i < 0) ? nullptr : &tokens[i]->tr;
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
	for (auto &tre : tokens) {
		if (!(tre->tr.flags & TRF_PARSEFUNC) && tre->tr.szService != nullptr)
			mir_free(tre->tr.szService);

		if (tre->tr.szTokenString.w != nullptr)
			mir_free(tre->tr.szTokenString.w);

		if (tre->tr.szHelpText != nullptr)
			mir_free(tre->tr.szHelpText);

		if ((tre->tr.flags & TRF_CLEANUP) && !(tre->tr.flags & TRF_CLEANUPFUNC) && tre->tr.szCleanupService != nullptr)
			mir_free(tre->tr.szCleanupService);

		mir_free(tre);
	}
	tokens.destroy();

	return 0;
}
