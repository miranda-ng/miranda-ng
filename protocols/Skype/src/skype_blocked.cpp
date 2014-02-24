#include "skype.h"
#include "skype_chat.h"

int CSkypeProto::BlockCommand(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact)
	{
		SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
			
		ContactRef contact;
		if ( !this->GetContact(sid, contact) || !contact)
			return 0;

		bool isBlocked = false;
		if (contact->IsMemberOfHardwiredGroup(ContactGroup::CONTACTS_BLOCKED_BY_ME, isBlocked) && !isBlocked)
		{
			BlockParam param(hContact, this);
			if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_BLOCK), NULL, CSkypeProto::SkypeBlockProc, (LPARAM)&param) == IDOK)
			{
				if ( !contact->SetBlocked(true, param.abuse))
					return 0;

				if (param.remove)
				{
					contact->SetBuddyStatus(false);
					this->contactList.remove_val(contact);
					::CallService(MS_DB_CONTACT_DELETE, wParam, 0);
				}
				else if (this->getByte(hContact, "IsSkypeOut", 0) > 0)
					this->setWord(hContact, "Status", ID_STATUS_OFFLINE);
			}
		}
		else
		{
			if (contact->SetBlocked(false))
				if (this->getByte(hContact, "IsSkypeOut", 0) > 0)
					this->setWord(hContact, "Status", ID_STATUS_ONTHEPHONE);
		}
	}

	return 0;
}

INT_PTR CSkypeProto::OpenBlockedListCommand(WPARAM, LPARAM)
{
	char *title = ::mir_t2a(this->m_tszUserName);
	OPENOPTIONSDIALOG ood;
	ood.cbSize = sizeof(OPENOPTIONSDIALOG);
	ood.pszGroup = "Network";
	ood.pszPage = title;
	ood.pszTab = "Blocked contacts";
	return ::Options_Open(&ood);
	::mir_free(title);
}

INT_PTR CALLBACK CSkypeProto::SkypeBlockProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BlockParam *param = (BlockParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		if (lParam)
		{
			param = (BlockParam *)lParam;
			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			::TranslateDialogDefault(hwndDlg);			

			wchar_t *nick = (wchar_t *)::CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)param->hContact, GCDNF_TCHAR);

			TCHAR oldTitle[256], newTitle[256];
			::GetDlgItemText(hwndDlg, IDC_HEADERBAR, oldTitle, SIZEOF(oldTitle));
			::mir_sntprintf(newTitle, SIZEOF(newTitle), ::TranslateTS(oldTitle), nick);
			::SetDlgItemText(hwndDlg, IDC_HEADERBAR, newTitle);

			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)::Skin_GetIcon("Skype_block", ICON_BIG));
			::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL,	(LPARAM)::Skin_GetIcon("Skype_block"));
			
			wchar_t text[1024];
			::mir_sntprintf(
				text, 
				SIZEOF(text), 
				::TranslateT("Are you sure you want to block \"%s\" (%s)? They won't be able to contact you and won't appear in your Contact List."),
				nick,
				ptrW(::db_get_wsa(param->hContact, param->ppro->m_szModuleName, SKYPE_SETTINGS_SID)));
			::SetDlgItemText(hwndDlg, IDC_MESSAGE, text);

			::Utils_RestoreWindowPosition(hwndDlg, 0, MODULE, "BlockWindow");
		}
		break;

	case WM_COMMAND:
		if (HIWORD( wParam ) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				param->remove = ::IsDlgButtonChecked(hwndDlg, IDC_REMOVE_FROM_CL) > 0;
				param->abuse = ::IsDlgButtonChecked(hwndDlg, IDC_REPORT_ABUSE) > 0;

				::Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "BlockWindow");
				::Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
				::Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
				::EndDialog(hwndDlg, IDOK);
				break;

			case IDCANCEL:
				::Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "BlockWindow");
				::Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
				::Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
				::EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}
	return FALSE;
}

static WNDPROC oldWndProc = NULL;

