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
	created:	11:4:2006   17:27
	filename: 	MirandaExchange.h
	file base:	MirandaExchange
	file ext:	h
	author:		Attila Vajda
	
	purpose:	Miranda Exchange Plugin
*********************************************************************/

//define if you want to test only the miranda part, and not the exchange one.
//#define NO_EXCHANGE_TEST

#ifndef NO_EXCHANGE_TEST

#if !defined(MIRANDAEXCHANGE_H__INCLUDED_)
#define MIRANDAEXCHANGE_H__INCLUDED_

#undef _DEBUG
#undef DEBUG

#include <mapix.h>
#include <mapival.h>

#define _DEBUG
#define DEBUG


#pragma once

#define MAX_NUMBER_OF_HEADERS 512

#if !defined(__cplusplus) || defined(CINTERFACE)
#define MAPICALL(x)     (x)->lpVtbl
#else
#define MAPICALL(x)     (x)
#endif

#define FREEPROWS(x)    \
{                       \
    FreeProws((x));     \
	(x) = NULL;         \
}


#define pidFolderMin 0x6638 
#define pidProfileMin 0x6600 
#define PR_PROFILE_UNRESOLVED_NAME PROP_TAG( PT_STRING8, pidProfileMin+0x07) 
#define PR_PROFILE_UNRESOLVED_SERVER PROP_TAG( PT_STRING8, pidProfileMin+0x08)
#define PR_FOLDER_CHILD_COUNT PROP_TAG( PT_LONG, pidFolderMin) 
 
// -----------------------------------------------------------------------------
// Helpers.h: Template Class that wraps MAPI buffers or interfaces so that you
//            don't have to concern yourself with freeing or releasing them.
//
// Copyright (C) Microsoft Corp. 1986 - 2000.  All Rights Reserved.
// -----------------------------------------------------------------------------

#ifndef _HELPERS_H_
#define _HELPERS_H_

// -----------------------------------------------------------------------------
// Use this class to wrap a pointer to a mapi buffer.  It will automaticly get 
// freed when it goes out of scope.  Use the instance of this just as you would
// its base type.  Example:
//     CMAPIBuffer< LPENTRYID>  lpeid;
//     HRESULT hr = HrMAPIFindDefaultMsgStore( m_pMAPISession, &cbeid, &lpeid);
//     hr = m_pMAPISession->OpenMsgStore( 0, cbeid, lpeid, ...
//
// The intended use of this class is for declaring mapi buffer pointers on the
// stack or inside of another class or structure.  There is NO value in creating
// an instance of this class on the heap with new().  It's sole purpose in life
// is to keep you from having to remember to free the buffer.
//
// If you need to reuse the pointer be sure to call MAPIFREEBUFFER before 
// reusing it.  Example:
//     CMAPIBuffer< LPENTRYID>  lpeid;
//     HRESULT hr = HrMAPIFindDefaultMsgStore( m_pMAPISession, &cbeid, &lpeid);
//     ... // Do some other work... 
//     MAPIFREEBUFFER( lpeid);
//     hr = HrGetSomeOtherEID( &cbeid, &lpeid);
// -----------------------------------------------------------------------------

template< class TYPE>
class CMAPIBuffer
{
public:
    CMAPIBuffer( TYPE ptr = NULL)   { m_ptr = ptr;}
    ~CMAPIBuffer()                  { MAPIFREEBUFFER( m_ptr);}

    // Returns a pointer to the TYPE by just specifying the object.
    operator TYPE() {return( m_ptr);}

    // Returns the address of the object correct for the base type.
    TYPE* operator &() {return( &m_ptr);}

    void operator =(LPVOID lpv)  {m_ptr = (TYPE) lpv;}

protected:
    TYPE m_ptr;
};

