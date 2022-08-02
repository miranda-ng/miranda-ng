/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

enum ECascadeType {
	MCAS_DISABLED	= 2,
	MCAS_ALL		= 4,
	MCAS_EXIMPORT	= 8,
	MCAS_NOTINITIATED = 128
};

HGENMENU hMenuItemRefresh = nullptr;
HGENMENU *hMenuItemAccount	= nullptr;

/**
 * Helper function to remove all menu items backward (first item second group).
 * Note: since miranda 0.8 genmenu is part of core (we don't need to check for).
 *
 * @param	pItems		- poiter to HGENMENU array
 * @param	Count		- number of array elements
 *
 * @return	0 on success, -1 on failure
 **/
INT_PTR RemoveMenuItems(HGENMENU *pItems, int Count)
{
	if (!Count || !pItems) {
		return -1;
	}
	while (Count--) {
		if (pItems[Count]) {
			Menu_RemoveItem(pItems[Count]);
			pItems[Count] = nullptr;
		}
	}
	return 0;
}

/**
 * This function rebuilds the contactmenu. If user selected to cascade menus,
 * a root menu item is created which holds the popup for all the other items.
 * Note: since miranda 0.8 genmenu is part of core (we don't need to check for).
 *
 * @param	none
 *
 * @return	nothing
 **/
void RebuildContact()
{
	HGENMENU mhRoot = nullptr;
	HGENMENU mhExIm = nullptr;
	static HGENMENU hMenuItem[4] = { nullptr, nullptr, nullptr, nullptr };

	SvcEMailRebuildMenu();
	SvcHomepageRebuildMenu();

	// load options
	int flag = g_plugin.getByte(SET_MI_CONTACT, MCAS_NOTINITIATED);
	if (flag == MCAS_NOTINITIATED) {
		flag = MCAS_EXIMPORT | TRUE;
		g_plugin.setByte(SET_MI_CONTACT, flag);
	}

	// delete all MenuItems and set all bytes 0 to avoid problems
	RemoveMenuItems(hMenuItem, _countof(hMenuItem));

	// support new genmenu style
	CMenuItem mi(&g_plugin);

	
	switch (flag) {
	case 3:
		//cascade off
		mhRoot = mhExIm = nullptr;
		hMenuItem[0] = nullptr;
		break;
	case 5:
		//cascade all
		SET_UID(mi, 0xeb3736b6, 0x3b6, 0x40fa, 0xa7, 0xe5, 0xbb, 0x40, 0xeb, 0x55, 0xf7, 0x84);
		mi.position = 1000050000;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
		mi.name.a = MODULELONGNAME;
		mhRoot = Menu_AddContactMenuItem(&mi);
		hMenuItem[0] = mhRoot;
		mhExIm = mhRoot;
		break;
	case 9:
		//cascade Ex/Import
		SET_UID(mi, 0x100de420, 0x4835, 0x4f8d, 0xbd, 0x58, 0x4f, 0x31, 0xcc, 0xe6, 0xc1, 0xa8);
		mi.position = 1000050100;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_BTN_EXIMPORT);
		mi.name.a = LPGEN("Export/import contact");
		mhExIm = Menu_AddContactMenuItem(&mi);
		hMenuItem[0] = mhExIm;
		mhRoot = nullptr;
		break;
	default:
		//disable Menu
		return;
	}

	// ContactDetailsPS's menuitem
	SET_UID(mi, 0xe8731d53, 0x95af, 0x42cf, 0xae, 0x27, 0xc7, 0xa7, 0xa, 0xbf, 0x14, 0x1c);
	mi.root = mhRoot;	
	mi.pszService = MS_USERINFO_SHOWDIALOG;
	mi.name.a = LPGEN("User &details");
	mi.position = 1000050000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
	hMenuItem[1] = Menu_AddContactMenuItem(&mi);

	Menu_ConfigureItem(hMenuItem[1], MCI_OPT_HOTKEY, MAKELPARAM(VK_F3, MOD_ALT));

	// VCard's Ex/Import menuitems
	mi.root = mhExIm;

	// Export
	SET_UID(mi, 0xD88AF855, 0x4C85, 0x4D84, 0x9D, 0xAB, 0x8C, 0xEC, 0x5E, 0x8E, 0xED, 0xC2);
	mi.pszService = MS_USERINFO_VCARD_EXPORT;
	mi.name.a = mhExIm != NULL ? LPGEN("&Export") : LPGEN("&Export user details");
	mi.position = 1000050200;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_EXPORT);
	hMenuItem[2] = Menu_AddContactMenuItem(&mi);

	// Import
	SET_UID(mi, 0x3DD52244, 0x0AD8, 0x4A0E, 0xB2, 0x6B, 0xB4, 0xB8, 0x4B, 0x1C, 0x33, 0x7C);
	mi.pszService = MS_USERINFO_VCARD_IMPORT;
	mi.name.a = mhExIm != NULL ? LPGEN("&Import") : LPGEN("&Import user details");
	mi.position = 1000050300;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_IMPORT);
	hMenuItem[3] = Menu_AddContactMenuItem(&mi);
}

