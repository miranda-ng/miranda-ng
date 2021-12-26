/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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

/********************************************************************
	created:	2006/04/11
	created:	11:4:2006   17:28
	filename: 	MirandaExchange.cpp
	file base:	MirandaExchange
	file ext:	cpp
	author:		Attila Vajda

	purpose:	Miranda Exchange Plugin
*********************************************************************/

#include "stdafx.h"
#include "MirandaExchange.h"
#include "utils.h"

#include <time.h>

#ifndef NO_EXCHANGE_TEST

//////////////////////////////////////////////////////////////////////////
HRESULT HrMAPIFindDefaultMsgStore(    // RETURNS: return code
	IN LPMAPISESSION lplhSession,   // session pointer
	OUT ULONG *lpcbeid,             // count of bytes in entry ID
	OUT LPENTRYID *lppeid)          // entry ID of default store
{
	HRESULT hr = NOERROR;
	HRESULT hrT = NOERROR;
	SCODE sc = 0;
	LPMAPITABLE lpTable = nullptr;
	LPSRowSet lpRows = nullptr;
	LPENTRYID lpeid = nullptr;
	ULONG cbeid = 0;
	ULONG cRows = 0;
	ULONG i = 0;

	SizedSPropTagArray(2, rgPropTagArray) = {2,{PR_DEFAULT_STORE,PR_ENTRYID}};

	// Get the list of available message stores from MAPI
	hrT = MAPICALL(lplhSession)->GetMsgStoresTable(0, &lpTable);
	if (FAILED(hrT))
		goto err_out;
	// Get the row count for the message recipient table
	hrT = MAPICALL(lpTable)->GetRowCount(0, &cRows);
	if (FAILED(hrT))
		goto err_out;
	// Set the columns to return
	hrT = MAPICALL(lpTable)->SetColumns((LPSPropTagArray)&rgPropTagArray, 0);
	if (FAILED(hrT))
		goto err_out;
	// Go to the beginning of the recipient table for the envelope
	hrT = MAPICALL(lpTable)->SeekRow(BOOKMARK_BEGINNING, 0, nullptr);
	if (FAILED(hrT))
		goto err_out;
	// Read all the rows of the table
	hrT = MAPICALL(lpTable)->QueryRows(cRows, 0, &lpRows);
	if (SUCCEEDED(hrT) && lpRows != nullptr && lpRows->cRows == 0) {
		hrT = MAPI_E_NOT_FOUND;
		goto err_out;
	}

	if (FAILED(hrT))
		goto err_out;
	for (i = 0; i < cRows; i++) {
		if (lpRows->aRow[i].lpProps[0].Value.b == FALSE)
			continue;
		cbeid = lpRows->aRow[i].lpProps[1].Value.bin.cb;
		sc = MAPIAllocateBuffer(cbeid, (void **)&lpeid);
		if (FAILED(sc)) {
			cbeid = 0;
			lpeid = nullptr;
		}
		else {
			// Copy entry ID of message store
			memcpy(lpeid, lpRows->aRow[i].lpProps[1].Value.bin.lpb, cbeid);
		}
		break;
	}

err_out:
	if (lpRows != nullptr)
		FreeProws(lpRows);
	UlRelease(lpTable);
	*lpcbeid = cbeid;
	*lppeid = lpeid;

	return hr;
}

CKeeper::CKeeper(LPTSTR szSender, LPTSTR szSubject, LPSTR szEntryID)
{
	m_szSender = nullptr;
	m_szSubject = nullptr;
	m_szEntryID = nullptr;
	m_nSizeSender = 0;
	m_nSizeSubject = 0;
	m_nSizeEntryID = 0;

	if (nullptr != szSender) {
		m_nSizeSender = (UINT)mir_wstrlen(szSender) + 1;
		m_szSender = new wchar_t[m_nSizeSender];
		memset(m_szSender, 0, m_nSizeSender * sizeof(wchar_t));
		mir_wstrcpy(m_szSender, szSender);
	}

	if (nullptr != szSubject) {
		m_nSizeSubject = (UINT)mir_wstrlen(szSubject) + 1;
		m_szSubject = new wchar_t[m_nSizeSubject];
		memset(m_szSubject, 0, m_nSizeSubject * sizeof(wchar_t));
		mir_wstrcpy(m_szSubject, szSubject);
	}

	if (nullptr != szEntryID) {
		m_nSizeEntryID = (UINT)mir_strlen(szEntryID) + 1;
		m_szEntryID = new char[m_nSizeEntryID];
		memset(m_szEntryID, 0, m_nSizeEntryID * sizeof(char));
		mir_strcpy(m_szEntryID, szEntryID);
	}
}

