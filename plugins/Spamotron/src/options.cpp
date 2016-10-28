#include "stdafx.h"

wchar_t currentResponse[256] = {0};

wchar_t* _getCOptS(wchar_t *buf, unsigned int buflen, MCONTACT hContact, const char* option, const wchar_t *def)
{
	DBVARIANT dbv = {0};
	wcsnset(buf, 0, buflen);
	if (db_get_ws(hContact, PLUGIN_NAME, option, &dbv) != 0)
		wcsncpy(buf, def, min(buflen, mir_wstrlen(def)+1));
	else if (dbv.type == DBVT_WCHAR) {
		wcsncpy(buf, dbv.ptszVal, min(buflen, mir_wstrlen(dbv.ptszVal)+1));
	}
	db_free(&dbv);
	return buf;
}
wchar_t* _getMOptS(wchar_t *buf, unsigned int buflen, const char* module, const char* option, const wchar_t *def)
{
	wchar_t* tmp;
	DBVARIANT dbv = {0};
	wcsnset(buf, 0, buflen);
	if (db_get_s(NULL, module, option, &dbv) != 0)
		wcsncpy(buf, def, min(buflen, mir_wstrlen(def)+1));
	else if (dbv.type == DBVT_WCHAR) {
		wcsncpy(buf, dbv.ptszVal, min(buflen, mir_wstrlen(dbv.ptszVal)+1));
	} else {
		tmp = mir_a2u(dbv.pszVal);
		wcsncpy(buf, tmp, min(buflen, mir_wstrlen(tmp)+1));
		mir_free(tmp);
	}
	db_free(&dbv);
	return buf;
}


BOOL _saveDlgItemText(HWND hDialog, int controlID, char* option)
{
	int len;
	wchar_t *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (wchar_t *)malloc((len + 1)*sizeof(wchar_t));
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
	wchar_t *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (wchar_t*)malloc((len+1)*sizeof(wchar_t));
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
	wchar_t *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (wchar_t *)malloc((len + 1)*sizeof(wchar_t));
	GetDlgItemText(hDialog, controlID, tmp, len + 1);
	_setOptD(option, _wtoi(tmp));
	free(tmp);
	return TRUE;
}
BOOL _saveDlgItemScore(HWND hDialog, int controlID, char* option)
{
	int len;
	wchar_t *tmp;
	len = GetWindowTextLength(GetDlgItem(hDialog, controlID));
	tmp = (wchar_t *)malloc((len + 1)*sizeof(wchar_t));
	GetDlgItemText(hDialog, controlID, tmp, len + 1);
	_setOptD(option, wcstod(tmp, NULL)/SCORE_C);
	return TRUE;
}

extern HINSTANCE hInst;

