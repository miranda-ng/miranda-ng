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

#include "icqoscar.h"

void CIcqProto::handleIcqExtensionsFam(BYTE *pBuffer, size_t wBufferLength, snac_header* pSnacHeader)
{
	switch (pSnacHeader->wSubtype) {
	case ICQ_META_ERROR:
		handleExtensionError(pBuffer, wBufferLength);
		break;

	case ICQ_META_SRV_REPLY:
		handleExtensionServerInfo(pBuffer, wBufferLength, pSnacHeader->wFlags);
		break;

	default:
		debugLogA("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_EXTENSIONS_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;
	}
}


void CIcqProto::handleExtensionError(BYTE *buf, size_t wPackLen)
{
	WORD wErrorCode;

	if (wPackLen < 2)
		wErrorCode = 0;

	if (wPackLen >= 2 && wPackLen <= 6)
		unpackWord(&buf, &wErrorCode);
	else {
		// TODO: cookies need to be handled and freed here on error
		unpackWord(&buf, &wErrorCode);
		wPackLen -= 2;

		oscar_tlv_chain *chain = readIntoTLVChain(&buf, wPackLen, 0);
		if (chain) {
			oscar_tlv* pTLV;

			pTLV = chain->getTLV(0x21, 1); // get meta error data
			if (pTLV && pTLV->wLen >= 8) {
				BYTE *pBuffer = pTLV->pData;
				WORD wData;
				pBuffer += 6;
				unpackLEWord(&pBuffer, &wData); // get request type
				switch (wData) {
				case CLI_META_INFO_REQ:
					if (pTLV->wLen >= 12) {
						WORD wSubType;
						WORD wCookie;

						unpackWord(&pBuffer, &wCookie);
						unpackLEWord(&pBuffer, &wSubType);
						// more sofisticated detection, send ack
						if (wSubType == META_REQUEST_FULL_INFO) {
							MCONTACT hContact;
							cookie_fam15_data *pCookieData = NULL;

							int foundCookie = FindCookie(wCookie, &hContact, (void**)&pCookieData);
							if (foundCookie && pCookieData) {
								ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);

								ReleaseCookie(wCookie);  // we do not leak cookie and memory
							}

							debugLogA("Full info request error 0x%02x received", wErrorCode);
						}
						else if (wSubType == META_SET_PASSWORD_REQ) {
							// failed to change user password, report to UI
							ProtoBroadcastAck(NULL, ACKTYPE_SETINFO, ACKRESULT_FAILED, (HANDLE)wCookie, 0);

							debugLogA("Meta change password request failed, error 0x%02x", wErrorCode);
						}
						else debugLogA("Meta request error 0x%02x received", wErrorCode);
					}
					else debugLogA("Meta request error 0x%02x received", wErrorCode);
					break;

				default:
					debugLogA("Unknown request 0x%02x error 0x%02x received", wData, wErrorCode);
				}
				disposeChain(&chain);
				return;
			}
			disposeChain(&chain);
		}
	}
	LogFamilyError(ICQ_EXTENSIONS_FAMILY, wErrorCode);
}

void CIcqProto::handleExtensionServerInfo(BYTE *buf, size_t wPackLen, WORD wFlags)
{
	// The entire packet is encapsulated in a TLV type 1
	oscar_tlv_chain *chain = readIntoTLVChain(&buf, wPackLen, 0);
	if (chain == NULL) {
		debugLogA("Error: Broken snac 15/3 %d", 1);
		return;
	}

	oscar_tlv *dataTlv = chain->getTLV(0x0001, 1);
	if (dataTlv == NULL) {
		disposeChain(&chain);
		debugLogA("Error: Broken snac 15/3 %d", 2);
		return;
	}
	
	BYTE *databuf = dataTlv->pData;
	wPackLen -= 4;

	_ASSERTE(dataTlv->wLen == wPackLen);
	_ASSERTE(wPackLen >= 10);

	if ((dataTlv->wLen == wPackLen) && (wPackLen >= 10)) {
		DWORD dwMyUin;
		WORD wBytesRemaining, wCookie, wRequestType;
		unpackLEWord(&databuf, &wBytesRemaining);
		unpackLEDWord(&databuf, &dwMyUin);
		unpackLEWord(&databuf, &wRequestType);
		unpackWord(&databuf, &wCookie);

		_ASSERTE(wBytesRemaining == (wPackLen - 2));
		if (wBytesRemaining == (wPackLen - 2)) {
			wPackLen -= 10;
			switch (wRequestType) {
			case SRV_META_INFO_REPLY:     // SRV_META request replies
				handleExtensionMetaResponse(databuf, wPackLen, wCookie, wFlags);
				break;

			default:
				debugLogA("Warning: Ignoring Meta response - Unknown type %d", wRequestType);
				break;
			}
		}
	}
	else debugLogA("Error: Broken snac 15/3 %d", 3);

	if (chain)
		disposeChain(&chain);
}


