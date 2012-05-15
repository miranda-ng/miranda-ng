/*
 * Copyright (c) 2010 Rozhuk Ivan <rozhuk.im@gmail.com>
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


#if !defined(AFX_MEMORY_REPLACE__H__INCLUDED_)
#define AFX_MEMORY_REPLACE__H__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef DebugBreak
#define DebugBreak()
#endif


#ifdef MEMALLOC
#define MEMORY_REPLACE_MEMALLOC(Size)	MEMALLOC(Size)
#define MEMORY_REPLACE_MEMFREE(Mem)		MEMFREE(Mem)
#else
#define MEMORY_REPLACE_MEMALLOC(Size)	malloc(((Size)+sizeof(void*)))
#define MEMORY_REPLACE_MEMFREE(Mem)		if ((Mem)) {free((void*)(Mem));(Mem)=NULL;}
#endif


//  0	- all ok
// -1	- bad params
// -2	- small dst buff
// -3	- memory allocation failed

__inline int MemoryReplaceEx(void *pSrcBuff,size_t dwSrcBuffSize,size_t dwReplaceItemsCount,void **ppInReplaceItems,size_t *pdwInReplaceItemsCounts,void **ppOutReplaceItems,size_t *pdwOutReplaceItemsCounts,void *pDstBuff,size_t dwDstBuffSize,size_t *pdwDstBuffSize,size_t *pdwReplacesCount)
{
	int iRet=-1;

	if (pSrcBuff && dwSrcBuffSize && (dwReplaceItemsCount==0 || (dwReplaceItemsCount && ppInReplaceItems && pdwInReplaceItemsCounts && ppOutReplaceItems && pdwOutReplaceItemsCounts)) && pDstBuff && dwDstBuffSize)
	{
		if (dwReplaceItemsCount==0)
		{// no replace, copy mem
			if (dwDstBuffSize>=dwSrcBuffSize)
			{
				memmove(pDstBuff,pSrcBuff,dwSrcBuffSize);
				if (pdwDstBuffSize) (*pdwDstBuffSize)=dwSrcBuffSize;
				if (pdwReplacesCount) (*pdwReplacesCount)=0;
				iRet=0;
			}else{// small dst buff
				iRet=-2;
			}
		}else{
			unsigned char **ppFounded;

			ppFounded=(unsigned char**)MEMORY_REPLACE_MEMALLOC((sizeof(unsigned char*)*dwReplaceItemsCount));
			if (ppFounded)
			{
				unsigned char *pDstBuffCur,*pSrcBuffCur,*pSrcBuffCurPrev,*pDstBuffMax;
				size_t i,dwFirstFoundedIndex,dwFoundedCount,dwMemPartToCopy,dwReplacesCount;

				pSrcBuffCurPrev=(unsigned char*)pSrcBuff;
				pDstBuffCur=(unsigned char*)pDstBuff;
				pDstBuffMax=(((unsigned char*)pDstBuff)+dwDstBuffSize);
				dwFirstFoundedIndex=0;
				dwFoundedCount=0;
				dwReplacesCount=0;

				for(i=0;i<dwReplaceItemsCount;i++)
				{// loking for in first time
					ppFounded[i]=(unsigned char*)MemoryFind((pSrcBuffCurPrev-(unsigned char*)pSrcBuff),pSrcBuff,dwSrcBuffSize,ppInReplaceItems[i],pdwInReplaceItemsCounts[i]);
					if (ppFounded[i]) dwFoundedCount++;
				}

				while(dwFoundedCount)
				{
					for(i=0;i<dwReplaceItemsCount;i++)
					{// looking for first to replace
						if (ppFounded[i] && (ppFounded[i]<ppFounded[dwFirstFoundedIndex] || ppFounded[dwFirstFoundedIndex]==NULL)) dwFirstFoundedIndex=i;
					}

					if (ppFounded[dwFirstFoundedIndex])
					{// in founded
						dwMemPartToCopy=(ppFounded[dwFirstFoundedIndex]-pSrcBuffCurPrev);
						if (pDstBuffMax>(pDstBuffCur+(dwMemPartToCopy+pdwInReplaceItemsCounts[dwFirstFoundedIndex])))
						{
							dwReplacesCount++;
							memmove(pDstBuffCur,pSrcBuffCurPrev,dwMemPartToCopy);pDstBuffCur+=dwMemPartToCopy;
							memmove(pDstBuffCur,ppOutReplaceItems[dwFirstFoundedIndex],pdwOutReplaceItemsCounts[dwFirstFoundedIndex]);pDstBuffCur+=pdwOutReplaceItemsCounts[dwFirstFoundedIndex];
							pSrcBuffCurPrev=(ppFounded[dwFirstFoundedIndex]+pdwInReplaceItemsCounts[dwFirstFoundedIndex]);

							for(i=0;i<dwReplaceItemsCount;i++)
							{// loking for in next time // update founded records
								if (ppFounded[i] && ppFounded[i]<pSrcBuffCurPrev)
								{
									ppFounded[i]=(unsigned char*)MemoryFind((pSrcBuffCurPrev-(unsigned char*)pSrcBuff),pSrcBuff,dwSrcBuffSize,ppInReplaceItems[i],pdwInReplaceItemsCounts[i]);
									if (ppFounded[i]==NULL) dwFoundedCount--;
								}
							}
						}else{// ERROR_BUFFER_OVERFLOW
							iRet=-2;
							DebugBreak();
							break;
						}
					}else{// сюда по идее никогда не попадём, на всякий случай.
						DebugBreak();
						break;
					}
				}
				pSrcBuffCur=(((unsigned char*)pSrcBuff)+dwSrcBuffSize);
				memmove(pDstBuffCur,pSrcBuffCurPrev,(pSrcBuffCur-pSrcBuffCurPrev));
				pDstBuffCur+=(pSrcBuffCur-pSrcBuffCurPrev);
				(*((unsigned short*)pDstBuffCur))=0;

				MEMORY_REPLACE_MEMFREE(ppFounded);

				if (pdwDstBuffSize) (*pdwDstBuffSize)=(pDstBuffCur-((unsigned char*)pDstBuff));
				if (pdwReplacesCount) (*pdwReplacesCount)=dwReplacesCount;
				iRet=0;
			}else{
				iRet=-3;
			}
		}
	}
return(iRet);
}


#endif // !defined(AFX_MEMORY_REPLACE__H__INCLUDED_)
