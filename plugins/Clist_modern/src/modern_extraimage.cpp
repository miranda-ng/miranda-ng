#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"



#define ExtraImageIconsIndexCount 6

bool visar[EXTRACOLUMNCOUNT];
int ExtraImageIconsIndex[ExtraImageIconsIndexCount];
int EnabledColumnCount = 0;
BOOL g_mutex_bSetAllExtraIconsCycle = 0;

static HIMAGELIST hExtraImageList = NULL;
static HIMAGELIST hWideExtraImageList = NULL;

void ExtraImage_SetAllExtraIcons(HWND hwndList,HANDLE hContact);

void ClearExtraIcons();

bool ImageCreated = FALSE;
BYTE ExtraOrder[] = 
{
	1, // EXTRA_ICON_EMAIL	
	2, // EXTRA_ICON_PROTO	
	3, // EXTRA_ICON_SMS	
	4, // EXTRA_ICON_ADV1	
	5, // EXTRA_ICON_ADV2	
	6, // EXTRA_ICON_WEB	
	7, // EXTRA_ICON_CLIENT 
	0, // EXTRA_ICON_VISMODE 
	8, // EXTRA_ICON_ADV3	
	9, // EXTRA_ICON_ADV4	
};

BOOL HasExtraIconsService()
{
	return ServiceExists("ExtraIcon/Register");
}

bool isColumnVisible(int extra)
{
	if (HasExtraIconsService())
		return true;

	int i=0;
	for (i=0; i < sizeof(ExtraOrder)/sizeof(ExtraOrder[0]); i++)
		if (ExtraOrder[i] == extra)
		{
			switch(i+1)
			{
				case EXTRA_ICON_EMAIL:   return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_EMAIL",SETTING_EXTRA_ICON_EMAIL_DEFAULT) != 0;
				case EXTRA_ICON_PROTO:   return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_PROTO",SETTING_EXTRA_ICON_PROTO_DEFAULT) != 0;
				case EXTRA_ICON_SMS:     return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_SMS",SETTING_EXTRA_ICON_SMS_DEFAULT) != 0;
				case EXTRA_ICON_ADV1:    return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_ADV1",SETTING_EXTRA_ICON_ADV1_DEFAULT) != 0;
				case EXTRA_ICON_ADV2:    return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_ADV2",SETTING_EXTRA_ICON_ADV2_DEFAULT) != 0;	
				case EXTRA_ICON_WEB:	    return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_WEB",SETTING_EXTRA_ICON_WEB_DEFAULT) != 0;
				case EXTRA_ICON_CLIENT:  return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_CLIENT",SETTING_EXTRA_ICON_CLIENT_DEFAULT) != 0;
				case EXTRA_ICON_VISMODE: return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_VISMODE",SETTING_EXTRA_ICON_VISMODE_DEFAULT) != 0;
				case EXTRA_ICON_ADV3:    return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_ADV3",SETTING_EXTRA_ICON_ADV3_DEFAULT) != 0;
				case EXTRA_ICON_ADV4:    return db_get_b(NULL,CLUIFrameModule,"EXTRA_ICON_ADV4",SETTING_EXTRA_ICON_ADV4_DEFAULT) != 0;			
			}
			break;
		}
	return(FALSE);
}

void GetVisColumns()
{
	visar[0] = isColumnVisible(0);
	visar[1] = isColumnVisible(1);
	visar[2] = isColumnVisible(2);
	visar[3] = isColumnVisible(3);
	visar[4] = isColumnVisible(4);
	visar[5] = isColumnVisible(5);
	visar[6] = isColumnVisible(6);
	visar[7] = isColumnVisible(7);
	visar[8] = isColumnVisible(8);
	visar[9] = isColumnVisible(9);
};

__inline int bti(bool b)
{
	return(b?1:0);
};
int colsum(int from,int to)
{
	int i,sum;
	if (from < 0 || from >= EXTRACOLUMNCOUNT){return(-1);};
	if (to < 0 || to >= EXTRACOLUMNCOUNT){return(-1);};
	if (to < from){return(-1);};

	sum = 0;
	for (i = from;i <= to;i++)
	{
		sum += bti(visar[i]);
	};
	return(sum);
};




int ExtraImage_ExtraIDToColumnNum(int extra)
{
	if (HasExtraIconsService())
	{
		if (extra < 1 || extra > EXTRACOLUMNCOUNT) 
			return -1;
		else
			return extra-1;
	}

	int ord = ExtraOrder[extra-1];
    if ( !visar[ord]) return -1;
	return (colsum(0,ord)-1);
};

int ExtraImage_ColumnNumToExtraID(int column)
{
	if (HasExtraIconsService())
		return column+1;

	for (int i=0; i < EXTRA_ICON_COUNT; i++)
		if (ExtraImage_ExtraIDToColumnNum(i) == column)
			return i;

	return -1;
}