CKeeper::~CKeeper()
{
	if (m_nSizeSender > 0 && nullptr != m_szSender) {
		m_nSizeSender = 0;
		delete[] m_szSender;
		m_szSender = nullptr;
	}

	if (m_nSizeSubject > 0 && nullptr != m_szSubject) {
		m_nSizeSubject = 0;
		delete[] m_szSubject;
		m_szSubject = nullptr;
	}

	if (m_nSizeEntryID > 0 && nullptr != m_szEntryID) {
		m_nSizeEntryID = 0;
		delete[] m_szEntryID;
		m_szEntryID = nullptr;
	}
}

CMirandaExchange::~CMirandaExchange()
{
	if (nullptr != m_szUsername) {
		delete[] m_szUsername;
		m_szUsername = nullptr;
	}

	if (nullptr != m_szPassword) {
		delete[] m_szPassword;
		m_szPassword = nullptr;
	}

	if (nullptr != m_szExchangeServer) {
		delete[] m_szExchangeServer;
		m_szExchangeServer = nullptr;
	}

	if (nullptr != m_lpInbox) {
		UlRelease(m_lpInbox);
		m_lpInbox = nullptr;
	}

	if (nullptr != m_lpMDB) {
		UlRelease(m_lpMDB);
		m_lpMDB = nullptr;
	}

	if (nullptr != m_lpMAPISession) {
		m_lpMAPISession->Logoff(NULL, NULL, NULL);
		UlRelease(m_lpMAPISession);
		m_lpMAPISession = nullptr;
	}

	if (m_nNumberOfHeaders > 0) {
		for (UINT i = 0; i < m_nNumberOfHeaders; i++) {
			if (nullptr != m_HeadersKeeper[i]) {
				delete m_HeadersKeeper[i];
				m_HeadersKeeper[i] = nullptr;
			}
		}

		m_nNumberOfHeaders = 0;
	}

	//MAPIUninitialize(); 
}


HRESULT CallOpenEntry(LPMDB lpMDB, LPADRBOOK lpAB, LPMAPICONTAINER lpContainer, LPMAPISESSION lpMAPISession,
	ULONG cbEntryID, LPENTRYID lpEntryID, ULONG ulFlags, ULONG* ulObjTypeRet, LPUNKNOWN* lppUnk)
{
	if (!lppUnk) return MAPI_E_INVALID_PARAMETER;
	HRESULT			hRes = S_OK;
	ULONG			ulObjType = NULL;
	LPUNKNOWN		lpUnk = nullptr;
	ULONG			ulNoCacheFlags = NULL;

	*lppUnk = nullptr;

	//ulFlags |= MAPI_NO_CACHE;
	//in case we need to retry without MAPI_NO_CACHE - do not add MAPI_NO_CACHE to ulFlags after this point
	//if (MAPI_NO_CACHE & ulFlags) ulNoCacheFlags = ulFlags & ~MAPI_NO_CACHE;
	ulNoCacheFlags = ulFlags;

	if (lpMDB) {
		//Log(L"CallOpenEntry: Calling OpenEntry on MDB with ulFlags = 0x%X\n",ulFlags);
		lpMDB->OpenEntry(
			cbEntryID,
			lpEntryID,
			nullptr,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk);
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags) {
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
			(lpMDB->OpenEntry(
				cbEntryID,
				lpEntryID,
				nullptr,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes)) {
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
		}
	}
	if (lpAB && !lpUnk) {
		hRes = S_OK;
		//Log(L"CallOpenEntry: Calling OpenEntry on AB with ulFlags = 0x%X\n",ulFlags);
		(lpAB->OpenEntry(
			cbEntryID,
			lpEntryID,
			nullptr,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk));
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags) {
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
			(lpAB->OpenEntry(
				cbEntryID,
				lpEntryID,
				nullptr,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes)) {
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
		}
	}

	if (lpContainer && !lpUnk) {
		hRes = S_OK;
		//Log(L"CallOpenEntry: Calling OpenEntry on Container with ulFlags = 0x%X\n",ulFlags);
		(lpContainer->OpenEntry(
			cbEntryID,
			lpEntryID,
			nullptr,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk));
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags) {
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
			(lpContainer->OpenEntry(
				cbEntryID,
				lpEntryID,
				nullptr,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes)) {
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
		}
	}

	if (lpMAPISession && !lpUnk) {
		hRes = S_OK;
		//Log(L"CallOpenEntry: Calling OpenEntry on Session with ulFlags = 0x%X\n",ulFlags);
		(lpMAPISession->OpenEntry(
			cbEntryID,
			lpEntryID,
			nullptr,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk));
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags) {
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
			(lpMAPISession->OpenEntry(
				cbEntryID,
				lpEntryID,
				nullptr,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes)) {
			if (lpUnk) (lpUnk)->Release();
			lpUnk = nullptr;
		}
	}

	if (lpUnk) {
		//Log(L"OnOpenEntryID: Got object (0x%08X) of type 0x%08X = %s\n",lpUnk,ulObjType,ObjectTypeToString(ulObjType));
		*lppUnk = lpUnk;
	}
	if (ulObjTypeRet) *ulObjTypeRet = ulObjType;
	return hRes;
}

