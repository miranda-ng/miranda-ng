#include "Mra.h"
#include "MraSelectEMail.h"

struct MraSelectEMailDlgData
{
	CMraProto *ppro;
	MCONTACT hContact;
	DWORD dwType;
};

void AddContactEMailToListParam(MCONTACT hContact, BOOL bMRAOnly, LPSTR lpszModule, LPSTR lpszValueName, HWND hWndList)
{
	CMStringA szEmail;
	if (DB_GetStringA(hContact, lpszModule, lpszValueName, szEmail)) {
		if (bMRAOnly == FALSE || IsEMailMR(szEmail)) {
			WCHAR wszBuff[MAX_EMAIL_LEN];
			MultiByteToWideChar(MRA_CODE_PAGE, 0, szEmail, -1, wszBuff, SIZEOF(wszBuff));
			if (SendMessage(hWndList, LB_FINDSTRING, -1, (LPARAM)wszBuff) == LB_ERR)
				SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)wszBuff);
		}
	}

	for (int i = 0; true; i++) {
		char szBuff[MAX_PATH];
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s%lu", lpszValueName, i);
		if (DB_GetStringA(hContact, lpszModule, szBuff, szEmail)) {
			if (bMRAOnly == FALSE || IsEMailMR(szEmail)) {
				WCHAR wszBuff[MAX_EMAIL_LEN];
				MultiByteToWideChar(MRA_CODE_PAGE, 0, szEmail, -1, wszBuff, SIZEOF(wszBuff));
				if (SendMessage(hWndList, LB_FINDSTRING, -1, (LPARAM)wszBuff) == LB_ERR)
					SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)wszBuff);
			}
		}
		else if (i > EMAILS_MIN_COUNT)
			break;
	}
}

INT_PTR CALLBACK MraSelectEMailDlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	MraSelectEMailDlgData *dat = (MraSelectEMailDlgData*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		dat = (MraSelectEMailDlgData*)lParam;
		{
			BOOL bMRAOnly;

			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			HWND hWndList = GetDlgItem(hWndDlg, IDC_LIST_EMAILS);

			switch (dat->dwType) {
			case MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD:
				bMRAOnly = FALSE;
				break;
			case MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM:
			case MRA_SELECT_EMAIL_TYPE_READ_BLOG:
				bMRAOnly = TRUE;
				break;
			default:
				bMRAOnly = FALSE;
				break;
			}

			LPSTR lpszProto = dat->hContact ? GetContactProto(dat->hContact) : dat->ppro->m_szModuleName;

			AddContactEMailToListParam(dat->hContact, bMRAOnly, lpszProto, "e-mail", hWndList);
			AddContactEMailToListParam(dat->hContact, bMRAOnly, "UserInfo", "e-mail", hWndList);
			AddContactEMailToListParam(dat->hContact, bMRAOnly, "UserInfo", "Mye-mail", hWndList);
			AddContactEMailToListParam(dat->hContact, bMRAOnly, "UserInfo", "Companye-mail", hWndList);
			AddContactEMailToListParam(dat->hContact, bMRAOnly, "UserInfo", "MyCompanye-mail", hWndList);
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hWndDlg);
		break;

	case WM_DESTROY:
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, 0);
		mir_free(dat);
		EndDialog(hWndDlg, NO_ERROR);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LIST_EMAILS:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM)GetDlgItem(hWndDlg, IDOK));
			break;

		case IDOK:
			{
				CMStringA szEmail;
				WCHAR wszBuff[MAX_PATH];

				SendDlgItemMessage(hWndDlg, IDC_LIST_EMAILS, LB_GETTEXT, SendDlgItemMessage(hWndDlg, IDC_LIST_EMAILS, LB_GETCURSEL, 0, 0), (LPARAM)wszBuff);
				szEmail = wszBuff;
				szEmail.MakeLower();

				switch (dat->dwType) {
				case MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD:
					{
						CMStringA szUrl;
						szUrl.Format("http://cards.mail.ru/event.html?rcptname=%s&rcptemail=%s", GetContactNameA(dat->hContact), szEmail);
						dat->ppro->MraMPopSessionQueueAddUrl(dat->ppro->hMPopSessionQueue, szUrl);
					}
					break;
				case MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_FOTO_URL, szEmail);
					break;
				case MRA_SELECT_EMAIL_TYPE_READ_BLOG:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_BLOGS_URL, szEmail);
					break;
				case MRA_SELECT_EMAIL_TYPE_VIEW_VIDEO:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_VIDEO_URL, szEmail);
					break;
				case MRA_SELECT_EMAIL_TYPE_ANSWERS:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_ANSWERS_URL, szEmail);
					break;
				case MRA_SELECT_EMAIL_TYPE_WORLD:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_WORLD_URL, szEmail);
					break;
				}
			}
			//break;
		case IDCANCEL:
			DestroyWindow(hWndDlg);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

DWORD CMraProto::MraSelectEMailDlgShow(MCONTACT hContact, DWORD dwType)
{
	MraSelectEMailDlgData *dat = (MraSelectEMailDlgData*)mir_calloc(sizeof(MraSelectEMailDlgData));
	if (dat) {
		dat->ppro = this;
		dat->hContact = hContact;
		dat->dwType = dwType;
		DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_SELECT_EMAIL), NULL, MraSelectEMailDlgProc, (LPARAM)dat);
	}
	return 0;
}
