/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef M_CRYPTO_H__
#define M_CRYPTO_H__ 1

typedef struct tagCRYPTOAPI
{
	DWORD	dwVersion;

	void	(__cdecl *pfnSetKey)( const char* );  // sets the master password
	void	(__cdecl *pfnSetKeyT)( const TCHAR* );

	void	(__cdecl *pfnPurgeKey)( void );   // purges a key from memory

	void  (__cdecl *pfnEncodeString)( const char* src, char* buf, size_t buf_len);
	void  (__cdecl *pfnEncodeStringT)( const TCHAR* src, char* buf, size_t buf_len);

	void  (__cdecl *pfnDecodeString)( const char* src, char* buf, size_t buf_len);
	void  (__cdecl *pfnDecodeStringT)( const char* src, TCHAR* buf, size_t buf_len);
}
	CRYPTO_INTERFACE;

//retrieves the crypto interface  v0.10.0.2+
//wParam=0
//lParam=0
//returns CRYPTO_INTERFACE* on success or NULL on failure
#define MS_CRYPTO_GETINTERFACE "SRCrypto/GetInterface"

#endif // M_CRYPTO_H__
