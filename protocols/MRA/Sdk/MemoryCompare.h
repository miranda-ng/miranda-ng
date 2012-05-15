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



#if !defined(AFX_MEMORYCOMPARE__H__INCLUDED_)
#define AFX_MEMORYCOMPARE__H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//  If the string pointed to by lpString1 is less than the string pointed 
// to by lpString2, the return value is negative. 
//  If the string pointed to by lpString1 is greater than the string pointed 
// to by lpString2, the return value is positive. 
//  If the strings are equal, the return value is zero. 
//
// lpString1<lpString2 >> ret=1=CSTR_LESS_THAN
// lpString1=lpString2 >> ret=2=CSTR_EQUAL
// lpString1>lpString2 >> ret=3=CSTR_GREATER_THAN

#define CMEM_EQUAL			0
#define CMEM_GREATER_THAN	1
#define CMEM_LESS_THAN		2


__inline unsigned int MemoryCompare(const void *pBuff1,size_t dwBuff1Size,const void *pBuff2,size_t dwBuff2Size)
{
	unsigned int uiRet;

	if (dwBuff1Size==dwBuff2Size)
	{
		if (pBuff1==pBuff2)
		{
			uiRet=CMEM_EQUAL;
		}else{
			if (pBuff1 && pBuff2)
			{
				int iRet;
				
				iRet=memcmp(pBuff1,pBuff2,dwBuff1Size);
				if (iRet==0)
				{
					uiRet=CMEM_EQUAL;
				}else{
					if (iRet<0)
					{
						uiRet=CMEM_GREATER_THAN;
					}else{
						uiRet=CMEM_LESS_THAN;
					}
				}
			}else{
				if (pBuff1)
				{//pBuff2==NULL
					uiRet=CMEM_GREATER_THAN;
				}else{//pBuff1==NULL
					uiRet=CMEM_LESS_THAN;
				}
			}
		}
	}else{
		if (dwBuff1Size<dwBuff2Size)
		{
			uiRet=CMEM_LESS_THAN;
		}else{
			uiRet=CMEM_GREATER_THAN;
		}
	}
return(uiRet);
}



#endif // !defined(AFX_MEMORYCOMPARE__H__INCLUDED_)
