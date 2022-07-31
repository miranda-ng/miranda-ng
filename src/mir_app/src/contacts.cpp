/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

static wchar_t* nameOrderDescr[] =
{
	LPGENW("My custom name (not movable)"),
	LPGENW("Nick"),
	LPGENW("FirstName"),
	LPGENW("E-mail"),
	LPGENW("LastName"),
	LPGENW("Username"),
	LPGENW("FirstName LastName"),
	LPGENW("LastName FirstName"),
	LPGENW("'(Unknown contact)' (not movable)")
};

uint8_t nameOrder[_countof(nameOrderDescr)];

static wchar_t* ProcessDatabaseValueDefault(MCONTACT hContact, const char *szProto, const char *szSetting)
{
	DBVARIANT dbv;
	if (!db_get_ws(hContact, szProto, szSetting, &dbv)) {
		switch (dbv.type) {
		case DBVT_ASCIIZ:
			if (!dbv.pszVal[0]) break;
		case DBVT_WCHAR:
			if (!dbv.pwszVal[0]) break;
			return dbv.pwszVal;
		}
		db_free(&dbv);
	}

	if (db_get(hContact, szProto, szSetting, &dbv))
		return nullptr;

	wchar_t buf[40];
	switch (dbv.type) {
	case DBVT_BYTE:
		return mir_wstrdup(_itow(dbv.bVal, buf, 10));
	case DBVT_WORD:
		return mir_wstrdup(_itow(dbv.wVal, buf, 10));
	case DBVT_DWORD:
		return mir_wstrdup(_itow(dbv.dVal, buf, 10));
	case DBVT_BLOB:
		return mir_wstrdup(bin2hexW(dbv.pbVal, min(int(dbv.cpbVal), 19), buf));
	}

	db_free(&dbv);
	return nullptr;
}