/**
 * This function rebuilds the mainmenu. If user selected to cascade menus,
 * a root menu item is created which holds the popup for all the other items.
 * Note: since miranda 0.8 genmenu is part of core (we don't need to check for).
 *
 * @param	none
 *
 * @return	nothing
 **/
void RebuildMain()
{
	uint8_t item = 0;

	HGENMENU mhRoot = nullptr;
	HGENMENU mhExIm = nullptr;
	static HGENMENU hMenuItem[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	// load options
	int flag = g_plugin.getByte(SET_MI_MAIN, MCAS_NOTINITIATED);
	if (flag == MCAS_NOTINITIATED) {
		flag = MCAS_ALL | TRUE;
		g_plugin.setByte(SET_MI_MAIN, flag);
	}

	// delete all MenuItems and set all bytes 0 to avoid problems
	RemoveMenuItems(hMenuItem, _countof(hMenuItem));

	// support new genmenu style
	CMenuItem mi(&g_plugin);

	switch (flag) {
	case 3:
		//cascade off
		mhRoot = mhExIm = nullptr;
		hMenuItem[item++] = nullptr;
		break;
	case 5:
		//cascade all
		SET_UID(mi, 0x17d277d5, 0x2772, 0x40c6, 0xbd, 0xbb, 0x2c, 0x3a, 0xcc, 0xda, 0xc, 0x72);
		mi.position = 500050000;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
		mi.name.a = MODULELONGNAME;
		mhRoot = Menu_AddMainMenuItem(&mi);
		hMenuItem[item++] = mhRoot;
		mhExIm = mhRoot;
		break;
	case 9:
		//cascade Ex/Import
		SET_UID(mi, 0x80666af7, 0x1c92, 0x4ea6, 0xa8, 0xdc, 0x25, 0x88, 0x88, 0x27, 0x92, 0x68);
		mi.position = 500050000;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_BTN_EXIMPORT);
		mi.name.a = LPGEN("Export/import contact");
		mhExIm = Menu_AddMainMenuItem(&mi);
		hMenuItem[item++] = mhExIm;
		mhRoot = nullptr;
		break;
	default:
		//disable Menue
		return;
	}

	// details dialog
	SET_UID(mi, 0x42852ca4, 0x4941, 0x4219, 0x8b, 0x98, 0x33, 0x46, 0x8c, 0x32, 0xd8, 0xb8);
	mi.root = mhRoot;
	mi.pszService = MS_USERINFO_SHOWDIALOG;
	mi.name.a = LPGEN("View/change my &details...");
	mi.position = 500050000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
	hMenuItem[item++] = Menu_AddMainMenuItem(&mi);

	// VCard's Ex/Import menuitems
	mi.root = mhExIm;

	// Export
	SET_UID(mi, 0xf7e939c0, 0xecd4, 0x46d4, 0x8f, 0x77, 0x81, 0xe6, 0xa1, 0x0, 0x1f, 0xde);
	mi.pszService = MS_USERINFO_VCARD_EXPORTALL;
	mi.name.a = LPGEN("Export all contacts");
	mi.position = 500150000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_EXPORT);
	hMenuItem[item++] = Menu_AddMainMenuItem(&mi);

	// Import
	SET_UID(mi, 0x80f7441, 0xa907, 0x48ad, 0xa3, 0xf0, 0x13, 0x1e, 0xc0, 0xef, 0xd0, 0x4d);
	mi.pszService = MS_USERINFO_VCARD_IMPORTALL;
	mi.name.a = LPGEN("Import all contacts");
	mi.position = 500151000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_IMPORT);
	hMenuItem[item++] = Menu_AddMainMenuItem(&mi);

	// reminder
	mi.root = mhRoot;
	bool bRemindMenus = g_plugin.iRemindState && g_plugin.bRemindMenuEnabled;
	if (bRemindMenus) {
		// Check anniversaries
		SET_UID(mi, 0xcc1c8aa1, 0x7d1, 0x4554, 0x9c, 0xfe, 0xc4, 0x1e, 0xb9, 0x5d, 0x4a, 0xf);
		mi.pszService = MS_USERINFO_REMINDER_CHECK;
		mi.name.a = LPGEN("Check anniversaries");
		mi.position = 500251000;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_SEARCH);
		hMenuItem[item++] = Menu_AddMainMenuItem(&mi);
	}
	else {
		hMenuItem[item++] = nullptr;
		hMenuItem[item++] = nullptr;
	}

	// Refresh Contact Details
	SET_UID(mi, 0x8b03ac79, 0x958a, 0x4967, 0x9d, 0x89, 0xd1, 0x4d, 0xf0, 0x91, 0x1c, 0x78);
	mi.pszService = MS_USERINFO_REFRESH;
	mi.name.a = LPGEN("Refresh contact details");
	mi.position = 500254000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_BTN_UPDATE);
	hMenuItem[item++] = Menu_AddMainMenuItem(&mi);

	// anniversary list
	SET_UID(mi, 0x1b5e5630, 0xdaef, 0x40e0, 0xae, 0xb1, 0x62, 0x85, 0x9, 0x8e, 0x5e, 0x45);
	mi.pszService = MS_USERINFO_REMINDER_LIST;
	mi.name.a = LPGEN("Anniversary list");
	mi.position = 500252000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ANNIVERSARY);
	hMenuItem[item++] = Menu_AddMainMenuItem(&mi);
}

