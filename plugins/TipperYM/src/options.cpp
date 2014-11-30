/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "common.h"

OPTIONS opt;
ICONSTATE exIcons[EXICONS_COUNT];

extern int IsTrayProto(const TCHAR *swzProto, BOOL bExtendedTip)
{
	if (swzProto == NULL)
		return 0;

	char szSetting[64];
	if (bExtendedTip)
		strcpy(szSetting, "TrayProtocolsEx");
	else
		strcpy(szSetting, "TrayProtocols");

	DBVARIANT dbv;
	int result = 1;
	if (!db_get_ts(NULL, MODULE, szSetting, &dbv))
	{
		result = _tcsstr(dbv.ptszVal, swzProto) ? 1 : 0;
		db_free(&dbv);
	}

	return result;
}

void CreateDefaultItems()
{
	DSListNode *ds_node;
	DIListNode *di_node;

	for (int i = 0; defaultItemList[i].szName; i++)
	{
		if (defaultItemList[i].szName[0] == '-')
		{
			di_node = (DIListNode *)mir_alloc(sizeof(DIListNode));
			_tcsncpy(di_node->di.swzLabel, _T(""), LABEL_LEN);
			_tcsncpy(di_node->di.swzValue, _T(""), VALUE_LEN);
			di_node->di.bLineAbove = true;
			di_node->di.bIsVisible = true;
			di_node->di.bParseTipperVarsFirst = false;
			di_node->next = opt.diList;
			opt.diList = di_node;
			opt.iDiCount++;
		}
		else
		{
			PRESETITEM *item = GetPresetItemByName(defaultItemList[i].szName);
			if (item == NULL) continue;

			for (int j = 0; j < MAX_PRESET_SUBST_COUNT; j++)
			{
				PRESETSUBST *subst = GetPresetSubstByName(item->szNeededSubst[j]);
				if (subst == NULL) continue;

				ds_node = (DSListNode *)mir_alloc(sizeof(DSListNode));
				_tcsncpy(ds_node->ds.swzName, subst->swzName, LABEL_LEN);
				ds_node->ds.type = subst->type;
				strncpy(ds_node->ds.szSettingName, subst->szSettingName, SETTING_NAME_LEN);
				ds_node->ds.iTranslateFuncId = subst->iTranslateFuncId;
				ds_node->next = opt.dsList;
				opt.dsList = ds_node;
				opt.iDsCount++;
			}

			di_node = (DIListNode *)mir_alloc(sizeof(DIListNode));
			_tcsncpy(di_node->di.swzLabel, TranslateTS(item->swzLabel), LABEL_LEN);
			_tcsncpy(di_node->di.swzValue, item->swzValue, VALUE_LEN);
			di_node->di.bLineAbove = false;
			di_node->di.bValueNewline = defaultItemList[i].bValueNewline;
			di_node->di.bIsVisible = true;
			di_node->di.bParseTipperVarsFirst = false;
			di_node->next = opt.diList;
			opt.diList = di_node;
			opt.iDiCount++;
		}
	}
}

bool LoadDS(DISPLAYSUBST *ds, int index)
{
	char setting[512];
	DBVARIANT dbv;

	mir_snprintf(setting, SIZEOF(setting), "Name%d", index);
	ds->swzName[0] = 0;
	if (!db_get_ts(0, MODULE_ITEMS, setting, &dbv))
	{
		_tcsncpy(ds->swzName, dbv.ptszVal, SIZEOF(ds->swzName));
		ds->swzName[SIZEOF(ds->swzName) - 1] = 0;
		db_free(&dbv);
	}
	else
		return false;

	mir_snprintf(setting, SIZEOF(setting), "Type%d", index);
	ds->type = (DisplaySubstType)db_get_b(0, MODULE_ITEMS, setting, DVT_PROTODB);

	mir_snprintf(setting, SIZEOF(setting), "Module%d", index);
	ds->szModuleName[0] = 0;
	if (!db_get(0, MODULE_ITEMS, setting, &dbv))
	{
		strncpy(ds->szModuleName, dbv.pszVal, MODULE_NAME_LEN);
		ds->szModuleName[MODULE_NAME_LEN - 1] = 0;
		db_free(&dbv);
	}

	mir_snprintf(setting, SIZEOF(setting), "Setting%d", index);
	ds->szSettingName[0] = 0;
	if (!db_get(0, MODULE_ITEMS, setting, &dbv))
	{
		strncpy(ds->szSettingName, dbv.pszVal, SETTING_NAME_LEN);
		ds->szSettingName[SETTING_NAME_LEN - 1] = 0;
		db_free(&dbv);
	}

	mir_snprintf(setting, SIZEOF(setting), "TransFuncId%d", index);
	ds->iTranslateFuncId = db_get_dw(0, MODULE_ITEMS, setting, (DWORD)-1);

	// a little backward compatibility
	if ((DWORD)ds->iTranslateFuncId == (DWORD)-1)
	{
		mir_snprintf(setting, SIZEOF(setting), "TransFunc%d", index);
		ds->iTranslateFuncId = (DWORD)db_get_w(0, MODULE_ITEMS, setting, 0);
	}

	return true;
}

void SaveDS(DISPLAYSUBST *ds, int index)
{
	char setting[512];

	mir_snprintf(setting, SIZEOF(setting), "Name%d", index);
	db_set_ts(0, MODULE_ITEMS, setting, ds->swzName);
	mir_snprintf(setting, SIZEOF(setting), "Type%d", index);
	db_set_b(0, MODULE_ITEMS, setting, (BYTE)ds->type);
	mir_snprintf(setting, SIZEOF(setting), "Module%d", index);
	db_set_s(0, MODULE_ITEMS, setting, ds->szModuleName);
	mir_snprintf(setting, SIZEOF(setting), "Setting%d", index);
	db_set_s(0, MODULE_ITEMS, setting, ds->szSettingName);
	mir_snprintf(setting, SIZEOF(setting), "TransFuncId%d", index);
	db_set_dw(0, MODULE_ITEMS, setting, (WORD)ds->iTranslateFuncId);
}

bool LoadDI(DISPLAYITEM *di, int index)
{
	char setting[512];
	DBVARIANT dbv;

	mir_snprintf(setting, SIZEOF(setting), "DILabel%d", index);
	di->swzLabel[0] = 0;
	if (!db_get_ts(0, MODULE_ITEMS, setting, &dbv))
	{
		_tcsncpy( di->swzLabel, dbv.ptszVal, SIZEOF(di->swzLabel));
		di->swzLabel[SIZEOF(di->swzLabel) - 1] = 0;
		db_free(&dbv);
	} else
		return false;

	mir_snprintf(setting, SIZEOF(setting), "DIValue%d", index);
	di->swzValue[0] = 0;
	if (!db_get_ts(0, MODULE_ITEMS, setting, &dbv))
	{
		_tcsncpy(di->swzValue, dbv.ptszVal, SIZEOF(di->swzValue));
		di->swzValue[SIZEOF(di->swzValue) - 1] = 0;
		db_free(&dbv);
	}

	mir_snprintf(setting, SIZEOF(setting), "DIType%d", index);
	di->type = (DisplayItemType)db_get_b(0, MODULE_ITEMS, setting, DIT_ALL);
	mir_snprintf(setting, SIZEOF(setting), "DILineAbove%d", index);
	di->bLineAbove = (db_get_b(0, MODULE_ITEMS, setting, 0) == 1);
	mir_snprintf(setting, SIZEOF(setting), "DIValNewline%d", index);
	di->bValueNewline = (db_get_b(0, MODULE_ITEMS, setting, 0) == 1);
	mir_snprintf(setting, SIZEOF(setting), "DIVisible%d", index);
	di->bIsVisible = (db_get_b(0, MODULE_ITEMS, setting, 1) == 1);
	mir_snprintf(setting, SIZEOF(setting), "DITipperVarsFirst%d", index);
	di->bParseTipperVarsFirst = (db_get_b(0, MODULE_ITEMS, setting, 0) == 1);

	return true;
}