HRESULT CallOpenEntry(LPMDB lpMDB, LPADRBOOK lpAB, LPMAPICONTAINER lpContainer, LPMAPISESSION lpMAPISession,
	LPSBinary lpSBinary, ULONG ulFlags, ULONG* ulObjTypeRet, LPUNKNOWN* lppUnk)
{
	return CallOpenEntry(lpMDB, lpAB, lpContainer, lpMAPISession, lpSBinary ? lpSBinary->cb : 0,
		(LPENTRYID)(lpSBinary ? lpSBinary->lpb : nullptr), ulFlags, ulObjTypeRet, lppUnk);
}

HRESULT CMirandaExchange::InitializeAndLogin(LPCTSTR szUsername, LPCTSTR szPassword, LPCTSTR szExchangeServer)
{
	_popupUtil(TranslateT("Connecting to Exchange ..."));
	UINT  nSize = 0;
	short nSizeOfTCHAR = sizeof(wchar_t);

	if (m_szUsername == nullptr && nullptr != szUsername) {
		nSize = (UINT)mir_wstrlen(szUsername);
		if (nSize > 0) {
			nSize++;
			m_szUsername = new wchar_t[nSize];
			memset(m_szUsername, 0, nSize * nSizeOfTCHAR);
			mir_wstrcpy(m_szUsername, szUsername);
		}
	}

	if (m_szPassword == nullptr && nullptr != szPassword) {
		nSize = (UINT)mir_wstrlen(szPassword);
		if (nSize > 0) {
			nSize++;
			m_szPassword = new wchar_t[nSize];
			memset(m_szPassword, 0, nSize * nSizeOfTCHAR);
			mir_wstrcpy(m_szPassword, szPassword);
		}
	}

	if (m_szExchangeServer == nullptr && nullptr != szExchangeServer) {
		nSize = (UINT)mir_wstrlen(szExchangeServer);
		if (nSize > 0) {
			nSize++;
			m_szExchangeServer = new wchar_t[nSize];
			memset(m_szExchangeServer, 0, nSize * nSizeOfTCHAR);
			mir_wstrcpy(m_szExchangeServer, szExchangeServer);
		}
	}

	if (!m_bLoginOK || m_lpInbox || nullptr == m_lpMAPISession) {
		HRESULT hr = S_OK;
		MAPIINIT_0 mapiInit = {MAPI_INIT_VERSION , MAPI_MULTITHREAD_NOTIFICATIONS};

		if (!m_bNoInitAgain) {
			m_bNoInitAgain = true;
			hr = MAPIInitialize(&mapiInit);
		}

		if (SUCCEEDED(hr)) {
			wchar_t szPIDandName[128];
			wchar_t szPID[20];

			_wstrtime(szPID);
			wcsncpy(szPIDandName, m_szUsername, _countof(szPIDandName) - 1);
			mir_wstrncat(szPIDandName, szPID, _countof(szPIDandName) - mir_wstrlen(szPIDandName));

			hr = CreateProfile(szPIDandName);
			if (FAILED(hr)) {
				//Log("Create profile failed: 0x%08X", hr);
				return hr;
			}

			uint32_t dwFlags = MAPI_EXPLICIT_PROFILE | MAPI_EXTENDED | MAPI_NEW_SESSION | MAPI_NO_MAIL;

			hr = MAPILogonEx(0, (LPTSTR)mir_u2a(szPIDandName), (LPTSTR)mir_u2a(m_szPassword), dwFlags, &m_lpMAPISession);
			if (FAILED(hr)) {
				//Log( L"MAPI Logon failed: 0x%08X", hr );
				return hr;
			}

			LPPROFADMIN pProfAdmin = nullptr;
			hr = MAPIAdminProfiles(0, &pProfAdmin);
			if ((FAILED(hr)) || (nullptr == pProfAdmin)) {
				//Log("Admin profile interface creation failed: 0x%08X", hr);
			}
			else {
				hr = pProfAdmin->DeleteProfile((LPTSTR)mir_u2a(szPIDandName), 0);
				if (FAILED(hr)) {
					//Log( "Failed to delete the profile: 0x%08X", hr );
				}
			}

			if (pProfAdmin)
				pProfAdmin->Release();

			ULONG cbDefStoreEid = 0;
			CMAPIBuffer<LPENTRYID>  pDefStoreEid = NULL;
			hr = HrMAPIFindDefaultMsgStore(m_lpMAPISession, &cbDefStoreEid, &pDefStoreEid);
			if (FAILED(hr))
				return hr;

			// Open default message store
			LPMDB pDefMsgStore = nullptr;
			hr = m_lpMAPISession->OpenMsgStore(0, cbDefStoreEid, pDefStoreEid, nullptr, MAPI_BEST_ACCESS, &pDefMsgStore);
			if (nullptr == pDefMsgStore)
				return hr;

			ULONG cbInboxEID = NULL;
			CMAPIBuffer<LPENTRYID>  lpInboxEID = NULL;
			HRESULT hRes = pDefMsgStore->GetReceiveFolder(L"IPM", NULL, &cbInboxEID, &lpInboxEID, nullptr);
			m_lpMDB = pDefMsgStore;
			if (cbInboxEID && lpInboxEID) {
				hRes = CallOpenEntry(pDefMsgStore, nullptr, nullptr, nullptr, cbInboxEID, lpInboxEID, MAPI_BEST_ACCESS, nullptr, (LPUNKNOWN*)&m_lpInbox);

				if (m_lpInbox && hRes == S_OK)
					m_bFolderInboxOK = true;
			}
		}
	}

	return S_OK;
}

