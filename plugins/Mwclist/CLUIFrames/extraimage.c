#include "../commonheaders.h"

extern int DefaultImageListColorDepth;

int EnabledColumnCount=0;
boolean visar[EXTRACOLUMNCOUNT];
#define ExtraImageIconsIndexCount 3
int ExtraImageIconsIndex[ExtraImageIconsIndexCount];

static HANDLE hExtraImageListRebuilding,hExtraImageApplying;
HANDLE hExtraImageClick;

static HIMAGELIST hExtraImageList;
extern HINSTANCE g_hInst;
extern HIMAGELIST hCListImages;

extern int ClcIconsChanged(WPARAM,LPARAM);

void SetAllExtraIcons(HWND hwndList,HANDLE hContact);
void LoadExtraImageFunc();
extern HICON LoadIconFromExternalFile (char *filename,int i,boolean UseLibrary,boolean registerit,char *IconName,char *SectName,char *Description,int internalidx);
boolean ImageCreated=FALSE;
void ReloadExtraIcons();
BYTE ExtraOrder[]=
{
	0, // EXTRA_ICON_EMAIL	
	1, // EXTRA_ICON_PROTO	
	2, // EXTRA_ICON_SMS	
	3, // EXTRA_ICON_ADV1	
	4, // EXTRA_ICON_ADV2	
	5, // EXTRA_ICON_WEB	
	6, // EXTRA_ICON_CLIENT 
	7, // EXTRA_ICON_ADV3	
	8, // EXTRA_ICON_ADV4	
};

BOOL HasExtraIconsService()
{
	return ServiceExists("ExtraIcon/Register");
}

boolean isColumnVisible(int extra)
{
	int i=0;

	if (HasExtraIconsService())
		return TRUE;

	for (i=0; i<sizeof(ExtraOrder)/sizeof(ExtraOrder[0]); i++)
		if (ExtraOrder[i]==extra)
		{
			switch(i+1)
			{
				case EXTRA_ICON_EMAIL:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_EMAIL",1));
				case EXTRA_ICON_PROTO:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_PROTO",1));
				case EXTRA_ICON_SMS:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_SMS",1));
				case EXTRA_ICON_ADV1:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV1",1));
				case EXTRA_ICON_ADV2:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV2",1));	
				case EXTRA_ICON_WEB:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_WEB",1));
				case EXTRA_ICON_CLIENT:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_CLIENT",1));
				case EXTRA_ICON_ADV3:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV3",0));
				case EXTRA_ICON_ADV4:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV4",0));			
			}
			break;
		}
	return(FALSE);
}

void GetVisColumns()
{
	visar[0]=isColumnVisible(0);
	visar[1]=isColumnVisible(1);
	visar[2]=isColumnVisible(2);
	visar[3]=isColumnVisible(3);
	visar[4]=isColumnVisible(4);
	visar[5]=isColumnVisible(5);
	visar[6]=isColumnVisible(6);
	visar[7]=isColumnVisible(7);
	visar[8]=isColumnVisible(8);
};

__inline int bti(boolean b)
{
	return(b?1:0);
};
int colsum(int from,int to)
{
	int i,sum;
	if (from<0||from>=EXTRACOLUMNCOUNT){return(-1);};
	if (to<0||to>=EXTRACOLUMNCOUNT){return(-1);};
	if (to<from){return(-1);};

	sum=0;
	for (i=from;i<=to;i++)
	{
		sum+=bti(visar[i]);
	};
	return(sum);
};

int ExtraToColumnNum(int extra)
{
	if (HasExtraIconsService())
	{
		if (extra < 1 || extra > EXTRACOLUMNCOUNT) 
			return -1;
		else
			return extra-1;
	}
	else
	{
		int cnt=EnabledColumnCount;
		int extracnt=EXTRACOLUMNCOUNT-1;
		int ord=ExtraOrder[extra-1];
		if (!visar[ord]) return -1;
		return (colsum(0,ord)-1);
	}
};

int ColumnNumToExtra(int column)
{
	int i;

	if (HasExtraIconsService())
		return column+1;

	for (i=0; i<sizeof(ExtraOrder)/sizeof(ExtraOrder[0]); i++)
		if (ExtraOrder[i]==column)
			return i+1;

	return -1;
}

INT_PTR SetIconForExtraColumn(WPARAM wParam,LPARAM lParam)
{
	pIconExtraColumn piec;
	int icol;
	HANDLE hItem;

	if (pcli->hwndContactTree==0){return(-1);};
	if (wParam==0||lParam==0){return(-1);};
	piec=(pIconExtraColumn)lParam;

	if (piec->cbSize!=sizeof(IconExtraColumn)){return(-1);};
	icol=ExtraToColumnNum(piec->ColumnType);
	if (icol==-1){return(-1);};
	hItem=(HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)wParam,0);
	if (hItem==0){return(-1);};

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(icol,piec->hImage));	
	return(0);
};