MIR_APP_DLL(wchar_t*) Contact::GetInfo(int type, MCONTACT hContact, const char *szProto)
{
	if (hContact == 0 && szProto == nullptr)
		return nullptr;

	if (szProto == nullptr)
		szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return nullptr;

	const char *uid;
	wchar_t *res;
	DBVARIANT dbv;
	switch (type) {
	case CNF_FIRSTNAME:  return ProcessDatabaseValueDefault(hContact, szProto, "FirstName");
	case CNF_LASTNAME:   return ProcessDatabaseValueDefault(hContact, szProto, "LastName");
	case CNF_NICK:       return ProcessDatabaseValueDefault(hContact, szProto, "Nick");
	case CNF_EMAIL:      return ProcessDatabaseValueDefault(hContact, szProto, "e-mail");
	case CNF_CITY:       return ProcessDatabaseValueDefault(hContact, szProto, "City");
	case CNF_STATE:      return ProcessDatabaseValueDefault(hContact, szProto, "State");
	case CNF_PHONE:      return ProcessDatabaseValueDefault(hContact, szProto, "Phone");
	case CNF_HOMEPAGE:   return ProcessDatabaseValueDefault(hContact, szProto, "Homepage");
	case CNF_ABOUT:      return ProcessDatabaseValueDefault(hContact, szProto, "About");
	case CNF_AGE:        return ProcessDatabaseValueDefault(hContact, szProto, "Age");
	case CNF_GENDER:     return ProcessDatabaseValueDefault(hContact, szProto, "Gender");
	case CNF_FAX:        return ProcessDatabaseValueDefault(hContact, szProto, "Fax");
	case CNF_CELLULAR:	return ProcessDatabaseValueDefault(hContact, szProto, "Cellular");
	case CNF_BIRTHDAY:	return ProcessDatabaseValueDefault(hContact, szProto, "BirthDay");
	case CNF_BIRTHMONTH:	return ProcessDatabaseValueDefault(hContact, szProto, "BirthMonth");
	case CNF_BIRTHYEAR:	return ProcessDatabaseValueDefault(hContact, szProto, "BirthYear");
	case CNF_STREET:		return ProcessDatabaseValueDefault(hContact, szProto, "Street");
	case CNF_ZIP:			return ProcessDatabaseValueDefault(hContact, szProto, "ZIP");
	case CNF_LANGUAGE1:	return ProcessDatabaseValueDefault(hContact, szProto, "Language1");
	case CNF_LANGUAGE2:	return ProcessDatabaseValueDefault(hContact, szProto, "Language2");
	case CNF_LANGUAGE3:	return ProcessDatabaseValueDefault(hContact, szProto, "Language3");
	case CNF_CONAME:		return ProcessDatabaseValueDefault(hContact, szProto, "Company");
	case CNF_CODEPT:     return ProcessDatabaseValueDefault(hContact, szProto, "CompanyDepartment");
	case CNF_COPOSITION: return ProcessDatabaseValueDefault(hContact, szProto, "CompanyPosition");
	case CNF_COSTREET:   return ProcessDatabaseValueDefault(hContact, szProto, "CompanyStreet");
	case CNF_COCITY:     return ProcessDatabaseValueDefault(hContact, szProto, "CompanyCity");
	case CNF_COSTATE:    return ProcessDatabaseValueDefault(hContact, szProto, "CompanyState");
	case CNF_COZIP:      return ProcessDatabaseValueDefault(hContact, szProto, "CompanyZIP");
	case CNF_COHOMEPAGE: return ProcessDatabaseValueDefault(hContact, szProto, "CompanyHomepage");
	case CNF_CUSTOMNICK: 
		if (hContact)
			return ProcessDatabaseValueDefault(hContact, "CList", "MyHandle");
		return ProcessDatabaseValueDefault(0, szProto, "MyHandle");

	case CNF_COUNTRY:
	case CNF_COCOUNTRY:
		if (!db_get_ws(hContact, szProto, type == CNF_COUNTRY ? "CountryName" : "CompanyCountryName", &dbv))
			return dbv.pwszVal;

		if (!db_get(hContact, szProto, type == CNF_COUNTRY ? "Country" : "CompanyCountry", &dbv)) {
			if (dbv.type == DBVT_WORD) {
				int countryCount;
				struct CountryListEntry *countries;
				CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&countryCount, (LPARAM)&countries);
				for (int i = 0; i < countryCount; i++)
					if (countries[i].id == dbv.wVal)
						return mir_a2u(countries[i].szName);
			}
			else {
				db_free(&dbv);
				return ProcessDatabaseValueDefault(hContact, szProto, type == CNF_COUNTRY ? "Country" : "CompanyCountry");
			}
		}
		break;

	case CNF_FIRSTLAST:
		if (!db_get_ws(hContact, szProto, "FirstName", &dbv)) {
			DBVARIANT dbv2;
			if (!db_get_ws(hContact, szProto, "LastName", &dbv2)) {
				size_t len = mir_wstrlen(dbv.pwszVal) + mir_wstrlen(dbv2.pwszVal) + 2;
				wchar_t *buf = (wchar_t*)mir_alloc(sizeof(wchar_t)*len);
				if (buf != nullptr)
					mir_wstrcat(mir_wstrcat(mir_wstrcpy(buf, dbv.pwszVal), L" "), dbv2.pwszVal);
				db_free(&dbv);
				db_free(&dbv2);
				return buf;
			}
			db_free(&dbv);
		}
		break;

	case CNF_UNIQUEID:
		if (db_mc_isMeta(hContact)) {
			wchar_t buf[40];
			_itow(hContact, buf, 10);
			return mir_wstrdup(buf);
		}
	
		uid = Proto_GetUniqueId(szProto);
		if (uid)
			return ProcessDatabaseValueDefault(hContact, szProto, uid);
		break;

	case CNF_DISPLAYUID:
		if (res = ProcessDatabaseValueDefault(hContact, szProto, "display_uid"))
			return res;
			
		uid = Proto_GetUniqueId(szProto);
		if (uid)
			return ProcessDatabaseValueDefault(hContact, szProto, uid);
		break;

	case CNF_DISPLAYNC:
	case CNF_DISPLAY:
		for (auto &it : nameOrder) {
			switch (it) {
			case 0: // custom name
				// make sure we aren't in CNF_DISPLAYNC mode
				// don't get custom name for nullptr contact
				if (type == CNF_DISPLAY)
					if (res = ProcessDatabaseValueDefault(hContact, (hContact != 0) ? "CList" : szProto, "MyHandle"))
						return res;
				break;

			case 1:
				if (res = ProcessDatabaseValueDefault(hContact, szProto, "Nick")) // nick
					return res;
				break;
			case 2:
				if (res = ProcessDatabaseValueDefault(hContact, szProto, "FirstName")) // First Name
					return res;
				break;
			case 3:
				if (res = ProcessDatabaseValueDefault(hContact, szProto, "e-mail")) // E-mail
					return res;
				break;
			case 4:
				if (res = ProcessDatabaseValueDefault(hContact, szProto, "LastName")) // Last Name
					return res;
				break;

			case 5: // Unique id
				// protocol must define a PFLAG_UNIQUEIDSETTING
				uid = Proto_GetUniqueId(szProto);
				if ((INT_PTR)uid != CALLSERVICE_NOTFOUND && uid) {
					if (!db_get_ws(hContact, szProto, uid, &dbv)) {
						if (dbv.type == DBVT_BYTE || dbv.type == DBVT_WORD || dbv.type == DBVT_DWORD) {
							long value = (dbv.type == DBVT_BYTE) ? dbv.bVal : (dbv.type == DBVT_WORD ? dbv.wVal : dbv.dVal);
							wchar_t buf[40];
							_ltow(value, buf, 10);
							return mir_wstrdup(buf);
						}
						return dbv.pwszVal;
					}
				}
				break;

			case 6: // first + last name
			case 7: // last + first name
				if (!db_get_ws(hContact, szProto, it == 6 ? "FirstName" : "LastName", &dbv)) {
					DBVARIANT dbv2;
					if (!db_get_ws(hContact, szProto, it == 6 ? "LastName" : "FirstName", &dbv2)) {
						size_t len = mir_wstrlen(dbv.pwszVal) + mir_wstrlen(dbv2.pwszVal) + 2;
						wchar_t *buf = (wchar_t*)mir_alloc(sizeof(wchar_t)*len);
						if (buf != nullptr)
							mir_wstrcat(mir_wstrcat(mir_wstrcpy(buf, dbv.pwszVal), L" "), dbv2.pwszVal);

						db_free(&dbv);
						db_free(&dbv2);
						return buf;
					}
					db_free(&dbv);
				}
				break;

			case 8:
				return mir_wstrdup(TranslateT("'(Unknown contact)'"));
			}
		}
		break;

	case CNF_MYNOTES:
		return ProcessDatabaseValueDefault(hContact, "UserInfo", "MyNotes");

	case CNF_TIMEZONE:
		HANDLE hTz = TimeZone_CreateByContact(hContact, nullptr, TZF_KNOWNONLY);
		if (hTz) {
			LPTIME_ZONE_INFORMATION tzi = TimeZone_GetInfo(hTz);
			int offset = tzi->Bias + tzi->StandardBias;

			char str[80];
			mir_snprintf(str, offset ? "UTC%+d:%02d" : "UTC", offset / -60, abs(offset % 60));
			return mir_a2u(str);
		}
		break;
	}

	return nullptr;
}