HRESULT CMirandaExchange::CreateProfile(LPTSTR szProfileName)
{
	enum { iSvcName, iSvcUID, cptaSvc };

	SizedSPropTagArray(cptaSvc, sptCols) =
	{
		cptaSvc,
			PR_SERVICE_NAME,
			PR_SERVICE_UID
	};

	ULONG ulFlags = 0;
	CMAPIInterface<LPPROFADMIN> pProfAdmin = NULL;
	HRESULT hr = MAPIAdminProfiles(ulFlags, &pProfAdmin);
	if (FAILED(hr) || pProfAdmin == nullptr)
		return hr;

	hr = pProfAdmin->CreateProfile(szProfileName, nullptr, NULL, ulFlags);
	if (FAILED(hr)) {
		pProfAdmin->DeleteProfile(szProfileName, ulFlags);
		return hr;
	}

	CMAPIInterface<LPSERVICEADMIN> pMsgSvcAdmin = NULL;
	hr = pProfAdmin->AdminServices(szProfileName, nullptr, NULL, ulFlags, &pMsgSvcAdmin);
	if (FAILED(hr) || pMsgSvcAdmin == nullptr)
		return hr;

	hr = pMsgSvcAdmin->CreateMsgService(L"MSEMS", L"", NULL, 0);
	if (FAILED(hr))
		return hr;

	CMAPIInterface<LPMAPITABLE> pMsgSvcTable = NULL;
	hr = pMsgSvcAdmin->GetMsgServiceTable(0, &pMsgSvcTable);
	if (FAILED(hr) || pMsgSvcTable == nullptr)
		return hr;

	SRestriction sres;
	sres.rt = RES_CONTENT;
	sres.res.resContent.ulFuzzyLevel = FL_FULLSTRING;
	sres.res.resContent.ulPropTag = PR_SERVICE_NAME_A;
	SPropValue spv;
	sres.res.resContent.lpProp = &spv;
	spv.ulPropTag = PR_SERVICE_NAME_A;
	spv.Value.lpszA = (LPSTR)"MSEMS";

	LPSRowSet pRows = nullptr;
	hr = HrQueryAllRows(pMsgSvcTable, (LPSPropTagArray)&sptCols, &sres, nullptr, 0, &pRows);
	if (FAILED(hr))
		return hr;
	
	size_t nSize = mir_wstrlen(m_szUsername);
	wchar_t *szUniqName = (wchar_t*)mir_alloc(sizeof(wchar_t) * (nSize + 4));
	if (szUniqName != nullptr) {
		memcpy(szUniqName, L"=", sizeof(wchar_t));
		memcpy((szUniqName + 1), m_szUsername, (sizeof(wchar_t) * (nSize + 1)));
		
		// Set values for PR_PROFILE_UNRESOLVED_NAME and PR_PROFILE_UNRESOLVED_SERVER
		SPropValue spval[2];
		spval[0].ulPropTag = PR_PROFILE_UNRESOLVED_NAME;
		spval[0].Value.lpszA = mir_u2a(szUniqName);
		spval[1].ulPropTag = PR_PROFILE_UNRESOLVED_SERVER;
		spval[1].Value.lpszA = mir_u2a(m_szExchangeServer);

		// Configure msg service
		pMsgSvcAdmin->ConfigureMsgService((LPMAPIUID)pRows->aRow->lpProps[iSvcUID].Value.bin.lpb, 0, NULL, 2, spval);
		mir_free(szUniqName);
	}

	FreeProws(pRows);
	return hr;
}

