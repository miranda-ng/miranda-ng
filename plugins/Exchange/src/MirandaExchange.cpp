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
	LPMAPITABLE lpTable = NULL;
	LPSRowSet lpRows = NULL;
	LPENTRYID lpeid = NULL;
	ULONG cbeid = 0;
	ULONG cRows = 0;
	ULONG i = 0;

	SizedSPropTagArray(2, rgPropTagArray)={2,{PR_DEFAULT_STORE,PR_ENTRYID}};

	// Get the list of available message stores from MAPI
	hrT = MAPICALL(lplhSession)->GetMsgStoresTable( 0, &lpTable);
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
	hrT = MAPICALL(lpTable)->SeekRow( BOOKMARK_BEGINNING, 0, NULL);
	if (FAILED(hrT))
		goto err_out;
	// Read all the rows of the table
	hrT = MAPICALL(lpTable)->QueryRows( cRows, 0, &lpRows);
	if (SUCCEEDED(hrT) && lpRows != NULL && lpRows->cRows == 0) {
		hrT = MAPI_E_NOT_FOUND;
		goto err_out;
	}

	if (FAILED(hrT))
		goto err_out;
	for (i = 0; i < cRows; i ++) {
		if (lpRows->aRow[i].lpProps[0].Value.b == FALSE)
			continue;
		cbeid = lpRows->aRow[i].lpProps[1].Value.bin.cb;
		sc = MAPIAllocateBuffer(cbeid, (void **)&lpeid);
		if (FAILED(sc)) {
			cbeid = 0;
			lpeid = NULL;
		} else {
			// Copy entry ID of message store
			memcpy(lpeid, lpRows->aRow[i].lpProps[1].Value.bin.lpb, cbeid);
		}
		break;
	}

err_out:
	if (lpRows != NULL)
		FreeProws(lpRows);
	UlRelease(lpTable);
	*lpcbeid = cbeid;
	*lppeid = lpeid;

	return hr;
}


CKeeper::CKeeper( LPTSTR szSender, LPTSTR szSubject, LPSTR szEntryID)
{
	m_szSender        = NULL ;
	m_szSubject       = NULL ;
	m_szEntryID       = NULL ;
	m_nSizeSender     = 0    ;
	m_nSizeSubject    = 0    ;
	m_nSizeEntryID    = 0    ;
 
	if (NULL != szSender) {
		m_nSizeSender = (UINT)mir_tstrlen(szSender)+1;
		m_szSender = new TCHAR[ m_nSizeSender ];
		memset(m_szSender, 0, m_nSizeSender * sizeof(TCHAR));
		_tcscpy(m_szSender, szSender);
	}
	
	if (NULL != szSubject) {
		m_nSizeSubject = (UINT)mir_tstrlen(szSubject) +1;
		m_szSubject = new TCHAR[m_nSizeSubject];
		memset(m_szSubject, 0, m_nSizeSubject * sizeof(TCHAR));
		_tcscpy(m_szSubject, szSubject);
	}
	
	if (NULL != szEntryID) {
		m_nSizeEntryID = (UINT)mir_strlen( szEntryID ) +1;
		m_szEntryID = new char[m_nSizeEntryID];
		memset(m_szEntryID, 0, m_nSizeEntryID * sizeof(char));
		strcpy(m_szEntryID, szEntryID );
	}
}

CKeeper::~CKeeper()
{
	if ( m_nSizeSender>0 && NULL != m_szSender )
	{
		m_nSizeSender =0;
		delete[] m_szSender;
		m_szSender    = NULL;
	}

	if ( m_nSizeSubject>0 && NULL != m_szSubject )
	{
		m_nSizeSubject =0;
		delete[] m_szSubject;
		m_szSubject   = NULL;
	}

	if ( m_nSizeEntryID>0 && NULL != m_szEntryID )
	{
		m_nSizeEntryID = 0;
		delete[] m_szEntryID;
		m_szEntryID = NULL;
	}
}

CMirandaExchange::CMirandaExchange()
{
	m_szUsername       = NULL  ;
	m_szPassword       = NULL  ;
	m_szExchangeServer = NULL  ;
	m_lpMAPISession    = NULL  ;
	m_lpInbox          = NULL  ;
	m_lpMDB            = NULL;
	m_bLoginOK         = false ;
	m_bFolderInboxOK   = false ;
	m_nNumberOfHeaders = 0     ;
}

