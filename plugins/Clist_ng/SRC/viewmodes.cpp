/*
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

contact list view modes (CLVM)

$Id: viewmodes.cpp 131 2010-09-29 04:50:58Z silvercircle $

*/

#include <commonheaders.h>

#define TIMERID_VIEWMODEEXPIRE 100

extern HPEN g_hPenCLUIFrames;
extern FRAMEWND *wndFrameViewMode;

typedef int (__cdecl *pfnEnumCallback)(char *szName);
static HWND clvmHwnd = 0;
static int clvm_curItem = 0;
HMENU hViewModeMenu = 0;

static HWND hwndSelector = 0;
static HIMAGELIST himlViewModes = 0;
static HANDLE hInfoItem = 0;
static int nullImage;
static DWORD stickyStatusMask = 0;
static char g_szModename[2048];

static int g_ViewModeOptDlg = FALSE;

static UINT _page1Controls[] = {IDC_STATIC1, IDC_STATIC2, IDC_STATIC3, IDC_STATIC5, IDC_STATIC4,
    IDC_STATIC8, IDC_ADDVIEWMODE, IDC_DELETEVIEWMODE, IDC_NEWVIEMODE, IDC_GROUPS, IDC_PROTOCOLS,
    IDC_VIEWMODES, IDC_STATUSMODES, IDC_STATIC12, IDC_STATIC13, IDC_STATIC14, IDC_PROTOGROUPOP, IDC_GROUPSTATUSOP,
    IDC_AUTOCLEAR, IDC_AUTOCLEARVAL, IDC_AUTOCLEARSPIN, IDC_STATIC15, IDC_STATIC16,
	IDC_LASTMESSAGEOP, IDC_LASTMESSAGEUNIT, IDC_LASTMSG, IDC_LASTMSGVALUE, 0};

static UINT _page2Controls[] = {IDC_CLIST, IDC_STATIC9, IDC_STATIC8, IDC_CLEARALL, IDC_CURVIEWMODE2, 0};


/*
 * enumerate all view modes, call the callback function with the mode name
 * useful for filling lists, menus and so on..
 */

int CLVM_EnumProc(const char *szSetting, LPARAM lParam)
{
	pfnEnumCallback EnumCallback = (pfnEnumCallback)lParam;
	if (szSetting != NULL)
		EnumCallback((char *)szSetting);
	return(0);
}

void CLVM_EnumModes(pfnEnumCallback EnumCallback)
{
    DBCONTACTENUMSETTINGS dbces;

    dbces.pfnEnumProc = CLVM_EnumProc;
    dbces.szModule = CLVM_MODULE;
    dbces.ofsSettings=0;
    dbces.lParam = (LPARAM)EnumCallback;
    CallService(MS_DB_CONTACT_ENUMSETTINGS,0,(LPARAM)&dbces);
}

int FillModes(char *szsetting)
{
    if(szsetting[0] == 'ö')
        return 1;
    SendDlgItemMessageA(clvmHwnd, IDC_VIEWMODES, LB_INSERTSTRING, -1, (LPARAM)szsetting);
    return 1;
}

static void ShowPage(HWND hwnd, int page)
{
    int i = 0;
    int pageChange = 0;

    if(page == 0 && IsWindowVisible(GetDlgItem(hwnd, _page2Controls[0])))
        pageChange = 1;

    if(page == 1 && IsWindowVisible(GetDlgItem(hwnd, _page1Controls[0])))
        pageChange = 1;

    if(pageChange)
        SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

    switch(page) {
        case 0:
            while(_page1Controls[i] != 0)
                ShowWindow(GetDlgItem(hwnd, _page1Controls[i++]), SW_SHOW);
            i = 0;
            while(_page2Controls[i] != 0)
                ShowWindow(GetDlgItem(hwnd, _page2Controls[i++]), SW_HIDE);
            break;
        case 1:
            while(_page1Controls[i] != 0)
                ShowWindow(GetDlgItem(hwnd, _page1Controls[i++]), SW_HIDE);
            i = 0;
            while(_page2Controls[i] != 0)
                ShowWindow(GetDlgItem(hwnd, _page2Controls[i++]), SW_SHOW);
            break;
    }
    if(pageChange) {
        SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
    }
}

static int UpdateClistItem(HANDLE hContact, DWORD mask)
{
    int i;

    for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
        SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, (WPARAM)hContact, MAKELONG(i - ID_STATUS_OFFLINE,
                           (1 << (i - ID_STATUS_OFFLINE)) & mask ? i - ID_STATUS_OFFLINE : nullImage));

    return 0;
}

static DWORD GetMaskForItem(HANDLE hItem)
{
    int i;
    DWORD dwMask = 0;

    for(i = 0; i <= ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE; i++)
        dwMask |= (SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i) == nullImage ? 0 : 1 << i);

    return dwMask;
}

static void UpdateStickies()
{
	MCONTACT hContact = db_find_first();
    HANDLE hItem;
    DWORD localMask;
    int i;

    while(hContact) {
        hItem = (HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_FINDCONTACT, (WPARAM)hContact, 0);
        if(hItem)
            SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, cfg::getByte(hContact, "CLVM", g_szModename, 0) ? 1 : 0);
        localMask = HIWORD(cfg::getDword(hContact, "CLVM", g_szModename, 0));
        UpdateClistItem(hItem, (localMask == 0 || localMask == stickyStatusMask) ? stickyStatusMask : localMask);
        hContact = db_find_next(hContact);
    }

    {
        HANDLE hItem;

        for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
            SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, (WPARAM)hInfoItem, MAKELONG(i - ID_STATUS_OFFLINE, (1 << (i - ID_STATUS_OFFLINE)) & stickyStatusMask ? i - ID_STATUS_OFFLINE : ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE + 1));

        hItem=(HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETNEXTITEM,CLGN_ROOT,0);
        hItem=(HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST,CLM_GETNEXTITEM,CLGN_NEXTGROUP, (LPARAM)hItem);
        while(hItem) {
            for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
                SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELONG(i - ID_STATUS_OFFLINE, nullImage));
            hItem=(HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
        }
        ShowPage(clvmHwnd, 0);
    }
}