HRESULT CMirandaExchange::isMapiSessionOK(LPMAPISESSION)
{
	return S_OK;
}

HRESULT CMirandaExchange::CheckForNewMails(int &nNewMails)
{
	if (m_nNumberOfHeaders > 0 && nullptr != m_HeadersKeeper) {
		for (UINT i = 0; i < m_nNumberOfHeaders; i++) {
			if (nullptr != m_HeadersKeeper[i]) {
				delete m_HeadersKeeper[i];
				m_HeadersKeeper[i] = nullptr;
			}
		}

		m_nNumberOfHeaders = 0;
	}

	m_nNumberOfHeaders = 0;

	HRESULT hRes;
	try {
		if (m_lpMAPISession != nullptr && (isMapiSessionOK(m_lpMAPISession) == S_OK) && m_lpInbox != nullptr && m_bFolderInboxOK) {
			hRes = CheckInFolder(m_lpInbox);
		}
		else {
			m_bLoginOK = 0;
			hRes = InitializeAndLogin(nullptr, nullptr, nullptr);

			if (hRes == S_OK) {
				hRes = CheckInFolder(m_lpInbox);
			}
		}

		if (hRes == S_OK) {
			nNewMails = m_nNumberOfHeaders;
		}
	}
	catch (...) {
		hRes = E_FAIL;
	}

	return hRes;
}

