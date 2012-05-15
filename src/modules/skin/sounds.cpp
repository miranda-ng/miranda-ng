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

struct SoundItem {
	char*  name;
	TCHAR* section;
	TCHAR* description;
	char*  tempFile;
};

static BOOL bModuleInitialized = FALSE;
static struct SoundItem *soundList = NULL;
static int soundCount;
static HANDLE hPlayEvent = NULL;

static INT_PTR ServiceSkinAddNewSound(WPARAM, LPARAM lParam)
{
	SKINSOUNDDESCEX *ssd = ( SKINSOUNDDESCEX* )lParam;
	switch( ssd->cbSize ) {
	case sizeof( SKINSOUNDDESCEX ):
	case SKINSOUNDDESC_SIZE_V1:
	case SKINSOUNDDESC_SIZE_V2:
		break;

	default:
		return 1;
	}

	if ( ssd->pszName == NULL || ssd->pszDescription == NULL)
		return 1;

	DBVARIANT dbv;
	DWORD dwFlags = ( ssd->cbSize == sizeof(SKINSOUNDDESCEX)) ? ssd->dwFlags : 0;

	soundList=(struct SoundItem*)mir_realloc(soundList,sizeof(struct SoundItem)*(soundCount+1));
	SoundItem* item = &soundList[soundCount++];
	item->name = mir_strdup( ssd->pszName );
	item->tempFile = NULL;
	#if defined( _UNICODE )
		TCHAR* ptszDefaultFile;
		if ( dwFlags & SSDF_UNICODE ) {
			item->description = mir_tstrdup( TranslateTS( ssd->ptszDescription ));
			item->section = mir_tstrdup( TranslateTS( ssd->cbSize != SKINSOUNDDESC_SIZE_V1 && ssd->pszSection != NULL ? ssd->ptszSection : _T("Other")));
			ptszDefaultFile = mir_tstrdup( ssd->ptszDefaultFile );
		}
		else {
			item->description = LangPackPcharToTchar( ssd->pszDescription );
			item->section = LangPackPcharToTchar( ssd->cbSize != SKINSOUNDDESC_SIZE_V1 && ssd->pszSection != NULL ? ssd->pszSection : "Other" );
			ptszDefaultFile = mir_a2t( ssd->pszDefaultFile );
		}

		if ( ptszDefaultFile ) {
			if ( DBGetContactSettingString(NULL, "SkinSounds", item->name, &dbv))
				DBWriteContactSettingTString(NULL, "SkinSounds", item->name, ptszDefaultFile);
			else
				DBFreeVariant(&dbv);
			mir_free( ptszDefaultFile );
		}
	#else
		item->description = mir_tstrdup( TranslateTS( ssd->pszDescription ));
		item->section = mir_tstrdup( TranslateTS( ssd->cbSize != SKINSOUNDDESC_SIZE_V1 && ssd->pszSection != NULL ? ssd->pszSection : "Other" ));
		if ( ssd->pszDefaultFile ) {
			if ( DBGetContactSettingString(NULL, "SkinSounds", item->name, &dbv))
				DBWriteContactSettingString(NULL, "SkinSounds", item->name, ssd->pszDefaultFile);
			else
				DBFreeVariant(&dbv);
		}
	#endif
	return 0;
}

static int SkinPlaySoundDefault(WPARAM wParam, LPARAM lParam)
{
	char * pszFile = (char *) lParam;
	if ( pszFile && (DBGetContactSettingByte(NULL,"Skin","UseSound",0) || (int)wParam==1))
		PlaySoundA(pszFile, NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);

	return 0;
}

static INT_PTR ServiceSkinPlaySound(WPARAM, LPARAM lParam)
{
	char* pszSoundName = ( char* )lParam;
	int j;

	for (j=0; j<soundCount; j++) {
		if ( pszSoundName && strcmp( soundList[j].name, pszSoundName ) == 0) {
			if (DBGetContactSettingByte(NULL, "SkinSoundsOff", pszSoundName, 0)==0) {
				DBVARIANT dbv;

				if (DBGetContactSettingString(NULL, "SkinSounds", pszSoundName, &dbv)==0) {
					char szFull[MAX_PATH];

					pathToAbsolute(dbv.pszVal, szFull, NULL);
					NotifyEventHooks(hPlayEvent, 0, (LPARAM)szFull);
					DBFreeVariant(&dbv);
				}
			}
			return 0;
		}
	}
	return 1;
}