CMirandaExchange::~CMirandaExchange()
{
	if ( NULL != m_szUsername )
	{
		delete[] m_szUsername;
		m_szUsername = NULL;
	}

	if ( NULL != m_szPassword )
	{
		delete[] m_szPassword;
		m_szPassword = NULL;
	}	
	
	if ( NULL != m_szExchangeServer )
	{
		delete[] m_szExchangeServer;
		m_szExchangeServer = NULL;
	}

    if ( NULL != m_lpInbox )
    {
		UlRelease(m_lpInbox);
		m_lpInbox = NULL;
    }

	if ( NULL != m_lpMDB )
	{
		UlRelease(m_lpMDB );
		m_lpMDB = NULL;
	}
	
	if ( NULL!= m_lpMAPISession )
	{
		m_lpMAPISession->Logoff(NULL,NULL,NULL);
		UlRelease(m_lpMAPISession );
		m_lpMAPISession = NULL;
	}

	if ( m_nNumberOfHeaders>0)
	{
		for( UINT i=0; i<m_nNumberOfHeaders; i++ )
		{
			if ( NULL != m_HeadersKeeper[i])
			{
				delete m_HeadersKeeper[i];
				m_HeadersKeeper[i] = NULL;
			}
		}

		m_nNumberOfHeaders =0 ;
	}

	//MAPIUninitialize(); 
}


HRESULT CallOpenEntry( LPMDB lpMDB, LPADRBOOK lpAB, LPMAPICONTAINER lpContainer, LPMAPISESSION lpMAPISession,
                       ULONG cbEntryID, LPENTRYID lpEntryID, ULONG ulFlags, ULONG* ulObjTypeRet, LPUNKNOWN* lppUnk)
{
	if (!lppUnk) return MAPI_E_INVALID_PARAMETER;
	HRESULT			hRes = S_OK;
	ULONG			ulObjType = NULL;
	LPUNKNOWN		lpUnk = NULL;
	ULONG			ulNoCacheFlags = NULL;
	
	*lppUnk = NULL;

	//ulFlags |= MAPI_NO_CACHE;
	//in case we need to retry without MAPI_NO_CACHE - do not add MAPI_NO_CACHE to ulFlags after this point
	//if (MAPI_NO_CACHE & ulFlags) ulNoCacheFlags = ulFlags & ~MAPI_NO_CACHE;
	ulNoCacheFlags = ulFlags;

	if (lpMDB)
	{
		//Log(_T("CallOpenEntry: Calling OpenEntry on MDB with ulFlags = 0x%X\n"),ulFlags);
		lpMDB->OpenEntry(
			cbEntryID,
			lpEntryID,
			NULL,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk);
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags)
		{
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
			(lpMDB->OpenEntry(
				cbEntryID,
				lpEntryID,
				NULL,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes))
		{
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
		}
	}
	if (lpAB && !lpUnk)
	{
		hRes = S_OK;
		//Log(_T("CallOpenEntry: Calling OpenEntry on AB with ulFlags = 0x%X\n"),ulFlags);
		(lpAB->OpenEntry(
			cbEntryID,
			lpEntryID,
			NULL,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk));
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags)
		{
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
			(lpAB->OpenEntry(
				cbEntryID,
				lpEntryID,
				NULL,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes))
		{
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
		}
	}

	if (lpContainer && !lpUnk)
	{
		hRes = S_OK;
		//Log(_T("CallOpenEntry: Calling OpenEntry on Container with ulFlags = 0x%X\n"),ulFlags);
		(lpContainer->OpenEntry(
			cbEntryID,
			lpEntryID,
			NULL,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk));
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags)
		{
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
			(lpContainer->OpenEntry(
				cbEntryID,
				lpEntryID,
				NULL,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes))
		{
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
		}
	}

	if (lpMAPISession && !lpUnk)
	{
		hRes = S_OK;
		//Log(_T("CallOpenEntry: Calling OpenEntry on Session with ulFlags = 0x%X\n"),ulFlags);
		(lpMAPISession->OpenEntry(
			cbEntryID,
			lpEntryID,
			NULL,//no interface
			ulFlags,
			&ulObjType,
			&lpUnk));
		if (MAPI_E_UNKNOWN_FLAGS == hRes && ulNoCacheFlags)
		{
			hRes = S_OK;
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
			(lpMAPISession->OpenEntry(
				cbEntryID,
				lpEntryID,
				NULL,//no interface
				ulNoCacheFlags,
				&ulObjType,
				&lpUnk));
		}
		if (FAILED(hRes))
		{
			if (lpUnk) (lpUnk)->Release();
			lpUnk = NULL;
		}
	}

	if (lpUnk)
	{
		//Log(_T("OnOpenEntryID: Got object (0x%08X) of type 0x%08X = %s\n"),lpUnk,ulObjType,ObjectTypeToString(ulObjType));
		*lppUnk = lpUnk;		
	}
	if (ulObjTypeRet) *ulObjTypeRet = ulObjType;
	return hRes;
}