void SaveDI(DISPLAYITEM *di, int index)
{
	char setting[512];

	mir_snprintf(setting, SIZEOF(setting), "DILabel%d", index);
	if (db_set_ts(0, MODULE_ITEMS, setting, di->swzLabel))
	{
		char buff[LABEL_LEN];
		t2a(di->swzLabel, buff, LABEL_LEN);
		db_set_s(0, MODULE_ITEMS, setting, buff);
	}

	mir_snprintf(setting, SIZEOF(setting), "DIValue%d", index);
	if (db_set_ts(0, MODULE_ITEMS, setting, di->swzValue))
	{
		char buff[VALUE_LEN];
		t2a(di->swzValue, buff, VALUE_LEN);
		db_set_s(0, MODULE_ITEMS, setting, buff);
	}

	mir_snprintf(setting, SIZEOF(setting), "DIType%d", index);
	db_set_b(0, MODULE_ITEMS, setting, (BYTE)di->type);
	mir_snprintf(setting, SIZEOF(setting), "DILineAbove%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bLineAbove ? 1 : 0);
	mir_snprintf(setting, SIZEOF(setting), "DIValNewline%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bValueNewline ? 1 : 0);
	mir_snprintf(setting, SIZEOF(setting), "DIVisible%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bIsVisible ? 1 : 0);
	mir_snprintf(setting, SIZEOF(setting), "DITipperVarsFirst%d", index);
	db_set_b(0, MODULE_ITEMS, setting, di->bParseTipperVarsFirst ? 1 : 0);
}

void SaveOptions()
{
	db_set_dw(0, MODULE, "MaxWidth", opt.iWinWidth);
	db_set_dw(0, MODULE, "MaxHeight", opt.iWinMaxHeight);
	db_set_b(0, MODULE, "AvatarOpacity", (BYTE)opt.iAvatarOpacity);
	db_set_b(0, MODULE, "AvatarRoundCorners", (opt.bAvatarRound ? 1 : 0));
	db_set_b(0, MODULE, "TitleIconLayout", (BYTE)opt.titleIconLayout);
	db_set_b(0, MODULE, "TitleShow", (opt.bShowTitle ? 1 : 0));
	if (ServiceExists(MS_AV_DRAWAVATAR))
		db_set_b(0, MODULE, "AVLayout", (BYTE)opt.avatarLayout);
	opt.bWaitForAvatar = (opt.avatarLayout == PAV_NONE) ? false : true;

	db_set_dw(0, MODULE, "AVSize", opt.iAvatarSize);
	db_set_dw(0, MODULE, "TextIndent", opt.iTextIndent);
	db_set_dw(0, MODULE, "TitleIndent", opt.iTitleIndent);
	db_set_dw(0, MODULE, "ValueIndent", opt.iValueIndent);
	db_set_b(0, MODULE, "ShowNoFocus", (opt.bShowNoFocus ? 1 : 0));

	db_set_w(0, MODULE, "TimeIn", opt.iTimeIn);
	CallService(MS_CLC_SETINFOTIPHOVERTIME, opt.iTimeIn, 0);

	db_set_w(0, MODULE, "Padding", opt.iPadding);
	db_set_w(0, MODULE, "OuterAvatarPadding", opt.iOuterAvatarPadding);
	db_set_w(0, MODULE, "InnerAvatarPadding", opt.iInnerAvatarPadding);
	db_set_w(0, MODULE, "TextPadding", opt.iTextPadding);
	db_set_b(0, MODULE, "Position", (BYTE)opt.pos);
	db_set_dw(0, MODULE, "MinWidth", (DWORD)opt.iMinWidth);
	db_set_dw(0, MODULE, "MinHeight", (DWORD)opt.iMinHeight);
	db_set_dw(0, MODULE, "SidebarWidth", (DWORD)opt.iSidebarWidth);
	db_set_b(0, MODULE, "MouseTollerance", (BYTE)opt.iMouseTollerance);
	db_set_b(0, MODULE, "SBarTips", (opt.bStatusBarTips ? 1 : 0));

	db_set_w(0, MODULE, "LabelVAlign", opt.iLabelValign);
	db_set_w(0, MODULE, "LabelHAlign", opt.iLabelHalign);
	db_set_w(0, MODULE, "ValueVAlign", opt.iValueValign);
	db_set_w(0, MODULE, "ValueHAlign", opt.iValueHalign);

	db_set_b(0, MODULE, "OriginalAvSize", (opt.bOriginalAvatarSize ? 1 : 0));
	db_set_b(0, MODULE, "AvatarBorder", (opt.bAvatarBorder ? 1 : 0));
}

void SaveItems()
{
	int index = 0;
	DSListNode *ds_node = opt.dsList;
	while (ds_node)
	{
		SaveDS(&ds_node->ds, index);
		ds_node = ds_node->next;
		index++;
	}

	db_set_w(0, MODULE_ITEMS, "DSNumValues", index);

	index = 0;
	DIListNode *di_node = opt.diList;
	opt.bWaitForStatusMsg = false;
	while (di_node)
	{
		SaveDI(&di_node->di, index);
		if (di_node->di.bIsVisible && _tcsstr(di_node->di.swzValue, _T("sys:status_msg")))
			opt.bWaitForStatusMsg = true;
		di_node = di_node->next;
		index++;
	}

	db_set_w(0, MODULE_ITEMS, "DINumValues", index);
}

void SaveSkinOptions()
{
	db_set_b(0, MODULE, "Border", (opt.bBorder ? 1 : 0));
	db_set_b(0, MODULE, "DropShadow", (opt.bDropShadow ? 1 : 0));
	db_set_b(0, MODULE, "RoundCorners", (opt.bRound ? 1 : 0));
	db_set_b(0, MODULE, "AeroGlass", (opt.bAeroGlass ? 1 : 0));
	db_set_b(0, MODULE, "Opacity", (BYTE)opt.iOpacity);
	db_set_b(0, MODULE, "ShowEffect", (BYTE)opt.showEffect);
	db_set_b(0, MODULE, "ShowEffectSpeed", (BYTE)opt.iAnimateSpeed);
	db_set_b(0, MODULE, "LoadFonts", (opt.bLoadFonts ? 1 : 0));
	db_set_b(0, MODULE, "LoadProportions", (opt.bLoadProportions ? 1 : 0));
	db_set_dw(0, MODULE, "EnableColoring", opt.iEnableColoring);
}

void LoadObsoleteSkinSetting()
{
	char setting[128];
	DBVARIANT dbv;

	for (int i = 0; i < SKIN_ITEMS_COUNT; i++)
	{
		mir_snprintf(setting, 128, "SPaintMode%d", i);
		opt.transfMode[i] = (TransformationMode)db_get_b(0, MODULE, setting, 0);
		mir_snprintf(setting, 128, "SImgFile%d", i);
		if (!db_get_ts(NULL, MODULE, setting, &dbv))
		{
			opt.szImgFile[i] = mir_tstrdup(dbv.ptszVal);
			db_free(&dbv);
		}

		mir_snprintf(setting, 128, "SGlyphMargins%d", i);
		DWORD margins = db_get_dw(NULL, MODULE, setting, 0);
		opt.margins[i].top = LOBYTE(LOWORD(margins));
		opt.margins[i].right = HIBYTE(LOWORD(margins));
		opt.margins[i].bottom = LOBYTE(HIWORD(margins));
		opt.margins[i].left = HIBYTE(HIWORD(margins));
	}
}

void LoadOptions()
{
	opt.iWinWidth = db_get_dw(0, MODULE, "MaxWidth", 420);
	opt.iWinMaxHeight = db_get_dw(0, MODULE, "MaxHeight", 400);
	opt.iAvatarOpacity = db_get_b(0, MODULE, "AvatarOpacity", 100);
	if (opt.iAvatarOpacity > 100) opt.iAvatarOpacity = 100;
	opt.bAvatarRound = (db_get_b(0, MODULE, "AvatarRoundCorners", opt.bRound ? 1 : 0) == 1);
	opt.titleIconLayout = (PopupIconTitleLayout)db_get_b(0, MODULE, "TitleIconLayout", (BYTE)PTL_LEFTICON);
	opt.bShowTitle = (db_get_b(0, MODULE, "TitleShow", 1) == 1);
	if (ServiceExists(MS_AV_DRAWAVATAR))
		opt.avatarLayout = (PopupAvLayout)db_get_b(0, MODULE, "AVLayout", PAV_RIGHT);
	else
		opt.avatarLayout = PAV_NONE;

	opt.bWaitForAvatar = (opt.avatarLayout == PAV_NONE) ? false : true;
	opt.iAvatarSize = db_get_dw(0, MODULE, "AVSize", 60); //tweety
	opt.iTextIndent = db_get_dw(0, MODULE, "TextIndent", 22);
	opt.iTitleIndent = db_get_dw(0, MODULE, "TitleIndent", 22);
	opt.iValueIndent = db_get_dw(0, MODULE, "ValueIndent", 10);
	opt.iSidebarWidth = db_get_dw(0, MODULE, "SidebarWidth", 22);
	opt.bShowNoFocus = (db_get_b(0, MODULE, "ShowNoFocus", 1) == 1);

	int i, real_count = 0;
	opt.dsList = 0;
	DSListNode *ds_node;

	opt.iDsCount = db_get_w(0, MODULE_ITEMS, "DSNumValues", 0);
	for (i = opt.iDsCount - 1; i >= 0; i--)
	{
		ds_node = (DSListNode *)mir_alloc(sizeof(DSListNode));
		if (LoadDS(&ds_node->ds, i))
		{
			ds_node->next = opt.dsList;
			opt.dsList = ds_node;
			real_count++;
		}
		else
			mir_free(ds_node);
	}
	opt.iDsCount = real_count;

	real_count = 0;
	opt.diList = 0;
	DIListNode *di_node;

	opt.bWaitForStatusMsg = false;
	opt.iDiCount = db_get_w(0, MODULE_ITEMS, "DINumValues", 0);
	for (i = opt.iDiCount - 1; i >= 0; i--)
	{
		di_node = (DIListNode *)mir_alloc(sizeof(DIListNode));
		if (LoadDI(&di_node->di, i))
		{
			di_node->next = opt.diList;
			opt.diList = di_node;
			real_count++;
			if (di_node->di.bIsVisible && _tcsstr(di_node->di.swzValue, _T("sys:status_msg")))
				opt.bWaitForStatusMsg = true;
		}
		else
			mir_free(di_node);
	}
	opt.iDiCount = real_count;

	opt.iTimeIn = db_get_w(0, MODULE, "TimeIn", 750);
	opt.iPadding = db_get_w(0, MODULE, "Padding", 4);
	opt.iOuterAvatarPadding = db_get_w(0, MODULE, "OuterAvatarPadding", 6);
	opt.iInnerAvatarPadding = db_get_w(0, MODULE, "InnerAvatarPadding", 10);
	opt.iTextPadding = db_get_w(0, MODULE, "TextPadding", 4);
	opt.pos = (PopupPosition)db_get_b(0, MODULE, "Position", (BYTE)PP_BOTTOMRIGHT);
	opt.iMinWidth = db_get_dw(0, MODULE, "MinWidth", 0);
	opt.iMinHeight = db_get_dw(0, MODULE, "MinHeight", 0);

	opt.iMouseTollerance = db_get_b(0, MODULE, "MouseTollerance", (BYTE)GetSystemMetrics(SM_CXSMICON));
	opt.bStatusBarTips = (db_get_b(0, MODULE, "SBarTips", 1) == 1);

	// convert defunct last message and status message options to new 'sys' items, and remove the old settings
	if (db_get_b(0, MODULE, "ShowLastMessage", 0))
	{
		db_unset(0, MODULE, "ShowLastMessage");

		// find end of list
		di_node = opt.diList;
		while (di_node && di_node->next)
			di_node = di_node->next;

		// last message item
		if (di_node)
		{
			di_node->next = (DIListNode *)mir_alloc(sizeof(DIListNode));
			di_node = di_node->next;
		}
		else
		{
			opt.diList = (DIListNode *)mir_alloc(sizeof(DIListNode));
			di_node = opt.diList;
		}

		_tcsncpy(di_node->di.swzLabel, _T("Last message: (%sys:last_msg_reltime% ago)"), LABEL_LEN);
		_tcsncpy(di_node->di.swzValue, _T("%sys:last_msg%"), VALUE_LEN);
		di_node->di.bLineAbove = di_node->di.bValueNewline = true;
		di_node->next = 0;
		opt.iDiCount++;
	}

	if (db_get_b(0, MODULE, "ShowStatusMessage", 0))
	{
		db_unset(0, MODULE, "ShowStatusMessage");

		// find end of list
		di_node = opt.diList;
		while (di_node && di_node->next)
			di_node = di_node->next;

		// status message item
		if (di_node)
		{
			di_node->next = (DIListNode *)mir_alloc(sizeof(DIListNode));
			di_node = di_node->next;
		}
		else
		{
			opt.diList = (DIListNode *)mir_alloc(sizeof(DIListNode));
			di_node = opt.diList;
		}

		_tcsncpy(di_node->di.swzLabel, _T("Status message:"), LABEL_LEN);
		_tcsncpy(di_node->di.swzValue, _T("%sys:status_msg%"), VALUE_LEN);
		di_node->di.bLineAbove = di_node->di.bValueNewline = true;
		di_node->next = 0;
		opt.iDiCount++;
	}

	opt.iLabelValign = db_get_w(0, MODULE, "LabelVAlign", DT_TOP /*DT_VCENTER*/);
	opt.iLabelHalign = db_get_w(0, MODULE, "LabelHAlign", DT_LEFT);
	opt.iValueValign = db_get_w(0, MODULE, "ValueVAlign", DT_TOP /*DT_VCENTER*/);
	opt.iValueHalign = db_get_w(0, MODULE, "ValueHAlign", DT_LEFT);

	// tray tooltip
	opt.bTraytip = db_get_b(0, MODULE, "TrayTip", 1) ? true : false;
	opt.bHandleByTipper = db_get_b(0, MODULE, "ExtendedTrayTip", 1) ? true : false;
	opt.bExpandTraytip = db_get_b(0, MODULE, "ExpandTrayTip", 1) ? true : false;
	opt.bHideOffline = db_get_b(0, MODULE, "HideOffline", 0) ? true : false;
	opt.iExpandTime = db_get_dw(0, MODULE, "ExpandTime", 1000);
	opt.iFirstItems = db_get_dw(0, MODULE, "TrayTipItems", TRAYTIP_NUMCONTACTS | TRAYTIP_LOGON | TRAYTIP_STATUS | TRAYTIP_CLIST_EVENT);
	opt.iSecondItems = db_get_dw(0, MODULE, "TrayTipItemsEx", TRAYTIP_NUMCONTACTS | TRAYTIP_LOGON | TRAYTIP_STATUS | TRAYTIP_STATUS_MSG | TRAYTIP_EXTRA_STATUS | TRAYTIP_MIRANDA_UPTIME | TRAYTIP_CLIST_EVENT);
	opt.iFavoriteContFlags = db_get_dw(0, MODULE, "FavContFlags", FAVCONT_APPEND_PROTO);

	// extra setting
	opt.bWaitForContent= db_get_b(0, MODULE, "WaitForContent", 0) ? true : false;
	opt.bGetNewStatusMsg = db_get_b(0, MODULE, "GetNewStatusMsg", 0) ? true : false;
	opt.bDisableIfInvisible = db_get_b(0, MODULE, "DisableInvisible", 1) ? true : false;
	opt.bRetrieveXstatus = db_get_b(0, MODULE, "RetrieveXStatus", 0) ? true : false;
	opt.bOriginalAvatarSize = db_get_b(0, MODULE, "OriginalAvSize", 0) ? true : false;
	opt.bAvatarBorder = db_get_b(0, MODULE, "AvatarBorder", 0) ? true : false;
	opt.bLimitMsg = db_get_b(0, MODULE, "LimitMsg", 0) ? true : false;
	opt.iLimitCharCount = db_get_b(0, MODULE, "LimitCharCount", 64);
	opt.iSmileyAddFlags = db_get_dw(0, MODULE, "SmileyAddFlags", SMILEYADD_ENABLE);

	DBVARIANT dbv;
	// Load the icons order
	for (i = 0; i < EXICONS_COUNT; i++)
	{
		opt.exIconsOrder[i]=i;
		opt.exIconsVis[i]=1;
	}

	if (!db_get(NULL, MODULE, "IconOrder", &dbv))
	{
		CopyMemory(opt.exIconsOrder,dbv.pbVal,dbv.cpbVal);
		db_free(&dbv);
	}

	if (!db_get(NULL, MODULE, "icons_vis", &dbv))
	{
		CopyMemory(opt.exIconsVis,dbv.pbVal,dbv.cpbVal);
		db_free(&dbv);
	}

	for (i = 0; i < EXICONS_COUNT; i++)
	{
		exIcons[i].order = opt.exIconsOrder[i];
		exIcons[i].vis = opt.exIconsVis[i];
	}

	opt.iOpacity = db_get_b(0, MODULE, "Opacity", 75);
	opt.bBorder = db_get_b(0, MODULE, "Border", 1) ? true : false;
	opt.bDropShadow = db_get_b(0, MODULE, "DropShadow", 1) ? true : false;
	opt.bRound = db_get_b(0, MODULE, "RoundCorners", 1) ? true : false;
	opt.bAeroGlass = db_get_b(0, MODULE, "AeroGlass", 0) ? true : false;
	opt.showEffect = (PopupShowEffect)db_get_b(0, MODULE, "ShowEffect", (BYTE)PSE_FADE);
	opt.iAnimateSpeed = db_get_b(0, MODULE, "ShowEffectSpeed", 12);

	if (opt.iAnimateSpeed < 1)
		opt.iAnimateSpeed = 1;
	else if (opt.iAnimateSpeed > 20)
		opt.iAnimateSpeed = 20;

	int iBgImg = db_get_b(0, MODULE, "SBgImage", 0);
	opt.skinMode = (SkinMode)db_get_b(0, MODULE, "SkinEngine", iBgImg ? SM_OBSOLOTE : SM_COLORFILL);
	opt.bLoadFonts = db_get_b(0, MODULE, "LoadFonts", 1) ? true : false;
	opt.bLoadProportions= db_get_b(0, MODULE, "LoadProportions", 1) ? true : false;
	opt.iEnableColoring = db_get_dw(0, MODULE, "EnableColoring", 0);
	opt.szSkinName[0] = 0;

	if (opt.skinMode == SM_OBSOLOTE)
	{
		LoadObsoleteSkinSetting();
	}
	else if (opt.skinMode == SM_IMAGE)
	{
		if (!db_get_ts(NULL, MODULE, "SkinName", &dbv))
		{
			_tcscpy(opt.szSkinName, dbv.ptszVal);
			db_free(&dbv);
		}
	}

	if (opt.iDsCount == 0 && opt.iDiCount == 0)
	{
		// set up some reasonable defaults
		CreateDefaultItems();
		SaveOptions();
		SaveItems();
	}
}

INT_PTR CALLBACK DlgProcAddItem(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DISPLAYITEM *di = (DISPLAYITEM *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault( hwndDlg );
			di = (DISPLAYITEM *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)di);

			SetDlgItemText(hwndDlg, IDC_ED_LABEL, di->swzLabel);
			SetDlgItemText(hwndDlg, IDC_ED_VALUE, di->swzValue);

			for (int i = 0; i < SIZEOF(displayItemTypes); i++)
			{
				int index = SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_ADDSTRING, (WPARAM)-1, (LPARAM)TranslateTS(displayItemTypes[i].title));
				SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_SETITEMDATA, index, (LPARAM)displayItemTypes[i].type);
				if (displayItemTypes[i].type == di->type)
					SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_SETCURSEL, index, 0);
			}

			CheckDlgButton(hwndDlg, IDC_CHK_LINEABOVE, di->bLineAbove ? TRUE : FALSE);
			CheckDlgButton(hwndDlg, IDC_CHK_VALNEWLINE, di->bValueNewline ? TRUE : FALSE);
			CheckDlgButton(hwndDlg, IDC_CHK_PARSETIPPERFIRST, di->bParseTipperVarsFirst ? TRUE : FALSE);

			for (int i = 0; presetItems[i].szID; i++)
				SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_ADDSTRING, 0, (LPARAM)TranslateTS(presetItems[i].swzName));

			variables_skin_helpbutton(hwndDlg, IDC_BTN_VARIABLE);

			SetFocus(GetDlgItem(hwndDlg, IDC_ED_LABEL));
			return TRUE;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDOK:
					{
						GetDlgItemText(hwndDlg, IDC_ED_LABEL, di->swzLabel, LABEL_LEN);
						GetDlgItemText(hwndDlg, IDC_ED_VALUE, di->swzValue, VALUE_LEN);

						int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_GETCURSEL, 0, 0);
						int type = SendDlgItemMessage(hwndDlg, IDC_CMB_TYPE, CB_GETITEMDATA, sel, 0);
						for (int i = 0; i < SIZEOF(displayItemTypes); i++)
						{
							if (displayItemTypes[i].type == type)
								di->type = displayItemTypes[i].type;
						}

						di->bLineAbove = (IsDlgButtonChecked(hwndDlg, IDC_CHK_LINEABOVE) ? true : false);
						di->bValueNewline = (IsDlgButtonChecked(hwndDlg, IDC_CHK_VALNEWLINE) ? true : false);
						di->bParseTipperVarsFirst = (IsDlgButtonChecked(hwndDlg, IDC_CHK_PARSETIPPERFIRST) ? true : false);

						sel = SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETCURSEL, 0, 0);
						if (sel != CB_ERR)
						{
							TCHAR buff[256];
							SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETLBTEXT, sel, (LPARAM)buff);
							for (int i = 0; presetItems[i].szID; i++)
							{
								if (_tcscmp(buff, TranslateTS(presetItems[i].swzName)) == 0)
								{
									if (presetItems[i].szNeededSubst[0])
										EndDialog(hwndDlg, IDPRESETITEM + i);
									else
										EndDialog(hwndDlg, IDOK);
									break;
								}
							}
						}
						else
							EndDialog(hwndDlg, IDOK);

						return TRUE;
					}
					case IDCANCEL:
					{
						EndDialog(hwndDlg, IDCANCEL);
						return TRUE;
					}
					case IDC_BTN_VARIABLE:
					{
						if (GetFocus() == GetDlgItem(hwndDlg, IDC_ED_LABEL))
							variables_showhelp(hwndDlg, IDC_ED_LABEL, VHF_FULLDLG, NULL, NULL);
						else
							variables_showhelp(hwndDlg, IDC_ED_VALUE, VHF_FULLDLG, NULL, NULL);
						return TRUE;
					}
				}
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				if (LOWORD(wParam) == IDC_CMB_PRESETITEMS)
				{
					int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETCURSEL, 0, 0);
					if (sel != CB_ERR)
					{
						TCHAR buff[256];
						SendDlgItemMessage(hwndDlg, IDC_CMB_PRESETITEMS, CB_GETLBTEXT, sel, (LPARAM)buff);
						for (int i = 0; presetItems[i].szID; i++)
						{
							if (_tcscmp(buff, TranslateTS(presetItems[i].swzName)) == 0)
							{
								SetDlgItemText(hwndDlg, IDC_ED_LABEL, TranslateTS(presetItems[i].swzLabel));
								SetDlgItemText(hwndDlg, IDC_ED_VALUE, presetItems[i].swzValue);
								break;
							}
						}
					}
				}
			}
			break;
		}
	}

	return 0;
}

