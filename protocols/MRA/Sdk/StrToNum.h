/*
 * Copyright (c) 2005 Rozhuk Ivan <rozhuk.im@gmail.com>
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



#if !defined(AFX_STRTONUM__H__INCLUDED_)
#define AFX_STRTONUM__H__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000





__inline SIZE_T StrToUNum(LPCSTR lpcszString,SIZE_T dwStringLen)
{
	SIZE_T dwNum=0;
	BYTE bCurentFigure;


	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			dwNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			dwNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

return(dwNum);
}


__inline DWORD StrToUNum32(LPCSTR lpcszString,SIZE_T dwStringLen)
{
	DWORD dwNum=0;
	BYTE bCurentFigure;


	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			dwNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			dwNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

return(dwNum);
}


__inline DWORDLONG StrToUNum64(LPCSTR lpcszString,SIZE_T dwStringLen)
{
	DWORDLONG dwlNum=0;
	BYTE bCurentFigure;


	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			dwlNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			dwlNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

return(dwlNum);
}




__inline DWORD StrToUNumEx(LPCSTR lpcszString,SIZE_T dwStringLen,SIZE_T *pdwNum)
{
	DWORD dwRetErrorCode;
	SIZE_T dwNum=0,dwProcessed=0;
	BYTE bCurentFigure;


	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			dwNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			dwNum+=bCurentFigure;// добавляем цифру в младший разряд
			dwProcessed++;// увеличиваем счётчик обработанных цифр
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	if (dwProcessed)
	{// как минимум одна цифра была обработана
		if (pdwNum) (*pdwNum)=dwNum;
		if (dwProcessed==dwStringLen)
		{// в строке были только цифры, всё отработало как нужно //Операция успешно завершена.
			dwRetErrorCode=NO_ERROR;
		}else{// в строке были не только цифры //Имеются дополнительные данные.
			dwRetErrorCode=ERROR_MORE_DATA;
		}
	}else{// строка вообще не содержала цифр //Недопустимые данные.
		dwRetErrorCode=ERROR_INVALID_DATA;
	}
return(dwRetErrorCode);
}


__inline DWORD StrToUNumEx32(LPCSTR lpcszString,SIZE_T dwStringLen,DWORD *pdwNum)
{
	DWORD dwRetErrorCode;
	DWORD dwNum=0,dwProcessed=0;
	BYTE bCurentFigure;


	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			dwNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			dwNum+=bCurentFigure;// добавляем цифру в младший разряд
			dwProcessed++;// увеличиваем счётчик обработанных цифр
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	if (dwProcessed)
	{// как минимум одна цифра была обработана
		if (pdwNum) (*pdwNum)=dwNum;
		if (dwProcessed==dwStringLen)
		{// в строке были только цифры, всё отработало как нужно //Операция успешно завершена.
			dwRetErrorCode=NO_ERROR;
		}else{// в строке были не только цифры //Имеются дополнительные данные.
			dwRetErrorCode=ERROR_MORE_DATA;
		}
	}else{// строка вообще не содержала цифр //Недопустимые данные.
		dwRetErrorCode=ERROR_INVALID_DATA;
	}
return(dwRetErrorCode);
}


__inline DWORD StrToUNumEx64(LPCSTR lpcszString,SIZE_T dwStringLen,DWORDLONG *pdwlNum)
{
	DWORD dwRetErrorCode;
	DWORDLONG dwlNum=0;
	SIZE_T dwProcessed=0;
	BYTE bCurentFigure;


	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			dwlNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			dwlNum+=bCurentFigure;// добавляем цифру в младший разряд
			dwProcessed++;// увеличиваем счётчик обработанных цифр
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	if (dwProcessed)
	{// как минимум одна цифра была обработана
		if (pdwlNum) (*pdwlNum)=dwlNum;
		if (dwProcessed==dwStringLen)
		{// в строке были только цифры, всё отработало как нужно //Операция успешно завершена.
			dwRetErrorCode=NO_ERROR;
		}else{// в строке были не только цифры //Имеются дополнительные данные.
			dwRetErrorCode=ERROR_MORE_DATA;
		}
	}else{// строка вообще не содержала цифр //Недопустимые данные.
		dwRetErrorCode=ERROR_INVALID_DATA;
	}
return(dwRetErrorCode);
}




__inline SSIZE_T StrToNum(LPCSTR lpcszString,SIZE_T dwStringLen)
{
	SSIZE_T lNum=0,lSingn=1;
	BYTE bCurentFigure;


	while(dwStringLen && ((bCurentFigure=((*lpcszString)-48))>9))
	{
		if (bCurentFigure=='-') lSingn=-1;
		if (bCurentFigure=='+') lSingn=1;

		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			lNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			lNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}
	lNum*=lSingn;

return(lNum);
}


__inline LONG StrToNum32(LPCSTR lpcszString,SIZE_T dwStringLen)
{
	LONG lNum=0,lSingn=1;
	BYTE bCurentFigure;


	while(dwStringLen && ((bCurentFigure=((*lpcszString)-48))>9))
	{
		if (bCurentFigure=='-') lSingn=-1;
		if (bCurentFigure=='+') lSingn=1;

		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			lNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			lNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}
	lNum*=lSingn;

return(lNum);
}


__inline LONGLONG StrToNum64(LPCSTR lpcszString,SIZE_T dwStringLen)
{
	LONGLONG llNum=0,llSingn=1;
	BYTE bCurentFigure;


	while(dwStringLen && ((bCurentFigure=((*lpcszString)-48))>9))
	{
		if (bCurentFigure=='-') llSingn=-1;
		if (bCurentFigure=='+') llSingn=1;

		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			llNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			llNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}
	llNum*=llSingn;

return(llNum);
}



__inline DWORD StrToNumEx(LPCSTR lpcszString,SIZE_T dwStringLen,SSIZE_T *plNum)
{
	DWORD dwRetErrorCode;
	SIZE_T dwProcessed=0;
	SSIZE_T lNum=0,lSingn=1;
	BYTE bCurentFigure;


	while(dwStringLen && ((bCurentFigure=((*lpcszString)-48))>9))
	{
		if (bCurentFigure=='-') lSingn=-1;
		if (bCurentFigure=='+') lSingn=1;

		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			lNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			lNum+=bCurentFigure;// добавляем цифру в младший разряд
			dwProcessed++;// увеличиваем счётчик обработанных цифр
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	if (dwProcessed)
	{// как минимум одна цифра была обработана
		if (plNum) (*plNum)=(lNum*lSingn);
		if (dwProcessed==dwStringLen)
		{// в строке были только цифры, всё отработало как нужно //Операция успешно завершена.
			dwRetErrorCode=NO_ERROR;
		}else{// в строке были не только цифры //Имеются дополнительные данные.
			dwRetErrorCode=ERROR_MORE_DATA;
		}
	}else{// строка вообще не содержала цифр //Недопустимые данные.
		dwRetErrorCode=ERROR_INVALID_DATA;
	}
return(dwRetErrorCode);
}


__inline DWORD StrToNumEx32(LPCSTR lpcszString,SIZE_T dwStringLen,LONG *plNum)
{
	DWORD dwRetErrorCode;
	SIZE_T dwProcessed=0;
	LONG lNum=0,lSingn=1;
	BYTE bCurentFigure;


	while(dwStringLen && ((bCurentFigure=((*lpcszString)-48))>9))
	{
		if (bCurentFigure=='-') lSingn=-1;
		if (bCurentFigure=='+') lSingn=1;

		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			lNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			lNum+=bCurentFigure;// добавляем цифру в младший разряд
			dwProcessed++;// увеличиваем счётчик обработанных цифр
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	if (dwProcessed)
	{// как минимум одна цифра была обработана
		if (plNum) (*plNum)=(lNum*lSingn);
		if (dwProcessed==dwStringLen)
		{// в строке были только цифры, всё отработало как нужно //Операция успешно завершена.
			dwRetErrorCode=NO_ERROR;
		}else{// в строке были не только цифры //Имеются дополнительные данные.
			dwRetErrorCode=ERROR_MORE_DATA;
		}
	}else{// строка вообще не содержала цифр //Недопустимые данные.
		dwRetErrorCode=ERROR_INVALID_DATA;
	}
return(dwRetErrorCode);
}


__inline DWORD StrToNumEx64(LPCSTR lpcszString,SIZE_T dwStringLen,LONGLONG *pllNum)
{
	DWORD dwRetErrorCode;
	SIZE_T dwProcessed=0;
	LONGLONG llNum=0,llSingn=1;
	BYTE bCurentFigure;


	while(dwStringLen && ((bCurentFigure=((*lpcszString)-48))>9))
	{
		if (bCurentFigure=='-') llSingn=-1;
		if (bCurentFigure=='+') llSingn=1;

		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			llNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			llNum+=bCurentFigure;// добавляем цифру в младший разряд
			dwProcessed++;// увеличиваем счётчик обработанных цифр
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	if (dwProcessed)
	{// как минимум одна цифра была обработана
		if (pllNum) (*pllNum)=(llNum*llSingn);
		if (dwProcessed==dwStringLen)
		{// в строке были только цифры, всё отработало как нужно //Операция успешно завершена.
			dwRetErrorCode=NO_ERROR;
		}else{// в строке были не только цифры //Имеются дополнительные данные.
			dwRetErrorCode=ERROR_MORE_DATA;
		}
	}else{// строка вообще не содержала цифр //Недопустимые данные.
		dwRetErrorCode=ERROR_INVALID_DATA;
	}
return(dwRetErrorCode);
}




#endif // !defined(AFX_STRTONUM__H__INCLUDED_)