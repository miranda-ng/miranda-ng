#include "stdafx.h"

struct SmpData
{
	HWND dialog;
	TrustLevel oldlevel;
	ConnContext *context;
	bool responder;
	wchar_t *question;
};

typedef std::map<HANDLE, SmpData> SmpForContactMap;
SmpForContactMap smp_for_contact;

static INT_PTR CALLBACK DlgSMPUpdateProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ConnContext *context = (ConnContext*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			if (!lParam) {
				DestroyWindow(hwndDlg);
				return FALSE;
			}
			TranslateDialogDefault(hwndDlg);

			SmpData *data = (SmpData*)lParam;
			context = data->context;
			MCONTACT hContact = (UINT_PTR)context->app_data;
			data->dialog = hwndDlg;
			//smp_for_contact.insert(SmpForContactMap::value_type(context->app_data, *data));
			if (smp_for_contact[context->app_data].dialog) SendMessage(smp_for_contact[context->app_data].dialog, WMU_REFRESHSMP, 0, 0);
			smp_for_contact[context->app_data].context = data->context;
			smp_for_contact[context->app_data].dialog = hwndDlg;
			smp_for_contact[context->app_data].oldlevel = data->oldlevel;
			smp_for_contact[context->app_data].responder = data->responder;
			mir_free(data);

			wchar_t title[512], *proto = mir_a2u(Proto_GetBaseAccountName(hContact));
			const wchar_t *name = contact_get_nameT(hContact);
			mir_snwprintf(title, TranslateW(LANG_SMP_PROGRESS_TITLE), name, proto);
			SetWindowText(hwndDlg, title);
			mir_snwprintf(title, TranslateW(LANG_SMP_PROGRESS_DESC), name, proto);
			mir_free(proto);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_HEADPRO, title);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)context);

			// Move window to screen center
			// Get the owner window and dialog box rectangles. 
			HWND hwndOwner; RECT rcOwner, rcDlg, rc;
			if ((hwndOwner = GetParent(hwndDlg)) == nullptr)
				hwndOwner = GetDesktopWindow();

			GetWindowRect(hwndOwner, &rcOwner);
			GetWindowRect(hwndDlg, &rcDlg);
			CopyRect(&rc, &rcOwner);

			// Offset the owner and dialog box rectangles so that right and bottom 
			// values represent the width and height, and then offset the owner again 
			// to discard space taken up by the dialog box. 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
			OffsetRect(&rc, -rc.left, -rc.top);
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

			// The new position is the sum of half the remaining space and the owner's 
			// original position. 

			SetWindowPos(hwndDlg,
				HWND_TOP,
				rcOwner.left + (rc.right / 2),
				rcOwner.top + (rc.bottom / 2),
				0, 0,          // Ignores size arguments. 
				SWP_NOSIZE);

			// end center dialog

			SendDlgItemMessage(hwndDlg, IDC_PGB_SMP, PBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(hwndDlg, IDC_PGB_SMP, PBM_SETPOS, 10, 0);

			EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), true);
			EnableWindow(GetDlgItem(hwndDlg, IDOK), false);
		}
		return TRUE;

	case WMU_REFRESHSMP:
		SendDlgItemMessage(hwndDlg, IDC_PGB_SMP, PBM_SETPOS, wParam, 0);
		switch (wParam) {
		case 0:
			EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), false);
			EnableWindow(GetDlgItem(hwndDlg, IDOK), true);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_PROGRESS, TranslateW(LANG_SMP_ERROR));
			smp_for_contact.erase(context->app_data);
			break;

		case 100:
			EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), false);
			EnableWindow(GetDlgItem(hwndDlg, IDOK), true);
			smp_for_contact.erase(context->app_data);
			if (context->smstate->sm_prog_state == OTRL_SMP_PROG_SUCCEEDED) {
				if (context->active_fingerprint->trust && context->active_fingerprint->trust[0])
					SetDlgItemText(hwndDlg, IDC_STC_SMP_PROGRESS, TranslateW(LANG_SMP_SUCCESS));
				else 
					SetDlgItemText(hwndDlg, IDC_STC_SMP_PROGRESS, TranslateW(LANG_SMP_SUCCESS_VERIFY));
			}
			else SetDlgItemText(hwndDlg, IDC_STC_SMP_PROGRESS, TranslateW(LANG_SMP_FAILED));
			break;

		default:
			SetDlgItemText(hwndDlg, IDC_STC_SMP_PROGRESS, L"");
		}
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDOK:
				smp_for_contact.erase(context->app_data);
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, NULL);
				__fallthrough;

			case IDCANCEL:
				DestroyWindow(hwndDlg);
				break;
			}
		}
		break;

	case WM_DESTROY:
		if (context)
			otr_abort_smp(context);
		break;
	}

	return FALSE;
}