MIR_APP_DLL(int) Contact::GetStatus(MCONTACT hContact)
{
	if (hContact == 0)
		return ID_STATUS_OFFLINE;

	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return ID_STATUS_OFFLINE;

	return db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Contact::IsGroupChat(MCONTACT hContact, const char *szProto)
{
	if (szProto == nullptr) {
		szProto = Proto_GetBaseAccountName(hContact);
		if (szProto == nullptr)
			return false;
	}

	return db_get_b(hContact, szProto, "ChatRoom") != 0;
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Contact::IsHidden(MCONTACT hContact)
{
	return db_get_b(hContact, "CList", "Hidden") != 0;
}

MIR_APP_DLL(void) Contact::Hide(MCONTACT hContact, bool bHidden)
{
	if (bHidden)
		db_set_b(hContact, "CList", "Hidden", 1);
	else
		db_unset(hContact, "CList", "Hidden");
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Contact::OnList(MCONTACT hContact)
{
	return db_get_b(hContact, "CList", "NotOnList", 0) == 0;
}

MIR_APP_DLL(void) Contact::PutOnList(MCONTACT hContact)
{
	db_unset(hContact, "CList", "NotOnList");
}

MIR_APP_DLL(void) Contact::RemoveFromList(MCONTACT hContact)
{
	db_set_b(hContact, "CList", "NotOnList", 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options dialog

class CContactOptsDlg : public CDlgBase
{
	CCtrlTreeView m_nameOrder;

public:
	CContactOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_CONTACT),
		m_nameOrder(this, IDC_NAMEORDER)
	{
		m_nameOrder.SetFlags(MTREE_DND);
		m_nameOrder.OnBeginDrag = Callback(this, &CContactOptsDlg::OnBeginDrag);
	}

	bool OnInitDialog() override
	{
		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		for (int i = 0; i < _countof(nameOrderDescr); i++) {
			tvis.item.lParam = nameOrder[i];
			tvis.item.pszText = TranslateW(nameOrderDescr[nameOrder[i]]);
			m_nameOrder.InsertItem(&tvis);
		}
		return true;
	}

	bool OnApply() override
	{
		TVITEMEX tvi;
		tvi.hItem = m_nameOrder.GetRoot();
		int i = 0;
		while (tvi.hItem != nullptr) {
			tvi.mask = TVIF_PARAM | TVIF_HANDLE;
			m_nameOrder.GetItem(&tvi);
			nameOrder[i++] = (uint8_t)tvi.lParam;
			tvi.hItem = m_nameOrder.GetNextSibling(tvi.hItem);
		}
		db_set_blob(0, "Contact", "NameOrder", nameOrder, _countof(nameOrderDescr));
		g_clistApi.pfnInvalidateDisplayNameCacheEntry(INVALID_CONTACT_ID);
		return true;
	}

	void OnBeginDrag(CCtrlTreeView::TEventInfo *evt)
	{
		LPNMTREEVIEW pNotify = evt->nmtv;
		if (pNotify->itemNew.lParam == 0 || pNotify->itemNew.lParam == _countof(nameOrderDescr) - 1)
			pNotify->hdr.code = 0; // deny dragging
	}
};

static int ContactOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1000000000;
	odp.szGroup.a = LPGEN("Contact list");
	odp.szTitle.a = LPGEN("Contact names");
	odp.pDialog = new CContactOptsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	return PopupOptionsInit(wParam);
}

int LoadContactsModule(void)
{
	for (uint8_t i = 0; i < _countof(nameOrder); i++)
		nameOrder[i] = i;

	DBVARIANT dbv;
	if (!db_get(0, "Contact", "NameOrder", &dbv)) {
		memcpy(nameOrder, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
	}

	HookEvent(ME_OPT_INITIALISE, ContactOptInit);
	return 0;
}