void CIcqProto::handleExtensionMetaResponse(BYTE *databuf, size_t wPacketLen, WORD wCookie, WORD wFlags)
{
	WORD wReplySubtype;
	BYTE bResultCode;

	_ASSERTE(wPacketLen >= 3);
	if (wPacketLen >= 3) {
		// Reply subtype
		unpackLEWord(&databuf, &wReplySubtype);
		wPacketLen -= 2;

		// Success byte
		unpackByte(&databuf, &bResultCode);
		wPacketLen -= 1;

		switch (wReplySubtype) {
		case META_SET_PASSWORD_ACK:
			parseUserInfoUpdateAck(databuf, wPacketLen, wCookie, wReplySubtype, bResultCode);
			break;

		case SRV_RANDOM_FOUND:
		case SRV_USER_FOUND:
		case SRV_LAST_USER_FOUND:
			parseSearchReplies(databuf, wPacketLen, wCookie, wReplySubtype, bResultCode);
			break;

		case META_PROCESSING_ERROR:  // Meta processing error server reply
			// Todo: We only use this as an SMS ack, that will have to change
			{
				// Terminate buffer
				char *pszInfo = (char *)_alloca(wPacketLen + 1);
				if (wPacketLen > 0)
					memcpy(pszInfo, databuf, wPacketLen);
				pszInfo[wPacketLen] = 0;

				ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_FAILED, (HANDLE)wCookie, (LPARAM)pszInfo);
				FreeCookie(wCookie);
			}
			break;

		case META_SMS_DELIVERY_RECEIPT:
			// Todo: This overlaps with META_SET_AFFINFO_ACK.
			// Todo: Check what happens if result != A
			if (wPacketLen > 8) {
				WORD wNetworkNameLen;
				WORD wAckLen;
				char *pszInfo;

				databuf += 6;    // Some unknowns
				wPacketLen -= 6;

				unpackWord(&databuf, &wNetworkNameLen);
				if (wPacketLen >= (wNetworkNameLen + 2)) {
					databuf += wNetworkNameLen;
					wPacketLen -= wNetworkNameLen;

					unpackWord(&databuf, &wAckLen);
					if (pszInfo = (char *)_alloca(wAckLen + 1)) {
						// Terminate buffer
						if (wAckLen > 0)
							memcpy(pszInfo, databuf, wAckLen);
						pszInfo[wAckLen] = 0;

						ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_SENTREQUEST, (HANDLE)wCookie, (LPARAM)pszInfo);
						FreeCookie(wCookie);

						// Parsing success
						break;
					}
				}
			}

			// Parsing failure
			debugLogA("Error: Failure parsing META_SMS_DELIVERY_RECEIPT");
			break;

		case META_DIRECTORY_DATA:
		case META_DIRECTORY_RESPONSE:
			if (bResultCode == 0x0A)
				handleDirectoryQueryResponse(databuf, wPacketLen, wCookie, wReplySubtype, wFlags);
			else
				debugLogA("Error: Directory request failed, code %u", bResultCode);
			break;

		case META_DIRECTORY_UPDATE_ACK:
			if (bResultCode == 0x0A)
				handleDirectoryUpdateResponse(databuf, wPacketLen, wCookie, wReplySubtype);
			else
				debugLogA("Error: Directory request failed, code %u", bResultCode);
			break;

		case META_BASIC_USERINFO:
		case META_WORK_USERINFO:
		case META_MORE_USERINFO:
		case META_NOTES_USERINFO:
		case META_EMAIL_USERINFO:
		case META_INTERESTS_USERINFO:
		case META_AFFILATIONS_USERINFO:
		case META_SHORT_USERINFO:
		case META_HPAGECAT_USERINFO:
			debugLogA("Warning: Ignored 15/03 (legacy user info) replysubtype x%x", wReplySubtype);
			break;

		default:
			debugLogA("Warning: Ignored 15/03 replysubtype x%x", wReplySubtype);
			break;
		}

		// Success
		return;
	}

	// Failure
	debugLogA("Warning: Broken 15/03 ExtensionMetaResponse");
}

void CIcqProto::ReleaseSearchCookie(DWORD dwCookie, cookie_search *pCookie)
{
	if (pCookie) {
		FreeCookie(dwCookie);
		if (pCookie->dwMainId) {
			if (pCookie->dwStatus) {
				SAFE_FREE((void**)&pCookie);
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)dwCookie, 0);
			}
			else pCookie->dwStatus = 1;
		}
		else {
			SAFE_FREE((void**)&pCookie);
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)dwCookie, 0);
		}
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)dwCookie, 0);
}

