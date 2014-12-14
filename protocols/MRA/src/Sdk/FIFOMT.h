/*
 * Copyright (c) 2003 Rozhuk Ivan <rozhuk.im@gmail.com>
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



#if !defined(AFX_FIFO_MT__H__INCLUDED_)
#define AFX_FIFO_MT__H__INCLUDED_

#pragma once

#include "Sdk/ListMT.h"
#include "Sdk/InterlockedFunctions.h"

typedef LIST_MT				FIFO_MT, *PFIFO_MT, *LPFIFO_MT;
typedef PCLIST_MT			PCFIFO_MT, LPCFIFO_MT;

typedef LIST_MT_ITEM		FIFO_MT_ITEM, *PFIFO_MT_ITEM, *LPFIFO_MT_ITEM;
typedef PCLIST_MT_ITEM		PCFIFO_MT_ITEM, LPCFIFO_MT_ITEM;

typedef LIST_MT_ITERATOR	FIFO_MT_ITERATOR, *PFIFO_MT_ITERATOR, *LPFIFO_MT_ITERATOR;
typedef PCLIST_MT_ITERATOR	PCFIFO_MT_ITERATOR, LPCFIFO_MT_ITERATOR;


__inline size_t FifoMTItemPush(PCFIFO_MT pcpmtFifoMT,PCFIFO_MT_ITEM pcffmtiFifoItem,LPVOID lpData)
{
	mir_cslock l(pcpmtFifoMT->cs);
	return ListMTItemAdd(pcpmtFifoMT,pcffmtiFifoItem,lpData);
}


__inline DWORD FifoMTItemPop(PCFIFO_MT pcpmtFifoMT,PFIFO_MT_ITEM *ppffmtiFifoItem,LPVOID *plpData)
{
	DWORD dwRetErrorCode;
	PLIST_MT_ITEM plmtiItem;

	mir_cslock l(pcpmtFifoMT->cs);
	if ((dwRetErrorCode=ListMTItemGetFirst(pcpmtFifoMT,&plmtiItem,plpData))==NO_ERROR)
	{
		if (ppffmtiFifoItem) (*ppffmtiFifoItem)=plmtiItem;
		return ListMTItemDelete(pcpmtFifoMT,plmtiItem);
	}

	return dwRetErrorCode;
}

#define FifoMTGetCount(pcpmtFifoMT) ListMTGetCount(pcpmtFifoMT)


#endif // !defined(AFX_FIFO_MT__H__INCLUDED_)
