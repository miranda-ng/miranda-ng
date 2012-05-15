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
#include "parse_variables.h"

// this is for get and put(s)
static CRITICAL_SECTION csVarRegister;
static VARIABLEREGISTER *vr = NULL;
static int vrCount = 0;

static int addToVariablesRegister(TCHAR *szName, TCHAR *szText) {

	int i;

	if ( (szName == NULL) || (szText == NULL) || (_tcslen(szName) <= 0) ) {
		return -1;
	}
	EnterCriticalSection(&csVarRegister);
	for (i=0;i<vrCount;i++) {
		if ( (!_tcscmp(vr[i].szName, szName)) ) { // && (vr[i].dwOwnerThread == GetCurrentThreadId()) ) {
			free(vr[i].szText);
			vr[i].szText = _tcsdup(szText);
			LeaveCriticalSection(&csVarRegister);
			
			return 0;
		}
	}
	vr = ( VARIABLEREGISTER* )realloc(vr, (vrCount+1)*sizeof(VARIABLEREGISTER));
	if (vr == NULL) {
		LeaveCriticalSection(&csVarRegister);
		return -1;
	}
	vr[vrCount].szName = _tcsdup(szName);
	vr[vrCount].szText = _tcsdup(szText);
	vr[vrCount].dwOwnerThread = GetCurrentThreadId();
	vrCount += 1;
	LeaveCriticalSection(&csVarRegister);

	return 0;
}

static TCHAR *searchVariableRegister(TCHAR *szName) {

	TCHAR *res;
	int i;

	res = NULL;
	if ( (szName == NULL) || (_tcslen(szName) <= 0) ) {
		return NULL;
	}
	EnterCriticalSection(&csVarRegister);
	for (i=0;i<vrCount;i++) {
		if ( (!_tcscmp(vr[i].szName, szName)) ) { // && (vr[i].dwOwnerThread == GetCurrentThreadId()) ) {
			res = _tcsdup(vr[i].szText);
			LeaveCriticalSection(&csVarRegister);
			return res;
		}
	}
	LeaveCriticalSection(&csVarRegister);
	
	return NULL;
}

int clearVariableRegister() {

	int i, count;	

	count = 0;
	EnterCriticalSection(&csVarRegister);
	for (i=0;i<vrCount;i++) {
		if (vr[i].dwOwnerThread == GetCurrentThreadId()) {
			free(vr[i].szName);
			free(vr[i].szText);
			if (vrCount > 1) {
				memcpy(&vr[i], &vr[vrCount-1], sizeof(VARIABLEREGISTER));
				vr = ( VARIABLEREGISTER* )realloc(vr, (vrCount-1)*sizeof(VARIABLEREGISTER));
				if (vr == NULL) {
					LeaveCriticalSection(&csVarRegister);
					return -1;
				}
				vrCount -= 1;
			}
			else {
				free(vr);
				vr = NULL;
				vrCount = 0;
			}
			count += 1;
		}
	}
	LeaveCriticalSection(&csVarRegister);

	return count;
}

static TCHAR *parsePut(ARGUMENTSINFO *ai) {

	FORMATINFO fi;

	if (ai->argc != 3) {
		return NULL;
	}
//	ai->flags |= AIF_DONTPARSE;
	if (addToVariablesRegister(ai->targv[1], ai->targv[2])) {
		return NULL;
	}

	memcpy(&fi, ai->fi, sizeof(fi));
	fi.tszFormat = ai->targv[2];
	fi.flags |= FIF_TCHAR;
	
	return formatString(&fi);
}

static TCHAR *parsePuts(ARGUMENTSINFO *ai) {

	if (ai->argc != 3) {
		return NULL;
	}
//	ai->flags |= AIF_DONTPARSE;
	if (addToVariablesRegister(ai->targv[1], ai->targv[2])) {
		return NULL;
	}

	return _tcsdup(_T(""));
}
	
static TCHAR *parseGet(ARGUMENTSINFO *ai) {

	TCHAR *szText;

	if (ai->argc != 2) {
		return NULL;
	}
//	ai->flags |= AIF_DONTPARSE;
	szText = searchVariableRegister(ai->targv[1]);
	if (szText == NULL) {
		return NULL;
	}

	return szText;
}

int registerVariablesTokens()
{
	registerIntToken(_T(GET), parseGet, TRF_FUNCTION, "Variables\t(x)\tvariable set by put(s) with name x");
	registerIntToken(_T(PUT), parsePut, TRF_FUNCTION, "Variables\t(x,y)\tx, and stores y as variable named x");//TRF_UNPARSEDARGS);
	registerIntToken(_T(PUTS), parsePuts, TRF_FUNCTION, "Variables\t(x,y)\tonly stores y as variables x");//TRF_UNPARSEDARGS);
	InitializeCriticalSection(&csVarRegister);

	return 0;
}

void unregisterVariablesTokens()
{
	DeleteCriticalSection(&csVarRegister);
}
