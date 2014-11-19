/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"
#include "m_smileyadd.h"

void CMsnProto::Lists_Uninit(void)
{
	Lists_Wipe();
}

void CMsnProto::Lists_Wipe(void)
{
	mir_cslock lck(m_csLists);
	m_arContacts.destroy();
}

bool CMsnProto::Lists_IsInList(int list, const char* email)
{
	mir_cslock lck(m_csLists);

	MsnContact *p = m_arContacts.find((MsnContact*)&email);
	if (p == NULL)
		return false;
	if (list == -1)
		return true;
	return (p->list & list) == list;
}

MsnContact* CMsnProto::Lists_Get(const char* email)
{
	mir_cslock lck(m_csLists);
	return m_arContacts.find((MsnContact*)&email);
}

MsnContact* CMsnProto::Lists_Get(MCONTACT hContact)
{
	mir_cslock lck(m_csLists);

	for (int i = 0; i < m_arContacts.getCount(); ++i)
		if (m_arContacts[i].hContact == hContact)
			return &m_arContacts[i];

	return NULL;
}

MsnPlace* CMsnProto::Lists_GetPlace(const char* wlid)
{
	mir_cslock lck(m_csLists);

	char *szEmail, *szInst;
	parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, &szInst);

	if (szInst == NULL)
		szInst = (char*)sttVoidUid;

	MsnContact* p = m_arContacts.find((MsnContact*)&szEmail);
	if (p == NULL)
		return NULL;

	return p->places.find((MsnPlace*)&szInst);
}

MsnPlace* CMsnProto::Lists_AddPlace(const char* email, const char* id, unsigned cap1, unsigned cap2)
{
	mir_cslock lck(m_csLists);

	MsnContact *p = m_arContacts.find((MsnContact*)&email);
	if (p == NULL)
		return NULL;

	MsnPlace *pl = p->places.find((MsnPlace*)&id);
	if (pl == NULL) {
		pl = new MsnPlace;
		pl->id = mir_strdup(id);
		pl->cap1 = cap1;
		pl->cap2 = cap2;
		pl->p2pMsgId = 0;
		pl->p2pPktNum = 0;
		p->places.insert(pl);
	}

	return pl;
}

MsnContact* CMsnProto::Lists_GetNext(int &i)
{
	mir_cslock lck(m_csLists);

	MsnContact *p = NULL;
	while (p == NULL && ++i < m_arContacts.getCount())
		if (m_arContacts[i].hContact)
			p = &m_arContacts[i];

	return p;
}

int CMsnProto::Lists_GetMask(const char* email)
{
	mir_cslock lck(m_csLists);

	MsnContact *p = m_arContacts.find((MsnContact*)&email);
	return p ? p->list : 0;
}

int CMsnProto::Lists_GetNetId(const char* email)
{
	if (email[0] == 0) return NETID_UNKNOWN;

	mir_cslock lck(m_csLists);

	MsnContact *p = m_arContacts.find((MsnContact*)&email);
	return p ? p->netId : NETID_UNKNOWN;
}

unsigned CMsnProto::p2p_getMsgId(const char* wlid, int inc)
{
	mir_cslock lck(m_csLists);
	MsnPlace *p = Lists_GetPlace(wlid);

	unsigned res = p && p->p2pMsgId ? p->p2pMsgId : MSN_GenRandom();
	if (p)
		p->p2pMsgId = res + inc;

	return res;
}

unsigned CMsnProto::p2p_getPktNum(const char* wlid)
{
	mir_cslock lck(m_csLists);

	MsnPlace *p = Lists_GetPlace(wlid);
	return p ? p->p2pPktNum++ : 0;
}

