/*
 * Copyright (c) 2009 Rozhuk Ivan <rozhuk.im@gmail.com>
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



#if !defined(AFX_TIME_FUNCS__H__INCLUDED_)
#define AFX_TIME_FUNCS__H__INCLUDED_

#pragma once

#include <windows.h>
#include <time.h>



#define _MAX__TIME32_T	0x7fffd27f // number of seconds from
                                   //   00:00:00, 01/01/1970 UTC to
                                   //   23:59:59, 01/18/2038 UTC 

// Number of 100 nanosecond units from 1/1/1601 to 1/1/1970
#define EPOCH_BIAS  116444736000000000i64

//nion to facilitate converting from FILETIME to unsigned __int64
typedef union {
        unsigned __int64 ft_scalar;
        FILETIME ft_struct;
        } FT;


inline __time32_t _time32(__time32_t *timeptr)
{
	__time64_t tim;
	FT nt_time;

	GetSystemTimeAsFileTime(&(nt_time.ft_struct));
	tim=(__time64_t)((nt_time.ft_scalar-EPOCH_BIAS)/10000000i64);
	if (tim > (__time64_t)(_MAX__TIME32_T)) tim=(__time64_t)(-1);
	if (timeptr) *timeptr = (__time32_t)(tim);// store time if requested

return(__time32_t)(tim);
}


inline __time32_t MakeTime32FromLocalSystemTime(CONST PSYSTEMTIME pcstSystemTime)
{
	__time64_t tim=0;
	FT nt_time;

	if (SystemTimeToFileTime(pcstSystemTime,&(nt_time.ft_struct)))
	{
		if (LocalFileTimeToFileTime(&(nt_time.ft_struct),&(nt_time.ft_struct)))
		{
			tim=(__time64_t)((nt_time.ft_scalar-EPOCH_BIAS)/10000000i64);
			if (tim > (__time64_t)(_MAX__TIME32_T)) tim=(__time64_t)(-1);
		}
	}
return(__time32_t)(tim);
}


inline BOOL MakeLocalSystemTimeFromTime32(__time32_t tim32,PSYSTEMTIME pstSystemTime)
{
	BOOL bRet=FALSE;

	if (pstSystemTime)
	{
		__time64_t tim=(__time64_t)tim32;
		FT nt_time;

		//if (tim==(__time64_t)(-1)) tim=(__time64_t)(_MAX__TIME32_T);
		nt_time.ft_scalar=(__time64_t)((tim*10000000i64)+EPOCH_BIAS);
		if (FileTimeToLocalFileTime(&(nt_time.ft_struct),&(nt_time.ft_struct)))
		{
			bRet=FileTimeToSystemTime(&(nt_time.ft_struct),pstSystemTime);
		}
	}
return(bRet);
}


#endif // !defined(AFX_TIME_FUNCS__H__INCLUDED_)
