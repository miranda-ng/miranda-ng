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


/*__inline DWORD MemoryCompare(LPCVOID lpcSource1,SIZE_T dwSource1Size,LPCVOID lpcSource2,SIZE_T dwSource2Size)
{
	DWORD dwRet;

	__asm
	{
		mov			ecx,dwSource1Size	//; ecx = Source string 1 Size
		cmp			ecx,dwSource2Size	//; сверяем длинны участков памяти
		jg			short greater_than
		jl			short less_than
		test		ecx,ecx
		jz			short equal			//; NULL=NULL

		mov			esi,lpcSource1		//; edi = Source string 1
		mov			edi,lpcSource2		//; esi = Source string 2
		cmp			edi,esi				//; сверяем указатели на участки памяти
		je			short equal			//; это один и тотже участок, они естественно равны

		test		esi,esi				//; lpcSource1=NULL, lpcSource1<lpcSource2
		jz			short less_than		//; CSTR_LESS_THAN

		test		edi,edi				//; lpcSource2=NULL, lpcSource1>lpcSource2
		jz			short greater_than	//; CSTR_GREATER_THAN

		//cld								//; сканируя в прямом направлении

		repe		cmpsb				//;	цикл сравнения.
	cmp_loop:
		//sub			ecx,4
		//jz			short equal

		//inc			esi
		//inc			edi
		//mov			al,byte ptr [esi]
		//cmpsd
		//cmp			al,byte ptr [edi]
		//je			short cmp_loop
		jg			short greater_than
		jl			short less_than

	equal:								//; если мы попали сюда, значит, они
		mov			dwRet,CSTR_EQUAL	//; совпадают (match)
		jmp			end_func
	less_than:							//; не совпадают
		mov			dwRet,CSTR_LESS_THAN
		jmp			end_func
	greater_than:
		mov			dwRet,CSTR_GREATER_THAN

	end_func:
	}
return(dwRet);
}//*/

__inline DWORD MemoryCompare(LPCVOID lpcSource1,SIZE_T dwSource1Size,LPCVOID lpcSource2,SIZE_T dwSource2Size)
{
	DWORD dwRet;

	if (dwSource1Size==dwSource2Size)
	{
		if (lpcSource1==lpcSource2)
		{
			dwRet=CSTR_EQUAL;
		}else{
			if (lpcSource1 && lpcSource2)
			{
#ifdef _INC_MEMORY
				dwRet=(2+memcmp(lpcSource1,lpcSource2,dwSource1Size));
#else
				SIZE_T dwDiffPosition;

				//dwDiffPosition=RtlCompareMemory(lpcSource1,lpcSource2,dwSource1Size);
				for(dwDiffPosition=0; (dwDiffPosition<dwSource1Size) && (((const BYTE*)lpcSource1)[dwDiffPosition]==((const BYTE*)lpcSource2)[dwDiffPosition]); dwDiffPosition++);
				if (dwDiffPosition==dwSource1Size)
				{
					dwRet=CSTR_EQUAL;
				}else{
					if ((*((BYTE*)(((SIZE_T)lpcSource1)+dwDiffPosition)))>(*((BYTE*)(((SIZE_T)lpcSource2)+dwDiffPosition))))
					{
						dwRet=CSTR_GREATER_THAN;
					}else{
						dwRet=CSTR_LESS_THAN;
					}
				}
#endif
			}else{
				if (lpcSource1)
				{//lpcSource2==NULL
					dwRet=CSTR_GREATER_THAN;
				}else{//lpcSource1==NULL
					dwRet=CSTR_LESS_THAN;
				}
			}
		}
	}else{
		if (dwSource1Size<dwSource2Size)
		{
			dwRet=CSTR_LESS_THAN;
		}else{
			dwRet=CSTR_GREATER_THAN;
		}
	}
return(dwRet);
}//*/


/*
__inline DWORD MemoryCompareEx(LPCVOID lpcSource1,SIZE_T dwSource1Size,LPCVOID lpcSource2,SIZE_T dwSource2Size,SIZE_T *pdwDiffPosition)
{
	DWORD dwRet;

	if (dwSource1Size==dwSource2Size)
	{
		if (lpcSource1==lpcSource2)
		{
			dwRet=CSTR_EQUAL;
		}else{
			if (lpcSource1 && lpcSource2)
			{
				SIZE_T dwDiffPosition;

				dwDiffPosition=RtlCompareMemory(lpcSource1,lpcSource2,dwSource1Size);
				if (dwDiffPosition==dwSource1Size)
				{
					dwRet=CSTR_EQUAL;
				}else{
					if ((*((BYTE*)(((SIZE_T)lpcSource1)+dwDiffPosition)))>(*((BYTE*)(((SIZE_T)lpcSource2)+dwDiffPosition))))
					{
						dwRet=CSTR_GREATER_THAN;
					}else{
						dwRet=CSTR_LESS_THAN;
					}
				}

				if (pdwDiffPosition) (*pdwDiffPosition)=dwDiffPosition;
			}else{
				if (lpcSource1)
				{//lpcSource2==NULL
					dwRet=CSTR_GREATER_THAN;
				}else{//lpcSource1==NULL
					dwRet=CSTR_LESS_THAN;
				}
			}
		}
	}else{
		if (dwSource1Size<dwSource2Size)
		{
			dwRet=CSTR_LESS_THAN;
		}else{
			dwRet=CSTR_GREATER_THAN;
		}
	}
return(dwRet);
}
*/


#endif // !defined(AFX_MEMORYCOMPARE__H__INCLUDED_)
