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
#include "parse_regexp.h"
#define PCRE_STATIC
#include "pcre.h"

/*
	pattern, subject
*/
static TCHAR *parseRegExpCheck(ARGUMENTSINFO *ai) {

	const char *err;
	int erroffset;
	char szVal[34], *arg1, *arg2;
	int offsets[99];

	if (ai->argc != 3) {
		return NULL;
	}
	ai->flags = AIF_FALSE;

	arg1 = mir_t2a(ai->targv[1]);
	arg2 = mir_t2a(ai->targv[2]);

	pcre *ppat = pcre_compile(arg1, 0, &err, &erroffset, NULL);
	if (ppat == NULL) {
		mir_free(arg1);
		mir_free(arg2);
		return NULL;
	}
	pcre_extra *extra = pcre_study(ppat, 0, &err);
	int nmat = pcre_exec(ppat, extra, arg2, strlen(arg2), 0, 0, offsets, 99);
	mir_free(arg1);
	mir_free(arg2);
	if (nmat > 0) {
		ai->flags &= ~AIF_FALSE;
		_ltoa(nmat, szVal, 10);

		TCHAR *res = mir_a2t(szVal);

		return res;
	}

	return mir_tstrdup(_T("0"));
}

/*
	pattern, subject, substring no (== PCRE string no (starting at 0))
*/
static TCHAR *parseRegExpSubstr(ARGUMENTSINFO *ai) {

	const char *err, *substring;
	char *res, *arg1, *arg2, *arg3;
	int erroffset, number;
	int offsets[99];
	TCHAR *tres;

	if (ai->argc != 4) {
		return NULL;
	}

	arg1 = mir_t2a(ai->targv[1]);
	arg2 = mir_t2a(ai->targv[2]);
	arg3 = mir_t2a(ai->targv[3]);

	number = atoi(arg3);
	if (number < 0) {
		mir_free(arg1);
		mir_free(arg2);
		mir_free(arg3);
		return NULL;
	}
	ai->flags = AIF_FALSE;	
	pcre *ppat = pcre_compile(arg1, 0, &err, &erroffset, NULL);
	if (ppat == NULL) {
		mir_free(arg1);
		mir_free(arg2);
		mir_free(arg3);
		return NULL;
	}
	pcre_extra *extra = pcre_study(ppat, 0, &err);
	int nmat = pcre_exec(ppat, extra, arg2, strlen(arg2), 0, 0, offsets, 99);
	if (nmat >= 0) {
		ai->flags &= ~AIF_FALSE;
	}
	if (pcre_get_substring(arg2, offsets, nmat, number, &substring) < 0) {
		ai->flags |= AIF_FALSE;
	}
	else {
		res = mir_strdup(substring);
		pcre_free_substring(substring);


		tres = mir_a2t(res);

		mir_free(res);
		mir_free(arg1);
		mir_free(arg2);
		mir_free(arg3);

		return tres;
	}
	mir_free(arg1);
	mir_free(arg2);
	mir_free(arg3);

	return mir_tstrdup(_T(""));
}

int registerRegExpTokens() {

	registerIntToken(_T(REGEXPCHECK), parseRegExpCheck, TRF_FUNCTION, LPGEN("Regular Expressions")"\t(x,y)\t"LPGEN("(ANSI input only) the number of substring matches found in y with pattern x"));
	registerIntToken(_T(REGEXPSUBSTR), parseRegExpSubstr, TRF_FUNCTION, LPGEN("Regular Expressions")"\t(x,y,z)\t"LPGEN("(ANSI input only) substring match number z found in subject y with pattern x"));


	return 0;
}