HRESULT CallOpenEntry( LPMDB lpMDB, LPADRBOOK lpAB, LPMAPICONTAINER lpContainer, LPMAPISESSION lpMAPISession,
                       LPSBinary lpSBinary, ULONG ulFlags, ULONG* ulObjTypeRet, LPUNKNOWN* lppUnk)
{
	return CallOpenEntry( lpMDB, lpAB, lpContainer, lpMAPISession, lpSBinary?lpSBinary->cb:0, 
					(LPENTRYID)(lpSBinary?lpSBinary->lpb:0), ulFlags, ulObjTypeRet, lppUnk);
}

HRESULT CMirandaExchange::InitializeAndLogin( LPCTSTR szUsername, LPCTSTR szPassword, LPCTSTR szExchangeServer )
{
	_popupUtil(TranslateT("Connecting to Exchange ..."));
	UINT  nSize = 0;
	short nSizeOfTCHAR = sizeof( TCHAR );

	if (m_szUsername == NULL && NULL != szUsername) {
		nSize = (UINT)mir_tstrlen(szUsername);
		if (nSize > 0) {	
			nSize++;
			m_szUsername = new TCHAR[nSize];
			memset ( m_szUsername, 0, nSize * nSizeOfTCHAR );
			_tcscpy( m_szUsername, szUsername );
		}
	}	
	
	if (m_szPassword == NULL && NULL != szPassword) {
		nSize = (UINT)mir_tstrlen(szPassword);
		if (nSize > 0) {	
			nSize++;
			m_szPassword = new TCHAR[nSize];
			memset(m_szPassword, 0, nSize * nSizeOfTCHAR);
			_tcscpy(m_szPassword, szPassword);
		}
	}

	if (m_szExchangeServer == NULL && NULL != szExchangeServer) {
		nSize = (UINT)mir_tstrlen(szExchangeServer);
		if (nSize > 0) {	
			nSize++;
			m_szExchangeServer = new TCHAR[nSize];
			memset(m_szExchangeServer, 0, nSize * nSizeOfTCHAR);
			_tcscpy(m_szExchangeServer, szExchangeServer);
		}
	}
	
	if (!m_bLoginOK || m_lpInbox || NULL == m_lpMAPISession) {
		HRESULT hr          = S_OK;
		MAPIINIT_0 mapiInit = { MAPI_INIT_VERSION , MAPI_MULTITHREAD_NOTIFICATIONS };
		
		if ( !m_bNoInitAgain) {
			m_bNoInitAgain = true;
			hr = MAPIInitialize( &mapiInit) ;
		}
		
		if ( SUCCEEDED(hr)) {
			TCHAR	szPIDandName[128];
			TCHAR	szPID[20];

			_tstrtime(szPID);
			_tcsncpy(szPIDandName, m_szUsername, SIZEOF(szPIDandName)-1);		
			_tcsncat(szPIDandName, szPID, SIZEOF(szPIDandName) - mir_tstrlen(szPIDandName));
			
			hr = CreateProfile(szPIDandName);
			if ( HR_FAILED(hr)) {
				//Log("Create profile failed: 0x%08X", hr);
				return hr;
			}

			DWORD dwFlags = MAPI_EXPLICIT_PROFILE|MAPI_EXTENDED|MAPI_NEW_SESSION|MAPI_NO_MAIL ;
			
			hr = MAPILogonEx( 0, (LPTSTR)mir_t2a(szPIDandName), (LPTSTR)mir_t2a(m_szPassword), dwFlags, &m_lpMAPISession );

			if (FAILED(hr)) {
				//Log( _T("MAPI Logon failed: 0x%08X"), hr );
				return hr;
			}
			
			LPPROFADMIN pProfAdmin = NULL;
			hr = MAPIAdminProfiles( 0, &pProfAdmin );
			
			if ((FAILED(hr)) || (NULL == pProfAdmin)) 
			{
				//Log("Admin profile interface creation failed: 0x%08X", hr);
			}
			else {
				hr = pProfAdmin->DeleteProfile( (LPTSTR)mir_t2a(szPIDandName), 0 );
				if ( FAILED(hr) )
				{
					//Log( "Failed to delete the profile: 0x%08X", hr );
				}
			}
			
			if (pProfAdmin)
				pProfAdmin->Release();
			
			ULONG cbDefStoreEid = 0;

			CMAPIBuffer< LPENTRYID>  pDefStoreEid = NULL;
			hr = HrMAPIFindDefaultMsgStore(m_lpMAPISession, &cbDefStoreEid, &pDefStoreEid );
			if (FAILED(hr)) 
				return hr;
			
			// Open default message store
			LPMDB pDefMsgStore = NULL;
			hr = m_lpMAPISession->OpenMsgStore(0, cbDefStoreEid, pDefStoreEid, NULL,
				MAPI_BEST_ACCESS, &pDefMsgStore);
			
			HRESULT                  hRes       = S_OK;
			ULONG                    cbInboxEID = NULL;
			CMAPIBuffer< LPENTRYID>  lpInboxEID = NULL;
			
			if (NULL == pDefMsgStore )
				return hr;

			hRes = pDefMsgStore->GetReceiveFolder( _T("IPM"), NULL, &cbInboxEID,  &lpInboxEID, NULL);
			m_lpMDB = pDefMsgStore;
			if (cbInboxEID && lpInboxEID) {
				hRes = CallOpenEntry( pDefMsgStore, NULL, NULL, NULL, cbInboxEID, lpInboxEID, MAPI_BEST_ACCESS, NULL, (LPUNKNOWN*)&m_lpInbox);
			
				if ( m_lpInbox && hRes == S_OK)
					m_bFolderInboxOK = true;
			}
		}
	}

	return S_OK;
}

