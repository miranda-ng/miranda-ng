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
#include <pcre.h>

static void (*pcreFree)(void *);
static pcre_extra *(*pcreStudy)(const pcre *, int, const char **);
static pcre *(*pcreCompile)(const char *, int, const char **, int *, const unsigned char *);
static int  (*pcreExec)(const pcre *, const pcre_extra *, const char *, int, int, int, int *, int);
static void *(*pcreMalloc)(size_t);
static void (*pcreFreeSubstring)(const char *);
static int  (*pcreGetSubstring)(const char *, int *, int, int, const char **);

/*
	pattern, subject
*/
static TCHAR *parseRegExpCheck(ARGUMENTSINFO *ai) {

	const char *err;
	int erroffset, nmat;
	pcre_extra *extra;
	pcre *ppat;
	char szVal[34], *arg1, *arg2;
	int offsets[99];
	TCHAR *res;

	if (ai->argc != 3) {
		return NULL;
	}
	ai->flags = AIF_FALSE;

	arg1 = mir_t2a(ai->targv[1]);
	arg2 = mir_t2a(ai->targv[2]);

	ppat = pcreCompile(arg1, 0, &err, &erroffset, NULL);
	if (ppat == NULL) {
		mir_free(arg1);
		mir_free(arg2);
		return NULL;
	}
	extra = pcreStudy(ppat, 0, &err);
	nmat = pcreExec(ppat, extra, arg2, strlen(arg2), 0, 0, offsets, 99);
	mir_free(arg1);
	mir_free(arg2);
	if (nmat > 0) {
		ai->flags &= ~AIF_FALSE;
		_ltoa(nmat, szVal, 10);

		res = mir_a2t(szVal);

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
	int erroffset, nmat, number;
	pcre_extra *extra;
	pcre *ppat;
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
	ppat = pcreCompile(arg1, 0, &err, &erroffset, NULL);
	if (ppat == NULL) {
		mir_free(arg1);
		mir_free(arg2);
		mir_free(arg3);
		return NULL;
	}
	extra = pcreStudy(ppat, 0, &err);
	nmat = pcreExec(ppat, extra, arg2, strlen(arg2), 0, 0, offsets, 99);
	if (nmat >= 0) {
		ai->flags &= ~AIF_FALSE;
	}
	if (pcreGetSubstring(arg2, offsets, nmat, number, &substring) < 0) {
		ai->flags |= AIF_FALSE;
	}
	else {
		res = mir_strdup(substring);
		pcreFreeSubstring(substring);


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

int initPcre() {

	HMODULE hModule;
	
	hModule = LoadLibraryA("pcre.dll");
	if (hModule == NULL) {
		char path[MAX_PATH];
		char *cur;

		GetModuleFileNameA(NULL, path, sizeof(path));
		cur = strrchr(path, '\\');
		if (cur != NULL)
			strcpy(cur+1, "pcre.dll");
		else
			strcpy(path, "pcre.dll");
		hModule = LoadLibraryA(path);
		if (hModule == NULL) {
			if (cur != NULL)
				strcpy(cur+1, "pcre3.dll");
			else
				strcpy(path, "pcre3.dll");
			hModule = LoadLibraryA(path);
		}
	}
	if (hModule == NULL) {
		return -1;
	}
	pcreMalloc = (void *(__cdecl *)(size_t))GetProcAddress(hModule, "pcre_malloc");
	pcreFree = (void (__cdecl *)(void *))GetProcAddress(hModule, "pcre_free");
	pcreStudy = (struct pcre_extra *(__cdecl *)(const struct real_pcre *,int ,const char ** ))GetProcAddress(hModule, "pcre_study");
	pcreCompile = (struct real_pcre *(__cdecl *)(const char *,int ,const char ** ,int *,const unsigned char *))GetProcAddress(hModule, "pcre_compile");
	pcreExec = (int (__cdecl *)(const struct real_pcre *,const struct pcre_extra *,const char *,int ,int ,int ,int *,int ))GetProcAddress(hModule, "pcre_exec");
	pcreFreeSubstring = (void (__cdecl *)(const char *))GetProcAddress(hModule, "pcre_free_substring");
	pcreGetSubstring = (int (__cdecl *)(const char *,int *,int ,int ,const char ** ))GetProcAddress(hModule, "pcre_get_substring");

	return 0;
}

int registerRegExpTokens() {

	if (initPcre() != 0) {
		log_infoA("Variables: pcre.dll for PCRE not found");
		return -1;
	}


	registerIntToken(_T(REGEXPCHECK), parseRegExpCheck, TRF_FUNCTION, "Regular Expressions\t(x,y)\t(ANSI input only) the number of substring matches found in y with pattern x");
	registerIntToken(_T(REGEXPSUBSTR), parseRegExpSubstr, TRF_FUNCTION, "Regular Expressions\t(x,y,z)\t(ANSI input only) substring match number z found in subject y with pattern x");


	return 0;
}