LRESULT CALLBACK CSkypeProto::SkypeBlockedOptionsSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN)
	{
		LVHITTESTINFO hi;
		hi.pt.x = LOWORD(lParam); hi.pt.y = HIWORD(lParam);
		ListView_SubItemHitTest(hwnd, &hi);
		if (hi.iSubItem == 1)
		{
			LVITEM lvi = {0};
			lvi.mask = LVIF_IMAGE | LVIF_PARAM;
			lvi.stateMask = -1;
			lvi.iItem = hi.iItem;
			if (ListView_GetItem(hwnd, &lvi))
			{
				ContactParam *param = (ContactParam *)lvi.lParam;
					
				if (param->contact->SetBlocked(false))
				{
					SEString data;
					param->contact->GetIdentity(data);
					ptrW sid(::mir_utf8decodeW(data));
					
					MCONTACT hContact = param->ppro->GetContactBySid(sid);
					if (::db_get_b(hContact, param->ppro->m_szModuleName, "IsSkypeOut", 0) > 0)
						::db_set_w(hContact, param->ppro->m_szModuleName, "Status", ID_STATUS_ONTHEPHONE);

					ListView_DeleteItem(hwnd, lvi.iItem);
					
					int nItem = ::SendMessage(::GetDlgItem(GetParent(hwnd), IDC_CONTACTS), CB_ADDSTRING, 0, (LPARAM)sid);
					::SendMessage(::GetDlgItem(GetParent(hwnd), IDC_CONTACTS), CB_SETITEMDATA, nItem, hContact);
				}
			}
		}
	}

	return ::CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);
}

int ImageList_AddIconFromIconLib(HIMAGELIST hIml, const char *name)
{
	HICON icon = ::Skin_GetIconByHandle(::Skin_GetIconHandle(name));
	int res = ImageList_AddIcon(hIml, icon);
	::Skin_ReleaseIcon(icon);
	return res;
}