// -----------------------------------------------------------------------------
// Use this class to wrap a pointer to a mapi interface.  It is nearly identical
// to the above class except that it releases the interface pointer when the 
// instance of the object goes out of scope.
//
// The intended use of this class is for declaring mapi interface pointers on the
// stack or inside of another class or structure.  There is NO value in creating
// an instance of this class on the heap with new().  It's sole purpose in life
// is to keep you from having to remember to release the interface.
// -----------------------------------------------------------------------------

template< class TYPE>
class CMAPIInterface
{
public:
    CMAPIInterface( TYPE ptr = NULL)   { m_ptr = ptr;}
    ~CMAPIInterface()                  { UlRelease( m_ptr);}

    // Returns a pointer to the TYPE by just specifying the object.
    operator TYPE()         {return( m_ptr);}

    // Returns the address of the object correct for the base type.
    TYPE* operator &() {return( &m_ptr);}

    // Returns a pointer to the TYPE for -> operations.
    TYPE operator ->()      {return( m_ptr);}

    void operator =(LPVOID lpv)  {m_ptr = (TYPE) lpv;}

protected:
    TYPE m_ptr;
};

// $--CMAPIIsInitialized--------------------------------------------------------
// Initialize MAPI using MAPIInitialize().  If it is successful then create an
// instance of this object which will uninitialize MAPI at destruction.
// -----------------------------------------------------------------------------

class CMAPIIsInitialized
{
public:
    ~CMAPIIsInitialized()     {MAPIUninitialize();}
};

// $--CMAPISession--------------------------------------------------------------
// Works just like CMAPIInterface but it also Logs off at destruction.
// -----------------------------------------------------------------------------

class CMAPISession : public CMAPIInterface< LPMAPISESSION>
{
public:
    // DESTRUCTOR logs off of the MAPI session and releases the session handle.
    ~CMAPISession() 
    {
        if ( m_ptr)
            m_ptr->Logoff( 0L, 0L, 0L);
    }
};

// -----------------------------------------------------------------------------

#endif // _HELPERS_H_

class CKeeper
{
public:
	CKeeper  ( LPTSTR szSender, LPTSTR szSubject, LPSTR szEntryID );
	~CKeeper ();

public:
	UINT m_nSizeSender ;
	UINT m_nSizeSubject;
	UINT m_nSizeEntryID;

	LPTSTR m_szSender  ;
	LPTSTR m_szSubject ;
	LPSTR m_szEntryID ;
};


class CMirandaExchange  
{
public:

                  CMirandaExchange();
	              //CMirandaExchange   ( LPCTSTR szUsername, LPCTSTR szPassword, LPCTSTR szExchangeServer );
	virtual       ~CMirandaExchange  ();

	HRESULT       CheckForNewMails   ( int &nNewMails );
	HRESULT       CreateProfile      ( LPTSTR szProcessID );
	HRESULT       InitializeAndLogin ( LPCTSTR szUsername, LPCTSTR szPassword, LPCTSTR szExchangeServer );
	HRESULT       CheckInFolder      ( LPMAPIFOLDER lpFolder );
	HRESULT       MarkAsRead         ( LPTSTR szEntryID );
	HRESULT       LogOFF             ();
	HRESULT       OpenTheMessage        ( LPTSTR szEntryID );	
	HRESULT       isMapiSessionOK    ( LPMAPISESSION lpSession );
	CKeeper*       m_HeadersKeeper[MAX_NUMBER_OF_HEADERS+1];

private:
	LPTSTR        m_szUsername;
	LPTSTR        m_szPassword;
	LPTSTR        m_szExchangeServer;
	LPMAPISESSION m_lpMAPISession;
	LPMAPIFOLDER  m_lpInbox;
	LPMDB         m_lpMDB;
	bool          m_bLoginOK;
	bool          m_bFolderInboxOK;

	bool          m_bNoInitAgain;
	UINT          m_nNumberOfHeaders;
	mir_cs m_myCritical;

};

#endif // !defined(MIRANDAEXCHANGE_H__INCLUDED_)
#endif //NO_EXCHANGE_TEST