//wparam=hIcon
//return hImage on success,-1 on failure
INT_PTR AddIconToExtraImageList(WPARAM wParam,LPARAM lParam)
{
	if (hExtraImageList==0||wParam==0){return(-1);};
	return((int)ImageList_AddIcon(hExtraImageList,(HICON)wParam));
}

void SetNewExtraColumnCount()
{
	int newcount;

	GetVisColumns();
	newcount=colsum(0,EXTRACOLUMNCOUNT-1);
	DBWriteContactSettingByte(NULL,CLUIFrameModule,"EnabledColumnCount",(BYTE)newcount);
	EnabledColumnCount=newcount;
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNS,EnabledColumnCount,0);
}

int OnIconLibIconChanged(WPARAM wParam,LPARAM lParam)
{
	HICON hicon;
	hicon=LoadIconFromExternalFile("clisticons.dll",0,TRUE,FALSE,"Email","Contact List","Email Icon",-IDI_EMAIL);
	ExtraImageIconsIndex[0]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[0],hicon );		

	hicon=LoadIconFromExternalFile("clisticons.dll",1,TRUE,FALSE,"Sms","Contact List","Sms Icon",-IDI_SMS);
	ExtraImageIconsIndex[1]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[1],hicon );		

	hicon=LoadIconFromExternalFile("clisticons.dll",4,TRUE,FALSE,"Web","Contact List","Web Icon",-IDI_GLOBUS);
	ExtraImageIconsIndex[2]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[2],hicon );		

	NotifyEventHooks(ME_SKIN_ICONSCHANGED,0,0);
	pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);
	return 0;
}

void ReloadExtraIcons()
{
	int count,i;
	PROTOACCOUNT **accs;
	HICON hicon;

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNSSPACE,DBGetContactSettingByte(NULL,"CLUI","ExtraColumnSpace",18),0);					
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,0,(LPARAM)NULL);		
	if (hExtraImageList){ImageList_Destroy(hExtraImageList);};
	hExtraImageList=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,256);

	if (!HasExtraIconsService())
	{
		//loading icons
		hicon=LoadIconFromExternalFile("clisticons.dll",0,TRUE,TRUE,"Email","Contact List","Email Icon",-IDI_EMAIL);
		if (!hicon) hicon=LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_EMAIL));
		ExtraImageIconsIndex[0]=ImageList_AddIcon(hExtraImageList,hicon );


		hicon=LoadIconFromExternalFile("clisticons.dll",1,TRUE,TRUE,"Sms","Contact List","Sms Icon",-IDI_SMS);
		if (!hicon) hicon=LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SMS));
		ExtraImageIconsIndex[1]=ImageList_AddIcon(hExtraImageList,hicon );

		hicon=LoadIconFromExternalFile("clisticons.dll",4,TRUE,TRUE,"Web","Contact List","Web Icon",-IDI_GLOBUS);
		if (!hicon) hicon=LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_GLOBUS));
		ExtraImageIconsIndex[2]=ImageList_AddIcon(hExtraImageList,hicon );	

		//calc only needed protocols
		ProtoEnumAccounts( &count, &accs );
		for ( i=0; i < count; i++ )
			if ( IsAccountEnabled( accs[i] ) && CallProtoService( accs[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0 ))
				ImageList_AddIcon( hExtraImageList, LoadSkinnedProtoIcon( accs[i]->szModuleName, ID_STATUS_ONLINE ));
	}

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,0,(LPARAM)hExtraImageList);		

	//SetAllExtraIcons(hImgList);
	SetNewExtraColumnCount();
	NotifyEventHooks(hExtraImageListRebuilding,0,0);
	ImageCreated=TRUE;
}

void ClearExtraIcons();

void ReAssignExtraIcons()
{
	ClearExtraIcons();
	SetNewExtraColumnCount();
	SetAllExtraIcons(pcli->hwndContactTree,0);
	SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
}

void ClearExtraIcons()
{
	int i;
	HANDLE hContact,hItem;

	//EnabledColumnCount=DBGetContactSettingByte(NULL,CLUIFrameModule,"EnabledColumnCount",5);
	//SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNS,EnabledColumnCount,0);
	SetNewExtraColumnCount();

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do {

		hItem=(HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)hContact,0);
		if (hItem==0){continue;};
		for (i=0;i<EnabledColumnCount;i++)
		{
			SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i,0xFF));	
		};

	} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
};