static int FillDialog(HWND hwnd)
{
	LVCOLUMN lvc = {0};
	HWND hwndList = GetDlgItem(hwnd, IDC_PROTOCOLS);
	LVITEMA item = {0};
	int protoCount = 0, i, newItem;
	PROTOACCOUNT **accs = 0;

	CLVM_EnumModes(FillModes);
	ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_PROTOCOLS), LVS_EX_CHECKBOXES);
	lvc.mask = LVCF_FMT;
	lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
	ListView_InsertColumn(GetDlgItem(hwnd, IDC_PROTOCOLS), 0, &lvc);

	// fill protocols...

	Proto_EnumAccounts( &protoCount, &accs );
	item.mask = LVIF_TEXT;
	item.iItem = 1000;
	for (i = 0; i < protoCount; i++) {
		item.pszText = accs[i]->szModuleName;
		newItem = SendMessageA(hwndList, LVM_INSERTITEMA, 0, (LPARAM)&item);
	}

	ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
	ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);

	// fill groups
	{
		LVITEM item = {0};
		char buf[20];
		DBVARIANT dbv = {0};

		hwndList = GetDlgItem(hwnd, IDC_GROUPS);

		ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
		lvc.mask = LVCF_FMT;
		lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		ListView_InsertColumn(hwndList, 0, &lvc);

		item.mask = LVIF_TEXT;
		item.iItem = 1000;

		item.pszText = TranslateT("Ungrouped contacts");
		newItem = SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);

		for(i = 0;;i++) {
			mir_snprintf(buf, 20, "%d", i);
			if(cfg::getTString(NULL, "CListGroups", buf, &dbv))
				break;

			item.pszText = &dbv.ptszVal[1];
			newItem = SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);
			db_free(&dbv);
		}
		ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
		ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);
	}
	hwndList = GetDlgItem(hwnd, IDC_STATUSMODES);
	ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
	lvc.mask = LVCF_FMT;
	lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
	ListView_InsertColumn(hwndList, 0, &lvc);
	{
		LVITEMW item = { 0 };
		item.mask = LVIF_TEXT;
		item.iItem = 1000;
		for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
			item.pszText = TranslateTS(pcli->pfnGetStatusModeDescription(i, 0));
			item.iItem = i - ID_STATUS_OFFLINE;
			newItem = SendMessageA(hwndList, LVM_INSERTITEMA, 0, (LPARAM)&item);
		}
	}
	ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
	ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);

	SendDlgItemMessage(hwnd, IDC_PROTOGROUPOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("And"));
	SendDlgItemMessage(hwnd, IDC_PROTOGROUPOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Or"));
	SendDlgItemMessage(hwnd, IDC_GROUPSTATUSOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("And"));
	SendDlgItemMessage(hwnd, IDC_GROUPSTATUSOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Or"));

	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Older than"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Newer than"));

	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Minutes"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Hours"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Days"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEOP, CB_SETCURSEL, 0, 0);
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_SETCURSEL, 0, 0);
	SetDlgItemInt(hwnd, IDC_LASTMSGVALUE, 0, 0);
	return 0;
}

static void SetAllChildIcons(HWND hwndList,HANDLE hFirstItem,int iColumn,int iImage)
{
	int typeOfFirst,iOldIcon;
	HANDLE hItem,hChildItem;

	typeOfFirst=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hFirstItem,0);
	//check groups
	if(typeOfFirst==CLCIT_GROUP) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hFirstItem);
	while(hItem) {
		hChildItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hChildItem)
            SetAllChildIcons(hwndList,hChildItem,iColumn,iImage);
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTGROUP,(LPARAM)hItem);
	}
	//check contacts
	if(typeOfFirst==CLCIT_CONTACT) hItem=hFirstItem;
	else hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hFirstItem);
	while(hItem) {
		iOldIcon=SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
		if(iOldIcon!=0xFF && iOldIcon!=iImage) SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_NEXTCONTACT,(LPARAM)hItem);
	}
}

static void SetIconsForColumn(HWND hwndList,HANDLE hItem,HANDLE hItemAll,int iColumn,int iImage)
{
	int itemType;

	itemType=SendMessage(hwndList,CLM_GETITEMTYPE,(WPARAM)hItem,0);
	if(itemType==CLCIT_CONTACT) {
		int oldiImage = SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
		if (oldiImage!=0xFF&&oldiImage!=iImage)
			SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
	}
	else if(itemType==CLCIT_INFO) {
        int oldiImage = SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
        if (oldiImage!=0xFF&&oldiImage!=iImage)
            SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		if(hItem == hItemAll)
            SetAllChildIcons(hwndList,hItem,iColumn,iImage);
		else
            SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage)); //hItemUnknown
	}
	else if(itemType==CLCIT_GROUP) {
        int oldiImage = SendMessage(hwndList,CLM_GETEXTRAIMAGE,(WPARAM)hItem,iColumn);
        if (oldiImage!=0xFF&&oldiImage!=iImage)
            SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(iColumn,iImage));
		hItem=(HANDLE)SendMessage(hwndList,CLM_GETNEXTITEM,CLGN_CHILD,(LPARAM)hItem);
		if(hItem)
            SetAllChildIcons(hwndList,hItem,iColumn,iImage);
	}
}

void SaveViewMode(const char *name, const wchar_t *szGroupFilter, const char *szProtoFilter, DWORD statusMask, DWORD stickyStatusMask, unsigned int options,
                  unsigned int stickies, unsigned int operators, unsigned int lmdat)
{
    char szSetting[512];

    mir_snprintf(szSetting, 512, "%c%s_PF", 246, name);
    cfg::writeString(NULL, CLVM_MODULE, szSetting, szProtoFilter);
    mir_snprintf(szSetting, 512, "%c%s_GF", 246, name);
    cfg::writeTString(NULL, CLVM_MODULE, szSetting, szGroupFilter);
    mir_snprintf(szSetting, 512, "%c%s_SM", 246, name);
    cfg::writeDword(CLVM_MODULE, szSetting, statusMask);
    mir_snprintf(szSetting, 512, "%c%s_SSM", 246, name);
    cfg::writeDword(CLVM_MODULE, szSetting, stickyStatusMask);
    mir_snprintf(szSetting, 512, "%c%s_OPT", 246, name);
    cfg::writeDword(CLVM_MODULE, szSetting, options);
    mir_snprintf(szSetting, 512, "%c%s_LM", 246, name);
    cfg::writeDword(CLVM_MODULE, szSetting, lmdat);

    cfg::writeDword(CLVM_MODULE, name, MAKELONG((unsigned short)operators, (unsigned short)stickies));
}

/*
 * saves the state of the filter definitions for the current item
 */