INT_PTR CALLBACK DlgProcAddSubst(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DISPLAYSUBST *ds = (DISPLAYSUBST *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault( hwndDlg );
			ds = (DISPLAYSUBST *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ds);

			SetDlgItemText(hwndDlg, IDC_ED_LABEL, ds->swzName);

			switch (ds->type)
			{
				case DVT_PROTODB:
					CheckDlgButton(hwndDlg, IDC_CHK_PROTOMOD, TRUE);
					SetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->szSettingName);
					break;
				case DVT_DB:
					SetDlgItemTextA(hwndDlg, IDC_ED_MODULE, ds->szModuleName);
					SetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->szSettingName);
					break;
			}

			for (int i = 0; i < iTransFuncsCount; i++)
			{
				int index = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_ADDSTRING, (WPARAM)-1, (LPARAM)TranslateTS(translations[i].swzName));
				SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_SETITEMDATA, index, (LPARAM)translations[i].id);
			}

			for (int i = 0; i < iTransFuncsCount; i++)
			{
				int id = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETITEMDATA, i, 0);
				if (id == ds->iTranslateFuncId)
					SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_SETCURSEL, i, 0);
			}

			SendMessage(hwndDlg, WMU_ENABLE_MODULE_ENTRY, 0, 0);
			SetFocus(GetDlgItem(hwndDlg, IDC_ED_LABEL));
			return TRUE;
		}
		case WMU_ENABLE_MODULE_ENTRY:
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_CHK_PROTOMOD);
			EnableWindow(hw, TRUE);
			hw = GetDlgItem(hwndDlg, IDC_ED_MODULE);
			EnableWindow(hw, !IsDlgButtonChecked(hwndDlg, IDC_CHK_PROTOMOD));
			hw = GetDlgItem(hwndDlg, IDC_ED_SETTING);
			EnableWindow(hw, TRUE);
			return TRUE;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
					case IDC_CHK_PROTOMOD:
					{
						SendMessage(hwndDlg, WMU_ENABLE_MODULE_ENTRY, 0, 0);
						break;
					}
					case IDOK:
					{
						GetDlgItemText(hwndDlg, IDC_ED_LABEL, ds->swzName, LABEL_LEN);
						if (ds->swzName[0] == 0)
						{
							MessageBox(hwndDlg, TranslateT("You must enter a label"), TranslateT("Invalid Substitution"), MB_OK | MB_ICONWARNING);
							return TRUE;
						}

						if (IsDlgButtonChecked(hwndDlg, IDC_CHK_PROTOMOD))
							ds->type = DVT_PROTODB;
						else
						{
							ds->type = DVT_DB;
							GetDlgItemTextA(hwndDlg, IDC_ED_MODULE, ds->szModuleName, SIZEOF(ds->szModuleName));
						}

						GetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->szSettingName, SIZEOF(ds->szSettingName));

						int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETCURSEL, 0, 0);
						ds->iTranslateFuncId = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETITEMDATA, sel, 0);

						EndDialog(hwndDlg, IDOK);
						return TRUE;
					}
					case IDCANCEL:
					{
						EndDialog(hwndDlg, IDCANCEL);
						return TRUE;
					}
				}
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE)
				return TRUE;

			break;
		}
	}

	return 0;
}

