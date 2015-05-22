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

// this is for get and put(s)
static mir_cs csVarRegister;
static VARIABLEREGISTER *vr = NULL;
static int vrCount = 0;

static int addToVariablesRegister(TCHAR *szName, TCHAR *szText)
{
	if ((szName == NULL) || (szText == NULL) || (mir_tstrlen(szName) <= 0))
		return -1;

	mir_cslock lck(csVarRegister);
	for (int i = 0; i < vrCount; i++) {
		if ((!_tcscmp(vr[i].szName, szName))) {
			mir_free(vr[i].szText);
			vr[i].szText = mir_tstrdup(szText);
			return 0;
		}
	}
	VARIABLEREGISTER *pvr = (VARIABLEREGISTER*)mir_realloc(vr, (vrCount + 1)*sizeof(VARIABLEREGISTER));
	if (pvr == NULL)
		return -1;

	vr = pvr;
	vr[vrCount].szName = mir_tstrdup(szName);
	vr[vrCount].szText = mir_tstrdup(szText);
	vr[vrCount++].dwOwnerThread = GetCurrentThreadId();
	return 0;
}

static TCHAR *searchVariableRegister(TCHAR *szName)
{
	if ((szName == NULL) || (mir_tstrlen(szName) <= 0))
		return NULL;

	mir_cslock lck(csVarRegister);
	for (int i = 0; i < vrCount; i++)
		if ((!_tcscmp(vr[i].szName, szName)))
			return mir_tstrdup(vr[i].szText);

	return NULL;
}

static TCHAR *parsePut(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	//	ai->flags |= AIF_DONTPARSE;
	if (addToVariablesRegister(ai->targv[1], ai->targv[2]))
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.tszFormat = ai->targv[2];
	fi.flags |= FIF_TCHAR;
	return formatString(&fi);
}

static TCHAR *parsePuts(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if (addToVariablesRegister(ai->targv[1], ai->targv[2]))
		return NULL;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseGet(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	return searchVariableRegister(ai->targv[1]);
}

void registerVariablesTokens()
{
	registerIntToken(GET, parseGet, TRF_FUNCTION, LPGEN("Variables")"\t(x)\t"LPGEN("variable set by put(s) with name x"));
	registerIntToken(PUT, parsePut, TRF_FUNCTION, LPGEN("Variables")"\t(x,y)\t"LPGEN("x, and stores y as variable named x"));//TRF_UNPARSEDARGS);
	registerIntToken(PUTS, parsePuts, TRF_FUNCTION, LPGEN("Variables")"\t(x,y)\t"LPGEN("only stores y as variables x"));//TRF_UNPARSEDARGS);
}

void unregisterVariablesTokens()
{
	mir_cslock lck(csVarRegister);
	for (int i = 0; i < vrCount; i++) {
		mir_free(vr[i].szName);
		mir_free(vr[i].szText);
	}
	mir_free(vr);
	vr = NULL;
	vrCount = 0;
}
