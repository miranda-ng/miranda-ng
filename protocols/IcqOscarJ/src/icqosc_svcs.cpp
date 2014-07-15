// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  High-level code for exported API services
// -----------------------------------------------------------------------------

#include "icqoscar.h"

INT_PTR CIcqProto::AddServerContact(WPARAM wParam, LPARAM lParam)
{
	if (!m_bSsiEnabled) return 0;

	// Does this contact have a UID?
	DWORD dwUin;
	uid_str szUid;
	if (!getContactUid(wParam, &dwUin, &szUid) && !getWord(wParam, DBSETTING_SERVLIST_ID, 0) && !getWord(wParam, DBSETTING_SERVLIST_IGNORE, 0)) {
		/// TODO: remove possible 0x6A TLV in contact server-list data!!!
		// Read group from DB
		char *pszGroup = getContactCListGroup(wParam);

		servlistAddContact(wParam, pszGroup);
		SAFE_FREE((void**)&pszGroup);
	}
	return 0;
}

static int LookupDatabaseSetting(const FieldNamesItem* table, int code, DBVARIANT *dbv, BYTE type)
{
	char *text = LookupFieldName(table, code);
	if (!text) {
		dbv->type = DBVT_DELETED;
		return 1;
	}

	if (type == DBVT_ASCIIZ) {
		dbv->pszVal = mir_strdup(Translate(text));
		dbv->type = DBVT_ASCIIZ;
	}
	else if (type == DBVT_UTF8 || !type) {
		char tmp[MAX_PATH];
		dbv->pszVal = mir_strdup(ICQTranslateUtfStatic(text, tmp, MAX_PATH));
		dbv->type = DBVT_UTF8;
	}
	else if (type == DBVT_WCHAR) {
		WCHAR* wtext = make_unicode_string(text);
		dbv->pwszVal = mir_wstrdup(TranslateW(wtext));
		dbv->type = DBVT_WCHAR;
		SAFE_FREE((void**)&wtext);
	}
	return 0; // Success
}