static void SetTreeItemText(DIListNode *node, TCHAR **pszText)
{
	if (node->di.swzLabel[0] == 0)
	{
		if (node->di.swzValue[0] == 0 && node->di.bLineAbove)
			*pszText = _T("--------------------------------------");
		else
			*pszText = TranslateT("<No Label>");
	}
	else
		*pszText = node->di.swzLabel;
}

static OPTBUTTON btns[9] = {
	IDC_BTN_ADD,		SKINICON_OTHER_ADDCONTACT,	0,				LPGENT("Add item"),
	IDC_BTN_SEPARATOR,	0,							IDI_SEPARATOR,	LPGENT("Add separator"),
	IDC_BTN_EDIT,		SKINICON_OTHER_RENAME,		0,				LPGENT("Edit"),
	IDC_BTN_REMOVE,		SKINICON_OTHER_DELETE,		0,				LPGENT("Remove"),
	IDC_BTN_UP,			0,							IDI_UP,			LPGENT("Move up"),
	IDC_BTN_DOWN,		0,							IDI_DOWN,		LPGENT("Move down"),
	IDC_BTN_ADD2,		SKINICON_OTHER_ADDCONTACT,	0,				LPGENT("Add"),
	IDC_BTN_REMOVE2,	SKINICON_OTHER_DELETE,		0,				LPGENT("Remove"),
	IDC_BTN_EDIT2,		SKINICON_OTHER_RENAME,		0,				LPGENT("Edit")
};

