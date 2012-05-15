/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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
#include "genmenu.h"

#define STR_SEPARATOR _T("-----------------------------------")

extern bool bIconsDisabled;
extern int DefaultImageListColorDepth;
long handleCustomDraw(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD);
void RebuildProtoMenus( int );

struct OrderData
{
	int dragging;
	HTREEITEM hDragItem;
	int iInitMenuValue;
};

typedef struct tagMenuItemOptData
{
	TCHAR* name;
	TCHAR* defname;
	char*  uniqname;

	int pos;
	boolean show;
	DWORD isSelected;
	int id;

	PMO_IntMenuItem pimi;
}
	MenuItemOptData,*lpMenuItemOptData;

static BOOL GetCurrentMenuObjectID(HWND hwndDlg, int* result)
{
	TVITEM tvi;
	HWND hTree = GetDlgItem(hwndDlg, IDC_MENUOBJECTS);
	HTREEITEM hti = TreeView_GetSelection(hTree);
	if ( hti == NULL )
		return FALSE;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hti;
	TreeView_GetItem(hTree, &tvi);
	*result = (int)tvi.lParam;
	return TRUE;
}

static int SaveTree(HWND hwndDlg)
{
	TVITEM tvi;
	int count;
	TCHAR idstr[100];
	char menuItemName[256], DBString[256], MenuNameItems[256];
	int menupos;
	int MenuObjectId, runtimepos;
	TIntMenuObject* pimo;
	MenuItemOptData* iod;
	HWND hTree = GetDlgItem( hwndDlg, IDC_MENUITEMS );

	if ( !GetCurrentMenuObjectID( hwndDlg, &MenuObjectId ))
		return 0;

	tvi.hItem = TreeView_GetRoot( hTree );
	tvi.cchTextMax = 99;
	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
	tvi.pszText = idstr;
	count = 0;

	menupos = GetMenuObjbyId( MenuObjectId );
	if ( menupos == -1 )
		return -1;

	pimo = g_menus[menupos];

	mir_snprintf( MenuNameItems, sizeof(MenuNameItems), "%s_Items", pimo->Name);
	runtimepos = 100;

	while ( tvi.hItem != NULL ) {
		TreeView_GetItem( hTree, &tvi );
		iod = ( MenuItemOptData* )tvi.lParam;
		if ( iod->pimi ) {
			GetMenuItemName( iod->pimi, menuItemName, sizeof( menuItemName ));

			mir_snprintf( DBString, SIZEOF(DBString), "%s_visible", menuItemName );
			DBWriteContactSettingByte( NULL, MenuNameItems, DBString, iod->show );

			mir_snprintf( DBString, SIZEOF(DBString), "%s_pos", menuItemName );
			DBWriteContactSettingDword( NULL, MenuNameItems, DBString, runtimepos );

			mir_snprintf( DBString, SIZEOF(DBString), "%s_name", menuItemName );
			if ( lstrcmp( iod->name, iod->defname ) != 0 )
				DBWriteContactSettingTString( NULL, MenuNameItems, DBString, iod->name );
			else
				DBDeleteContactSetting( NULL, MenuNameItems, DBString );

			runtimepos += 100;
		}

		if ( iod->name && !_tcscmp( iod->name, STR_SEPARATOR) && iod->show )
			runtimepos += SEPARATORPOSITIONINTERVAL;

		tvi.hItem = TreeView_GetNextSibling( hTree, tvi.hItem );
		count++;
	}
	return 1;
}

static int BuildMenuObjectsTree(HWND hwndDlg)
{
	TVINSERTSTRUCT tvis;
	HWND hTree = GetDlgItem(hwndDlg,IDC_MENUOBJECTS);
	int i;

	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	TreeView_DeleteAllItems( hTree );
	if ( g_menus.getCount() == 0 )
		return FALSE;

	for ( i=0; i < g_menus.getCount(); i++ ) {
		if ( g_menus[i]->id == (int)hStatusMenuObject  || !g_menus[i]->m_bUseUserDefinedItems )
			continue;

		tvis.item.lParam  = ( LPARAM )g_menus[i]->id;
		tvis.item.pszText = LangPackPcharToTchar( g_menus[i]->Name );
		tvis.item.iImage  = tvis.item.iSelectedImage = TRUE;
		TreeView_InsertItem( hTree, &tvis );
		mir_free( tvis.item.pszText );
	}
	return 1;
}

