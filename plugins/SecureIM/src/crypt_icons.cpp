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

	for (auto &it : arIcoList)
		if (it->mode == ((type << 8) | mode))
			return *it;

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
	p->hCLIcon = nullptr;
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
	if (p.hCLIcon == nullptr)
		p.hCLIcon = ExtraIcon_AddIcon(p.icon);

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

	for (int i = MODE_NATIVE; i < MODE_CNT; i++) {
		int flags = (mode & SECURED) ? 0 : MBF_DISABLED;
		if (mode == -1 || (mode & 0x0f) != i || isChatRoom(hContact))
			flags |= MBF_HIDDEN;  // отключаем все ненужные иконки
		Srmm_SetIconFlags(hContact, MODULENAME, i, flags);
		if (hMC)
			Srmm_SetIconFlags(hMC, MODULENAME, i, flags);
	}
}

void ShowStatusIcon(MCONTACT hContact)
{
	ShowStatusIcon(hContact, isContactSecured(hContact));
}

void ShowStatusIconNotify(MCONTACT hContact)
{
	uint8_t mode = isContactSecured(hContact);
	NotifyEventHooks(g_hEvent[(mode&SECURED) != 0], hContact, 0);
	ShowStatusIcon(hContact, mode);
}

void RefreshContactListIcons(void)
{
	for (auto &it : arIcoList)
		it->hCLIcon = nullptr;

	for (auto &hContact : Contacts())
		if (isSecureProtocol(hContact))
			ShowStatusIcon(hContact);
}