INT_PTR CIcqProto::GetInfoSetting(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTGETSETTING *cgs = (DBCONTACTGETSETTING*)lParam;
	BYTE type = cgs->pValue->type;
	INT_PTR rc = db_get_s(hContact, cgs->szModule, cgs->szSetting, cgs->pValue, 0);
	if (rc)
		return rc;

	// Success
	DBVARIANT dbv = *cgs->pValue;
	if (dbv.type == DBVT_BLOB) {
		cgs->pValue->pbVal = (BYTE*)mir_alloc(dbv.cpbVal);
		memcpy(cgs->pValue->pbVal, dbv.pbVal, dbv.cpbVal);
	}
	else if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_UTF8) {
		//  convert to the desired type
		if (!type)
			type = dbv.type;

		// type is correct, only move it to miranda's heap
		if (dbv.type == type)
			cgs->pValue->pszVal = mir_strdup(dbv.pszVal);
		else if (type == DBVT_WCHAR) {
			if (dbv.type != DBVT_UTF8) {
				int len = MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, NULL, 0);
				cgs->pValue->pwszVal = (WCHAR*)mir_alloc((len + 1)*sizeof(WCHAR));
				if (cgs->pValue->pwszVal == NULL)
					rc = 1;
				else {
					MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, cgs->pValue->pwszVal, len);
					cgs->pValue->pwszVal[len] = '\0';
				}
			}
			else {
				char *savePtr = dbv.pszVal ? strcpy((char*)_alloca(strlennull(dbv.pszVal) + 1), dbv.pszVal) : NULL;
				if (!mir_utf8decode(savePtr, &cgs->pValue->pwszVal))
					rc = 1;
			}
		}
		else if (type == DBVT_UTF8) {
			cgs->pValue->pszVal = mir_utf8encode(dbv.pszVal);
			if (cgs->pValue->pszVal == NULL)
				rc = 1;
		}
		else if (type == DBVT_ASCIIZ) {
			cgs->pValue->pszVal = mir_strdup(dbv.pszVal);
			mir_utf8decode(cgs->pValue->pszVal, NULL);
		}

		cgs->pValue->type = type;
	}
	else if (!strcmpnull(cgs->szModule, m_szModuleName) && (dbv.type == DBVT_BYTE || dbv.type == DBVT_WORD || dbv.type == DBVT_DWORD)) {
		int code = (dbv.type == DBVT_BYTE) ? dbv.bVal : ((dbv.type == DBVT_WORD) ? dbv.wVal : dbv.dVal);

		if (!strcmpnull(cgs->szSetting, "Language1") || !strcmpnull(cgs->szSetting, "Language2") || !strcmpnull(cgs->szSetting, "Language3"))
			rc = LookupDatabaseSetting(languageField, code, cgs->pValue, type);
		else if (!strcmpnull(cgs->szSetting, "Country") || !strcmpnull(cgs->szSetting, "OriginCountry") || !strcmpnull(cgs->szSetting, "CompanyCountry")) {
			if (code == 420) code = 42; // conversion of obsolete codes (OMG!)
			else if (code == 421) code = 4201;
			else if (code == 102) code = 1201;
			rc = LookupDatabaseSetting(countryField, code, cgs->pValue, type);
		}
		else if (!strcmpnull(cgs->szSetting, "Gender"))
			rc = LookupDatabaseSetting(genderField, code, cgs->pValue, type);
		else if (!strcmpnull(cgs->szSetting, "MaritalStatus"))
			rc = LookupDatabaseSetting(maritalField, code, cgs->pValue, type);
		else if (!strcmpnull(cgs->szSetting, "StudyLevel"))
			rc = LookupDatabaseSetting(studyLevelField, code, cgs->pValue, type);
		else if (!strcmpnull(cgs->szSetting, "CompanyIndustry"))
			rc = LookupDatabaseSetting(industryField, code, cgs->pValue, type);
		else if (!strcmpnull(cgs->szSetting, "Interest0Cat") || !strcmpnull(cgs->szSetting, "Interest1Cat") || !strcmpnull(cgs->szSetting, "Interest2Cat") || !strcmpnull(cgs->szSetting, "Interest3Cat"))
			rc = LookupDatabaseSetting(interestsField, code, cgs->pValue, type);
	}
	// Release database memory
	db_free(&dbv);
	return rc;
}