HRESULT CMirandaExchange::CreateProfile( LPTSTR szProfileName )
{
	HRESULT	hr = S_OK;
	CMAPIInterface<LPPROFADMIN> pProfAdmin = NULL;
	CMAPIInterface<LPSERVICEADMIN> pMsgSvcAdmin = NULL;
	CMAPIInterface<LPMAPITABLE> pMsgSvcTable = NULL;
	LPSRowSet pRows = NULL;
	ULONG ulFlags = 0;
	SRestriction sres;
	SIZE_T nSize;
	TCHAR* szUniqName;
	enum {iSvcName, iSvcUID, cptaSvc};
	
	SizedSPropTagArray(cptaSvc, sptCols) = 
	{	
		cptaSvc,
			PR_SERVICE_NAME,
			PR_SERVICE_UID
	};
	ulFlags &= ~MAPI_UNICODE;
	hr = MAPIAdminProfiles(ulFlags, &pProfAdmin);
	if (FAILED(hr) || pProfAdmin == NULL)
		return hr;
	hr = pProfAdmin->CreateProfile((LPTSTR)mir_t2a(szProfileName), NULL, NULL, ulFlags);
	
	if (FAILED(hr)) {
		pProfAdmin->DeleteProfile((LPTSTR)mir_t2a(szProfileName), ulFlags);
		return hr;
	}
	hr = pProfAdmin->AdminServices( (LPTSTR)mir_t2a(szProfileName), NULL, NULL, ulFlags, &pMsgSvcAdmin);
	
	if (FAILED(hr) || pMsgSvcAdmin == NULL)
		return hr;
	hr = pMsgSvcAdmin->CreateMsgService((LPTSTR)("MSEMS"), (LPTSTR)("")/*"Microsoft Exchange Server"*/, NULL, 0);
	
	if (FAILED(hr))
		return hr;
	hr = pMsgSvcAdmin->GetMsgServiceTable(0, &pMsgSvcTable);
	if (FAILED(hr) || pMsgSvcTable == NULL)
		return hr;

	sres.rt = RES_CONTENT;
	sres.res.resContent.ulFuzzyLevel = FL_FULLSTRING;
	sres.res.resContent.ulPropTag = PR_SERVICE_NAME_A;
	SPropValue spv;
	sres.res.resContent.lpProp = &spv;
	spv.ulPropTag = PR_SERVICE_NAME_A;
	spv.Value.lpszA = (LPSTR)"MSEMS";
	
	hr = HrQueryAllRows(pMsgSvcTable, 
		(LPSPropTagArray) &sptCols,
		&sres,
		NULL,
		0,
		&pRows);
	
	if (FAILED(hr))
		return hr;
	nSize = mir_tstrlen(m_szUsername);
	szUniqName = (TCHAR*)mir_alloc(sizeof(TCHAR) * (nSize + 4));
	if (szUniqName != NULL) {
		memcpy(szUniqName, _T("="), sizeof(TCHAR));
		memcpy((szUniqName + 1), m_szUsername, (sizeof(TCHAR) * (nSize + 1)));
		// Set values for PR_PROFILE_UNRESOLVED_NAME and PR_PROFILE_UNRESOLVED_SERVER
		SPropValue spval[2];
		spval[0].ulPropTag = PR_PROFILE_UNRESOLVED_NAME;
		spval[0].Value.lpszA = mir_t2a(szUniqName);
		spval[1].ulPropTag = PR_PROFILE_UNRESOLVED_SERVER;
		spval[1].Value.lpszA = mir_t2a(m_szExchangeServer);

		// Configure msg service
		/*hr =*/ pMsgSvcAdmin->ConfigureMsgService(
				(LPMAPIUID) pRows->aRow->lpProps[iSvcUID].Value.bin.lpb,
				0, NULL, 2, spval);

		mir_free(szUniqName);
	}
	FreeProws(pRows);

	return hr;
}