void SaveState()
{
    wchar_t newGroupFilter[2048] = _T("|");
    char newProtoFilter[2048] = "|";
    int i, iLen;
    HWND hwndList;
    char *szModeName = NULL;
    DWORD statusMask = 0;
	MCONTACT hContact;
    HANDLE hItem;
    DWORD operators = 0;

    if(clvm_curItem == -1)
        return;

    {
        LVITEMA item = {0};
        char szTemp[256];

        hwndList = GetDlgItem(clvmHwnd, IDC_PROTOCOLS);
        for(i = 0; i < ListView_GetItemCount(hwndList); i++) {
            if(ListView_GetCheckState(hwndList, i)) {
                item.mask = LVIF_TEXT;
                item.pszText = szTemp;
                item.cchTextMax = 255;
                item.iItem = i;
                SendMessageA(hwndList, LVM_GETITEMA, 0, (LPARAM)&item);
                strncat(newProtoFilter, szTemp, 2048);
                strncat(newProtoFilter, "|", 2048);
                newProtoFilter[2047] = 0;
            }
        }
    }

    {
        LVITEM item = {0};
        wchar_t szTemp[256];

        hwndList = GetDlgItem(clvmHwnd, IDC_GROUPS);

        operators |= ListView_GetCheckState(hwndList, 0) ? CLVM_INCLUDED_UNGROUPED : 0;

        for(i = 0; i < ListView_GetItemCount(hwndList); i++) {
            if(ListView_GetCheckState(hwndList, i)) {
                item.mask = LVIF_TEXT;
                item.pszText = szTemp;
                item.cchTextMax = 255;
                item.iItem = i;
                SendMessage(hwndList, LVM_GETITEM, 0, (LPARAM)&item);
                _tcsncat(newGroupFilter, szTemp, 2048);
                _tcsncat(newGroupFilter, _T("|"), 2048);
                newGroupFilter[2047] = 0;
            }
        }
    }
    hwndList = GetDlgItem(clvmHwnd, IDC_STATUSMODES);
    for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
        if(ListView_GetCheckState(hwndList, i - ID_STATUS_OFFLINE))
            statusMask |= (1 << (i - ID_STATUS_OFFLINE));
    }
    iLen = SendMessageA(GetDlgItem(clvmHwnd, IDC_VIEWMODES), LB_GETTEXTLEN, clvm_curItem, 0);
    if(iLen) {
        unsigned int stickies = 0;
        DWORD dwGlobalMask, dwLocalMask;
		BOOL translated;

        szModeName = ( char* )malloc(iLen + 1);
        if(szModeName) {
            DWORD options, lmdat;
            //char *vastring = NULL;
            //int len = GetWindowTextLengthA(GetDlgItem(clvmHwnd, IDC_VARIABLES)) + 1;

            //vastring = (char *)malloc(len);
            //if(vastring)
            //    GetDlgItemTextA(clvmHwnd, IDC_VARIABLES, vastring, len);
            SendDlgItemMessageA(clvmHwnd, IDC_VIEWMODES, LB_GETTEXT, clvm_curItem, (LPARAM)szModeName);
            dwGlobalMask = GetMaskForItem(hInfoItem);
			hContact = db_find_first();
            while(hContact) {
                hItem = (HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_FINDCONTACT, (WPARAM)hContact, 0);
                if(hItem) {
                    if(SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
                        dwLocalMask = GetMaskForItem(hItem);
                        cfg::writeDword(hContact, "CLVM", szModeName, MAKELONG(1, (unsigned short)dwLocalMask));
                        stickies++;
                    }
                    else {
                        if(cfg::getDword(hContact, "CLVM", szModeName, 0))
                        	cfg::writeDword(hContact, "CLVM", szModeName, 0);
                    }
                }
				hContact = db_find_next(hContact);
            }
            operators |= ((SendDlgItemMessage(clvmHwnd, IDC_PROTOGROUPOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_PROTOGROUP_OP : 0) |
                        (SendDlgItemMessage(clvmHwnd, IDC_GROUPSTATUSOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_GROUPSTATUS_OP : 0) |
                        (IsDlgButtonChecked(clvmHwnd, IDC_AUTOCLEAR) ? CLVM_AUTOCLEAR : 0) |
						(IsDlgButtonChecked(clvmHwnd, IDC_LASTMSG) ? CLVM_USELASTMSG : 0));

            options = SendDlgItemMessage(clvmHwnd, IDC_AUTOCLEARSPIN, UDM_GETPOS, 0, 0);

			lmdat = MAKELONG(GetDlgItemInt(clvmHwnd, IDC_LASTMSGVALUE, &translated, FALSE),
							 MAKEWORD(SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEOP, CB_GETCURSEL, 0, 0),
								      SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEUNIT, CB_GETCURSEL, 0, 0)));

            SaveViewMode(szModeName, newGroupFilter, newProtoFilter, statusMask, dwGlobalMask, options,
                         stickies, operators, lmdat);
            //free(vastring);
            free(szModeName);
        }
    }
    Utils::enableDlgControl(clvmHwnd, IDC_APPLY, FALSE);
}


/*
 * updates the filter list boxes with the data taken from the filtering string
 */