INT_PTR CALLBACK DlgProcOptsContent(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			for (int i = 0; i < SIZEOF(btns); i++)
			{
				SendDlgItemMessage(hwndDlg, btns[i].id, BUTTONSETASFLATBTN, TRUE, 0);
				SendDlgItemMessage(hwndDlg, btns[i].id, BUTTONADDTOOLTIP, (WPARAM)TranslateTS(btns[i].swzTooltip), BATF_TCHAR);
				if (btns[i].uintCoreIconId)
					SendDlgItemMessage(hwndDlg, btns[i].id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(btns[i].uintCoreIconId));
				else
				{
					HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(btns[i].uintResIconId));
					SendDlgItemMessage(hwndDlg, btns[i].id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
					DestroyIcon(hIcon);
				}
			}

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE) | TVS_CHECKBOXES);

			TVINSERTSTRUCT tvi = {0};
			tvi.hInsertAfter = TVI_LAST;
			tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
			tvi.item.stateMask = TVIS_STATEIMAGEMASK;

			DIListNode *di_node = opt.diList, *di_value;
			while (di_node)
			{
				di_value = (DIListNode *)mir_alloc(sizeof(DIListNode));
				*di_value = *di_node;
				tvi.item.lParam = (LPARAM)di_value;
				tvi.item.state = INDEXTOSTATEIMAGEMASK(di_value->di.bIsVisible ? 2 : 1);
				SetTreeItemText(di_value, &tvi.item.pszText);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);
				di_node = di_node->next;
			}

			DSListNode *ds_node = opt.dsList, *ds_value;
			while (ds_node)
			{
				ds_value = (DSListNode *)mir_alloc(sizeof(DSListNode));
				*ds_value = *ds_node;
				int index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
				SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)ds_value);
				ds_node = ds_node->next;
			}

			SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
			return FALSE;
		}
		case WMU_ENABLE_LIST_BUTTONS:
		{
			HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
			if (hItem)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_REMOVE), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), TreeView_GetPrevSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem) ? TRUE : FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem) ? TRUE : FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EDIT), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_REMOVE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_EDIT), FALSE);
			}

			int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
			if (sel == -1)
			{
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE2);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT2);
				EnableWindow(hw, FALSE);
			}
			else
			{
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE2);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT2);
				EnableWindow(hw, TRUE);
			}
			return TRUE;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LST_SUBST)
				SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
			else if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			else if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_LST_SUBST)
			{
				int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
				if (sel != CB_ERR)
				{
					DSListNode *ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
					if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&ds_value->ds) == IDOK)
					{
						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);

						sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, sel, (LPARAM)ds_value);

						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, sel, 0);
						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
			else if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
					case IDC_BTN_ADD:
					{
						DIListNode *di_value = (DIListNode *)mir_alloc(sizeof(DIListNode));
						memset(di_value, 0, sizeof(DIListNode));
						di_value->di.bIsVisible = true;

						int result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&di_value->di);
						if (result == IDOK || (result >= IDPRESETITEM && result < (IDPRESETITEM + 100)))
						{
							TVINSERTSTRUCT tvi = {0};
							tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
							tvi.item.stateMask = TVIS_STATEIMAGEMASK;
							tvi.item.lParam = (LPARAM)di_value;
							tvi.item.state = INDEXTOSTATEIMAGEMASK(2);
							SetTreeItemText(di_value, &tvi.item.pszText);

							HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
							if (hItem)
								tvi.hInsertAfter = hItem;
							else
								tvi.hInsertAfter = TVI_LAST;
							TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);

							if (hItem)
							{
								HTREEITEM hNewItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem);
								if (hNewItem)
									TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hNewItem);
							}

							if (result >= IDPRESETITEM)
							{
								for (int i = 0; i < MAX_PRESET_SUBST_COUNT; i++)
								{

									PRESETSUBST *subst = GetPresetSubstByName(presetItems[result - IDPRESETITEM].szNeededSubst[i]);
									if (subst == NULL)
										break;
									if (SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_FINDSTRING, -1, (LPARAM)subst->swzName) == LB_ERR)
									{
										DSListNode *ds_value = (DSListNode *)mir_alloc(sizeof(DSListNode));
										memset(ds_value, 0, sizeof(DSListNode));
										ds_value->next = NULL;
										ds_value->ds.type = subst->type;
										_tcscpy(ds_value->ds.swzName, subst->swzName);

										if (ds_value->ds.type == DVT_DB && subst->szModuleName)
											strcpy(ds_value->ds.szModuleName, subst->szModuleName);

										if (subst->szSettingName)
											strcpy(ds_value->ds.szSettingName, subst->szSettingName);

										ds_value->ds.iTranslateFuncId = subst->iTranslateFuncId;

										int index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
										SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)ds_value);
										SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, index, 0);
									}
								}
							}

							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}

						return TRUE;
					}
					case IDC_BTN_SEPARATOR:
					{
						DIListNode *di_value = (DIListNode *)mir_alloc(sizeof(DIListNode));
						memset(di_value, 0, sizeof(DIListNode));
						di_value->di.bIsVisible = true;
						di_value->di.bLineAbove = true;

						TVINSERTSTRUCT tvi = {0};
						tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
						tvi.item.stateMask = TVIS_STATEIMAGEMASK;
						tvi.item.lParam = (LPARAM)di_value;
						tvi.item.state = INDEXTOSTATEIMAGEMASK(2);
						tvi.item.pszText = _T("---------------------------------");

						HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
						if (hItem)
							tvi.hInsertAfter = hItem;
						else
							tvi.hInsertAfter = TVI_LAST;
						TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);

						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						return TRUE;
					}
					case IDC_BTN_REMOVE:
					{
						TVITEM item = {0};
						item.mask = TVIF_PARAM;
						item.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
						if (item.hItem)
						{
							if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item))
							{
								DIListNode *di_value = (DIListNode *)item.lParam;
								mir_free(di_value);
								TreeView_DeleteItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), item.hItem);
								SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}
						return TRUE;
					}
					case IDC_BTN_UP:
					case IDC_BTN_DOWN:
					{
						HTREEITEM hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
						if (hItem)
						{
							HTREEITEM hNewItem;
							if (LOWORD(wParam) == IDC_BTN_UP)
								hNewItem = TreeView_GetPrevSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem);
							else
								hNewItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem);

							if (hNewItem)
							{
								TCHAR buff[512], buff2[512];
								LPARAM tmpParam;
								UINT tmpState;

								TVITEM item = {0};
								item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
								item.stateMask = TVIS_STATEIMAGEMASK;
								item.hItem = hItem;
								item.pszText = buff;
								item.cchTextMax = SIZEOF(buff);
								if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item))
								{
									tmpParam = item.lParam;
									tmpState = item.state;
									item.hItem = hNewItem;
									item.pszText = buff2;
									if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item))
									{
										item.hItem = hItem;
										TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);

										item.hItem = hNewItem;
										item.pszText = buff;
										item.lParam = tmpParam;
										item.state = tmpState;
										TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
										TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hNewItem);
									}
								}
							}
						}

						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						return TRUE;
					}
					case IDC_BTN_EDIT:
					{
						TVITEM item = {0};
						item.mask = TVIF_PARAM;
						item.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
						if (item.hItem )
						{
							if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item))
							{
								DIListNode *di_value = (DIListNode *)item.lParam;
								if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&di_value->di) == IDOK)
								{
									item.mask = TVIF_TEXT;
									SetTreeItemText(di_value, &item.pszText);
									TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
									SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
									SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
								}
							}
						}

						return TRUE;
					}
					case IDC_BTN_ADD2:
					{
						DSListNode *ds_value = (DSListNode *)mir_alloc(sizeof(DSListNode));
						memset(ds_value, 0, sizeof(DSListNode));
						if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&ds_value->ds) == IDOK)
						{
							int index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)ds_value);
							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, index, 0);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}

						return TRUE;
					}
					case IDC_BTN_REMOVE2:
					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
						if (sel != LB_ERR)
						{
							DSListNode *ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
							mir_free(ds_value);

							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}

						return TRUE;
					}
					case IDC_BTN_EDIT2:
					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
						if (sel != LB_ERR)
						{
							DSListNode *ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
							if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&ds_value->ds) == IDOK)
							{
								SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);

								sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.swzName);
								SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, sel, (LPARAM)ds_value);

								SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, sel, 0);
								SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}

						return TRUE;
					}
					default:
					{
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						return TRUE;
					}
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{
					DIListNode *di_node;
					while (opt.diList)
					{
						di_node = opt.diList;
						opt.diList = opt.diList->next;
						mir_free(di_node);
					}

					DIListNode *di_value;
					opt.iDiCount = TreeView_GetCount(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));

					TVITEM item = {0};
					item.mask = TVIF_PARAM;
					item.hItem = TreeView_GetLastVisible(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
					while (item.hItem != NULL)
					{
						if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item))
						{
							di_node = (DIListNode *)item.lParam;
							di_value = (DIListNode *)mir_alloc(sizeof(DIListNode));
							*di_value = *di_node;
							di_value->next = opt.diList;
							opt.diList = di_value;
						}
						item.hItem = TreeView_GetPrevSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), item.hItem);
					}

					DSListNode *ds_node;
					while (opt.dsList)
					{
						ds_node = opt.dsList;
						opt.dsList = opt.dsList->next;
						mir_free(ds_node);
					}

					DSListNode *ds_value;
					opt.iDsCount = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCOUNT, 0, 0);
					for (int i = opt.iDsCount - 1; i >= 0; i--)
					{
						ds_node = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, i, 0);
						ds_value = (DSListNode *)mir_alloc(sizeof(DSListNode));
						*ds_value = *ds_node;
						ds_value->next = opt.dsList;
						opt.dsList = ds_value;
					}

					SaveItems();
					return TRUE;
				}
				case NM_DBLCLK:
				{
					TVITEM item = {0};
					item.mask = TVIF_PARAM;
					item.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
					if (item.hItem)
					{
						if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item))
						{
							DIListNode *di_value = (DIListNode *)item.lParam;
							if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&di_value->di) == IDOK)
							{
								item.mask = TVIF_TEXT;
								SetTreeItemText(di_value, &item.pszText);
								TreeView_SetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
								SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}
					}
					break;
				}
				case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x = (short)LOWORD(GetMessagePos());
					hti.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
					if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					{
						if (hti.flags & TVHT_ONITEMSTATEICON)
						{
							TVITEMA item = {0};
							item.hItem = hti.hItem;
							item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
							item.stateMask = TVIS_STATEIMAGEMASK;
							TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &item);

							if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 1)
								((DIListNode *)item.lParam)->di.bIsVisible = true;
							else
								((DIListNode *)item.lParam)->di.bIsVisible = false;

							TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &item);
							SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
						}
					}
					break;
				}
				case TVN_SELCHANGEDA:
				case TVN_SELCHANGEDW:
				{
					SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
					break;
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			DIListNode *di_value;
			TVITEM tvi = {0};
			tvi.mask = TVIF_PARAM;
			HTREEITEM hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));

			while (hItem != NULL)
			{
				tvi.hItem = hItem;
				if (TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi))
				{
					di_value = (DIListNode *)tvi.lParam;
					mir_free(di_value);
				}
				hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), hItem);
			}

			DSListNode *ds_value;
			int count = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCOUNT, 0, 0);
			for (int i = 0; i < count; i++)
			{
				ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, i, 0);
				mir_free(ds_value);
			}

			break;
		}
	}

	return 0;
}

INT_PTR CALLBACK DlgProcOptsAppearance(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault( hwndDlg );

			CheckDlgButton(hwndDlg, IDC_CHK_SHOWTITLE, opt.bShowTitle ? TRUE : FALSE);
			CheckDlgButton(hwndDlg, IDC_CHK_NOFOCUS, opt.bShowNoFocus ? TRUE : FALSE);
			CheckDlgButton(hwndDlg, IDC_CHK_SBAR, opt.bStatusBarTips ? TRUE : FALSE);

			SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("No icon"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on left"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on right"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_SETCURSEL, (int)opt.titleIconLayout, 0);

			SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom right"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom left"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top right"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top left"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_SETCURSEL, (int)opt.pos, 0);

			SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Center"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom"));
			switch (opt.iLabelValign)
			{
				case DT_TOP: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 0, 0); break;
				case DT_VCENTER: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 1, 0); break;
				case DT_BOTTOM: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 2, 0); break;
			}

			SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Center"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom"));
			switch (opt.iValueValign)
			{
				case DT_TOP: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 0, 0); break;
				case DT_VCENTER: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 1, 0); break;
				case DT_BOTTOM: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 2, 0); break;
			}

			SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
			switch (opt.iLabelHalign)
			{
				case DT_LEFT: SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_SETCURSEL, 0, 0); break;
				case DT_RIGHT: SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_SETCURSEL, 1, 0); break;
			}

			SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
			switch (opt.iValueHalign)
			{
				case DT_LEFT: SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_SETCURSEL, 0, 0); break;
				case DT_RIGHT: SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_SETCURSEL, 1, 0); break;
			}

			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("No avatar"));
			if (ServiceExists(MS_AV_DRAWAVATAR))
			{
				SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left avatar"));
				SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right avatar"));
			}
			else
			{
				HWND hw = GetDlgItem(hwndDlg, IDC_CMB_AV);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_SPIN_AVSIZE);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_ED_AVSIZE);
				EnableWindow(hw, FALSE);
			}
			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_SETCURSEL, (int)opt.avatarLayout, 0);

			SendDlgItemMessage(hwndDlg, IDC_SPIN_WIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_MINWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_MAXHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_MINHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_AVSIZE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 16));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_TITLEINDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_VALUEINDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_PADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_TEXTPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_OUTAVPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_INAVPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_HOVER, UDM_SETRANGE, 0, (LPARAM)MAKELONG(5000, 5));
			SendDlgItemMessage(hwndDlg, IDC_SPIN_SBWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 0));

			SetDlgItemInt(hwndDlg, IDC_ED_WIDTH, opt.iWinWidth, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, opt.iWinMaxHeight, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_MINWIDTH, opt.iMinWidth, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_MINHEIGHT, opt.iMinHeight, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, opt.iAvatarSize, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_INDENT, opt.iTextIndent, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_TITLEINDENT, opt.iTitleIndent, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_VALUEINDENT, opt.iValueIndent, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_PADDING, opt.iPadding, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_TEXTPADDING, opt.iTextPadding, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_OUTAVPADDING, opt.iOuterAvatarPadding, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_INAVPADDING, opt.iInnerAvatarPadding, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_HOVER, opt.iTimeIn, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, opt.iSidebarWidth, FALSE);

			CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERSAV, opt.bAvatarRound);
			CheckDlgButton(hwndDlg, IDC_CHK_AVBORDER, opt.bAvatarBorder);
			CheckDlgButton(hwndDlg, IDC_CHK_ORIGINALAVSIZE, opt.bOriginalAvatarSize);

			if (opt.bOriginalAvatarSize)
				SetDlgItemText(hwndDlg, IDC_STATIC_AVATARSIZE, TranslateT("Max avatar size:"));

			return FALSE;
		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDC_CHK_ORIGINALAVSIZE)
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_CHK_ORIGINALAVSIZE))
					SetDlgItemText(hwndDlg, IDC_STATIC_AVATARSIZE, TranslateT("Max avatar size:"));
				else
					SetDlgItemText(hwndDlg, IDC_STATIC_AVATARSIZE, TranslateT("Avatar size:"));
			}

			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			else if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			else if (HIWORD(wParam) == BN_CLICKED)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
		case WM_NOTIFY:
		{
			if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY)
			{
				BOOL trans;
				int new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_WIDTH, &trans, FALSE);
				if (trans) opt.iWinWidth = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_MINWIDTH, &trans, FALSE);
				if (trans) opt.iMinWidth = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, &trans, FALSE);
				if (trans) opt.iWinMaxHeight = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_MINHEIGHT, &trans, FALSE);
				if (trans) opt.iMinHeight = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, &trans, FALSE);
				if (trans) opt.iAvatarSize = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_INDENT, &trans, FALSE);
				if (trans) opt.iTextIndent = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_TITLEINDENT, &trans, FALSE);
				if (trans) opt.iTitleIndent = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_VALUEINDENT, &trans, FALSE);
				if (trans) opt.iValueIndent = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_PADDING, &trans, FALSE);
				if (trans) opt.iPadding = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_TEXTPADDING, &trans, FALSE);
				if (trans) opt.iTextPadding = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_OUTAVPADDING, &trans, FALSE);
				if (trans) opt.iOuterAvatarPadding = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_INAVPADDING, &trans, FALSE);
				if (trans) opt.iInnerAvatarPadding = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_HOVER, &trans, FALSE);
				if (trans) opt.iTimeIn = new_val;
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, &trans, FALSE);
				if (trans) opt.iSidebarWidth = new_val;

				opt.titleIconLayout = (PopupIconTitleLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_GETCURSEL, 0, 0);
				opt.avatarLayout = (PopupAvLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_GETCURSEL, 0, 0);
				opt.pos = (PopupPosition)SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_GETCURSEL, 0, 0);

				opt.bAvatarBorder = IsDlgButtonChecked(hwndDlg, IDC_CHK_AVBORDER) ? true : false;
				opt.bAvatarRound = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERSAV) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERSAV))  ? true : false;
				opt.bOriginalAvatarSize = IsDlgButtonChecked(hwndDlg, IDC_CHK_ORIGINALAVSIZE) ? true : false;

				opt.bShowTitle = IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWTITLE) ? true : false;
				opt.bShowNoFocus = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOFOCUS) ? true : false;
				opt.bStatusBarTips = IsDlgButtonChecked(hwndDlg, IDC_CHK_SBAR) ? true : false;

				switch (SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_GETCURSEL, 0, 0))
				{
					case 0: opt.iLabelValign = DT_TOP; break;
					case 1: opt.iLabelValign = DT_VCENTER; break;
					case 2: opt.iLabelValign = DT_BOTTOM; break;
				}

				switch (SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_GETCURSEL, 0, 0))
				{
					case 0: opt.iValueValign = DT_TOP; break;
					case 1: opt.iValueValign = DT_VCENTER; break;
					case 2: opt.iValueValign = DT_BOTTOM; break;
				}

				switch (SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_GETCURSEL, 0, 0))
				{
					case 0: opt.iLabelHalign = DT_LEFT; break;
					case 1: opt.iLabelHalign = DT_RIGHT; break;
				}

				switch (SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_GETCURSEL, 0, 0))
				{
					case 0: opt.iValueHalign = DT_LEFT; break;
					case 1: opt.iValueHalign = DT_RIGHT; break;
				}

				SaveOptions();
				return TRUE;
			}
			break;
		}
	}

	return 0;
}