void CIcqProto::parseSearchReplies(unsigned char *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype, BYTE bResultCode)
{
	BYTE bParsingOK = FALSE; // For debugging purposes only
	BOOL bLastUser = FALSE;

	cookie_search *pCookie;
	if (!FindCookie(wCookie, NULL, (void**)&pCookie)) {
		debugLogA("Warning: Received unexpected search reply");
		pCookie = NULL;
	}

	switch (wReplySubtype) {
	case SRV_LAST_USER_FOUND: // Search: last user found reply
		bLastUser = TRUE;

	case SRV_USER_FOUND:      // Search: user found reply
		if (bLastUser)
			debugLogA("SNAC(0x15,0x3): Last search reply");
		else
			debugLogA("SNAC(0x15,0x3): Search reply");

		if (bResultCode == 0xA) {
			ICQSEARCHRESULT sr = { 0 };
			DWORD dwUin;
			char szUin[UINMAXLEN];
			size_t wLen;

			sr.hdr.cbSize = sizeof(sr);

			// Remaining bytes
			if (wPacketLen < 2)
				break;
			unpackLEWord(&databuf, &wLen);
			wPacketLen -= 2;

			_ASSERTE(wLen <= wPacketLen);
			if (wLen > wPacketLen)
				break;

			// Uin
			if (wPacketLen < 4)
				break;
			unpackLEDWord(&databuf, &dwUin); // Uin
			wPacketLen -= 4;
			sr.uin = dwUin;
			_itoa(dwUin, szUin, 10);
			sr.hdr.id = (FNAMECHAR*)szUin;

			// Nick
			if (wPacketLen < 2)
				break;
			unpackLEWord(&databuf, &wLen);
			wPacketLen -= 2;
			if (wLen > 0) {
				if (wPacketLen < wLen || (databuf[wLen - 1] != 0))
					break;
				sr.hdr.nick = (FNAMECHAR*)databuf;
				databuf += wLen;
			}
			else {
				sr.hdr.nick = NULL;
			}

			// First name
			if (wPacketLen < 2)
				break;
			unpackLEWord(&databuf, &wLen);
			wPacketLen -= 2;
			if (wLen > 0) {
				if (wPacketLen < wLen || (databuf[wLen - 1] != 0))
					break;
				sr.hdr.firstName = (FNAMECHAR*)databuf;
				databuf += wLen;
			}
			else sr.hdr.firstName = NULL;

			// Last name
			if (wPacketLen < 2)
				break;
			unpackLEWord(&databuf, &wLen);
			wPacketLen -= 2;
			if (wLen > 0) {
				if (wPacketLen < wLen || (databuf[wLen - 1] != 0))
					break;
				sr.hdr.lastName = (FNAMECHAR*)databuf;
				databuf += wLen;
			}
			else sr.hdr.lastName = NULL;

			// E-mail name
			if (wPacketLen < 2)
				break;
			unpackLEWord(&databuf, &wLen);
			wPacketLen -= 2;
			if (wLen > 0) {
				if (wPacketLen < wLen || (databuf[wLen - 1] != 0))
					break;
				sr.hdr.email = (FNAMECHAR*)databuf;
				databuf += wLen;
			}
			else sr.hdr.email = NULL;

			// Authentication needed flag
			if (wPacketLen < 1)
				break;
			unpackByte(&databuf, &sr.auth);

			// Finally, broadcast the result
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)wCookie, (LPARAM)&sr);

			// Broadcast "Last result" ack if this was the last user found
			if (wReplySubtype == SRV_LAST_USER_FOUND) {
				if (wPacketLen >= 10) {
					DWORD dwLeft;

					databuf += 5;
					unpackLEDWord(&databuf, &dwLeft);
					if (dwLeft)
						debugLogA("Warning: %d search results omitted", dwLeft);
				}
				ReleaseSearchCookie(wCookie, pCookie);
			}
			bParsingOK = TRUE;
		}
		else {
			// Failed search
			debugLogA("SNAC(0x15,0x3): Search error %u", bResultCode);

			ReleaseSearchCookie(wCookie, pCookie);

			bParsingOK = TRUE;
		}
		break;

	case SRV_RANDOM_FOUND: // Random search server reply
	default:
		if (pCookie)
			ReleaseCookie(wCookie);
		break;
	}

	// For debugging purposes only
	if (!bParsingOK) {
		debugLogA("Warning: Parsing error in 15/03 search reply type x%x", wReplySubtype);
		_ASSERTE(!bParsingOK);
	}
}

void CIcqProto::parseUserInfoUpdateAck(unsigned char *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype, BYTE bResultCode)
{
	switch (wReplySubtype) {
	case META_SET_PASSWORD_ACK:  // Set user password server ack
		if (bResultCode == 0xA)
			ProtoBroadcastAck(NULL, ACKTYPE_SETINFO, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0);
		else
			ProtoBroadcastAck(NULL, ACKTYPE_SETINFO, ACKRESULT_FAILED, (HANDLE)wCookie, 0);

		FreeCookie(wCookie);
		break;

	default:
		debugLogA("Warning: Ignored 15/03 user info update ack type x%x", wReplySubtype);
		break;
	}
}

UserInfoRecordItem rEmail[] =
{
	{ 0x64, DBVT_UTF8, "e-mail%u" }
};

UserInfoRecordItem rAddress[] =
{
	{ 0x64, DBVT_UTF8, "Street" },
	{ 0x6E, DBVT_UTF8, "City" },
	{ 0x78, DBVT_UTF8, "State" },
	{ 0x82, DBVT_UTF8, "ZIP" },
	{ 0x8C, DBVT_WORD, "Country" }
};

