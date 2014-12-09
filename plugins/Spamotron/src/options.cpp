#include "common.h"

TCHAR currentResponse[256] = {0};

TCHAR* _getCOptS(TCHAR *buf, unsigned int buflen, MCONTACT hContact, const char* option, const TCHAR *def)
{
	DBVARIANT dbv = {0};
	_tcsnset(buf, 0, buflen);
	if (db_get_ts(hContact, PLUGIN_NAME, option, &dbv) != 0)
		_tcsncpy(buf, def, min(buflen, _tcslen(def)+1));
	else if (dbv.type == DBVT_TCHAR) {
		_tcsncpy(buf, dbv.ptszVal, min(buflen, _tcslen(dbv.ptszVal)+1));
	}
	db_free(&dbv);
	return buf;
}
TCHAR* _getMOptS(TCHAR *buf, unsigned int buflen, const char* module, const char* option, const TCHAR *def)
{
	TCHAR* tmp;
	DBVARIANT dbv = {0};
	_tcsnset(buf, 0, buflen);
	if (db_get_s(NULL, module, option, &dbv) != 0)
		_tcsncpy(buf, def, min(buflen, _tcslen(def)+1));
	else if (dbv.type == DBVT_TCHAR) {
		_tcsncpy(buf, dbv.ptszVal, min(buflen, _tcslen(dbv.ptszVal)+1));
	} else {
		tmp = mir_a2u(dbv.pszVal);
		_tcsncpy(buf, tmp, min(buflen, _tcslen(tmp)+1));
		mir_free(tmp);
	}
	db_free(&dbv);
	return buf;
}


BOOL _saveDlgItemText(HWND hDialog, int controlID, char* option)
{
	int len;
	TCHAR *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (TCHAR *)malloc((len + 1)*sizeof(TCHAR));
	GetDlgItemText(hDialog, controlID, tmp, len + 1);
	_setOptTS(option, tmp);
	free(tmp);
	return TRUE;
}
int _saveDlgItemResponse(HWND hDialog, int controlID, char* option)
{
	int ret = 0;
	int isRegex = 0;
	int len;
	TCHAR *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (TCHAR*)malloc((len+1)*sizeof(TCHAR));
	GetDlgItemText(hDialog, controlID, tmp, len+1);
	isRegex = _isregex(tmp);
	if (!isRegex)
		ret = _saveDlgItemText(hDialog, controlID, option) ? 1 : 0;
	else {
		if (_isvalidregex(tmp))
			ret = _saveDlgItemText(hDialog, controlID, option) ? 1 : 0;
		else
			ret = -1;
	}
	free(tmp);
	return ret;
}
BOOL _saveDlgItemInt(HWND hDialog, int controlID, char* option)
{
	int len;
	TCHAR *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (TCHAR *)malloc((len + 1)*sizeof(TCHAR));
	GetDlgItemText(hDialog, controlID, tmp, len + 1);
	_setOptD(option, _ttoi(tmp));
	free(tmp);
	return TRUE;
}
BOOL _saveDlgItemScore(HWND hDialog, int controlID, char* option)
{
	int len;
	TCHAR *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (TCHAR *)malloc((len + 1)*sizeof(TCHAR));
	GetDlgItemText(hDialog, controlID, tmp, len + 1);
	_setOptD(option, _tcstod(tmp, NULL)/SCORE_C);
	return TRUE;
}

extern HINSTANCE hInst;