HRESULT CMirandaExchange::isMapiSessionOK( LPMAPISESSION )
{
	return S_OK;
}

HRESULT CMirandaExchange::CheckForNewMails( int &nNewMails)
{
	if ( m_nNumberOfHeaders>0 && NULL != m_HeadersKeeper )
	{
		for( UINT i=0; i<m_nNumberOfHeaders; i++ )
		{
			if ( NULL != m_HeadersKeeper[i])
			{
				delete m_HeadersKeeper[i];
				m_HeadersKeeper[i] = NULL;
			}
		}

		m_nNumberOfHeaders =0 ;
	}
	
	m_nNumberOfHeaders = 0;

	HRESULT hRes;
	try
	{
		if ( m_lpMAPISession != NULL && (isMapiSessionOK(m_lpMAPISession)== S_OK) && m_lpInbox != NULL && m_bFolderInboxOK )
		{
			hRes= CheckInFolder( m_lpInbox );
		}
		else
		{	
			m_bLoginOK = 0;
			hRes = InitializeAndLogin(NULL,NULL,NULL);

			if (hRes == S_OK)
			{
				hRes = CheckInFolder( m_lpInbox );
			}
		}

		if (hRes == S_OK)
		{
			nNewMails = m_nNumberOfHeaders;
		}
	}
	catch (...)
	{
		hRes = E_FAIL;
	}

	return hRes;
}