int CMsnProto::Lists_Add(int list, int netId, const char* email, MCONTACT hContact, const char* nick, const char* invite)
{
	mir_cslock lck(m_csLists);

	MsnContact* p = m_arContacts.find((MsnContact*)&email);
	if (p == NULL) {
		p = new MsnContact;
		p->list = list;
		p->netId = netId;
		p->email = _strlwr(mir_strdup(email));
		p->invite = mir_strdup(invite);
		p->nick = mir_strdup(nick);
		p->hContact = hContact;
		p->p2pMsgId = 0;
		m_arContacts.insert(p);
	}
	else {
		p->list |= list;
		if (invite) replaceStr(p->invite, invite);
		if (hContact) p->hContact = hContact;
		if (list & LIST_FL) p->netId = netId;
		if (p->netId == NETID_UNKNOWN && netId != NETID_UNKNOWN)
			p->netId = netId;
	}
	return p->list;
}

void CMsnProto::Lists_Remove(int list, const char* email)
{
	mir_cslock lck(m_csLists);

	int i = m_arContacts.getIndex((MsnContact*)&email);
	if (i != -1) {
		MsnContact &p = m_arContacts[i];
		p.list &= ~list;
		if (list & LIST_PL) { mir_free(p.invite); p.invite = NULL; }
		if (p.list == 0 && p.hContact == NULL)
			m_arContacts.remove(i);
	}
}


void CMsnProto::Lists_Populate(void)
{
	MCONTACT hContact = db_find_first(m_szModuleName);
	while (hContact) {
		MCONTACT hNext = db_find_next(hContact, m_szModuleName);
		char szEmail[MSN_MAX_EMAIL_LEN] = "";
		if (db_get_static(hContact, m_szModuleName, "wlid", szEmail, sizeof(szEmail)))
			db_get_static(hContact, m_szModuleName, "e-mail", szEmail, sizeof(szEmail));
		if (szEmail[0]) {
			bool localList = getByte(hContact, "LocalList", 0) != 0;
			if (localList)
				Lists_Add(LIST_LL, NETID_MSN, szEmail, hContact);
			else
				Lists_Add(0, NETID_UNKNOWN, szEmail, hContact);
		}
		else CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		hContact = hNext;
	}
}

void CMsnProto::MSN_CleanupLists(void)
{
	for (int i = m_arContacts.getCount(); i--;) {
		MsnContact& p = m_arContacts[i];
		if (p.list & LIST_FL)
			MSN_SetContactDb(p.hContact, p.email);

		if (p.list & LIST_PL) {
			if (p.list & (LIST_AL | LIST_BL))
				MSN_AddUser(NULL, p.email, p.netId, LIST_PL + LIST_REMOVE);
			else
				MSN_AddAuthRequest(p.email, p.nick, p.invite);
		}

		if (p.hContact && !(p.list & (LIST_LL | LIST_FL | LIST_PL)) && p.list != LIST_RL) {
			int count = db_event_count(p.hContact);
			if (count) {
				TCHAR text[256];
				TCHAR *sze = mir_a2t(p.email);
				mir_sntprintf(text, SIZEOF(text), TranslateT("Contact %s has been removed from the server.\nWould you like to keep it as \"Local Only\" contact to preserve history?"), sze);
				mir_free(sze);

				TCHAR title[128];
				mir_sntprintf(title, SIZEOF(title), TranslateT("%s protocol"), m_tszUserName);

				if (MessageBox(NULL, text, title, MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) == IDYES) {
					MSN_AddUser(p.hContact, p.email, 0, LIST_LL);
					setByte(p.hContact, "LocalList", 1);
					continue;
				}
			}

			if (!(p.list & (LIST_LL | LIST_FL))) {
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)p.hContact, 0);
				p.hContact = NULL;
			}
		}

		if (p.list & (LIST_LL | LIST_FL) && p.hContact) {
			TCHAR path[MAX_PATH];
			MSN_GetCustomSmileyFileName(p.hContact, path, SIZEOF(path), "", 0);
			if (path[0]) {
				SMADD_CONT cont;
				cont.cbSize = sizeof(SMADD_CONT);
				cont.hContact = p.hContact;
				cont.type = 0;
				cont.path = path;

				CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, (LPARAM)&cont);
			}
		}
	}
}