static HTREEITEM FindNamedTreeItemAtRoot(HWND hwndTree, const TCHAR* name)
{
	TVITEM tvi;
	TCHAR  str[128];

	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = SIZEOF(str);
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while( tvi.hItem != NULL ) {
		TreeView_GetItem( hwndTree, &tvi );
		if ( !_tcsicmp( str, name ))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling( hwndTree, tvi.hItem );
	}
	return NULL;
}

#define DM_REBUILD_STREE (WM_USER+1)
#define DM_HIDEPANE      (WM_USER+2)
#define DM_SHOWPANE      (WM_USER+3)
#define DM_CHECKENABLED  (WM_USER+4)
INT_PTR CALLBACK DlgProcSoundOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndTree = NULL;
	switch (msg) {
	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(hwndTree, TVSIL_STATE));
		break;
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hwndTree = GetDlgItem(hwndDlg, IDC_SOUNDTREE);
		SetWindowLongPtr(hwndTree,GWL_STYLE,GetWindowLongPtr(hwndTree,GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		SendMessage(hwndDlg, DM_HIDEPANE, 0, 0);
		SendMessage(hwndDlg, DM_REBUILD_STREE, 0, 0);
		TreeView_SetItemState(hwndTree, 0, TVIS_SELECTED, TVIS_SELECTED);
		CheckDlgButton(hwndDlg, IDC_ENABLESOUNDS, DBGetContactSettingByte(NULL, "Skin", "UseSound", 0));
		SendMessage(hwndDlg, DM_CHECKENABLED, 0, 0);
		return TRUE;

	case DM_REBUILD_STREE:
		TreeView_SelectItem(hwndTree, NULL);
		ShowWindow(hwndTree, SW_HIDE);
		TreeView_DeleteAllItems(hwndTree);
		{
			TVINSERTSTRUCT tvis;
			int i;
			tvis.hParent = NULL;
			tvis.hInsertAfter = TVI_SORT;
			tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
			tvis.item.state = tvis.item.stateMask = TVIS_EXPANDED;
			for( i=0; i < soundCount; i++ ) {
				tvis.item.stateMask = TVIS_EXPANDED;
				tvis.item.state = TVIS_EXPANDED;
				tvis.hParent = FindNamedTreeItemAtRoot( hwndTree, soundList[i].section );
				if ( tvis.hParent == NULL ) {
					tvis.item.lParam = -1;
					tvis.item.pszText = soundList[i].section;
					tvis.hParent = tvis.item.hItem = TreeView_InsertItem( hwndTree, &tvis );
					tvis.item.stateMask = TVIS_STATEIMAGEMASK;
					tvis.item.state = INDEXTOSTATEIMAGEMASK(0);
					TreeView_SetItem( hwndTree, &tvis.item );
				}
				tvis.item.stateMask = TVIS_STATEIMAGEMASK;
				tvis.item.state = INDEXTOSTATEIMAGEMASK(!DBGetContactSettingByte(NULL,"SkinSoundsOff",soundList[i].name,0)?2:1);
				tvis.item.lParam = i;
				tvis.item.pszText = soundList[i].description;
				TreeView_InsertItem( hwndTree, &tvis );
		}	}
		{	TVITEM tvi;
			tvi.hItem = TreeView_GetRoot(hwndTree);
			while ( tvi.hItem != NULL ) {
				tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
				TreeView_GetItem(hwndTree, &tvi);
				if ( tvi.lParam == -1 )
					TreeView_SetItemState(hwndTree, tvi.hItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);

				tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
		}	}

		ShowWindow(hwndTree, SW_SHOW);
		break;

	case DM_HIDEPANE:
		ShowWindow(GetDlgItem(hwndDlg, IDC_SGROUP), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_NAME), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_NAMEVAL), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_SLOC), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOCATION), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHANGE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_GETMORE), SW_HIDE);
		break;

	case DM_SHOWPANE:
		ShowWindow(GetDlgItem(hwndDlg, IDC_SGROUP), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_NAME), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_NAMEVAL), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_SLOC), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOCATION), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHANGE), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_GETMORE), SW_SHOW);
		break;

	case DM_CHECKENABLED:
		EnableWindow(GetDlgItem(hwndDlg, IDC_SOUNDTREE), IsDlgButtonChecked(hwndDlg, IDC_ENABLESOUNDS));
		if (!IsDlgButtonChecked(hwndDlg, IDC_ENABLESOUNDS))
			SendMessage(hwndDlg, DM_HIDEPANE, 0, 0);
		else if (TreeView_GetSelection(hwndTree)&&TreeView_GetParent(hwndTree, TreeView_GetSelection(hwndTree)))
			SendMessage(hwndDlg, DM_SHOWPANE, 0, 0);
		break;

	case WM_COMMAND:
		if ( LOWORD(wParam) == IDC_ENABLESOUNDS )
			SendMessage(hwndDlg, DM_CHECKENABLED, 0, 0);

		if ( LOWORD(wParam) == IDC_PREVIEW ) {
			TVITEM tvi;
			HTREEITEM hti;

			ZeroMemory(&tvi,sizeof(tvi));
			ZeroMemory(&hti,sizeof(hti));
			hti=TreeView_GetSelection(hwndTree);
			if (hti==NULL) break;
			tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|TVIF_TEXT;
			tvi.hItem = hti;
			if (TreeView_GetItem(hwndTree, &tvi)==FALSE) break;
			if (tvi.lParam==-1) break;
			if (soundList[tvi.lParam].tempFile)
				 NotifyEventHooks(hPlayEvent, 1, (LPARAM)soundList[tvi.lParam].tempFile);
			else {
				DBVARIANT dbv;
				if(!DBGetContactSettingString(NULL,"SkinSounds",soundList[tvi.lParam].name,&dbv)) {
					char szPathFull[MAX_PATH];

					pathToAbsolute(dbv.pszVal, szPathFull, NULL);
					NotifyEventHooks(hPlayEvent, 1, (LPARAM)szPathFull);
					DBFreeVariant(&dbv);
			}	}
			break;
		}
		if ( LOWORD( wParam ) == IDC_CHANGE ) {
			char str[MAX_PATH] = "", strFull[MAX_PATH], strdir[MAX_PATH]="", filter[MAX_PATH];
			OPENFILENAMEA ofn;
			TVITEM tvi;
			HTREEITEM hti;

			ZeroMemory(&tvi,sizeof(tvi));
			ZeroMemory(&hti,sizeof(hti));
			hti=TreeView_GetSelection(hwndTree);
			if (hti==NULL) break;
			tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|TVIF_TEXT;
			tvi.hItem = hti;
			if (TreeView_GetItem(hwndTree, &tvi)==FALSE) break;
			if (tvi.lParam==-1) break;
			if (soundList[tvi.lParam].tempFile)
				mir_snprintf(strFull, SIZEOF(strFull), "%s", soundList[tvi.lParam].tempFile);
			else {
				if (DBGetContactSettingByte(NULL, "SkinSoundsOff", soundList[tvi.lParam].name, 0)==0) {
					DBVARIANT dbv;

					if (DBGetContactSettingString(NULL, "SkinSounds", soundList[tvi.lParam].name, &dbv)==0) {
						pathToAbsolute(dbv.pszVal, strdir, NULL);
						DBFreeVariant(&dbv);
			}	}	}

			mir_snprintf(strFull, SIZEOF(strFull), "%s", soundList[tvi.lParam].tempFile?soundList[tvi.lParam].tempFile:"");
			pathToAbsolute(strFull, strdir, NULL);
			ZeroMemory(&ofn, sizeof(ofn));
			mir_snprintf(filter, SIZEOF(filter), "%s (*.wav; *.mp3; *.ogg; *.flac)%c*.WAV; *.MP3; *.OGG; *.FLAC%c%s (*)%c*%c", Translate("Sound Files"), 0, 0, Translate("All Files"), 0, 0);
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = GetParent(hwndDlg);
			ofn.hInstance = NULL;
			ofn.lpstrFilter = filter;
			{   char* slash = strrchr(strdir, '\\');
				if (slash) {
					*slash = 0;
					ofn.lpstrInitialDir = strdir;
				}
			}
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER|OFN_LONGNAMES|OFN_NOCHANGEDIR;
			ofn.nMaxFile = SIZEOF(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = "wav";
			if(!GetOpenFileNameA(&ofn)) break;
			CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)str, (LPARAM)strFull);
			soundList[tvi.lParam].tempFile = mir_strdup(strFull);
			SetDlgItemTextA(hwndDlg, IDC_LOCATION, strFull);
		}
		if(LOWORD(wParam)==IDC_GETMORE) {
			CallService(MS_UTILS_OPENURL,1,(LPARAM)"http://addons.miranda-im.org/index.php?action=display&id=5");
			break;
		}
        if(LOWORD(wParam)==IDC_LOCATION) {
            break;
        }
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY)
			{
				int i;

            DBWriteContactSettingByte(NULL, "Skin", "UseSound", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLESOUNDS));
				for ( i=0; i < soundCount; i++ )
					if ( soundList[i].tempFile )
						DBWriteContactSettingString(NULL,"SkinSounds",soundList[i].name,soundList[i].tempFile);
				{
					TVITEM tvi,tvic;
					tvi.hItem = TreeView_GetRoot(hwndTree);
					while ( tvi.hItem != NULL ) {
						tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
						TreeView_GetItem(hwndTree, &tvi);
						if ( tvi.lParam == -1 ) {
							tvic.hItem = TreeView_GetChild(hwndTree, tvi.hItem);
							while ( tvic.hItem != NULL ) {
								tvic.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
								TreeView_GetItem(hwndTree, &tvic);
								if ((( tvic.state & TVIS_STATEIMAGEMASK ) >> 12 == 2 )) {
									DBCONTACTGETSETTING cgs;
									cgs.szModule = "SkinSoundsOff";
									cgs.szSetting = soundList[tvic.lParam].name;
									CallService(MS_DB_CONTACT_DELETESETTING,(WPARAM)(HANDLE)NULL,(LPARAM)&cgs);
								}
								else DBWriteContactSettingByte(NULL,"SkinSoundsOff",soundList[tvic.lParam].name,1);
								tvic.hItem=TreeView_GetNextSibling(hwndTree,tvic.hItem);
						}	}

						tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
				}	}
				return TRUE;
			}
			break;
		case IDC_SOUNDTREE:
			switch(((NMHDR*)lParam)->code) {
			case TVN_SELCHANGEDA:
				{
					NMTREEVIEW *pnmtv = (NMTREEVIEW*)lParam;
					TVITEM tvi = pnmtv->itemNew;

					if (tvi.lParam==-1) {
						SendMessage(hwndDlg, DM_HIDEPANE, 0, 0);
					}
					else {
						TCHAR buf[256];
						DBVARIANT dbv;

						mir_sntprintf(buf, SIZEOF(buf), _T("%s: %s"), soundList[tvi.lParam].section, soundList[tvi.lParam].description);
						SetDlgItemText(hwndDlg, IDC_NAMEVAL, buf);
						if (soundList[tvi.lParam].tempFile)
							SetDlgItemTextA(hwndDlg, IDC_LOCATION, soundList[tvi.lParam].tempFile);
						else if(!DBGetContactSettingString(NULL,"SkinSounds",soundList[tvi.lParam].name,&dbv)) {
							SetDlgItemTextA(hwndDlg, IDC_LOCATION, dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else SetDlgItemText(hwndDlg, IDC_LOCATION, TranslateT("<not specified>"));
						SendMessage(hwndDlg, DM_SHOWPANE, 0, 0);
					}
				}
				break;
			case TVN_KEYDOWN:
				{
					NMTVKEYDOWN* ptkd = (NMTVKEYDOWN*)lParam;

					if (ptkd&&ptkd->wVKey==VK_SPACE&&TreeView_GetSelection(ptkd->hdr.hwndFrom))
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x=(short)LOWORD(GetMessagePos());
					hti.pt.y=(short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
					if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
						if (hti.flags&TVHT_ONITEM)
							if(hti.flags&TVHT_ONITEMSTATEICON)
								if (TreeView_GetParent(hwndTree, hti.hItem)!=NULL)
									SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
			}	}
			break;
		}
		break;
	}
	return FALSE;
}