void UpdateFilters()
{
    DBVARIANT dbv_pf = {0};
    DBVARIANT dbv_gf = {0};
    char szSetting[128];
    char *szBuf = NULL;
    int iLen;
	DWORD statusMask = 0;
    DWORD dwFlags;
    DWORD opt;
    char  szTemp[100];

    if(clvm_curItem == LB_ERR)
        return;

    iLen = SendDlgItemMessageA(clvmHwnd, IDC_VIEWMODES, LB_GETTEXTLEN, clvm_curItem, 0);

    if(iLen == 0)
        return;

    szBuf = (char *)malloc(iLen + 1);
    SendDlgItemMessageA(clvmHwnd, IDC_VIEWMODES, LB_GETTEXT, clvm_curItem, (LPARAM)szBuf);
    strncpy(g_szModename, szBuf, sizeof(g_szModename));
    g_szModename[sizeof(g_szModename) - 1] = 0;
    mir_snprintf(szTemp, 100, Translate("Current view mode: %s"), g_szModename);
    SetDlgItemTextA(clvmHwnd, IDC_CURVIEWMODE2, szTemp);
    mir_snprintf(szSetting, 128, "%c%s_PF", 246, szBuf);
    if(db_get(NULL, CLVM_MODULE, szSetting, &dbv_pf))
        goto cleanup;
    mir_snprintf(szSetting, 128, "%c%s_GF", 246, szBuf);
    if(cfg::getTString(NULL, CLVM_MODULE, szSetting, &dbv_gf))
        goto cleanup;
    mir_snprintf(szSetting, 128, "%c%s_OPT", 246, szBuf);
    if((opt = cfg::getDword(NULL, CLVM_MODULE, szSetting, -1)) != -1) {
        SendDlgItemMessage(clvmHwnd, IDC_AUTOCLEARSPIN, UDM_SETPOS, 0, MAKELONG(LOWORD(opt), 0));
    }
    mir_snprintf(szSetting, 128, "%c%s_SM", 246, szBuf);
    statusMask = cfg::getDword(CLVM_MODULE, szSetting, -1);
    mir_snprintf(szSetting, 128, "%c%s_SSM", 246, szBuf);
    stickyStatusMask = cfg::getDword(CLVM_MODULE, szSetting, -1);
    dwFlags = cfg::getDword(CLVM_MODULE, szBuf, 0);
    {
        LVITEMA item = {0};
        char szTemp[256];
        char szMask[256];
        int i;
        HWND hwndList = GetDlgItem(clvmHwnd, IDC_PROTOCOLS);

        item.mask = LVIF_TEXT;
        item.pszText = szTemp;
        item.cchTextMax = 255;

        for(i = 0; i < ListView_GetItemCount(hwndList); i++) {
            item.iItem = i;
            SendMessageA(hwndList, LVM_GETITEMA, 0, (LPARAM)&item);
            mir_snprintf(szMask, 256, "%s|", szTemp);
            if(dbv_pf.pszVal && strstr(dbv_pf.pszVal, szMask))
                ListView_SetCheckState(hwndList, i, TRUE)
            else
                ListView_SetCheckState(hwndList, i, FALSE);
        }
    }
    {
        LVITEM item = {0};
        wchar_t szTemp[256];
        wchar_t szMask[256];
        int i;
        HWND hwndList = GetDlgItem(clvmHwnd, IDC_GROUPS);

        item.mask = LVIF_TEXT;
        item.pszText = szTemp;
        item.cchTextMax = 255;

        ListView_SetCheckState(hwndList, 0, dwFlags & CLVM_INCLUDED_UNGROUPED ? TRUE : FALSE);

        for(i = 1; i < ListView_GetItemCount(hwndList); i++) {
            item.iItem = i;
            SendMessage(hwndList, LVM_GETITEM, 0, (LPARAM)&item);
            _sntprintf(szMask, 256, _T("%s|"), szTemp);
            if(dbv_gf.ptszVal && _tcsstr(dbv_gf.ptszVal, szMask))
                ListView_SetCheckState(hwndList, i, TRUE)
            else
                ListView_SetCheckState(hwndList, i, FALSE);
        }
    }
    {
        HWND hwndList = GetDlgItem(clvmHwnd, IDC_STATUSMODES);
        int i;

        for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
            if((1 << (i - ID_STATUS_OFFLINE)) & statusMask)
                ListView_SetCheckState(hwndList, i - ID_STATUS_OFFLINE, TRUE)
            else
                ListView_SetCheckState(hwndList, i - ID_STATUS_OFFLINE, FALSE);
        }
    }
    SendDlgItemMessage(clvmHwnd, IDC_PROTOGROUPOP, CB_SETCURSEL, dwFlags & CLVM_PROTOGROUP_OP ? 1 : 0, 0);
    SendDlgItemMessage(clvmHwnd, IDC_GROUPSTATUSOP, CB_SETCURSEL, dwFlags & CLVM_GROUPSTATUS_OP ? 1 : 0, 0);
    CheckDlgButton(clvmHwnd, IDC_AUTOCLEAR, dwFlags & CLVM_AUTOCLEAR ? 1 : 0);
    UpdateStickies();

	{
		int useLastMsg = dwFlags & CLVM_USELASTMSG;
		DWORD lmdat;
		BYTE bTmp;

		CheckDlgButton(clvmHwnd, IDC_LASTMSG, useLastMsg);
		Utils::enableDlgControl(clvmHwnd, IDC_LASTMESSAGEOP, useLastMsg);
		Utils::enableDlgControl(clvmHwnd, IDC_LASTMSGVALUE, useLastMsg);
		Utils::enableDlgControl(clvmHwnd, IDC_LASTMESSAGEUNIT, useLastMsg);

	    mir_snprintf(szSetting, 128, "%c%s_LM", 246, szBuf);
		lmdat = cfg::getDword(CLVM_MODULE, szSetting, 0);

		SetDlgItemInt(clvmHwnd, IDC_LASTMSGVALUE, LOWORD(lmdat), FALSE);
		bTmp = LOBYTE(HIWORD(lmdat));
		SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEOP, CB_SETCURSEL, bTmp, 0);
		bTmp = HIBYTE(HIWORD(lmdat));
		SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEUNIT, CB_SETCURSEL, bTmp, 0);
	}

	ShowPage(clvmHwnd, 0);
cleanup:
    db_free(&dbv_pf);
    db_free(&dbv_gf);
    free(szBuf);
}