static int sortfunc(const void *a,const void *b)
{
	lpMenuItemOptData *sd1,*sd2;
	sd1=(lpMenuItemOptData *)a;
	sd2=(lpMenuItemOptData *)b;
	if ((*sd1)->pos > (*sd2)->pos)
		return 1;

	if ((*sd1)->pos < (*sd2)->pos)
		return -1;

	return 0;
}

static int InsertSeparator(HWND hwndDlg)
{
	MenuItemOptData *PD;
	TVINSERTSTRUCT tvis = {0};
	TVITEM tvi = {0};
	HTREEITEM hti = {0};
	HWND hMenuTree = GetDlgItem( hwndDlg, IDC_MENUITEMS );

	hti = TreeView_GetSelection( hMenuTree );
	if ( hti == NULL )
		return 1;

	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
	tvi.hItem = hti;
	if ( TreeView_GetItem( hMenuTree, &tvi) == FALSE )
		return 1;

	PD = ( MenuItemOptData* )mir_alloc( sizeof( MenuItemOptData ));
	ZeroMemory( PD, sizeof( MenuItemOptData ));
	PD->id   = -1;
	PD->name = mir_tstrdup( STR_SEPARATOR );
	PD->show = TRUE;
	PD->pos  = ((MenuItemOptData *)tvi.lParam)->pos-1;

	tvis.item.lParam = (LPARAM)(PD);
	tvis.item.pszText = PD->name;
	tvis.item.iImage = tvis.item.iSelectedImage = PD->show;
	tvis.hParent = NULL;
	tvis.hInsertAfter = hti;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	TreeView_InsertItem( hMenuTree, &tvis );
	return 1;
}

static void FreeTreeData( HWND hwndDlg )
{
	HTREEITEM hItem = TreeView_GetRoot(GetDlgItem(hwndDlg,IDC_MENUITEMS));
	while ( hItem != NULL ) {
		TVITEM tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hItem;
		TreeView_GetItem( GetDlgItem( hwndDlg, IDC_MENUITEMS ), &tvi );
		{	MenuItemOptData* O = (MenuItemOptData *)tvi.lParam;
			if ( O->name ) mir_free( O->name );
			if ( O->defname ) mir_free( O->defname );
			if ( O->uniqname ) mir_free( O->uniqname );
			mir_free( O );
		}

		tvi.lParam = 0;
		TreeView_SetItem(GetDlgItem(hwndDlg,IDC_MENUITEMS), &tvi);

		hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg,IDC_MENUITEMS), hItem);
}	}