INT_PTR SetIconForExtraColumn(WPARAM wParam,LPARAM lParam)
{
	pIconExtraColumn piec;
	int icol;
	HANDLE hItem;

	if (pcli->hwndContactTree == 0){return(-1);};
	if (wParam == 0 || lParam == 0){return(-1);};
	piec = (pIconExtraColumn)lParam;

	if (piec->cbSize != sizeof(IconExtraColumn)){return(-1);};
	icol = ExtraImage_ExtraIDToColumnNum(piec->ColumnType);
	if (icol == -1){return(-1);};
	hItem = (HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)wParam,0);
	if (hItem == 0){return(-1);};

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(icol,piec->hImage));	
	return(0);
}

//wparam = hIcon
//return hImage on success,-1 on failure
INT_PTR AddIconToExtraImageList(WPARAM wParam,LPARAM lParam)
{
	if (hExtraImageList == 0 || wParam == 0)
		return -1;

	int res = ((int)ImageList_AddIcon(hExtraImageList,(HICON)wParam));
	return (res > 254) ? -1 : res;
}

void SetNewExtraColumnCount()
{
	LoadPositionsFromDB(ExtraOrder);
	GetVisColumns();

	int newcount = colsum(0,EXTRACOLUMNCOUNT-1);
	db_set_b(NULL,CLUIFrameModule,"EnabledColumnCount",(BYTE)newcount);
	EnabledColumnCount = newcount;
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNS,EnabledColumnCount,0);
}

void ExtraImage_ReloadExtraIcons()
{
	int count,i;
	PROTOACCOUNT **accs;
	HICON hicon;
	BOOL needFree;

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNSSPACE,db_get_b(NULL,"CLUI","ExtraColumnSpace",18),0);					
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,0,(LPARAM)NULL);		
	if (hExtraImageList){ImageList_Destroy(hExtraImageList);};
	if (hWideExtraImageList){ImageList_Destroy(hWideExtraImageList);};

	hExtraImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,256);
	hWideExtraImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,256);

	if ( !HasExtraIconsService()) {
		//loading icons
		HICON hIcon = LoadSkinnedIcon(SKINICON_OTHER_SENDEMAIL);
		ExtraImageIconsIndex[0] = ImageList_AddIcon(hExtraImageList, hIcon);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
		hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMS);
		ExtraImageIconsIndex[1] = ImageList_AddIcon(hExtraImageList, hIcon);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
		hIcon = LoadSkinnedIcon(SKINICON_EVENT_URL);
		ExtraImageIconsIndex[2] = ImageList_AddIcon(hExtraImageList, hIcon);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
		
		//calc only needed protocols
		//adding protocol icons
		ProtoEnumAccounts( &count, &accs );
		for (i=0;i < count;i++) {
			if ( !IsAccountEnabled(accs[i]) || CallProtoService(accs[i]->szModuleName, PS_GETCAPS,PFLAGNUM_2, 0 ) ==  0) 
				continue;

			hIcon = LoadSkinnedProtoIcon(accs[i]->szModuleName,ID_STATUS_ONLINE);
			ImageList_AddIcon(hExtraImageList, hIcon);
			CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
		}

		hicon = CLUI_LoadIconFromExternalFile("clisticons.dll",5,TRUE,TRUE,"AlwaysVis","Contact List",Translate("Always Visible"),-IDI_ALWAYSVIS,&needFree);
		if ( !hicon) {hicon = LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_ALWAYSVIS));needFree = TRUE;}
		ExtraImageIconsIndex[3] = ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		hicon = CLUI_LoadIconFromExternalFile("clisticons.dll",6,TRUE,TRUE,"NeverVis","Contact List",Translate("Never Visible"),-IDI_NEVERVIS,&needFree);
		if ( !hicon) {hicon = LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_NEVERVIS));needFree = TRUE;}
		ExtraImageIconsIndex[4] = ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		hicon = CLUI_LoadIconFromExternalFile("clisticons.dll",7,TRUE,TRUE,"ChatActivity","Contact List",Translate("Chat Activity"),-IDI_CHAT,&needFree);
		if ( !hicon) {hicon = LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_CHAT));needFree = TRUE;}
		ExtraImageIconsIndex[5] = ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);
	}

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,(WPARAM)hWideExtraImageList,(LPARAM)hExtraImageList);		
	//ExtraImage_SetAllExtraIcons(hImgList);
	SetNewExtraColumnCount();
	NotifyEventHooks(g_CluiData.hEventExtraImageListRebuilding,0,0);
	ImageCreated = TRUE;
}

