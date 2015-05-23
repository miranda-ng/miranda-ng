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

static TCHAR *parseAnd(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	for (unsigned i = 1; i < ai->argc; i++) {
		fi.tszFormat = ai->targv[i];
		mir_free(formatString(&fi));

		if (fi.eCount > 0) {
			ai->flags |= AIF_FALSE;
			return mir_tstrdup(_T(""));
		}
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseFalse(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_FALSE;
	return mir_tstrdup(_T(""));
}

static TCHAR *parseIf(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.eCount = fi.pCount = 0;
	fi.tszFormat = ai->targv[1];
	mir_free(formatString(&fi));

	return mir_tstrdup((fi.eCount == 0) ? ai->targv[2] : ai->targv[3]);
}

static TCHAR *parseIf2(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.eCount = fi.pCount = 0;
	fi.tszFormat = ai->targv[1];
	TCHAR *szCondition = formatString(&fi);
	if (fi.eCount == 0)
		return szCondition;

	mir_free(szCondition);
	return mir_tstrdup(ai->targv[2]);
}

static TCHAR *parseIf3(ARGUMENTSINFO *ai)
{
	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	for (unsigned i = 1; i < ai->argc; i++) {
		fi.eCount = fi.pCount = 0;
		fi.tszFormat = ai->targv[i];
		TCHAR *szCondition = formatString(&fi);
		if (fi.eCount == 0)
			return szCondition;

		mir_free(szCondition);
	}

	return NULL;
}

static TCHAR *parseIfequal(ARGUMENTSINFO *ai)
{
	if (ai->argc != 5)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.szFormat = ai->argv[1];
	ptrT tszFirst(formatString(&fi));
	fi.szFormat = ai->argv[2];
	ptrT tszSecond(formatString(&fi));
	if (tszFirst == NULL || tszSecond == NULL)
		return NULL;

	if (ttoi(tszFirst) == ttoi(tszSecond))
		return mir_tstrdup(ai->targv[3]);

	return mir_tstrdup(ai->targv[4]);
}

static TCHAR *parseIfgreater(ARGUMENTSINFO *ai)
{
	if (ai->argc != 5)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.szFormat = ai->argv[1];
	ptrT tszFirst(formatString(&fi));
	fi.szFormat = ai->argv[2];
	ptrT tszSecond(formatString(&fi));
	if (tszFirst == NULL || tszSecond == NULL)
		return NULL;

	if (ttoi(tszFirst) > ttoi(tszSecond))
		return mir_tstrdup(ai->targv[3]);

	return mir_tstrdup(ai->targv[4]);
}

static TCHAR *parseIflonger(ARGUMENTSINFO *ai)
{
	if (ai->argc != 5)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.szFormat = ai->argv[1];
	ptrT tszFirst(formatString(&fi));
	fi.szFormat = ai->argv[2];
	ptrT tszSecond(formatString(&fi));
	if (tszFirst == NULL || tszSecond == NULL)
		return NULL;

	if (mir_tstrlen(tszFirst) > mir_tstrlen(tszSecond))
		return mir_tstrdup(ai->targv[3]);

	return mir_tstrdup(ai->targv[4]);
}

/*

  ?for(init, cond, incr, show)

  */
static TCHAR *parseFor(ARGUMENTSINFO *ai)
{
	if (ai->argc != 5)
		return NULL;

	TCHAR *res = mir_tstrdup(_T(""));

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.eCount = fi.pCount = 0;
	fi.tszFormat = ai->targv[1];
	mir_free(formatString(&fi));
	fi.tszFormat = ai->targv[2];
	mir_free(formatString(&fi));
	while (fi.eCount == 0) {
		fi.tszFormat = ai->targv[4];
		TCHAR *parsed = formatString(&fi);
		if (parsed != NULL) {
			if (res == NULL) {
				res = (TCHAR*)mir_alloc(mir_tstrlen(parsed) + 1 * sizeof(TCHAR));
				if (res == NULL) {
					mir_free(parsed);
					return NULL;
				}
			}
			else res = (TCHAR*)mir_realloc(res, (mir_tstrlen(res) + mir_tstrlen(parsed) + 1)*sizeof(TCHAR));

			mir_tstrcat(res, parsed);
			mir_free(parsed);
		}
		fi.tszFormat = ai->targv[3];
		mir_free(formatString(&fi));
		fi.eCount = 0;
		fi.tszFormat = ai->targv[2];
		mir_free(formatString(&fi));
	}

	return res;
}

static TCHAR *parseEqual(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if (ttoi(ai->targv[1]) != ttoi(ai->targv[2]))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseGreater(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if (ttoi(ai->targv[1]) <= ttoi(ai->targv[2]))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseLonger(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if (mir_tstrlen(ai->targv[1]) <= mir_tstrlen(ai->targv[2]))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseNot(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2) {
		return NULL;
	}

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.tszFormat = ai->targv[1];
	mir_free(formatString(&fi));

	if (fi.eCount == 0)
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseOr(ARGUMENTSINFO *ai)
{
	if (ai->argc < 2)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	ai->flags |= AIF_FALSE;
	for (unsigned i = 1; (i < ai->argc) && (ai->flags&AIF_FALSE); i++) {
		fi.tszFormat = ai->targv[i];
		fi.eCount = 0;
		mir_free(formatString(&fi));

		if (fi.eCount == 0)
			ai->flags &= ~AIF_FALSE;
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseTrue(ARGUMENTSINFO *ai)
{
	return (ai->argc != 1) ? NULL : mir_tstrdup(_T(""));
}

static TCHAR *parseXor(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	FORMATINFO fi;
	memcpy(&fi, ai->fi, sizeof(fi));
	ai->flags = AIF_FALSE;
	fi.tszFormat = ai->targv[0];
	mir_free(formatString(&fi));
	int val1 = fi.eCount == 0;

	fi.tszFormat = ai->targv[1];
	mir_free(formatString(&fi));
	int val2 = fi.eCount == 0;

	ai->flags |= ((val1 & AIF_FALSE) == !(val2 & AIF_FALSE)) ? 0 : AIF_FALSE;
	return mir_tstrdup(_T(""));
}

void registerLogicTokens()
{
	registerIntToken(AND, parseAnd, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y, ...)\t"LPGEN("performs logical AND (x && y && ...)"));
	registerIntToken(STR_FALSE, parseFalse, TRF_FIELD, LPGEN("Logical Expressions")"\t"LPGEN("Boolean FALSE"));
	registerIntToken(FOR, parseFor, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("performs w, then shows z and performs y while x is TRUE"));
	registerIntToken(IF, parseIf, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y,z)\t"LPGEN("shows y if x is TRUE, otherwise it shows z"));
	registerIntToken(IF2, parseIf2, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("shows x if x is TRUE, otherwise it shows y (if(x,x,y))"));
	registerIntToken(IF3, parseIf3, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y, ...)\t"LPGEN("the first argument parsed successfully"));
	registerIntToken(IFEQUAL, parseIfequal, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("y if w = x, else z"));
	registerIntToken(IFGREATER, parseIfgreater, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("y if w > x, else z"));
	registerIntToken(IFLONGER, parseIflonger, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("y if string length of w > x, else z"));
	registerIntToken(EQUAL, parseEqual, TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("TRUE if x = y"));
	registerIntToken(GREATER, parseGreater, TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("TRUE if x > y"));
	registerIntToken(LONGER, parseLonger, TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("TRUE if x is longer than y"));
	registerIntToken(NOT, parseNot, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x)\t"LPGEN("performs logical NOT (!x)"));
	registerIntToken(OR, parseOr, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y,...)\t"LPGEN("performs logical OR (x || y || ...)"));
	registerIntToken(STR_TRUE, parseTrue, TRF_FIELD, LPGEN("Logical Expressions")"\t"LPGEN("Boolean TRUE"));
	registerIntToken(XOR, parseXor, TRF_UNPARSEDARGS | TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("performs logical XOR (x ^ y)"));
}