INT_PTR CALLBACK DlgProcOptionsMain(HWND optDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0, i, j, numProtocols;
	PROTOCOLDESCRIPTOR** pd;
	TCHAR pName[256] = {0};
	char protoOption[256] = {0};
	char protoName[256] = {0};
	HWND hProtocolsList = GetDlgItem(optDlg, IDC_OPT_PROTOCOLS);
	LVITEM lvi = {0};
	LVCOLUMN lvc = {0};
	TCHAR buf[512];

	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(optDlg);
			bInitializing = 1;
			
			///Main enable switch
			SendDlgItemMessage(optDlg, IDC_OPT_OUT_MSG_APPROVE, BM_SETCHECK, _getOptB("ApproveOnMsgOut", defaultApproveOnMsgOut), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_IN_MSG_APPROVE, BM_SETCHECK, _getOptB("ApproveOnMsgIn", defaultApproveOnMsgIn), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_DONT_REPLY_SAME_MSG, BM_SETCHECK, _getOptB("DontReplySameMsg", defaultDontReplySameMsg), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_DONT_REPLY_MSG, BM_SETCHECK, _getOptB("DontReplyMsg", defaultDontReplyMsg), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_HIDE_UNTIL_VERIFIED, BM_SETCHECK, _getOptB("HideUnverified", defaultHideUnverified), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_ADD_PERMANENTLY, BM_SETCHECK, _getOptB("AddPermanently", defaultAddPermanently), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_LOG_ACTIONS, BM_SETCHECK, _getOptB("LogActions", defaultLogActions), 0);

			SetDlgItemText(optDlg, IDC_OPT_IN_MSG_APPROVE_WORDLIST, _getOptS(buf, SIZEOF(buf), "ApproveOnMsgInWordlist", defaultApproveOnMsgInWordlist));
			SetDlgItemText(optDlg, IDC_OPT_MAX_MSG_CONTACT, _itot((unsigned int)_getOptD("MaxMsgContactCountPerDay", defaultMaxMsgContactCountPerDay), buf, 10));
			SetDlgItemText(optDlg, IDC_OPT_MAX_SAME_MSG, _itot((unsigned int)_getOptD("MaxSameMsgCountPerDay", defaultMaxSameMsgCountPerDay), buf, 10));
			SetDlgItemText(optDlg, IDC_OPT_DONT_REPLY_MSG_WORDLIST, _getOptS(buf, SIZEOF(buf), "DontReplyMsgWordlist", defaultDontReplyMsgWordlist));

			///Individual protocols list
			ListView_SetExtendedListViewStyle(hProtocolsList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
			lvc.mask = LVCF_WIDTH;
			lvc.cx = 120;
			ListView_InsertColumn(hProtocolsList, 0, &lvc);
			lvi.mask = LVIF_TEXT | LVIF_STATE;
			CallService(MS_PROTO_ENUMACCOUNTS, (LPARAM)&numProtocols, (WPARAM)&pd);
			for (i = 0, j = 0; i < numProtocols; i++)
			{
				lvi.iItem = i;
				_getMOptS(pName, 200*sizeof(TCHAR), pd[i]->szName, "AM_BaseProto", _T(""));
				if (_tcscmp(pName, _T("ICQ")) != 0)
					continue;
				lvi.pszText = mir_a2u(pd[i]->szName);
				ListView_InsertItem(hProtocolsList, &lvi);
				memset(protoOption, 0, sizeof(protoOption));
				strcat(protoOption, "proto_");
				strcat(protoOption, pd[i]->szName);
				ListView_SetCheckState(hProtocolsList, j++, _getOptB(protoOption, 0));
			}

			bInitializing = 0;
			return TRUE;

		case WM_COMMAND:
			if (bInitializing)
				return FALSE;
			switch (LOWORD(wParam)) {
				case IDC_OPT_OUT_MSG_APPROVE:
				case IDC_OPT_IN_MSG_APPROVE:
				case IDC_OPT_DONT_REPLY_SAME_MSG:
				case IDC_OPT_DONT_REPLY_MSG:
				case IDC_OPT_ADD_PERMANENTLY:
				case IDC_OPT_HIDE_UNTIL_VERIFIED:
				case IDC_OPT_LOG_ACTIONS:
					if (HIWORD(wParam) != BN_CLICKED)
						return  FALSE;
					break;
				case IDC_OPT_IN_MSG_APPROVE_WORDLIST:
				case IDC_OPT_MAX_MSG_CONTACT:
				case IDC_OPT_MAX_SAME_MSG:
				case IDC_OPT_DONT_REPLY_MSG_WORDLIST:
					if (HIWORD(wParam) != EN_CHANGE)
						return FALSE;
					break;
			}
			SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			if (bInitializing)
				return FALSE;
			switch (LOWORD(wParam)) {
				case IDC_OPT_PROTOCOLS:
					if (
						((LPNMHDR)lParam)->code == LVN_ITEMCHANGED && 
						((LPNMLISTVIEW)lParam)->uChanged & LVIF_STATE &&
						(((LPNMLISTVIEW)lParam)->uOldState & LVIS_STATEIMAGEMASK) != \
						  (((LPNMLISTVIEW)lParam)->uNewState & LVIS_STATEIMAGEMASK)
						)
						SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
					break;
			}
			switch (((NMHDR*)lParam)->code) {
				case PSN_APPLY:
					_setOptB("ApproveOnMsgOut", SendDlgItemMessage(optDlg, IDC_OPT_OUT_MSG_APPROVE, BM_GETCHECK, 0, 0));
					_setOptB("ApproveOnMsgIn", SendDlgItemMessage(optDlg, IDC_OPT_IN_MSG_APPROVE, BM_GETCHECK, 0, 0));
					_setOptB("DontReplySameMsg", SendDlgItemMessage(optDlg, IDC_OPT_DONT_REPLY_SAME_MSG, BM_GETCHECK, 0, 0));
					_setOptB("DontReplyMsg", SendDlgItemMessage(optDlg, IDC_OPT_DONT_REPLY_MSG, BM_GETCHECK, 0, 0));
					_setOptB("AddPermanently", SendDlgItemMessage(optDlg, IDC_OPT_ADD_PERMANENTLY, BM_GETCHECK, 0, 0));
					_setOptB("HideUnverified", SendDlgItemMessage(optDlg, IDC_OPT_HIDE_UNTIL_VERIFIED, BM_GETCHECK, 0, 0));
					_setOptB("LogActions", SendDlgItemMessage(optDlg, IDC_OPT_LOG_ACTIONS, BM_GETCHECK, 0, 0));
					_saveDlgItemText(optDlg, IDC_OPT_IN_MSG_APPROVE_WORDLIST, "ApproveOnMsgInWordlist");
					_saveDlgItemText(optDlg, IDC_OPT_DONT_REPLY_MSG_WORDLIST, "DontReplyMsgWordlist");
					_saveDlgItemInt(optDlg, IDC_OPT_MAX_MSG_CONTACT, "MaxMsgContactCountPerDay");
					_saveDlgItemInt(optDlg, IDC_OPT_MAX_SAME_MSG, "MaxSameMsgCountPerDay");
					numProtocols = ListView_GetItemCount(hProtocolsList);
					for (i = 0; i < numProtocols; i++) {
						ListView_GetItemText(hProtocolsList, i, 0, buf, SIZEOF(buf));
						//wcstombs(protoName, buf, SIZEOF(buf));
						memset(protoOption, 0, sizeof(protoOption));
						strcat(protoOption, "proto_");
						strcat(protoOption, mir_u2a(buf));
						_setOptB(protoOption, ListView_GetCheckState(hProtocolsList, i));
					}
					return TRUE;
			}
			break;
		case WM_DESTROY:
			break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcOptionsQuestion(HWND optDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	int i, selectedMode;
	HWND ht;
	TCHAR *buf; 
	unsigned int buflen = 500;

	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(optDlg);
			bInitializing = 1;

			ht = GetDlgItem(optDlg, IDC_OPT_MODE);
			SendMessage(ht, CB_ADDSTRING, 0, (LPARAM)TranslateT("Simple"));
			SendMessage(ht, CB_SETITEMDATA, 0, SPAMOTRON_MODE_PLAIN);
			SendMessage(ht, CB_ADDSTRING, 0, (LPARAM)TranslateT("Math expression"));
			SendMessage(ht, CB_SETITEMDATA, 1, SPAMOTRON_MODE_MATH);
			SendMessage(ht, CB_ADDSTRING, 0, (LPARAM)TranslateT("Round-robin"));
			SendMessage(ht, CB_SETITEMDATA, 2, SPAMOTRON_MODE_ROTATE);
			SendMessage(ht, CB_ADDSTRING, 0, (LPARAM)TranslateT("Random"));
			SendMessage(ht, CB_SETITEMDATA, 3, SPAMOTRON_MODE_RANDOM);
			
			selectedMode = _getOptB("Mode", defaultMode);
			for (i = 0; i < SendMessage(ht, CB_GETCOUNT, 0, 0); i++) {
				if (SendMessage(ht, CB_GETITEMDATA, i, 0) == selectedMode) {
					SendMessage(ht, CB_SETCURSEL, i, 0);
					break;
				}
			}
			SetDlgItemText(optDlg, IDC_OPT_MATH_RESPONSE, TranslateT("Will be automatically evaluated from %mathexpr%"));
			buf = (TCHAR *)malloc(buflen*sizeof(TCHAR));
			switch (selectedMode) {
				case SPAMOTRON_MODE_PLAIN:	
				case SPAMOTRON_MODE_ROTATE:
				case SPAMOTRON_MODE_RANDOM:
					ShowWindow(GetDlgItem(optDlg, IDC_STATIC_MODEMSG), 0);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_MATH_RESPONSE), 0);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), 1);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_CCRESPONSE), 1);
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), TRUE);
					SetDlgItemText(optDlg, IDC_OPT_CHALLENGE, _getOptS(buf, buflen, "Challenge", defaultChallenge));
					SetDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, _getOptS(buf, buflen, "AuthChallenge", defaultAuthChallenge));
					break;
				case SPAMOTRON_MODE_MATH:
					ShowWindow(GetDlgItem(optDlg, IDC_STATIC_MODEMSG), 1);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_MATH_RESPONSE), 1);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), 0);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_CCRESPONSE), 0);
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), FALSE);
					SetDlgItemText(optDlg, IDC_OPT_CHALLENGE, _getOptS(buf, buflen, "ChallengeMath", defaultChallengeMath));
					SetDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, _getOptS(buf, buflen, "AuthChallengeMath", defaultAuthChallengeMath));
					break;
			}
			SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_SUCCESS, BM_SETCHECK, _getOptB("ReplyOnSuccess", defaultReplyOnSuccess), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_AUTH, BM_SETCHECK, _getOptB("ReplyOnAuth", defaultReplyOnAuth), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_MSG, BM_SETCHECK, _getOptB("ReplyOnMsg", defaultReplyOnMsg), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_KEEP_BLOCKED_MSG, BM_SETCHECK, _getOptB("KeepBlockedMsg", defaultKeepBlockedMsg), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL, BM_SETCHECK, _getOptB("MarkMsgUnreadOnApproval", defaultMarkMsgUnreadOnApproval), 0);
			EnableWindow(GetDlgItem(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL), _getOptB("KeepBlockedMsg", defaultKeepBlockedMsg));
			SendDlgItemMessage(optDlg, IDC_OPT_CCRESPONSE, BM_SETCHECK, _getOptB("ResponseCC", defaultResponseCC), 0);
			SetDlgItemText(optDlg, IDC_OPT_RESPONSE, _getOptS(buf, buflen, "Response", defaultResponse));
			SetDlgItemText(optDlg, IDC_OPT_SUCCESS_RESPONSE, _getOptS(buf, buflen, "SuccessResponse", defaultSuccessResponse));
			free(buf);

			bInitializing = 0;
			return TRUE;

		case WM_COMMAND:
			if (bInitializing)
				return FALSE;
			switch (LOWORD(wParam)) {
				case IDC_OPT_MODE:
					if (HIWORD(wParam) != CBN_SELCHANGE)
						return FALSE;
					i = SendMessage(GetDlgItem(optDlg, IDC_OPT_MODE), CB_GETCURSEL, 0, 0);
					selectedMode = SendMessage(GetDlgItem(optDlg, IDC_OPT_MODE), CB_GETITEMDATA, i, 0);
					buf = (TCHAR*)malloc(buflen*sizeof(TCHAR));
					switch (selectedMode) {
						case SPAMOTRON_MODE_PLAIN:	
						case SPAMOTRON_MODE_ROTATE:
						case SPAMOTRON_MODE_RANDOM:
							ShowWindow(GetDlgItem(optDlg, IDC_STATIC_MODEMSG), 0);
							ShowWindow(GetDlgItem(optDlg, IDC_OPT_MATH_RESPONSE), 0);
							ShowWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), 1);
							ShowWindow(GetDlgItem(optDlg, IDC_OPT_CCRESPONSE), 1);
							EnableWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), TRUE);
							SetDlgItemText(optDlg, IDC_OPT_CHALLENGE, _getOptS(buf, buflen, "Challenge", defaultChallenge));
							SetDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, _getOptS(buf, buflen, "AuthChallenge", defaultAuthChallenge));
							break;
						case SPAMOTRON_MODE_MATH:
							ShowWindow(GetDlgItem(optDlg, IDC_STATIC_MODEMSG), 1);
							ShowWindow(GetDlgItem(optDlg, IDC_OPT_MATH_RESPONSE), 1);
							ShowWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), 0);
							ShowWindow(GetDlgItem(optDlg, IDC_OPT_CCRESPONSE), 0);
							EnableWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), FALSE);
							SetDlgItemText(optDlg, IDC_OPT_CHALLENGE, _getOptS(buf, buflen, "ChallengeMath", defaultChallengeMath));
							SetDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, _getOptS(buf, buflen, "AuthChallengeMath", defaultAuthChallengeMath));
							break;
					}
					free(buf);
					break;
				case IDC_OPT_REPLY_ON_SUCCESS:
				case IDC_OPT_REPLY_ON_AUTH:
				case IDC_OPT_REPLY_ON_MSG:
				case IDC_OPT_KEEP_BLOCKED_MSG:
				case IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL:
				case IDC_OPT_CCRESPONSE:
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL), SendDlgItemMessage(optDlg, IDC_OPT_KEEP_BLOCKED_MSG, BM_GETCHECK, 0, 0));
					if (HIWORD(wParam) != BN_CLICKED)
						return FALSE;
					break;
				case IDC_OPT_CHALLENGE:
				case IDC_OPT_RESPONSE:
				case IDC_OPT_SUCCESS_RESPONSE:
				case IDC_OPT_AUTH_CHALLENGE:
					if (HIWORD(wParam) != EN_CHANGE)
						return FALSE;
					break;
				case IDC_DEFAULTS:
					SetDlgItemText(optDlg, IDC_STATIC_MODEMSG, _T(""));
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_MATH_RESPONSE), 0);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), 1);
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), TRUE);
					SendDlgItemMessage(optDlg, IDC_OPT_MODE, CB_SETCURSEL, 0, 0);
					SendDlgItemMessage(optDlg, IDC_OPT_CCRESPONSE, BM_SETCHECK, defaultResponseCC, 0);
					SetDlgItemText(optDlg, IDC_OPT_CHALLENGE, defaultChallenge);
					SetDlgItemText(optDlg, IDC_OPT_RESPONSE, defaultResponse);
					SetDlgItemText(optDlg, IDC_OPT_SUCCESS_RESPONSE, defaultSuccessResponse);
					SetDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, defaultAuthChallenge);
					SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_SUCCESS, BM_SETCHECK, defaultReplyOnSuccess, 0);
					SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_AUTH, BM_SETCHECK, defaultReplyOnAuth, 0);
					SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_MSG, BM_SETCHECK, defaultReplyOnMsg, 0);
					SendDlgItemMessage(optDlg, IDC_OPT_KEEP_BLOCKED_MSG, BM_SETCHECK, defaultKeepBlockedMsg, 0);
					SendDlgItemMessage(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL, BM_SETCHECK, defaultMarkMsgUnreadOnApproval, 0);
					break;
			}
			SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
			break;	
		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code) {
				case PSN_APPLY:
					i = SendMessage(GetDlgItem(optDlg, IDC_OPT_MODE), CB_GETCURSEL, 0, 0);
					selectedMode = SendMessage(GetDlgItem(optDlg, IDC_OPT_MODE), CB_GETITEMDATA, i, 0);
					_setOptB("Mode", selectedMode);
					_setOptB("ReplyOnSuccess", SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_SUCCESS, BM_GETCHECK, 0, 0));
					_setOptB("ReplyOnAuth", SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_AUTH, BM_GETCHECK, 0, 0));
					_setOptB("ReplyOnMsg", SendDlgItemMessage(optDlg, IDC_OPT_REPLY_ON_MSG, BM_GETCHECK, 0, 0));
					_setOptB("KeepBlockedMsg", SendDlgItemMessage(optDlg, IDC_OPT_KEEP_BLOCKED_MSG, BM_GETCHECK, 0, 0));
					_setOptB("MarkMsgUnreadOnApproval", SendDlgItemMessage(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL, BM_GETCHECK, 0, 0));
					_setOptB("ResponseCC", SendDlgItemMessage(optDlg, IDC_OPT_CCRESPONSE, BM_GETCHECK, 0, 0));
					switch (selectedMode) {
						case SPAMOTRON_MODE_PLAIN:
							_saveDlgItemText(optDlg, IDC_OPT_CHALLENGE, "Challenge");
							_saveDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, "AuthChallenge");
							break;
						case SPAMOTRON_MODE_MATH:
							_saveDlgItemText(optDlg, IDC_OPT_CHALLENGE, "ChallengeMath");
							_saveDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, "AuthChallengeMath");
							break;
					}
					if (_saveDlgItemResponse(optDlg, IDC_OPT_RESPONSE, "Response") == -1) {
						MessageBox(NULL, TranslateT("Invalid regular expression.\nKeeping previous value."), L"Error", MB_OK);
						return FALSE;
					}
					_saveDlgItemText(optDlg, IDC_OPT_SUCCESS_RESPONSE, "SuccessResponse");

					return TRUE;
			}
			break;
		case WM_DESTROY:
			break;
	}
	return FALSE;
}


