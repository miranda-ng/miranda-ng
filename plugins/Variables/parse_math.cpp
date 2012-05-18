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
#include "parse_math.h"

static TCHAR *parseAdd(ARGUMENTSINFO *ai) {

	unsigned int i;
	int result;

	if (ai->argc < 3) {
		return NULL;
	}
	result = 0;
	for (i=1;i<ai->argc;i++) {
		result += ttoi(ai->targv[i]);
	}
	return itot(result);
}

static TCHAR *parseDiv(ARGUMENTSINFO *ai) {

	int val1, val2;

	if (ai->argc != 3) {
		return NULL;
	}

	val1 = ttoi(ai->targv[1]);
	val2 = ttoi(ai->targv[2]);
	if (val2 == 0) {
		return NULL;
	}
	return itot(val1/val2);
}

static TCHAR *parseHex(ARGUMENTSINFO *ai) {

	int val;
	unsigned int i, zeros;
	int padding;
	TCHAR *res, szVal[34];

	if (ai->argc != 3)
		return NULL;

	val = ttoi(ai->targv[1]);
	padding = ttoi(ai->targv[2]);
	mir_sntprintf(szVal, SIZEOF(szVal), _T("%x"), val);
	zeros = max(padding - (signed int)_tcslen(szVal), 0);
	res = ( TCHAR* )malloc((zeros + _tcslen(szVal) + 3)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (zeros + _tcslen(szVal) + 3)*sizeof(TCHAR));
	_tcscpy(res, _T("0x"));
	for (i=0;i<zeros;i++)
		*(res+2+i) = _T('0');

	_tcscat(res, szVal);
	return res;
}

static TCHAR *parseMod(ARGUMENTSINFO *ai) {

	int val1, val2;

	if (ai->argc != 3) {
		return NULL;
	}
	val1 = ttoi(ai->targv[1]);
	val2 = ttoi(ai->targv[2]);
	if (val2 == 0) {
		return NULL;
	}

	return itot(val1%val2);
}

static TCHAR *parseMul(ARGUMENTSINFO *ai) {

	unsigned int i;
	int result;

	if (ai->argc < 3) {
		return NULL;
	}
	result = ttoi(ai->targv[1]);
	for (i=2;i<ai->argc;i++) {
		result *= ttoi(ai->targv[i]);
	}

	return itot(result);
}

static TCHAR *parseMuldiv(ARGUMENTSINFO *ai) {

	if (ai->argc != 4) {
		return NULL;
	}
	if (ttoi(ai->targv[3]) == 0) {
		return NULL;
	}

	return itot((ttoi(ai->targv[1])*ttoi(ai->targv[2]))/ttoi(ai->targv[3]));
}

static TCHAR *parseMin(ARGUMENTSINFO *ai) {

	unsigned int i;
	int minVal;

	if (ai->argc < 2) {
		return NULL;
	}
	minVal = ttoi(ai->targv[1]);
	for (i=2;i<ai->argc;i++) {
		minVal = min(ttoi(ai->targv[i]), minVal);
	}

	return itot(minVal);
}

static TCHAR *parseMax(ARGUMENTSINFO *ai) {

	unsigned int i;
	int maxVal;

	if (ai->argc < 2) {
		return NULL;
	}
	maxVal = ttoi(ai->targv[1]);
	for (i=2;i<ai->argc;i++) {
		maxVal = max(ttoi(ai->targv[i]), maxVal);
	}

	return itot(maxVal);
}

static TCHAR *parseNum(ARGUMENTSINFO *ai) {

	int val;
	unsigned int zeros, i;
	int padding;
	TCHAR *res, *szVal, *cur;

	if (ai->argc != 3)
		return NULL;

	val = ttoi(ai->targv[1]);
	padding = ttoi(ai->targv[2]);
	szVal = itot(val);
	if (szVal == NULL)
		return NULL;

	zeros = max(padding - (signed int)_tcslen(szVal), 0);
	res = ( TCHAR* )malloc((zeros + _tcslen(szVal) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	ZeroMemory(res, (zeros + _tcslen(szVal) + 1)*sizeof(TCHAR));
	cur = res;
	for (i=0;i<zeros;i++)
		*cur++ = _T('0');

	_tcscat(res, szVal);
	free(szVal);

	return res;
}

static TCHAR *parseRand(ARGUMENTSINFO *ai) {

	return itot(rand());
}

static TCHAR *parseSub(ARGUMENTSINFO *ai) {

	unsigned int i;
	int result;

	if (ai->argc < 3) {
		return NULL;
	}
	result = ttoi(ai->targv[1]);
	for (i=2;i<ai->argc;i++) {
		result -= ttoi(ai->targv[i]);
	}

	return itot(result);
}

int registerMathTokens() {

	registerIntToken(_T(ADD), parseAdd, TRF_FUNCTION, "Mathematical Functions\t(x,y ,...)\tx + y + ...");
	registerIntToken(_T(DIV), parseDiv,	TRF_FUNCTION, "Mathematical Functions\t(x,y)\tx divided by y");
	registerIntToken(_T(HEX), parseHex,	TRF_FUNCTION, "Mathematical Functions\t(x,y)\tconverts decimal value x to hex value and padds to length y");
	registerIntToken(_T(MOD), parseMod, TRF_FUNCTION, "Mathematical Functions\t(x,y)\tx modulo y (remainder of x divided by y)");
	registerIntToken(_T(MUL), parseMul,	TRF_FUNCTION, "Mathematical Functions\t(x,y)\tx times y");
	registerIntToken(_T(MULDIV), parseMuldiv, TRF_FUNCTION, "Mathematical Functions\t(x,y,z)\tx times y divided by z");
	registerIntToken(_T(MIN), parseMin,	TRF_FUNCTION, "Mathematical Functions\t(x,y,...)\tminimum value of (decimal) arguments");
	registerIntToken(_T(MAX), parseMax,	TRF_FUNCTION, "Mathematical Functions\t(x,y,...)\tmaximum value of (decimal) arguments");
	registerIntToken(_T(NUM), parseNum,	TRF_FUNCTION, "Mathematical Functions\t(x,y)\tpads decimal value x to length y with zeros");
	registerIntToken(_T(RAND), parseRand, TRF_FUNCTION, "Mathematical Functions\t()\trandom number");
	registerIntToken(_T(SUB), parseSub, TRF_FUNCTION, "Mathematical Functions\t(x,y,...)\tx - y - ...");
	srand((unsigned int)GetTickCount());

	return 0;
}