UserInfoRecordItem rOriginAddress[] =
{
	{ 0x64, DBVT_UTF8, "OriginStreet" },
	{ 0x6E, DBVT_UTF8, "OriginCity" },
	{ 0x78, DBVT_UTF8, "OriginState" },
	{ 0x8C, DBVT_WORD, "OriginCountry" }
};

UserInfoRecordItem rCompany[] =
{
	{ 0x64, DBVT_UTF8, "CompanyPosition" },
	{ 0x6E, DBVT_UTF8, "Company" },
	{ 0x7D, DBVT_UTF8, "CompanyDepartment" },
	{ 0x78, DBVT_UTF8, "CompanyHomepage" },
	{ 0x82, DBVT_WORD, "CompanyIndustry" },
	{ 0xAA, DBVT_UTF8, "CompanyStreet" },
	{ 0xB4, DBVT_UTF8, "CompanyCity" },
	{ 0xBE, DBVT_UTF8, "CompanyState" },
	{ 0xC8, DBVT_UTF8, "CompanyZIP" },
	{ 0xD2, DBVT_WORD, "CompanyCountry" }
};

UserInfoRecordItem rEducation[] =
{
	{ 0x64, DBVT_WORD, "StudyLevel" },
	{ 0x6E, DBVT_UTF8, "StudyInstitute" },
	{ 0x78, DBVT_UTF8, "StudyDegree" },
	{ 0x8C, DBVT_WORD, "StudyYear" }
};

UserInfoRecordItem rInterest[] =
{
	{ 0x64, DBVT_UTF8, "Interest%uText" },
	{ 0x6E, DBVT_WORD, "Interest%uCat" }
};

int CIcqProto::parseUserInfoRecord(MCONTACT hContact, oscar_tlv *pData, UserInfoRecordItem pRecordDef[], int nRecordDef, int nMaxRecords)
{
	int nRecords = 0;

	if (pData && pData->wLen >= 2) {
		BYTE *pRecords = pData->pData;
		WORD wRecordCount;
		unpackWord(&pRecords, &wRecordCount);
		oscar_tlv_record_list *cData = readIntoTLVRecordList(&pRecords, pData->wLen - 2, nMaxRecords > wRecordCount ? wRecordCount : nMaxRecords);
		oscar_tlv_record_list *cDataItem = cData;
		while (cDataItem) {
			oscar_tlv_chain *cItem = cDataItem->item;

			for (int i = 0; i < nRecordDef; i++) {
				char szItemKey[MAX_PATH];
				mir_snprintf(szItemKey, MAX_PATH, pRecordDef[i].szDbSetting, nRecords);

				switch (pRecordDef[i].dbType) {
				case DBVT_UTF8:
					writeDbInfoSettingTLVStringUtf(hContact, szItemKey, cItem, pRecordDef[i].wTLV);
					break;

				case DBVT_WORD:
					writeDbInfoSettingTLVWord(hContact, szItemKey, cItem, pRecordDef[i].wTLV);
					break;
				}
			}
			nRecords++;

			cDataItem = cDataItem->next;
		}
		// release memory
		disposeRecordList(&cData);
	}

	// remove old data from database
	if (!nRecords || nMaxRecords > 1)
		for (int i = nRecords; i <= nMaxRecords; i++)
			for (int j = 0; j < nRecordDef; j++) {
				char szItemKey[MAX_PATH];
				mir_snprintf(szItemKey, MAX_PATH, pRecordDef[j].szDbSetting, i);
				delSetting(hContact, szItemKey);
			}

	return nRecords;
}


