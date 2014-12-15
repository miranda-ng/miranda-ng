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
#include "resources.h"

#define VALUE_SETTING_PROP "DIValue%d"
#define LABEL_SETTING_PROP "DILabel%d"

#define LAST_WRITTEN_LABEL_SETTING "LastWrittenTipperLabel"

static LPSTR TipperItemProps[] =
{
	LABEL_SETTING_PROP,
	"DILineAbove%d",
	"DITipperVarsFirst%d",
	"DIValNewline%d",
	VALUE_SETTING_PROP,
	"DIVisible%d"
};

#define TIPPER_ITEMS_MOD_NAME     "Tipper_Items"
#define TIPPER_ITEM_COUNT_SETTING "DINumValues"
#define UNREAD_THREADS_RAW        _T("%raw:") _T(SHORT_PLUGIN_NAME) _T("/UnreadThreads%")
#define UNREAD_THREADS_LABEL      LPGENT("Unread threads:")

void ShiftTipperSettings(LPSTR buff, int count, LPSTR format)
{
	for (int i = count; i > 0; i--) {
		DBVARIANT dbv;
		sprintf(buff, format, i - 1); //!!!!!!!!!!!!!!!

		if (db_get(0, TIPPER_ITEMS_MOD_NAME, buff, &dbv))
			break;

		__try {
			if (DBVT_ASCIIZ == dbv.type) {
				db_free(&dbv);
				if (db_get_ws(0, TIPPER_ITEMS_MOD_NAME, buff, &dbv))
					break;
			}

			if (db_get_s(0, TIPPER_ITEMS_MOD_NAME, buff, &dbv))
				break;

			sprintf(buff, format, i); //!!!!!!!!!!!!!!!!
			db_set(NULL, TIPPER_ITEMS_MOD_NAME, buff, &dbv);
		}
		__finally {
			db_free(&dbv);
		}
	}
}

void SetLabelProp(int index, LPSTR setting)
{
	sprintf(setting, LABEL_SETTING_PROP, index); //!!!!!!!!!!!!!!

	ptrT tszProp(db_get_tsa(0, TIPPER_ITEMS_MOD_NAME, setting));
	if (tszProp == NULL)
		return;

	ptrT tszLastWritten(db_get_tsa(0, SHORT_PLUGIN_NAME, LAST_WRITTEN_LABEL_SETTING));
	if (tszLastWritten == NULL)
		return;

	if (!mir_tstrcmp(tszProp, tszLastWritten)) {
		LPTSTR label = TranslateTS(UNREAD_THREADS_LABEL);
		db_set_ts(0, SHORT_PLUGIN_NAME, LAST_WRITTEN_LABEL_SETTING, label);
		db_set_ts(0, TIPPER_ITEMS_MOD_NAME, setting, label);
	}
}

void AddTipperItem()
{
	WORD itemCount = db_get_w(0, TIPPER_ITEMS_MOD_NAME, TIPPER_ITEM_COUNT_SETTING, WORD(-1));
	if (itemCount == WORD(-1))
		return;

	int i, l = 0;
	for (i = itemCount; i > 0; i /= 10) l++; // var setting path
	l += 30; // const setting part

	LPSTR setting = (LPSTR)_alloca(l * sizeof(TCHAR));

	for (i = 0; i < itemCount; i++) {
		mir_snprintf(setting, l, VALUE_SETTING_PROP, i);
		ptrT tszSetting(db_get_tsa(0, TIPPER_ITEMS_MOD_NAME, setting));
		if (tszSetting) {
			if (!mir_tstrcmp(UNREAD_THREADS_RAW, tszSetting)) {
				SetLabelProp(i, setting);
				return;
			}
		}
	}

	for (i = 0; i < SIZEOF(TipperItemProps); i++)
		ShiftTipperSettings(setting, itemCount, TipperItemProps[i]);

#define WRITE_TIPPER_PROPS(index, value)\
		mir_snprintf(setting, l, TipperItemProps[##index##], 0);\
		db_set_ts(0, TIPPER_ITEMS_MOD_NAME, setting, ##value##)
#define WRITE_TIPPER_PROPB(index, value)\
		mir_snprintf(setting, l, TipperItemProps[##index##], 0);\
		db_set_b(0, TIPPER_ITEMS_MOD_NAME, setting, ##value##)

	LPTSTR label = TranslateTS(UNREAD_THREADS_LABEL);

	db_set_ts(0, SHORT_PLUGIN_NAME, LAST_WRITTEN_LABEL_SETTING, label);

	WRITE_TIPPER_PROPS(0, label);
	WRITE_TIPPER_PROPB(1, 0);
	WRITE_TIPPER_PROPB(2, 0);
	WRITE_TIPPER_PROPB(3, 0);
	WRITE_TIPPER_PROPS(4, UNREAD_THREADS_RAW);
	WRITE_TIPPER_PROPB(5, 1);

	db_set_w(0, TIPPER_ITEMS_MOD_NAME, TIPPER_ITEM_COUNT_SETTING, itemCount + 1);
}