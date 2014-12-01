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

static TCHAR *parseAdd(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return NULL;

	int result = 0;
	for (unsigned int i = 1; i < ai->argc; i++)
		result += ttoi(ai->targv[i]);

	return itot(result);
}

static TCHAR *parseDiv(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	int val1 = ttoi(ai->targv[1]);
	int val2 = ttoi(ai->targv[2]);
	if (val2 == 0)
		return NULL;

	return itot(val1 / val2);
}

static TCHAR *parseHex(ARGUMENTSINFO *ai)
{
	unsigned int i;
	TCHAR szVal[34];

	if (ai->argc != 3)
		return NULL;

	int val = ttoi(ai->targv[1]);
	int padding = ttoi(ai->targv[2]);
	mir_sntprintf(szVal, SIZEOF(szVal), _T("%x"), val);
	unsigned int zeros = max(padding - (signed int)_tcslen(szVal), 0);
	TCHAR *res = (TCHAR*)mir_alloc((zeros + _tcslen(szVal) + 3)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((zeros + _tcslen(szVal) + 3) * sizeof(TCHAR)));
	_tcscpy(res, _T("0x"));
	for (i = 0; i < zeros; i++)
		*(res + 2 + i) = '0';

	_tcscat(res, szVal);
	return res;
}

static TCHAR *parseMod(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	int val1 = ttoi(ai->targv[1]);
	int val2 = ttoi(ai->targv[2]);
	if (val2 == 0)
		return NULL;

	return itot(val1 % val2);
}

static TCHAR *parseMul(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return NULL;

	int result = ttoi(ai->targv[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		result *= ttoi(ai->targv[i]);

	return itot(result);
}

static TCHAR *parseMuldiv(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return NULL;

	if (ttoi(ai->targv[3]) == 0)
		return NULL;

	return itot((ttoi(ai->targv[1])*ttoi(ai->targv[2])) / ttoi(ai->targv[3]));
}

static TCHAR *parseMin(ARGUMENTSINFO *ai)
{
	if (ai->argc < 2)
		return NULL;

	int minVal = ttoi(ai->targv[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		minVal = min(ttoi(ai->targv[i]), minVal);

	return itot(minVal);
}

static TCHAR *parseMax(ARGUMENTSINFO *ai)
{
	if (ai->argc < 2)
		return NULL;

	int maxVal = ttoi(ai->targv[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		maxVal = max(ttoi(ai->targv[i]), maxVal);

	return itot(maxVal);
}

static TCHAR *parseNum(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	int val = ttoi(ai->targv[1]);
	int padding = ttoi(ai->targv[2]);
	TCHAR *szVal = itot(val);
	if (szVal == NULL)
		return NULL;

	unsigned zeros = max(padding - (signed int)_tcslen(szVal), 0);
	TCHAR *res = (TCHAR*)mir_alloc((zeros + _tcslen(szVal) + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	memset(res, 0, ((zeros + _tcslen(szVal) + 1) * sizeof(TCHAR)));
	TCHAR *cur = res;
	for (unsigned i = 0; i < zeros; i++)
		*cur++ = '0';

	_tcscat(res, szVal);
	mir_free(szVal);

	return res;
}

static TCHAR *parseRand(ARGUMENTSINFO *ai)
{
	return itot(rand());
}

static TCHAR *parseSub(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return NULL;

	int result = ttoi(ai->targv[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		result -= ttoi(ai->targv[i]);

	return itot(result);
}

void registerMathTokens()
{
	registerIntToken(ADD, parseAdd, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y ,...)\t"LPGEN("x + y + ..."));
	registerIntToken(DIV, parseDiv, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y)\t"LPGEN("x divided by y"));
	registerIntToken(HEX, parseHex, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y)\t"LPGEN("converts decimal value x to hex value and padds to length y"));
	registerIntToken(MOD, parseMod, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y)\t"LPGEN("x modulo y (remainder of x divided by y)"));
	registerIntToken(MUL, parseMul, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y)\t"LPGEN("x times y"));
	registerIntToken(MULDIV, parseMuldiv, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y,z)\t"LPGEN("x times y divided by z"));
	registerIntToken(MIN, parseMin, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y,...)\t"LPGEN("minimum value of (decimal) arguments"));
	registerIntToken(MAX, parseMax, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y,...)\t"LPGEN("maximum value of (decimal) arguments"));
	registerIntToken(NUM, parseNum, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y)\t"LPGEN("pads decimal value x to length y with zeros"));
	registerIntToken(RAND, parseRand, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t()\t"LPGEN("random number"));
	registerIntToken(SUB, parseSub, TRF_FUNCTION, LPGEN("Mathematical Functions")"\t(x,y,...)\t"LPGEN("x - y - ..."));
	srand((unsigned int)GetTickCount());
}