INT_PTR CALLBACK DlgProcOptionsMain(HWND optDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0, i, j, numProtocols;
	PROTOACCOUNT **pd;
	wchar_t pName[256] = {0};
	char protoOption[256] = {0};
	HWND hProtocolsList = GetDlgItem(optDlg, IDC_OPT_PROTOCOLS);
	LVITEM lvi = {0};
	LVCOLUMN lvc = {0};
	wchar_t buf[512];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(optDlg);
		bInitializing = 1;

		///Main enable switch
		CheckDlgButton(optDlg, IDC_OPT_OUT_MSG_APPROVE, _getOptB("ApproveOnMsgOut", defaultApproveOnMsgOut) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(optDlg, IDC_OPT_IN_MSG_APPROVE, _getOptB("ApproveOnMsgIn", defaultApproveOnMsgIn) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(optDlg, IDC_OPT_DONT_REPLY_SAME_MSG, _getOptB("DontReplySameMsg", defaultDontReplySameMsg) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(optDlg, IDC_OPT_DONT_REPLY_MSG, _getOptB("DontReplyMsg", defaultDontReplyMsg) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(optDlg, IDC_OPT_HIDE_UNTIL_VERIFIED, _getOptB("HideUnverified", defaultHideUnverified) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(optDlg, IDC_OPT_ADD_PERMANENTLY, _getOptB("AddPermanently", defaultAddPermanently) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(optDlg, IDC_OPT_LOG_ACTIONS, _getOptB("LogActions", defaultLogActions) ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(optDlg, IDC_OPT_IN_MSG_APPROVE_WORDLIST, _getOptS(buf, _countof(buf), "ApproveOnMsgInWordlist", defaultApproveOnMsgInWordlist));
		SetDlgItemText(optDlg, IDC_OPT_MAX_MSG_CONTACT, _itow((unsigned int)_getOptD("MaxMsgContactCountPerDay", defaultMaxMsgContactCountPerDay), buf, 10));
		SetDlgItemText(optDlg, IDC_OPT_MAX_SAME_MSG, _itow((unsigned int)_getOptD("MaxSameMsgCountPerDay", defaultMaxSameMsgCountPerDay), buf, 10));
		SetDlgItemText(optDlg, IDC_OPT_DONT_REPLY_MSG_WORDLIST, _getOptS(buf, _countof(buf), "DontReplyMsgWordlist", defaultDontReplyMsgWordlist));

		///Individual protocols list
		ListView_SetExtendedListViewStyle(hProtocolsList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
		lvc.mask = LVCF_WIDTH;
		lvc.cx = 120;
		ListView_InsertColumn(hProtocolsList, 0, &lvc);
		lvi.mask = LVIF_TEXT | LVIF_STATE;
		Proto_EnumAccounts(&numProtocols, &pd);
		for (i = 0, j = 0; i < numProtocols; i++)
		{
			lvi.iItem = i;
			_getMOptS(pName, 200*sizeof(wchar_t), pd[i]->szModuleName, "AM_BaseProto", L"");
			if (mir_wstrcmp(pName, L"ICQ") != 0)
				continue;
			lvi.pszText = mir_a2u(pd[i]->szModuleName);
			ListView_InsertItem(hProtocolsList, &lvi);
			memset(protoOption, 0, sizeof(protoOption));
			mir_strcat(protoOption, "proto_");
			mir_strcat(protoOption, pd[i]->szModuleName);
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
			_setOptB("ApproveOnMsgOut", IsDlgButtonChecked(optDlg, IDC_OPT_OUT_MSG_APPROVE));
			_setOptB("ApproveOnMsgIn", IsDlgButtonChecked(optDlg, IDC_OPT_IN_MSG_APPROVE));
			_setOptB("DontReplySameMsg", IsDlgButtonChecked(optDlg, IDC_OPT_DONT_REPLY_SAME_MSG));
			_setOptB("DontReplyMsg", IsDlgButtonChecked(optDlg, IDC_OPT_DONT_REPLY_MSG));
			_setOptB("AddPermanently", IsDlgButtonChecked(optDlg, IDC_OPT_ADD_PERMANENTLY));
			_setOptB("HideUnverified", IsDlgButtonChecked(optDlg, IDC_OPT_HIDE_UNTIL_VERIFIED));
			_setOptB("LogActions", IsDlgButtonChecked(optDlg, IDC_OPT_LOG_ACTIONS));
			_saveDlgItemText(optDlg, IDC_OPT_IN_MSG_APPROVE_WORDLIST, "ApproveOnMsgInWordlist");
			_saveDlgItemText(optDlg, IDC_OPT_DONT_REPLY_MSG_WORDLIST, "DontReplyMsgWordlist");
			_saveDlgItemInt(optDlg, IDC_OPT_MAX_MSG_CONTACT, "MaxMsgContactCountPerDay");
			_saveDlgItemInt(optDlg, IDC_OPT_MAX_SAME_MSG, "MaxSameMsgCountPerDay");
			numProtocols = ListView_GetItemCount(hProtocolsList);
			for (i = 0; i < numProtocols; i++) {
				ListView_GetItemText(hProtocolsList, i, 0, buf, _countof(buf));
				//wcstombs(protoName, buf, _countof(buf));
				memset(protoOption, 0, sizeof(protoOption));
				mir_strcat(protoOption, "proto_");
				mir_strcat(protoOption, mir_u2a(buf));
				_setOptB(protoOption, ListView_GetCheckState(hProtocolsList, i));
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcOptionsQuestion(HWND optDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	int i, selectedMode;
	HWND ht;
	wchar_t *buf; 
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
			buf = (wchar_t *)malloc(buflen*sizeof(wchar_t));
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
			CheckDlgButton(optDlg, IDC_OPT_REPLY_ON_SUCCESS, _getOptB("ReplyOnSuccess", defaultReplyOnSuccess) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_REPLY_ON_AUTH, _getOptB("ReplyOnAuth", defaultReplyOnAuth) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_REPLY_ON_MSG, _getOptB("ReplyOnMsg", defaultReplyOnMsg) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_KEEP_BLOCKED_MSG, _getOptB("KeepBlockedMsg", defaultKeepBlockedMsg) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL, _getOptB("MarkMsgUnreadOnApproval", defaultMarkMsgUnreadOnApproval) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL), _getOptB("KeepBlockedMsg", defaultKeepBlockedMsg));
			CheckDlgButton(optDlg, IDC_OPT_CCRESPONSE, _getOptB("ResponseCC", defaultResponseCC) ? BST_CHECKED : BST_UNCHECKED);
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
					i = SendDlgItemMessage(optDlg, IDC_OPT_MODE, CB_GETCURSEL, 0, 0);
					selectedMode = SendDlgItemMessage(optDlg, IDC_OPT_MODE, CB_GETITEMDATA, i, 0);
					buf = (wchar_t*)malloc(buflen*sizeof(wchar_t));
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
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL), IsDlgButtonChecked(optDlg, IDC_OPT_KEEP_BLOCKED_MSG));
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
					SetDlgItemText(optDlg, IDC_STATIC_MODEMSG, L"");
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_MATH_RESPONSE), 0);
					ShowWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), 1);
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_RESPONSE), TRUE);
					SendDlgItemMessage(optDlg, IDC_OPT_MODE, CB_SETCURSEL, 0, 0);
					CheckDlgButton(optDlg, IDC_OPT_CCRESPONSE, defaultResponseCC ? BST_CHECKED : BST_UNCHECKED);
					SetDlgItemText(optDlg, IDC_OPT_CHALLENGE, defaultChallenge);
					SetDlgItemText(optDlg, IDC_OPT_RESPONSE, defaultResponse);
					SetDlgItemText(optDlg, IDC_OPT_SUCCESS_RESPONSE, defaultSuccessResponse);
					SetDlgItemText(optDlg, IDC_OPT_AUTH_CHALLENGE, defaultAuthChallenge);
					CheckDlgButton(optDlg, IDC_OPT_REPLY_ON_SUCCESS, defaultReplyOnSuccess ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton(optDlg, IDC_OPT_REPLY_ON_AUTH, defaultReplyOnAuth ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton(optDlg, IDC_OPT_REPLY_ON_MSG, defaultReplyOnMsg ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton(optDlg, IDC_OPT_KEEP_BLOCKED_MSG, defaultKeepBlockedMsg ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL, defaultMarkMsgUnreadOnApproval ? BST_CHECKED : BST_UNCHECKED);
					break;
			}
			SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
			break;	
		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code) {
				case PSN_APPLY:
					i = SendDlgItemMessage(optDlg, IDC_OPT_MODE, CB_GETCURSEL, 0, 0);
					selectedMode = SendDlgItemMessage(optDlg, IDC_OPT_MODE, CB_GETITEMDATA, i, 0);
					_setOptB("Mode", selectedMode);
					_setOptB("ReplyOnSuccess", IsDlgButtonChecked(optDlg, IDC_OPT_REPLY_ON_SUCCESS));
					_setOptB("ReplyOnAuth", IsDlgButtonChecked(optDlg, IDC_OPT_REPLY_ON_AUTH));
					_setOptB("ReplyOnMsg", IsDlgButtonChecked(optDlg, IDC_OPT_REPLY_ON_MSG));
					_setOptB("KeepBlockedMsg", IsDlgButtonChecked(optDlg, IDC_OPT_KEEP_BLOCKED_MSG));
					_setOptB("MarkMsgUnreadOnApproval", IsDlgButtonChecked(optDlg, IDC_OPT_MARK_MSG_UNREAD_ON_APPROVAL));
					_setOptB("ResponseCC", IsDlgButtonChecked(optDlg, IDC_OPT_CCRESPONSE));
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
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED), enable && IsDlgButtonChecked(hwnd, IDC_OPT_BAYES_AUTO_APPROVE));
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
	wchar_t *dbuf;
	wchar_t buf[MAX_BUFFER_LENGTH];
	char cbuf[MAX_BUFFER_LENGTH];
	switch (msg) {
		case WM_INITDIALOG:
			bInitializing = 1;
			TranslateDialogDefault(optDlg);
			bEnabled = _getOptB("BayesEnabled", defaultBayesEnabled);
			CheckDlgButton(optDlg, IDC_OPT_BAYES_ENABLED, bEnabled ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_BAYES_BLOCK_MSG, _getOptB("BayesBlockMsg", defaultBayesBlockMsg) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_BAYES_AUTO_APPROVE, _getOptB("BayesAutoApprove", defaultBayesAutoApprove) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_BAYES_AUTOLEARN_APPROVED, _getOptB("BayesAutolearnApproved", defaultBayesAutolearnApproved) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED, _getOptB("BayesAutolearnAutoApproved", defaultBayesAutolearnAutoApproved) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_BAYES_AUTOLEARN_NOT_APPROVED, _getOptB("BayesAutolearnNotApproved", defaultBayesAutolearnNotApproved) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_BAYES_AUTOLEARN_OUTGOING, _getOptB("BayesAutolearnOutgoing", defaultBayesAutolearnOutgoing) ? BST_CHECKED : BST_UNCHECKED);
			
			EnableControlsBayes(optDlg, bEnabled);

			mir_snwprintf(buf, L"%0.02f", (double)_getOptD("BayesSpamScore", defaultBayesSpamScore)*SCORE_C);
			SetDlgItemText(optDlg, IDC_OPT_BAYES_SPAM_SCORE, buf);
			mir_snwprintf(buf, L"%.02f", (double)_getOptD("BayesHamScore", defaultBayesHamScore)*SCORE_C);
			SetDlgItemText(optDlg, IDC_OPT_BAYES_HAM_SCORE, buf);
			mir_snwprintf(buf, L"%d", _getOptD("BayesWaitApprove", defaultBayesWaitApprove));
			SetDlgItemText(optDlg, IDC_OPT_BAYES_WAIT_APPROVE, buf);

			if (bEnabled) {
				mir_snwprintf(buf, L"%d", get_msg_count(SPAM));
				SetDlgItemText(optDlg, IDC_STATIC_SPAM_COUNT, buf);
				mir_snwprintf(buf, L"%d", get_msg_count(HAM));
				SetDlgItemText(optDlg, IDC_STATIC_HAM_COUNT, buf);
			}

			bInitializing = 0;
			break;
		case WM_COMMAND:
			if (bInitializing)
				return FALSE;
			switch (LOWORD(wParam)) {
				case IDC_OPT_BAYES_ENABLED:
					bEnabled = IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_ENABLED);
					EnableControlsBayes(optDlg, bEnabled);
				case IDC_OPT_BAYES_AUTO_APPROVE:
					bEnabled = IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_ENABLED);
					EnableWindow(GetDlgItem(optDlg, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED), 
						bEnabled && IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_AUTO_APPROVE));
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
					dbuf = (wchar_t *)malloc(len*sizeof(wchar_t));
					if (!dbuf)
						return FALSE;
					GetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, dbuf, len);
					learn_ham(dbuf);
					SetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, L"");
					free(dbuf);

					mir_snwprintf(buf, L"%d", get_msg_count(SPAM));
					SetDlgItemText(optDlg, IDC_STATIC_SPAM_COUNT, buf);
					mir_snwprintf(buf, L"%d", get_msg_count(HAM));
					SetDlgItemText(optDlg, IDC_STATIC_HAM_COUNT, buf);
					return FALSE;

				case IDC_OPT_BAYES_SPAM:
					// Learn spam from learnbox
					len = GetWindowTextLength(GetDlgItem(optDlg, IDC_OPT_BAYES_LEARNBOX))+1;
					dbuf = (wchar_t *)malloc(len*sizeof(wchar_t));
					if (!dbuf)
						return FALSE;
					GetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, dbuf, len);
					learn_spam(dbuf);
					SetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, L"");
					free(dbuf);

					mir_snwprintf(buf, L"%d", get_msg_count(SPAM));
					SetDlgItemText(optDlg, IDC_STATIC_SPAM_COUNT, buf);
					mir_snwprintf(buf, L"%d", get_msg_count(HAM));
					SetDlgItemText(optDlg, IDC_STATIC_HAM_COUNT, buf);
					return FALSE;

				case IDC_CHECK_MSG:
					len = GetWindowTextLength(GetDlgItem(optDlg, IDC_OPT_BAYES_LEARNBOX))+1;
					dbuf = (wchar_t *)malloc((len)*sizeof(wchar_t));
					if (!dbuf)
						return FALSE;
					GetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, dbuf, len);
					mir_snprintf(cbuf, "%0.04f", get_msg_score(dbuf));
					SetDlgItemText(optDlg, IDC_OPT_BAYES_LEARNBOX, L"");
					MessageBoxA(NULL, cbuf, Translate("Message score"), MB_OK);
					free(dbuf);
					return FALSE;

			}
			SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code) {
				case PSN_APPLY:
					_setOptB("BayesEnabled", IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_ENABLED));
					_setOptB("BayesBlockMsg", IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_BLOCK_MSG));
					_setOptB("BayesAutoApprove", IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_AUTO_APPROVE));
					_setOptB("BayesAutolearnApproved", IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_AUTOLEARN_APPROVED));
					_setOptB("BayesAutolearnAutoApproved", IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_AUTOLEARN_AUTOAPPROVED));
					_setOptB("BayesAutolearnNotApproved", IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_AUTOLEARN_NOT_APPROVED));
					_setOptB("BayesAutolearnOutgoing", IsDlgButtonChecked(optDlg, IDC_OPT_BAYES_AUTOLEARN_OUTGOING));
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

int OnOptInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = PLUGIN_NAME;
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Settings");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_MAIN);
	odp.pfnDlgProc = DlgProcOptionsMain;
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Messages");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_Q);
	odp.pfnDlgProc = DlgProcOptionsQuestion;
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Bayes");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_BAYES);
	odp.pfnDlgProc = DlgProcOptionsBayes;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_SPAMOTRON_POPUPS);
		odp.pfnDlgProc = DlgProcOptionsPopups;
		odp.szGroup.a = LPGEN("Popups");
		odp.szTab.a = NULL;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}
