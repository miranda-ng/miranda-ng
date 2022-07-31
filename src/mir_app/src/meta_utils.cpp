/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014-22 Miranda NG team
Copyright © 2004-07 Scott Ellis
Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

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

#include "clc.h"
#include "metacontacts.h"

HANDLE invisiGroup;
POINT menuMousePoint;

/** Update the MetaContact login, depending on the protocol desired
*
* The login of the "MetaContacts" protocol will be copied from the login
* of the specified protocol.
*
* @param szProto :	The name of the protocol used to get the login that will be
*					affected to the "MetaContacts" protocol.
*
* @return			O on success, 1 otherwise.
*/

int Meta_SetNick(char *szProto)
{
	ptrW tszNick(Contact::GetInfo(CNF_DISPLAY, 0, szProto));
	if (tszNick == nullptr)
		return 1;

	db_set_ws(0, META_PROTO, "Nick", tszNick);
	return 0;
}

/** Assign a contact (hSub) to a metacontact (hMeta)
*
* @param hSub : HANDLE to a contact that should be assigned
* @param hMeta : HANDLE to a metacontact that will host the contact
* @param set_as_default : bool flag to indicate whether the new contact becomes the default
*
* @return TRUE on success, FALSE otherwise
*/

BOOL Meta_Assign(MCONTACT hSub, MCONTACT hMeta, bool set_as_default)
{
	DBCachedContact *ccDest = CheckMeta(hMeta), *ccSub = g_pCurrDb->getCache()->GetCachedContact(hSub);
	if (ccDest == nullptr || ccSub == nullptr)
		return FALSE;

	char *szProto = Proto_GetBaseAccountName(hSub);
	if (szProto == nullptr) {
		MessageBoxW(nullptr, TranslateT("Could not retrieve contact protocol"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Get the login of the subcontact
	const char *field = Proto_GetUniqueId(szProto);
	DBVARIANT dbv;
	if (db_get(hSub, szProto, field, &dbv)) {
		MessageBoxW(nullptr, TranslateT("Could not get unique ID of contact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Check that is is 'on the list'
	if (!Contact::OnList(hSub)) {
		MessageBoxW(nullptr, TranslateT("Contact is 'not on list' - please add the contact to your contact list before assigning."), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	if (ccDest->nSubs >= MAX_CONTACTS) {
		MessageBoxW(nullptr, TranslateT("Metacontact is full"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	// write the contact's protocol
	char buffer[512];
	mir_snprintf(buffer, "Protocol%d", ccDest->nSubs);
	if (db_set_s(hMeta, META_PROTO, buffer, szProto)) {
		MessageBoxW(nullptr, TranslateT("Could not write contact protocol to metacontact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	// write the login
	mir_snprintf(buffer, "Login%d", ccDest->nSubs);
	if (db_set(hMeta, META_PROTO, buffer, &dbv)) {
		MessageBoxW(nullptr, TranslateT("Could not write unique ID of contact to metacontact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	db_free(&dbv);

	// If we can get the nickname of the subcontact...
	if (!db_get(hSub, szProto, "Nick", &dbv)) {
		// write the nickname
		mir_snprintf(buffer, "Nick%d", ccDest->nSubs);
		if (db_set(hMeta, META_PROTO, buffer, &dbv)) {
			MessageBoxW(nullptr, TranslateT("Could not write nickname of contact to metacontact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
			db_free(&dbv);
			return FALSE;
		}

		db_free(&dbv);
	}

	// write the display name
	mir_snprintf(buffer, "CListName%d", ccDest->nSubs);
	db_set_ws(hMeta, META_PROTO, buffer, Clist_GetContactDisplayName(hSub));

	// Get the status
	uint16_t status = db_get_w(hSub, szProto, "Status", ID_STATUS_OFFLINE);

	// write the status
	mir_snprintf(buffer, "Status%d", ccDest->nSubs);
	db_set_w(hMeta, META_PROTO, buffer, status);

	// write the handle
	mir_snprintf(buffer, "Handle%d", ccDest->nSubs);
	db_set_dw(hMeta, META_PROTO, buffer, hSub);

	// write status string
	mir_snprintf(buffer, "StatusString%d", ccDest->nSubs);

	wchar_t *szStatus = Clist_GetStatusModeDescription(status, 0);
	db_set_ws(hMeta, META_PROTO, buffer, szStatus);

	// Write the link in the contact
	db_set_dw(hSub, META_PROTO, "ParentMeta", hMeta);
	db_set_b(hSub, META_PROTO, "IsSubcontact", true);

	// update count of contacts
	db_set_dw(hMeta, META_PROTO, "NumContacts", ccDest->nSubs+1);
	ccDest->nSubs++;
	ccDest->pSubs = (MCONTACT*)mir_realloc(ccDest->pSubs, sizeof(MCONTACT)*ccDest->nSubs);
	ccDest->pSubs[ccDest->nSubs-1] = hSub;
	ccSub->parentID = hMeta;

	if (set_as_default)
		db_mc_setDefaultNum(hMeta, ccDest->nSubs-1, true);

	// set nick to most online contact that can message
	MCONTACT most_online = Meta_GetMostOnline(ccDest);
	Meta_CopyContactNick(ccDest, most_online);

	// set status to that of most online contact
	Meta_FixStatus(ccDest);

	// if the new contact is the most online contact with avatar support, get avatar info
	most_online = Meta_GetMostOnlineSupporting(ccDest, PFLAGNUM_4, PF4_AVATARS);
	if (most_online == hSub) {
		PROTO_AVATAR_INFORMATION ai;
		ai.hContact = hMeta;
		ai.format = PA_FORMAT_UNKNOWN;
		wcsncpy_s(ai.filename, L"X", _TRUNCATE);

		if (CallProtoService(META_PROTO, PS_GETAVATARINFO, 0, (LPARAM)&ai) == GAIR_SUCCESS)
			db_set_ws(hMeta, "ContactPhoto", "File", ai.filename);
	}

	// merge sub's events to the meta-history
	g_pCurrDb->MetaMergeHistory(ccDest, ccSub);

	// hide sub finally
	Contact::Hide(ccSub->contactID);

	NotifyEventHooks(hSubcontactsChanged, hMeta, 0);
	return TRUE;
}

/**
*	Convenience method - get most online contact supporting messaging
*
*/

MCONTACT Meta_GetMostOnline(DBCachedContact *cc)
{
	return Meta_GetMostOnlineSupporting(cc, PFLAGNUM_1, PF1_IM);
}

/** Get the 'most online' contact for a meta contact (according to above order) which supports the specified
* protocol service, and copies nick from that contact
*
* @param hMetaHANDLE to a metacontact
*
* @return HANDLE to a contact
*/

static int GetStatusPriority(int status)
{
	switch (status) {
		case ID_STATUS_FREECHAT:   return 1;
		case ID_STATUS_AWAY:       return 2;
		case ID_STATUS_OCCUPIED:   return 3;
		case ID_STATUS_NA:         return 4;
		case ID_STATUS_DND:        return 5;
		case ID_STATUS_OFFLINE:    return 6;
	}

	return 0;
}

MCONTACT Meta_GetMostOnlineSupporting(DBCachedContact *cc, int pflagnum, unsigned long capability)
{
	if (cc == nullptr || cc->nDefault == -1)
		return 0;

	// if the default is beyond the end of the list (eek!) return null
	if (cc->nDefault >= cc->nSubs)
		return 0;

	int most_online_status = ID_STATUS_OFFLINE;
	MCONTACT most_online_contact = Meta_GetContactHandle(cc, cc->nDefault);
	char *szProto = Proto_GetBaseAccountName(most_online_contact);
	if (szProto && Proto_GetStatus(szProto) >= ID_STATUS_ONLINE) {
		uint32_t caps = CallProtoService(szProto, PS_GETCAPS, pflagnum, 0);
		if (capability == -1 || (caps & capability) == capability) {
			most_online_status = db_get_w(most_online_contact, szProto, "Status", ID_STATUS_OFFLINE);

			// if our default is not offline, and option to use default is set - return default
			// and also if our default is online, return it
			if (most_online_status != ID_STATUS_OFFLINE)
				return most_online_contact;
		}
		else most_online_status = ID_STATUS_OFFLINE;
	}
	else most_online_status = ID_STATUS_OFFLINE;

	char *most_online_proto = szProto;
	// otherwise, check all the subcontacts for the one closest to the ONLINE state which supports the required capability
	for (int i = 0; i < cc->nSubs; i++) {
		if (i == cc->nDefault) // already checked that (i.e. initial value of most_online_contact and most_online_status are those of the default contact)
			continue;

		MCONTACT hContact = Meta_GetContactHandle(cc, i);
		szProto = Proto_GetBaseAccountName(hContact);
		if (szProto == nullptr || Proto_GetStatus(szProto) < ID_STATUS_ONLINE) // szProto offline or connecting
			continue;

		uint32_t caps = CallProtoService(szProto, PS_GETCAPS, pflagnum, 0);
		if (capability == -1 || (caps & capability) == capability) {
			int status = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			if (status == ID_STATUS_ONLINE) {
				most_online_contact = hContact;
				most_online_proto = szProto;
				return most_online_contact;
			}
			if (status <= ID_STATUS_OFFLINE) // status below ID_STATUS_OFFLINE is a connecting status
				continue;

			if (GetStatusPriority(status) < GetStatusPriority(most_online_status)) {
				most_online_status = status;
				most_online_contact = hContact;
				most_online_proto = szProto;
			}
		}
	}

	// no online contacts? if we're trying to message, use 'send offline' contact
	if (most_online_status == ID_STATUS_OFFLINE && capability == PF1_IM) {
		MCONTACT hOffline = Meta_GetContactHandle(cc, db_get_dw(cc->contactID, META_PROTO, "OfflineSend", INVALID_CONTACT_ID));
		if (hOffline)
			most_online_contact = hOffline;
	}

	return most_online_contact;
}

DBCachedContact* CheckMeta(MCONTACT hMeta)
{
	if (!g_bMetaEnabled)
		return nullptr;

	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hMeta);
	return (cc == nullptr || cc->nSubs == -1) ? nullptr : cc;
}

int Meta_GetContactNumber(DBCachedContact *cc, MCONTACT hContact)
{
	if (g_bMetaEnabled)
		for (int i = 0; i < cc->nSubs; i++)
			if (cc->pSubs[i] == hContact)
				return i;

	return -1;
}

MCONTACT Meta_GetContactHandle(DBCachedContact *cc, int contact_number)
{
	if (contact_number >= cc->nSubs || contact_number < 0 || !g_bMetaEnabled)
		return 0;

	return cc->pSubs[contact_number];
}

/** Hide all contacts linked to any meta contact, and set handle links
*
* Additionally, set all sub contacts and metacontacts to offline so that status notifications are always sent
*
* and ensure metafilter in place
*/
int Meta_HideLinkedContacts(void)
{
	DBVARIANT dbv, dbv2;
	char buffer[512];

	for (auto &hContact : Contacts()) {
		DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hContact);
		if (cc == nullptr || cc->parentID == 0)
			continue;

		DBCachedContact *ccMeta = CheckMeta(cc->parentID);
		if (ccMeta == nullptr)
			continue;

		// get contact number
		int contact_number = Meta_GetContactNumber(cc, hContact);

		// find metacontact
		if (contact_number < 0 || contact_number >= ccMeta->nSubs)
			continue;

		// update metacontact's record of status for this contact
		mir_snprintf(buffer, "Status%d",contact_number);

		// prepare to update metacontact record of subcontat status
		char *szProto = Proto_GetBaseAccountName(hContact);

		uint16_t status = (!szProto) ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		db_set_w(ccMeta->contactID, META_PROTO, buffer, status);

		// update metacontact's record of nick for this contact
		if (szProto && !db_get(hContact, szProto, "Nick", &dbv)) {
			mir_snprintf(buffer, "Nick%d",contact_number);
			db_set(ccMeta->contactID, META_PROTO, buffer, &dbv);

			mir_snprintf(buffer, "CListName%d",contact_number);
			if (db_get(hContact, "CList", "MyHandle", &dbv2))
				db_set(ccMeta->contactID, META_PROTO, buffer, &dbv);
			else {
				db_set(ccMeta->contactID, META_PROTO, buffer, &dbv2);
				db_free(&dbv2);
			}

			db_free(&dbv);
		}
		else {
			if (!db_get(hContact, "CList", "MyHandle", &dbv)) {
				mir_snprintf(buffer,"CListName%d",contact_number);
				db_set(ccMeta->contactID, META_PROTO, buffer, &dbv);
				db_free(&dbv);
			}
		}

		MCONTACT hMostOnline = Meta_GetMostOnline(ccMeta); // set nick
		Meta_CopyContactNick(ccMeta, hMostOnline);
		Meta_FixStatus(ccMeta);
	}

	return 0;
}

int Meta_HideMetaContacts(bool bHide)
{
	for (auto &hContact : Contacts()) {
		bool bSet;
		DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hContact);
		if (cc->IsSub()) // show on hide, reverse flag
			bSet = !bHide;
		else if (cc->IsMeta())
			bSet = bHide;
		else
			continue;

		Contact::Hide(hContact, bSet);
	}

	if (bHide) {
		for (auto &p : arMetaWindows)
			SendMessage(p->m_hWnd, WM_CLOSE, 0, 0);
		arMetaWindows.destroy();
	}

	return 0;
}

int Meta_CopyContactNick(DBCachedContact *ccMeta, MCONTACT hContact)
{
	if (g_metaOptions.bLockHandle)
		hContact = Meta_GetContactHandle(ccMeta, 0);

	if (!hContact)
		return 1;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return 1;

	if (g_metaOptions.clist_contact_name == CNNT_NICK) {
		ptrW tszNick(db_get_wsa(hContact, szProto, "Nick"));
		if (tszNick) {
			db_set_ws(ccMeta->contactID, META_PROTO, "Nick", tszNick);
			return 0;
		}
	}
	else if (g_metaOptions.clist_contact_name == CNNT_DISPLAYNAME) {
		wchar_t *name = Clist_GetContactDisplayName(hContact, GCDNF_NOUNKNOWN);
		if (name) {
			db_set_ws(ccMeta->contactID, META_PROTO, "Nick", name);
			return 0;
		}
	}
	return 1;
}

int Meta_SetAllNicks()
{
	for (auto &hContact : Contacts()) {
		DBCachedContact *cc = CheckMeta(hContact);
		if (cc == nullptr)
			continue;
		MCONTACT most_online = Meta_GetMostOnline(cc);
		Meta_CopyContactNick(cc, most_online);
		Meta_FixStatus(cc);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void SwapValues(MCONTACT hContact, LPCSTR szSetting, int n1, int n2)
{
	char buf1[100], buf2[100];
	mir_snprintf(buf1, "%s%d", szSetting, n1);
	mir_snprintf(buf2, "%s%d", szSetting, n2);

	DBVARIANT dbv1, dbv2;
	int ok1 = !db_get(hContact, META_PROTO, buf1, &dbv1);
	int ok2 = !db_get(hContact, META_PROTO, buf2, &dbv2);
	if (ok1) {
		db_set(hContact, META_PROTO, buf2, &dbv1);
		db_free(&dbv1);
	}
	if (ok2) {
		db_set(hContact, META_PROTO, buf1, &dbv2);
		db_free(&dbv2);
	}
}

int Meta_SwapContacts(DBCachedContact *cc, int n1, int n2)
{
	SwapValues(cc->contactID, "Protocol", n1, n2);
	SwapValues(cc->contactID, "Status", n1, n2);
	SwapValues(cc->contactID, "StatusString", n1, n2);
	SwapValues(cc->contactID, "Login", n1, n2);
	SwapValues(cc->contactID, "Nick", n1, n2);
	SwapValues(cc->contactID, "CListName", n1, n2);
	SwapValues(cc->contactID, "Handle", n1, n2);

	MCONTACT tmp = cc->pSubs[n1];
	cc->pSubs[n1] = cc->pSubs[n2];
	cc->pSubs[n2] = tmp;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Meta_GetSubNick(MCONTACT hMeta, int i, CMStringW &tszDest)
{
	char idStr[50];
	mir_snprintf(idStr, "Login%d", i);

	DBVARIANT dbv;
	if(db_get(hMeta, META_PROTO, idStr, &dbv))
		return;
	switch (dbv.type) {
	case DBVT_ASCIIZ:
		tszDest = dbv.pszVal;
		break;
	case DBVT_WCHAR:
		tszDest = dbv.pwszVal;
		break;
	case DBVT_BYTE:
		tszDest.Format(L"%d", dbv.bVal);
		break;
	case DBVT_WORD:
		tszDest.Format(L"%d", dbv.wVal);
		break;
	case DBVT_DWORD:
		tszDest.Format(L"%d", dbv.dVal);
		break;
	default:
		tszDest.Empty();
	}
	db_free(&dbv);
}

/////////////////////////////////////////////////////////////////////////////////////////

void Meta_FixStatus(DBCachedContact *ccMeta)
{
	uint16_t status = ID_STATUS_OFFLINE;

	MCONTACT most_online = db_mc_getMostOnline(ccMeta->contactID);
	if (most_online) {
		char *szProto = Proto_GetBaseAccountName(most_online);
		if (szProto)
			status = db_get_w(most_online, szProto, "Status", ID_STATUS_OFFLINE);
	}

	db_set_w(ccMeta->contactID, META_PROTO, "Status", status);
	Srmm_SetIconFlags(ccMeta->contactID, META_PROTO, 0, ccMeta->IsMeta() ? 0 : MBF_HIDDEN);
}
