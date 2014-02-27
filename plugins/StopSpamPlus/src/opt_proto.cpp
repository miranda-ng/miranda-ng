#include "headers.h"

struct ProtocolData
{
	char *RealName;
	int show,enabled;
};

int IsProtoIM(const PROTOACCOUNT *pa)
{
	return (CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM);
}

int FillTree(HWND hwnd)
{
	ProtocolData *PD;
	int i,n;
	PROTOACCOUNT** pa;

	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;	

	TreeView_DeleteAllItems(hwnd);

	if (CallService(MS_PROTO_ENUMACCOUNTS, (LPARAM)&n, (WPARAM)&pa))
		return FALSE;

	for ( i = 0; i < n; i++ ) {
		if (IsAccountEnabled( pa[i] )) {
			PD = ( ProtocolData* )mir_alloc( sizeof( ProtocolData ));
			PD->RealName = pa[i]->szModuleName;
			PD->enabled = IsProtoIM( pa[i]);
			PD->show = PD->enabled ? (plSets->ProtoDisabled(PD->RealName)?1:0) : 100;

			tvis.item.lParam = ( LPARAM )PD;
			tvis.item.pszText = pa[i]->tszAccountName;
			tvis.item.iImage = tvis.item.iSelectedImage = PD->show;
			TreeView_InsertItem( hwnd, &tvis );
		}
	}

	return 0;
}

INT_PTR CALLBACK ProtoDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndProto = GetDlgItem(hwnd, IDC_PROTO);

	switch (msg) 
    {

	case WM_INITDIALOG: 
		TranslateDialogDefault(hwnd);

		SetWindowLongPtr(hwndProto, GWL_STYLE, GetWindowLongPtr(hwndProto, GWL_STYLE) | TVS_NOHSCROLL);
		{
			HIMAGELIST himlCheckBoxes = ImageList_Create( GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), ILC_COLOR32|ILC_MASK, 2, 2 );
			HICON Icon;
			Icon=(HICON)LoadSkinnedIcon(SKINICON_OTHER_NOTICK);
			ImageList_AddIcon(himlCheckBoxes, Icon);
			Skin_ReleaseIcon(Icon);
			Icon=(HICON)LoadSkinnedIcon(SKINICON_OTHER_TICK);
			ImageList_AddIcon(himlCheckBoxes, Icon);
			Skin_ReleaseIcon(Icon);

			TreeView_SetImageList(hwndProto, himlCheckBoxes, TVSIL_NORMAL);
		}

		FillTree(hwndProto);
		return TRUE;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {		
		case 0: 
			if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
				
				std::ostringstream out;
				
				TVITEM tvi;
				tvi.hItem = TreeView_GetRoot(hwndProto);
				tvi.cchTextMax = 32;
				tvi.mask = TVIF_PARAM | TVIF_HANDLE;

				while ( tvi.hItem != NULL ) {
					TreeView_GetItem(hwndProto, &tvi);

					if (tvi.lParam!=0) {
						ProtocolData* ppd = ( ProtocolData* )tvi.lParam;
						if (ppd->enabled && ppd->show)
							out << ppd->RealName << " ";
					}

					tvi.hItem = TreeView_GetNextSibling(hwndProto, tvi.hItem );
				}

				plSets->DisabledProtoList=out.str();
			}
			break;

		case IDC_PROTO:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_DELETEITEMA: 
				{
					NMTREEVIEWA * pnmtv = (NMTREEVIEWA *) lParam;
					if (pnmtv && pnmtv->itemOld.lParam)
						mir_free((ProtocolData*)pnmtv->itemOld.lParam);
				}
				break;

			case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x=(short)LOWORD(GetMessagePos());
					hti.pt.y=(short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
					if ( TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti )) {
						if ( hti.flags & TVHT_ONITEMICON ) {
							TVITEMA tvi;
							tvi.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
							tvi.hItem = hti.hItem;
							TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);

							ProtocolData *pData = ( ProtocolData* )tvi.lParam;
							if ( pData->enabled ) {
								tvi.iImage = tvi.iSelectedImage = !tvi.iImage;
								pData->show = tvi.iImage;
								TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
								SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);
			}	}	}	}	}
			break;
		}
		break;
	}
	return FALSE;
}
