/*
Copyright (c) 2014-17 Robert Pösel, 2017-25 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////
// Account manager dialog

INT_PTR CALLBACK DummyAccountProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDummyProto *ppro = (CDummyProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CDummyProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );

		Window_SetIcon_IcoLib(hwndDlg, ppro->m_hProtoIcon);
		{
			FillTemplateCombo(hwndDlg, IDC_TEMPLATE);
		
			int templateId = ppro->getTemplateId();
			SendDlgItemMessage(hwndDlg, IDC_TEMPLATE, CB_SETCURSEL, templateId, 0);

			boolean allowSending = ppro->getByte(DUMMY_KEY_ALLOW_SENDING, 0);
			CheckDlgButton(hwndDlg, IDC_ALLOW_SENDING, allowSending ? BST_CHECKED : BST_UNCHECKED);

			ppro->selectTemplate(hwndDlg, templateId);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TEMPLATE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int templateId = SendDlgItemMessage(hwndDlg, IDC_TEMPLATE, CB_GETCURSEL, 0, 0);
				ppro->selectTemplate(hwndDlg, templateId);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		case IDC_ID_TEXT:
		case IDC_ID_SETTING:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		default:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			int templateId = SendDlgItemMessage(hwndDlg, IDC_TEMPLATE, CB_GETCURSEL, 0, 0);
			ppro->setByte(DUMMY_ID_TEMPLATE, templateId);

			// Save custom fields only when this is custom template
			if (templateId == 0) {
				char str[128];
				GetDlgItemTextA(hwndDlg, IDC_ID_TEXT, str, _countof(str));
				ppro->setString(DUMMY_ID_TEXT, str);

				GetDlgItemTextA(hwndDlg, IDC_ID_SETTING, str, _countof(str));
				ppro->setString(DUMMY_ID_SETTING, str);
			}

			ppro->setByte(DUMMY_KEY_ALLOW_SENDING, IsDlgButtonChecked(hwndDlg, IDC_ALLOW_SENDING));
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		break;
	}

	return FALSE;
}

MWindow CDummyProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ACCMGRUI), hwndParent, DummyAccountProc, (LPARAM)this);
}
