/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#pragma once

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <stdint.h>
#endif

#include <wchar.h>

#pragma pack(push, 1)

#ifdef __INTERFACE_ONLY__
#define __INTERFACE_VIRTUAL__
#else
#define __INTERFACE_VIRTUAL__ virtual
#endif

class CCipher
{
public:
	typedef struct TCipherInterface
	{
		CCipher * self;
		uint32_t Size;
		void           (__cdecl CCipher::*Destroy)();

		const wchar_t* (__cdecl CCipher::*Name)();
		const wchar_t* (__cdecl CCipher::*Description)();
		const uint32_t (__cdecl CCipher::*BlockSizeBytes)();
		const bool     (__cdecl CCipher::*IsStreamCipher)();

		void           (__cdecl CCipher::*SetKey)(void* Key, uint32_t KeyLength);
		void           (__cdecl CCipher::*Encrypt)(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);
		void           (__cdecl CCipher::*Decrypt)(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte);

	} TCipherInterface;
	TCipherInterface * m_Interface;

#ifndef __INTERFACE_ONLY__
	virtual void __cdecl Destroy()
	{
		delete this;
	}

	CCipher()
	{
		m_Interface = new TCipherInterface;
		m_Interface->Size = sizeof(TCipherInterface);
		m_Interface->self = this;
		m_Interface->Destroy        = &CCipher::Destroy;
		m_Interface->Name           = &CCipher::Name;
		m_Interface->Description    = &CCipher::Description;
		m_Interface->BlockSizeBytes = &CCipher::BlockSizeBytes;
		m_Interface->IsStreamCipher = &CCipher::IsStreamCipher;
		m_Interface->SetKey         = &CCipher::SetKey;
		m_Interface->Encrypt        = &CCipher::Encrypt;
		m_Interface->Decrypt        = &CCipher::Decrypt;
	};
#endif

#ifdef __INTERFACE_ONLY__
	CCipher(TCipherInterface * Interface)
	{
		m_Interface = Interface;
	};
#endif

	__INTERFACE_VIRTUAL__ ~CCipher()
#ifdef __INTERFACE_ONLY__
	{
		(m_Interface->self->*(m_Interface->Destroy))();
	}
#else
	{	}
#endif
	;

	__INTERFACE_VIRTUAL__ const wchar_t* __cdecl Name()
#ifdef __INTERFACE_ONLY__
	{
		return (m_Interface->self->*(m_Interface->Name))();
	}
#else
	= 0
#endif
		;

	__INTERFACE_VIRTUAL__ const wchar_t* __cdecl Description()
#ifdef __INTERFACE_ONLY__
	{
		return (m_Interface->self->*(m_Interface->Description))();
	}
#else
	= 0
#endif
		;

	__INTERFACE_VIRTUAL__ const uint32_t __cdecl BlockSizeBytes()
#ifdef __INTERFACE_ONLY__
	{
		return (m_Interface->self->*(m_Interface->BlockSizeBytes))();
	}
#else
		= 0
#endif
		;

	__INTERFACE_VIRTUAL__ const bool __cdecl IsStreamCipher()
#ifdef __INTERFACE_ONLY__
	{
		return (m_Interface->self->*(m_Interface->IsStreamCipher))();
	}
#else
		= 0
#endif
		;

	__INTERFACE_VIRTUAL__ void __cdecl SetKey(void* Key, uint32_t KeyLength)
#ifdef __INTERFACE_ONLY__
	{
		return (m_Interface->self->*(m_Interface->SetKey))(Key, KeyLength);
	}
#else
		= 0
#endif
		;
	__INTERFACE_VIRTUAL__ void __cdecl Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
#ifdef __INTERFACE_ONLY__
	{
		return (m_Interface->self->*(m_Interface->Encrypt))(Data, Size, Nonce, StartByte);
	}
#else
		= 0
#endif
		;
	__INTERFACE_VIRTUAL__ void __cdecl Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
#ifdef __INTERFACE_ONLY__
	{
		return (m_Interface->self->*(m_Interface->Decrypt))(Data, Size, Nonce, StartByte);
	}
#else
		= 0
#endif
		;

};

typedef struct TCipherInfo
{
	uint32_t cbSize;
	const uint32_t ID;
	const wchar_t* Name;
	const wchar_t* Description;
	CCipher::TCipherInterface * (__cdecl *Create)();
} TCipherInfo;

#pragma pack(pop)
