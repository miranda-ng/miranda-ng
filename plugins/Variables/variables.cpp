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

BOOL (WINAPI *pfnEnableThemeDialogTexture)(HANDLE, DWORD) = 0;
static BOOL bWarningShown = FALSE; // unicode on ansi warning

/* some handles */
static HANDLE
	hFormatStringService,
	hFreeMemoryService,
	hRegisterVariableService,
	hGetMMIService,
	hShowHelpService,
	hShowHelpExService,
	hGetIconService;

static HANDLE
	hOptionsHook = NULL,
	hIconsChangedHook = NULL;

HCURSOR hCurSplitNS;

struct ParseOptions gParseOpts;

extern HINSTANCE hInst;

TCHAR *getArguments(TCHAR *string, TCHAR ***aargv, int *aargc) {

	BOOL bDontParse, bNewArg, bDone;
	TCHAR *cur, *scur, **argv;
	int i, argc, brackets;

	*aargv = NULL;
	*aargc = 0;
	argc = brackets = 0;
	argv = NULL;
	cur = string;
	while (*cur == _T(' ')) {
		cur++;
	}
	if (*cur != _T('(')) {
		return NULL;
	}
	cur++;
	scur = cur-1;
	bDontParse = bNewArg = bDone = FALSE;
	while ( (!bDone) && (*cur != _T('\0')) ) {
		switch (*cur) {
		case _T(DONTPARSE_CHAR):
			if (bDontParse) {
				bDontParse = FALSE;
			}
			else {
				bDontParse = TRUE;
			}
			break;

		case _T(','):
			if ( (!bDontParse) && (brackets == 0) ) {
				bNewArg = TRUE;
			}
			break;

		case _T('('):
			if (!bDontParse) {
				brackets += 1;
			}
			break;

		case _T(')'):
			if ( (brackets == 0) && (!bDontParse) ) {
				bDone = bNewArg = TRUE;
			}
			else if ( (brackets > 0) && (!bDontParse) ) {
				brackets -= 1;
			}
			break;
		}
		if (bNewArg) {
			argv = ( TCHAR** )realloc(argv, (argc+1)*sizeof(TCHAR *));
			if (argv == NULL) {
				return NULL;
			}
			if (cur > scur) {
				argv[argc] = ( TCHAR* )malloc((cur-scur+2)*sizeof(TCHAR));
				if (argv[argc] == NULL) {
					return NULL;
				}
				memset(argv[argc], '\0', (cur-(scur+1)+1)*sizeof(TCHAR));
				_tcsncpy(argv[argc], scur+1, cur-(scur+1));
			}
			else {
				argv[argc] = _tcsdup(_T(""));
			}
			argc += 1;
			bNewArg = FALSE;
			scur = cur;
		}
		cur++;
	}
	// set args
	if (*(cur-1) == _T(')')) {
		*aargv = argv;
		*aargc = argc;
	}
	else {
		for (i=0;i<argc;i++) {
			if (argv[i] != NULL) {
				free(argv[i]);
			}
		}
		free(argv);
		*aargv = NULL;
		*aargc = 0;
		cur = NULL;
	}

	return cur;
}

int isValidTokenChar(TCHAR tc) {

	return (
		(tc != _T('(')) &&
		(tc != _T(',')) &&
		(tc != _T(')')) &&
		(tc != _T(FIELD_CHAR)) &&
		(tc != _T(FUNC_CHAR)) &&
		(tc != _T(FUNC_ONCE_CHAR)) &&
		(tc != _T('/')) &&
		(tc != _T('\0'))
		);
}

