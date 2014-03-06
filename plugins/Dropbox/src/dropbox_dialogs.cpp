#include "common.h"

INT_PTR CALLBACK CDropbox::TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *token = reinterpret_cast<char*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		token = (char*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			//::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)CSkypeProto::IconList[0].Handle);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL,	(LPARAM)CSkypeProto::IconList[0].Handle);

			/*wchar_t title[MAX_PATH];
			::mir_sntprintf(
			title,
			MAX_PATH,
			::TranslateT("Enter a password for %s:"),
			param->login);*/
			//::SetDlgItemText(hwndDlg, IDC_INSTRUCTION, title);

			SendDlgItemMessage(hwndDlg, IDC_TOKEN, EM_LIMITTEXT, 128 - 1, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			char data[128];
			GetDlgItemTextA(hwndDlg, IDC_TOKEN, data, SIZEOF(data));
			strcpy(token, data);

			EndDialog(hwndDlg, IDOK);
		}
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
	}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CDropbox::MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CheckDlgButton(hwndDlg, IDC_USE_SHORT_LINKS, db_get_b(NULL, MODULE, "UseSortLinks", 1));
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDC_GETAUTH:
				CallService(MS_UTILS_OPENURL, 0, (LPARAM)DROPBOX_WWW_URL DROPBOX_API_VER "/oauth2/authorize?response_type=code&client_id=" DROPBOX_API_KEY);
				break;

			case IDC_USE_SHORT_LINKS:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_AUTHORIZE:
				{
					char request_token[128];
					GetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, request_token, SIZEOF(request_token));

					char data[1024];
					mir_snprintf(
						data,
						SIZEOF(data),
						"grant_type=authorization_code&code=%s",
						request_token);

					HttpRequest *request = new HttpRequest(INSTANCE->hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/oauth2/token");
					request->pData = mir_strdup(data);
					request->dataLength = (int)strlen(data);
					request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
					request->AddBasicAuthHeader(DROPBOX_API_KEY, DROPBOX_API_SECRET);

					mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

					delete request;

					MCONTACT hContact = CDropbox::GetDefaultContact();

					if (response)
					{
						JSONNODE *root = json_parse(response->pData);
						if (root)
						{
							if (response->resultCode == HTTP_STATUS::OK)
							{
								JSONNODE *node = json_get(root, "access_token");
								ptrA access_token = ptrA(mir_u2a(json_as_string(node)));
								db_set_s(NULL, MODULE, "TokenSecret", access_token);

								if (hContact)
								{
									if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
										db_set_w(hContact, MODULE, "Status", ID_STATUS_ONLINE);
								}

								ShowNotification(TranslateT("You have been authorized"), MB_ICONINFORMATION);
							}
							else
							{
								JSONNODE *node = json_get(root, "error_description");
								ptrW error_description(json_as_string(node));

								ShowNotification((wchar_t*)error_description, MB_ICONERROR);
							}
						}
					}
					else
						HandleFileTransferError(response, hContact);

				}
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY)
		{
			db_set_b(NULL, MODULE, "UseSortLinks", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USE_SHORT_LINKS));
		}
		break;
	}
	return FALSE;
}