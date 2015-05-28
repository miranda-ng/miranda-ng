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

	return CMString(FORMAT, _T("<%S:%s>"), szProto, szUniqueID).Detach();
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

	return CMString(FORMAT, _T("<%S:%s>"), szProto, szUniqueID).Detach();
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

	ptrT szUniqueID;
	char *szProto = GetContactProto(hContact);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, hContact);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		TCHAR tszID[40];
		mir_sntprintf(tszID, SIZEOF(tszID), _T("%p"), hContact);
		szUniqueID = mir_tstrdup(tszID);
	}

	return CMString(FORMAT, _T("<%S:%s>"), szProto, szUniqueID).Detach();
}

void registerMetaContactsTokens()
{
	registerIntToken(MC_GETPARENT, parseGetParent, TRF_FUNCTION, LPGEN("Metacontacts")"\t(x)\t"LPGEN("get parent metacontact of contact x"));
	registerIntToken(MC_GETDEFAULT, parseGetDefault, TRF_FUNCTION, LPGEN("Metacontacts")"\t(x)\t"LPGEN("get default subcontact x"));
	registerIntToken(MC_GETMOSTONLINE, parseGetMostOnline, TRF_FUNCTION, LPGEN("Metacontacts")"\t(x)\t"LPGEN("get the 'most online' subcontact x"));
}