void CIcqProto::handleDirectoryQueryResponse(BYTE *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype, WORD wFlags)
{
	WORD wBytesRemaining = 0;
	snac_header requestSnac = { 0 };
	BYTE requestResult;

	debugLogA("Received directory query response");

	if (wPacketLen >= 2)
		unpackLEWord(&databuf, &wBytesRemaining);
	wPacketLen -= 2;
	_ASSERTE(wPacketLen == wBytesRemaining);

	if (!unpackSnacHeader(&requestSnac, &databuf, &wPacketLen) || !requestSnac.bValid) {
		debugLogA("Error: Failed to parse directory response");
		return;
	}

	cookie_directory_data *pCookieData;
	MCONTACT hContact;
	// check request cookie
	if (!FindCookie(wCookie, &hContact, (void**)&pCookieData) || !pCookieData) {
		debugLogA("Warning: Ignoring unrequested directory reply type (x%x, x%x)", requestSnac.wFamily, requestSnac.wSubtype);
		return;
	}
	/// FIXME: we should really check the snac contents according to cookie data here ?? 

	// Check if this is the last packet for this request
	BOOL bMoreDataFollows = wFlags & 0x0001 && requestSnac.wFlags & 0x0001;

	if (wPacketLen >= 3)
		unpackByte(&databuf, &requestResult);
	else {
		debugLogA("Error: Malformed directory response");
		if (!bMoreDataFollows)
			ReleaseCookie(wCookie);
		return;
	}

	if (requestResult != 1 && requestResult != 4) {
		debugLogA("Error: Directory request failed, status %u", requestResult);

		if (!bMoreDataFollows) {
			if (pCookieData->bRequestType == DIRECTORYREQUEST_INFOUSER)
				ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);
			else if (pCookieData->bRequestType == DIRECTORYREQUEST_SEARCH)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0); // should report error here, but Find/Add module does not support that
			ReleaseCookie(wCookie);
		}
		return;
	}

	size_t wLen;
	unpackWord(&databuf, &wLen);
	wPacketLen -= 3;
	if (wLen)
		debugLogA("Warning: Data in error message present!");

	if (wPacketLen <= 0x16) { // sanity check
		debugLogA("Error: Malformed directory response");

		if (!bMoreDataFollows) {
			if (pCookieData->bRequestType == DIRECTORYREQUEST_INFOUSER)
				ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);
			else if (pCookieData->bRequestType == DIRECTORYREQUEST_SEARCH)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0); // should report error here, but Find/Add module does not support that
			ReleaseCookie(wCookie);
		}
		return;
	}
	databuf += 0x10; // unknown stuff
	wPacketLen -= 0x10;

	DWORD dwItemCount;
	WORD wPageCount;

	/// FIXME: check itemcount, pagecount against the cookie data ???

	unpackDWord(&databuf, &dwItemCount);
	unpackWord(&databuf, &wPageCount);
	wPacketLen -= 6;

	if (pCookieData->bRequestType == DIRECTORYREQUEST_SEARCH && !bMoreDataFollows)
		debugLogA("Directory Search: %d contacts found (%u pages)", dwItemCount, wPageCount);

	if (wPacketLen <= 2) { // sanity check, block expected
		debugLogA("Error: Malformed directory response");

		if (!bMoreDataFollows) {
			if (pCookieData->bRequestType == DIRECTORYREQUEST_INFOUSER)
				ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);
			else if (pCookieData->bRequestType == DIRECTORYREQUEST_SEARCH)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0); // should report error here, but Find/Add module does not support that
			ReleaseCookie(wCookie);
		}
		return;
	}

	WORD wData;
	unpackWord(&databuf, &wData); // This probably the count of items following (a block)
	wPacketLen -= 2;
	if (wPacketLen >= 2 && wData >= 1) {
		unpackWord(&databuf, &wLen);  // This is the size of the first item
		wPacketLen -= 2;
	}

	if (wData == 0 && pCookieData->bRequestType == DIRECTORYREQUEST_SEARCH) {
		debugLogA("Directory Search: No contacts found");
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0);
		ReleaseCookie(wCookie);
		return;
	}

	_ASSERTE(wData == 1 && wPacketLen == wLen);
	if (wData != 1 || wPacketLen != wLen) {
		debugLogA("Error: Malformed directory response (missing data)");

		if (!bMoreDataFollows) {
			if (pCookieData->bRequestType == DIRECTORYREQUEST_INFOUSER)
				ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);
			else if (pCookieData->bRequestType == DIRECTORYREQUEST_SEARCH)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0); // should report error here, but Find/Add module does not support that
			ReleaseCookie(wCookie);
		}
		return;
	}

	oscar_tlv_chain *pDirectoryData = readIntoTLVChain(&databuf, wLen, -1);
	if (pDirectoryData) {
		switch (pCookieData->bRequestType) {
		case DIRECTORYREQUEST_INFOOWNER:
			parseDirectoryUserDetailsData(NULL, pDirectoryData, wCookie, pCookieData, wReplySubtype);
			break;

		case DIRECTORYREQUEST_INFOUSER:
			{
				DWORD dwUin = 0;
				char *szUid = pDirectoryData->getString(0x32, 1);
				if (!szUid) {
					debugLogA("Error: Received unrecognized data from the directory");
					break;
				}

				if (IsStringUIN(szUid))
					dwUin = atoi(szUid);

				if (hContact != HContactFromUID(dwUin, szUid, NULL)) {
					debugLogA("Error: Received data does not match cookie contact, ignoring.");
					SAFE_FREE(&szUid);
					break;
				}
				else SAFE_FREE(&szUid);
			}

		case DIRECTORYREQUEST_INFOMULTI:
			parseDirectoryUserDetailsData(hContact, pDirectoryData, wCookie, pCookieData, wReplySubtype);
			break;

		case DIRECTORYREQUEST_SEARCH:
			parseDirectorySearchData(pDirectoryData, wCookie, pCookieData, wReplySubtype);
			break;

		default:
			debugLogA("Error: Unknown cookie type %x for directory response!", pCookieData->bRequestType);
		}
		disposeChain(&pDirectoryData);
	}
	else debugLogA("Error: Failed parsing directory response");

	// Release Memory
	if (!bMoreDataFollows)
		ReleaseCookie(wCookie);
}

