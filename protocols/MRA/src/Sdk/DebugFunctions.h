/*
 * Copyright (c) 2004 Rozhuk Ivan <rozhuk.im@gmail.com>
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



#if !defined(AFX_DEBUG_FUNCTIONS__H__INCLUDED_)
#define AFX_DEBUG_FUNCTIONS__H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef szCRLF
#define szCRLF						TEXT("\r\n")
#endif
//////////////////////////////////////////////////////////////////////////
////////////////////////////DebugPrint////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// DebugBuildPrint(Helo world);
// Отображение текста в Debug окне во время отладки
#ifdef _DEBUG
	#define DebugPrintA(szText) OutputDebugStringA((szText))
	#define DebugPrintW(szText) OutputDebugStringW((szText))
	#define DebugPrintCRLFA(szText) OutputDebugStringA((szText));OutputDebugStringA("\r\n")
	#define DebugPrintCRLFW(szText) OutputDebugStringW((szText));OutputDebugStringW(L"\r\n")
#else
	#define DebugPrintA(szText)
	#define DebugPrintW(szText)
	#define DebugPrintCRLFA(szText)
	#define DebugPrintCRLFW(szText)
#endif //_DEBUG


#ifdef UNICODE
	#define DebugPrint DebugPrintW
	#define DebugPrintCRLF DebugPrintCRLFW
#else
	#define DebugPrint DebugPrintA
	#define DebugPrintCRLF DebugPrintCRLFA
#endif
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
////////////////////////////DebugBuildPrint///////////////////////////////
//////////////////////////////////////////////////////////////////////////
// #pragma DebugBuildPrint(Helo world);
// Отображение сообщений в Build окне во время компиляции
#ifdef _DEBUG
	#pragma warning(disable:4081)
	#define chSTR2(x)	#x
	#define chSTR(x)	chSTR2(x)
	#define DebugBuildPrint(szText) message(__FILE__ "(" chSTR(__LINE__) "): " #szText)
	#pragma warning(default:4081)
#else
	#define DebugBuildPrint(szText)
#endif //_DEBUG
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
////////////////////////////DebugBufferFill///////////////////////////////
//////////////////////////////////////////////////////////////////////////
// DebugBufferFill(szString,sizeof(szString));
// Полностью заполняет переданный буффер символом "A", применяется
// для выходного буффера на входе в функцию.
#ifdef _DEBUG
	#define DebugBufferFill(lpBuffer,dwSize) memset(lpBuffer,'A',dwSize)
#else
	#define DebugBufferFill(lpBuffer,dwSize)
#endif //_DEBUG
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
////////////////////////////DebugBreak////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// DebugBreak();
// Точка останова, более удобная альтернатива API
#if defined(_DEBUG)
	#if defined(_X86_)
		#define DebugBreak() _asm{int 3}
	#else
		#include <intrin.h>
		#define DebugBreak() __debugbreak()
	#endif
#else
	#define DebugBreak()
#endif //_DEBUG
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
////////////////////////////DebugBreakIf//////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// DebugBreakIf();
// Точка останова, более удобная альтернатива API, срабатывает при условии
#if defined(_DEBUG)
	#define DebugBreakIf(a) if ((a)) DebugBreak();
#else
	#define DebugBreakIf(a)
#endif //_DEBUG
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




#endif // !defined(AFX_DEBUG_FUNCTIONS__H__INCLUDED_)
