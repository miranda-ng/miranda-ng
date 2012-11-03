#include "commonheaders.h"

typedef struct {
	HICON icon;
	SHORT mode;
} ICON_CACHE;


ICON_CACHE *ICONS_CACHE = NULL;
int icons_cache = 0;


// преобразует mode в HICON который НЕ НУЖНО разрушать в конце
HICON mode2icon(int mode,int type) {

	int m=mode&0x0f,s=(mode&SECURED)>>4,i; // разобрали на части - режим и состояние
	HICON icon;

	if ( icons_cache ) {
		for(i=0;i<icons_cache;i++) {
			if ( ICONS_CACHE[i].mode == ((type<<8) | mode)) {
				return ICONS_CACHE[i].icon;
			}
		}
	}

	i=s;
	switch(type) {
	case 1: i+=IEC_CL_DIS; break;
	case 2: i+=ICO_CM_DIS; break;
	case 3: i+=ICO_MW_DIS; break;
	}

	if ( type==1 ) {
		icon = BindOverlayIcon(g_hIEC[i],g_hICO[ICO_OV_NAT+m]);
	}
	else {
		icon = BindOverlayIcon(g_hICO[i],g_hICO[ICO_OV_NAT+m]);
	}

	ICONS_CACHE = (ICON_CACHE*) mir_realloc(ICONS_CACHE,sizeof(ICON_CACHE)*(icons_cache+1));
	ICONS_CACHE[icons_cache].icon = icon;
	ICONS_CACHE[icons_cache].mode = (type<<8) | mode;
	icons_cache++;

	return icon;
}

// обновляет иконки в clist и в messagew
void ShowStatusIcon(HANDLE hContact,int mode)
{
	HANDLE hMC = getMetaContact(hContact);

	// обновить иконки в clist
	if (mode != -1) {
		HICON hIcon = mode2icon(mode, 1);
		ExtraIcon_SetIcon(g_hCLIcon, hContact, hIcon);
		if ( hMC )
			ExtraIcon_SetIcon(g_hCLIcon, hMC, hIcon);
	}

	if ( ServiceExists(MS_MSG_MODIFYICON)) {  // обновить иконки в srmm
		StatusIconData sid;
		memset(&sid,0,sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.szModule = (char*)szModuleName;
		for(int i=MODE_NATIVE; i<MODE_CNT;i++) {
			sid.dwId = i;
			sid.flags = (mode & SECURED) ? 0 : MBF_DISABLED;
			if (mode == -1 || (mode & 0x0f) != i || isChatRoom(hContact))
				sid.flags |= MBF_HIDDEN;  // отключаем все ненужные иконки
			CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
			if ( hMC )
				CallService(MS_MSG_MODIFYICON, (WPARAM)hMC, (LPARAM)&sid);
		}
	}
}

void ShowStatusIcon(HANDLE hContact)
{
	ShowStatusIcon(hContact,isContactSecured(hContact));
}

void ShowStatusIconNotify(HANDLE hContact)
{
	int mode = isContactSecured(hContact);
	NotifyEventHooks(g_hEvent[(mode&SECURED)!=0], (WPARAM)hContact, 0);
	ShowStatusIcon(hContact,mode);
}

void RefreshContactListIcons(void)
{
	HANDLE hContact = db_find_first();
	while (hContact) { // и снова зажигаем иконку
		if ( isSecureProtocol(hContact))
			ShowStatusIcon(hContact);
		hContact = db_find_next(hContact);
	}
}

// EOF