INT_PTR CIcqProto::ChangeInfoEx(WPARAM wParam, LPARAM lParam)
{
	if (!icqOnline() || !wParam)
		return 0;

	PBYTE buf = NULL;
	int buflen = 0;
	BYTE b;

	// userinfo
	ppackTLVWord(&buf, &buflen, 0x1C2, (WORD)GetACP());

	if (wParam & CIXT_CONTACT) { // contact information
		BYTE *pBlock = NULL;
		int cbBlock = 0;
		int nItems = 0;

		// Emails
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "e-mail0", 0x78, 0x64, 0x00);
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "e-mail1", 0x78, 0x64, 0x00);
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "e-mail2", 0x78, 0x64, 0x00);
		ppackTLVBlockItems(&buf, &buflen, 0x8C, &nItems, &pBlock, (WORD*)&cbBlock, FALSE);

		// Phones
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "Phone", 0x6E, 0x64, 0x01);
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "CompanyPhone", 0x6E, 0x64, 0x02);
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "Cellular", 0x6E, 0x64, 0x03);
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "Fax", 0x6E, 0x64, 0x04);
		nItems += ppackTLVWordStringItemFromDB(&pBlock, &cbBlock, "CompanyFax", 0x6E, 0x64, 0x05);
		ppackTLVBlockItems(&buf, &buflen, 0xC8, &nItems, &pBlock, (WORD*)&cbBlock, FALSE);

		ppackTLVByte(&buf, &buflen, 0x1EA, getByte("AllowSpam", 0));
	}

	if (wParam & CIXT_BASIC) { // upload basic user info
		ppackTLVStringUtfFromDB(&buf, &buflen, "Nick", 0x78);
		ppackTLVStringUtfFromDB(&buf, &buflen, "FirstName", 0x64);
		ppackTLVStringUtfFromDB(&buf, &buflen, "LastName", 0x6E);
		ppackTLVStringUtfFromDB(&buf, &buflen, "About", 0x186);
	}

	if (wParam & CIXT_MORE) {
		b = getByte("Gender", 0);
		ppackTLVByte(&buf, &buflen, 0x82, (BYTE)(b ? (b == 'M' ? 2 : 1) : 0));

		ppackTLVDateFromDB(&buf, &buflen, "BirthYear", "BirthMonth", "BirthDay", 0x1A4);

		ppackTLVWord(&buf, &buflen, 0xAA, getByte("Language1", 0));
		ppackTLVWord(&buf, &buflen, 0xB4, getByte("Language2", 0));
		ppackTLVWord(&buf, &buflen, 0xBE, getByte("Language3", 0));

		ppackTLVWord(&buf, &buflen, 0x12C, getByte("MaritalStatus", 0));
	}

	if (wParam & CIXT_WORK) {
		BYTE *pBlock = NULL;
		int cbBlock = 0;
		int nItems = 1;

		// Jobs
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "CompanyPosition", 0x64);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "Company", 0x6E);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "CompanyDepartment", 0x7D);
		ppackTLVStringFromDB(&pBlock, &cbBlock, "CompanyHomepage", 0x78);
		ppackTLVWord(&pBlock, &cbBlock, 0x82, getWord("CompanyIndustry", 0));
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "CompanyStreet", 0xAA);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "CompanyCity", 0xB4);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "CompanyState", 0xBE);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "CompanyZIP", 0xC8);
		ppackTLVDWord(&pBlock, &cbBlock, 0xD2, getWord("CompanyCountry", 0));
		/// TODO: pack unknown data (need to preserve them in Block Items)
		ppackTLVBlockItems(&buf, &buflen, 0x118, &nItems, &pBlock, (WORD*)&cbBlock, TRUE);

		//			ppackTLVWord(&buf, &buflen, getWord("CompanyOccupation", 0), TLV_OCUPATION, 1); // Lost In Conversion
	}

	if (wParam & CIXT_EDUCATION) {
		BYTE *pBlock = NULL;
		int cbBlock = 0;
		int nItems = 1;

		// Studies
		ppackTLVWord(&pBlock, &cbBlock, 0x64, getWord("StudyLevel", 0));
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "StudyInstitute", 0x6E);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "StudyDegree", 0x78);
		ppackTLVWord(&pBlock, &cbBlock, 0x8C, getWord("StudyYear", 0));
		ppackTLVBlockItems(&buf, &buflen, 0x10E, &nItems, &pBlock, (WORD*)&cbBlock, TRUE);
	}

	if (wParam & CIXT_LOCATION) {
		BYTE *pBlock = NULL;
		int cbBlock = 0;
		int nItems = 1;

		// Home Address
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "Street", 0x64);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "City", 0x6E);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "State", 0x78);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "ZIP", 0x82);
		ppackTLVDWord(&pBlock, &cbBlock, 0x8C, getWord("Country", 0));
		ppackTLVBlockItems(&buf, &buflen, 0x96, &nItems, &pBlock, (WORD*)&cbBlock, TRUE);

		nItems = 1;
		// Origin Address
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "OriginStreet", 0x64);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "OriginCity", 0x6E);
		ppackTLVStringUtfFromDB(&pBlock, &cbBlock, "OriginState", 0x78);
		ppackTLVDWord(&pBlock, &cbBlock, 0x8C, getWord("OriginCountry", 0));
		ppackTLVBlockItems(&buf, &buflen, 0xA0, &nItems, &pBlock, (WORD*)&cbBlock, TRUE);

		ppackTLVStringFromDB(&buf, &buflen, "Homepage", 0xFA);

		// Timezone
		WORD wTimezone = getByte("Timezone", 0);
		if ((wTimezone & 0x0080) == 0x80) wTimezone |= 0xFF00; // extend signed number
		ppackTLVWord(&buf, &buflen, 0x17C, wTimezone);
	}

	if (wParam & CIXT_BACKGROUND) {
		BYTE *pBlock = NULL;
		int cbBlock = 0;
		int nItems = 0;

		// Interests
		nItems += ppackTLVWordStringUtfItemFromDB(&pBlock, &cbBlock, "Interest0Text", 0x6E, 0x64, getWord("Interest0Cat", 0));
		nItems += ppackTLVWordStringUtfItemFromDB(&pBlock, &cbBlock, "Interest1Text", 0x6E, 0x64, getWord("Interest1Cat", 0));
		nItems += ppackTLVWordStringUtfItemFromDB(&pBlock, &cbBlock, "Interest2Text", 0x6E, 0x64, getWord("Interest2Cat", 0));
		nItems += ppackTLVWordStringUtfItemFromDB(&pBlock, &cbBlock, "Interest3Text", 0x6E, 0x64, getWord("Interest3Cat", 0));
		ppackTLVBlockItems(&buf, &buflen, 0x122, &nItems, &pBlock, (WORD*)&cbBlock, FALSE);
	}

	DWORD dwCookie = icq_changeUserDirectoryInfoServ(buf, (WORD)buflen, DIRECTORYREQUEST_UPDATEOWNER);

	SAFE_FREE((void**)&buf);

	return dwCookie;
}