static int BuildTree(HWND hwndDlg,int MenuObjectId, BOOL bReread)
{
	char menuItemName[256],MenuNameItems[256];
	char buf[256];

	FreeTreeData( hwndDlg );
	TreeView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_MENUITEMS));

	int menupos = GetMenuObjbyId( MenuObjectId );
	if ( menupos == -1 )
		return FALSE;

	TIntMenuObject* pimo = g_menus[menupos];
	if ( pimo->m_items.first == NULL )
		return FALSE;

	mir_snprintf( MenuNameItems, sizeof(MenuNameItems), "%s_Items", pimo->Name );

	int count = 0;
	{	
		for ( PMO_IntMenuItem p = pimo->m_items.first; p != NULL; p = p->next )
			if ( p->mi.root == ( HGENMENU )-1 || p->mi.root == NULL )
				count++;
	}

	lpMenuItemOptData *PDar = ( lpMenuItemOptData* )mir_alloc( sizeof( lpMenuItemOptData )*count );

	count = 0;
	{
		for ( PMO_IntMenuItem p = pimo->m_items.first; p != NULL; p = p->next ) {
			if ( p->mi.root != ( HGENMENU )-1 && p->mi.root != NULL )
				continue;

			MenuItemOptData *PD = ( MenuItemOptData* )mir_calloc( sizeof( MenuItemOptData ));
			GetMenuItemName( p, menuItemName, sizeof( menuItemName ));
			{
				DBVARIANT dbv;
				mir_snprintf(buf, SIZEOF(buf), "%s_name", menuItemName);

				if ( !DBGetContactSettingTString( NULL, MenuNameItems, buf, &dbv )) {
					PD->name = mir_tstrdup( dbv.ptszVal );
					DBFreeVariant( &dbv );
				}
				else PD->name = mir_tstrdup( p->mi.ptszName );
			}

			PD->pimi = p;
			PD->defname = mir_tstrdup( p->mi.ptszName );

			mir_snprintf( buf, SIZEOF(buf), "%s_visible", menuItemName );
			PD->show = DBGetContactSettingByte( NULL, MenuNameItems, buf, 1 );

			if ( bReread ) {
				mir_snprintf( buf, SIZEOF(buf), "%s_pos", menuItemName );
				PD->pos = DBGetContactSettingDword( NULL, MenuNameItems, buf, 1 );
			}
			else PD->pos = ( PD->pimi ) ? PD->pimi->originalPosition : 0;

			PD->id = p->iCommand;

			if ( p->UniqName )
				PD->uniqname = mir_strdup( p->UniqName );

			PDar[ count ] = PD;
			count++;
	}	}

	qsort( PDar, count, sizeof( lpMenuItemOptData ), sortfunc );

	SendDlgItemMessage(hwndDlg, IDC_MENUITEMS, WM_SETREDRAW, FALSE, 0);
	int lastpos = 0;
	bool first = TRUE;

	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	for ( int i=0; i < count; i++ ) {
		if ( PDar[i]->pos - lastpos >= SEPARATORPOSITIONINTERVAL ) {
			MenuItemOptData *PD = ( MenuItemOptData* )mir_calloc( sizeof( MenuItemOptData ));
			PD->id = -1;
			PD->name = mir_tstrdup( STR_SEPARATOR );
			PD->pos = PDar[i]->pos - 1;
			PD->show = TRUE;

			tvis.item.lParam = ( LPARAM )PD;
			tvis.item.pszText = PD->name;
			tvis.item.iImage = tvis.item.iSelectedImage = PD->show;
			SendDlgItemMessage(hwndDlg, IDC_MENUITEMS, TVM_INSERTITEM, 0, (LPARAM)&tvis);
		}

		tvis.item.lParam = ( LPARAM )PDar[i];
		tvis.item.pszText = PDar[i]->name;
		tvis.item.iImage = tvis.item.iSelectedImage = PDar[i]->show;
		
		HTREEITEM hti = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_MENUITEMS, TVM_INSERTITEM, 0, (LPARAM)&tvis);
		if ( first ) {
			TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_MENUITEMS),hti);
			first=FALSE;
		}

		lastpos = PDar[i]->pos;
	}

	SendDlgItemMessage( hwndDlg, IDC_MENUITEMS, WM_SETREDRAW, TRUE, 0 );
	mir_free( PDar );
	ShowWindow( GetDlgItem( hwndDlg, IDC_NOTSUPPORTWARNING ),( pimo->m_bUseUserDefinedItems ) ? SW_HIDE : SW_SHOW );
	EnableWindow( GetDlgItem( hwndDlg, IDC_MENUITEMS ), pimo->m_bUseUserDefinedItems );
	EnableWindow( GetDlgItem( hwndDlg, IDC_INSERTSEPARATOR ), pimo->m_bUseUserDefinedItems );
	return 1;
}

static void RebuildCurrent( HWND hwndDlg )
{
	int MenuObjectID;
	if ( GetCurrentMenuObjectID( hwndDlg, &MenuObjectID ))
		BuildTree( hwndDlg, MenuObjectID, TRUE );
}

static void ResetMenuItems( HWND hwndDlg )
{
	int MenuObjectID;
	if ( GetCurrentMenuObjectID( hwndDlg, &MenuObjectID ))
		BuildTree( hwndDlg, MenuObjectID, FALSE );
}