HRESULT CMirandaExchange::LogOFF()
{
	try {
		if (nullptr != m_lpInbox) {
			UlRelease(m_lpInbox);
			m_lpInbox = nullptr;
		}

		if (nullptr != m_lpMDB) {
			UlRelease(m_lpMDB);
			m_lpMDB = nullptr;
		}

		if (nullptr != m_lpMAPISession) {
			m_lpMAPISession->Logoff(NULL, NULL, NULL);
			m_lpMAPISession->Release();
			m_lpMAPISession = nullptr;
		}

		if (m_nNumberOfHeaders > 0 && nullptr != m_HeadersKeeper) {
			for (UINT i = 0; i < m_nNumberOfHeaders; i++) {
				if (nullptr != m_HeadersKeeper[i]) {
					delete m_HeadersKeeper[i];
					m_HeadersKeeper[i] = nullptr;
				}
			}

			m_nNumberOfHeaders = 0;
		}
	}
	catch (...) {

	}

	return S_OK;
}

HRESULT CMirandaExchange::MarkAsRead(LPTSTR szEntryID)
{
	LPMESSAGE lpMessage = nullptr;
	LPBYTE lpData = nullptr;
	ULONG  ulC = 0;

	HexToBin(szEntryID, ulC, lpData);

	CallOpenEntry(m_lpMDB, nullptr, nullptr, m_lpMAPISession, ulC, (LPENTRYID)lpData, MAPI_BEST_ACCESS, nullptr, (LPUNKNOWN*)&lpMessage);
	delete lpData;

	if (nullptr != lpMessage) {
		lpMessage->SetReadFlag(0);
		lpMessage->SaveChanges(FORCE_SAVE);

		lpMessage->Release();
		lpMessage = nullptr;
	}

	return 0;
}


HRESULT CMirandaExchange::CheckInFolder(LPMAPIFOLDER lpFolder)
{
	HRESULT hr = NOERROR;
	CMAPIInterface<LPMAPITABLE> lpTable = NULL;
	LPSRowSet lpRow = nullptr;
	LPSPropValue lpRowProp = nullptr;
	ULONG i = 0L;
	wchar_t* szSenderName = nullptr;
	wchar_t* szSubject = nullptr;
	LPSTR szEntryID = nullptr;

	if (lpFolder == nullptr || !m_bFolderInboxOK)
		return hr;

	SizedSPropTagArray(5, sptaDETAILS) =
	{
		5,
		{
			PR_ENTRYID,
				PR_MESSAGE_FLAGS,
				PR_SENDER_NAME,
				PR_ORIGINAL_SENDER_EMAIL_ADDRESS,
				PR_SUBJECT
		}
	};

	CMAPIInterface<LPMAPITABLE> lpMessageTable;

	hr = lpFolder->GetContentsTable(0, &lpMessageTable);
	if (FAILED(hr))
		return E_FAIL;

	LPSRowSet lpRowsR = nullptr;

	//////////////////////////////////////////////////////////////////////////
	SRestriction srRoot;
	srRoot.rt = RES_BITMASK;
	srRoot.res.resBitMask.relBMR = BMR_EQZ;
	srRoot.res.resBitMask.ulPropTag = PR_MESSAGE_FLAGS;
	srRoot.res.resBitMask.ulMask = MSGFLAG_READ;

	SizedSSortOrderSet(1, sso) = {1, 0, 0, { PR_MESSAGE_DELIVERY_TIME, TABLE_SORT_DESCEND }};

	hr = HrQueryAllRows(lpMessageTable, (LPSPropTagArray)&sptaDETAILS, &srRoot, (LPSSortOrderSet)& sso, 0L, &lpRowsR);

	//////////////////////////////////////////////////////////////////////////

	if (FAILED(hr))
		return E_FAIL;

	for (i = 0; (i < lpRowsR->cRows) && (m_nNumberOfHeaders < MAX_NUMBER_OF_HEADERS); ++i) {
		if (!(lpRowsR->aRow[i].lpProps[1].Value.l & MSGFLAG_READ)) {

			if (!FAILED(lpRowsR->aRow[i].lpProps[2].Value.err)) {
				szSenderName = lpRowsR->aRow[i].lpProps[2].Value.lpszW;
			}

			if (nullptr == szSenderName) {
				if (!FAILED(lpRowsR->aRow[i].lpProps[3].Value.err)) {
					szSenderName = lpRowsR->aRow[i].lpProps[3].Value.lpszW;
				}
			}


			if (!FAILED(lpRowsR->aRow[i].lpProps[4].Value.err)) {
				szSubject = lpRowsR->aRow[i].lpProps[4].Value.lpszW;
			}

			szEntryID = BinToHex(lpRowsR->aRow[i].lpProps[0].Value.bin.cb, lpRowsR->aRow[i].lpProps[0].Value.bin.lpb);
			m_HeadersKeeper[m_nNumberOfHeaders] = new CKeeper(szSenderName, szSubject, szEntryID);
			m_nNumberOfHeaders++;

			delete[] szEntryID;

			szEntryID = nullptr;
			szSubject = nullptr;
			szSenderName = nullptr;
		}
	}
	FreeProws(lpRowsR);


	if (m_nNumberOfHeaders < MAX_NUMBER_OF_HEADERS) {
		const enum { IDISPNAME, IENTRYID, ICHILDCOUNT };

		static SizedSPropTagArray(3, rgColProps) =
		{
			3,
			{
				PR_DISPLAY_NAME_A,
				PR_ENTRYID,
				PR_FOLDER_CHILD_COUNT
			}
		};


		hr = MAPICALL(lpFolder)->GetHierarchyTable(MAPI_DEFERRED_ERRORS, &lpTable);
		if (!FAILED(hr)) {
			hr = HrQueryAllRows(lpTable, (LPSPropTagArray)&rgColProps, nullptr, nullptr, 0L, &lpRow);
			if (!FAILED(hr)) {
				for (i = 0; i < lpRow->cRows; i++) {
					lpRowProp = lpRow->aRow[i].lpProps;
					CMAPIInterface<LPMAPIFOLDER> lpSubFolder = NULL;
					hr = CallOpenEntry(m_lpMDB, nullptr, nullptr, nullptr, lpRowProp[IENTRYID].Value.bin.cb, (LPENTRYID)lpRowProp[IENTRYID].Value.bin.lpb, MAPI_BEST_ACCESS, nullptr, (LPUNKNOWN*)&lpSubFolder);
					if (!FAILED(hr)) {
						hr = CheckInFolder(lpSubFolder);
						//if (FAILED(hr) ){//Log("failed checkinfolder for %s\n",lpRowProp[IDISPNAME].Value.lpszA );}
					}
				}
				FreeProws(lpRow);
				lpRow = nullptr;
			}
		}
	}

	return hr;
}