INT_PTR CIcqProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	if (wParam == AF_MAXSIZE) {
		POINT *size = (POINT*)lParam;
		if (size) {
			size->x = 64;
			size->y = 64;
		}
		return 0;
	}

	if (wParam == AF_PROPORTION)
		return PIP_NONE;

	if (wParam == AF_FORMATSUPPORTED)
		return (lParam == PA_FORMAT_JPEG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_XML || lParam == PA_FORMAT_BMP);

	if (wParam == AF_ENABLED)
		return (m_bSsiEnabled && m_bAvatarsEnabled);

	if (wParam == AF_DONTNEEDDELAYS)
		return 0;

	// server accepts images of 7168 bytees, not bigger
	if (wParam == AF_MAXFILESIZE)
		return 7168;

	// do not request avatar again if server gave an error
	if (wParam == AF_DELAYAFTERFAIL)
		return 1 * 60 * 60 * 1000; // one hour

	// avatars can be fetched all the time (server only operation)
	if (wParam == AF_FETCHIFPROTONOTVISIBLE || wParam == AF_FETCHIFCONTACTOFFLINE)
		return 1;

	return 0;
}

INT_PTR CIcqProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT *pai = (PROTO_AVATAR_INFORMATIONT*)lParam;
	DWORD dwUIN;
	uid_str szUID;
	DBVARIANT dbv = { DBVT_DELETED };

	if (!m_bAvatarsEnabled) return GAIR_NOAVATAR;

	if (getSetting(pai->hContact, "AvatarHash", &dbv) || dbv.type != DBVT_BLOB || (dbv.cpbVal != 0x14 && dbv.cpbVal != 0x09)) {
		db_free(&dbv);
		return GAIR_NOAVATAR; // we did not found avatar hash or hash invalid - no avatar available
	}

	if (getContactUid(pai->hContact, &dwUIN, &szUID)) {
		db_free(&dbv);
		return GAIR_NOAVATAR; // we do not support avatars for invalid contacts
	}

	int dwPaFormat = getByte(pai->hContact, "AvatarType", PA_FORMAT_UNKNOWN);

	if (dwPaFormat != PA_FORMAT_UNKNOWN) { // we know the format, test file
		TCHAR tszFile[MAX_PATH * 2 + 4];

		GetFullAvatarFileName(dwUIN, szUID, dwPaFormat, tszFile, MAX_PATH * 2);

		lstrcpyn(pai->filename, tszFile, SIZEOF(pai->filename)); // Avatar API does not support unicode :-(
		pai->format = dwPaFormat;

		if (!IsAvatarChanged(pai->hContact, dbv.pbVal, dbv.cpbVal)) { // hashes are the same
			if (_taccess(tszFile, 0) == 0) {
				db_free(&dbv);

				return GAIR_SUCCESS; // we have found the avatar file, whoala
			}
		}
	}

	if (IsAvatarChanged(pai->hContact, dbv.pbVal, dbv.cpbVal)) { // we didn't received the avatar before - this ensures we will not request avatar again and again
		if ((wParam & GAIF_FORCE) != 0 && pai->hContact != 0) { // request avatar data
			TCHAR tszFile[MAX_PATH * 2 + 4];

			GetAvatarFileName(dwUIN, szUID, tszFile, MAX_PATH * 2);
			GetAvatarData(pai->hContact, dwUIN, szUID, dbv.pbVal, dbv.cpbVal, tszFile);
			lstrcpyn(pai->filename, tszFile, SIZEOF(pai->filename)); // Avatar API does not support unicode :-(

			db_free(&dbv);

			return GAIR_WAITFOR;
		}
	}
	db_free(&dbv);

	return GAIR_NOAVATAR;
}


