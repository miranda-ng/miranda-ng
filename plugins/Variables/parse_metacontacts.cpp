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
#include "parse_metacontacts.h"
#include "contact.h"

#include "m_metacontacts.h"

static TCHAR *parseGetParent(ARGUMENTSINFO *ai) 
{
	if (ai->argc != 2)
		return NULL;

	HANDLE hContact = NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString( &ci );
	if ( count == 1 && ci.hContacts != NULL ) {
		hContact = ci.hContacts[0];
		free(ci.hContacts);
	}
	else {
		if (ci.hContacts != NULL)
			free(ci.hContacts);
		return NULL;
	}

	hContact = (HANDLE)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);
	if (hContact == NULL)
		return NULL;

	TCHAR* res = NULL;
	TCHAR* szUniqueID = NULL;
	char* szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if ( szProto != NULL )
		szUniqueID = getContactInfoT(CNF_UNIQUEID, hContact, 1);

	if ( szUniqueID == NULL ) {
		szProto = PROTOID_HANDLE;
		szUniqueID = itot((int)hContact);
		if ( szProto == NULL || szUniqueID == NULL )
			return NULL;
	}

	res = ( TCHAR* )malloc((strlen(szProto) + _tcslen(szUniqueID) + 4)*sizeof(TCHAR));
	if (res == NULL) {
		free(szUniqueID);
		return NULL;
	}

	TCHAR* tszProto;
	#ifdef UNICODE
		tszProto = a2u(szProto);
	#else
		tszProto = _strdup(szProto);
	#endif

	if ( tszProto != NULL && szUniqueID != NULL ) {
		wsprintf(res, _T("<%s:%s>"), tszProto, szUniqueID);
		free(szUniqueID);
		free(tszProto);
	}

	return res;
}

static TCHAR *parseGetDefault(ARGUMENTSINFO *ai) 
{
	if (ai->argc != 2)
		return NULL;

	HANDLE hContact = NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString( &ci );
	if ( count == 1 && ci.hContacts != NULL ) {
		hContact = ci.hContacts[0];
		free(ci.hContacts);
	}
	else {
		if (ci.hContacts != NULL)
			free(ci.hContacts);
		return NULL;
	}

	hContact = (HANDLE)CallService(MS_MC_GETDEFAULTCONTACT, (WPARAM)hContact, 0);
	if ( hContact == NULL )
		return NULL;

	TCHAR* res = NULL;
	TCHAR* szUniqueID = NULL;
	char* szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, hContact, 1);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		szUniqueID = itot((int)hContact);
		if ( szProto == NULL || szUniqueID == NULL )
			return NULL;
	}

	res = ( TCHAR* )malloc((strlen(szProto) + _tcslen(szUniqueID) + 4)*sizeof(TCHAR));
	if (res == NULL) {
		free(szUniqueID);
		return NULL;
	}

	TCHAR* tszProto;
	#ifdef UNICODE
		tszProto = a2u(szProto);
	#else
		tszProto = _strdup(szProto);
	#endif

	if ( tszProto != NULL && szUniqueID != NULL ) {
		wsprintf(res, _T("<%s:%s>"), tszProto, szUniqueID);
		free(szUniqueID);
		free(tszProto);
	}

	return res;
}

static TCHAR *parseGetMostOnline(ARGUMENTSINFO *ai) 
{
	if (ai->argc != 2)
		return NULL;

	HANDLE hContact = NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString( &ci );
	if ( count == 1 && ci.hContacts != NULL ) {
		hContact = ci.hContacts[0];
		free( ci.hContacts );
	}
	else {
		if ( ci.hContacts != NULL )
			free( ci.hContacts );
		return NULL;
	}

	hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
	if (hContact == NULL)
		return NULL;

	TCHAR* res = NULL;
	TCHAR* szUniqueID = NULL;
	char* szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, hContact, 1);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		szUniqueID = itot((int)hContact);
		if ( szProto == NULL || szUniqueID == NULL )
			return NULL;
	}

	res = ( TCHAR* )malloc((strlen(szProto) + _tcslen(szUniqueID) + 4)*sizeof(TCHAR));
	if (res == NULL) {
		free(szUniqueID);
		return NULL;
	}

	TCHAR* tszProto;
	#ifdef UNICODE
		tszProto = a2u(szProto);
	#else
		tszProto = _strdup(szProto);
	#endif

	if ( tszProto != NULL && szUniqueID != NULL ) {
		wsprintf(res, _T("<%s:%s>"), tszProto, szUniqueID);
		free(szUniqueID);
		free(tszProto);
	}

	return res;
}

int registerMetaContactsTokens() 
{
	if ( ServiceExists( MS_MC_GETPROTOCOLNAME )) {
		registerIntToken( _T(MC_GETPARENT), parseGetParent, TRF_FUNCTION, "MetaContacts\t(x)\tget parent metacontact of contact x");
		registerIntToken( _T(MC_GETDEFAULT), parseGetDefault, TRF_FUNCTION, "MetaContacts\t(x)\tget default subcontact x");
		registerIntToken( _T(MC_GETMOSTONLINE), parseGetMostOnline, TRF_FUNCTION, "MetaContacts\t(x)\tget the 'most online' subcontact x");
	}

	return 0;
}
