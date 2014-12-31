{
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
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
  PMICryptoEngine = ^TMICryptoEngine;
  TMICryptoEngine = record
    dwVersion:dword;

    destroy:procedure(); stdcall;

    // get/set the instance key
    getKeyLength:function():size_t; stdcall;
    getKey      :function(pKey:PByte; cbKeyLen:size_t):ByteBool; stdcall;
    setKey      :function(const pKey:PByte; cbKeyLen:size_t):ByteBool; stdcall;

    generateKey:function():ByteBool; stdcall; // creates a new key inside
    purgeKey   :procedure(); stdcall;     // purges a key from memory

    // sets the master password (in utf-8)
    checkPassword:function (const pszPassword:PAnsiChar):ByteBool; stdcall;
    setPassword  :procedure(const pszPassword:PAnsiChar); stdcall;

    // result must be freed using mir_free or assigned to mir_ptr<BYTE>
    encodeString:function(const src:PAnsiChar; var cbResultLen:size_t):PByte; stdcall;
    encodeBuffer:function(const src:pointer; cbLen:size_t; var cbResultLen:size_t):PByte; stdcall;

    // result must be freed using mir_free or assigned to ptrA/ptrT
    decodeString:function(const pBuf:PByte; bufLen:size_t; var cbResultLen:size_t):PAnsiChar; stdcall;
    decodeBuffer:function(const pBuf:PByte; bufLen:size_t; var cbResultLen:size_t):pointer; stdcall;
  end;

/////////////////////////////////////////////////////////////////////////////////////////
// registers a crypto provider v0.94+
// wParam = (int)hLangpack
// lParam = PCRYPTO_PROVIDER
// returns HANDLE on success or NULL on failure

type
  pfnCryptoProviderFactory = function():PMICryptoEngine; cdecl;

const
  CPF_UNICODE = 1;

type
  PCRYPTO_PROVIDER = ^TCRYPTO_PROVIDER;
  TCRYPTO_PROVIDER = record
    dwSize  :dword;
    dwFlags :dword;     // one of CPF_* constants
    pszName :PAnsiChar; // unique id
    szDescr :TChar;     // description, auto translated by core
    pFactory:pfnCryptoProviderFactory;
  end;
  tagCRYPTOPROVIDER = TCRYPTO_PROVIDER;

const
  MS_CRYPTO_REGISTER_ENGINE:PAnsiChar = 'Crypto/RegisterEngine';

///////////////////////////////////////////////////////////////////////////////////////////////
// retrieves list of all available crypto providers
// wParam = (WPARAM)(int*)piNumProviders
// lParam = (CRYPTO_PROVIDER***) pointer to an array of PCRYPTO_PROVIDER, like enumaccounts
// always returns 0

  MS_CRYPTO_ENUM_PROVIDERS:PAnsiChar = 'Crypto/EnumProviders';

///////////////////////////////////////////////////////////////////////////////////////////////
// retrieves a crypto provider by name
// wParam = 0 (unused)
// lParam = (const char*)pszProviderName = CRYPTO_PROVIDER::pszName
// returns 0 on fail or CRYPTO_PROVIDER* on success

  MS_CRYPTO_GET_PROVIDER:PAnsiChar = 'Crypto/GetProvider';

{$ENDIF}