INT_PTR CALLBACK DlgProcViewModesSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    clvmHwnd = hwndDlg;

    switch(msg) {
        case WM_INITDIALOG:
        {
            int i = 0;
            TCITEMA tci;
            RECT rcClient;
            CLCINFOITEM cii = {0};
            HICON hIcon;

            if(IS_THEMED)
                Api::pfnEnableThemeDialogTexture(hwndDlg, ETDT_ENABLETAB);

            himlViewModes = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 12, 0);
            for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
                ImageList_AddIcon(himlViewModes, Skin_LoadProtoIcon(NULL, i));

            hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_MINIMIZE), IMAGE_ICON, 16, 16, 0);
            nullImage = ImageList_AddIcon(himlViewModes, hIcon);
            DestroyIcon(hIcon);
            GetClientRect(hwndDlg, &rcClient);

            tci.mask = TCIF_PARAM|TCIF_TEXT;
            tci.lParam = 0;
            tci.pszText = Translate("Sticky contacts");
            SendMessageA(GetDlgItem(hwndDlg, IDC_TAB), TCM_INSERTITEMA, (WPARAM)0, (LPARAM)&tci);

            tci.pszText = Translate("Filtering");
            SendMessageA(GetDlgItem(hwndDlg, IDC_TAB), TCM_INSERTITEMA, (WPARAM)0, (LPARAM)&tci);

            TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TAB), 0);

            TranslateDialogDefault(hwndDlg);
            FillDialog(hwndDlg);
            Utils::enableDlgControl(hwndDlg, IDC_ADDVIEWMODE, FALSE);

            SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)himlViewModes);
            SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETEXTRACOLUMNS, ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE, 0);
            cii.cbSize = sizeof(cii);
            cii.hParentGroup = 0;
            cii.pszText = _T("*** All contacts ***");
            hInfoItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
            SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
            if(SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, 0, 0) != LB_ERR) {
                clvm_curItem = 0;
                UpdateFilters();
            }
            else
                clvm_curItem = -1;
            g_ViewModeOptDlg = TRUE;
            i = 0;
            while(_page2Controls[i] != 0)
                ShowWindow(GetDlgItem(hwndDlg, _page2Controls[i++]), SW_HIDE);
            ShowWindow(hwndDlg, SW_SHOWNORMAL);
            Utils::enableDlgControl(hwndDlg, IDC_APPLY, FALSE);
            SendDlgItemMessage(hwndDlg, IDC_AUTOCLEARSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 0));
            SetWindowText(hwndDlg, TranslateT("Configure view modes"));
            return TRUE;
        }
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDC_PROTOGROUPOP:
                case IDC_GROUPSTATUSOP:
				case IDC_LASTMESSAGEUNIT:
				case IDC_LASTMESSAGEOP:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    	Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
                    break;
                case IDC_AUTOCLEAR:
                	Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
                    break;
				case IDC_LASTMSG:
					{
						int bUseLastMsg = IsDlgButtonChecked(hwndDlg, IDC_LASTMSG);
						Utils::enableDlgControl(hwndDlg, IDC_LASTMESSAGEOP, bUseLastMsg);
						Utils::enableDlgControl(hwndDlg, IDC_LASTMESSAGEUNIT, bUseLastMsg);
						Utils::enableDlgControl(hwndDlg, IDC_LASTMSGVALUE, bUseLastMsg);
						Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
						break;
					}
                case IDC_AUTOCLEARVAL:
				case IDC_LASTMSGVALUE:
                    if(HIWORD(wParam) == EN_CHANGE && GetFocus() == (HWND)lParam)
                    	Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
                    break;
                case IDC_DELETEVIEWMODE:
                {
                    if(MessageBoxA(0, Translate("Really delete this view mode? This cannot be undone"), Translate("Delete a view mode"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        char szSetting[256];
                        int iLen = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETTEXTLEN, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), 0);
                        if(iLen) {
                            char *szBuf = ( char* )malloc(iLen + 1);
                            if(szBuf) {
                                MCONTACT hContact;

                                SendDlgItemMessageA(hwndDlg, IDC_VIEWMODES, LB_GETTEXT, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), (LPARAM)szBuf);
                                mir_snprintf(szSetting, 256, "%c%s_PF", 246, szBuf);
                                db_unset(NULL, CLVM_MODULE, szSetting);
                                mir_snprintf(szSetting, 256, "%c%s_GF", 246, szBuf);
								db_unset(NULL, CLVM_MODULE, szSetting);
                                mir_snprintf(szSetting, 256, "%c%s_SM", 246, szBuf);
								db_unset(NULL, CLVM_MODULE, szSetting);
                                mir_snprintf(szSetting, 256, "%c%s_VA", 246, szBuf);
								db_unset(NULL, CLVM_MODULE, szSetting);
                                mir_snprintf(szSetting, 256, "%c%s_SSM", 246, szBuf);
								db_unset(NULL, CLVM_MODULE, szSetting);
								db_unset(NULL, CLVM_MODULE, szBuf);
                                if(!strcmp(cfg::dat.current_viewmode, szBuf) && lstrlenA(szBuf) == lstrlenA(cfg::dat.current_viewmode)) {
                                    cfg::dat.bFilterEffective = 0;
                                    pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
                                    SetWindowTextA(hwndSelector, Translate("No view mode"));
                                }
								hContact = db_find_first();
                                while(hContact) {
                                    if(cfg::getDword(hContact, "CLVM", szBuf, -1) != -1)
                                    	cfg::writeDword(hContact, "CLVM", szBuf, 0);
									hContact = db_find_next(hContact);
                                }
                                SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_DELETESTRING, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), 0);
                                if(SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, 0, 0) != LB_ERR) {
                                    clvm_curItem = 0;
                                    UpdateFilters();
                                }
                                else
                                    clvm_curItem = -1;
                                free(szBuf);
                            }
                        }
                    }
                    break;
                }
                case IDC_ADDVIEWMODE:
                {
                    char szBuf[256];

                    szBuf[0] = 0;
                    GetDlgItemTextA(hwndDlg, IDC_NEWVIEMODE, szBuf, 256);
                    szBuf[255] = 0;

                    if(lstrlenA(szBuf) > 2) {
                        if(cfg::getDword(CLVM_MODULE, szBuf, -1) != -1)
                            MessageBox(0, TranslateT("A view mode with this name does alredy exist"), TranslateT("Duplicate name"), MB_OK);
                        else {
                            int iNewItem = SendDlgItemMessageA(hwndDlg, IDC_VIEWMODES, LB_INSERTSTRING, -1, (LPARAM)szBuf);
                            if(iNewItem != LB_ERR) {
                                SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, (WPARAM)iNewItem, 0);
                                SaveViewMode(szBuf, _T(""), "", -1, -1, 0, 0, 0, 0);
                                clvm_curItem = iNewItem;
                                UpdateStickies();
                                SendDlgItemMessage(hwndDlg, IDC_PROTOGROUPOP, CB_SETCURSEL, 0, 0);
                                SendDlgItemMessage(hwndDlg, IDC_GROUPSTATUSOP, CB_SETCURSEL, 0, 0);
                            }
                        }
                        SetDlgItemTextA(hwndDlg, IDC_NEWVIEMODE, "");
                    }
                    Utils::enableDlgControl(hwndDlg, IDC_ADDVIEWMODE, FALSE);
                    break;
                }
                case IDC_CLEARALL:
                {
                    HANDLE		hItem;
					MCONTACT	hContact = db_find_first();

                    while(hContact) {
                        hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, (WPARAM)hContact, 0);
                        if(hItem)
                            SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 0);
						hContact = db_find_next(hContact);
                    }
                }
                case IDOK:
                case IDC_APPLY:
                    SaveState();
                    if(cfg::dat.bFilterEffective)
                        ApplyViewMode(cfg::dat.current_viewmode);
                    if(LOWORD(wParam) == IDOK)
                        DestroyWindow(hwndDlg);
                    break;
                case IDCANCEL:
                    DestroyWindow(hwndDlg);
                    break;
            }
            if(LOWORD(wParam) == IDC_NEWVIEMODE && HIWORD(wParam) == EN_CHANGE)
            	Utils::enableDlgControl(hwndDlg, IDC_ADDVIEWMODE, TRUE);
            if(LOWORD(wParam) == IDC_VIEWMODES && HIWORD(wParam) == LBN_SELCHANGE) {
                SaveState();
                clvm_curItem = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0);
                UpdateFilters();
            }
            break;
        case WM_NOTIFY:
        {
            switch (((LPNMHDR) lParam)->idFrom) {
                case IDC_GROUPS:
                case IDC_STATUSMODES:
                case IDC_PROTOCOLS:
                case IDC_CLIST:
                    if (((LPNMHDR) lParam)->code == NM_CLICK || ((LPNMHDR) lParam)->code == CLN_CHECKCHANGED)
                    	Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
                    switch (((LPNMHDR)lParam)->code)
                    {
                        case CLN_NEWCONTACT:
                        case CLN_LISTREBUILT:
                            //SetAllContactIcons(GetDlgItem(hwndDlg,IDC_CLIST));
                            //fall through
                            /*
                        case CLN_CONTACTMOVED:
                            SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);
                            break;
                        case CLN_OPTIONSCHANGED:
                            ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
                            break;
                        case CLN_CHECKCHANGED:
                        {
                            HANDLE hItem;
                            NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
                            int typeOfItem = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETITEMTYPE,(WPARAM)nm->hItem, 0);
                            break;
                        }*/
                        case NM_CLICK:
                        {
                            HANDLE hItem;
                            NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
                            DWORD hitFlags;
                            int iImage;

                            if(nm->iColumn==-1)
                                break;
                            hItem = (HANDLE)SendDlgItemMessage(hwndDlg,IDC_CLIST,CLM_HITTEST,(WPARAM)&hitFlags,MAKELPARAM(nm->pt.x,nm->pt.y));
                            if(hItem==NULL) break;
                            if(!(hitFlags&CLCHT_ONITEMEXTRA))
                                break;
                            iImage = SendDlgItemMessage(hwndDlg,IDC_CLIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(nm->iColumn,0));
                            if(iImage == nullImage)
                                iImage = nm->iColumn;
                            else if(iImage!=0xFF)
                                iImage = nullImage;
                            SetIconsForColumn(GetDlgItem(hwndDlg,IDC_CLIST),hItem,hInfoItem,nm->iColumn,iImage);
                            //SetListGroupIcons(GetDlgItem(hwndDlg,IDC_CLIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hInfoItem,NULL);
                            break;
                        }
                    }
                    break;
                case IDC_TAB:
                    if (((LPNMHDR) lParam)->code == TCN_SELCHANGE) {
                        int id = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));
                        if(id == 0)
                            ShowPage(hwndDlg, 0);
                        else
                            ShowPage(hwndDlg, 1);
                        break;
                    }

            }
            break;
        }
        case WM_DESTROY:
            ImageList_RemoveAll(himlViewModes);
            ImageList_Destroy(himlViewModes);
            g_ViewModeOptDlg = FALSE;
            break;
    }
    return FALSE;
}

