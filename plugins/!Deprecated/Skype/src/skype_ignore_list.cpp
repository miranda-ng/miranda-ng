#include "skype.h"
#include "skype_chat.h"

int CSkypeProto::IgnoreCommand(WPARAM wParam, LPARAM)
{
	CContact::Ref contact;
	HANDLE hContact = (HANDLE)wParam;
	SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
	if (this->GetContact(sid, contact))
	{
		//bool state = ::CallService(MS_IGNORE_ISIGNORED, wParam, IGNOREEVENT_ALL);
		bool state = ::db_get_b(hContact, this->m_szModuleName, "Ignore", 0) > 0;
		if (contact->SetBlocked(!state))
		{
			//::CallService(!state ? MS_IGNORE_IGNORE : MS_IGNORE_UNIGNORE, wParam, IGNOREEVENT_ALL);
			::db_set_b(hContact, this->m_szModuleName, "Ignore", (int)!state);
		}
	}

	return 0;
}

int CSkypeProto::BlockCommand(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact)
	{
		this->IgnoreCommand(wParam, lParam);

		::db_set_b(hContact, "CList", "Hidden", 1);
	}

	return 0;
}

INT_PTR CSkypeProto::OpenIgnoreListCommand(WPARAM, LPARAM)
{
	::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_IGNORE_LIST), NULL, CSkypeProto::IgnoreListWndProc, (LPARAM)this);

	return 0;
}

static WNDPROC oldWndProc = NULL;

static LRESULT CALLBACK IgnoreListSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN)
	{
		LVHITTESTINFO hi;
		hi.pt.x = LOWORD(lParam); hi.pt.y = HIWORD(lParam);
		ListView_SubItemHitTest(hwnd, &hi);
		if (hi.iSubItem == 1)
		{
			LVITEM lvi = {0};
			lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_GROUPID;
			lvi.stateMask = -1;
			lvi.iItem = hi.iItem;
			if (ListView_GetItem(hwnd, &lvi))
			{
				if (lvi.iGroupId == 1)
				{
					CContact *contact = (CContact *)lvi.lParam;

					if (contact->SetBlocked(false))
						ListView_DeleteItem(hwnd, lvi.iItem);
				}
				/*else if (lvi.iGroupId == 2)
				{
					CConversation *convo = (CConversation *)lvi.lParam;

					if (convo->SetBlocked(false))
						ListView_DeleteItem(hwnd, lvi.iItem);
				}*/
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

INT_PTR CALLBACK CSkypeProto::IgnoreListWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		if (lParam)
		{
			ppro = (CSkypeProto *)lParam;
			::TranslateDialogDefault(hwndDlg);

			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)::Skin_GetIcon("Skype_block", ICON_BIG));
			::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)::Skin_GetIcon("Skype_block"));

			HWND hwndList = ::GetDlgItem(hwndDlg, IDC_BM_LIST);
			{   // IDC_BM_LIST setup
				oldWndProc = (WNDPROC)::SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)IgnoreListSubProc);

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
				LVGROUP lvg;
				lvg.cbSize = sizeof(LVGROUP);
				lvg.mask = LVGF_HEADER | LVGF_GROUPID;

				lvg.pszHeader = ::TranslateT("Contacts");
				lvg.iGroupId = 1;
				ListView_InsertGroup(hwndList, 0, &lvg);

				lvg.pszHeader = ::TranslateT("Conferences");
				lvg.iGroupId = 2;
				ListView_InsertGroup(hwndList, 0, &lvg);

				ListView_EnableGroupView(hwndList, TRUE);

				::SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

				if ( !ppro->IsOnline())
					::EnableWindow(hwndList, FALSE);
			}

			SEString data;
			ContactGroupRef blockedList;
			ppro->GetHardwiredContactGroup(ContactGroup::CONTACTS_BLOCKED_BY_ME, blockedList);
			
			ContactRefs contacts;
			blockedList->GetContacts(contacts);
			for (size_t i = 0; i < contacts.size(); i++)
			{
				auto contact = contacts[i];

				contact->GetPropDisplayname(data);
				ptrW name = ::mir_utf8decodeW(data);

				LVITEM lvi = {0};
				lvi.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
				lvi.iItem = (int)i;
				lvi.iGroupId = 1;
				lvi.iImage = 0;
				lvi.lParam = (LPARAM)contact.fetch();
				lvi.pszText = name;
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

			ConversationRefs conversations;
			blockedList->GetConversations(conversations);
			for (size_t i = 0; i < conversations.size(); i++)
			{
				auto conversation = conversations[i];

				uint type = conversation->GetUintProp(Conversation::P_TYPE);
				if (type != Conversation::CONFERENCE)
					continue;

				conversation->GetPropDisplayname(data);
				ptrW name = ::mir_utf8decodeW(data);

				LVITEM lvi = {0};
				lvi.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
				lvi.iItem = (int)i;
				lvi.iGroupId = 2;
				lvi.iImage = 0;
				lvi.lParam = (LPARAM)conversation.fetch();
				lvi.pszText = name;
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

			::Utils_RestoreWindowPosition(hwndDlg, 0, MODULE, "IgnoreListWindow");
		}
		break;

	case WM_NOTIFY:
		switch(LOWORD(wParam))
        {
        case IDC_BM_LIST:
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
						CContact *contact = (CContact *)lvi.lParam;

						contact->GetIdentity(data);
						ptrW sid(::mir_utf8decodeW(data));
						::CallService(MS_MSG_SENDMESSAGE, (WPARAM)ppro->GetContactBySid(sid), 0);
					}
					/*else if (lvi.iGroupId == 2)
					{
						CConversation *conversation = (CConversation *)lvi.lParam;

						conversation->GetPropIdentity(data);
						ptrW cid(::mir_utf8decodeW(data));
						CSkypeProto::ReplaceSpecialChars(cid);
					}*/
				}
			}
		}
		break;
		
	case WM_COMMAND:
		if (HIWORD( wParam ) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case IDCANCEL:
				::Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "IgnoreListWindow");
				::Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
				::Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
				::DestroyWindow(hwndDlg);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}