/* pretty much the main loop */
static TCHAR* replaceDynVars(TCHAR* szTemplate, FORMATINFO* fi)
{
	TCHAR
		*string,
		*cur,   // current position (pnt only)
		*tcur,  // temp cur			(pnt only)
		*scur,  // start of variable(pnt only)
		*parsedToken,   // parsed result (dyn alloc)
		**argv, // arguments (dyn alloc)
		**pargv, //  dyn alloc
		*token; // variable name (pnt only)
 	int argc, i, parsedTokenLen, initStrLen, tokenLen, scurPos, curPos, tmpVarPos;
	unsigned int pos;
	FORMATINFO afi;
	TOKENREGISTEREX *tr;
	ARGUMENTSINFO ai = { 0 };

	string = _tcsdup(szTemplate);
	if (string == NULL)
		return NULL;

	argc = parsedTokenLen = initStrLen = tokenLen = 0;
	cur = tcur = scur = token = parsedToken = NULL;
	pargv = argv = NULL;
	//fi->pCount = 0;
	memcpy(&afi, fi, sizeof(afi));
	for (pos = 0;pos < _tcslen(string);pos++) {
		// string may move in memory, iterate by remembering the position in the string
		cur = string+pos;
		// free memory from last iteration, this way we can bail out at any time in the loop
		if (parsedToken != NULL)
			free(parsedToken);

		for (i=0;i<argc;i++)
			if (argv[i] != NULL)
				free(argv[i]);

		if (argv != NULL)
			free(argv);

		argc = parsedTokenLen = initStrLen = tokenLen = 0;
		tcur = scur = token = parsedToken = NULL;
		pargv = argv = NULL;
		// new round
		if (*cur == _T(DONTPARSE_CHAR)) {
			MoveMemory(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
			if (*cur == _T(DONTPARSE_CHAR))
				continue;

			while ( (*cur != _T(DONTPARSE_CHAR)) && (*cur != _T('\0')))
				cur++;

			MoveMemory(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
			pos = cur-string-1;
			continue;
		}
		// remove end of lines
		else if ( (!_tcsncmp(cur, _T("\r\n"), 2)) && (gParseOpts.bStripEOL) ) {
			MoveMemory(cur, cur+2, (_tcslen(cur+2)+1)*sizeof(TCHAR));
			pos = cur-string-1;
			continue;
		}
		else if ( ((*cur == _T('\n')) && (gParseOpts.bStripEOL)) || ((*cur == _T(' ')) && (gParseOpts.bStripWS)) ) {
			MoveMemory(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
			pos = cur - string - 1;
			continue;
		}
		// remove comments
		else if (!_tcsncmp(cur, _T(COMMENT_STRING), _tcslen(_T(COMMENT_STRING)))) {
			scur = cur;
			while ( (_tcsncmp(cur, _T("\r\n"), 2)) && (*cur != _T('\n')) && (*cur != _T('\0')))
				cur++;

			if (*cur == _T('\0')) {
				*scur = _T('\0');
				string = ( TCHAR* )realloc(string, (_tcslen(string)+1)*sizeof(TCHAR));
				continue;
			}
			MoveMemory(scur, cur, (_tcslen(cur)+1)*sizeof(TCHAR));
			pos = scur-string-1;
			continue;
		}
		else if ( (*cur != _T(FIELD_CHAR)) && (*cur != _T(FUNC_CHAR)) && (*cur != _T(FUNC_ONCE_CHAR)) ) {
			if (gParseOpts.bStripAll) {
				MoveMemory(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
				pos = cur - string - 1;
			}
			continue;
		}
		scur = tcur = cur+1;
		while (isValidTokenChar(*tcur))
			tcur++;

		if (tcur == cur) {
			fi->eCount += 1;
			continue;
		}
		token = ( TCHAR* )malloc((tcur-scur+1)*sizeof(TCHAR));
		if (token == NULL) {
			fi->eCount += 1;
			return NULL;
		}
		memset(token, '\0', (tcur-scur+1)*sizeof(TCHAR));
		_tcsncpy(token, cur+1, tcur-scur);
		// cur points to FIELD_CHAR or FUNC_CHAR
 		tmpVarPos = -1;
 		tr = NULL;
 		if (*cur==_T(FIELD_CHAR)) {
 			for(i = 0; i < fi->cbTemporaryVarsSize; i += 2) {
 				if (lstrcmp(fi->tszaTemporaryVars[i], token) == 0) {
 					tmpVarPos = i;
 					break;
 				}
 			}
 		}
 		if (tmpVarPos < 0)
 			tr = searchRegister(token, (*cur==_T(FIELD_CHAR))?TRF_FIELD:TRF_FUNCTION);
 		free(token);
 		if (tmpVarPos < 0 && tr == NULL) {
			fi->eCount += 1;
			// token not found, continue
			continue;
		}
		scur = cur; // store this pointer for later use
		if (*cur == _T(FIELD_CHAR)) {
 			size_t len = _tcslen(tr != NULL ? tr->tszTokenString : fi->tszaTemporaryVars[tmpVarPos]);
			cur++;
 			if (*(cur + len) != _T(FIELD_CHAR)) { // the next char after the token should be %
				fi->eCount += 1;
				continue;
			}
 			cur += len+1;
		}
		else if ( (*cur == _T(FUNC_CHAR)) || (*cur == _T(FUNC_ONCE_CHAR)) ) {
			TCHAR *argcur;

			cur += _tcslen(tr->tszTokenString)+1;
			argcur = getArguments(cur, &argv, &argc);
			if ( (argcur == cur) || (argcur == NULL) ) {
				fi->eCount += 1;
				// error getting arguments
				continue;
			}
			cur = argcur;
			// arguments
			for (i=0;i<argc;i++) {
				if (argv[i] != NULL) {
					if (!(tr->flags&TRF_UNPARSEDARGS)) {
						afi.tszFormat = argv[i];
						afi.eCount = afi.pCount = 0;
						argv[i] = formatString(&afi);
						fi->eCount += afi.eCount;
						fi->pCount += afi.pCount;
						free(afi.szFormat);
					}
				}
				if (argv[i] == NULL)
					argv[i] = _tcsdup(_T(""));
			}
		}
		// cur should now point at the character after FIELD_CHAR or after the last ')'
 		if (tr != NULL) {
 			pargv = ( TCHAR** )malloc((argc+1)*sizeof(TCHAR *));
 			if (pargv == NULL) {
 				fi->eCount += 1;
 				return NULL;
 			}
 			for (i=0;i<argc;i++)
 				pargv[i+1] = argv[i];

 			pargv[0] = tr->tszTokenString;
 			ZeroMemory(&ai, sizeof(ai));
 			ai.cbSize = sizeof(ai);
 			ai.argc = argc+1;
 			ai.targv = pargv;
 			ai.fi = fi;
 			if ( (*scur == _T(FUNC_ONCE_CHAR)) || (*scur == _T(FIELD_CHAR)))
 				ai.flags |= AIF_DONTPARSE;

 			parsedToken = parseFromRegister(&ai);
 			free(pargv);
 		}
 		else parsedToken = fi->tszaTemporaryVars[tmpVarPos + 1];

		if (parsedToken == NULL) {
			fi->eCount += 1;
			continue;
		}

		//replaced a var
		if ( ai.flags & AIF_FALSE )
			fi->eCount++;
		else
			fi->pCount++;

		// 'special' chars need to be taken care of (DONTPARSE, TRYPARSE, \r\n)
		// if the var contains the escape character, this character must be doubled, we don't want it to act as an esacpe char
		/*for (tcur=parsedToken;*tcur != '\0';tcur++) {
			if (*tcur == DONTPARSE_CHAR) {//|| (*(var+pos) == ')') ) {
				parsedToken = realloc(parsedToken, strlen(parsedToken) + 2);
				if (parsedToken == NULL) {
					fi->err = EMEM;
					return NULL;
				}
				CopyMemory(tcur+1, tcur, strlen(tcur)+1);
				tcur++;
			}
		}*/

		parsedTokenLen = _tcslen(parsedToken);
		initStrLen = _tcslen(string);
		tokenLen = cur-scur;
		scurPos = scur-string;
		curPos = cur-string;
		if (tokenLen < parsedTokenLen) {
			// string needs more memory
			string = ( TCHAR* )realloc(string, (initStrLen-tokenLen+parsedTokenLen+1)*sizeof(TCHAR));
			if (string == NULL) {
				fi->eCount += 1;
				return NULL;
			}
		}
		scur = string+scurPos;
		cur = string+curPos;
		MoveMemory(scur + parsedTokenLen, cur, (_tcslen(cur)+1)*sizeof(TCHAR));
		CopyMemory(scur, parsedToken, parsedTokenLen*sizeof(TCHAR));
		{
			int len;

			len = _tcslen(string);
			string = ( TCHAR* )realloc(string, (len+1)*sizeof(TCHAR));
		}
		if (( ai.flags & AIF_DONTPARSE ) || tmpVarPos >= 0)
			pos += parsedTokenLen;

		pos--; // parse the same pos again, it changed

 		if ( tr == NULL )
 			parsedToken = NULL; // To avoid free
	}
	if (parsedToken != NULL)
		free(parsedToken);

	for ( i=0; i < argc; i++ )
		if ( argv[i] != NULL )
			free( argv[i] );

	if ( argv != NULL )
		free(argv);

	return ( TCHAR* )realloc(string, (_tcslen(string)+1)*sizeof(TCHAR));
}

/*
	MS_VARS_FORMATSTRING
*/
static INT_PTR formatStringService(WPARAM wParam, LPARAM lParam) {

 	int res, i;
 	BOOL copied;
	FORMATINFO *fi, tempFi;
	FORMATINFOV1 *fiv1;
	TCHAR *tszFormat, *orgFormat, *tszSource, *orgSource, *tRes;

 	if (((FORMATINFO *)wParam)->cbSize >= sizeof(FORMATINFO)) {
		ZeroMemory(&tempFi, sizeof(FORMATINFO));
		CopyMemory(&tempFi, (FORMATINFO *)wParam, sizeof(FORMATINFO));
		fi = &tempFi;
	}
 	else if (((FORMATINFO *)wParam)->cbSize == FORMATINFOV2_SIZE) {
 		ZeroMemory(&tempFi, sizeof(FORMATINFO));
 		CopyMemory(&tempFi, (FORMATINFO *)wParam, FORMATINFOV2_SIZE);
 		fi = &tempFi;
 	}
	else {
		// old struct, must be ANSI
		fiv1 = (FORMATINFOV1 *)wParam;
		ZeroMemory(&tempFi, sizeof(FORMATINFO));
		tempFi.cbSize = sizeof(FORMATINFO);
		tempFi.hContact = fiv1->hContact;
		tempFi.szFormat = fiv1->szFormat;
		tempFi.szExtraText = fiv1->szSource;
		fi = &tempFi;
	}
	orgFormat = fi->tszFormat;
	orgSource = fi->tszExtraText;
#ifdef UNICODE
	if (!(fi->flags&FIF_TCHAR)) {
 		copied = TRUE;
		log_debugA("a2u (%s)", fi->szExtraText);
		tszFormat = fi->szFormat!=NULL?a2u(fi->szFormat):NULL;
		tszSource = fi->szExtraText!=NULL?a2u(fi->szExtraText):NULL;
 		for(i = 0; i < fi->cbTemporaryVarsSize; i++) {
 			fi->tszaTemporaryVars[i] = fi->szaTemporaryVars[i]!=NULL?a2u(fi->szaTemporaryVars[i]):NULL;
 		}
	}
	else {
 		copied = FALSE;
 		tszFormat = fi->tszFormat;
 		tszSource = fi->tszExtraText;
	}
#else
	if (fi->flags&FIF_UNICODE) {
		if (!bWarningShown) {
			MessageBoxA(NULL, "A plugin using UNICODE encoding tries to make use of the ANSI version of Variables.\r\nPlease use the UNICODE version of Variables or the ANSI version of the plugin using Variables.", "Variables", MB_OK);
			bWarningShown = TRUE;
		}
		return (int)NULL;
	}
 	copied = FALSE;
 	tszFormat = fi->szFormat;
 	tszSource = fi->szExtraText;
#endif
	fi->tszFormat = tszFormat;
	fi->tszExtraText = tszSource;

	tRes = formatString(fi);
#ifdef UNICODE
	if (!(fi->flags&FIF_TCHAR)) {
		res = (int)u2a(tRes);
		free(tRes);
	}
	else {
		res = (int)tRes;
	}
#else
	res = (int)tRes;
#endif
 	if (copied) {
 		if (tszFormat != NULL) {
 			free(tszFormat);
 		}
 		if (tszSource != NULL) {
 			free(tszSource);
 		}
 		for(i = 0; i < fi->cbTemporaryVarsSize; i++) {
 			if (fi->tszaTemporaryVars != NULL) {
 				free(fi->tszaTemporaryVars);
 			}
 		}
 	}
	//fi->tszFormat = orgFormat;
	//fi->tszExtraText = orgSource;

	if (((FORMATINFO *)wParam)->cbSize == sizeof(FORMATINFOV1)) {
		((FORMATINFOV1 *)wParam)->eCount = fi->eCount;
		((FORMATINFOV1 *)wParam)->pCount = fi->pCount;
	}
	else {
		((FORMATINFO *)wParam)->eCount = fi->eCount;
		((FORMATINFO *)wParam)->pCount = fi->pCount;
	}
//	clearVariableRegister();?

	return res;
}

TCHAR *formatString(FORMATINFO *fi) {

	/* the service to format a given string */
	TCHAR *string, *formattedString;

	if (fi->eCount + fi->pCount > 5000) {
		fi->eCount += 1;
		fi->pCount += 1;
		log_debugA("Variables: Overflow protection; %d parses", fi->eCount + fi->pCount);
		return NULL;
	}
	if ((fi == NULL) || (fi->tszFormat == NULL)) {
		return NULL;
	}
	string = _tcsdup(fi->tszFormat);
	if (string == NULL) {
		return NULL;
	}
	formattedString = replaceDynVars(string, fi);
	free(string);
	if (formattedString == NULL) {
		return NULL;
	}

	return formattedString;
}

/*
	MS_VARS_FREEMEMORY
*/
static INT_PTR freeMemory(WPARAM wParam, LPARAM lParam) {

	if ((void*)wParam == NULL) {
		return -1;
	}
	free((void*)wParam);

	return 0;
}

/*
	MS_VARS_GET_MMI
	this code is copied from Miranda's core (miranda.c)
*/
INT_PTR getMemoryManagerInterface(WPARAM wParam, LPARAM lParam) {

	struct MM_INTERFACE *mmi = (struct MM_INTERFACE*) lParam;
	if (mmi || mmi->cbSize == sizeof(struct MM_INTERFACE))
	{
		mmi->mmi_malloc = malloc;
		mmi->mmi_realloc = realloc;
		mmi->mmi_free = free;
		return 0;
	}
	return 1;
}

int setParseOptions(struct ParseOptions *po) {

	if (po == NULL) {
		po = &gParseOpts;
	}
	ZeroMemory(po, sizeof(struct ParseOptions));
	if (!db_getb(SETTING_STRIPALL, 0)) {
		po->bStripEOL = db_getb(SETTING_STRIPCRLF, 0);
		po->bStripWS = db_getb(SETTING_STRIPWS, 0);
	}
	else {
		po->bStripAll = TRUE;
	}

	return 0;
}

int LoadVarModule() {

	HMODULE hUxTheme;

	if ( (initTokenRegister() != 0) || (initContactModule() != 0) ) {

		return -1;
	}
	setParseOptions(NULL);
	hFormatStringService = CreateServiceFunction(MS_VARS_FORMATSTRING, formatStringService);
	hFreeMemoryService = CreateServiceFunction(MS_VARS_FREEMEMORY, freeMemory);
	hRegisterVariableService = CreateServiceFunction(MS_VARS_REGISTERTOKEN, registerToken);
	hGetMMIService = CreateServiceFunction(MS_VARS_GET_MMI, getMemoryManagerInterface);
	// help dialog
	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hUxTheme = NULL;
	if(IsWinVerXPPlus()) {
		hUxTheme = GetModuleHandle(_T("uxtheme.dll"));
		if (hUxTheme) {
			pfnEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
		}
	}
	hShowHelpService = CreateServiceFunction(MS_VARS_SHOWHELP, showHelpService);
	hShowHelpExService = CreateServiceFunction(MS_VARS_SHOWHELPEX, showHelpExService);
	if (ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid;
		char szFile[MAX_PATH];

		ZeroMemory(&sid, sizeof(SKINICONDESC));
		sid.cbSize = sizeof(SKINICONDESC);
		sid.ptszSection = TranslateT("Variables");
		sid.ptszDescription = TranslateT("Help");
		sid.pszName = "vars_help";
		GetModuleFileNameA(hInst, szFile, MAX_PATH);
		sid.pszDefaultFile = szFile;
		sid.iDefaultIndex = -IDI_V;
		sid.cx = sid.cy = 16;
		sid.flags = SIDF_TCHAR;
		CallService(MS_SKIN2_ADDICON, (WPARAM)0, (LPARAM)&sid);
		hIconsChangedHook = HookEvent(ME_SKIN2_ICONSCHANGED, iconsChanged);
	}
	hGetIconService = CreateServiceFunction(MS_VARS_GETSKINITEM, getSkinItemService);
	hOptionsHook = HookEvent(ME_OPT_INITIALISE, OptionsInit);

	// register internal tokens
	registerExternalTokens();
	registerLogicTokens();
	registerMathTokens();
	registerMirandaTokens();
	registerStrTokens();
	registerSystemTokens();
	registerVariablesTokens();
	registerRegExpTokens();
	registerInetTokens();
	registerXsltTokens();
	registerAliasTokens();
	registerMetaContactsTokens();

	log_debugA("Variables: Internal tokens registered");

	if (db_getb(SETTING_PARSEATSTARTUP, 0)) {
		FORMATINFO fi;

		ZeroMemory(&fi, sizeof(fi));
		fi.cbSize = sizeof(fi);
		fi.tszFormat = db_gets(SETTING_STARTUPTEXT, NULL);
		if (fi.tszFormat != NULL) {
			freeMemory((WPARAM)formatString(&fi), 0);
			free(fi.tszFormat);
		}
	}
	log_debugA("Variables: Init done");

	return 0;
}

int UnloadVarModule() {

	UnhookEvent(hOptionsHook);
	if (hIconsChangedHook != NULL)
		UnhookEvent(hIconsChangedHook);

	DestroyServiceFunction(hRegisterVariableService);
	DestroyServiceFunction(hFreeMemoryService);
	DestroyServiceFunction(hFormatStringService);
	DestroyServiceFunction(hGetMMIService);
	DestroyServiceFunction(hShowHelpService);
	DestroyServiceFunction(hShowHelpExService);
	DestroyServiceFunction(hGetIconService);
	DestroyCursor(hCurSplitNS);
	deinitContactModule();
	deInitExternal();
	deinitTokenRegister();
	unregisterAliasTokens();
	unregisterVariablesTokens();
	return 0;
}