static int menuCounter = 0;

static int FillMenuCallback(char *szSetting)
{
    if(szSetting[0] == 246)
        return 1;

    AppendMenuA(hViewModeMenu, MF_STRING, menuCounter++, szSetting);
    return 1;
}

void BuildViewModeMenu()
{
    if(hViewModeMenu)
        DestroyMenu(hViewModeMenu);

    menuCounter = 100;
    hViewModeMenu = CreatePopupMenu();
    CLVM_EnumModes(FillMenuCallback);

	if(GetMenuItemCount(hViewModeMenu) > 0)
		AppendMenuA(hViewModeMenu, MF_SEPARATOR, 0, NULL);

	AppendMenuA(hViewModeMenu, MF_STRING, 10001, Translate("Setup View Modes..."));
	AppendMenuA(hViewModeMenu, MF_STRING, 10002, Translate("Clear current View Mode"));

}

static UINT _buttons[] = {IDC_RESETMODES, IDC_SELECTMODE, IDC_CONFIGUREMODES, 0};

LRESULT CALLBACK ViewModeFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CREATE:
		{
			HWND hwndButton;

			hwndSelector = CreateWindowEx(0, _T("CLCButtonClass"), _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU) IDC_SELECTMODE, g_hInst, NULL);
			SendMessage(hwndSelector, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Select a view mode"), 0);
			SendMessage(hwndSelector, BM_SETASMENUACTION, 1, 0);
			hwndButton = CreateWindowEx(0, _T("CLCButtonClass"), _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU) IDC_CONFIGUREMODES, g_hInst, NULL);
			SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Setup view modes"), 0);
			hwndButton = CreateWindowEx(0, _T("CLCButtonClass"), _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU) IDC_RESETMODES, g_hInst, NULL);
			SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Clear view mode and return to default display"), 0);
			SendMessage(hwnd, WM_USER + 100, 0, 0);
			return FALSE;
		}
	case WM_NCCALCSIZE:
		{
			BOOL hasTitleBar = wndFrameViewMode ? wndFrameViewMode->TitleBar.ShowTitleBar : 0;
			return FrameNCCalcSize(hwnd, DefWindowProc, wParam, lParam, hasTitleBar);
		}
	case WM_NCPAINT:
		{
			BOOL hasTitleBar = wndFrameViewMode ? wndFrameViewMode->TitleBar.ShowTitleBar : 0;
			return FrameNCPaint(hwnd, DefWindowProc, wParam, lParam, hasTitleBar);
		}
	case WM_SIZE:
		{
			RECT rcCLVMFrame;
			HDWP PosBatch = BeginDeferWindowPos(3);
			GetClientRect(hwnd, &rcCLVMFrame);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_RESETMODES), 0,
				rcCLVMFrame.right - 23, 1, 22, 22, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_CONFIGUREMODES), 0,
				rcCLVMFrame.right - 46, 1, 22, 22, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_SELECTMODE), 0,
				1, 1, rcCLVMFrame.right - 48, 22, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			EndDeferWindowPos(PosBatch);
			break;
		}
	case WM_USER + 100:
			SendMessage(GetDlgItem(hwnd, IDC_RESETMODES), BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("core_main_15"));
			SendMessage(GetDlgItem(hwnd, IDC_CONFIGUREMODES), BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("core_main_32"));
			SendMessage(GetDlgItem(hwnd, IDC_SELECTMODE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("core_main_11"));
		{
			int bSkinned = cfg::getByte("CLCExt", "bskinned", 0);
			int i = 0;

			while(_buttons[i] != 0) {
				SendMessage(GetDlgItem(hwnd, _buttons[i]), BM_SETSKINNED, 0, bSkinned);
				if(bSkinned) {
					SendDlgItemMessage(hwnd, _buttons[i], BUTTONSETASFLATBTN, 0, 0);
					SendDlgItemMessage(hwnd, _buttons[i], BUTTONSETASFLATBTN + 10, 0, 0);
				}
				else {
					SendDlgItemMessage(hwnd, _buttons[i], BUTTONSETASFLATBTN, 0, 1);
					SendDlgItemMessage(hwnd, _buttons[i], BUTTONSETASFLATBTN + 10, 0, 1);
				}
				i++;
			}
		}
		if(cfg::dat.bFilterEffective)
			SetWindowTextA(GetDlgItem(hwnd, IDC_SELECTMODE), cfg::dat.current_viewmode);
		else
			SetWindowText(GetDlgItem(hwnd, IDC_SELECTMODE), TranslateT("No view mode"));
		break;

	case WM_ERASEBKGND:
		return(1);

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rc;
			HDC hdcMem;
			HANDLE  hbp = 0;

			GetClientRect(hwnd, &rc);
			INIT_PAINT(hdc, rc, hdcMem);

			Gfx::drawBGFromSurface(hwnd, rc, hdcMem);
			FINALIZE_PAINT(hbp, &rc, 0);
			EndPaint(hwnd, &ps);
			return 0;
		}
	case WM_TIMER:
		{
			switch(wParam) {
			case TIMERID_VIEWMODEEXPIRE:
				{
					POINT pt;
					RECT rcCLUI;

					GetWindowRect(pcli->hwndContactList, &rcCLUI);
					GetCursorPos(&pt);
					if(PtInRect(&rcCLUI, pt))
						break;

					KillTimer(hwnd, wParam);
					if(!cfg::dat.old_viewmode[0])
						SendMessage(hwnd, WM_COMMAND, IDC_RESETMODES, 0);
					else
						ApplyViewMode((const char *)cfg::dat.old_viewmode);
					break;
			}	}
			break;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
			case IDC_SELECTMODE:
				{
					RECT rc;
					POINT pt;
					int selection;
					MENUITEMINFOA mii = {0};
					char szTemp[256];

					BuildViewModeMenu();
					//GetWindowRect(GetDlgItem(hwnd, IDC_SELECTMODE), &rc);
					GetWindowRect((HWND)lParam, &rc);
					pt.x = rc.left;
					pt.y = rc.bottom;
					selection = TrackPopupMenu(hViewModeMenu,TPM_RETURNCMD|TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
					if(selection) {

						if(selection == 10001)
							goto clvm_config_command;
						else if(selection == 10002)
							goto clvm_reset_command;

						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_STRING;
						mii.dwTypeData = szTemp;
						mii.cch = 256;
						GetMenuItemInfoA(hViewModeMenu, selection, FALSE, &mii);
						ApplyViewMode(szTemp);
					}
					break;
				}
			case IDC_RESETMODES:
clvm_reset_command:
				cfg::dat.bFilterEffective = 0;
				pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
				SetWindowTextA(GetDlgItem(hwnd, IDC_SELECTMODE), Translate("No view mode"));
				CallService(MS_CLIST_SETHIDEOFFLINE, (WPARAM)cfg::dat.boldHideOffline, 0);
				cfg::dat.boldHideOffline = (BYTE)-1;
				CLUI::setButtonStates(pcli->hwndContactList);
				cfg::dat.current_viewmode[0] = 0;
				cfg::dat.old_viewmode[0] = 0;
				cfg::writeString(NULL, "CList", "LastViewMode", "");
				break;
			case IDC_CONFIGUREMODES:
clvm_config_command:
				if(!g_ViewModeOptDlg)
					CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_OPT_VIEWMODES), 0, DlgProcViewModesSetup, 0);
				break;
			}
			break;
		}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

