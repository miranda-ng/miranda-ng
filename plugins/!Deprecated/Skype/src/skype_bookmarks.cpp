#include "skype.h"
#include "skype_chat.h"

int CSkypeProto::SetBookmarkCommand(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	if (this->IsOnline() && this->IsChatRoom(hContact))
		this->BookmarkChatRoom(hContact);

	return 0;
}

static WNDPROC oldWndProc = NULL;

static LRESULT CALLBACK BookmarkListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
			if (ListView_GetItem(hwnd, &lvi) && lvi.iGroupId == 1)
			{
				CConversation *convo = (CConversation *)lvi.lParam;

				if (convo->SetBookmark(false))
					ListView_DeleteItem(hwnd, lvi.iItem);
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

INT_PTR CALLBACK CSkypeProto::SkypeBookmarksProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
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

			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)::Skin_GetIcon("Skype_bookmark", ICON_BIG));
			::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)::Skin_GetIcon("Skype_bookmark"));

			HWND hwndList = ::GetDlgItem(hwndDlg, IDC_BM_LIST);
			{   // IDC_BM_LIST setup
				oldWndProc = (WNDPROC)::SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)BookmarkListWndProc);

				HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 4, 0);
				ImageList_AddIconFromIconLib(hIml, "Skype_bookmark");
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

				lvg.pszHeader = ::TranslateT("Conferences");
				lvg.iGroupId = 1;
				ListView_InsertGroup(hwndList, 0, &lvg);

				/*lvg.pszHeader = ::TranslateT("Contacts");
				lvg.iGroupId = 2;
				ListView_InsertGroup(hwndList, 0, &lvg);*/

				ListView_EnableGroupView(hwndList, TRUE);

				::SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

				if ( !ppro->IsOnline())
					::EnableWindow(hwndList, FALSE);
			}

			SEString data;
			ConversationRefs conversations;
			ppro->GetConversationList(conversations, Conversation::BOOKMARKED_CONVERSATIONS);
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
				lvi.iGroupId = 1;
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
			};

			::Utils_RestoreWindowPosition(hwndDlg, 0, MODULE, "BookmarksWindow");
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
				lvi.mask = LVIF_PARAM;
				lvi.stateMask = -1;
				lvi.iItem = iItem;
				if (ListView_GetItem(hwndList, &lvi))
				{
					//ppro->FindChatRoom

					CConversation *conversation = (CConversation *)lvi.lParam;

					SEString data;
					conversation->GetPropIdentity(data);
					ptrW cid = ::mir_utf8decodeW(data);
					CSkypeProto::ReplaceSpecialChars(cid);

					ChatRoom *room = ppro->FindChatRoom(cid);
					if (room != NULL)
						::CallProtoService(ppro->m_szModuleName, PS_JOINCHAT, (WPARAM)room->GetContactHandle(), 0);
					else
					{
						conversation->GetPropDisplayname(data);
						ptrW name = ::mir_utf8decodeW(data);

						ChatRoom *room = new ChatRoom(cid, name, ppro);
						room->Start(conversation->ref(), true);
					}
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
				::Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "BookmarksWindow");
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