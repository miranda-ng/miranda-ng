#include "Mra.h"
#include "MraSelectEMail.h"

struct MraSelectEMailDlgData
{
	CMraProto *ppro;
	HANDLE hContact;
	DWORD dwType;
};

void AddContactEMailToListParam(HANDLE hContact, BOOL bMRAOnly, LPSTR lpszModule, LPSTR lpszValueName, HWND hWndList)
{
	CHAR szBuff[MAX_PATH], szEMail[MAX_EMAIL_LEN];
	WCHAR wszBuff[MAX_PATH];
	size_t i, dwEMailSize;

	if (DB_GetStaticStringA(hContact, lpszModule, lpszValueName, szEMail, SIZEOF(szEMail), &dwEMailSize)) {
		if (bMRAOnly == FALSE || IsEMailMR(szEMail, dwEMailSize)) {
			MultiByteToWideChar(MRA_CODE_PAGE, 0, szEMail, (dwEMailSize+1), wszBuff, SIZEOF(wszBuff));
			if (SendMessage(hWndList, LB_FINDSTRING, -1, (LPARAM)wszBuff) == LB_ERR) SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)wszBuff);
		}
	}

	for (i = 0;TRUE;i++) {
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s%lu", lpszValueName, i);
		if (DB_GetStaticStringA(hContact, lpszModule, szBuff, szEMail, SIZEOF(szEMail), &dwEMailSize)) {
			if (bMRAOnly == FALSE || IsEMailMR(szEMail, dwEMailSize)) {
				MultiByteToWideChar(MRA_CODE_PAGE, 0, szEMail, (dwEMailSize+1), wszBuff, SIZEOF(wszBuff));
				if (SendMessage(hWndList, LB_FINDSTRING, -1, (LPARAM)wszBuff) == LB_ERR) SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)wszBuff);
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
			HWND hWndList;
			BOOL bMRAOnly;
			LPSTR lpszProto;

			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			hWndList = GetDlgItem(hWndDlg, IDC_LIST_EMAILS);

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

			if (dat->hContact)
				lpszProto = GetContactProto(dat->hContact);
			else
				lpszProto = dat->ppro->m_szModuleName;

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
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)0);
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
				CHAR szEMail[MAX_EMAIL_LEN];
				WCHAR wszBuff[MAX_PATH];
				size_t dwEMailSize;

				dwEMailSize = SendMessage(GetDlgItem(hWndDlg, IDC_LIST_EMAILS), LB_GETTEXT, SendMessage(GetDlgItem(hWndDlg, IDC_LIST_EMAILS), LB_GETCURSEL, 0, 0), (LPARAM)wszBuff);
				WideCharToMultiByte(MRA_CODE_PAGE, 0, wszBuff, (dwEMailSize+1), szEMail, SIZEOF(szEMail), NULL, NULL);
				BuffToLowerCase(szEMail, szEMail, dwEMailSize);

				switch (dat->dwType) {
				case MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD:
					{
						size_t dwUrlSize;
						CHAR szUrl[BUFF_SIZE_URL];
						dwUrlSize = mir_snprintf(szUrl, SIZEOF(szUrl), "http://cards.mail.ru/event.html?rcptname=%s&rcptemail=%s", GetContactNameA(dat->hContact), szEMail);
						dat->ppro->MraMPopSessionQueueAddUrl(dat->ppro->hMPopSessionQueue, szUrl, dwUrlSize);
					}
					break;
				case MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_FOTO_URL, sizeof(MRA_FOTO_URL), szEMail, dwEMailSize);
					break;
				case MRA_SELECT_EMAIL_TYPE_READ_BLOG:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_BLOGS_URL, sizeof(MRA_BLOGS_URL), szEMail, dwEMailSize);
					break;
				case MRA_SELECT_EMAIL_TYPE_VIEW_VIDEO:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_VIDEO_URL, sizeof(MRA_VIDEO_URL), szEMail, dwEMailSize);
					break;
				case MRA_SELECT_EMAIL_TYPE_ANSWERS:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_ANSWERS_URL, sizeof(MRA_ANSWERS_URL), szEMail, dwEMailSize);
					break;
				case MRA_SELECT_EMAIL_TYPE_WORLD:
					dat->ppro->MraMPopSessionQueueAddUrlAndEMail(dat->ppro->hMPopSessionQueue, MRA_WORLD_URL, sizeof(MRA_WORLD_URL), szEMail, dwEMailSize);
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

DWORD CMraProto::MraSelectEMailDlgShow(HANDLE hContact, DWORD dwType)
{
	MraSelectEMailDlgData *dat = (MraSelectEMailDlgData*)mir_calloc(sizeof(MraSelectEMailDlgData));
	if (dat) {
		dat->ppro = this;
		dat->hContact = hContact;
		dat->dwType = dwType;
		DialogBoxParam(masMraSettings.hInstance, MAKEINTRESOURCE(IDD_DIALOG_SELECT_EMAIL), NULL, MraSelectEMailDlgProc, (LPARAM)dat);
	}
	return 0;
}
