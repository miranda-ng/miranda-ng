/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "variables.h"

static TCHAR *parseGetParent(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	MCONTACT hContact = NULL;

	CONTACTSINFO ci = { sizeof(ci) };
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if (count == 1 && ci.hContacts != NULL) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else {
		mir_free(ci.hContacts);
		return NULL;
	}

	hContact = db_mc_getMeta(hContact);
	if (hContact == NULL)
		return NULL;

	ptrT szUniqueID;
	char* szProto = GetContactProto(hContact);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, hContact);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		TCHAR tszID[40];
		mir_sntprintf(tszID, SIZEOF(tszID), _T("%p"), hContact);
		szUniqueID = mir_tstrdup(tszID);
	}

	if (szUniqueID == NULL)
		return NULL;

	return mir_tstrdup(CMString(FORMAT, _T("<%S:%s>"), szProto, szUniqueID));
}

static TCHAR *parseGetDefault(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	MCONTACT hContact = NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if (count == 1 && ci.hContacts != NULL) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else {
		mir_free(ci.hContacts);
		return NULL;
	}

	hContact = db_mc_getDefault(hContact);
	if (hContact == NULL)
		return NULL;

	TCHAR* szUniqueID = NULL;
	char* szProto = GetContactProto(hContact);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, hContact);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		szUniqueID = (TCHAR *)mir_alloc(32);
		mir_sntprintf(szUniqueID, 32, _T("%p"), hContact);
		if (szProto == NULL || szUniqueID == NULL)
			return NULL;
	}

	size_t size = strlen(szProto) + _tcslen(szUniqueID) + 4;
	TCHAR *res = (TCHAR *)mir_alloc(size * sizeof(TCHAR));
	if (res == NULL) {
		mir_free(szUniqueID);
		return NULL;
	}

	TCHAR *tszProto = mir_a2t(szProto);
	if (tszProto != NULL && szUniqueID != NULL) {
		mir_sntprintf(res, size, _T("<%s:%s>"), tszProto, szUniqueID);
		mir_free(szUniqueID);
		mir_free(tszProto);
	}

	return res;
}

static TCHAR *parseGetMostOnline(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	MCONTACT hContact = NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if (count == 1 && ci.hContacts != NULL) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else {
		mir_free(ci.hContacts);
		return NULL;
	}

	hContact = db_mc_getMostOnline(hContact);
	if (hContact == NULL)
		return NULL;

	TCHAR* szUniqueID = NULL;
	char* szProto = GetContactProto(hContact);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, hContact);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		szUniqueID = (TCHAR *)mir_alloc(32);
		mir_sntprintf(szUniqueID, 32, _T("%p"), hContact);
		if (szProto == NULL || szUniqueID == NULL)
			return NULL;
	}

	size_t size = strlen(szProto) + _tcslen(szUniqueID) + 4;
	TCHAR *res = (TCHAR *)mir_alloc(size * sizeof(TCHAR));
	if (res == NULL) {
		mir_free(szUniqueID);
		return NULL;
	}

	TCHAR *tszProto = mir_a2t(szProto);
	if (tszProto != NULL && szUniqueID != NULL) {
		mir_sntprintf(res, size, _T("<%s:%s>"), tszProto, szUniqueID);
		mir_free(szUniqueID);
		mir_free(tszProto);
	}

	return res;
}

void registerMetaContactsTokens()
{
	registerIntToken(MC_GETPARENT, parseGetParent, TRF_FUNCTION, LPGEN("Metacontacts")"\t(x)\t"LPGEN("get parent metacontact of contact x"));
	registerIntToken(MC_GETDEFAULT, parseGetDefault, TRF_FUNCTION, LPGEN("Metacontacts")"\t(x)\t"LPGEN("get default subcontact x"));
	registerIntToken(MC_GETMOSTONLINE, parseGetMostOnline, TRF_FUNCTION, LPGEN("Metacontacts")"\t(x)\t"LPGEN("get the 'most online' subcontact x"));
}
