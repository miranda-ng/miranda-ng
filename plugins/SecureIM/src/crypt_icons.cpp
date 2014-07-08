#include "commonheaders.h"

struct ICON_CACHE
{
	ICON_CACHE::~ICON_CACHE() {
		::DestroyIcon(icon);
	}

	HICON  icon;
	HANDLE hCLIcon;
	SHORT  mode;
};

OBJLIST<ICON_CACHE> arIcoList(10);

// преобразует mode в HICON который НЕ НУЖНО разрушать в конце
static ICON_CACHE& getCacheItem(int mode, int type)
{
	int m = mode & 0x0f, s = (mode & SECURED) >> 4, i; // разобрали на части - режим и состояние
	HICON icon;

	for (i = 0; i < arIcoList.getCount(); i++)
		if (arIcoList[i].mode == ((type << 8) | mode))
			return arIcoList[i];

	i = s;
	switch (type) {
	case 1: i += IEC_CL_DIS; break;
	case 2: i += ICO_CM_DIS; break;
	case 3: i += ICO_MW_DIS; break;
	}

	if (type == 1)
		icon = BindOverlayIcon(g_hIEC[i], g_hICO[ICO_OV_NAT + m]);
	else
		icon = BindOverlayIcon(g_hICO[i], g_hICO[ICO_OV_NAT + m]);

	ICON_CACHE *p = new ICON_CACHE;
	p->icon = icon;
	p->mode = (type << 8) | mode;
	p->hCLIcon = NULL;
	arIcoList.insert(p);

	return *p;
}

HICON mode2icon(int mode, int type)
{
	return getCacheItem(mode, type).icon;
}

HANDLE mode2clicon(int mode, int type)
{
	if (!bASI && !(mode & SECURED))
		return INVALID_HANDLE_VALUE;

	ICON_CACHE &p = getCacheItem(mode, type);
	if (p.hCLIcon == NULL)
		p.hCLIcon = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)p.icon, 0);

	return p.hCLIcon;
}

// обновляет иконки в clist и в messagew
void ShowStatusIcon(MCONTACT hContact, int mode)
{
	MCONTACT hMC = db_mc_getMeta(hContact);

	// обновить иконки в clist
	if (mode != -1) {
		HANDLE hIcon = mode2clicon(mode, 1);
		ExtraIcon_SetIcon(g_hCLIcon, hContact, hIcon);
		if (hMC)
			ExtraIcon_SetIcon(g_hCLIcon, hMC, hIcon);
	}
	else {
		ExtraIcon_Clear(g_hCLIcon, hContact);
		if (hMC)
			ExtraIcon_Clear(g_hCLIcon, hMC);
	}

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = (char*)MODULENAME;
	for (int i = MODE_NATIVE; i < MODE_CNT; i++) {
		sid.dwId = i;
		sid.flags = (mode & SECURED) ? 0 : MBF_DISABLED;
		if (mode == -1 || (mode & 0x0f) != i || isChatRoom(hContact))
			sid.flags |= MBF_HIDDEN;  // отключаем все ненужные иконки
		Srmm_ModifyIcon(hContact, &sid);
		if (hMC)
			Srmm_ModifyIcon(hMC, &sid);
	}
}

void ShowStatusIcon(MCONTACT hContact)
{
	ShowStatusIcon(hContact, isContactSecured(hContact));
}

void ShowStatusIconNotify(MCONTACT hContact)
{
	BYTE mode = isContactSecured(hContact);
	NotifyEventHooks(g_hEvent[(mode&SECURED) != 0], hContact, 0);
	ShowStatusIcon(hContact, mode);
}

void RefreshContactListIcons(void)
{
	for (int i = 0; i < arIcoList.getCount(); i++)
		arIcoList[i].hCLIcon = 0;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		if (isSecureProtocol(hContact))
			ShowStatusIcon(hContact);
}