HRESULT CMirandaExchange::LogOFF()
{
	try
	{
		if (NULL != m_lpInbox)
		{
			UlRelease(m_lpInbox);
			m_lpInbox = NULL;
		}

		if (NULL != m_lpMDB)
		{
			UlRelease(m_lpMDB);
			m_lpMDB = NULL;
		}

		if ( NULL!= m_lpMAPISession )
		{
			m_lpMAPISession->Logoff( NULL, NULL, NULL );
			m_lpMAPISession->Release();
			m_lpMAPISession = NULL;
		}

		if ( m_nNumberOfHeaders>0 && NULL != m_HeadersKeeper )
		{
			for( UINT i=0; i<m_nNumberOfHeaders; i++ )
			{
				if ( NULL != m_HeadersKeeper[i])
				{
					delete m_HeadersKeeper[i];
					m_HeadersKeeper[i] = NULL;
				}
			}

			m_nNumberOfHeaders =0 ;
		}
	}
	catch (...)
	{

	}

	return S_OK;
}

HRESULT CMirandaExchange::MarkAsRead( LPTSTR szEntryID )
{
    LPMESSAGE lpMessage = NULL ;
	LPBYTE lpData = NULL ;
	ULONG  ulC    = 0    ;

	HexToBin(szEntryID, ulC, lpData);

	CallOpenEntry( m_lpMDB, NULL, NULL, m_lpMAPISession, ulC, (LPENTRYID) lpData, MAPI_BEST_ACCESS, NULL, (LPUNKNOWN*)&lpMessage);
	delete lpData;

	if ( NULL != lpMessage)
	{
		lpMessage->SetReadFlag( 0 );
		lpMessage->SaveChanges(FORCE_SAVE);	
		
		lpMessage->Release();
		lpMessage = NULL;
	}

	return 0;
}


HRESULT CMirandaExchange::CheckInFolder( LPMAPIFOLDER lpFolder )
{
	HRESULT hr = NOERROR;
	CMAPIInterface<LPMAPITABLE> lpTable = NULL;
	LPSRowSet lpRow = NULL;
	LPSPropValue lpRowProp = NULL;
	ULONG i = 0L;
	TCHAR* szSenderName = NULL;
	TCHAR* szSubject = NULL;
	LPSTR szEntryID = NULL;
	
	if ( lpFolder == NULL || !m_bFolderInboxOK )
		return hr;

	SizedSPropTagArray(5,sptaDETAILS) =
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
	
	hr = lpFolder->GetContentsTable( 0, &lpMessageTable );
	if ( HR_FAILED( hr ) )
	{
		return -1;
	}

	LPSRowSet lpRowsR = NULL;
	
	//////////////////////////////////////////////////////////////////////////
	SRestriction srRoot;
	srRoot.rt                       = RES_BITMASK;
	srRoot.res.resBitMask.relBMR    = BMR_EQZ; 
	srRoot.res.resBitMask.ulPropTag = PR_MESSAGE_FLAGS; 
	srRoot.res.resBitMask.ulMask    = MSGFLAG_READ;
	
	SizedSSortOrderSet( 1, sso ) = { 1, 0, 0, { PR_MESSAGE_DELIVERY_TIME, TABLE_SORT_DESCEND } };
	
	hr = HrQueryAllRows( lpMessageTable, (LPSPropTagArray) &sptaDETAILS,&srRoot,(LPSSortOrderSet) & sso, 0L, &lpRowsR );
	
	//////////////////////////////////////////////////////////////////////////
	
	if (HR_FAILED(hr))
		return -1;

	for( i = 0; ( i < lpRowsR->cRows) && ( m_nNumberOfHeaders < MAX_NUMBER_OF_HEADERS ); ++i )
	{
		if ( !(lpRowsR->aRow[ i ].lpProps[ 1 ].Value.l & MSGFLAG_READ) )
		{
			
			if ( !FAILED(lpRowsR->aRow[i].lpProps[2].Value.err) ) 
			{
				szSenderName = lpRowsR->aRow[i].lpProps[2].Value.lpszW;
			}
			
			if ( NULL == szSenderName)
			{
				if ( !FAILED(lpRowsR->aRow[i].lpProps[3].Value.err))
				{
					szSenderName = lpRowsR->aRow[i].lpProps[3].Value.lpszW;
				}
			}
			
			
			if ( !FAILED(lpRowsR->aRow[i].lpProps[4].Value.err) )
			{
				szSubject = lpRowsR->aRow[i].lpProps[4].Value.lpszW;
			}
			
			szEntryID = BinToHex( lpRowsR->aRow[i].lpProps[0].Value.bin.cb, lpRowsR->aRow[i].lpProps[0].Value.bin.lpb );
			m_HeadersKeeper[m_nNumberOfHeaders] = new CKeeper(szSenderName, szSubject, szEntryID );
			m_nNumberOfHeaders++;

			delete[] szEntryID;

			szEntryID    = NULL;
			szSubject    = NULL;
			szSenderName = NULL;
		}
	}
	FreeProws(lpRowsR);

	
	if (m_nNumberOfHeaders < MAX_NUMBER_OF_HEADERS) {
		const enum {IDISPNAME, IENTRYID, ICHILDCOUNT};

		static SizedSPropTagArray ( 3, rgColProps) = 
		{
			3,
			{ 
				PR_DISPLAY_NAME_A, 
				PR_ENTRYID, 
				PR_FOLDER_CHILD_COUNT
			}
		};


		hr = MAPICALL( lpFolder)->GetHierarchyTable( MAPI_DEFERRED_ERRORS, &lpTable);
		if (!FAILED(hr)) {
			hr = HrQueryAllRows( lpTable, (LPSPropTagArray)&rgColProps, NULL, NULL, 0L, &lpRow);
			if (!FAILED(hr)) {
				for(i = 0; i < lpRow->cRows; i ++) {
					lpRowProp = lpRow->aRow[i].lpProps;
					CMAPIInterface<LPMAPIFOLDER> lpSubFolder = NULL;
					hr = CallOpenEntry( m_lpMDB, NULL, NULL, NULL, lpRowProp[IENTRYID].Value.bin.cb, (LPENTRYID)lpRowProp[IENTRYID].Value.bin.lpb, MAPI_BEST_ACCESS, NULL, (LPUNKNOWN*)&lpSubFolder );
					if ( !FAILED(hr) )
					{
						hr = CheckInFolder( lpSubFolder );
						//if (FAILED(hr) ){//Log("failed checkinfolder for %s\n",lpRowProp[IDISPNAME].Value.lpszA );}
					}
				}
				FreeProws(lpRow);
				lpRow = NULL;
			}
		}
	}

	return hr;
}