/**
 * This function rebuilds the clist context menu (clist main groupmenu). If user selected to
 * cascade menus, a root menu item is created which holds the popup for all the other items.
 * Note: since miranda 0.8 genmenu is part of core (we don't need to check for).
 *
 * @param	none
 *
 * @return	nothing
 **/
void RebuildGroup()
{
	int flag = 0;
	uint8_t item = 0;

	HGENMENU mhRoot = nullptr;
	HGENMENU mhExIm = nullptr;
	static HGENMENU hMenuItem[3] = { nullptr, nullptr, nullptr };

	// load options
	flag = g_plugin.getByte(SET_MI_GROUP, MCAS_NOTINITIATED);
	if (flag == MCAS_NOTINITIATED) {
		flag = MCAS_EXIMPORT | TRUE;
		g_plugin.setByte(SET_MI_GROUP, flag);
	}

	// delete all MenuItems and set all bytes 0 to avoid problems
	RemoveMenuItems(hMenuItem, _countof(hMenuItem));

	// create service name main (prevent to generate {(Null)/Ex-/Import Group} in db) and set pointer to end it
	char text[200];
	mir_strcpy(text, "UserInfo");

	CMenuItem mi(&g_plugin);
	mi.pszService = text;

	switch (flag) {
	case 3:
		//cascade off
		mhRoot = mhExIm = nullptr;
		hMenuItem[item++] = nullptr;
		break;
	case 5:
		//cascade all
		SET_UID(mi, 0xfefe20db, 0x431f, 0x4fef, 0x9d, 0xa6, 0x70, 0xcd, 0x25, 0xf1, 0x2f, 0x1d);
		mi.position = 250000;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
		mi.name.a = MODULELONGNAME;
		mhRoot = Menu_AddGroupMenuItem(&mi);
		hMenuItem[item++] = mhRoot;
		mhExIm = mhRoot;
		break;
	case 9:
		//cascade Ex/Import
		SET_UID(mi, 0x3ec4da1, 0x8a1a, 0x4734, 0xba, 0x99, 0x7, 0x9c, 0x2, 0x46, 0xff, 0xec);
		mi.position = 250100;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_BTN_EXIMPORT);
		mi.name.a = LPGEN("Export/import contact");
		mhExIm = Menu_AddGroupMenuItem(&mi);
		hMenuItem[item++] = mhExIm;
		mhRoot = nullptr;
		break;
	default:
		//disable Menue
		return;
	}

	// VCard's Ex/Import menuitems
	mi.root = mhExIm;

	// Export
	SET_UID(mi, 0x9a0d81ec, 0x6795, 0x421a, 0xb2, 0x79, 0x41, 0xbd, 0xde, 0x29, 0x3b, 0xa4);
	mi.pszService = MS_USERINFO_VCARD_EXPORTALL;
	mi.name.a = LPGEN("Export all contacts");
	mi.position = 250200;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_EXPORT);
	hMenuItem[item++] = Menu_AddGroupMenuItem(&mi);

	// Import
	SET_UID(mi, 0xd6d98c8f, 0x5cdf, 0x4138, 0x88, 0x6c, 0x31, 0x1a, 0x5a, 0x9, 0x56, 0xbb);
	mi.pszService = MS_USERINFO_VCARD_IMPORTALL;
	mi.name.a = LPGEN("Import all contacts");
	mi.position = 250300;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_IMPORT);
	hMenuItem[item++] = Menu_AddGroupMenuItem(&mi);
}