INT_PTR CIcqProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!m_bAvatarsEnabled) return -2;

	if (!wParam) return -3;

	TCHAR *tszFile = GetOwnAvatarFileName();
	if (tszFile && !_taccess(tszFile, 0)) {
		_tcsncpy((TCHAR*)wParam, tszFile, (int)lParam);
		SAFE_FREE(&tszFile);
		return 0;
	}

	SAFE_FREE(&tszFile);
	return -1;
}


INT_PTR CIcqProto::GrantAuthorization(WPARAM wParam, LPARAM lParam)
{
	if (icqOnline() && wParam != 0) {
		DWORD dwUin;
		uid_str szUid;
		if (getContactUid(wParam, &dwUin, &szUid))
			return 0; // Invalid contact

		// send without reason, do we need any ?
		icq_sendGrantAuthServ(dwUin, szUid, NULL);
		// auth granted, remove contact menu item
		delSetting(wParam, "Grant");
	}

	return 0;
}

int CIcqProto::OnIdleChanged(WPARAM wParam, LPARAM lParam)
{
	int bIdle = (lParam & IDF_ISIDLE);
	int bPrivacy = (lParam & IDF_PRIVACY);
	if (bPrivacy)
		return 0;

	setDword("IdleTS", bIdle ? time(0) : 0);

	if (m_bTempVisListEnabled) // remove temporary visible users
		sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_REMOVETEMPVISIBLE, BUL_TEMPVISIBLE);

	icq_setidle(bIdle ? 1 : 0);
	return 0;
}

INT_PTR CIcqProto::RevokeAuthorization(WPARAM wParam, LPARAM lParam)
{
	if (icqOnline() && wParam != 0) {
		DWORD dwUin;
		uid_str szUid;
		if (getContactUid(wParam, &dwUin, &szUid))
			return 0; // Invalid contact

		if (MessageBox(NULL, TranslateT("Are you sure you want to revoke user's authorization?\nThis will remove you from his/her list on some clients."), TranslateT("Confirmation"), MB_ICONQUESTION | MB_YESNO) != IDYES)
			return 0;

		icq_sendRevokeAuthServ(dwUin, szUid);
	}

	return 0;
}


INT_PTR CIcqProto::SendSms(WPARAM wParam, LPARAM lParam)
{
	if (icqOnline() && wParam && lParam)
		return icq_sendSMSServ((const char *)wParam, (const char *)lParam);

	return 0; // Failure
}