void CMsnProto::MSN_CreateContList(void)
{
	bool *used = (bool*)mir_calloc(m_arContacts.getCount()*sizeof(bool));

	char cxml[8192];

	size_t sz = mir_snprintf(cxml, sizeof(cxml), "<ml l=\"1\">");
	{
		mir_cslock lck(m_csLists);

		for (int i = 0; i < m_arContacts.getCount(); i++) {
			if (used[i]) continue;

			const char* lastds = strchr(m_arContacts[i].email, '@');
			bool newdom = true;

			for (int j = 0; j < m_arContacts.getCount(); j++) {
				if (used[j]) continue;

				const MsnContact& C = m_arContacts[j];
				if (C.list == LIST_RL || C.list == LIST_PL || C.list == LIST_LL) {
					used[j] = true;
					continue;
				}

				const char *dom = strchr(C.email, '@');
				if (dom == NULL && lastds == NULL) {
					if (sz == 0) sz = mir_snprintf(cxml + sz, sizeof(cxml), "<ml l=\"1\">");
					if (newdom) {
						sz += mir_snprintf(cxml + sz, sizeof(cxml) - sz, "<t>");
						newdom = false;
					}

					sz += mir_snprintf(cxml + sz, sizeof(cxml) - sz, "<c n=\"%s\" l=\"%d\"/>", C.email, C.list & ~(LIST_RL | LIST_LL));
					used[j] = true;
				}
				else if (dom != NULL && lastds != NULL && _stricmp(lastds, dom) == 0) {
					if (sz == 0) sz = mir_snprintf(cxml, sizeof(cxml), "<ml l=\"1\">");
					if (newdom) {
						sz += mir_snprintf(cxml + sz, sizeof(cxml) - sz, "<d n=\"%s\">", lastds + 1);
						newdom = false;
					}

					*(char*)dom = 0;
					sz += mir_snprintf(cxml + sz, sizeof(cxml) - sz, "<c n=\"%s\" l=\"%d\" t=\"%d\"/>", C.email, C.list & ~(LIST_RL | LIST_LL), C.netId);
					*(char*)dom = '@';
					used[j] = true;
				}

				if (used[j] && sz > 7400) {
					sz += mir_snprintf(cxml + sz, sizeof(cxml) - sz, "</%c></ml>", lastds ? 'd' : 't');
					msnNsThread->sendPacket("ADL", "%d\r\n%s", sz, cxml);
					sz = 0;
					newdom = true;
				}
			}
			if (!newdom)
				sz += mir_snprintf(cxml + sz, sizeof(cxml) - sz, lastds ? "</d>" : "</t>");
		}
	}

	if (sz) {
		sz += mir_snprintf(cxml + sz, sizeof(cxml) - sz, "</ml>");
		msnNsThread->sendPacket("ADL", "%d\r\n%s", sz, cxml);
	}

	mir_free(used);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN Server List Manager dialog procedure

static void AddPrivacyListEntries(HWND hwndList, CMsnProto *proto)
{
	CLCINFOITEM cii = { 0 };
	cii.cbSize = sizeof(cii);
	cii.flags = CLCIIF_BELOWCONTACTS;

	// Delete old info
	HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);
	while (hItem) {
		HANDLE hItemNext = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactInfo(hItem))
			SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);

		hItem = hItemNext;
	}

	// Add new info
	for (int i = 0; i < proto->m_arContacts.getCount(); ++i) {
		MsnContact &cont = proto->m_arContacts[i];
		if (!(cont.list & (LIST_FL | LIST_LL))) {
			cii.pszText = (TCHAR*)cont.email;
			HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_ADDINFOITEMA, 0, (LPARAM)&cii);

			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0, (cont.list & LIST_LL) ? 1 : 0));
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(1, (cont.list & LIST_FL) ? 2 : 0));
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(2, (cont.list & LIST_AL) ? 3 : 0));
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(3, (cont.list & LIST_BL) ? 4 : 0));
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(4, (cont.list & LIST_RL) ? 5 : 0));
		}
	}
}