static UINT iconsExpertOnlyControls[]={IDC_IMPORT};

static int SkinOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = -200000000;
	odp.hInstance = hMirandaInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SOUND);
	odp.pszGroup = LPGEN("Customize");
	odp.pszTitle = LPGEN("Sounds");
	odp.pfnDlgProc = DlgProcSoundOpts;
	odp.flags = ODPF_BOLDGROUPS;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	return 0;
}

static int SkinSystemModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE,SkinOptionsInit);
	return 0;
}

int LoadSkinSounds(void)
{
	bModuleInitialized = TRUE;

	soundList=NULL;
	soundCount=0;
	CreateServiceFunction(MS_SKIN_ADDNEWSOUND,ServiceSkinAddNewSound);
	CreateServiceFunction(MS_SKIN_PLAYSOUND,ServiceSkinPlaySound);
	HookEvent(ME_SYSTEM_MODULESLOADED,SkinSystemModulesLoaded);
	hPlayEvent=CreateHookableEvent(ME_SKIN_PLAYINGSOUND);
	SetHookDefaultForHookableEvent(hPlayEvent, SkinPlaySoundDefault);
	return 0;
}

void UnloadSkinSounds(void)
{
	int i;

	if ( !bModuleInitialized ) return;

	for(i=0;i<soundCount;i++) {
		mir_free(soundList[i].name);
		mir_free(soundList[i].section);
		mir_free(soundList[i].description);
		if (soundList[i].tempFile) mir_free(soundList[i].tempFile);
	}
	if(soundCount) mir_free(soundList);
}