INT_PTR CIcqProto::SendYouWereAdded(WPARAM wParam, LPARAM lParam)
{
	if (lParam && icqOnline()) {
		CCSDATA* ccs = (CCSDATA*)lParam;
		if (ccs->hContact) {
			DWORD dwUin, dwMyUin;

			if (getContactUid(ccs->hContact, &dwUin, NULL))
				return 1; // Invalid contact

			dwMyUin = getContactUin(NULL);

			if (dwUin) {
				icq_sendYouWereAddedServ(dwUin, dwMyUin);
				return 0; // Success
			}
		}
	}

	return 1; // Failure
}

INT_PTR CIcqProto::SetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR* tszFile = (TCHAR*)lParam;
	int iRet = -1;

	if (!m_bAvatarsEnabled || !m_bSsiEnabled) return -2;

	if (tszFile) { // set file for avatar
		int dwPaFormat = ::ProtoGetAvatarFileFormat(tszFile);
		if (dwPaFormat != PA_FORMAT_XML) {
			// if it should be image, check if it is valid
			HBITMAP avt = (HBITMAP)CallService(MS_UTILS_LOADBITMAPT, 0, (WPARAM)tszFile);
			if (!avt) return iRet;
			DeleteObject(avt);
		}

		TCHAR tszMyFile[MAX_PATH + 1];
		GetFullAvatarFileName(0, NULL, dwPaFormat, tszMyFile, MAX_PATH);
		// if not in our storage, copy
		if (lstrcmp(tszFile, tszMyFile) && !CopyFile(tszFile, tszMyFile, FALSE)) {
			debugLogA("Failed to copy our avatar to local storage.");
			return iRet;
		}

		BYTE *hash = calcMD5HashOfFile(tszMyFile);
		if (hash) {
			BYTE *ihash = (BYTE*)_alloca(0x14);
			// upload hash to server
			ihash[0] = 0;    //unknown
			ihash[1] = dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC; //hash type
			ihash[2] = 1;    //hash status
			ihash[3] = 0x10; //hash len
			memcpy(ihash + 4, hash, 0x10);
			updateServAvatarHash(ihash, 0x14);

			if (setSettingBlob(NULL, "AvatarHash", ihash, 0x14))
				debugLogA("Failed to save avatar hash.");

			TCHAR tmp[MAX_PATH];
			PathToRelativeT(tszMyFile, tmp);
			setTString(NULL, "AvatarFile", tmp);

			iRet = 0;

			SAFE_FREE((void**)&hash);
		}
	}
	else { // delete user avatar
		delSetting("AvatarFile");
		setSettingBlob(NULL, "AvatarHash", hashEmptyAvatar, 9);
		updateServAvatarHash(hashEmptyAvatar, 9); // set blank avatar
		iRet = 0;
	}

	return iRet;
}

INT_PTR CIcqProto::SetNickName(WPARAM wParam, LPARAM lParam)
{
	if (!icqOnline())
		return 0; // failure

	if (wParam & SMNN_UNICODE)
		setTString("Nick", (WCHAR*)lParam);
	else
		setString("Nick", (char*)lParam);

	return ChangeInfoEx(CIXT_BASIC, 0);
}

// TODO: Adding needs some more work in general

MCONTACT CIcqProto::AddToListByUIN(DWORD dwUin, DWORD dwFlags)
{
	int bAdded;
	MCONTACT hContact = HContactFromUIN(dwUin, &bAdded);
	if (hContact == NULL) // Failure
		return NULL;

	if (!(dwFlags & PALF_TEMPORARY) && db_get_b(hContact, "CList", "NotOnList", 0)) {
		setContactHidden(hContact, 0);
		db_unset(hContact, "CList", "NotOnList");
	}
	return hContact; // Success
}

MCONTACT CIcqProto::AddToListByUID(const char *szUID, DWORD dwFlags)
{
	int bAdded;
	MCONTACT hContact = HContactFromUID(0, szUID, &bAdded);
	if (hContact == 0)
		return 0; // Failure

	if (!(dwFlags & PALF_TEMPORARY) && db_get_b(hContact, "CList", "NotOnList", 0)) {
		setContactHidden(hContact, 0);
		db_unset(hContact, "CList", "NotOnList");
	}

	return hContact; // Success
}


