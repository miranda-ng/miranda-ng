/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CRowItem::CRowItem(MCONTACT _1, QSMainDlg *pDlg) :
	hContact(_1)
{
	auto *pa = Proto_GetContactAccount(hContact);
	if (pa != nullptr) {
		szProto = pa->szModuleName;
		if (!pa->IsEnabled())
			bAccOff = true;

		if (db_mc_isMeta(hContact))
			bIsMeta = true;
		else if (db_mc_isSub(hContact))
			bIsSub = true;
	}
	else {
		szProto = nullptr;
		bAccDel = true;
	}

	if (bAccDel || bAccOff)
		status = ID_STATUS_OFFLINE;
	else
		status = Contact::GetStatus(hContact);

	if (int nCount = g_plugin.m_columns.getCount()) {
		pValues = new Val[nCount];
		for (int i = 0; i < nCount; i++)
			pValues[i].LoadOneItem(hContact, g_plugin.m_columns[i], pDlg);
	}
	else pValues = nullptr;
}

CRowItem::~CRowItem()
{
	delete[] pValues;
}

void CRowItem::GetCellColor(int idx, COLORREF &clrBack, COLORREF &clrText)
{
	if (g_plugin.m_flags & QSO_COLORIZE) {
		if (bAccDel) {
			clrBack = g_plugin.m_colors[bkg_del];
			clrText = g_plugin.m_colors[fgr_del];
			return;
		}
		if (bAccOff) {
			clrBack = g_plugin.m_colors[bkg_dis];
			clrText = g_plugin.m_colors[fgr_dis];
			return;
		}
		if (bIsMeta) {
			clrBack = g_plugin.m_colors[bkg_meta];
			clrText = g_plugin.m_colors[fgr_meta];
			return;
		}
		if (bIsSub) {
			clrBack = g_plugin.m_colors[bkg_sub];
			clrText = g_plugin.m_colors[fgr_sub];
			return;
		}
		if (bInList) {
			clrBack = g_plugin.m_colors[bkg_hid];
			clrText = g_plugin.m_colors[fgr_hid];
			return;
		}
	}

	if ((g_plugin.m_flags & QSO_DRAWGRID) == 0 && idx % 2 == 1) {
		clrBack = g_plugin.m_colors[bkg_odd];
		clrText = g_plugin.m_colors[fgr_odd];
	}
	else {
		clrBack = g_plugin.m_colors[bkg_norm];
		clrText = g_plugin.m_colors[fgr_norm];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t* int2strw(uint32_t num)
{
	wchar_t buf[64];
	_itow_s(num, buf, 10);
	return mir_wstrdup(buf);
}

static wchar_t* hex2strw(uint32_t num)
{
	wchar_t buf[64];
	_itow_s(num, buf, 16);
	return mir_wstrdup(buf);
}

void CRowItem::Val::LoadOneItem(MCONTACT hContact, const ColumnItem &pCol, QSMainDlg *pDlg)
{
	data = UINT_PTR(-1);
	replaceStrW(text, nullptr);

	switch (pCol.setting_type) {
	case QST_SCRIPT:
		{
			VARSW vars(pCol.script);
			if (g_bVarsInstalled)
				text = variables_parse(vars, 0, hContact);
			else 
				text = vars.detach();
		}
		break;

	case QST_SERVICE:
		// !!!!!!!!!!!!!!!!!!! not implemented
		break;

	case QST_CONTACTINFO:
		text = Contact::GetInfo(pCol.cnftype, hContact);
		if (text)
			data = _wtoi(text);
		break;

	case QST_OTHER:
		switch (pCol.other) {
		case QSTO_ACCOUNT:
			if (auto *pa = Proto_GetContactAccount(hContact))
				text = mir_wstrdup(pa->tszAccountName);
			break;

		case QSTO_LASTSEEN:
			data = BuildLastSeenTimeInt(hContact, "SeenModule");
			text = BuildLastSeenTime(data);
			break;

		case QSTO_DISPLAYNAME:
			text = mir_wstrdup(Clist_GetContactDisplayName(hContact, 0));
			break;

		case QSTO_LASTEVENT:
			if (MEVENT hDbEvent = db_event_last(hContact)) {
				DBEVENTINFO dbei = {};
				db_event_get(hDbEvent, &dbei);
				data = dbei.timestamp;
				text = TimeToStrW(data);
			}
			else text = 0;			 
			break;

		case QSTO_METACONTACT:
			text = pDlg->DoMeta(hContact);
			break;

		case QSTO_EVENTCOUNT:
			data = db_event_count(hContact);
			text = int2strw(data);
			break;
		}
		break;

	case QST_SETTING:
		auto *szNodule = pCol.module;
		if (!mir_strlen(szNodule))
			szNodule = Proto_GetBaseAccountName(hContact);

		switch (pCol.datatype) {
		case QSTS_STRING:
			text = db_get_wsa(hContact, szNodule, pCol.setting);
			break;

		case QSTS_BYTE:
			data = db_get_b(hContact, szNodule, pCol.setting);
			text = int2strw(data);
			break;

		case QSTS_WORD:
			data = db_get_w(hContact, szNodule, pCol.setting);
			text = int2strw(data);
			break;

		case QSTS_DWORD:
			if (pCol.setting == nullptr) {
				data = hContact;
				text = hex2strw(data);
			}
			else {
				data = db_get_dw(hContact, szNodule, pCol.setting);
				text = int2strw(data);
			}
			break;

		case QSTS_SIGNED:
			data = db_get_dw(hContact, szNodule, pCol.setting);
			text = int2strw(data);
			break;

		case QSTS_HEXNUM:
			data = db_get_dw(hContact, szNodule, pCol.setting);
			text = hex2strw(data);
			break;

		case QSTS_TIMESTAMP:
			data = db_get_dw(hContact, szNodule, pCol.setting);
			if (data != 0)
				text = TimeToStrW(data);
			break;
		}
	}
}