HRESULT CMirandaExchange::OpenTheMessage( LPTSTR )
{
	//(Default)//// HKEY_CLASSES_ROOT\mailto\shell\open\command
	HKEY hTheKey;
	HRESULT hRes = E_FAIL;

	TCHAR szRegValue[ 512 ];
	DWORD dwLength  = 512 ;
	DWORD dwType = REG_SZ;

	if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
        _T("mailto\\shell\\open\\command"),
        0,
        KEY_ALL_ACCESS | KEY_EXECUTE | KEY_QUERY_VALUE ,
        &hTheKey) == ERROR_SUCCESS
		)
	{	
		LONG lResult = RegQueryValueEx( hTheKey, NULL, NULL,  (LPDWORD)&dwType, (LPBYTE)szRegValue, &dwLength);
		RegCloseKey( hTheKey );

		if ( lResult != ERROR_SUCCESS )
		{
			hRes = E_FAIL;
		}
		else
		{
					
			TCHAR* szTheEnd = _tcsstr( szRegValue,_T(".EXE") );

			if ( NULL != szTheEnd )
			{
				szRegValue[ mir_tstrlen(szRegValue) - mir_tstrlen(szTheEnd) +5 ]  = _T('\0');
				_tcscat( szRegValue, _T(" /recycle") );
				STARTUPINFO         si;
				PROCESS_INFORMATION pi;
				
				memset(&si, 0, sizeof(STARTUPINFO));
				
				si.cb           =   sizeof  (   STARTUPINFO);
				si.dwFlags      =   STARTF_USESHOWWINDOW;
				si.wShowWindow  =   SW_SHOWNORMAL;
				
				if ( CreateProcess   (   NULL,
					szRegValue,
					NULL,
					NULL,
					0,
					NORMAL_PRIORITY_CLASS,
					NULL,
					NULL,
					&si,
					&pi
					))
				{
					hRes = S_OK;
				}
			}
		}
	}
	else
	{
		hRes = E_FAIL;
	}

	return hRes;

}
#endif //NO_EXCHANGE_TEST