void EnableControlsBayes(HWND hwnd, BOOL enable)
{
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_FILTERING_GROUP), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_LEARNING_GROUP), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_BLOCK_MSG), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_SPAM_SCORE), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_AUTO_APPROVE), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_HAM_SCORE), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_AUTOLEARN_APPROVED), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED), enable &&
		SendDlgItemMessage(hwnd, IDC_OPT_BAYES_AUTO_APPROVE, BM_GETCHECK, 0, 0));
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_AUTOLEARN_NOT_APPROVED), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_WAIT_APPROVE), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_AUTOLEARN_NOT_APPROVED2), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_STATIC_DAYSASSPAM), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_AUTOLEARN_OUTGOING), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_LEARNBOX), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_SPAM), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_HAM), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_CHECK_MSG), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SPAM_COUNT), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_STATIC_HAM_COUNT), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SPAMCOUNT_LABEL), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_STATIC_HAMCOUNT_LABEL), enable);
}

INT_PTR CALLBACK DlgProcOptionsBayes(HWND optDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0, len;
	BOOL bEnabled;
	TCHAR *dbuf;
	TCHAR buf[MAX_BUFFER_LENGTH];
	char cbuf[MAX_BUFFER_LENGTH];
	switch (msg) {
		case WM_INITDIALOG:
			bInitializing = 1;
			TranslateDialogDefault(optDlg);
			bEnabled = _getOptB("BayesEnabled", defaultBayesEnabled);
			SendDlgItemMessage(optDlg, IDC_OPT_BAYES_ENABLED, BM_SETCHECK, bEnabled, 0);
			SendDlgItemMessage(optDlg, IDC_OPT_BAYES_BLOCK_MSG, BM_SETCHECK, _getOptB("BayesBlockMsg", defaultBayesBlockMsg), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTO_APPROVE, BM_SETCHECK, _getOptB("BayesAutoApprove", defaultBayesAutoApprove), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_APPROVED, BM_SETCHECK, _getOptB("BayesAutolearnApproved", defaultBayesAutolearnApproved), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED, BM_SETCHECK, _getOptB("BayesAutolearnAutoApproved", defaultBayesAutolearnAutoApproved), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_NOT_APPROVED, BM_SETCHECK, _getOptB("BayesAutolearnNotApproved", defaultBayesAutolearnNotApproved), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_OUTGOING, BM_SETCHECK, _getOptB("BayesAutolearnOutgoing", defaultBayesAutolearnOutgoing), 0);
			
			EnableControlsBayes(optDlg, bEnabled);

			mir_sntprintf(buf, SIZEOF(buf), _T("%0.02f"), (double)_getOptD("BayesSpamScore", defaultBayesSpamScore)*SCORE_C);
			SetDlgItemText(optDlg, IDC_OPT_BAYES_SPAM_SCORE, buf);
			mir_sntprintf(buf, SIZEOF(buf), _T("%.02f"), (double)_getOptD("BayesHamScore", defaultBayesHamScore)*SCORE_C);
			SetDlgItemText(optDlg, IDC_OPT_BAYES_HAM_SCORE, buf);
			mir_sntprintf(buf, SIZEOF(buf), _T("%d"), _getOptD("BayesWaitApprove", defaultBayesWaitApprove));
			SetDlgItemText(optDlg, IDC_OPT_BAYES_WAIT_APPROVE, buf);

			if (bEnabled) {
				mir_sntprintf(buf, SIZEOF(buf), _T("%d"), get_msg_count(SPAM));
				SetDlgItemText(optDlg, IDC_STATIC_SPAM_COUNT, buf);
				mir_sntprintf(buf, SIZEOF(buf), _T("%d"), get_msg_count(HAM));
				SetDlgItemText(optDlg, IDC_STATIC_HAM_COUNT, buf);
			}

			bInitializing = 0;
			break;
		case WM_COMMAND:
			if (bInitializing)
				return FALSE;
			switch (LOWORD(wParam)) {
				case IDC_OPT_BAYES_ENABLED:
					bEnabled = SendDlgItemMessage(optDlg, IDC_OPT_BAYES_ENABLED, BM_GETCHECK, 0, 0);
					EnableControlsBayes(optDlg, bEnabled);
				case IDC_OPT_BAYES_AUTO_APPROVE:
					bEnabled = SendDlgItemMessage(optDlg, IDC_OPT_BAYES_ENABLED, BM_GETCHECK, 0, 0);
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED), 
						bEnabled && SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTO_APPROVE, BM_GETCHECK, 0, 0));
				case IDC_OPT_BAYES_BLOCK_MSG:
				case IDC_OPT_BAYES_AUTOLEARN_APPROVED:
				case IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED:
				case IDC_OPT_BAYES_AUTOLEARN_NOT_APPROVED:
				case IDC_OPT_BAYES_AUTOLEARN_OUTGOING:
					if (HIWORD(wParam) != BN_CLICKED)
						return FALSE;
					break;
				case IDC_OPT_BAYES_SPAM_SCORE:
				case IDC_OPT_BAYES_HAM_SCORE:
				case IDC_OPT_BAYES_WAIT_APPROVE:
					if (HIWORD(wParam) != EN_CHANGE)
						return FALSE;
					break;
				case IDC_OPT_BAYES_LEARNBOX:
					return FALSE;
				case IDC_OPT_BAYES_HAM:
					// Learn ham from learnbox
					len = GetWindowTextLength(GetDlgItem(optDlg, IDC_OPT_BAYES_LEARNBOX))+1;
					dbuf = (TCHAR *)malloc(len*sizeof(TCHAR));
					if (!dbuf)
						return FALSE;
					GetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, dbuf, len);
					learn_ham(dbuf);
					SetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, _T(""));
					free(dbuf);

					mir_sntprintf(buf, SIZEOF(buf), _T("%d"), get_msg_count(SPAM));
					SetDlgItemText(optDlg, IDC_STATIC_SPAM_COUNT, buf);
					mir_sntprintf(buf, SIZEOF(buf), _T("%d"), get_msg_count(HAM));
					SetDlgItemText(optDlg, IDC_STATIC_HAM_COUNT, buf);
					return FALSE;

				case IDC_OPT_BAYES_SPAM:
					// Learn spam from learnbox
					len = GetWindowTextLength(GetDlgItem(optDlg, IDC_OPT_BAYES_LEARNBOX))+1;
					dbuf = (TCHAR *)malloc(len*sizeof(TCHAR));
					if (!dbuf)
						return FALSE;
					GetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, dbuf, len);
					learn_spam(dbuf);
					SetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, _T(""));
					free(dbuf);

					mir_sntprintf(buf, SIZEOF(buf), _T("%d"), get_msg_count(SPAM));
					SetDlgItemText(optDlg, IDC_STATIC_SPAM_COUNT, buf);
					mir_sntprintf(buf, SIZEOF(buf), _T("%d"), get_msg_count(HAM));
					SetDlgItemText(optDlg, IDC_STATIC_HAM_COUNT, buf);
					return FALSE;

				case IDC_CHECK_MSG:
					len = GetWindowTextLength(GetDlgItem(optDlg, IDC_OPT_BAYES_LEARNBOX))+1;
					dbuf = (TCHAR *)malloc((len)*sizeof(TCHAR));
					if (!dbuf)
						return FALSE;
					GetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, dbuf, len);
					mir_snprintf(cbuf, SIZEOF(cbuf), "%0.04f", get_msg_score(dbuf));
					SetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, _T(""));
					MessageBoxA(NULL, cbuf, Translate("Message score"), MB_OK);
					free(dbuf);
					return FALSE;

			}
			SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code) {
				case PSN_APPLY:
					_setOptB("BayesEnabled", SendDlgItemMessage(optDlg, IDC_OPT_BAYES_ENABLED, BM_GETCHECK, 0, 0));
					_setOptB("BayesBlockMsg", SendDlgItemMessage(optDlg, IDC_OPT_BAYES_BLOCK_MSG, BM_GETCHECK, 0, 0));
					_setOptB("BayesAutoApprove", SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTO_APPROVE, BM_GETCHECK, 0, 0));
					_setOptB("BayesAutolearnApproved", SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_APPROVED, BM_GETCHECK, 0, 0));
					_setOptB("BayesAutolearnAutoApproved", SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED, BM_GETCHECK, 0, 0));
					_setOptB("BayesAutolearnNotApproved", SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_NOT_APPROVED, BM_GETCHECK, 0, 0));
					_setOptB("BayesAutolearnOutgoing", SendDlgItemMessage(optDlg, IDC_OPT_BAYES_AUTOLEARN_OUTGOING, BM_GETCHECK, 0, 0));
					_saveDlgItemScore(optDlg, IDC_OPT_BAYES_SPAM_SCORE, "BayesSpamScore");
					_saveDlgItemScore(optDlg, IDC_OPT_BAYES_HAM_SCORE, "BayesHamScore");
					_saveDlgItemInt(optDlg, IDC_OPT_BAYES_WAIT_APPROVE, "BayesWaitApprove");
					break;
			}
			break;
		case WM_DESTROY:
			break;
	}
	return FALSE;
}

extern INT_PTR CALLBACK DlgProcOptionsPopups(HWND optDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int OnOptInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.ptszGroup = LPGENT("Message sessions");
	odp.ptszTitle = _T(PLUGIN_NAME);
	odp.flags = ODPF_TCHAR | ODPF_BOLDGROUPS;

	odp.ptszTab = LPGENT("Settings");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_MAIN);
	odp.pfnDlgProc = DlgProcOptionsMain;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Messages");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_Q);
	odp.pfnDlgProc = DlgProcOptionsQuestion;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Bayes");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_BAYES);
	odp.pfnDlgProc = DlgProcOptionsBayes;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_POPUPS);
		odp.pfnDlgProc = DlgProcOptionsPopups;
		odp.ptszGroup = LPGENT("Popups");
		odp.ptszTab = NULL;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}