INT_PTR CALLBACK DlgProcOptsExtra(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EXTRAICONDATA *dat = (EXTRAICONDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_CHK_WAITFORCONTENT, opt.bWaitForContent);
			CheckDlgButton(hwndDlg, IDC_CHK_GETSTATUSMSG, opt.bGetNewStatusMsg);
			CheckDlgButton(hwndDlg, IDC_CHK_DISABLEINVISIBLE, opt.bDisableIfInvisible);
			CheckDlgButton(hwndDlg, IDC_CHK_RETRIEVEXSTATUS, opt.bRetrieveXstatus);
			CheckDlgButton(hwndDlg, IDC_CHK_LIMITMSG, opt.bLimitMsg);
			CheckDlgButton(hwndDlg, IDC_CHK_ENABLESMILEYS, opt.iSmileyAddFlags & SMILEYADD_ENABLE);
			CheckDlgButton(hwndDlg, IDC_CHK_USEPROTOSMILEYS, opt.iSmileyAddFlags & SMILEYADD_USEPROTO);
			CheckDlgButton(hwndDlg, IDC_CHK_ONLYISOLATED, opt.iSmileyAddFlags & SMILEYADD_ONLYISOLATED);
			CheckDlgButton(hwndDlg, IDC_CHK_RESIZESMILEYS, opt.iSmileyAddFlags & SMILEYADD_RESIZE);

			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_DISABLEINVISIBLE), opt.bGetNewStatusMsg);

			BOOL bEnable = opt.iSmileyAddFlags & SMILEYADD_ENABLE;
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_USEPROTOSMILEYS), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ONLYISOLATED), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_RESIZESMILEYS), bEnable);

			SendDlgItemMessage(hwndDlg, IDC_SPIN_CHARCOUNT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(1024, 16));
			SetDlgItemInt(hwndDlg, IDC_ED_CHARCOUNT, opt.iLimitCharCount, FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ED_CHARCOUNT), opt.bLimitMsg);

			for (int i = 0; i < EXICONS_COUNT; i++)
			{
				exIcons[i].order = opt.exIconsOrder[i];
				exIcons[i].vis = opt.exIconsVis[i];
			}

			dat = (EXTRAICONDATA *)mir_alloc(sizeof(EXTRAICONDATA));
			dat->bDragging = false;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

			TVINSERTSTRUCT tvi = {0};
			tvi.hParent = 0;
			tvi.hInsertAfter = TVI_LAST;
			tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
			tvi.item.stateMask = TVIS_STATEIMAGEMASK;
			for (int i = 0; i < SIZEOF(extraIconName); i++)
			{
				tvi.item.lParam = (LPARAM)&exIcons[i];
				tvi.item.pszText = TranslateTS(extraIconName[exIcons[i].order]);
				tvi.item.state = INDEXTOSTATEIMAGEMASK(exIcons[i].vis ? 2 : 1);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &tvi);
			}

			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CHK_ENABLESMILEYS:
				{
					BOOL bEnable = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLESMILEYS);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_RESIZESMILEYS), bEnable);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_USEPROTOSMILEYS), bEnable);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ONLYISOLATED), bEnable);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_RESIZESMILEYS), bEnable);
					break;
				}
				case IDC_CHK_LIMITMSG:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_ED_CHARCOUNT), IsDlgButtonChecked(hwndDlg, IDC_CHK_LIMITMSG));
					break;
				}
				case IDC_CHK_GETSTATUSMSG:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_DISABLEINVISIBLE), IsDlgButtonChecked(hwndDlg, IDC_CHK_GETSTATUSMSG));
					break;
				}
			}

			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			
			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom)
			{
				case 0:
				{
					if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY)
					{
						TVITEM item = {0};
						item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS));

						int i = 0;
						while (item.hItem != NULL)
						{
							item.mask = TVIF_HANDLE | TVIF_PARAM;
							TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &item);
							opt.exIconsOrder[i] = ((ICONSTATE *)item.lParam)->order;
							opt.exIconsVis[i] = ((ICONSTATE *)item.lParam)->vis;
							item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), item.hItem);
							i++;
						}
						db_set_blob(NULL, MODULE, "IconOrder", opt.exIconsOrder, SIZEOF(opt.exIconsOrder));
						db_set_blob(NULL, MODULE, "icons_vis", opt.exIconsVis, SIZEOF(opt.exIconsVis));

						opt.iSmileyAddFlags = 0;
						opt.iSmileyAddFlags |= (IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLESMILEYS) ? SMILEYADD_ENABLE : 0)
							| (IsDlgButtonChecked(hwndDlg, IDC_CHK_USEPROTOSMILEYS) ? SMILEYADD_USEPROTO : 0)
							| (IsDlgButtonChecked(hwndDlg, IDC_CHK_ONLYISOLATED) ? SMILEYADD_ONLYISOLATED : 0)
							| (IsDlgButtonChecked(hwndDlg, IDC_CHK_RESIZESMILEYS) ? SMILEYADD_RESIZE : 0);

						opt.bWaitForContent = IsDlgButtonChecked(hwndDlg, IDC_CHK_WAITFORCONTENT) ? true : false;
						opt.bGetNewStatusMsg = IsDlgButtonChecked(hwndDlg, IDC_CHK_GETSTATUSMSG) ? true : false;
						opt.bDisableIfInvisible = IsDlgButtonChecked(hwndDlg, IDC_CHK_DISABLEINVISIBLE) ? true : false;
						opt.bRetrieveXstatus = IsDlgButtonChecked(hwndDlg, IDC_CHK_RETRIEVEXSTATUS) ? true : false;
						opt.bLimitMsg = IsDlgButtonChecked(hwndDlg, IDC_CHK_LIMITMSG) ? true : false;
						opt.iLimitCharCount = GetDlgItemInt(hwndDlg, IDC_ED_CHARCOUNT, 0, FALSE);

						db_set_dw(0, MODULE, "SmileyAddFlags", opt.iSmileyAddFlags);
						db_set_b(0, MODULE, "WaitForContent", opt.bWaitForContent ? 1 : 0);
						db_set_b(0, MODULE, "GetNewStatusMsg", opt.bGetNewStatusMsg ? 1 : 0);
						db_set_b(0, MODULE, "DisableInvisible", opt.bDisableIfInvisible ? 1 : 0);
						db_set_b(0, MODULE, "RetrieveXStatus", opt.bRetrieveXstatus ? 1 : 0);
						db_set_b(0, MODULE, "LimitMsg", opt.bLimitMsg ? 1 : 0);
						db_set_b(0, MODULE, "LimitCharCount", opt.iLimitCharCount);

						return TRUE;
					}
					break;
				}
				case IDC_TREE_EXTRAICONS:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case TVN_BEGINDRAGA:
						case TVN_BEGINDRAGW:
						{
							SetCapture(hwndDlg);
							dat->bDragging = true;
							dat->hDragItem = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
							TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), dat->hDragItem);
							break;
						}
						case NM_CLICK:
						{
							TVHITTESTINFO hti;
							hti.pt.x = (short)LOWORD(GetMessagePos());
							hti.pt.y = (short)HIWORD(GetMessagePos());
							ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
							if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
							{
								if (hti.flags & TVHT_ONITEMSTATEICON)
								{
									TVITEMA item;
									item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
									item.stateMask = TVIS_STATEIMAGEMASK;
									item.hItem = hti.hItem;
									TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &item);
									if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 1)
										((ICONSTATE *)item.lParam)->vis = 1;
									else
										((ICONSTATE *)item.lParam)->vis = 0;

									TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &item);
									SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (!dat->bDragging)
				break;

			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti.pt);
			TreeView_HitTest(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT))
			{
				HTREEITEM hItem = hti.hItem;
				hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS)) / 2;
				TreeView_HitTest(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti);
				if (!(hti.flags & TVHT_ABOVE))
					TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), hti.hItem, 1);
				else
					TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), hItem, 0);
			}
			else
			{
				if (hti.flags & TVHT_ABOVE)
					SendDlgItemMessage(hwndDlg, IDC_TREE_EXTRAICONS, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW)
					SendDlgItemMessage(hwndDlg, IDC_TREE_EXTRAICONS, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);

				TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), 0, 0);
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (!dat->bDragging)
				break;

			TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), 0, 0);
			dat->bDragging = false;
			ReleaseCapture();

			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti.pt);
			hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS)) / 2;
			TreeView_HitTest(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &hti);
			if (dat->hDragItem == hti.hItem)
				break;

			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;

			TVITEM item;
			item.mask = TVIF_HANDLE | TVIF_PARAM;
			item.hItem = dat->hDragItem;
			TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &item);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT) || (hti.hItem == TVI_FIRST))
			{
				TVINSERTSTRUCT tvis;
				TCHAR swzName[256];
				tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
				tvis.item.stateMask = TVIS_STATEIMAGEMASK;
				tvis.item.pszText = swzName;
				tvis.item.cchTextMax = SIZEOF(swzName);
				tvis.item.hItem = dat->hDragItem;
				tvis.item.state = INDEXTOSTATEIMAGEMASK(((ICONSTATE *)item.lParam)->vis ? 2 : 1);
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &tvis.item);
				TreeView_DeleteItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), dat->hDragItem);
				tvis.hParent = NULL;
				tvis.hInsertAfter = hti.hItem;
				TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_EXTRAICONS), &tvis));
				SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
			}

			break;
		}
		case WM_DESTROY:
		{
			mir_free(dat);
			break;
		}
	}
	return 0;
}

