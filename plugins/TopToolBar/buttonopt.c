#include "common.h"
#pragma hdrstop

extern HINSTANCE hInst;

char *DBGetString(HANDLE hContact,const char *szModule,const char *szSetting)
{
	char *str=NULL;
	DBVARIANT dbv;
	DBGetContactSetting(hContact,szModule,szSetting,&dbv);
	if(dbv.type==DBVT_ASCIIZ)
		str=strdup(dbv.pszVal);
	DBFreeVariant(&dbv);
	return str;
}




struct OrderData {
	int dragging;
	HTREEITEM hDragItem;
};



int CheckButtonOrder()
{
	boolean protochanged=FALSE;
	int StoredButCount;
	int i,count;
	int v;
	char buf[10];
	char buf2[10];
	
	//curproto=0;
	//curproto[1]='22';
		
	StoredButCount=DBGetContactSettingByte(0,TTB_OPTDIR,"ButCount",-1);
	if (StoredButCount==-1){protochanged=TRUE;};
	if (protochanged)
	{
		//reseting all settings;
		
		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&count,(LPARAM)&protos);
		
			v=0;
			for(i=0;i<count;i++) {
				if(protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
				itoa(v,&buf,10);			
				DBWriteContactSettingString(0,"Protocols",&buf,protos[i]->szName);		

				itoa(OFFSET_VISIBLE+v,&buf,10);//save default visible status
				DBWriteContactSettingByte(0,"Protocols",&buf,1);
				
			v++;
			};
		DBWriteContactSettingByte(0,TTB_OPTDIR,"ButCount",v);
			return(1);
	}
	else
	{
		return(0);
	};
	


};