static void SetContactIcons(MCONTACT hItem, HWND hwndList, CMsnProto* proto)
{
	if (!proto->MSN_IsMyContact(hItem)) {
		SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
		return;
	}

	char szEmail[MSN_MAX_EMAIL_LEN];
	if (db_get_static(hItem, proto->m_szModuleName, "e-mail", szEmail, sizeof(szEmail))) {
		SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
		return;
	}

	DWORD dwMask = proto->Lists_GetMask(szEmail);
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0, (dwMask & LIST_LL) ? 1 : 0));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(1, (dwMask & LIST_FL) ? 2 : 0));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(2, (dwMask & LIST_AL) ? 3 : 0));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(3, (dwMask & LIST_BL) ? 4 : 0));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(4, (dwMask & LIST_RL) ? 5 : 0));
}

static void SetAllContactIcons(MCONTACT hItem, HWND hwndList, CMsnProto* proto)
{
	if (hItem == NULL)
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		MCONTACT hItemN = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				SetAllContactIcons(hItemT, hwndList, proto);
		}
		else if (IsHContactContact(hItem))
			SetContactIcons(hItem, hwndList, proto);

		hItem = hItemN;
	}
}

static void SaveListItem(MCONTACT hContact, const char* szEmail, int list, int iPrevValue, int iNewValue, CMsnProto* proto)
{
	if (iPrevValue == iNewValue)
		return;

	if (iNewValue == 0) {
		if (list & LIST_FL) {
			DeleteParam param = { proto, hContact };
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DELETECONTACT), NULL, DlgDeleteContactUI, (LPARAM)&param);
			return;
		}

		list |= LIST_REMOVE;
	}

	proto->MSN_AddUser(hContact, szEmail, proto->Lists_GetNetId(szEmail), list);
}

static void SaveSettings(MCONTACT hItem, HWND hwndList, CMsnProto* proto)
{
	if (hItem == NULL)
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				SaveSettings(hItemT, hwndList, proto);
		}
		else {
			char szEmail[MSN_MAX_EMAIL_LEN];

			if (IsHContactContact(hItem)) {
				if (db_get_static(hItem, proto->m_szModuleName, "e-mail", szEmail, sizeof(szEmail)))
					continue;
			}
			else if (IsHContactInfo(hItem)) {
				TCHAR buf[MSN_MAX_EMAIL_LEN];
				SendMessage(hwndList, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)buf);
				WideCharToMultiByte(CP_ACP, 0, buf, -1, szEmail, sizeof(szEmail), 0, 0);

			}

			int dwMask = proto->Lists_GetMask(szEmail);
			SaveListItem(hItem, szEmail, LIST_LL, (dwMask & LIST_LL) ? 1 : 0, SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0, 0)), proto);
			SaveListItem(hItem, szEmail, LIST_FL, (dwMask & LIST_FL) ? 2 : 0, SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(1, 0)), proto);
			SaveListItem(hItem, szEmail, LIST_AL, (dwMask & LIST_AL) ? 3 : 0, SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(2, 0)), proto);
			SaveListItem(hItem, szEmail, LIST_BL, (dwMask & LIST_BL) ? 4 : 0, SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(3, 0)), proto);

			int newMask = proto->Lists_GetMask(szEmail);
			int xorMask = newMask ^ dwMask;

			if (xorMask && newMask & (LIST_FL | LIST_LL)) {
				MCONTACT hContact = IsHContactInfo(hItem) ? proto->MSN_HContactFromEmail(szEmail, szEmail, true, false) : hItem;
				proto->MSN_SetContactDb(hContact, szEmail);
			}

			if (xorMask & (LIST_FL | LIST_LL) && !(newMask & (LIST_FL | LIST_LL))) {
				if (!IsHContactInfo(hItem)) {
					CallService(MS_DB_CONTACT_DELETE, (WPARAM)hItem, 0);
					MsnContact* msc = proto->Lists_Get(szEmail);
					if (msc) msc->hContact = NULL;
				}
			}
		}
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
	}
}