HRESULT CMirandaExchange::OpenTheMessage(LPTSTR)
{
	//(Default)//// HKEY_CLASSES_ROOT\mailto\shell\open\command
	HKEY hTheKey;
	HRESULT hRes = E_FAIL;

	wchar_t szRegValue[512];
	DWORD dwLength = 512;
	DWORD dwType = REG_SZ;

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, L"mailto\\shell\\open\\command", 0, KEY_ALL_ACCESS | KEY_EXECUTE | KEY_QUERY_VALUE, &hTheKey) == ERROR_SUCCESS) {
		LONG lResult = RegQueryValueEx(hTheKey, nullptr, nullptr, (LPDWORD)&dwType, (LPBYTE)szRegValue, &dwLength);
		RegCloseKey(hTheKey);

		if (lResult != ERROR_SUCCESS) 
			hRes = E_FAIL;
		else {
			wchar_t *szTheEnd = wcsstr(szRegValue, L".EXE");
			if (nullptr != szTheEnd) {
				szRegValue[mir_wstrlen(szRegValue) - mir_wstrlen(szTheEnd) + 5] = '\0';
				mir_wstrcat(szRegValue, L" /recycle");

				STARTUPINFO si;
				memset(&si, 0, sizeof(STARTUPINFO));
				si.cb = sizeof(STARTUPINFO);
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_SHOWNORMAL;

				PROCESS_INFORMATION pi;
				if (CreateProcessW(nullptr, szRegValue, nullptr, nullptr, 0, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &pi)) {
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
					hRes = S_OK;
				}
			}
		}
	}
	else hRes = E_FAIL;

	return hRes;
}

#endif //NO_EXCHANGE_TEST