static HTREEITEM MoveItemAbove(HWND hTreeWnd, HTREEITEM hItem, HTREEITEM hInsertAfter)
{
	TVITEM tvi = { 0 };
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	if ( !SendMessage(hTreeWnd, TVM_GETITEM, 0, ( LPARAM )&tvi ))
		return NULL;
	if ( hItem && hInsertAfter ) {
		TVINSERTSTRUCT tvis;
		TCHAR name[128];
		if ( hItem == hInsertAfter )
			return hItem;

		tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvis.item.stateMask = 0xFFFFFFFF;
		tvis.item.pszText = name;
		tvis.item.cchTextMax = sizeof( name );
		tvis.item.hItem = hItem;
		tvis.item.iImage = tvis.item.iSelectedImage = (( MenuItemOptData* )tvi.lParam)->show;
		if(!SendMessage(hTreeWnd, TVM_GETITEM, 0, (LPARAM)&tvis.item))
			return NULL;
		if (!TreeView_DeleteItem(hTreeWnd,hItem))
			return NULL;
		tvis.hParent=NULL;
		tvis.hInsertAfter=hInsertAfter;
		return TreeView_InsertItem(hTreeWnd, &tvis);
	}
	return NULL;
}

WNDPROC MyOldWindowProc=NULL;

LRESULT CALLBACK LBTNDOWNProc(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg==WM_LBUTTONDOWN && !(GetKeyState(VK_CONTROL)&0x8000)) {

		TVHITTESTINFO hti;
		hti.pt.x=(short)LOWORD(lParam);
		hti.pt.y=(short)HIWORD(lParam);
		//		ClientToScreen(hwndDlg,&hti.pt);
		//		ScreenToClient(GetDlgItem(hwndDlg,IDC_MENUITEMS),&hti.pt);
		TreeView_HitTest(hwnd,&hti);
		if (hti.flags&TVHT_ONITEMLABEL) {
			/// LabelClicked Set/unset selection
			TVITEM tvi;
			HWND tvw=hwnd;
			tvi.mask=TVIF_HANDLE|TVIF_PARAM;
			tvi.hItem=hti.hItem;
			TreeView_GetItem( tvw, &tvi );

			if (!((MenuItemOptData *)tvi.lParam)->isSelected) { /* is not Selected*/
				// reset all selection except current
				HTREEITEM hit;
				hit=TreeView_GetRoot(tvw);
				if (hit)
					do {
						TVITEM tvi={0};
						tvi.mask=TVIF_HANDLE|TVIF_PARAM;
						tvi.hItem=hit;
						TreeView_GetItem(tvw, &tvi);

						if (hti.hItem!=hit)
							((MenuItemOptData *)tvi.lParam)->isSelected=0;
						else
							((MenuItemOptData *)tvi.lParam)->isSelected=1;
						TreeView_SetItem(tvw, &tvi);
					}
						while (hit=TreeView_GetNextSibling(tvw,hit));
	}	}	}

	return CallWindowProc(MyOldWindowProc,hwnd,uMsg,wParam,lParam);
}