void EnableControls(HWND hwndDlg, bool bEnableSkin)
{
	ShowWindow(GetDlgItem(hwndDlg, IDC_ST_PREVIEW), (bEnableSkin && opt.szPreviewFile[0]) ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(hwndDlg, IDC_PIC_PREVIEW), (bEnableSkin && opt.szPreviewFile[0]) ? SW_SHOW : SW_HIDE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SHADOW), !bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_BORDER), !bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERS), !bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOADFONTS), bEnableSkin);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOADPROPORTIONS), bEnableSkin);

	if(!bEnableSkin)
	{
		CheckDlgButton(hwndDlg, IDC_CHK_ENABLECOLORING, FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENABLECOLORING), FALSE);
	}
	else if (opt.iEnableColoring == -1)
	{
		CheckDlgButton(hwndDlg, IDC_CHK_ENABLECOLORING, TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENABLECOLORING), FALSE);
	}
	else
	{
		CheckDlgButton(hwndDlg, IDC_CHK_ENABLECOLORING, opt.iEnableColoring ? 1 : 0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ENABLECOLORING), TRUE);
	}
}

int iLastSel;
INT_PTR CALLBACK DlgProcOptsSkin(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			iLastSel = RefreshSkinList(hwndDlg);

			SendDlgItemMessage(hwndDlg, IDC_BTN_RELOADLIST, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_RELOADLIST, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Reload skin list"), BATF_TCHAR);
			HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RELOAD));
			SendDlgItemMessage(hwndDlg, IDC_BTN_RELOADLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			DestroyIcon(hIcon);

			SendDlgItemMessage(hwndDlg, IDC_BTN_APPLYSKIN, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_APPLYSKIN, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Apply skin"), BATF_TCHAR);
			hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPLY));
			SendDlgItemMessage(hwndDlg, IDC_BTN_APPLYSKIN, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			DestroyIcon(hIcon);

			SendDlgItemMessage(hwndDlg, IDC_BTN_GETSKINS, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_GETSKINS, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Get more skins"), BATF_TCHAR);
			SendDlgItemMessage(hwndDlg, IDC_BTN_GETSKINS, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_URL));


			SendDlgItemMessage(hwndDlg, IDC_SPIN_TRANS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 0));
			SetDlgItemInt(hwndDlg, IDC_ED_TRANS, opt.iOpacity, FALSE);

			CheckDlgButton(hwndDlg, IDC_CHK_BORDER, opt.bBorder);
			CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERS, opt.bRound);
			CheckDlgButton(hwndDlg, IDC_CHK_SHADOW, opt.bDropShadow);
			CheckDlgButton(hwndDlg, IDC_CHK_AEROGLASS, opt.bAeroGlass);
			CheckDlgButton(hwndDlg, IDC_CHK_LOADFONTS, opt.bLoadFonts);
			CheckDlgButton(hwndDlg, IDC_CHK_LOADPROPORTIONS, opt.bLoadProportions);

			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_AEROGLASS), MyDwmEnableBlurBehindWindow != 0);

			SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_ADDSTRING, 0, (LPARAM)TranslateT("None"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Animation"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Fade"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_SETCURSEL, (int)opt.showEffect, 0);

			SendDlgItemMessage(hwndDlg, IDC_SPIN_SPEED, UDM_SETRANGE, 0, (LPARAM)MAKELONG(20, 1));
			SetDlgItemInt(hwndDlg, IDC_ED_SPEED, opt.iAnimateSpeed, FALSE);

			EnableControls(hwndDlg, opt.skinMode == SM_IMAGE);
			return TRUE;
		}
		case WM_DRAWITEM:
		{
			if (wParam == IDC_PIC_PREVIEW)
			{
				DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
				HBRUSH hbr = GetSysColorBrush(COLOR_BTNFACE);
				FillRect(dis->hDC, &dis->rcItem, hbr);

				if (opt.szPreviewFile[0])
				{
					HDC hdcMem = CreateCompatibleDC(dis->hDC);
					HBITMAP hbmpPreview = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)opt.szPreviewFile, IMGL_TCHAR);
					if (hbmpPreview)
					{
						int iWidth = dis->rcItem.right - dis->rcItem.left;
						int iHeight = dis->rcItem.bottom - dis->rcItem.top;

						ResizeBitmap rb = {0};
						rb.size = sizeof(rb);
						rb.hBmp = hbmpPreview;
						rb.max_width = iWidth;
						rb.max_height = iHeight;
						rb.fit = RESIZEBITMAP_KEEP_PROPORTIONS;
						HBITMAP hbmpRes = (HBITMAP)CallService(MS_IMG_RESIZE, (WPARAM)&rb, 0);
						if (hbmpRes)
						{
							BITMAP bmp;
							GetObject(hbmpRes, sizeof(bmp), &bmp);
							SelectObject(hdcMem, hbmpRes);
							BitBlt(dis->hDC, (iWidth - bmp.bmWidth) / 2, (iHeight - bmp.bmHeight) / 2, iWidth, iHeight, hdcMem, 0, 0, SRCCOPY);
							if (hbmpPreview != hbmpRes)
								DeleteBitmap(hbmpRes);
						}

						DeleteBitmap(hbmpPreview);
					}

					DeleteDC(hdcMem);
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			switch (HIWORD(wParam))
			{
				case LBN_SELCHANGE:
				{
					if (LOWORD(wParam) == IDC_LB_SKINS)
					{
						HWND hwndList = GetDlgItem(hwndDlg, IDC_LB_SKINS);
						int iSel = ListBox_GetCurSel(hwndList);
						if (iSel != iLastSel)
						{
							if (iSel == 0)
							{
								opt.szPreviewFile[0] = 0;
								EnableControls(hwndDlg, false);
							}
							else if (iSel != LB_ERR)
							{
								TCHAR swzSkinName[256];
								if (ListBox_GetText(hwndList, iSel, swzSkinName) > 0)
									ParseSkinFile(swzSkinName, false, true);
								EnableControls(hwndDlg, true);
							}

							InvalidateRect(GetDlgItem(hwndDlg, IDC_PIC_PREVIEW), 0, FALSE);
							iLastSel = iSel;
						}
					}
					else if (LOWORD(wParam) == IDC_CMB_EFFECT)
					{
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}

					break;
				}
				case BN_CLICKED:
				{
					if (LOWORD(wParam) == IDC_BTN_APPLYSKIN)
					{
						int iSel = ListBox_GetCurSel(GetDlgItem(hwndDlg, IDC_LB_SKINS));
						if (iSel == 0)
						{
							opt.skinMode = SM_COLORFILL;
							opt.szSkinName[0] = 0;
						}
						else if (iSel != LB_ERR)
						{
							if (ListBox_GetText(GetDlgItem(hwndDlg, IDC_LB_SKINS), iSel, opt.szSkinName) > 0)
							{
								opt.skinMode = SM_IMAGE;
								ParseSkinFile(opt.szSkinName, false, false);
								ReloadFont(0, 0);
								SaveOptions();
							}
						}

						db_set_b(0, MODULE, "SkinEngine", opt.skinMode);
						db_set_ts(0, MODULE, "SkinName", opt.szSkinName);
						
						DestroySkinBitmap();
						SetDlgItemInt(hwndDlg, IDC_ED_TRANS, opt.iOpacity, FALSE);
					}
					else if (LOWORD(wParam) == IDC_BTN_RELOADLIST)
						iLastSel = RefreshSkinList(hwndDlg);
					else if (LOWORD(wParam) == IDC_CHK_LOADFONTS)
						opt.bLoadFonts = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADFONTS) ? true : false;
					else if (LOWORD(wParam) == IDC_CHK_LOADPROPORTIONS)
						opt.bLoadProportions = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADPROPORTIONS) ? true : false;
					else if (LOWORD(wParam) == IDC_BTN_GETSKINS)
						CallService(MS_UTILS_OPENURL, 0, (LPARAM)"http://miranda-ng.org/addons/category/48");

					break;
				}
			}

			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
		case WM_NOTIFY:
		{
			if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY )
			{
				opt.iOpacity = GetDlgItemInt(hwndDlg, IDC_ED_TRANS, 0, 0);
				opt.bDropShadow = IsDlgButtonChecked(hwndDlg, IDC_CHK_SHADOW) ? true : false;
				opt.bBorder = IsDlgButtonChecked(hwndDlg, IDC_CHK_BORDER) ? true : false;
				opt.bRound = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERS) ? true : false;
				opt.bAeroGlass = IsDlgButtonChecked(hwndDlg, IDC_CHK_AEROGLASS) ? true : false;
				opt.showEffect = (PopupShowEffect)SendDlgItemMessage(hwndDlg, IDC_CMB_EFFECT, CB_GETCURSEL, 0, 0);
				opt.iAnimateSpeed = GetDlgItemInt(hwndDlg, IDC_ED_SPEED, 0, 0);
				opt.bLoadFonts = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADFONTS) ? true : false;
				opt.bLoadProportions = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOADPROPORTIONS) ? true : false;

				if (opt.iEnableColoring != -1)
					opt.iEnableColoring = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLECOLORING) ? true : false;

				SaveSkinOptions();
				return TRUE;
			}
		}
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcFavouriteContacts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_DISABLEGROUPS && !db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, (WPARAM)FALSE, 0);
		else
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, (WPARAM)TRUE, 0);

		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETGREYOUTFLAGS, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETLEFTMARGIN, 2, 0);
		{
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
				if (hItem && db_get_b(hContact, MODULE, "FavouriteContact", 0))
					SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
			}
		}
		CheckDlgButton(hwndDlg, IDC_CHK_HIDEOFFLINE, opt.iFavoriteContFlags & FAVCONT_HIDE_OFFLINE);
		CheckDlgButton(hwndDlg, IDC_CHK_APPENDPROTO, opt.iFavoriteContFlags & FAVCONT_APPEND_PROTO);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BTN_OK:
			{
				BYTE isChecked;
				int count = 0;

				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
					if (hItem) {
						isChecked = (BYTE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
						db_set_b(hContact, MODULE, "FavouriteContact", isChecked);
						if (isChecked)
							count++;
					}
				}
				db_set_dw(0, MODULE, "FavouriteContactsCount", count);

				opt.iFavoriteContFlags = 0;
				opt.iFavoriteContFlags |= IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEOFFLINE) ? FAVCONT_HIDE_OFFLINE : 0
					| IsDlgButtonChecked(hwndDlg, IDC_CHK_APPENDPROTO) ? FAVCONT_APPEND_PROTO : 0;

				db_set_dw(0, MODULE, "FavContFlags", opt.iFavoriteContFlags);
			} // fall through
		case IDC_BTN_CANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcOptsTraytip(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_CHK_ENABLETRAYTIP, opt.bTraytip);
			CheckDlgButton(hwndDlg, IDC_CHK_HANDLEBYTIPPER, opt.bHandleByTipper);
			CheckDlgButton(hwndDlg, IDC_CHK_EXPAND, opt.bExpandTraytip);
			CheckDlgButton(hwndDlg, IDC_CHK_HIDEOFFLINE, opt.bHideOffline);
			SendDlgItemMessage(hwndDlg, IDC_SPIN_EXPANDTIME, UDM_SETRANGE, 0, (LPARAM)MAKELONG(5000, 10));
			SetDlgItemInt(hwndDlg, IDC_ED_EXPANDTIME, opt.iExpandTime, FALSE);
			SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_CHK_ENABLETRAYTIP, 0), 0);

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

			TVINSERTSTRUCT tvi = {0};
			tvi.hParent = 0;
			tvi.hInsertAfter = TVI_LAST;
			tvi.item.mask = TVIF_TEXT | TVIF_STATE;

			int i, count = 0;
			PROTOACCOUNT **accs;
			ProtoEnumAccounts(&count, &accs);

			for (i = 0; i < count; i++)
			{
				if (CallProtoService(accs[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) != 0)
				{
					tvi.item.pszText = accs[i]->tszAccountName;
					tvi.item.stateMask = TVIS_STATEIMAGEMASK;
					tvi.item.state = INDEXTOSTATEIMAGEMASK(IsTrayProto(accs[i]->tszAccountName, false) ? 2 : 1);
					TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), &tvi);
					tvi.item.state = INDEXTOSTATEIMAGEMASK(IsTrayProto(accs[i]->tszAccountName, true) ? 2 : 1);
					TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), &tvi);
				}
			}

			for (i = 0; i < SIZEOF(trayTipItems); i++)
			{
				tvi.item.pszText = TranslateTS(trayTipItems[i]);
				tvi.item.state = INDEXTOSTATEIMAGEMASK(opt.iFirstItems & (1 << i) ? 2 : 1);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &tvi);
				tvi.item.state = INDEXTOSTATEIMAGEMASK(opt.iSecondItems & (1 << i) ? 2 : 1);
				TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), &tvi);
			}

			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				UINT state;
				case IDC_CHK_ENABLETRAYTIP:
				{
					state = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_HANDLEBYTIPPER), state);
				} // fall through
				case IDC_CHK_HANDLEBYTIPPER:
				{
					state = IsDlgButtonChecked(hwndDlg, IDC_CHK_HANDLEBYTIPPER) & IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP);

					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_HIDEOFFLINE), state);
					EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), state);
					EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), state);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_FAVCONTACTS), state);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_EXPAND), state);
				} // fall through
				case IDC_CHK_EXPAND:
				{
					state = IsDlgButtonChecked(hwndDlg, IDC_CHK_HANDLEBYTIPPER)
						& IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP)
						& IsDlgButtonChecked(hwndDlg, IDC_CHK_EXPAND);
					EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), state);
					EnableWindow(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), state);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ED_EXPANDTIME), state);
					break;
				}
				case IDC_BTN_FAVCONTACTS:
				{
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_FAVCONTACTS), 0, DlgProcFavouriteContacts);
					break;
				}
			}

			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom)
			{
				case 0:
				{
					if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY)
					{
						TCHAR buff[256];
						TCHAR swzProtos[1024] = {0};

						TVITEM item;
						item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS));
						item.pszText = buff;
						item.cchTextMax = SIZEOF(buff);
						item.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
						item.stateMask = TVIS_STATEIMAGEMASK;
						while (item.hItem != NULL)
						{
							TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), &item);
							if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2)
							{
								_tcscat(swzProtos, buff);
								_tcscat(swzProtos, _T(" "));
							}

							item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_PROTOS), item.hItem);
						}

						db_set_ts(0, MODULE, "TrayProtocols", swzProtos);

						swzProtos[0] = 0;
						item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS));
						item.pszText = buff;
						while (item.hItem != NULL)
						{
							TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), &item);
							if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2)
							{
								_tcscat(swzProtos, buff);
								_tcscat(swzProtos, _T(" "));
							}

							item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_SECOND_PROTOS), item.hItem);
						}

						db_set_ts(0, MODULE, "TrayProtocolsEx", swzProtos);

						int count = 0;
						opt.iFirstItems = 0;
						item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS));
						item.mask = TVIF_HANDLE | TVIF_STATE;
						item.stateMask = TVIS_STATEIMAGEMASK;
						while (item.hItem != NULL)
						{
							TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), &item);
							if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2)
								opt.iFirstItems |= (1 << count);

							item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_FIRST_ITEMS), item.hItem);
							count++;
						}

						count = 0;
						opt.iSecondItems = 0;
						item.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS));
						while (item.hItem != NULL)
						{
							TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), &item);
							if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 2)
								opt.iSecondItems |= (1 << count);

							item.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg, IDC_TREE_SECOND_ITEMS), item.hItem);
							count++;
						}

						opt.bTraytip = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENABLETRAYTIP) ? true : false;
						opt.bHandleByTipper = IsDlgButtonChecked(hwndDlg, IDC_CHK_HANDLEBYTIPPER) ? true : false;
						opt.bExpandTraytip = IsDlgButtonChecked(hwndDlg, IDC_CHK_EXPAND) ? true : false;
						opt.bHideOffline = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEOFFLINE) ? true : false;
						opt.iExpandTime = max(min(GetDlgItemInt(hwndDlg, IDC_ED_EXPANDTIME, 0, FALSE), 5000), 10);

						db_set_b(0, MODULE, "TrayTip", (opt.bTraytip ? 1 : 0));
						db_set_b(0, MODULE, "ExtendedTrayTip", (opt.bHandleByTipper ? 1 : 0));
						db_set_b(0, MODULE, "ExpandTrayTip", (opt.bExpandTraytip ? 1 : 0));
						db_set_b(0, MODULE, "HideOffline", (opt.bHideOffline ? 1 : 0));
						db_set_dw(0, MODULE, "ExpandTime", opt.iExpandTime);
						db_set_dw(0, MODULE, "TrayTipItems", opt.iFirstItems);
						db_set_dw(0, MODULE, "TrayTipItemsEx", opt.iSecondItems);
						return TRUE;
					}
					break;
				}
				case IDC_TREE_FIRST_PROTOS:
				case IDC_TREE_SECOND_PROTOS:
				case IDC_TREE_FIRST_ITEMS:
				case IDC_TREE_SECOND_ITEMS:
				{
					if (((LPNMHDR)lParam)->code == NM_CLICK)
					{
						TVHITTESTINFO hti;
						hti.pt.x = (short)LOWORD(GetMessagePos());
						hti.pt.y = (short)HIWORD(GetMessagePos());
						ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
						if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
							if (hti.flags & TVHT_ONITEMSTATEICON)
								SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
					}
					break;
				}
			}
			break;
		}
	}

	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -790000000;
	odp.hInstance = hInst;

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_APPEARANCE);
	odp.pszTab	= LPGEN("Appearance");
	odp.pszTitle = LPGEN("Tooltips");
	odp.pszGroup = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsAppearance;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.pszTab = LPGEN("Tooltips");
	odp.pszGroup = LPGEN("Skins");
	odp.pfnDlgProc = DlgProcOptsSkin;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTENT);
	odp.pszTab = LPGEN("Content");
	odp.pszTitle = LPGEN("Tooltips");
	odp.pszGroup = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsContent;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TRAYTIP);
	odp.pszTab = LPGEN("Tray tooltip");
	odp.pszTitle = LPGEN("Tooltips");
	odp.pszGroup = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsTraytip;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_EXTRA);
	odp.pszTab = LPGEN("Extra");
	odp.pszTitle = LPGEN("Tooltips");
	odp.pszGroup = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcOptsExtra;
	Options_AddPage(wParam, &odp);
	return 0;
}

HANDLE hEventOptInit;

void InitOptions()
{
	hEventOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);
}

void DeinitOptions()
{
	UnhookEvent(hEventOptInit);

	DIListNode *di_node = opt.diList;
	while (opt.diList)
	{
		di_node = opt.diList;
		opt.diList = opt.diList->next;
		mir_free(di_node);
	}

	DSListNode *ds_node = opt.dsList;
	while (opt.dsList)
	{
		ds_node = opt.dsList;
		opt.dsList = opt.dsList->next;
		mir_free(ds_node);
	}

	for (int i = 0; i < SKIN_ITEMS_COUNT; i++)
	{
		if (opt.szImgFile[i])
			mir_free(opt.szImgFile[i]);
	}
}