/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::ICQAddRecvEvent(MCONTACT hContact, WORD wType, PROTORECVEVENT* pre, DWORD cbBlob, PBYTE pBlob, DWORD flags)
{
	if (pre->flags & PREF_CREATEREAD)
		flags |= DBEF_READ;

	if (pre->flags & PREF_UTF)
		flags |= DBEF_UTF;

	if (hContact && db_get_b(hContact, "CList", "Hidden", 0)) {
		DWORD dwUin;
		uid_str szUid;

		//setContactHidden(hContact, 0);

		// if the contact was hidden, add to client-list if not in server-list authed
		if (!getWord(hContact, DBSETTING_SERVLIST_ID, 0) || getByte(hContact, "Auth", 0)) {
			getContactUid(hContact, &dwUin, &szUid);
			icq_sendNewContact(dwUin, szUid); /// FIXME
		}
	}

	AddEvent(hContact, wType, pre->timestamp, flags, cbBlob, pBlob);
}

INT_PTR __cdecl CIcqProto::IcqAddCapability(WPARAM wParam, LPARAM lParam)
{
	ICQ_CUSTOMCAP *icqCustomCapIn = (ICQ_CUSTOMCAP*)lParam;
	ICQ_CUSTOMCAP *icqCustomCap = new ICQ_CUSTOMCAP;
	memcpy(icqCustomCap, icqCustomCapIn, sizeof(ICQ_CUSTOMCAP));
	CustomCapList.insert(icqCustomCap);
	return 0;
}


INT_PTR __cdecl CIcqProto::IcqCheckCapability(WPARAM hContact, LPARAM lParam)
{
	int res = 0;
	DBVARIANT dbvariant;
	ICQ_CUSTOMCAP *icqCustomCap = (ICQ_CUSTOMCAP *)lParam;

	db_get(hContact, m_szModuleName, "CapBuf", &dbvariant);
	if (dbvariant.type == DBVT_BLOB)
		res = MatchCapability(dbvariant.pbVal, dbvariant.cpbVal, (const capstr*)&icqCustomCap->caps, 0x10) ? 1 : 0;	// FIXME: Why icqCustomCap->caps is not capstr?

	db_free(&dbvariant);
	return res;
}



/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR icq_getEventTextMissedMessage(WPARAM wParam, LPARAM lParam)
{
	DBEVENTGETTEXT *pEvent = (DBEVENTGETTEXT *)lParam;

	INT_PTR nRetVal = 0;
	char *pszText = NULL;

	if (pEvent->dbei->cbBlob > 1) {
		switch (((WORD*)pEvent->dbei->pBlob)[0]) {
		case 0:
			pszText = LPGEN("** This message was blocked by the ICQ server ** The message was invalid.");
			break;

		case 1:
			pszText = LPGEN("** This message was blocked by the ICQ server ** The message was too long.");
			break;

		case 2:
			pszText = LPGEN("** This message was blocked by the ICQ server ** The sender has flooded the server.");
			break;

		case 4:
			pszText = LPGEN("** This message was blocked by the ICQ server ** You are too evil.");
			break;

		default:
			pszText = LPGEN("** Unknown missed message event.");
			break;
		}
		if (pEvent->datatype == DBVT_WCHAR) {
			WCHAR *pwszText;
			int wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszText, strlennull(pszText), NULL, 0);

			pwszText = (WCHAR*)_alloca((wchars + 1) * sizeof(WCHAR));
			pwszText[wchars] = 0;

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszText, strlennull(pszText), pwszText, wchars);

			nRetVal = (INT_PTR)mir_wstrdup(TranslateW(pwszText));
		}
		else if (pEvent->datatype == DBVT_ASCIIZ)
			nRetVal = (INT_PTR)mir_strdup(Translate(pszText));
	}

	return nRetVal;
}
