{
This file is part of HTTPServer a Miranda IM plugin
Copyright (C)2002 Kennet Nielsen 

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
}

{$IFNDEF M_HTTP_SERVER}
{$DEFINE M_HTTP_SERVER}

const
  OPT_SEND_LINK = 1;

type
  PSTFileShareInfo = ^TSTFileShareInfo;
  TSTFileShareInfo = record
    lStructSize  :dword; // Set to sizeof(STFileShareInfo)
    pszSrvPath   :TChar; // Server path
    dwMaxSrvPath :dword; // Buffer allocated for Server path only used when information
                         // is requested from HTTP server.
    pszRealPath  :TChar; // Real path can be relative or complete
    dwMaxRealPath:dword; // Buffer allocated for Real path only used when information is
                         // requested from HTTP server.
    dwAllowedIP  :dword; // The IP address which is allowed to access this share
    dwAllowedMask:dword; // A mask which is applied to IP address to allow other IP addresses
    nMaxDownloads:int;   // The maximum number of download which can be made on this share.
    dwOptions    :dword; // Use OPT_SEND_LINK to open a message window with the link to file
  end;

// dwMaxSrvPath Specifies the size, in chars, of the buffer pointed to by pszSrvPath. 
// The buffer must be large enough to store the path and file name string, 
// including the terminating null character. 

const
/////////////////////////////////////////////
///   Service MS_HTTP_ADD_CHANGE_REMOVE   ///
/////////////////////////////////////////////
//
// wParam = (WPARAM)0
// lParam = (LPARAM)LPSTFileShareInfo;
// Server path is the key when working with FileShareInfo.
// Two files can not be shared with the same "Server path" in the HTTP server.
// If the server path does not exists it will be added.
// If it does exists the action depends on what real path is.
// If real path is empty the entity will be removed else it 
// will just be updated with the new settings.
//
// returns 0 on success, nonzero on failure
  MS_HTTP_ADD_CHANGE_REMOVE:PAnsiChar = 'HTTPServer/AddChangeRemove';

/////////////////////////////////////////////
//////    Service MS_HTTP_GET_SHARE    //////
/////////////////////////////////////////////
//
// wParam = (WPARAM)0;
// lParam = (LPARAM)LPSTFileShareInfo;
// Returns the information for a share
// Server path must be set the the share you wish information for.
//
// returns 0 on success, nonzero on failure

  MS_HTTP_GET_SHARE:PAnsiChar = 'HTTPServer/GetShare';

/////////////////////////////////////////////
///  Service MS_HTTP_ACCEPT_CONNECTIONS   ///
/////////////////////////////////////////////
//
// wParam = (WPARAM)boolean(true/false);
// lParam = (LPARAM)0;
// Toggles the HTTP server state if wParam is FALSE
// Force enable HTTP server if wParam is TRUE
//
// returns 0 on success, nonzero on failure

  MS_HTTP_ACCEPT_CONNECTIONS:PAnsiChar = 'HTTPServer/AcceptConnections';

/////////////////////////////////////////////
////   Service MS_HTTP_GET_ALL_SHARES   /////
/////////////////////////////////////////////
//
// wParam = (WPARAM)0;
// lParam = (LPARAM)&LPSTFileShareInfo;
// Returns an array of all currently shared files in the HTTP Server
// LPSTFileShareInfo points to the first share.
// You must free the memory returned by using the miranda MS_SYSTEM_GET_MMI
// and calling MM_INTERFACE->free( LPSTFileShareInfo )
//
// returns the count of shares in the buffer pointed to by LPSTFileShareInfo

  MS_HTTP_GET_ALL_SHARES:PAnsiChar = 'HTTPServer/GetAllShares';

/////////////////////////////////////////////
////   Service MS_HTTP_GET_LINK         /////
/////////////////////////////////////////////
//
// wParam = (char*)pszSrvPath;
// lParam = 0
// Return URL Link on success, 0 on failure
// Return pointer must be mir_free by caller
//
// Return the URL link to the pszSrvPath

  MS_HTTP_GET_LINK:PAnsiChar = 'HTTPServer/GetLink';

{$ENDIF}
