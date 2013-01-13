//-----------------------------------------------------------------------------
// File: DXUtil.cpp
//
// Desc: Shortcut macros and functions for using DX objects
//
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#include <stdio.h> 
#include <stdarg.h>
#include "DXUtil.h"

//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertAnsiStringToWide()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       WCHAR string. cchDestChar defaults -1 which means it 
//       assumes strDest is large enough to store strSource
//-----------------------------------------------------------------------------
VOID DXUtil_ConvertAnsiStringToWide( WCHAR* wstrDestination, const CHAR* strSource, 
                                     int cchDestChar )
{
    if( wstrDestination==NULL || strSource==NULL )
        return;

    if( cchDestChar == -1 )
        cchDestChar = strlen(strSource)+1;

    MultiByteToWideChar( CP_ACP, 0, strSource, -1, 
                         wstrDestination, cchDestChar-1 );

    wstrDestination[cchDestChar-1] = 0;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertWideStringToAnsi()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       CHAR string. cchDestChar defaults -1 which means it 
//       assumes strDest is large enough to store strSource
//-----------------------------------------------------------------------------
VOID DXUtil_ConvertWideStringToAnsi( CHAR* strDestination, const WCHAR* wstrSource, 
                                     int cchDestChar )
{
    if( strDestination==NULL || wstrSource==NULL )
        return;

    if( cchDestChar == -1 )
        cchDestChar = wcslen(wstrSource)+1;

    WideCharToMultiByte( CP_ACP, 0, wstrSource, -1, strDestination, 
                         cchDestChar-1, NULL, NULL );

    strDestination[cchDestChar-1] = 0;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertGenericStringToAnsi()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       CHAR string. cchDestChar defaults -1 which means it 
//       assumes strDest is large enough to store strSource
//-----------------------------------------------------------------------------
VOID DXUtil_ConvertGenericStringToAnsi( CHAR* strDestination, const TCHAR* tstrSource, 
                                        int cchDestChar )
{
    if( strDestination==NULL || tstrSource==NULL || cchDestChar == 0 )
        return;

#ifdef _UNICODE
    DXUtil_ConvertWideStringToAnsi( strDestination, tstrSource, cchDestChar );
#else
    if( cchDestChar == -1 )
    {
        strcpy( strDestination, tstrSource );
    }
    else
    {
        strncpy( strDestination, tstrSource, cchDestChar );
        strDestination[cchDestChar-1] = '\0';
    }
#endif
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertGenericStringToWide()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       WCHAR string. cchDestChar defaults -1 which means it 
//       assumes strDest is large enough to store strSource
//-----------------------------------------------------------------------------
VOID DXUtil_ConvertGenericStringToWide( WCHAR* wstrDestination, const TCHAR* tstrSource, 
                                        int cchDestChar )
{
    if( wstrDestination==NULL || tstrSource==NULL || cchDestChar == 0 )
        return;

#ifdef _UNICODE
    if( cchDestChar == -1 )
    {
        wcscpy( wstrDestination, tstrSource );
    }
    else
    {
        wcsncpy( wstrDestination, tstrSource, cchDestChar );
        wstrDestination[cchDestChar-1] = L'\0';
    }
#else
    DXUtil_ConvertAnsiStringToWide( wstrDestination, tstrSource, cchDestChar );
#endif
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertAnsiStringToGeneric()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       TCHAR string. cchDestChar defaults -1 which means it 
//       assumes strDest is large enough to store strSource
//-----------------------------------------------------------------------------
VOID DXUtil_ConvertAnsiStringToGeneric( TCHAR* tstrDestination, const CHAR* strSource, 
                                        int cchDestChar )
{
    if( tstrDestination==NULL || strSource==NULL || cchDestChar == 0 )
        return;
        
#ifdef _UNICODE
    DXUtil_ConvertAnsiStringToWide( tstrDestination, strSource, cchDestChar );
#else
    if( cchDestChar == -1 )
    {
        strcpy( tstrDestination, strSource );
    }
    else
    {
        strncpy( tstrDestination, strSource, cchDestChar );
        tstrDestination[cchDestChar-1] = '\0';
    }
#endif
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertAnsiStringToGeneric()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       TCHAR string. cchDestChar defaults -1 which means it 
//       assumes strDest is large enough to store strSource
//-----------------------------------------------------------------------------
VOID DXUtil_ConvertWideStringToGeneric( TCHAR* tstrDestination, const WCHAR* wstrSource, 
                                        int cchDestChar )
{
    if( tstrDestination==NULL || wstrSource==NULL || cchDestChar == 0 )
        return;

#ifdef _UNICODE
    if( cchDestChar == -1 )
    {
        wcscpy( tstrDestination, wstrSource );
    }
    else
    {
        wcsncpy( tstrDestination, wstrSource, cchDestChar );
        tstrDestination[cchDestChar-1] = L'\0';
    }
#else
    DXUtil_ConvertWideStringToAnsi( tstrDestination, wstrSource, cchDestChar );
#endif
}

//-----------------------------------------------------------------------------
// Name: _DbgOut()
// Desc: Outputs a message to the debug stream
//-----------------------------------------------------------------------------
HRESULT _DbgOut( TCHAR* strFile, DWORD dwLine, HRESULT hr, TCHAR* strMsg )
{
    TCHAR buffer[256];
    wsprintf( buffer, _T("%s(%ld): "), strFile, dwLine );
    OutputDebugString( buffer );
    OutputDebugString( strMsg );

    if( hr )
    {
        wsprintf( buffer, _T("(hr=%08lx)\n"), hr );
        OutputDebugString( buffer );
    }

    OutputDebugString( _T("\n") );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_Trace()
// Desc: Outputs to the debug stream a formatted string with a variable-
//       argument list.
//-----------------------------------------------------------------------------
VOID DXUtil_Trace( TCHAR* strMsg, ... )
{
#if defined(DEBUG) | defined(_DEBUG)
    TCHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    _vsntprintf( strBuffer, 512, strMsg, args );
    va_end(args);

    OutputDebugString( strBuffer );
#else
    UNREFERENCED_PARAMETER(strMsg);
#endif
}