static int calcAgeFromBirthDate(double dDate)
{
	if (dDate > 0) { // date is stored as double with unit equal to a day, incrementing since 1/1/1900 0:00 GMT
		SYSTEMTIME sDate = { 0 };
		if (VariantTimeToSystemTime(dDate + 2, &sDate)) {
			SYSTEMTIME sToday = { 0 };
			GetLocalTime(&sToday);

			int nAge = sToday.wYear - sDate.wYear;
			if (sToday.wMonth < sDate.wMonth || (sToday.wMonth == sDate.wMonth && sToday.wDay < sDate.wDay))
				nAge--;

			return nAge;
		}
	}
	return 0;
}

void CIcqProto::parseDirectoryUserDetailsData(MCONTACT hContact, oscar_tlv_chain *cDetails, DWORD dwCookie, cookie_directory_data *pCookieData, WORD wReplySubType)
{
	if (pCookieData->bRequestType == DIRECTORYREQUEST_INFOMULTI && !hContact) {
		DWORD dwUin = 0;
		char *szUid = cDetails->getString(0x32, 1);
		if (!szUid) {
			debugLogA("Error: Received unrecognized data from the directory");
			return;
		}

		if (IsStringUIN(szUid))
			dwUin = atoi(szUid);

		hContact = HContactFromUID(dwUin, szUid, NULL);
		if (hContact == INVALID_CONTACT_ID) {
			debugLogA("Error: Received details for unknown contact \"%s\"", szUid);
			SAFE_FREE(&szUid);
			return;
		}
		
		debugLogA("Received user info for %s from directory", szUid);
		SAFE_FREE(&szUid);
	}

	oscar_tlv *pTLV = cDetails->getTLV(0x50, 1);
	if (pTLV && pTLV->wLen > 0)
		writeDbInfoSettingTLVStringUtf(hContact, "e-mail", cDetails, 0x50); // Verified e-mail
	else
		writeDbInfoSettingTLVStringUtf(hContact, "e-mail", cDetails, 0x55); // Pending e-mail

	writeDbInfoSettingTLVStringUtf(hContact, "FirstName", cDetails, 0x64);
	writeDbInfoSettingTLVStringUtf(hContact, "LastName", cDetails, 0x6E);
	writeDbInfoSettingTLVStringUtf(hContact, "Nick", cDetails, 0x78);
	// Home Address
	parseUserInfoRecord(hContact, cDetails->getTLV(0x96, 1), rAddress, SIZEOF(rAddress), 1);
	// Origin Address
	parseUserInfoRecord(hContact, cDetails->getTLV(0xA0, 1), rOriginAddress, SIZEOF(rOriginAddress), 1);
	// Phones
	pTLV = cDetails->getTLV(0xC8, 1);
	if (pTLV && pTLV->wLen >= 2) {
		BYTE *pRecords = pTLV->pData;
		WORD wRecordCount;
		unpackWord(&pRecords, &wRecordCount);
		oscar_tlv_record_list *cPhones = readIntoTLVRecordList(&pRecords, pTLV->wLen - 2, wRecordCount);
		if (cPhones) {
			oscar_tlv_chain *cPhone;
			cPhone = cPhones->getRecordByTLV(0x6E, 1);
			writeDbInfoSettingTLVStringUtf(hContact, "Phone", cPhone, 0x64);
			cPhone = cPhones->getRecordByTLV(0x6E, 2);
			writeDbInfoSettingTLVStringUtf(hContact, "CompanyPhone", cPhone, 0x64);
			cPhone = cPhones->getRecordByTLV(0x6E, 3);
			writeDbInfoSettingTLVStringUtf(hContact, "Cellular", cPhone, 0x64);
			cPhone = cPhones->getRecordByTLV(0x6E, 4);
			writeDbInfoSettingTLVStringUtf(hContact, "Fax", cPhone, 0x64);
			cPhone = cPhones->getRecordByTLV(0x6E, 5);
			writeDbInfoSettingTLVStringUtf(hContact, "CompanyFax", cPhone, 0x64);

			disposeRecordList(&cPhones);
		}
		else { // Remove old data when phones not available
			delSetting(hContact, "Phone");
			delSetting(hContact, "CompanyPhone");
			delSetting(hContact, "Cellular");
			delSetting(hContact, "Fax");
			delSetting(hContact, "CompanyFax");
		}
	}
	else { // Remove old data when phones not available
		delSetting(hContact, "Phone");
		delSetting(hContact, "CompanyPhone");
		delSetting(hContact, "Cellular");
		delSetting(hContact, "Fax");
		delSetting(hContact, "CompanyFax");
	}
	// Emails
	parseUserInfoRecord(hContact, cDetails->getTLV(0x8C, 1), rEmail, SIZEOF(rEmail), 4);

	writeDbInfoSettingTLVByte(hContact, "Timezone", cDetails, 0x17C);
	// Company
	parseUserInfoRecord(hContact, cDetails->getTLV(0x118, 1), rCompany, SIZEOF(rCompany), 1);
	// Education
	parseUserInfoRecord(hContact, cDetails->getTLV(0x10E, 1), rEducation, SIZEOF(rEducation), 1);

	switch (cDetails->getNumber(0x82, 1)) {
	case 1:
		setByte(hContact, "Gender", 'F');
		break;
	case 2:
		setByte(hContact, "Gender", 'M');
		break;
	default:
		delSetting(hContact, "Gender");
	}

	writeDbInfoSettingTLVStringUtf(hContact, "Homepage", cDetails, 0xFA);
	writeDbInfoSettingTLVDate(hContact, "BirthYear", "BirthMonth", "BirthDay", cDetails, 0x1A4);

	writeDbInfoSettingTLVByte(hContact, "Language1", cDetails, 0xAA);
	writeDbInfoSettingTLVByte(hContact, "Language2", cDetails, 0xB4);
	writeDbInfoSettingTLVByte(hContact, "Language3", cDetails, 0xBE);

	writeDbInfoSettingTLVByte(hContact, "MaritalStatus", cDetails, 0x12C);
	// Interests
	parseUserInfoRecord(hContact, cDetails->getTLV(0x122, 1), rInterest, SIZEOF(rInterest), 4);

	writeDbInfoSettingTLVStringUtf(hContact, "About", cDetails, 0x186);

	//	if (hContact)
	//		writeDbInfoSettingTLVStringUtf(hContact, DBSETTING_STATUS_NOTE, cDetails, 0x226);
	//	else
	if (!hContact) { // Owner contact needs special processing, in the database is current status note for the client
		// We just received the last status note set on directory, if it differs call SetStatusNote() to 
		// ensure the directory will be updated (it should be in process anyway)
		char *szClientStatusNote = getSettingStringUtf(hContact, DBSETTING_STATUS_NOTE, NULL);
		char *szDirectoryStatusNote = cDetails->getString(0x226, 1);

		if (strcmpnull(szClientStatusNote, szDirectoryStatusNote))
			SetStatusNote(szClientStatusNote, 1000, TRUE);

		// Release memory
		SAFE_FREE(&szDirectoryStatusNote);
		SAFE_FREE(&szClientStatusNote);
	}

	writeDbInfoSettingTLVByte(hContact, "PrivacyLevel", cDetails, 0x1F9);

	if (!hContact) {
		setByte(hContact, "Auth", !cDetails->getByte(0x19A, 1));
		writeDbInfoSettingTLVByte(hContact, "WebAware", cDetails, 0x212);
		writeDbInfoSettingTLVByte(hContact, "AllowSpam", cDetails, 0x1EA);
	}

	writeDbInfoSettingTLVWord(hContact, "InfoCP", cDetails, 0x1C2);

	if (hContact) { // Handle deprecated setting (Age & Birthdate are not separate fields anymore)
		int nAge = calcAgeFromBirthDate(cDetails->getDouble(0x1A4, 1));

		if (nAge)
			setWord(hContact, "Age", nAge);
		else
			delSetting(hContact, "Age");
	}
	else // we do not need to calculate age for owner
		delSetting(hContact, "Age");

	{ // Save user info last update time and privacy token
		double dInfoTime;
		BYTE pbEmptyMetaToken[0x10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int bHasMetaToken = FALSE;

		// Check if the details arrived with privacy token!
		if ((pTLV = cDetails->getTLV(0x3C, 1)) && pTLV->wLen == 0x10 && memcmp(pTLV->pData, pbEmptyMetaToken, 0x10))
			bHasMetaToken = TRUE;

		// !Important, we need to save the MDir server-item time - it can be newer than the one from the directory
		if ((dInfoTime = getSettingDouble(hContact, DBSETTING_METAINFO_TIME, 0)) > 0)
			setSettingDouble(hContact, DBSETTING_METAINFO_SAVED, dInfoTime);
		else if (bHasMetaToken || !hContact)
			writeDbInfoSettingTLVDouble(hContact, DBSETTING_METAINFO_SAVED, cDetails, 0x1CC);
		else
			setDword(hContact, DBSETTING_METAINFO_SAVED, time(NULL));
	}

	if (wReplySubType == META_DIRECTORY_RESPONSE)
		if (pCookieData->bRequestType == DIRECTORYREQUEST_INFOUSER)
			ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);

	// Remove user from info update queue. Removing is fast so we always call this
	// even if it is likely that the user is not queued at all.
	if (hContact)
		icq_DequeueUser(getContactUin(hContact));
}