INT_PTR CALLBACK CSkypeProto::SkypeBlockedOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		if (lParam)
		{
			ppro = (CSkypeProto *)lParam;
			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			::TranslateDialogDefault(hwndDlg);			

			HWND hwndList = ::GetDlgItem(hwndDlg, IDC_LIST);
			{   // IDC_BM_LIST setup
				oldWndProc = (WNDPROC)::SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)SkypeBlockedOptionsSubProc);

				HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 4, 0);
				ImageList_AddIconFromIconLib(hIml, "Skype_contact");
				ImageList_AddIconFromIconLib(hIml, "Skype_delete");
				ListView_SetImageList(hwndList, hIml, LVSIL_SMALL);
				///
				LVCOLUMN lvc = {0};
				lvc.mask = LVCF_WIDTH | LVCF_TEXT;
				//lvc.fmt = LVCFMT_JUSTIFYMASK;
				lvc.pszText = TranslateT("Name");
				lvc.cx = 220; // width of column in pixels
				ListView_InsertColumn(hwndList, 0, &lvc);
				//lvc.fmt = LVCFMT_RIGHT;
				lvc.pszText = L"";
				lvc.cx = 32 - GetSystemMetrics(SM_CXVSCROLL); // width of column in pixels
				ListView_InsertColumn(hwndList, 1, &lvc);
				///
				::SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

				if ( !ppro->IsOnline())
				{
					::EnableWindow(hwndList, FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CONTACTS), FALSE);
				}
			}

			if (ppro->IsOnline())
			{
				SEString data;
				ContactGroupRef blockedList;
				ppro->GetHardwiredContactGroup(ContactGroup::CONTACTS_BLOCKED_BY_ME, blockedList);
			
				CContact::Refs contacts;
				blockedList->GetContacts(contacts);
				for (size_t i = 0; i < contacts.size(); i++)
				{
					auto contact = contacts[i];

					ptrW sid(::mir_utf8decodeW(contact->GetSid()));

					LVITEM lvi = {0};
					lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
					lvi.iItem = (int)i;
					lvi.iImage = 0;
					lvi.lParam = (LPARAM)new ContactParam(contact, ppro);
					lvi.pszText = sid;
					int iRow = ListView_InsertItem(hwndList, &lvi);

					if (iRow != -1)
					{
						lvi.iItem = iRow;
						lvi.mask = LVIF_IMAGE;
						lvi.iSubItem = 1;
						lvi.iImage = 1;
						ListView_SetItem(hwndList, &lvi);
					}
				}
				///
				int nItem = 0;
				MCONTACT hContact = NULL;

				::EnterCriticalSection(&ppro->contact_search_lock);

				for (hContact = ::db_find_first(ppro->m_szModuleName); hContact && !ppro->isChatRoom(hContact); hContact = ::db_find_next(hContact, ppro->m_szModuleName))
				{
					ptrW sid(::db_get_wsa(hContact, ppro->m_szModuleName, SKYPE_SETTINGS_SID));

					ContactRef contact;
					ppro->GetContact((char *)_T2A(sid), contact);
					if ( !contacts.contains(contact))
					{
						nItem = ::SendMessage(::GetDlgItem(hwndDlg, IDC_CONTACTS), CB_ADDSTRING, 0, (LPARAM)sid);
						::SendMessage(::GetDlgItem(hwndDlg, IDC_CONTACTS), CB_SETITEMDATA, nItem, hContact);
					}
				}

				::LeaveCriticalSection(&ppro->contact_search_lock);
			}

		}
		break;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_CONTACTS:
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BLOCK), TRUE);
				break;

			case IDC_BLOCK:
				{
					int i = ::SendMessage(::GetDlgItem(hwndDlg, IDC_CONTACTS), CB_GETCURSEL, 0, 0);

					MCONTACT hContact = (MCONTACT)::SendMessage(GetDlgItem(hwndDlg, IDC_CONTACTS), CB_GETITEMDATA, i, 0);
					if ( !hContact)
						break;

					ptrW sid(::db_get_wsa(hContact, ppro->m_szModuleName, SKYPE_SETTINGS_SID));

					SEString data;
					ContactRef contact;
					if (!ppro->GetContact((char *)_T2A(sid), contact) || !contact)
						break;

					BlockParam param(hContact, ppro);
					if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_BLOCK), NULL, CSkypeProto::SkypeBlockProc, (LPARAM)&param) != IDOK)
						break;

					contact->SetBlocked(true, param.abuse);
					if (::db_get_b(hContact, ppro->m_szModuleName, "IsSkypeOut", 0) > 0)
						::db_set_w(hContact, ppro->m_szModuleName, "Status", ID_STATUS_OFFLINE);

					if (param.remove)
					{
						contact->SetBuddyStatus(false);
						ppro->contactList.remove_val(contact);
						::CallService(MS_DB_CONTACT_DELETE, wParam, 0);
					}

					if (contact->SetBlocked(true))
					{
						LVITEM lvi = {0};
						lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
						lvi.iItem = (int)i;
						lvi.iImage = 0;
						lvi.lParam = (LPARAM)new ContactParam(contact, ppro);
						lvi.pszText = sid;
						int iRow = ListView_InsertItem(::GetDlgItem(hwndDlg, IDC_LIST), &lvi);

						if (iRow != -1)
						{
							lvi.iItem = iRow;
							lvi.mask = LVIF_IMAGE;
							lvi.iSubItem = 1;
							lvi.iImage = 1;
							ListView_SetItem(::GetDlgItem(hwndDlg, IDC_LIST), &lvi);
						}
						::SendMessage(::GetDlgItem(hwndDlg, IDC_CONTACTS), CB_DELETESTRING, i, 0);
					}
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY && !ppro->IsOnline())
		{
			return TRUE;
		}
		break;

		switch(LOWORD(wParam))
        {
        case IDC_LIST:
            if (((LPNMHDR)lParam)->code == NM_DBLCLK)
            {
				HWND hwndList = ::GetDlgItem(hwndDlg, IDC_BM_LIST);
				int iItem = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (iItem < 0) break;
				LVITEM lvi = {0};
				lvi.mask = LVIF_PARAM | LVIF_GROUPID;
				lvi.stateMask = -1;
				lvi.iItem = iItem;
				if (ListView_GetItem(hwndList, &lvi))
				{
					SEString data;
					if (lvi.iGroupId == 1)
					{
						ContactParam *param = (ContactParam *)lvi.lParam;

						param->contact->GetIdentity(data);
						ptrW sid(::mir_utf8decodeW(data));
						::CallService(MS_MSG_SENDMESSAGE, (WPARAM)ppro->GetContactBySid(sid), 0);
					}
				}
			}
		}
		break;
	}
	return FALSE;
}