INT_PTR CALLBACK DlgProcMsnServLists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMsnProto *proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	NMCLISTCONTROL *nmc;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 5, 5);

			HICON hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT);
			ImageList_AddIcon(hIml, hIcon);
			Skin_ReleaseIcon(hIcon);

			hIcon = LoadIconEx("list_lc");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ICON_LC, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("list_fl");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ICON_FL, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("list_al");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ICON_AL, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("list_bl");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ICON_BL, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("list_rl");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ICON_RL, STM_SETICON, (WPARAM)hIcon, 0);

			HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

			SendMessage(hwndList, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			SendMessage(hwndList, CLM_SETEXTRACOLUMNS, 5, 0);

			EnableWindow(hwndList, ((CMsnProto*)lParam)->msnLoggedIn);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LISTREFRESH) {
			HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
			SendMessage(hwndList, CLM_AUTOREBUILD, 0, 0);

			EnableWindow(hwndList, proto->msnLoggedIn);
		}
		break;

	case WM_NOTIFY:
		nmc = (NMCLISTCONTROL*)lParam;
		if (nmc->hdr.idFrom == 0 && nmc->hdr.code == (unsigned)PSN_APPLY) {
			HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
			SaveSettings(NULL, hwndList, proto);
			SendMessage(hwndList, CLM_AUTOREBUILD, 0, 0);
			EnableWindow(hwndList, proto->msnLoggedIn);
		}
		else if (nmc->hdr.idFrom == IDC_LIST) {
			switch (nmc->hdr.code) {
			case CLN_NEWCONTACT:
				if ((nmc->flags & (CLNF_ISGROUP | CLNF_ISINFO)) == 0)
					SetContactIcons((MCONTACT)nmc->hItem, nmc->hdr.hwndFrom, proto);
				break;

			case CLN_LISTREBUILT:
				AddPrivacyListEntries(nmc->hdr.hwndFrom, proto);
				SetAllContactIcons(NULL, nmc->hdr.hwndFrom, proto);
				break;

			case NM_CLICK:
				// Make sure we have an extra column, also we can't change RL list
				if (nmc->iColumn == -1 || nmc->iColumn == 4)
					break;

				// Find clicked item
				DWORD hitFlags;
				HANDLE hItem = (HANDLE)SendMessage(nmc->hdr.hwndFrom, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nmc->pt.x, nmc->pt.y));
				if (hItem == NULL || !(IsHContactContact(hItem) || IsHContactInfo(hItem)))
					break;

				// It was not our extended icon
				if (!(hitFlags & CLCHT_ONITEMEXTRA))
					break;

				// Get image in clicked column (0=none, 1=LL, 2=FL, 3=AL, 4=BL, 5=RL)
				int iImage = SendMessage(nmc->hdr.hwndFrom, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn, 0));
				iImage = iImage ? 0 : nmc->iColumn + 1;

				SendMessage(nmc->hdr.hwndFrom, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn, iImage));
				if (iImage && SendMessage(nmc->hdr.hwndFrom, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn ^ 1, 0)) != EMPTY_EXTRA_ICON)
					if (nmc->iColumn == 2 || nmc->iColumn == 3)
						SendMessage(nmc->hdr.hwndFrom, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn ^ 1, 0));

				// Activate Apply button
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_DESTROY:
		HIMAGELIST hIml = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGELIST, 0, 0);
		ImageList_Destroy(hIml);
		ReleaseIconEx("list_fl");
		ReleaseIconEx("list_al");
		ReleaseIconEx("list_bl");
		ReleaseIconEx("list_rl");
		break;
	}

	return FALSE;
}