void ReAssignExtraIcons()
{
	ClearExtraIcons();
	SetNewExtraColumnCount();
	ExtraImage_SetAllExtraIcons(pcli->hwndContactTree,0);
	SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
}

void ClearExtraIcons()
{
	SetNewExtraColumnCount();

	HANDLE hContact = db_find_first();
	do {
		HANDLE hItem = (HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)hContact,0);
		if (hItem == 0)
			continue;

		for (int i=0;i < EnabledColumnCount;i++)
			SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i,0xFF));	
	}
		while(hContact = db_find_next(hContact));
};

void ExtraImage_SetAllExtraIcons(HWND hwndList,HANDLE hContact)
{
	HANDLE hItem;
	int locApparentMode = 0;
	char * locApparentModeProto = NULL;
	bool hcontgiven = FALSE;
	char *szProto;
	char *(ImgIndex[64]);
	int maxpr,count,i;
	PROTOACCOUNT **accs;
	pdisplayNameCacheEntry pdnce;
	int em,pr,sms,a1,a2,w1,c1;
	int tick = 0;
	g_mutex_bSetAllExtraIconsCycle = 1;
	hcontgiven = (hContact != 0);

	if (pcli->hwndContactTree == 0){return;};
	tick = GetTickCount();
	if (ImageCreated == FALSE) ExtraImage_ReloadExtraIcons();

	SetNewExtraColumnCount();

	BOOL hasExtraIconsService = HasExtraIconsService();
	if ( !hasExtraIconsService)
	{
		em = ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_EMAIL);	
		pr = ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_PROTO);
		sms = ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_SMS);
		a1 = ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_ADV1);
		a2 = ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_ADV2);
		w1 = ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_WEB);
		c1 = ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_CLIENT);

		memset( ImgIndex, 0, sizeof( ImgIndex ));
		ProtoEnumAccounts( &count, &accs );
		maxpr = 0;
		//calc only needed protocols
		for (i=0;i < count;i++) {
			if ( !IsAccountEnabled( accs[i] ) || CallProtoService(accs[i]->szModuleName,PS_GETCAPS,PFLAGNUM_2,0) == 0) continue;
			ImgIndex[maxpr] = accs[i]->szModuleName;
			maxpr++;
		}
	}

	if (hContact == NULL)
	{
		hContact = db_find_first();
	}	

	do {

		szProto = NULL;
		hItem = hContact;
		if (hItem == 0){continue;};
		pdnce = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hItem);
		if (pdnce == NULL) {continue;};

		//		szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);		
		szProto = pdnce->m_cache_cszProto;

		if ( !hasExtraIconsService)
		{
			{
				bool showweb;	
				showweb = FALSE;     
				if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_WEB) != -1)
				{

					if (szProto != NULL)
					{
						char *homepage;
						homepage = db_get_sa(pdnce->hContact,"UserInfo", "Homepage");
						if ( !homepage)
							homepage = db_get_sa(pdnce->hContact,pdnce->m_cache_cszProto, "Homepage");
						if (homepage != NULL)
						{											
							showweb = TRUE;				
							mir_free(homepage);
						}
					}
					SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_WEB),(showweb)?2:0xFF));	
				}
			}		
			{
				DBVARIANT dbv = {0};
				bool showemail;	
				showemail = TRUE;
				if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_EMAIL) != -1)
				{

					if (szProto == NULL || DBGetContactSettingString(hContact, szProto, "e-mail",&dbv)) 
					{
						db_free(&dbv);
						if (DBGetContactSettingString(hContact, "UserInfo", "Mye-mail0", &dbv))
							showemail = FALSE;					
					}
					SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_EMAIL),(showemail)?0:0xFF));	
					db_free(&dbv);
				}
			}

			{
				DBVARIANT dbv = {0};
				bool showsms;	
				showsms = TRUE;
				if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_SMS) != -1)
				{
					if (szProto == NULL || DBGetContactSettingString(hContact, szProto, "Cellular",&dbv)) 
					{
						db_free(&dbv);
						if (DBGetContactSettingString(hContact, "UserInfo", "MyPhone0", &dbv))
							showsms = FALSE;
					}
					SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_SMS),(showsms)?1:0xFF));	
					db_free(&dbv);
				}
			}		

			if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_PROTO) != -1) 
			{					
				for (i=0;i < maxpr;i++)
				{
					if ( !mir_strcmp(ImgIndex[i],szProto))
					{
						SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_PROTO),i+3));	
						break;
					};
				};				
			};
			if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_VISMODE) != -1) 
			{
				BYTE iconIndex = 0xFF;
				if (szProto != NULL)
				{
					if ( !db_get_b(hContact, szProto, "ChatRoom", 0))		
					{
						if (pdnce->ApparentMode == ID_STATUS_OFFLINE)
							iconIndex = ExtraImageIconsIndex[4];	
						else if (pdnce->ApparentMode == ID_STATUS_ONLINE)
						{
							if (szProto != locApparentModeProto)
							{
								locApparentModeProto = szProto;
								locApparentMode = CallProtoService(locApparentModeProto,PS_GETSTATUS,0,0);
							}
							if (locApparentMode == ID_STATUS_INVISIBLE || db_get_b(NULL,"CList","AlwaysShowAlwaysVisIcon",SETTING_ALWAYSVISICON_DEFAULT) == 1)
 								iconIndex = ExtraImageIconsIndex[3];						}
					}
					else 
					{
						if (pdnce->ApparentMode == ID_STATUS_OFFLINE)
							iconIndex = ExtraImageIconsIndex[5];	
						else iconIndex = 255;	
					}
				}
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_VISMODE),iconIndex));	
			}
		}
		NotifyEventHooks(g_CluiData.hEventExtraImageApplying,(WPARAM)hContact,0);
		if (hcontgiven) break;
		Sleep(0);
	} while(hContact = db_find_next(hContact));

	tick = GetTickCount()-tick;
	g_mutex_bSetAllExtraIconsCycle = 0;
	CLUI__cliInvalidateRect(hwndList,NULL,FALSE);
	Sleep(0);
}

