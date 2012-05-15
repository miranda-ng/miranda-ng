//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "StdAfx.h"
#include "tipper_items.h"
#include "resources.h"

static const int MAX_TIPPER_ITEM_PROP = 6 ;

static const LPSTR VALUE_SETTING_PROP = "DIValue%d";
static const LPSTR LABEL_SETTING_PROP = "DILabel%d";

static const LPSTR LAST_WRITTEN_LABEL_SETTING = "LastWrittenTipperLabel";

static LPSTR TipperItemProps[MAX_TIPPER_ITEM_PROP] = {
	LABEL_SETTING_PROP,
	"DILineAbove%d",
	"DITipperVarsFirst%d",
	"DIValNewline%d",
	VALUE_SETTING_PROP,
	"DIVisible%d"
};

static const LPSTR TIPPER_ITEMS_MOD_NAME = "Tipper_Items";
static const LPSTR TIPPER_ITEM_COUNT_SETTING = "DINumValues";
static const LPTSTR UNREAD_THREADS_RAW = _T("%raw:") _T(SHORT_PLUGIN_NAME) _T("/UnreadThreads%");
static const LPTSTR UNREAD_THREADS_LABEL = _T("Unread threads:");

void ShiftTipperSettings(LPSTR buff, int count, LPSTR format)
{
	for (int i = count; i > 0; i--) {
		DBCONTACTWRITESETTING cws;
		DBCONTACTGETSETTING cgs;
		cgs.szModule = TIPPER_ITEMS_MOD_NAME;
		sprintf(buff, format, i - 1);
		cgs.szSetting = buff;
		cgs.pValue = &cws.value;

		if (CallService(MS_DB_CONTACT_GETSETTING, 0, (LPARAM)&cgs)) break;
		__try {
			if (DBVT_ASCIIZ == cws.value.type) {
				DBFreeVariant(&cws.value);
				cws.value.type = DBVT_WCHAR;
				if (CallService(MS_DB_CONTACT_GETSETTING_STR, 0, (LPARAM)&cgs)) break;
			}

			if (CallService(MS_DB_CONTACT_GETSETTING_STR, 0, (LPARAM)&cgs)) break;

			cws.szModule = TIPPER_ITEMS_MOD_NAME;
			sprintf(buff, format, i);
			cws.szSetting = buff;
			CallService(MS_DB_CONTACT_WRITESETTING, 0, (LPARAM)&cws);
		}
		__finally {
			DBFreeVariant(&cws.value);
		}
	}
}

void SetLabelProp(int index, LPSTR setting)
{
	sprintf(setting, LABEL_SETTING_PROP, index);

	DBVARIANT dbv1 = {0};
	if (!DBGetContactSettingTString(0, TIPPER_ITEMS_MOD_NAME, setting, &dbv1))
		__try {
			DBVARIANT dbv2 = {0};
			if (!DBGetContactSettingTString(0, SHORT_PLUGIN_NAME, LAST_WRITTEN_LABEL_SETTING, &dbv2))
				__try {
					if (!lstrcmp(dbv1.ptszVal, dbv2.ptszVal)) {
						LPTSTR label = TranslateTS(UNREAD_THREADS_LABEL);
						DBWriteContactSettingTString(0, SHORT_PLUGIN_NAME, LAST_WRITTEN_LABEL_SETTING, label);
						DBWriteContactSettingTString(0, TIPPER_ITEMS_MOD_NAME, setting, label);
					}
				}
				__finally {
					DBFreeVariant(&dbv2);
				}
		}
		__finally {
			DBFreeVariant(&dbv1);
		}
}

void AddTipperItem()
{
	unsigned short itemCount = DBGetContactSettingWord(0, TIPPER_ITEMS_MOD_NAME,
		TIPPER_ITEM_COUNT_SETTING , unsigned short(-1));
	if (unsigned short(-1) == itemCount) return;

	int i, l = 0;
	for (i = itemCount; i > 0; i /= 10) l++; // var setting path
	l += 30; // const setting part

	LPSTR setting = (LPSTR)malloc(l * sizeof(TCHAR));
	__try {
		for (i = 0; i < itemCount; i++) {
			sprintf(setting, VALUE_SETTING_PROP, i);

			DBVARIANT dbv = {0};
			if (!DBGetContactSettingTString(0, TIPPER_ITEMS_MOD_NAME, setting, &dbv))
				__try {
					if (!lstrcmp(UNREAD_THREADS_RAW, dbv.ptszVal)) {
						SetLabelProp(i, setting);
						return;
					}
				}
				__finally {
					DBFreeVariant(&dbv);
				}
		}

		for (i = 0; i < MAX_TIPPER_ITEM_PROP; i++)
			ShiftTipperSettings(setting, itemCount, TipperItemProps[i]);

		#define WRITE_TIPPER_PROP(type, index, value)\
			sprintf(setting, TipperItemProps[##index##], 0);\
			DBWriteContactSetting##type##(0, TIPPER_ITEMS_MOD_NAME, setting, ##value##)

		LPTSTR label = TranslateTS(UNREAD_THREADS_LABEL);

		DBWriteContactSettingTString(0, SHORT_PLUGIN_NAME, LAST_WRITTEN_LABEL_SETTING, label);

		WRITE_TIPPER_PROP(TString,	0, label);
		WRITE_TIPPER_PROP(Byte,		1, 0);
		WRITE_TIPPER_PROP(Byte,		2, 0);
		WRITE_TIPPER_PROP(Byte,		3, 0);
		WRITE_TIPPER_PROP(TString,	4, UNREAD_THREADS_RAW);
		WRITE_TIPPER_PROP(Byte,		5, 1);

		#undef WRITE_TIPPER_PROP
	}
	__finally {
		free(setting);
	}

	DBWriteContactSettingWord(0, TIPPER_ITEMS_MOD_NAME,
		TIPPER_ITEM_COUNT_SETTING, itemCount + 1);
}