static INT_PTR CALLBACK GenMenuOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct OrderData *dat = (struct OrderData*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_MENUITEMS),GWLP_USERDATA);
	LPNMHDR hdr;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		dat=(struct OrderData*)mir_alloc(sizeof(struct OrderData));
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_MENUITEMS),GWLP_USERDATA,(LONG_PTR)dat);
		dat->dragging = 0;
		dat->iInitMenuValue = DBGetContactSettingByte( NULL, "CList", "MoveProtoMenus", FALSE );
		MyOldWindowProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_MENUITEMS),GWLP_WNDPROC);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_MENUITEMS),GWLP_WNDPROC,(LONG_PTR)&LBTNDOWNProc);
		{
			HIMAGELIST himlCheckBoxes;
			himlCheckBoxes=ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
				(IsWinVerXPPlus() ? ILC_COLOR32 : ILC_COLOR16) | ILC_MASK, 2, 2);

			ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_NOTICK);
			ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_TICK);

			TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_MENUOBJECTS),himlCheckBoxes,TVSIL_NORMAL);
			TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_MENUITEMS),himlCheckBoxes,TVSIL_NORMAL);
		}
		CheckDlgButton(hwndDlg, dat->iInitMenuValue ? IDC_RADIO2 : IDC_RADIO1, TRUE );
		CheckDlgButton(hwndDlg, IDC_DISABLEMENUICONS, bIconsDisabled );
		BuildMenuObjectsTree(hwndDlg);
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD(wParam) == BN_CLICKED || HIWORD( wParam ) == BN_DBLCLK ) {
			switch ( LOWORD( wParam )) {
			case IDC_INSERTSEPARATOR:
				InsertSeparator(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_RESETMENU:
				ResetMenuItems( hwndDlg );
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;

			case IDC_DISABLEMENUICONS:
			case IDC_RADIO1:
			case IDC_RADIO2:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;

			case IDC_GENMENU_DEFAULT:
				{
					TVITEM tvi;
					HTREEITEM hti;
					MenuItemOptData *iod;

					hti=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_MENUITEMS));
					if (hti==NULL)
						break;

					tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
					tvi.hItem=hti;
					TreeView_GetItem(GetDlgItem(hwndDlg,IDC_MENUITEMS),&tvi);
					iod = ( MenuItemOptData * )tvi.lParam;

					if ( iod->name && _tcsstr( iod->name, STR_SEPARATOR ))
						break;

					if (iod->name)
						mir_free(iod->name);
					iod->name = mir_tstrdup( iod->defname );

					SaveTree(hwndDlg);
					RebuildCurrent(hwndDlg);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
				break;

			case IDC_GENMENU_SET:
				{
					TVITEM tvi;
					TCHAR buf[256];
					MenuItemOptData *iod;

					HTREEITEM hti = TreeView_GetSelection( GetDlgItem( hwndDlg,IDC_MENUITEMS ));
					if ( hti == NULL )
						break;

					tvi.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
					tvi.hItem = hti;
					SendDlgItemMessage(hwndDlg, IDC_MENUITEMS, TVM_GETITEM, 0, (LPARAM)&tvi);
					iod = ( MenuItemOptData * )tvi.lParam;

					if ( iod->name && _tcsstr(iod->name, STR_SEPARATOR ))
						break;

					ZeroMemory(buf,sizeof( buf ));
					GetDlgItemText( hwndDlg, IDC_GENMENU_CUSTOMNAME, buf, SIZEOF( buf ));
					if (iod->name)
						mir_free(iod->name);

					iod->name = mir_tstrdup(buf);

					SaveTree(hwndDlg);
					RebuildCurrent(hwndDlg);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
				break;
		}	}
		break;

	case WM_NOTIFY:
		hdr = (LPNMHDR)lParam;
		switch( hdr->idFrom ) {
		case 0:
			if (hdr->code == PSN_APPLY ) {
				bIconsDisabled = IsDlgButtonChecked(hwndDlg, IDC_DISABLEMENUICONS) != 0;
				DBWriteContactSettingByte(NULL, "CList", "DisableMenuIcons", bIconsDisabled);
				SaveTree(hwndDlg);
				int iNewMenuValue = IsDlgButtonChecked(hwndDlg, IDC_RADIO1) ? 0 : 1;
				if ( iNewMenuValue != dat->iInitMenuValue ) {
					RebuildProtoMenus( iNewMenuValue );
					dat->iInitMenuValue = iNewMenuValue;
				}
				RebuildCurrent(hwndDlg);
			}
			break;

		case IDC_MENUOBJECTS:
			if (hdr->code == TVN_SELCHANGEDA )
				RebuildCurrent( hwndDlg );
			break;

		case IDC_MENUITEMS:
			switch (hdr->code) {
			case NM_CUSTOMDRAW:
				{
					int i= handleCustomDraw(GetDlgItem(hwndDlg,IDC_MENUITEMS),(LPNMTVCUSTOMDRAW) lParam);
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, i);
					return TRUE;
				}

			case TVN_BEGINDRAGA:
				SetCapture(hwndDlg);
				dat->dragging=1;
				dat->hDragItem=((LPNMTREEVIEW)lParam)->itemNew.hItem;
				TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_MENUITEMS),dat->hDragItem);
				break;

			case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x=(short)LOWORD(GetMessagePos());
					hti.pt.y=(short)HIWORD(GetMessagePos());
					ScreenToClient(hdr->hwndFrom,&hti.pt);
					if (TreeView_HitTest(hdr->hwndFrom,&hti)) {
						if (hti.flags&TVHT_ONITEMICON) {
							TVITEM tvi;
							tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
							tvi.hItem=hti.hItem;
							TreeView_GetItem(hdr->hwndFrom,&tvi);

							tvi.iImage=tvi.iSelectedImage=!tvi.iImage;
							((MenuItemOptData *)tvi.lParam)->show=tvi.iImage;
							TreeView_SetItem(hdr->hwndFrom,&tvi);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

							//all changes take effect in runtime
							//ShowWindow(GetDlgItem(hwndDlg,IDC_BUTTONORDERTREEWARNING),SW_SHOW);
						}
						/*--------MultiSelection----------*/
						if (hti.flags&TVHT_ONITEMLABEL) {
							/// LabelClicked Set/unset selection
							TVITEM tvi;
							HWND tvw=hdr->hwndFrom;
							tvi.mask=TVIF_HANDLE|TVIF_PARAM;
							tvi.hItem=hti.hItem;
							TreeView_GetItem(tvw,&tvi);
							if (GetKeyState(VK_CONTROL)&0x8000) {
								if (((MenuItemOptData *)tvi.lParam)->isSelected)
									((MenuItemOptData *)tvi.lParam)->isSelected=0;
								else
									((MenuItemOptData *)tvi.lParam)->isSelected=1;  //current selection order++.
								TreeView_SetItem(tvw,&tvi);
							}
							else if (GetKeyState(VK_SHIFT)&0x8000) {
								;  // shifted click
							}
							else {
								// reset all selection except current
								HTREEITEM hit;
								hit=TreeView_GetRoot(tvw);
								if (hit)
									do {
										TVITEM tvi={0};
										tvi.mask=TVIF_HANDLE|TVIF_PARAM;
										tvi.hItem=hit;
										TreeView_GetItem(tvw,&tvi);

										if (hti.hItem!=hit)
											((MenuItemOptData *)tvi.lParam)->isSelected=0;
										else
											((MenuItemOptData *)tvi.lParam)->isSelected=1;
										TreeView_SetItem(tvw,&tvi);
									}
										while (hit=TreeView_GetNextSibling(tvw,hit));
					}	}	}
					break;
				}
			case TVN_SELCHANGING:
				{
					LPNMTREEVIEW pn;
					pn = (LPNMTREEVIEW) lParam;
					//((MenuItemOptData *)(pn->itemNew.lParam))->isSelected=1;
					/*if (pn->action==NotKeyPressed)
					{
					remove all selection
					}
					*/
				}
			case TVN_SELCHANGEDA:
				{
					TVITEM tvi;
					HTREEITEM hti;
					MenuItemOptData *iod;

					SetDlgItemTextA(hwndDlg,IDC_GENMENU_CUSTOMNAME,"");
					SetDlgItemTextA(hwndDlg,IDC_GENMENU_SERVICE,"");

					EnableWindow(GetDlgItem(hwndDlg,IDC_GENMENU_CUSTOMNAME),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_GENMENU_DEFAULT),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_GENMENU_SET),FALSE);

					hti=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_MENUITEMS));
					if (hti==NULL)
						break;

					tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
					tvi.hItem=hti;
					TreeView_GetItem(GetDlgItem(hwndDlg,IDC_MENUITEMS),&tvi);

					if ( tvi.lParam == 0 )
						break;

					iod = ( MenuItemOptData * )tvi.lParam;

					if ( iod->name && _tcsstr(iod->name, STR_SEPARATOR))
						break;

					SetDlgItemText(hwndDlg,IDC_GENMENU_CUSTOMNAME,iod->name);

					if (iod->pimi->submenu.first == NULL && iod->uniqname)
						SetDlgItemTextA(hwndDlg, IDC_GENMENU_SERVICE, iod->uniqname);

					EnableWindow(GetDlgItem(hwndDlg,IDC_GENMENU_DEFAULT), lstrcmp(iod->name, iod->defname) != 0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_GENMENU_SET),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_GENMENU_CUSTOMNAME),TRUE);
					break;
				}
				break;
		}	}
		break;

	case WM_MOUSEMOVE:
		if (!dat||!dat->dragging) break;
		{
			TVHITTESTINFO hti;

			hti.pt.x=(short)LOWORD(lParam);
			hti.pt.y=(short)HIWORD(lParam);
			ClientToScreen(hwndDlg,&hti.pt);
			ScreenToClient(GetDlgItem(hwndDlg,IDC_MENUITEMS),&hti.pt);
			TreeView_HitTest(GetDlgItem(hwndDlg,IDC_MENUITEMS),&hti);
			if (hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
				HTREEITEM it = hti.hItem;
				hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_MENUITEMS))/2;
				TreeView_HitTest(GetDlgItem(hwndDlg,IDC_MENUITEMS),&hti);
				if (!(hti.flags&TVHT_ABOVE))
					TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_MENUITEMS),hti.hItem,1);
				else
					TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_MENUITEMS),it,0);
			}
			else {
				if (hti.flags&TVHT_ABOVE) SendDlgItemMessage(hwndDlg,IDC_MENUITEMS,WM_VSCROLL,MAKEWPARAM(SB_LINEUP,0),0);
				if (hti.flags&TVHT_BELOW) SendDlgItemMessage(hwndDlg,IDC_MENUITEMS,WM_VSCROLL,MAKEWPARAM(SB_LINEDOWN,0),0);
				TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_MENUITEMS),NULL,0);
		}	}
		break;

	case WM_LBUTTONUP:
		if (!dat->dragging)
			break;

		TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_MENUITEMS),NULL,0);
		dat->dragging=0;
		ReleaseCapture();
		{
			TVHITTESTINFO hti;
			hti.pt.x=(short)LOWORD(lParam);
			hti.pt.y=(short)HIWORD(lParam);
			ClientToScreen(hwndDlg,&hti.pt);
			ScreenToClient(GetDlgItem(hwndDlg,IDC_MENUITEMS),&hti.pt);
			hti.pt.y-=TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_MENUITEMS))/2;
			TreeView_HitTest(GetDlgItem(hwndDlg,IDC_MENUITEMS),&hti);
			if (hti.flags&TVHT_ABOVE) hti.hItem=TVI_FIRST;
			if (dat->hDragItem==hti.hItem) break;
			dat->hDragItem=NULL;
			if (hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)||(hti.hItem==TVI_FIRST)) {
				HWND tvw;
				HTREEITEM * pSIT;
				HTREEITEM FirstItem=NULL;
				UINT uITCnt,uSic ;
				tvw=GetDlgItem(hwndDlg,IDC_MENUITEMS);
				uITCnt=TreeView_GetCount(tvw);
				uSic=0;
				if (uITCnt) {
					pSIT=(HTREEITEM *)mir_alloc(sizeof(HTREEITEM)*uITCnt);
					if (pSIT) {
						HTREEITEM hit;
						hit=TreeView_GetRoot(tvw);
						if (hit)
							do {
								TVITEM tvi={0};
								tvi.mask=TVIF_HANDLE|TVIF_PARAM;
								tvi.hItem=hit;
								TreeView_GetItem(tvw,&tvi);
								if (((MenuItemOptData *)tvi.lParam)->isSelected) {
									pSIT[uSic]=tvi.hItem;

									uSic++;
								}
							}while (hit=TreeView_GetNextSibling(tvw,hit));
						// Proceed moving
						{
							UINT i;
							HTREEITEM insertAfter;
							insertAfter=hti.hItem;
							for (i=0; i<uSic; i++) {
								if (insertAfter) insertAfter=MoveItemAbove(tvw,pSIT[i],insertAfter);
								else break;
								if (!i) FirstItem=insertAfter;
						}	}
						// free pointers...
						mir_free(pSIT);
				}	}

				if (FirstItem) TreeView_SelectItem(tvw,FirstItem);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				SaveTree(hwndDlg);
		}	}
		break;

	case WM_DESTROY:
		if ( dat )
			mir_free( dat );

		ImageList_Destroy(TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_MENUOBJECTS),NULL,TVSIL_NORMAL));
		FreeTreeData( hwndDlg );
		break;

	}
	return FALSE;
}

