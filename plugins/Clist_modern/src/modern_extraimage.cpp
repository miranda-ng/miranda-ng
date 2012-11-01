#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"

#define ExtraImageIconsIndexCount 6

BOOL g_mutex_bSetAllExtraIconsCycle = 0;

static HIMAGELIST hExtraImageList = NULL;
static HIMAGELIST hWideExtraImageList = NULL;

void ExtraImage_SetAllExtraIcons(HWND hwndList,HANDLE hContact);

bool ImageCreated = FALSE;

int ExtraImage_ExtraIDToColumnNum(int extra)
{
	if (extra < 1 || extra > EXTRACOLUMNCOUNT)
		return -1;
	else
		return extra-1;
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
	db_set_b(NULL, CLUIFrameModule, "EnabledColumnCount", (BYTE)EXTRACOLUMNCOUNT);
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNS, EXTRACOLUMNCOUNT, 0);
}

void ExtraImage_ReloadExtraIcons()
{
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNSSPACE,db_get_b(NULL,"CLUI","ExtraColumnSpace",18),0);
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,0,(LPARAM)NULL);
	if (hExtraImageList)
		ImageList_Destroy(hExtraImageList);
	if (hWideExtraImageList)
		ImageList_Destroy(hWideExtraImageList);

	hExtraImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,256);
	hWideExtraImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,256);

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,(WPARAM)hWideExtraImageList,(LPARAM)hExtraImageList);
	SetNewExtraColumnCount();
	NotifyEventHooks(g_CluiData.hEventExtraImageListRebuilding,0,0);
	ImageCreated = TRUE;
}

void ExtraImage_SetAllExtraIcons(HWND hwndList,HANDLE hContact)
{
	if (pcli->hwndContactTree == 0)
		return;

	g_mutex_bSetAllExtraIconsCycle = 1;
	bool hcontgiven = (hContact != 0);

	if (ImageCreated == FALSE)
		ExtraImage_ReloadExtraIcons();

	SetNewExtraColumnCount();

	if (hContact == NULL)
		hContact = db_find_first();

	do {
		char *szProto = NULL;
		HANDLE hItem = hContact;
		if (hItem == 0)
			continue;
		
		pClcCacheEntry pdnce = (pClcCacheEntry)pcli->pfnGetCacheEntry(hItem);
		if (pdnce == NULL)
			continue;

		szProto = pdnce->m_cache_cszProto;
		NotifyEventHooks(g_CluiData.hEventExtraImageApplying, (WPARAM)hContact, 0);
		if (hcontgiven) break;
		Sleep(0);
	}
		while(hContact = db_find_next(hContact));

	g_mutex_bSetAllExtraIconsCycle = 0;
	CLUI__cliInvalidateRect(hwndList,NULL,FALSE);
	Sleep(0);
}

INT_PTR WideSetIconForExtraColumn(WPARAM wParam,LPARAM lParam)
{
	if (pcli->hwndContactTree == 0 || wParam == 0 || lParam == 0)
		return -1;

	pIconExtraColumn piec = (pIconExtraColumn)lParam;
	if (piec->cbSize != sizeof(IconExtraColumn))
		return -1;

	int icol = ExtraImage_ExtraIDToColumnNum(piec->ColumnType);
	if (icol == -1)
		return -1;

	HANDLE hItem = (HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)wParam,0);
	if (hItem == 0)
		return -1;

	if (piec->hImage == (HANDLE)0xFF)
		piec->hImage = (HANDLE)0xFFFF;

	SendMessage(pcli->hwndContactTree,CLM_SETWIDEEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(icol,piec->hImage));
	return 0;
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
	if (hExtraImageList)
		ImageList_Destroy(hExtraImageList);
	if (hWideExtraImageList)
		ImageList_Destroy(hWideExtraImageList);
	return 0;

}
void ExtraImage_LoadModule()
{
	CreateServiceFunction(MS_CLIST_EXTRA_SET_ICON, WideSetIconForExtraColumn);
	CreateServiceFunction(MS_CLIST_EXTRA_ADD_ICON, WideAddIconToExtraImageList);

	HookEvent(ME_SYSTEM_SHUTDOWN, ehhExtraImage_UnloadModule );
};
