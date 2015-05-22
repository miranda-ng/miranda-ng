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

/* some handles */
static HANDLE
	hFormatStringService,
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

TCHAR* getArguments(TCHAR *string, TCHAR ***aargv, int *aargc)
{
	BOOL bDontParse, bNewArg, bDone;
	TCHAR *cur, *scur, **argv;
	int i, argc, brackets;

	*aargv = NULL;
	*aargc = 0;
	argc = brackets = 0;
	argv = NULL;
	cur = string;
	while (*cur == ' ')
		cur++;

	if (*cur != '(')
		return NULL;

	cur++;
	scur = cur-1;
	bDontParse = bNewArg = bDone = FALSE;
	while ( (!bDone) && (*cur != 0)) {
		switch (*cur) {
		case DONTPARSE_CHAR:
			bDontParse = !bDontParse;
			break;

		case ',':
			if ((!bDontParse) && (brackets == 0))
				bNewArg = TRUE;
			break;

		case '(':
			if (!bDontParse)
				brackets++;
			break;

		case ')':
			if ((brackets == 0) && (!bDontParse))
				bDone = bNewArg = TRUE;
			else if ((brackets > 0) && (!bDontParse))
				brackets--;
			break;
		}
		if (bNewArg) {
			argv = ( TCHAR** )mir_realloc(argv, (argc+1)*sizeof(TCHAR*));
			if (argv == NULL)
				return NULL;

			if (cur > scur) {
				argv[argc] = (TCHAR*)mir_alloc((cur-scur+2)*sizeof(TCHAR));
				if (argv[argc] == NULL) {
					mir_free(argv);
					return NULL;
				}

				memset(argv[argc], '\0', (cur-(scur+1)+1)*sizeof(TCHAR));
				_tcsncpy(argv[argc], scur+1, cur-(scur+1));
			}
			else argv[argc] = mir_tstrdup(_T(""));

			argc++;
			bNewArg = FALSE;
			scur = cur;
		}
		cur++;
	}
	// set args
	if (*(cur-1) == ')') {
		*aargv = argv;
		*aargc = argc;
	}
	else {
		for (i=0;i<argc;i++) {
			if (argv[i] != NULL) {
				mir_free(argv[i]);
			}
		}
		mir_free(argv);
		*aargv = NULL;
		*aargc = 0;
		cur = NULL;
	}

	return cur;
}