static HWND hCLVMFrame;
HWND g_hwndViewModeFrame;

void CreateViewModeFrame()
{
    CLISTFrame frame = {0};
    WNDCLASS wndclass = {0};

    wndclass.style = 0;
    wndclass.lpfnWndProc = ViewModeFrameWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = g_hInst;
    wndclass.hIcon = 0;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) (COLOR_3DFACE);
    wndclass.lpszMenuName = 0;
    wndclass.lpszClassName = _T("CLVMFrameWindow");

    RegisterClass(&wndclass);

    ZeroMemory(&frame, sizeof(frame));
    frame.cbSize = sizeof(frame);
    frame.tname = _T("View modes");
    frame.TBtname = TranslateT("View Modes");
    frame.hIcon = 0;
    frame.height = 22;
    frame.Flags=F_VISIBLE|F_SHOWTBTIP|F_NOBORDER|F_TCHAR;
    frame.align = alBottom;
    frame.hWnd = CreateWindowEx(0, _T("CLVMFrameWindow"), _T("CLVM"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN, 0, 0, 20, 20, pcli->hwndContactList, (HMENU) 0, g_hInst, NULL);
    g_hwndViewModeFrame = frame.hWnd;
    hCLVMFrame = (HWND)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&frame,(LPARAM)0);
    CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)hCLVMFrame, FU_FMPOS);
}

const char *MakeVariablesString(const char *src, const char *UIN);