void CIcqProto::parseDirectorySearchData(oscar_tlv_chain *cDetails, DWORD dwCookie, cookie_directory_data *pCookieData, WORD wReplySubType)
{
	char *szUid = cDetails->getString(0x32, 1); // User ID

	debugLogA("Directory Search: Found user %s", szUid);

	ICQSEARCHRESULT isr = { 0 };
	isr.hdr.cbSize = sizeof(ICQSEARCHRESULT);
	isr.hdr.flags = PSR_TCHAR;
	isr.hdr.id = ansi_to_tchar(szUid);

	if (IsStringUIN(szUid))
		isr.uin = atoi(szUid);
	else
		isr.uin = 0;

	SAFE_FREE(&szUid);

	oscar_tlv *pTLV = cDetails->getTLV(0x50, 1);
	char *szData = NULL;

	if (pTLV && pTLV->wLen > 0)
		szData = cDetails->getString(0x50, 1); // Verified e-mail
	else
		szData = cDetails->getString(0x55, 1); // Pending e-mail
	if (mir_strlen(szData))
		isr.hdr.email = ansi_to_tchar(szData);
	SAFE_FREE(&szData);

	szData = cDetails->getString(0x64, 1); // First Name
	if (mir_strlen(szData))
		isr.hdr.firstName = utf8_to_tchar(szData);
	SAFE_FREE(&szData);

	szData = cDetails->getString(0x6E, 1); // Last Name
	if (mir_strlen(szData))
		isr.hdr.lastName = utf8_to_tchar(szData);
	SAFE_FREE(&szData);

	szData = cDetails->getString(0x78, 1); // Nick
	if (mir_strlen(szData))
		isr.hdr.nick = utf8_to_tchar(szData);
	SAFE_FREE(&szData);

	switch (cDetails->getNumber(0x82, 1)) // Gender
	{
	case 1:
		isr.gender = 'F';
		break;
	case 2:
		isr.gender = 'M';
		break;
	}

	pTLV = cDetails->getTLV(0x96, 1);
	if (pTLV && pTLV->wLen >= 4) {
		BYTE *buf = pTLV->pData;
		oscar_tlv_chain *chain = readIntoTLVChain(&buf, pTLV->wLen, 0);
		if (chain)
			isr.country = chain->getDWord(0x8C, 1); // Home Country
		disposeChain(&chain);
	}

	isr.auth = !cDetails->getByte(0x19A, 1); // Require Authorization
	isr.maritalStatus = cDetails->getNumber(0x12C, 1); // Marital Status

	// calculate Age if Birthdate is available
	isr.age = calcAgeFromBirthDate(cDetails->getDouble(0x1A4, 1));

	// Finally, broadcast the result
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)dwCookie, (LPARAM)&isr);

	// Release memory
	SAFE_FREE(&isr.hdr.id);
	SAFE_FREE(&isr.hdr.nick);
	SAFE_FREE(&isr.hdr.firstName);
	SAFE_FREE(&isr.hdr.lastName);
	SAFE_FREE(&isr.hdr.email);

	// Search is over, broadcast final ack
	if (wReplySubType == META_DIRECTORY_RESPONSE)
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)dwCookie, 0);
}