long handleCustomDraw(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD)
{
	if ( pNMTVCD == NULL )
		return -1;

	switch ( pNMTVCD->nmcd.dwDrawStage ) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			HTREEITEM hItem = (HTREEITEM) pNMTVCD->nmcd.dwItemSpec;
			TCHAR buf[255];
			TVITEM tvi = {0};
			int k=0;
			tvi.mask = TVIF_HANDLE |TVIF_PARAM|TVIS_SELECTED|TVIF_TEXT|TVIF_IMAGE;
			tvi.stateMask=TVIS_SELECTED;
			tvi.hItem = hItem;
			tvi.pszText=(LPTSTR)(&buf);
			tvi.cchTextMax=254;
			TreeView_GetItem(hWndTreeView, &tvi);
			if (((MenuItemOptData *)tvi.lParam)->isSelected) {
				pNMTVCD->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
				pNMTVCD->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT);
			}
			else {
				pNMTVCD->clrTextBk = GetSysColor(COLOR_WINDOW);
				pNMTVCD->clrText   = GetSysColor(COLOR_WINDOWTEXT);
			}

			/* At this point, you can change the background colors for the item
			and any subitems and return CDRF_NEWFONT. If the list-view control
			is in report mode, you can simply return CDRF_NOTIFYSUBITEMREDRAW
			to customize the item's subitems individually */
			if ( tvi.iImage == -1 ) {
				HBRUSH br;
				SIZE sz;
				RECT rc;
				k=1;

				GetTextExtentPoint32(pNMTVCD->nmcd.hdc,tvi.pszText,lstrlen(tvi.pszText),&sz);

				if (sz.cx+3>pNMTVCD->nmcd.rc.right-pNMTVCD->nmcd.rc.left) rc=pNMTVCD->nmcd.rc;
				else SetRect(&rc,pNMTVCD->nmcd.rc.left,pNMTVCD->nmcd.rc.top,pNMTVCD->nmcd.rc.left+sz.cx+3,pNMTVCD->nmcd.rc.bottom);

				br=CreateSolidBrush(pNMTVCD->clrTextBk);
				SetTextColor(pNMTVCD->nmcd.hdc,pNMTVCD->clrText);
				SetBkColor(pNMTVCD->nmcd.hdc,pNMTVCD->clrTextBk);
				FillRect(pNMTVCD->nmcd.hdc,&rc,br);
				DeleteObject(br);
				DrawText(pNMTVCD->nmcd.hdc,tvi.pszText,lstrlen(tvi.pszText),&pNMTVCD->nmcd.rc,DT_LEFT|DT_VCENTER|DT_NOPREFIX);
			}

			return CDRF_NEWFONT|(k?CDRF_SKIPDEFAULT:0);
		}
	}
	return 0;
}

INT_PTR CALLBACK ProtocolOrderOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int GenMenuOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize=sizeof(odp);
	odp.hInstance = hMirandaInst;
	odp.pszGroup = LPGEN("Customize");

	odp.position = -1000000000;
	odp.pszTemplate = MAKEINTRESOURCEA( IDD_OPT_GENMENU );
	odp.pszTitle = LPGEN("Menus");
	odp.pfnDlgProc = GenMenuOpts;
	odp.flags = ODPF_BOLDGROUPS;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.position = -10000000;
	odp.groupPosition = 1000000;
	odp.pszTemplate = MAKEINTRESOURCEA( IDD_OPT_PROTOCOLORDER );
	odp.pszTitle = LPGEN("Accounts");
	odp.pfnDlgProc = ProtocolOrderOpts;
	odp.flags = ODPF_BOLDGROUPS|ODPF_EXPERTONLY;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	return 0;
}