int isValidTokenChar(TCHAR tc) {

	return 
		(tc != '(') &&
		(tc != ',') &&
		(tc != ')') &&
		(tc != FIELD_CHAR) &&
		(tc != FUNC_CHAR) &&
		(tc != FUNC_ONCE_CHAR) &&
		(tc != '/') &&
		(tc != 0);
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
 	int argc = 0, i, scurPos, curPos, tmpVarPos;
	size_t pos;
	FORMATINFO afi;
	TOKENREGISTEREX *tr;
	ARGUMENTSINFO ai = { 0 };

	string = mir_tstrdup(szTemplate);
	if (string == NULL)
		return NULL;

	cur = tcur = scur = token = parsedToken = NULL;
	pargv = argv = NULL;
	//fi->pCount = 0;
	memcpy(&afi, fi, sizeof(afi));
	for (pos = 0; pos < _tcslen(string); pos++) {
		// string may move in memory, iterate by remembering the position in the string
		cur = string+pos;
		// mir_free memory from last iteration, this way we can bail out at any time in the loop
		mir_free(parsedToken);

		for (i = 0; i < argc; i ++)
			mir_free(argv[i]);
		mir_free(argv);
		argc = 0;
		tcur = scur = token = parsedToken = NULL;
		pargv = argv = NULL;
		// new round
		if (*cur == DONTPARSE_CHAR) {
			memmove(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
			if (*cur == DONTPARSE_CHAR)
				continue;

			while ( (*cur != DONTPARSE_CHAR) && (*cur != 0))
				cur++;

			memmove(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
			pos = cur-string-1;
			continue;
		}
		// remove end of lines
		else if ((!_tcsncmp(cur, _T("\r\n"), 2)) && (gParseOpts.bStripEOL)) {
			memmove(cur, cur+2, (_tcslen(cur+2)+1)*sizeof(TCHAR));
			pos = cur-string-1;
			continue;
		}
		else if ((*cur == '\n' && gParseOpts.bStripEOL) || (*cur == ' ' && gParseOpts.bStripWS)) {
			memmove(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
			pos = cur - string - 1;
			continue;
		}
		// remove comments
		else if (!_tcsncmp(cur, _T(COMMENT_STRING), _tcslen(_T(COMMENT_STRING)))) {
			scur = cur;
			while ( _tcsncmp(cur, _T("\r\n"), 2) && *cur != '\n' && *cur != 0)
				cur++;

			if (*cur == 0) {
				*scur = 0;
				string = (TCHAR*)mir_realloc(string, (_tcslen(string)+1)*sizeof(TCHAR));
				continue;
			}
			memmove(scur, cur, (_tcslen(cur)+1)*sizeof(TCHAR));
			pos = scur-string-1;
			continue;
		}
		else if ((*cur != FIELD_CHAR) && (*cur != FUNC_CHAR) && (*cur != FUNC_ONCE_CHAR)) {
			if (gParseOpts.bStripAll) {
				memmove(cur, cur+1, (_tcslen(cur+1)+1)*sizeof(TCHAR));
				pos = cur - string - 1;
			}
			continue;
		}
		scur = tcur = cur+1;
		while (isValidTokenChar(*tcur))
			tcur++;

		if (tcur == cur) {
			fi->eCount++;
			continue;
		}
		token = (TCHAR*)mir_alloc((tcur-scur+1)*sizeof(TCHAR));
		if (token == NULL) {
			fi->eCount++;
			return NULL;
		}
		memset(token, '\0', (tcur-scur+1)*sizeof(TCHAR));
		_tcsncpy(token, cur+1, tcur-scur);
		// cur points to FIELD_CHAR or FUNC_CHAR
 		tmpVarPos = -1;
 		tr = NULL;
 		if (*cur==FIELD_CHAR) {
 			for(i = 0; i < fi->cbTemporaryVarsSize; i += 2) {
 				if (mir_tstrcmp(fi->tszaTemporaryVars[i], token) == 0) {
 					tmpVarPos = i;
 					break;
 				}
 			}
 		}
 		if (tmpVarPos < 0)
 			tr = searchRegister(token, (*cur==FIELD_CHAR)?TRF_FIELD:TRF_FUNCTION);
 		mir_free(token);
 		if (tmpVarPos < 0 && tr == NULL) {
			fi->eCount++;
			// token not found, continue
			continue;
		}
		scur = cur; // store this pointer for later use
		if (*cur == FIELD_CHAR) {
 			size_t len = _tcslen(tr != NULL ? tr->tszTokenString : fi->tszaTemporaryVars[tmpVarPos]);
			cur++;
 			if (*(cur + len) != FIELD_CHAR) { // the next char after the token should be %
				fi->eCount++;
				continue;
			}
 			cur += len+1;
		}
		else if ((*cur == FUNC_CHAR) || (*cur == FUNC_ONCE_CHAR)) {
			TCHAR *argcur;

			cur += _tcslen(tr->tszTokenString)+1;
			argcur = getArguments(cur, &argv, &argc);
			if ((argcur == cur) || (argcur == NULL)) {
				fi->eCount++;
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
						mir_free(afi.szFormat);
					}
				}
				if (argv[i] == NULL)
					argv[i] = mir_tstrdup(_T(""));
			}
		}
		// cur should now point at the character after FIELD_CHAR or after the last ')'
 		if (tr != NULL) {
 			pargv = ( TCHAR** )mir_alloc((argc+1)*sizeof(TCHAR*));
 			if (pargv == NULL) {
 				fi->eCount++;
 				return NULL;
 			}
 			for (i=0;i<argc;i++)
 				pargv[i+1] = argv[i];

 			pargv[0] = tr->tszTokenString;
 			memset(&ai, 0, sizeof(ai));
 			ai.cbSize = sizeof(ai);
 			ai.argc = argc+1;
 			ai.targv = pargv;
 			ai.fi = fi;
 			if ((*scur == FUNC_ONCE_CHAR) || (*scur == FIELD_CHAR))
 				ai.flags |= AIF_DONTPARSE;

 			parsedToken = parseFromRegister(&ai);
 			mir_free(pargv);
 		}
 		else parsedToken = fi->tszaTemporaryVars[tmpVarPos + 1];

		if (parsedToken == NULL) {
			fi->eCount++;
			continue;
		}

		//replaced a var
		if (ai.flags & AIF_FALSE )
			fi->eCount++;
		else
			fi->pCount++;

		// 'special' chars need to be taken care of (DONTPARSE, TRYPARSE, \r\n)
		// if the var contains the escape character, this character must be doubled, we don't want it to act as an esacpe char
		/*for (tcur=parsedToken;*tcur != '\0';tcur++) {
			if (*tcur == DONTPARSE_CHAR) {//|| (*(var+pos) == ')')) {
				parsedToken = mir_realloc(parsedToken, mir_strlen(parsedToken) + 2);
				if (parsedToken == NULL) {
					fi->err = EMEM;
					return NULL;
				}
				memcpy(tcur+1, tcur, mir_strlen(tcur)+1);
				tcur++;
			}
		}*/

		size_t parsedTokenLen = _tcslen(parsedToken);
		size_t initStrLen = _tcslen(string);
		size_t tokenLen = cur-scur;
		scurPos = scur-string;
		curPos = cur-string;
		if (tokenLen < parsedTokenLen) {
			// string needs more memory
			string = (TCHAR*)mir_realloc(string, (initStrLen-tokenLen+parsedTokenLen+1)*sizeof(TCHAR));
			if (string == NULL) {
				fi->eCount++;
				return NULL;
			}
		}
		scur = string+scurPos;
		cur = string+curPos;
		memmove(scur + parsedTokenLen, cur, (_tcslen(cur)+1)*sizeof(TCHAR));
		memcpy(scur, parsedToken, parsedTokenLen*sizeof(TCHAR));
		{
			size_t len = _tcslen(string);
			string = (TCHAR*)mir_realloc(string, (len+1)*sizeof(TCHAR));
		}
		if (( ai.flags & AIF_DONTPARSE ) || tmpVarPos >= 0)
			pos += parsedTokenLen;

		pos--; // parse the same pos again, it changed

 		if (tr == NULL)
 			parsedToken = NULL; // To avoid mir_free
	}
	if (parsedToken != NULL)
		mir_free(parsedToken);

	for (i = 0; i < argc; i ++)
		mir_free(argv[i]);
	mir_free(argv);

	return (TCHAR*)mir_realloc(string, (_tcslen(string)+1)*sizeof(TCHAR));
}

/*
	MS_VARS_FORMATSTRING
*/
static INT_PTR formatStringService(WPARAM wParam, LPARAM lParam)
{
 	INT_PTR res;
 	int i;
 	BOOL copied;
	FORMATINFO *fi, tempFi;
	TCHAR *tszFormat, *orgFormat, *tszSource, *orgSource, *tRes;

 	if (((FORMATINFO *)wParam)->cbSize >= sizeof(FORMATINFO)) {
		memset(&tempFi, 0, sizeof(FORMATINFO));
		memcpy(&tempFi, (FORMATINFO *)wParam, sizeof(FORMATINFO));
		fi = &tempFi;
	}
 	else if (((FORMATINFO *)wParam)->cbSize == FORMATINFOV2_SIZE) {
 		memset(&tempFi, 0, sizeof(FORMATINFO));
 		memcpy(&tempFi, (FORMATINFO *)wParam, FORMATINFOV2_SIZE);
 		fi = &tempFi;
 	}
	else {
		// old struct, must be ANSI
		FORMATINFOV1 *fiv1 = (FORMATINFOV1 *)wParam;
		memset(&tempFi, 0, sizeof(FORMATINFO));
		tempFi.cbSize = sizeof(FORMATINFO);
		tempFi.hContact = fiv1->hContact;
		tempFi.szFormat = fiv1->szFormat;
		tempFi.szExtraText = fiv1->szSource;
		fi = &tempFi;
	}
	orgFormat = fi->tszFormat;
	orgSource = fi->tszExtraText;

	if (!(fi->flags&FIF_TCHAR)) {
 		copied = TRUE;
		log_debugA("mir_a2t (%s)", fi->szExtraText);
		tszFormat = fi->szFormat!=NULL?mir_a2t(fi->szFormat):NULL;
		tszSource = fi->szExtraText!=NULL?mir_a2t(fi->szExtraText):NULL;
 		for(i = 0; i < fi->cbTemporaryVarsSize; i++) {
 			fi->tszaTemporaryVars[i] = fi->szaTemporaryVars[i]!=NULL?mir_a2t(fi->szaTemporaryVars[i]):NULL;
 		}
	}
	else {
 		copied = FALSE;
 		tszFormat = fi->tszFormat;
 		tszSource = fi->tszExtraText;
	}

	fi->tszFormat = tszFormat;
	fi->tszExtraText = tszSource;

	tRes = formatString(fi);

	if (!(fi->flags&FIF_TCHAR)) {
		res = (INT_PTR)mir_u2a(tRes);
		mir_free(tRes);
	}
	else res = (INT_PTR)tRes;

 	if (copied) {
		mir_free(tszFormat);
		mir_free(tszSource);
 		for(i = 0; i < fi->cbTemporaryVarsSize; i++)
			mir_free(fi->tszaTemporaryVars);
 	}

	if (((FORMATINFO *)wParam)->cbSize == sizeof(FORMATINFOV1)) {
		((FORMATINFOV1 *)wParam)->eCount = fi->eCount;
		((FORMATINFOV1 *)wParam)->pCount = fi->pCount;
	}
	else {
		((FORMATINFO *)wParam)->eCount = fi->eCount;
		((FORMATINFO *)wParam)->pCount = fi->pCount;
	}

	return res;
}

TCHAR *formatString(FORMATINFO *fi)
{
	if (fi == NULL)
		return NULL;
	/* the service to format a given string */
	if ((fi->eCount + fi->pCount) > 5000) {
		fi->eCount++;
		fi->pCount++;
		log_debugA("Variables: Overflow protection; %d parses", (fi->eCount + fi->pCount));
		return NULL;
	}
	if (fi->tszFormat == NULL)
		return NULL;
	ptrT string( mir_tstrdup(fi->tszFormat));
	if (string == NULL)
		return NULL;

	return replaceDynVars(string, fi);
}

int setParseOptions(struct ParseOptions *po)
{
	if (po == NULL)
		po = &gParseOpts;

	memset(po, 0, sizeof(struct ParseOptions));
	if (!db_get_b(NULL, MODULENAME, SETTING_STRIPALL, 0)) {
		po->bStripEOL = db_get_b(NULL, MODULENAME, SETTING_STRIPCRLF, 0);
		po->bStripWS = db_get_b(NULL, MODULENAME, SETTING_STRIPWS, 0);
	}
	else po->bStripAll = TRUE;

	return 0;
}

static IconItem icon = { LPGEN("Help"), "vars_help", IDI_V };

int LoadVarModule()
{
	if (initTokenRegister() != 0 || initContactModule() != 0)
		return -1;

	setParseOptions(NULL);
	hFormatStringService = CreateServiceFunction(MS_VARS_FORMATSTRING, formatStringService);
	hRegisterVariableService = CreateServiceFunction(MS_VARS_REGISTERTOKEN, registerToken);
	// help dialog
	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);

	hShowHelpService = CreateServiceFunction(MS_VARS_SHOWHELP, showHelpService);
	hShowHelpExService = CreateServiceFunction(MS_VARS_SHOWHELPEX, showHelpExService);

	Icon_Register(hInst, LPGEN("Variables"), &icon, 1);

	hIconsChangedHook = HookEvent(ME_SKIN2_ICONSCHANGED, iconsChanged);

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
	registerAliasTokens();
	registerMetaContactsTokens();

	log_debugA("Variables: Internal tokens registered");

	if (db_get_b(NULL, MODULENAME, SETTING_PARSEATSTARTUP, 0)) {
		FORMATINFO fi = { 0 };
		fi.cbSize = sizeof(fi);
		fi.tszFormat = db_get_tsa(NULL, MODULENAME, SETTING_STARTUPTEXT);
		if (fi.tszFormat != NULL) {
			mir_free(formatString(&fi));
			mir_free(fi.tszFormat);
		}
	}
	log_debugA("Variables: Init done");

	return 0;
}

int UnloadVarModule()
{
	UnhookEvent(hOptionsHook);
	if (hIconsChangedHook != NULL)
		UnhookEvent(hIconsChangedHook);

	DestroyServiceFunction(hRegisterVariableService);
	DestroyServiceFunction(hFormatStringService);
	DestroyServiceFunction(hGetMMIService);
	DestroyServiceFunction(hShowHelpService);
	DestroyServiceFunction(hShowHelpExService);
	DestroyServiceFunction(hGetIconService);
	DestroyCursor(hCurSplitNS);
	deinitContactModule();
	deinitTokenRegister();
	unregisterAliasTokens();
	unregisterVariablesTokens();
	return 0;
}
