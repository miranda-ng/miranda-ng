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

static TCHAR *parseAnd(ARGUMENTSINFO *ai) {

	FORMATINFO fi;
	TCHAR *szCondition;
	unsigned int i;

	if (ai->argc < 3) {
		return NULL;
	}
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	for (i=1;i<ai->argc;i++) {
		fi.tszFormat = ai->targv[i];
		szCondition = formatString(&fi);
		mir_free(szCondition);
		//if (fi.pCount <= 0) {
		if (fi.eCount > 0) {
			ai->flags |= AIF_FALSE;
			return mir_tstrdup(_T(""));
		}
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseFalse(ARGUMENTSINFO *ai) {

	if (ai->argc != 1) {
		return NULL;
	}
	ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseIf(ARGUMENTSINFO *ai) {
	if (ai->argc != 4) {
		return NULL;
	}
	FORMATINFO fi;
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.eCount = fi.pCount = 0;
	fi.tszFormat = ai->targv[1];
	TCHAR *szCondition = formatString(&fi);
	mir_free(szCondition);
	//if (fi.pCount > 0) {
	if (fi.eCount == 0) {
		return mir_tstrdup(ai->targv[2]);
	}
	else {
		return mir_tstrdup(ai->targv[3]);
	}
}

static TCHAR *parseIf2(ARGUMENTSINFO *ai) {
	if (ai->argc != 3) {
		return NULL;
	}

	FORMATINFO fi;
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.eCount = fi.pCount = 0;
	fi.tszFormat = ai->targv[1];
	TCHAR *szCondition = formatString(&fi);
	//if (fi.pCount > 0) {
	if (fi.eCount == 0) {
		return szCondition;
	}
	else {
		if (szCondition != NULL) {
//			ai->flags |= AIF_DONTPARSE;
			mir_free(szCondition);
		}
		return mir_tstrdup(ai->targv[2]);
	}
}

static TCHAR *parseIf3(ARGUMENTSINFO *ai) {

	FORMATINFO fi;
	TCHAR *szCondition;
	unsigned int i;

	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	for (i=1;i<ai->argc;i++) {
		fi.eCount = fi.pCount = 0;
		fi.tszFormat = ai->targv[i];
		szCondition = formatString(&fi);
		//if (fi.pCount > 0) {
		if (fi.eCount == 0) {
//			ai->flags |= AIF_DONTPARSE;
			return szCondition;
		}
		if (szCondition != NULL) {
			mir_free(szCondition);
		}
	}
	
	return NULL;
}

static TCHAR *parseIfequal(ARGUMENTSINFO *ai)
{
	if (ai->argc != 5)
		return NULL;

	FORMATINFO fi = { 0 };
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.szFormat = ai->argv[1];
	TCHAR *tszFirst = formatString(&fi);
	fi.szFormat = ai->argv[2];
	TCHAR *tszSecond = formatString(&fi);
	if ((tszFirst == NULL) || (tszSecond == NULL)) {
		if (tszFirst != NULL)
			mir_free(tszFirst);

		if (tszSecond != NULL)
			mir_free(tszSecond);

		return NULL;
	}
	if ((ttoi(tszFirst)) == (ttoi(tszSecond))) {
		mir_free(tszFirst);
		mir_free(tszSecond);
		return mir_tstrdup(ai->targv[3]);
	}
	mir_free(tszFirst);
	mir_free(tszSecond);

	return mir_tstrdup(ai->targv[4]);
}

static TCHAR *parseIfgreater(ARGUMENTSINFO *ai) {
	if (ai->argc != 5) {
		return NULL;
	}

	FORMATINFO fi;
	//ai->flags |= AIF_DONTPARSE;
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.szFormat = ai->argv[1];
	TCHAR *tszFirst = formatString(&fi);
	fi.szFormat = ai->argv[2];
	TCHAR *tszSecond = formatString(&fi);
	if ((tszFirst == NULL) || (tszSecond == NULL)) {
		if (tszFirst != NULL) {
			mir_free(tszFirst);
		}
		if (tszSecond != NULL) {
			mir_free(tszSecond);
		}
		return NULL;
	}
	if ((ttoi(tszFirst)) > (ttoi(tszSecond))) {
		mir_free(tszFirst);
		mir_free(tszSecond);
		return mir_tstrdup(ai->targv[3]);
	}
	mir_free(tszFirst);
	mir_free(tszSecond);

	return mir_tstrdup(ai->targv[4]);
}

static TCHAR *parseIflonger(ARGUMENTSINFO *ai) {
	if (ai->argc != 5) {
		return NULL;
	}

	FORMATINFO fi;
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.szFormat = ai->argv[1];
	TCHAR *tszFirst = formatString(&fi);
	fi.szFormat = ai->argv[2];
	TCHAR *tszSecond = formatString(&fi);
	if ((tszFirst == NULL) || (tszSecond == NULL)) {
		if (tszFirst != NULL) {
			mir_free(tszFirst);
		}
		if (tszSecond != NULL) {
			mir_free(tszSecond);
		}
		return NULL;
	}
	if ( _tcslen(tszFirst) > _tcslen(tszSecond)) {
		mir_free(tszFirst);
		mir_free(tszSecond);
		return mir_tstrdup(ai->targv[3]);
	}
	mir_free(tszFirst);
	mir_free(tszSecond);

	return mir_tstrdup(ai->targv[4]);
}

/*

  ?for(init, cond, incr, show)

  */
static TCHAR *parseFor(ARGUMENTSINFO *ai) {
	if (ai->argc != 5) {
		return NULL;
	}

	TCHAR *parsed;
	FORMATINFO fi;

	TCHAR *res = mir_tstrdup(_T(""));
//	ai->flags |= AIF_DONTPARSE;
	ZeroMemory(&fi, sizeof(fi));
	CopyMemory(&fi, ai->fi, sizeof(fi));
	fi.eCount = fi.pCount = 0;
	fi.tszFormat = ai->targv[1];
	mir_free(formatString(&fi));
	fi.tszFormat = ai->targv[2];
	mir_free(formatString(&fi));
	while (fi.eCount == 0) {
		fi.tszFormat = ai->targv[4];
		parsed = formatString(&fi);
		if (parsed != NULL) {
			if (res == NULL) {
				res = (TCHAR*)mir_alloc( _tcslen(parsed)+1 * sizeof(TCHAR));
				if (res == NULL)
					return NULL;
			}
			else res = (TCHAR*)mir_realloc(res, (_tcslen(res)+_tcslen(parsed)+1)*sizeof(TCHAR));

			_tcscat(res, parsed);
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

	if (ttoi(ai->targv[1]) != ttoi( ai->targv[2] ))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseGreater(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if (ttoi(ai->targv[1]) <= ttoi(ai->targv[2] ))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseLonger(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	if ( _tcslen(ai->targv[1]) <= _tcslen(ai->targv[2]))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

static TCHAR *parseNot(ARGUMENTSINFO *ai) {
	if (ai->argc != 2) {
		return NULL;
	}

	FORMATINFO fi;
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	fi.tszFormat = ai->targv[1];
	TCHAR *szCondition = formatString(&fi);
	mir_free(szCondition);
	//if (fi.pCount > 0) {
	if (fi.eCount == 0) {
		ai->flags |= AIF_FALSE;
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseOr(ARGUMENTSINFO *ai) {

	unsigned int i;
	FORMATINFO fi;
	TCHAR *szCondition;

	if (ai->argc < 2) {
		return NULL;
	}
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	ai->flags |= AIF_FALSE;
	for(i=1;(i<ai->argc)&&(ai->flags&AIF_FALSE);i++) {
		fi.tszFormat = ai->targv[i];
		fi.eCount = 0;
		szCondition = formatString(&fi);
		mir_free(szCondition);
		//if (fi.pCount > 0) {
		if (fi.eCount == 0) {
			ai->flags &= ~AIF_FALSE;
		}
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseTrue(ARGUMENTSINFO *ai) {

	if (ai->argc != 1) {
		return NULL;
	}

	return mir_tstrdup(_T(""));
}

static TCHAR *parseXor(ARGUMENTSINFO *ai) {

	int val1, val2;
	FORMATINFO fi;
	TCHAR *szCondition;

	if (ai->argc != 3) {
		return NULL;
	}
	ZeroMemory(&fi, sizeof(fi));
	memcpy(&fi, ai->fi, sizeof(fi));
	ai->flags = AIF_FALSE;
	fi.tszFormat = ai->targv[0];
	szCondition = formatString(&fi);
	mir_free(szCondition);
	//val1 = fi.pCount > 0;
	val1 = fi.eCount == 0;
	fi.tszFormat = ai->targv[1];
	szCondition = formatString(&fi);
	mir_free(szCondition);
	//val2 = fi.pCount > 0;
	val2 = fi.eCount == 0;
	ai->flags |= ((val1&AIF_FALSE)==!(val2&AIF_FALSE))?0:AIF_FALSE;

	return mir_tstrdup(_T(""));
}

int registerLogicTokens() {

	registerIntToken(_T(AND), parseAnd, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y, ...)\t"LPGEN("performs logical AND (x && y && ...)"));
	registerIntToken(_T(STR_FALSE), parseFalse, TRF_FIELD, LPGEN("Logical Expressions")"\t"LPGEN("Boolean FALSE"));
	registerIntToken(_T(FOR), parseFor, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("performs w, then shows z and performs y while x is TRUE"));
	registerIntToken(_T(IF), parseIf, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y,z)\t"LPGEN("shows y if x is TRUE, otherwise it shows z"));
	registerIntToken(_T(IF2), parseIf2, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("shows x if x is TRUE, otherwise it shows y (if(x,x,y))"));
	registerIntToken(_T(IF3), parseIf3, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y, ...)\t"LPGEN("the first argument parsed successfully"));
	registerIntToken(_T(IFEQUAL), parseIfequal,	TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("y if w = x, else z"));
	registerIntToken(_T(IFGREATER), parseIfgreater,	TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("y if w > x, else z"));
	registerIntToken(_T(IFLONGER), parseIflonger, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(w,x,y,z)\t"LPGEN("y if string length of w > x, else z"));
	registerIntToken(_T(EQUAL), parseEqual,	TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("TRUE if x = y"));
	registerIntToken(_T(GREATER), parseGreater,	TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("TRUE if x > y"));
	registerIntToken(_T(LONGER), parseLonger, TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("TRUE if x is longer than y"));
	registerIntToken(_T(NOT), parseNot, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x)\t"LPGEN("performs logical NOT (!x)"));
	registerIntToken(_T(OR), parseOr, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y,...)\t"LPGEN("performs logical OR (x || y || ...)"));
	registerIntToken(_T(STR_TRUE), parseTrue, TRF_FIELD, LPGEN("Logical Expressions")"\t"LPGEN("Boolean TRUE"));
	registerIntToken(_T(XOR), parseXor, TRF_UNPARSEDARGS|TRF_FUNCTION, LPGEN("Logical Expressions")"\t(x,y)\t"LPGEN("performs logical XOR (x ^ y)"));
	
	return 0;
}