void ApplyViewMode(const char *name)
{
    char szSetting[256];
    DBVARIANT dbv = {0};

    cfg::dat.bFilterEffective = 0;

    mir_snprintf(szSetting, 256, "%c%s_PF", 246, name);
    if(!cfg::getString(NULL, CLVM_MODULE, szSetting, &dbv)) {
        if(lstrlenA(dbv.pszVal) >= 2) {
            strncpy(cfg::dat.protoFilter, dbv.pszVal, sizeof(cfg::dat.protoFilter));
            cfg::dat.protoFilter[sizeof(cfg::dat.protoFilter) - 1] = 0;
            cfg::dat.bFilterEffective |= CLVM_FILTER_PROTOS;
        }
        mir_free(dbv.pszVal);
    }
    mir_snprintf(szSetting, 256, "%c%s_GF", 246, name);
    if(!cfg::getTString(NULL, CLVM_MODULE, szSetting, &dbv)) {
        if(lstrlen(dbv.ptszVal) >= 2) {
            _tcsncpy(cfg::dat.groupFilter, dbv.ptszVal, safe_sizeof(cfg::dat.groupFilter));
            cfg::dat.groupFilter[safe_sizeof(cfg::dat.groupFilter) - 1] = 0;
            cfg::dat.bFilterEffective |= CLVM_FILTER_GROUPS;
        }
        mir_free(dbv.ptszVal);
    }
    mir_snprintf(szSetting, 256, "%c%s_SM", 246, name);
    cfg::dat.statusMaskFilter = cfg::getDword(CLVM_MODULE, szSetting, -1);
    if(cfg::dat.statusMaskFilter >= 1)
        cfg::dat.bFilterEffective |= CLVM_FILTER_STATUS;

    mir_snprintf(szSetting, 256, "%c%s_SSM", 246, name);
    cfg::dat.stickyMaskFilter = cfg::getDword(CLVM_MODULE, szSetting, -1);
    if(cfg::dat.stickyMaskFilter != -1)
        cfg::dat.bFilterEffective |= CLVM_FILTER_STICKYSTATUS;

    /*
    mir_snprintf(szSetting, 256, "%c%s_VA", 246, name);
    if(!DBGetContactSettingString(NULL, CLVM_MODULE, szSetting, &dbv)) {
        strncpy(g_CluiData.varFilter, dbv.pszVal, sizeof(g_CluiData.varFilter));
        g_CluiData.varFilter[sizeof(g_CluiData.varFilter) - 1] = 0;
        if(lstrlenA(g_CluiData.varFilter) > 10 && ServiceExists(MS_VARS_FORMATSTRING))
            g_CluiData.bFilterEffective |= CLVM_FILTER_VARIABLES;
        mir_free(dbv.ptszVal);
        if(g_CluiData.bFilterEffective & CLVM_FILTER_VARIABLES) {
            HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
            char UIN[256];
            char *id, *szProto;
            const char *varstring;
            char *temp;
            FORMATINFO fi;

            while(hContact) {
                szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
                if(szProto) {
                    id = (char*) CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
                    if(id) {
                        if(!DBGetContactSetting(hContact, szProto, id, &dbv)) {
                            if(dbv.type == DBVT_ASCIIZ) {
                                mir_snprintf(UIN, 256, "<%s:%s>", szProto, dbv.pszVal);
                            }
                            else {
                                mir_snprintf(UIN, 256, "<%s:%d>", szProto, dbv.dVal);
                            }
                            varstring = MakeVariablesString(g_CluiData.varFilter, UIN);
                            ZeroMemory(&fi, sizeof(fi));
                            fi.cbSize = sizeof(fi);
                            fi.szFormat = varstring;
                            fi.szSource = "";
                            fi.hContact = 0;
                            temp = (char *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
                            if(temp && atol(temp) > 0)
                                _DebugPopup(hContact, "%s, %d, %d, %d", temp, temp, fi.pCount, fi.eCount);
                            variables_free(temp);
                            DBFreeVariant(&dbv);
                        }
                    }
                }
                hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
            }
        }
    }*/

    cfg::dat.filterFlags = cfg::getDword(CLVM_MODULE, name, 0);

    KillTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE);

    if(cfg::dat.filterFlags & CLVM_AUTOCLEAR) {
        DWORD timerexpire;
        mir_snprintf(szSetting, 256, "%c%s_OPT", 246, name);
        timerexpire = LOWORD(cfg::getDword(CLVM_MODULE, szSetting, 0));
        strncpy(cfg::dat.old_viewmode, cfg::dat.current_viewmode, 256);
        cfg::dat.old_viewmode[255] = 0;
        SetTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE, timerexpire * 1000, NULL);
    }
    strncpy(cfg::dat.current_viewmode, name, 256);
    cfg::dat.current_viewmode[255] = 0;

	if(cfg::dat.filterFlags & CLVM_USELASTMSG) {
		DWORD unit;
		int i;
		BYTE bSaved = cfg::dat.sortOrder[0];

		cfg::dat.sortOrder[0] = SORTBY_LASTMSG;
		for(i = 0; i < cfg::nextCacheEntry; i++)
			cfg::eCache[i].dwLastMsgTime = INTSORT_GetLastMsgTime(cfg::eCache[i].hContact);

		cfg::dat.sortOrder[0] = bSaved;

		cfg::dat.bFilterEffective |= CLVM_FILTER_LASTMSG;
        mir_snprintf(szSetting, 256, "%c%s_LM", 246, name);
        cfg::dat.lastMsgFilter = cfg::getDword(CLVM_MODULE, szSetting, 0);
		if(LOBYTE(HIWORD(cfg::dat.lastMsgFilter)))
			cfg::dat.bFilterEffective |= CLVM_FILTER_LASTMSG_NEWERTHAN;
		else
			cfg::dat.bFilterEffective |= CLVM_FILTER_LASTMSG_OLDERTHAN;
		unit = LOWORD(cfg::dat.lastMsgFilter);
		switch(HIBYTE(HIWORD(cfg::dat.lastMsgFilter))) {
			case 0:
				unit *= 60;
				break;
			case 1:
				unit *= 3600;
				break;
			case 2:
				unit *= 86400;
				break;
		}
		cfg::dat.lastMsgFilter = unit;
	}

	if(HIWORD(cfg::dat.filterFlags) > 0)
        cfg::dat.bFilterEffective |= CLVM_STICKY_CONTACTS;

    if(cfg::dat.boldHideOffline == (BYTE)-1)
        cfg::dat.boldHideOffline = cfg::getByte("CList", "HideOffline", 0);

    CallService(MS_CLIST_SETHIDEOFFLINE, 0, 0);
    SetWindowTextA(hwndSelector, name);
    pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
    CLUI::setButtonStates(pcli->hwndContactList);

    cfg::writeString(NULL, "CList", "LastViewMode", cfg::dat.current_viewmode);
}


