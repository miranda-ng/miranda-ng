{
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
}

{$IFNDEF M_CRYPTO}
{$DEFINE M_CRYPTO}

type
  PCRYPTO_INTERFACE = ^TCRYPTO_INTERFACE;
  TCRYPTO_INTERFACE = record
	 dwVersion:dword;
    pfnSetKey :procedure(const astr:PAnsiChar); cdecl; // sets the master password
    pfnSetKeyT:procedure(const astr:TCHAR); cdecl;

    pfnPurgeKey:procedure(); cdecl; // purges a key from memory

    pfnEncodeString :procedure(const src:PAnsiChar; buf:PAnsiChar; buf_len:size_t); cdecl;
    pfnEncodeStringT:procedure(const src:TChar    ; buf:PAnsiChar; buf_len:size_t); cdecl;

    pfnDecodeString :procedure(const src:PAnsiChar; buf:PAnsiChar; buf_len:size_t); cdecl;
    pfnDecodeStringT:procedure(const src:PAnsiChar; buf:TChar    ; buf_len:size_t); cdecl;
  end;

  tagCRYPTOAPI = TCRYPTO_INTERFACE;

const
{
  retrieves the crypto interface  v0.10.0.2+
  wParam=0
  lParam=0
  returns CRYPTO_INTERFACE* on success or NULL on failure
}
  MS_CRYPTO_GETINTERFACE:PAnsiChar = 'SRCrypto/GetInterface';

{$ENDIF}