void CIcqProto::handleDirectoryUpdateResponse(BYTE *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype)
{
	WORD wBytesRemaining = 0;
	snac_header requestSnac = { 0 };
	BYTE requestResult;

	debugLogA("Received directory update response");

	if (wPacketLen >= 2)
		unpackLEWord(&databuf, &wBytesRemaining);
	wPacketLen -= 2;
	_ASSERTE(wPacketLen == wBytesRemaining);

	if (!unpackSnacHeader(&requestSnac, &databuf, &wPacketLen) || !requestSnac.bValid) {
		debugLogA("Error: Failed to parse directory response");
		return;
	}

	cookie_directory_data *pCookieData;
	MCONTACT hContact;
	// check request cookie
	if (!FindCookie(wCookie, &hContact, (void**)&pCookieData) || !pCookieData) {
		debugLogA("Warning: Ignoring unrequested directory reply type (x%x, x%x)", requestSnac.wFamily, requestSnac.wSubtype);
		return;
	}
	/// FIXME: we should really check the snac contents according to cookie data here ?? 

	if (wPacketLen >= 3)
		unpackByte(&databuf, &requestResult);
	else {
		debugLogA("Error: Malformed directory response");
		ReleaseCookie(wCookie);
		return;
	}
	if (requestResult != 1 && requestResult != 4) {
		debugLogA("Error: Directory request failed, status %u", requestResult);

		if (pCookieData->bRequestType == DIRECTORYREQUEST_UPDATEOWNER)
			ProtoBroadcastAck(NULL, ACKTYPE_SETINFO, ACKRESULT_FAILED, (HANDLE)wCookie, 0);

		ReleaseCookie(wCookie);
		return;
	}
	size_t wLen;

	unpackWord(&databuf, &wLen);
	wPacketLen -= 3;
	if (wLen)
		debugLogA("Warning: Data in error message present!");

	if (pCookieData->bRequestType == DIRECTORYREQUEST_UPDATEOWNER)
		ProtoBroadcastAck(NULL, ACKTYPE_SETINFO, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0);
	if (wPacketLen == 0x18) {
		DWORD64 qwMetaTime;
		BYTE pbEmptyMetaToken[0x10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		unpackQWord(&databuf, &qwMetaTime);
		setSettingBlob(NULL, DBSETTING_METAINFO_TIME, (BYTE*)&qwMetaTime, 8);

		if (memcmp(databuf, pbEmptyMetaToken, 0x10))
			setSettingBlob(NULL, DBSETTING_METAINFO_TOKEN, databuf, 0x10);
	}
	ReleaseCookie(wCookie);
}