/******************************
 * (Sub)GroupMenu
 ******************************/

/**
 * This function rebuilds the group context menu (clist main groupmenu). If user selected to
 * cascade menus, a root menu item is created which holds the popup for all the other items.
 * Note: since miranda 0.8 genmenu is part of core (we don't need to check for).
 *
 * @param	none
 *
 * @return	nothing
 **/
void RebuildSubGroup()
{
	int flag = 0;
	uint8_t item = 0;
	GroupMenuParam gmp = { 0 };

	HGENMENU mhRoot = nullptr;
	HGENMENU mhExIm = nullptr;
	static HGENMENU hMenuItem[3] = { nullptr, nullptr, nullptr };

	// load options
	flag = g_plugin.getByte(SET_MI_SUBGROUP, MCAS_NOTINITIATED);
	if (flag == MCAS_NOTINITIATED) {
		flag = MCAS_DISABLED | TRUE;
		g_plugin.setByte(SET_MI_SUBGROUP, flag);
	}

	// delete all MenuItems and set all bytes 0 to avoid problems
	RemoveMenuItems(hMenuItem, _countof(hMenuItem));

	// create service name main (prevent to generate {(Null)/Ex-/Import Group} in db) and set pointer to end it
	char text[200];
	mir_strcpy(text, "UserInfo");

	CMenuItem mi(&g_plugin);
	mi.pszService = text;
	char* tDest = text + mir_strlen(text);

	switch (flag) {
	case 3:
		//cascade off
		mhRoot = mhExIm = nullptr;
		hMenuItem[item++] = nullptr;
		break;
	case 5:
		//cascade all
		SET_UID(mi, 0x77b46004, 0xe845, 0x41f4, 0x9a, 0x56, 0x72, 0xd9, 0xa5, 0x61, 0x4d, 0x5);
		mi.position = 1050000;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
		mi.name.a = MODULELONGNAME;
		mhRoot = Menu_AddSubGroupMenuItem(&mi);
		hMenuItem[item++] = mhRoot;
		mhExIm = mhRoot;
		break;
	case 9:
		//cascade Ex/Import
		SET_UID(mi, 0xac5e810b, 0x4b41, 0x477c, 0x98, 0xfe, 0x8, 0x88, 0xec, 0x6f, 0xc3, 0xcf);
		mi.position = 1050100;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_BTN_EXIMPORT);
		mi.name.a = LPGEN("Export/import group");
		mhExIm = Menu_AddSubGroupMenuItem(&mi);
		hMenuItem[item++] = mhExIm;
		mhRoot = nullptr;
		break;
	default:
		//disable Menue
		return;
	}

	// VCard's Ex/Import menuitems
	mi.root = mhExIm;

	// Export
	SET_UID(mi, 0x65be2523, 0x15fd, 0x45ca, 0xae, 0xe6, 0xc2, 0x98, 0xd2, 0xa9, 0xff, 0xd5);
	mir_strcpy(tDest, "/ExportGroup");		//mi.pszService
	if (!ServiceExists(mi.pszService)) CreateServiceFunction(mi.pszService, svcExIm_Group_Service);
	mi.name.a = mhExIm != NULL ? LPGEN("&Export") : LPGEN("&Export group");
	mi.position = 1050200;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_EXPORT);
	gmp.wParam = TRUE;
	hMenuItem[item++] = Menu_AddSubGroupMenuItem(&mi, &gmp);

	// Import
	SET_UID(mi, 0xf6be7278, 0x4adb, 0x4e6a, 0x9f, 0x63, 0x79, 0xda, 0xbb, 0xcd, 0xbe, 0x42);
	mir_strcpy(tDest, "/ImportGroup");		//mi.pszService
	if (!ServiceExists(mi.pszService)) CreateServiceFunction(mi.pszService, svcExIm_Group_Service);
	mi.name.a = mhExIm != NULL ? LPGEN("&Import") : LPGEN("&Import group");
	mi.position = 1050300;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_IMPORT);
	gmp.wParam = FALSE;
	hMenuItem[item++] = Menu_AddSubGroupMenuItem(&mi, &gmp);
}

/******************************
 * Account Menu
 ******************************/