static BOOL CALLBACK ProtocolOrderOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	struct ProtocolOrderData *dat;

	dat=(struct ProtocolOrderData*)GetWindowLong(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),GWL_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG: {
			PROTOCOLDESCRIPTOR **protos;
			TVINSERTSTRUCT tvis;
			ProtocolData *PD;
			char szName[64];
			char *szSTName;
			char buf[10];
			int i,count;

			dat=(struct ProtocolOrderData*)malloc(sizeof(struct ProtocolOrderData));
			SetWindowLong(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),GWL_USERDATA,(LONG)dat);
			dat->dragging=0;

			SetWindowLong(GetDlgItem(hwndDlg,IDC_PROTOCOLVISIBILITY),GWL_STYLE,GetWindowLong(GetDlgItem(hwndDlg,IDC_PROTOCOLVISIBILITY),GWL_STYLE)|TVS_NOHSCROLL);
			SetWindowLong(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),GWL_STYLE,GetWindowLong(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),GWL_STYLE)|TVS_NOHSCROLL);

			{	HIMAGELIST himlCheckBoxes;
				himlCheckBoxes=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR4|ILC_MASK,2,2);
				ImageList_AddIcon(himlCheckBoxes,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_NOTICK)));
				ImageList_AddIcon(himlCheckBoxes,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_TICK)));
				TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_PROTOCOLVISIBILITY),himlCheckBoxes,TVSIL_NORMAL);
				//
				TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),himlCheckBoxes,TVSIL_NORMAL);

			}

			tvis.hParent=NULL;
			tvis.hInsertAfter=TVI_LAST;
			tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;	
			
			CheckProtocolOrder();

			count=DBGetContactSettingByte(0,"Protocols","ProtoCount",-1);
			if (count==-1){return(FALSE);};

			for(i=0;i<count;i++) {
				//if(protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
				itoa(i,&buf,10);
				szSTName=DBGetString(0,"Protocols",&buf);
				if (szSTName==NULL){continue;};
				
				CallProtoService(szSTName,PS_GETNAME,sizeof(szName),(LPARAM)szName);
				PD=(ProtocolData*)malloc(sizeof(ProtocolData));
				
				
				PD->RealName=szSTName;
				
				itoa(OFFSET_VISIBLE+i,&buf,10);
				PD->show=DBGetContactSettingByte(0,"Protocols",&buf,1);

				itoa(OFFSET_PROTOPOS+i,&buf,10);
				PD->protopos=DBGetContactSettingByte(0,"Protocols",&buf,-1);
					
				tvis.item.lParam=(LPARAM)PD;
				tvis.item.pszText=Translate(szName);
				tvis.item.iImage=tvis.item.iSelectedImage=PD->show;
				TreeView_InsertItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&tvis);
				tvis.item.iImage=tvis.item.iSelectedImage=PD->show;
				TreeView_InsertItem(GetDlgItem(hwndDlg,IDC_PROTOCOLVISIBILITY),&tvis);
				
				//free(szSTName);
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0: 
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{	
							TVITEM tvi;
							PROTOCOLDESCRIPTOR **protos;
							int count;
							char idstr[33];
							char buf[10];

							/*
							tvi.hItem=TreeView_GetRoot(GetDlgItem(hwndDlg,IDC_PROTOCOLVISIBILITY));
							tvi.cchTextMax=32;
							tvi.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_PARAM|TVIF_HANDLE;
							tvi.pszText=&idstr;
							//count=0;
							while(tvi.hItem!=NULL) {
								itoa(OFFSET_VISIBLE+count,&buf,10);
								TreeView_GetItem(GetDlgItem(hwndDlg,IDC_PROTOCOLVISIBILITY),&tvi);
								DBWriteContactSettingByte(NULL,"Protocols",&buf,(byte)tvi.iImage);
								tvi.hItem=TreeView_GetNextSibling(GetDlgItem(hwndDlg,IDC_PROTOCOLVISIBILITY),tvi.hItem);
								//count++;
							}
							*/
							tvi.hItem=TreeView_GetRoot(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER));
							tvi.cchTextMax=32;
							tvi.mask=TVIF_TEXT|TVIF_PARAM|TVIF_HANDLE;
							tvi.pszText=&idstr;
							//CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&count,(LPARAM)&protos);
							//count--;
							count=0;

							while(tvi.hItem!=NULL) {
								itoa(count,buf,10);
								TreeView_GetItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&tvi);
								DBWriteContactSettingString(NULL,"Protocols",&buf,((ProtocolData *)tvi.lParam)->RealName);
								
								itoa(OFFSET_PROTOPOS+count,&buf,10);//save pos in protos
								DBWriteContactSettingByte(0,"Protocols",&buf,((ProtocolData *)tvi.lParam)->protopos);

								itoa(OFFSET_VISIBLE+count,&buf,10);//save pos in protos
								DBWriteContactSettingByte(0,"Protocols",&buf,((ProtocolData *)tvi.lParam)->show);
								
								tvi.hItem=TreeView_GetNextSibling(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),tvi.hItem);
								
								count++;
							}
							CluiProtocolStatusChanged(0,0);
						}
					}
					break;
					/*
					case IDC_PROTOCOLORDER: //IDC_PROTOCOLVISIBILITY:
					if(((LPNMHDR)lParam)->code==NM_CLICK) {
						TVHITTESTINFO hti;
						hti.pt.x=(short)LOWORD(GetMessagePos());
						hti.pt.y=(short)HIWORD(GetMessagePos());
						ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
						if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
							if(hti.flags&TVHT_ONITEMICON) {
								TVITEM tvi;
								tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
								tvi.hItem=hti.hItem;
								TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
								tvi.iImage=tvi.iSelectedImage=!tvi.iImage;
								((ProtocolData *)tvi.lParam)->show=tvi.iImage;
								TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
								ShowWindow(GetDlgItem(hwndDlg,IDC_PROTOCOLORDERWARNING),SW_SHOW);
							}
					}
					break;
					*/
				case IDC_PROTOCOLORDER:
					switch (((LPNMHDR)lParam)->code) {
						case TVN_BEGINDRAG:
							SetCapture(hwndDlg);
							dat->dragging=1;
							dat->hDragItem=((LPNMTREEVIEW)lParam)->itemNew.hItem;
							TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),dat->hDragItem);
							//ShowWindow(GetDlgItem(hwndDlg,IDC_PROTOCOLORDERWARNING),SW_SHOW);
							break;
						case NM_CLICK:
							{
							
						TVHITTESTINFO hti;
						hti.pt.x=(short)LOWORD(GetMessagePos());
						hti.pt.y=(short)HIWORD(GetMessagePos());
						ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
						if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
							if(hti.flags&TVHT_ONITEMICON) {
								TVITEM tvi;
								tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
								tvi.hItem=hti.hItem;
								TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
								tvi.iImage=tvi.iSelectedImage=!tvi.iImage;
								((ProtocolData *)tvi.lParam)->show=tvi.iImage;
								TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
								
								//all changes take effect in runtime
								//ShowWindow(GetDlgItem(hwndDlg,IDC_PROTOCOLORDERWARNING),SW_SHOW);
							}
							
							
							
							};
						}
					break;
			}
			break;
		case WM_MOUSEMOVE:
			if(!dat->dragging) break;
			{	TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(lParam);
				hti.pt.y=(short)HIWORD(lParam);
				ClientToScreen(hwndDlg,&hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&hti.pt);
				TreeView_HitTest(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&hti);
				if(hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
					hti.pt.y-=TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER))/2;
					TreeView_HitTest(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&hti);
					TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),hti.hItem,1);
				}
				else {
					if(hti.flags&TVHT_ABOVE) SendDlgItemMessage(hwndDlg,IDC_PROTOCOLORDER,WM_VSCROLL,MAKEWPARAM(SB_LINEUP,0),0);
					if(hti.flags&TVHT_BELOW) SendDlgItemMessage(hwndDlg,IDC_PROTOCOLORDER,WM_VSCROLL,MAKEWPARAM(SB_LINEDOWN,0),0);
					TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),NULL,0);
				}
			}
			break;
		case WM_LBUTTONUP:
			if(!dat->dragging) break;
			TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),NULL,0);
			dat->dragging=0;
			ReleaseCapture();
			{	TVHITTESTINFO hti;
				TVITEM tvi;
				hti.pt.x=(short)LOWORD(lParam);
				hti.pt.y=(short)HIWORD(lParam);
				ClientToScreen(hwndDlg,&hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&hti.pt);
				hti.pt.y-=TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER))/2;
				TreeView_HitTest(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&hti);
				if(dat->hDragItem==hti.hItem) break;
				tvi.mask=TVIF_HANDLE|TVIF_PARAM;
				tvi.hItem=hti.hItem;
				TreeView_GetItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&tvi);
				if(hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
					TVINSERTSTRUCT tvis;
					char name[128];
					tvis.item.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
					tvis.item.stateMask=0xFFFFFFFF;
					tvis.item.pszText=name;
					tvis.item.cchTextMax=sizeof(name);
					tvis.item.hItem=dat->hDragItem;
					//
					tvis.item.iImage=tvis.item.iSelectedImage=((ProtocolData *)tvi.lParam)->show;
					
					TreeView_GetItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&tvis.item);
					

					TreeView_DeleteItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),dat->hDragItem);
					tvis.hParent=NULL;
					tvis.hInsertAfter=hti.hItem;
					TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),TreeView_InsertItem(GetDlgItem(hwndDlg,IDC_PROTOCOLORDER),&tvis));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
			break; 
	}
	return FALSE;
}

static int ProtocolOrderInit(WPARAM wParam,LPARAM lParam) {
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=-1000000000;
	odp.hInstance=hInst;//GetModuleHandle(NULL);
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPT_PROTOCOLORDER);
	odp.pszGroup=Translate("Contact List");
	odp.pszTitle=Translate("Protocols");
	odp.pfnDlgProc=ProtocolOrderOpts;
	odp.flags=ODPF_BOLDGROUPS|ODPF_EXPERTONLY;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}

int LoadProtocolOrderModule(void) {
	HookEvent(ME_OPT_INITIALISE,ProtocolOrderInit);
	return 0;
}