void SetAllExtraIcons(HWND hwndList,HANDLE hContact)
{
	HANDLE hItem;
	boolean hcontgiven=FALSE;
	char *szProto;
	char *ImgIndex[64];
	int maxpr,count,i;
	PROTOACCOUNT **accs;
	pdisplayNameCacheEntry pdnce;
	int em,pr,sms,a1,a2,w1,c1;
	int tick=0;
	BOOL hasExtraIconsService;
	hcontgiven=(hContact!=0);

	if (pcli->hwndContactTree==0){return;};
	tick=GetTickCount();
	if (ImageCreated==FALSE) ReloadExtraIcons();

	SetNewExtraColumnCount();

	hasExtraIconsService = HasExtraIconsService();
	if (!hasExtraIconsService)
	{
		em=ExtraToColumnNum(EXTRA_ICON_EMAIL);	
		pr=ExtraToColumnNum(EXTRA_ICON_PROTO);
		sms=ExtraToColumnNum(EXTRA_ICON_SMS);
		a1=ExtraToColumnNum(EXTRA_ICON_ADV1);
		a2=ExtraToColumnNum(EXTRA_ICON_ADV2);
		w1=ExtraToColumnNum(EXTRA_ICON_WEB);
		c1=ExtraToColumnNum(EXTRA_ICON_CLIENT);

		memset(ImgIndex,0,sizeof(ImgIndex));
		ProtoEnumAccounts( &count, &accs );

		maxpr=0;
		//calc only needed protocols
		for( i=0; i < count; i++ ) {
			if ( !IsAccountEnabled( accs[i] ) || CallProtoService( accs[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0 ) == 0 )
				continue;
			ImgIndex[maxpr] = accs[i]->szModuleName;
			maxpr++;
		}
	}

	if ( hContact == NULL )
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);

	do {
		szProto=NULL;
		hItem=hContact;
		if (hItem==0){continue;};
		pdnce=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hItem);
		if (pdnce==NULL) {continue;};

		//		szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);		
		szProto=pdnce->szProto;

		if (!hasExtraIconsService)
		{
			if (ExtraToColumnNum(EXTRA_ICON_WEB)!=-1)
			{
				boolean showweb=FALSE;

				if (szProto != NULL)
				{
					char *homepage;
					homepage=DBGetStringA(pdnce->hContact,"UserInfo", "Homepage");
					if (!homepage)
						homepage=DBGetStringA(pdnce->hContact,pdnce->szProto, "Homepage");
					if (homepage!=NULL)
					{											
						showweb=TRUE;				
						mir_free(homepage);
					}
				}

				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraToColumnNum(EXTRA_ICON_WEB),(showweb)?2:0xFF));	
			}

			if (ExtraToColumnNum(EXTRA_ICON_EMAIL)!=-1)
			{
				DBVARIANT dbv={0};
				boolean showemail=TRUE;

				if (szProto == NULL || DBGetContactSettingString(hContact, szProto, "e-mail",&dbv)) 
				{
					DBFreeVariant(&dbv);
					if (DBGetContactSettingString(hContact, "UserInfo", "Mye-mail0", &dbv))
						showemail=FALSE;
				}
				DBFreeVariant(&dbv);

				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraToColumnNum(EXTRA_ICON_EMAIL),(showemail)?0:0xFF));	
			}

			if (ExtraToColumnNum(EXTRA_ICON_SMS)!=-1)
			{
				DBVARIANT dbv={0};
				boolean showsms=TRUE;

				if (szProto == NULL || DBGetContactSettingString(hContact, szProto, "Cellular",&dbv)) {
					DBFreeVariant(&dbv);
					if (DBGetContactSettingString(hContact, "UserInfo", "MyPhone0", &dbv))
						showsms=FALSE;
				}
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraToColumnNum(EXTRA_ICON_SMS),(showsms)?1:0xFF));	
				DBFreeVariant(&dbv);
			}

			if(ExtraToColumnNum(EXTRA_ICON_PROTO)!=-1) 
			{					
				for (i=0;i<maxpr;i++)
				{
					if(!MyStrCmp(ImgIndex[i],szProto))
					{
						SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraToColumnNum(EXTRA_ICON_PROTO),i+3));	
						break;
					}
				}
			}
		}

		NotifyEventHooks(hExtraImageApplying,(WPARAM)hContact,0);
		if (hcontgiven) break;
		Sleep(0);
	}
		while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));

	tick=GetTickCount()-tick;
	InvalidateRect(hwndList,NULL,FALSE);
	Sleep(0);
}

void LoadExtraImageFunc()
{
	CreateServiceFunction(MS_CLIST_EXTRA_SET_ICON,SetIconForExtraColumn); 
	CreateServiceFunction(MS_CLIST_EXTRA_ADD_ICON,AddIconToExtraImageList); 

	hExtraImageListRebuilding=CreateHookableEvent(ME_CLIST_EXTRA_LIST_REBUILD);
	hExtraImageApplying=CreateHookableEvent(ME_CLIST_EXTRA_IMAGE_APPLY);
	hExtraImageClick=CreateHookableEvent(ME_CLIST_EXTRA_CLICK);
	HookEvent(ME_SKIN2_ICONSCHANGED,OnIconLibIconChanged);
}