/**
 * This function rebuilds the account context menu (clist status <account>). If user selected to
 * cascade menus, a root menu item is created which holds the popup for all the other items.
 * Note: since miranda 0.8 genmenu is part of core (we don't need to check for).
 *
 * @param	wParam				- 0 not used
 * @param	lParam				- clear bit for old menu items
 *								  0 don't delete old items (its calld by ME_CLIST_PREBUILDSTATUSMENU hook)
 *								  other then 0 delete old items first
 *
 * @return	always 0
 **/
INT_PTR RebuildAccount(WPARAM, LPARAM lParam)
{
	if (Miranda_IsTerminated())
		return 0;

	const uint8_t mItems = 3;				// menuitems to create
	uint8_t item = 0;

	// on call by hook or first start
	if (!lParam || !hMenuItemAccount) {
		size_t sizeNew = mItems * g_clistApi.menuProtos->getCount() * sizeof(HGENMENU);
		hMenuItemAccount = (HGENMENU*)mir_realloc(hMenuItemAccount, sizeNew);
		// set all bytes 0 to avoid problems
		memset(hMenuItemAccount, 0, sizeNew);
	}
	// on options change
	else // delete all MenuItems backward (first item second group)
		RemoveMenuItems(hMenuItemAccount, mItems * g_clistApi.menuProtos->getCount());

	// load options
	int flag = g_plugin.getByte(SET_MI_ACCOUNT, MCAS_NOTINITIATED);
	if (flag == MCAS_NOTINITIATED) {
		flag = MCAS_EXIMPORT | TRUE;
		g_plugin.setByte(SET_MI_ACCOUNT, flag);
	}

	// loop for all account names
	for (auto &it : *g_clistApi.menuProtos) {
		// set all bytes 0 to avoid problems
		item = 0;

		HGENMENU mhRoot = it->pMenu, mhExIm;
		if (mhRoot == nullptr)
			break;

		PROTOACCOUNT *pa = Proto_GetAccount(it->szProto);

		// create service name main (account module name) and set pointer to end it
		char text[200];
		mir_strcpy(text, it->szProto);

		CMenuItem mi(&g_plugin);
		mi.pszService = text;
		char* tDest = text + mir_strlen(text);

		// support new genmenu style
		mi.root = mhRoot;

		int i = g_clistApi.menuProtos->indexOf(&it);
		switch (flag) {
		case 3:
			// cascade off
			mhExIm = mhRoot;
			// seperator
			mi.position = 50100;
			hMenuItemAccount[mItems*i + item++] = Menu_AddStatusMenuItem(&mi, pa->szModuleName);
			break;
		case 5:
			// cascade all
			mi.position = 50100;
			mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
			mi.name.a = MODULELONGNAME;
			hMenuItemAccount[mItems*i + item] = Menu_AddStatusMenuItem(&mi, pa->szModuleName);
			mhRoot = hMenuItemAccount[mItems*i + item++];
			mhExIm = mhRoot;
			break;
		case 9:
			// cascade Ex/Import
			mi.position = 50100;
			mi.hIcolibItem = g_plugin.getIconHandle(IDI_BTN_EXIMPORT);
			mi.name.a = LPGEN("Export/import");
			hMenuItemAccount[mItems*i + item] = Menu_AddStatusMenuItem(&mi, pa->szModuleName);
			mhRoot = hMenuItemAccount[mItems*i + item++];
			mhExIm = mhRoot;
			break;
		default:
			// disable Menue
			return 0;
		}

		// VCard's Ex/Import menuitems
		mi.root = mhExIm;

		// Export
		mir_strcpy(tDest, "/ExportAccount");
		CreateServiceFunction(mi.pszService, svcExIm_Account_Service);
		mi.name.a = LPGEN("&Export xml");
		mi.position = 50200;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_EXPORT);
		hMenuItemAccount[mItems*i + item++] = Menu_AddStatusMenuItem(&mi, pa->szModuleName);

		// Import
		mir_strcpy(tDest, "/ImportAccount");
		CreateServiceFunction(mi.pszService, svcExIm_Account_Service);
		mi.name.a = LPGEN("&Import xml");
		mi.position = 50300;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_IMPORT);
		hMenuItemAccount[mItems*i + item++] = Menu_AddStatusMenuItem(&mi, pa->szModuleName);
	}
	return 0;
}

void RebuildMenu()
{
	RebuildMain();
	RebuildContact();
	RebuildGroup();
	RebuildSubGroup();
	RebuildAccount(NULL, 1);
}
