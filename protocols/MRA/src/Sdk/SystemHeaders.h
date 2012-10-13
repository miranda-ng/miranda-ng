/*
 * Copyright (c) 2008 Rozhuk Ivan <rozhuk.im@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */



#if !defined(AFX_SYSTEMHEADERS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_SYSTEMHEADERS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define WINVER       0x0502
#define _WIN32_WINNT 0x0502
#include <windows.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
#include <PrSht.h>
#include <Commdlg.h>
//#include <strsafe.h>


#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")
// RunTmChk.lib Imagehlp.lib kernel32.lib Crypt32.lib shlwapi.lib ws2_32.lib mswsock.lib





#endif // !defined(AFX_SYSTEMHEADERS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