static void SMPInitUpdateDialog(ConnContext *context, bool responder)
{
	if (!context) return;
	SmpData *data = (SmpData*)mir_calloc(sizeof(SmpData));
	data->context = context;
	data->oldlevel = otr_context_get_trust(context);
	data->responder = responder;
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SMP_PROGRESS), nullptr, DlgSMPUpdateProc, (LPARAM)data);
}

static INT_PTR CALLBACK DlgSMPResponseProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ConnContext *context = (ConnContext*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (!lParam) {
			DestroyWindow(hwndDlg);
			return FALSE;
		}

		TranslateDialogDefault(hwndDlg);
		{
			SmpData *data = (SmpData*)lParam;
			context = data->context;
			MCONTACT hContact = (UINT_PTR)context->app_data;
			data->dialog = hwndDlg;
			//smp_for_contact.insert(SmpForContactMap::value_type(context->app_data, *data));
			if (smp_for_contact[context->app_data].dialog) SendMessage(smp_for_contact[context->app_data].dialog, WMU_REFRESHSMP, 0, 0);
			smp_for_contact[context->app_data].context = data->context;
			smp_for_contact[context->app_data].dialog = hwndDlg;
			smp_for_contact[context->app_data].oldlevel = data->oldlevel;
			smp_for_contact[context->app_data].responder = data->responder;

			wchar_t buff[512], *proto = mir_a2u(Proto_GetBaseAccountName(hContact));
			mir_snwprintf(buff, TranslateW(LANG_SMP_VERIFY_TITLE), contact_get_nameT(hContact), proto);
			mir_free(proto);
			SetWindowText(hwndDlg, buff);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_HEAD, buff);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)context);

			if (data->question) {
				mir_snwprintf(buff, TranslateW(LANG_OTR_SMPQUESTION_RESPOND_DESC), contact_get_nameT(hContact));

				SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

				SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, data->question);
				SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, TRUE, 0);
				SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateW(LANG_SMP_QUESTION));

				SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, L"");
				SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, FALSE, 0);
				SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateW(LANG_SMP_ANSWER));


				ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
				ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);

				mir_free(data->question);
			}
			else {
				mir_snwprintf(buff, TranslateW(LANG_OTR_SMPPASSWORD_RESPOND_DESC), contact_get_nameT(hContact));

				SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

				SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, L"");
				SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, TRUE, 0);
				SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, L"");

				SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, L"");
				SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, FALSE, 0);
				SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateW(LANG_SMP_PASSWORD));


				ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
				ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);
			}
			mir_free(data);

			// Move window to screen center
			// Get the owner window and dialog box rectangles. 
			HWND hwndOwner = GetParent(hwndDlg); 
			if (hwndOwner == nullptr)
				hwndOwner = GetDesktopWindow();

			RECT rcOwner, rcDlg, rc;
			GetWindowRect(hwndOwner, &rcOwner);
			GetWindowRect(hwndDlg, &rcDlg);
			CopyRect(&rc, &rcOwner);

			// Offset the owner and dialog box rectangles so that right and bottom 
			// values represent the width and height, and then offset the owner again 
			// to discard space taken up by the dialog box. 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
			OffsetRect(&rc, -rc.left, -rc.top);
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

			// The new position is the sum of half the remaining space and the owner's 
			// original position. 

			SetWindowPos(hwndDlg,
				HWND_TOP,
				rcOwner.left + (rc.right / 2),
				rcOwner.top + (rc.bottom / 2),
				0, 0,          // Ignores size arguments. 
				SWP_NOSIZE);

			// end center dialog
			return TRUE;
		}

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDOK:
				SMPInitUpdateDialog(context, true);
				{
					ptrA ans(GetDlgItemTextUtf(hwndDlg, IDC_EDT_SMP_FIELD2));
					otr_continue_smp(context, (const unsigned char *)ans.get(), mir_strlen(ans));
				}
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, NULL);
				DestroyWindow(hwndDlg);
				break;

			case IDCANCEL:
				smp_for_contact.erase(context->app_data);
				DestroyWindow(hwndDlg);
				break;
			}
		}
		break;

	case WM_DESTROY:
		if (context)
			otr_abort_smp(context);
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcSMPInitProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ConnContext *context = (ConnContext*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			if (!lParam) {
				DestroyWindow(hwndDlg);
				return FALSE;
			}
			TranslateDialogDefault(hwndDlg);

			context = (ConnContext*)lParam;
			if (smp_for_contact.find(context->app_data) != smp_for_contact.end()) {
				DestroyWindow(hwndDlg);
				return FALSE;
			}

			MCONTACT hContact = (UINT_PTR)context->app_data;
			wchar_t title[512], *proto = mir_a2u(Proto_GetBaseAccountName(hContact));
			mir_snwprintf(title, TranslateW(LANG_SMP_VERIFY_TITLE), contact_get_nameT(hContact), proto);
			mir_free(proto);
			SetWindowText(hwndDlg, title);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_HEAD, title);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			// Move window to screen center
			// Get the owner window and dialog box rectangles. 
			HWND hwndOwner = GetParent(hwndDlg); 
			if (hwndOwner == nullptr)
				hwndOwner = GetDesktopWindow();

			RECT rcOwner, rcDlg, rc;
			GetWindowRect(hwndOwner, &rcOwner);
			GetWindowRect(hwndDlg, &rcDlg);
			CopyRect(&rc, &rcOwner);

			// Offset the owner and dialog box rectangles so that right and bottom 
			// values represent the width and height, and then offset the owner again 
			// to discard space taken up by the dialog box. 
			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
			OffsetRect(&rc, -rc.left, -rc.top);
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

			// The new position is the sum of half the remaining space and the owner's 
			// original position. 
			SetWindowPos(hwndDlg,
				HWND_TOP,
				rcOwner.left + (rc.right / 2),
				rcOwner.top + (rc.bottom / 2),
				0, 0,          // Ignores size arguments. 
				SWP_NOSIZE);

			HWND cmb = GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE);
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_SMPTYPE_QUESTION));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_SMPTYPE_PASSWORD));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_SMPTYPE_FINGERPRINT));
			SendMessage(cmb, CB_SELECTSTRING, -1, (WPARAM)TranslateW(LANG_SMPTYPE_QUESTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE), TRUE);

			Fingerprint *fp = context->active_fingerprint;
			if (!fp) {
				DestroyWindow(hwndDlg);
				return FALSE;
			}

			wchar_t buff[1024];
			if (!fp->trust || fp->trust[0] == '\0')
				mir_snwprintf(buff, TranslateW(LANG_OTR_SMPQUESTION_VERIFY_DESC), contact_get_nameT(hContact));
			else
				mir_snwprintf(buff, TranslateW(LANG_OTR_SMPQUESTION_VERIFIED_DESC), contact_get_nameT(hContact));

			SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

			SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, L"");
			SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, FALSE, 0);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateW(LANG_SMP_QUESTION));

			SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, L"");
			SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, FALSE, 0);
			SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateW(LANG_SMP_ANSWER));


			ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
			ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);
			SetFocus(GetDlgItem(hwndDlg, IDC_CBO_SMP_CHOOSE));
		}
		return FALSE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			{
				MCONTACT hContact = (UINT_PTR)context->app_data;
				wchar_t szMsg[1024];
				switch (LOWORD(wParam)) {
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
				case IDOK:
					GetDlgItemText(hwndDlg, IDC_CBO_SMP_CHOOSE, szMsg, _countof(szMsg));
					if (wcsncmp(szMsg, TranslateW(LANG_SMPTYPE_QUESTION), _countof(szMsg)) == 0) {
						if (smp_for_contact.find(context->app_data) != smp_for_contact.end()) {
							mir_snwprintf(szMsg, TranslateW(LANG_SMP_IN_PROGRESS), contact_get_nameT(hContact));
							ShowError(szMsg);
						}
						else {
							ptrA quest(GetDlgItemTextUtf(hwndDlg, IDC_EDT_SMP_FIELD1));
							ptrA ans(GetDlgItemTextUtf(hwndDlg, IDC_EDT_SMP_FIELD2));

							SMPInitUpdateDialog(context, false);
							otr_start_smp(context, quest, (const unsigned char*)ans.get(), mir_strlen(ans));
						}

					}
					else if (wcsncmp(szMsg, TranslateW(LANG_SMPTYPE_PASSWORD), _countof(szMsg)) == 0) {
						if (smp_for_contact.find(context->app_data) != smp_for_contact.end()) {
							mir_snwprintf(szMsg, TranslateW(LANG_SMP_IN_PROGRESS), contact_get_nameT(hContact));
							ShowError(szMsg);
						}
						else {
							ptrA ans(GetDlgItemTextUtf(hwndDlg, IDC_EDT_SMP_FIELD2));

							SMPInitUpdateDialog(context, false);
							otr_start_smp(context, nullptr, (const unsigned char*)ans.get(), mir_strlen(ans));
						}

					}
					else break;
					DestroyWindow(hwndDlg);
					break;
				case IDYES:
					VerifyFingerprint(context, true);
					DestroyWindow(hwndDlg);
					break;
				case IDNO:
					VerifyFingerprint(context, false);
					DestroyWindow(hwndDlg);
					break;
				}
			}
			break;
		
		case CBN_SELCHANGE:
			switch (LOWORD(wParam)) {
			case IDC_CBO_SMP_CHOOSE:
				MCONTACT hContact = (UINT_PTR)context->app_data;
				Fingerprint *fp = context->active_fingerprint;
				if (!fp) {
					DestroyWindow(hwndDlg);
					return TRUE;
				}
				
				BOOL trusted = false;
				if (fp->trust && fp->trust[0] != '\0')
					trusted = true;

				wchar_t buff[512];
				GetDlgItemText(hwndDlg, IDC_CBO_SMP_CHOOSE, buff, 255);
				if (wcsncmp(buff, TranslateW(LANG_SMPTYPE_QUESTION), 255) == 0) {
					if (trusted)
						mir_snwprintf(buff, TranslateW(LANG_OTR_SMPQUESTION_VERIFIED_DESC), contact_get_nameT(hContact));
					else
						mir_snwprintf(buff, TranslateW(LANG_OTR_SMPQUESTION_VERIFY_DESC), contact_get_nameT(hContact));

					SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

					SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, L"");
					SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, FALSE, 0);
					SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateW(LANG_SMP_QUESTION));

					SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, L"");
					SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, FALSE, 0);
					SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateW(LANG_SMP_ANSWER));


					ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
					ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);
				}
				else if (wcsncmp(buff, TranslateW(LANG_SMPTYPE_PASSWORD), 255) == 0) {
					if (trusted)
						mir_snwprintf(buff, TranslateW(LANG_OTR_SMPPASSWORD_VERIFIED_DESC), contact_get_nameT(hContact));
					else
						mir_snwprintf(buff, TranslateW(LANG_OTR_SMPPASSWORD_VERIFY_DESC), contact_get_nameT(hContact));

					SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

					SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, L"");
					SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, TRUE, 0);
					SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, L"");

					SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, L"");
					SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, FALSE, 0);
					SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateW(LANG_SMP_PASSWORD));


					ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_SHOWNA);
					ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_HIDE);
				}
				else if (wcsncmp(buff, TranslateW(LANG_SMPTYPE_FINGERPRINT), 255) == 0) {
					if (trusted)
						mir_snwprintf(buff, TranslateW(LANG_OTR_FPVERIFIED_DESC), contact_get_nameT(hContact));
					else
						mir_snwprintf(buff, TranslateW(LANG_OTR_FPVERIFY_DESC), contact_get_nameT(hContact));

					SetDlgItemText(hwndDlg, IDC_STC_SMP_INFO, buff);

					unsigned char hash[20];
					uint8_t *ret;
					{
						mir_cslock lck(lib_cs);
						ret = otrl_privkey_fingerprint_raw(otr_user_state, hash, context->accountname, context->protocol);
					}
					if (!ret) {
						DestroyWindow(hwndDlg);
						return FALSE;
					}

					otrl_privkey_hash_to_humanT(buff, hash);
					SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD1, buff);
					SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD1, EM_SETREADONLY, TRUE, 0);
					SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD1, TranslateW(LANG_YOUR_PRIVKEY));

					otrl_privkey_hash_to_humanT(buff, fp->fingerprint);
					SetDlgItemText(hwndDlg, IDC_EDT_SMP_FIELD2, buff);
					SendDlgItemMessage(hwndDlg, IDC_EDT_SMP_FIELD2, EM_SETREADONLY, TRUE, 0);
					SetDlgItemText(hwndDlg, IDC_STC_SMP_FIELD2, TranslateW(LANG_CONTACT_FINGERPRINT));

					ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDYES), SW_SHOWNA);
					ShowWindow(GetDlgItem(hwndDlg, IDNO), SW_SHOWNA);
				}
				break;
			}
		}
		break;
	}

	return FALSE;
}

void SMPInitDialog(ConnContext *context)
{
	if (context)
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SMP_INPUT), nullptr, DlgProcSMPInitProc, (LPARAM)context);
}

void SMPDialogUpdate(ConnContext *context, int percent)
{
	if (!context) return;
	SmpForContactMap::iterator it = smp_for_contact.find(context->app_data);
	if (it == smp_for_contact.end())	return;
	if (it->second.dialog) PostMessage(it->second.dialog, WMU_REFRESHSMP, percent, 0);
	TrustLevel level = otr_context_get_trust(context);
	if (!it->second.responder && it->second.oldlevel != level) {
		if (level == TRUST_PRIVATE)
			VerifyFingerprintMessage(context, true);
		else if (level == TRUST_UNVERIFIED)
			VerifyFingerprintMessage(context, false);
	}
}

void SMPDialogReply(ConnContext *context, const char* question)
{
	SmpData *data = (SmpData*)mir_calloc(sizeof(SmpData));
	data->context = context;
	data->oldlevel = TRUST_NOT_PRIVATE;
	data->responder = true;
	data->question = (question) ? mir_utf8decodeW(question) : nullptr;
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SMP_INPUT), nullptr, DlgSMPResponseProc, (LPARAM)data);
}