HWND SetToolTip(HWND hwnd, TCHAR * tip);

HWND tipHWND = NULL;

static int ehhShowExtraInfoTip(WPARAM wParam, LPARAM lParam)
{
	CLCEXTRAINFOTIP * eit = (CLCEXTRAINFOTIP *)lParam;
	//tipHWND = SetToolTip(eit->hwnd,_T("TEST"));
	//SendMessage(tipHWND,TTM_ACTIVATE,1,0);
	//SendMessage(tipHWND,TTM_POPUP,0,0);
	return 1;
	
}

int ehhHideExtraInfoTip(WPARAM wParam, LPARAM lParam)
{
	
	//SendMessage(tipHWND,TTM_ACTIVATE,0,0);
	return 0;
	
}

INT_PTR WideSetIconForExtraColumn(WPARAM wParam,LPARAM lParam)
{
	pIconExtraColumn piec;
	int icol;
	HANDLE hItem;

	if (pcli->hwndContactTree == 0){return(-1);};
	if (wParam == 0 || lParam == 0){return(-1);};
	piec = (pIconExtraColumn)lParam;

	if (piec->cbSize != sizeof(IconExtraColumn)){return(-1);};
	icol = ExtraImage_ExtraIDToColumnNum(piec->ColumnType);
	if (icol == -1){return(-1);};

	hItem = (HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)wParam,0);
	if (hItem == 0){return(-1);};
	if (piec->hImage == (HANDLE)0xFF)	piec->hImage = (HANDLE)0xFFFF;

	SendMessage(pcli->hwndContactTree,CLM_SETWIDEEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(icol,piec->hImage));	
	return(0);
};

//wparam = hIcon
//return hImage on success,-1 on failure
INT_PTR WideAddIconToExtraImageList(WPARAM wParam,LPARAM lParam)
{
	if (hWideExtraImageList == 0 || wParam == 0){return(-1);};
	int res = ((int)ImageList_AddIcon(hWideExtraImageList,(HICON)wParam));
	if (res == 0xFF)	res = ((int)ImageList_AddIcon(hWideExtraImageList,(HICON)wParam));
	if (res>0xFFFE) return -1;
	return res;
};
static int ehhExtraImage_UnloadModule(WPARAM wParam,LPARAM lParam)
{
	if (hExtraImageList) { ImageList_Destroy(hExtraImageList); };
	if (hWideExtraImageList) { ImageList_Destroy(hWideExtraImageList); };
	return 0;

}
void ExtraImage_LoadModule()
{
	CreateServiceFunction(MS_CLIST_EXTRA_SET_ICON,WideSetIconForExtraColumn); 
	CreateServiceFunction(MS_CLIST_EXTRA_ADD_ICON,WideAddIconToExtraImageList); 

	//CreateServiceFunction(MS_CLIST_EXTRA2_SET_ICON,WideSetIconForExtraColumn); 
	//CreateServiceFunction(MS_CLIST_EXTRA2_ADD_ICON,WideAddIconToExtraImageList); 
	

	
	//HookEvent(ME_SKIN2_ICONSCHANGED,OnIconLibIconChanged);

	HookEvent(ME_CLC_SHOWEXTRAINFOTIP, ehhShowExtraInfoTip );
	HookEvent(ME_CLC_HIDEINFOTIP, ehhHideExtraInfoTip );
	HookEvent(ME_SYSTEM_SHUTDOWN, ehhExtraImage_UnloadModule );
};





