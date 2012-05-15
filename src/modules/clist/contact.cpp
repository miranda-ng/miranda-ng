/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
*/
#include "commonheaders.h"
#include "clc.h"

extern HANDLE hContactIconChangedEvent;
extern HANDLE hGroupChangeEvent;

int sortByStatus;
int sortByProto;

static const struct {
	int status,order;
} statusModeOrder[]={
	{ID_STATUS_OFFLINE,500},
	{ID_STATUS_ONLINE,10},
	{ID_STATUS_AWAY,200},
	{ID_STATUS_DND,110},
	{ID_STATUS_NA,450},
	{ID_STATUS_OCCUPIED,100},
	{ID_STATUS_FREECHAT,0},
	{ID_STATUS_INVISIBLE,20},
	{ID_STATUS_ONTHEPHONE,150},
	{ID_STATUS_OUTTOLUNCH,425}};

static int GetContactStatus(HANDLE hContact)
{
	char* szProto = ( char* )CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (szProto == NULL)
		return ID_STATUS_OFFLINE;
	return DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
}

void fnChangeContactIcon(HANDLE hContact, int iIcon, int add)
{
	CallService(add ? MS_CLUI_CONTACTADDED : MS_CLUI_CONTACTSETICON, (WPARAM) hContact, iIcon);
	NotifyEventHooks(hContactIconChangedEvent, (WPARAM) hContact, iIcon);
}

int GetStatusModeOrdering(int statusMode)
{
	int i;
	for (i = 0; i < SIZEOF(statusModeOrder); i++)
		if (statusModeOrder[i].status == statusMode)
			return statusModeOrder[i].order;
	return 1000;
}

void fnLoadContactTree(void)
{
	HANDLE hContact;
	int i, status, hideOffline;

	CallService(MS_CLUI_LISTBEGINREBUILD, 0, 0);
	for (i = 1;; i++) {
		if ( cli.pfnGetGroupName(i, NULL) == NULL)
			break;
		CallService(MS_CLUI_GROUPADDED, i, 0);
	}

	hideOffline = DBGetContactSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) {
		status = GetContactStatus(hContact);
		if ((!hideOffline || status != ID_STATUS_OFFLINE) && !DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
			cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode((char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0), status, hContact), 1);
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	sortByStatus = DBGetContactSettingByte(NULL, "CList", "SortByStatus", SETTING_SORTBYSTATUS_DEFAULT);
	sortByProto = DBGetContactSettingByte(NULL, "CList", "SortByProto", SETTING_SORTBYPROTO_DEFAULT);
	CallService(MS_CLUI_SORTLIST, 0, 0);
	CallService(MS_CLUI_LISTENDREBUILD, 0, 0);
}

int fnCompareContacts(const struct ClcContact* c1, const struct ClcContact* c2)
{
	HANDLE a = c1->hContact, b = c2->hContact;
	TCHAR namea[128], *nameb;
	int statusa, statusb;
	int rc;

	statusa = DBGetContactSettingWord((HANDLE) a, c1->proto, "Status", ID_STATUS_OFFLINE);
	statusb = DBGetContactSettingWord((HANDLE) b, c2->proto, "Status", ID_STATUS_OFFLINE);

	if (sortByProto) {
		/* deal with statuses, online contacts have to go above offline */
		if ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE)) {
			return 2 * (statusa == ID_STATUS_OFFLINE) - 1;
		}
		/* both are online, now check protocols */
		rc = lstrcmpA( c1->proto, c2->proto);
		if (rc != 0 && (c1->proto != NULL && c2->proto != NULL))
			return rc;
		/* protocols are the same, order by display name */
	}

	if (sortByStatus) {
		int ordera, orderb;
		ordera = GetStatusModeOrdering(statusa);
		orderb = GetStatusModeOrdering(statusb);
		if (ordera != orderb)
			return ordera - orderb;
	}
	else {
		//one is offline: offline goes below online
		if ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE)) {
			return 2 * (statusa == ID_STATUS_OFFLINE) - 1;
		}
	}

	nameb = cli.pfnGetContactDisplayName( a, 0);
	_tcsncpy(namea, nameb, SIZEOF(namea));
	namea[ SIZEOF(namea)-1 ] = 0;
	nameb = cli.pfnGetContactDisplayName( b, 0);

	//otherwise just compare names
	return _tcsicmp(namea, nameb);
}

static UINT_PTR resortTimerId = 0;
static VOID CALLBACK SortContactsTimer(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(NULL, resortTimerId);
	resortTimerId = 0;
	CallService(MS_CLUI_SORTLIST, 0, 0);
}

void fnSortContacts(void)
{
	//avoid doing lots of resorts in quick succession
	sortByStatus = DBGetContactSettingByte(NULL, "CList", "SortByStatus", SETTING_SORTBYSTATUS_DEFAULT);
	sortByProto = DBGetContactSettingByte(NULL, "CList", "SortByProto", SETTING_SORTBYPROTO_DEFAULT);
	if (resortTimerId)
		KillTimer(NULL, resortTimerId);
	// setting this to a higher delay causes shutdown waits.
	resortTimerId = SetTimer(NULL, 0, 500, SortContactsTimer);
}

INT_PTR ContactChangeGroup(WPARAM wParam, LPARAM lParam)
{
	CLISTGROUPCHANGE grpChg = { sizeof(CLISTGROUPCHANGE), NULL, NULL };

	CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
	if ((HANDLE) lParam == NULL)
		DBDeleteContactSetting((HANDLE) wParam, "CList", "Group");
	else {
		grpChg.pszNewName = cli.pfnGetGroupName(lParam, NULL);
		DBWriteContactSettingTString((HANDLE) wParam, "CList", "Group", grpChg.pszNewName);
	}
	CallService(MS_CLUI_CONTACTADDED, wParam,
		cli.pfnIconFromStatusMode((char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0), GetContactStatus((HANDLE) wParam), (HANDLE) wParam));

	NotifyEventHooks(hGroupChangeEvent, wParam, (LPARAM)&grpChg);
	return 0;
}

int fnSetHideOffline(WPARAM wParam, LPARAM)
{
	switch(( int )wParam ) {
	case 0:
		DBWriteContactSettingByte(NULL, "CList", "HideOffline", 0);
		break;
	case 1:
		DBWriteContactSettingByte(NULL, "CList", "HideOffline", 1);
		break;
	case -1:
		DBWriteContactSettingByte(NULL, "CList", "HideOffline",
			(BYTE) ! DBGetContactSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT));
		break;
	}
	cli.pfnLoadContactTree();
	return 0;
}
