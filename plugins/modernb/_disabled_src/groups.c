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
*/
#include "commonheaders.h"

static int RenameGroup(WPARAM wParam,LPARAM lParam);
static int MoveGroupBefore(WPARAM wParam,LPARAM lParam);


static int CountGroups(void)
{
	DBVARIANT dbv;
	int i;
	char str[33];

	for(i=0;;i++) {
		itoa(i,str,10);
		if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
			break;
		DBFreeVariant(&dbv);
	}
	return i;
}

static int GroupNameExists(const TCHAR *name, int skipGroup)
{
	char idstr[33];
	DBVARIANT dbv;
	int i;

	for(i=0;;i++) {
		if (i == skipGroup)
			continue;
		itoa(i,idstr,10);
		if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
			break;
		if (!_tcscmp(((TCHAR*)dbv.ptszVal + 1), name)) {
			DBFreeVariant(&dbv);
			return 1;
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

static int CreateGroup(WPARAM wParam,LPARAM lParam)
{
	int newId=CountGroups();
	TCHAR newBaseName[127], newName[128];
	char str[33];
	int i;
	DBVARIANT dbv;

	if(wParam) {
		itoa(wParam-1,str,10);
		if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
			return 0;

		mir_sntprintf( newBaseName, SIZEOF(newBaseName), _T("%s\\%s"), dbv.pszVal + 1, TranslateT("New Group"));
		mir_free(dbv.pszVal);
	}
	else lstrcpyn( newBaseName, TranslateT( "New Group" ), SIZEOF( newBaseName ));

	itoa(newId,str,10);
	i=1;
	lstrcpyn( newName + 1, newBaseName, SIZEOF(newName) - 1);
	while(GroupNameExists(newName+1,-1))
		mir_sntprintf( newName + 1, SIZEOF(newName) - 1, _T("%s (%d)"), newBaseName, ++i );

	newName[0]=1|GROUPF_EXPANDED;  //1 is required so we never get '\0'
	DBWriteContactSettingTString(NULL, "CListGroups", str, newName);
	CallService(MS_CLUI_GROUPADDED,newId+1,1);
	return newId+1;
}

static int GetGroupName2(WPARAM wParam,LPARAM lParam)
{
	char idstr[33];
	DBVARIANT dbv;
	static char name[128];

	itoa(wParam-1,idstr,10);
	if(DBGetContactSettingTString(NULL,"CListGroups",idstr,&dbv))
		    return (int)(char*)NULL;
	lstrcpynA(name, dbv.pszVal + 1, SIZEOF(name));
	if ((DWORD *) lParam != NULL)
		*(DWORD *) lParam = dbv.pszVal[0];
	DBFreeVariant(&dbv);
	return (int)name;
}

TCHAR* GetGroupNameTS( int idx, DWORD* pdwFlags )
{
	char idstr[33];
	DBVARIANT dbv;
	static TCHAR name[128];

	itoa( idx-1, idstr, 10);
	if (DBGetContactSettingTString( NULL, "CListGroups", idstr, &dbv ))
		return NULL;

	lstrcpyn( name, (TCHAR*)dbv.ptszVal + 1, SIZEOF( name ));
	if ( pdwFlags != NULL )
		*pdwFlags = dbv.ptszVal[0];
	DBFreeVariant( &dbv );
	return name;
}

int GetGroupNameT(WPARAM wParam,LPARAM lParam)
{
	return (int)GetGroupNameTS ((int) wParam, (DWORD*)lParam);
}

static int GetGroupName(WPARAM wParam,LPARAM lParam)
{
	int ret;
	ret=GetGroupName2(wParam,lParam);
	if((int*)lParam)
		*(int*)lParam=0!=(*(int*)lParam&GROUPF_EXPANDED);
	return ret;
}

static int DeleteGroup(WPARAM wParam,LPARAM lParam)
{
	int i;
	char str[33];
	DBVARIANT dbv;
	HANDLE hContact;
	TCHAR name[256], szNewParent[256], *pszLastBackslash;

	//get the name
	itoa(wParam-1,str,10);
	if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
		return 1;
	if (DBGetContactSettingByte(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT))
		if (MessageBox((HWND)CallService(MS_CLUI_GETHWND, 0, 0), TranslateT("Are you sure you want to delete this group?  This operation can not be undone."), TranslateT("Delete Group"), MB_YESNO|MB_ICONQUESTION)==IDNO)
			return 1;
	lstrcpyn(name, (TCHAR*)dbv.ptszVal + 1, SIZEOF(name));
	DBFreeVariant(&dbv);
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	//must remove setting from all child contacts too
	//children are demoted to the next group up, not deleted.
	lstrcpy(szNewParent, name);
	pszLastBackslash = _tcsrchr(szNewParent, '\\');
	if (pszLastBackslash)
		pszLastBackslash[0] = '\0';
	else
		szNewParent[0] = '\0';
	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do {
		if (DBGetContactSettingTString(hContact, "CList", "Group", &dbv))
			continue;
		if (_tcscmp(dbv.ptszVal, name)) {
		DBFreeVariant(&dbv);
			continue;
		}
		DBFreeVariant(&dbv);
		if (szNewParent[0])
			DBWriteContactSettingTString(hContact, "CList", "Group", szNewParent);
		else
			DBDeleteContactSetting(hContact, "CList", "Group");
	} while((hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))!=NULL);
	//shuffle list of groups up to fill gap
	for(i=wParam-1;;i++) {
		itoa(i+1,str,10);
		if (DBGetContactSettingStringUtf(NULL, "CListGroups", str, &dbv))
			break;
		itoa(i,str,10);
		DBWriteContactSettingStringUtf(NULL, "CListGroups", str, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	itoa(i,str,10);
	DBDeleteContactSetting(NULL,"CListGroups",str);
	//rename subgroups
	{
		TCHAR szNewName[256];
		int len;

		len = lstrlen(name);
		for(i=0;;i++) {
			itoa(i,str,10);
			if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
				break;
			if (!_tcsncmp((TCHAR*)dbv.ptszVal + 1, name, len) && dbv.ptszVal[len + 1] == '\\' && _tcschr(dbv.ptszVal + len + 2, '\\') == NULL) {
				if (szNewParent[0])
					mir_sntprintf(szNewName, SIZEOF(szNewName), _T("%s\\%s"), szNewParent, dbv.ptszVal + len + 2);
				else
					lstrcpyn(szNewName, dbv.ptszVal + len + 2, SIZEOF(szNewName));
				RenameGroupT((WPARAM)(i + 1), (LPARAM)szNewName);
			}
			DBFreeVariant(&dbv);
		}
	}
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	LoadContactTree();
	return 0;
}

static int RenameGroupWithMove(int groupId,const TCHAR *szName,int move)
{
	char idstr[33];
	TCHAR str[256], oldName[256]; 
	DBVARIANT dbv;
	HANDLE hContact;

	if(GroupNameExists(szName,groupId)) {
		MessageBox(NULL,TranslateT("You already have a group with that name. Please enter a unique name for the group."),TranslateT("Rename Group"),MB_OK);
		return 1;
	}

	//do the change
	itoa(groupId,idstr,10);
	if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
		return 1;
	str[0]=dbv.ptszVal[0];
	lstrcpyn(oldName, (TCHAR*)dbv.ptszVal + 1, SIZEOF(oldName));
	DBFreeVariant(&dbv);
	{
		WORD l;
		l=lstrlen(szName)+sizeof(TCHAR);
		l=min(l,SIZEOF(str));
		l--;
		lstrcpyn(str + 1, szName, l);
		str[l+1]=_T('\0');
	}
	DBWriteContactSettingTString(NULL, "CListGroups", idstr, str);

	//must rename setting in all child contacts too
	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do {
		if (DBGetContactSettingTString(hContact, "CList", "Group", &dbv))
			continue;
		if ( _tcscmp(dbv.ptszVal, oldName))
			continue;
		DBWriteContactSettingTString(hContact, "CList", "Group", szName);
	} 
		while ((hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0)) != NULL);

	//rename subgroups
	{
		TCHAR szNewName[256];
		int len,i;

		len = lstrlen(oldName);
		for(i=0;;i++) {
			if (i == groupId)
				continue;
			itoa(i,idstr,10);
			if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
				break;
			if ( !_tcsncmp((TCHAR*)dbv.ptszVal + 1, oldName, len) && dbv.ptszVal[len + 1] == _T('\\') && _tcschr((TCHAR*)dbv.ptszVal+len + 2, _T('\\')) == NULL) {
				mir_sntprintf( szNewName, SIZEOF(szNewName), _T("%s\\%s"), szName,(TCHAR*)dbv.pszVal + len + 2 );
				RenameGroupWithMove(i,szNewName,0);	//luckily, child groups will never need reordering
			}
			DBFreeVariant(&dbv);
		}
	}

	//finally must make sure it's after any parent items
	if(move) {
		TCHAR *pszLastBackslash;
		int i;

		lstrcpyn(str, szName, SIZEOF(str));
		pszLastBackslash = _tcsrchr(str, '\\');
		if (pszLastBackslash == NULL)
			return 0;
		*pszLastBackslash='\0';
		for(i=0;;i++) {
			itoa(i,idstr,10);
			if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
				break;
			if (!lstrcmp((TCHAR*)dbv.ptszVal + 1, str)) {
				if (i < groupId)
					break;      //is OK
				MoveGroupBefore(groupId+1,i+2);
				break;
			}
			DBFreeVariant(&dbv);
		}
	}
	return 0;
}

int RenameGroupT( WPARAM groupID, LPARAM newName )
{
	return -1 != RenameGroupWithMove( (int)groupID-1, (TCHAR*)newName, 1);
}

static int RenameGroup(WPARAM wParam,LPARAM lParam)
{
	#if defined( _UNICODE )
		WCHAR* temp = mir_a2u(( char* )lParam );
		int result = ( -1 != RenameGroupWithMove(wParam - 1, temp, 1));
		mir_free( temp );
		return result;
	#else
		return -1 != RenameGroupWithMove(wParam - 1, (TCHAR*) lParam, 1);
	#endif
}

static int SetGroupExpandedState(WPARAM wParam,LPARAM lParam)
{
	char idstr[33];
	DBVARIANT dbv;

	itoa(wParam-1,idstr,10);
	if (DBGetContactSettingStringUtf(NULL, "CListGroups", idstr, &dbv))
		return 1;
	if (lParam)
		dbv.pszVal[0] |= GROUPF_EXPANDED;
	else
		dbv.pszVal[0] = dbv.pszVal[0] & ~GROUPF_EXPANDED;
	DBWriteContactSettingStringUtf(NULL, "CListGroups", idstr, dbv.pszVal);
	DBFreeVariant(&dbv);
	return 0;
}

static int SetGroupFlags(WPARAM wParam,LPARAM lParam)
{
	char idstr[33];
	DBVARIANT dbv;
	int flags,oldval,newval;

	itoa(wParam-1,idstr,10);
	if (DBGetContactSettingStringUtf(NULL, "CListGroups", idstr, &dbv))
		return 1;
	flags=LOWORD(lParam)&HIWORD(lParam);
	oldval=dbv.pszVal[0];
	newval=dbv.pszVal[0]=(dbv.pszVal[0]&~HIWORD(lParam))|flags;
	DBWriteContactSettingStringUtf(NULL, "CListGroups", idstr, dbv.pszVal);
	DBFreeVariant(&dbv);
	if ((oldval & GROUPF_HIDEOFFLINE) != (newval & GROUPF_HIDEOFFLINE))
		LoadContactTree();
	return 0;
}

static int MoveGroupBefore(WPARAM wParam,LPARAM lParam)
{
	int i,shuffleFrom,shuffleTo,shuffleDir;
	char str[33];
	TCHAR *szMoveName;
	DBVARIANT dbv;

	if (wParam == 0 || (LPARAM) wParam == lParam)
		return 0;
	itoa(wParam-1,str,10);
	if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
		return 0;
	szMoveName = dbv.ptszVal;
	//shuffle list of groups up to fill gap
	if (lParam == 0) {
		shuffleFrom = wParam - 1;
		shuffleTo = -1;
		shuffleDir = -1;
	}
	else {
		if ((LPARAM) wParam < lParam) {
			shuffleFrom = wParam - 1;
			shuffleTo = lParam - 2;
			shuffleDir = -1;
		}
	else {
			shuffleFrom = wParam - 1;
			shuffleTo = lParam - 1;
			shuffleDir = 1;
		}
	}
	if(shuffleDir==-1) {
		for(i=shuffleFrom;i!=shuffleTo;i++) {
			itoa(i+1,str,10);
			if (DBGetContactSettingStringUtf(NULL, "CListGroups", str, &dbv)) {
				shuffleTo = i;
				break;
			}
			itoa(i,str,10);
			DBWriteContactSettingStringUtf(NULL, "CListGroups", str, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	else {
		for(i=shuffleFrom;i!=shuffleTo;i--) {
			itoa(i-1,str,10);
			if (DBGetContactSettingStringUtf(NULL, "CListGroups", str, &dbv)) {
				mir_free(szMoveName);
				return 1;
			}                   //never happens
			itoa(i,str,10);
			DBWriteContactSettingStringUtf(NULL, "CListGroups", str, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	itoa(shuffleTo,str,10);
	DBWriteContactSettingTString(NULL, "CListGroups", str, szMoveName);
	mir_free(szMoveName);
	return shuffleTo+1;
}

static int BuildGroupMenu(WPARAM wParam,LPARAM lParam)
{
	char idstr[33];
	DBVARIANT dbv;
	int groupId;
	HMENU hRootMenu,hThisMenu;
	int nextMenuId=100;
	TCHAR *pBackslash,*pNextField,szThisField[128],szThisMenuItem[128];
	int menuId,compareResult,menuItemCount;
	MENUITEMINFO mii={0};

	if (DBGetContactSettingStringUtf(NULL, "CListGroups", "0", &dbv))
		return (int) (HMENU) NULL;
	DBFreeVariant(&dbv);
	hRootMenu=CreateMenu();
	for(groupId=0;;groupId++) {
		itoa(groupId,idstr,10);
		if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
			break;

		pNextField = (TCHAR*)dbv.ptszVal + 1;
		hThisMenu=hRootMenu;
		mii.cbSize=MENUITEMINFO_V4_SIZE;
		do {
			pBackslash = _tcschr(pNextField, '\\');
			if(pBackslash==NULL) {
				lstrcpyn(szThisField, pNextField, SIZEOF(szThisField));
				pNextField=NULL;
			}
			else {
				lstrcpyn(szThisField, pNextField, min( SIZEOF(szThisField), pBackslash - pNextField + 1));
				pNextField=pBackslash+1;
			}
			compareResult=1;
			menuItemCount=GetMenuItemCount(hThisMenu);
			for(menuId=0;menuId<menuItemCount;menuId++) {
				mii.fMask=MIIM_TYPE|MIIM_SUBMENU|MIIM_DATA;
				mii.cch = SIZEOF(szThisMenuItem);
				mii.dwTypeData=szThisMenuItem;
				GetMenuItemInfo(hThisMenu,menuId,TRUE,&mii);
				compareResult = lstrcmp(szThisField, szThisMenuItem);
				if(compareResult==0) {
					if(pNextField==NULL) {
						mii.fMask=MIIM_DATA;
						mii.dwItemData=groupId+1;
						SetMenuItemInfo(hThisMenu,menuId,TRUE,&mii);
					}
					else {
						if(mii.hSubMenu==NULL) {
							mii.fMask=MIIM_SUBMENU;
							mii.hSubMenu=CreateMenu();
							SetMenuItemInfo(hThisMenu,menuId,TRUE,&mii);
							mii.fMask=MIIM_DATA|MIIM_TYPE|MIIM_ID;
							//dwItemData doesn't change
							mii.fType=MFT_STRING;
							mii.dwTypeData=TranslateT("This group");
							mii.wID=nextMenuId++;
							InsertMenuItem(mii.hSubMenu,0,TRUE,&mii);
							mii.fMask=MIIM_TYPE;
							mii.fType=MFT_SEPARATOR;
							InsertMenuItem(mii.hSubMenu,1,TRUE,&mii);
						}
						hThisMenu=mii.hSubMenu;
					}
					break;
				}
				if ((int) mii.dwItemData - 1 > groupId)
					break;
			}
			if(compareResult) {
				mii.fMask=MIIM_TYPE|MIIM_ID;
				mii.wID=nextMenuId++;
				mii.dwTypeData=szThisField;
				mii.fType=MFT_STRING;
				if(pNextField) {
					mii.fMask|=MIIM_SUBMENU;
					mii.hSubMenu=CreateMenu();
				}
				else {
					mii.fMask|=MIIM_DATA;
					mii.dwItemData=groupId+1;
				}
				InsertMenuItem(hThisMenu,menuId,TRUE,&mii);
				if(pNextField) {
					hThisMenu=mii.hSubMenu;
				}
			}
		} while(pNextField);

		DBFreeVariant(&dbv);
	}
	return (int)hRootMenu;
}

int InitGroupServices(void)
{
	CreateServiceFunction(MS_CLIST_GROUPCREATE,CreateGroup);
	CreateServiceFunction(MS_CLIST_GROUPDELETE,DeleteGroup);
	CreateServiceFunction(MS_CLIST_GROUPRENAMET,RenameGroupT);
	CreateServiceFunction(MS_CLIST_GROUPGETNAME,GetGroupName);
	CreateServiceFunction(MS_CLIST_GROUPGETNAME2,GetGroupName2);
	CreateServiceFunction(MS_CLIST_GROUPGETNAMET,GetGroupNameT);
	CreateServiceFunction(MS_CLIST_GROUPSETEXPANDED,SetGroupExpandedState);
	CreateServiceFunction(MS_CLIST_GROUPSETFLAGS,SetGroupFlags);
	CreateServiceFunction(MS_CLIST_GROUPMOVEBEFORE,MoveGroupBefore);
	CreateServiceFunction(MS_CLIST_GROUPBUILDMENU,BuildGroupMenu);
	return 0;
}
