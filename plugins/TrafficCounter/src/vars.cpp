/*
Traffic Counter plugin for Miranda IM 
Copyright 2007-2011 Mironych.

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
/* ======================================================================================
Здесь содержатся функции для поддержки плагина Variables
Автор: Mironych
=======================================================================================*/

#include "commonheaders.h"

static TCHAR* GetTraffic(ARGUMENTSINFO *ai)
{
	DWORD tmp, tmpsn = 0, tmprn = 0, tmpst = 0, tmprt = 0;
	BYTE ed;

	if (ai->argc != 5) return NULL;

	if (!_tcscmp(ai->targv[1], _T("overall")))
	{
		tmpsn = OverallInfo.CurrentSentTraffic;
		tmprn = OverallInfo.CurrentRecvTraffic;
		tmpst = OverallInfo.TotalSentTraffic;
		tmprt = OverallInfo.TotalRecvTraffic;
	}
	else
	if (!_tcscmp(ai->targv[1], _T("summary")))
	{
		for (ed = 0; ed < NumberOfAccounts; ed++)
			if (ProtoList[ed].Visible)
			{
				tmpsn += ProtoList[ed].CurrentSentTraffic;
				tmprn += ProtoList[ed].CurrentRecvTraffic;
				tmpst += ProtoList[ed].TotalSentTraffic;
				tmprt += ProtoList[ed].TotalRecvTraffic;
			}
	}
	else
	{	// Ищем индекс протокола, переданного первым аргументом
		for (tmp = ed = 0; ed < NumberOfAccounts; ed++)
		{
			if (!ProtoList[ed].name) continue;
			TCHAR *buf = mir_a2t(ProtoList[ed].name);
			if (!_tcscmp(buf, ai->targv[1]))
			{
				tmpsn = ProtoList[ed].CurrentSentTraffic;
				tmprn = ProtoList[ed].CurrentRecvTraffic;
				tmpst = ProtoList[ed].TotalSentTraffic;
				tmprt = ProtoList[ed].TotalRecvTraffic;
				tmp = 0xAA; // Признак того, что протокол был найден
			}
			mir_free(buf);
		}
		if (tmp != 0xAA) return NULL;
	}

	if (!_tcscmp(ai->targv[2], _T("now")))
	{
		if (!_tcscmp(ai->targv[3], _T("sent"))) tmp = tmpsn;
		else
		if (!_tcscmp(ai->targv[3], _T("received"))) tmp = tmprn;
		else
		if (!_tcscmp(ai->targv[3], _T("both"))) tmp = tmprn + tmpsn;
		else return NULL;
	}
	else
	if (!_tcscmp(ai->targv[2], _T("total")))
	{
		if (!_tcscmp(ai->targv[3], _T("sent"))) tmp = tmpst;
		else
		if (!_tcscmp(ai->targv[3], _T("received"))) tmp = tmprt;
		else
		if (!_tcscmp(ai->targv[3], _T("both"))) tmp = tmprt + tmpst;
		else return NULL;
	}
	else return NULL;

	if (!_tcscmp(ai->targv[4], _T("b"))) ed = 0;
	else
	if (!_tcscmp(ai->targv[4], _T("k"))) ed = 1;
	else
	if (!_tcscmp(ai->targv[4], _T("m"))) ed = 2;
	else
	if (!_tcscmp(ai->targv[4], _T("d"))) ed = 3;
	else return NULL;

	// Получаем форматированную строку и возвращаем указатель на неё.
	// Сначала узнаем размер буфера.
	size_t l = GetFormattedTraffic(tmp, ed, NULL, 0);
	TCHAR *res = (TCHAR*)mir_alloc(l * sizeof(TCHAR));
	if (!res) return NULL;
	if (GetFormattedTraffic(tmp, ed, res, l))
		return res;

	mir_free(res);
	return NULL;
}

static TCHAR* GetTime(ARGUMENTSINFO *ai)
{
	BYTE ed, flag;
	DWORD Duration;

	if (ai->argc != 4) return NULL;

	// Ищем индекс протокола, переданного первым аргументом
	for (flag = ed = 0; ed < NumberOfAccounts; ed++)
	{
		TCHAR *buf;
		if (!ProtoList[ed].name) continue;
		buf = mir_a2t(ProtoList[ed].name);
		if (!_tcscmp(buf, ai->targv[1]))
		{
			flag = 0xAA;
			if (!_tcscmp(ai->targv[2], _T("now")))
				Duration = ProtoList[ed].Session.Timer;
			else if (!_tcscmp(ai->targv[2], _T("total")))
				Duration = ProtoList[ed].Total.Timer;
			else flag = 0;
			break;
		}
		mir_free(buf);
	}
	if ( (flag != 0xAA) && !_tcscmp(ai->targv[1], _T("summary")) )
	{
		flag = 0xAA;
		if (!_tcscmp(ai->targv[2], _T("now")))
			Duration = OverallInfo.Session.Timer;
		else if (!_tcscmp(ai->targv[2], _T("total")))
			Duration = OverallInfo.Total.Timer;
		else flag = 0;
	}
	
	if (flag != 0xAA) return NULL;

	// Получаем форматированную строку и возвращаем указатель на неё.
	// Сначала узнаем размер буфера.
	size_t l = GetDurationFormatM(Duration, ai->targv[3], NULL, 0);
	TCHAR *res = (TCHAR*)mir_alloc(l * sizeof(TCHAR));
	if (!res) return NULL;
	GetDurationFormatM(Duration, ai->targv[3], res, l);

	return res;
}

void RegisterVariablesTokens(void)
{
	TOKENREGISTER trs;
	
	if (!bVariablesExists) return;
		
	memset(&trs, 0, sizeof(trs));
	trs.cbSize = sizeof(TOKENREGISTER);

	// Функция, возвращающая трафик
	trs.tszTokenString = _T("tc_GetTraffic");
	trs.parseFunctionT = GetTraffic;
	trs.szHelpText = "Traffic counter\t(A,B,C,D)\tGet traffic counter value. A: <ProtocolName> OR overall OR summary; B: now OR total; C: sent OR received OR both; D: b - in bytes, k - in kilobytes, m - in megabytes, d - dynamic";
	trs.flags = TRF_TCHAR | TRF_PARSEFUNC | TRF_FUNCTION | TRF_FREEMEM;
	trs.memType = TR_MEM_MIRANDA;
	CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM)&trs);
	// Функция, возвращающая время
	trs.tszTokenString = _T("tc_GetTime");
	trs.parseFunctionT = GetTime;
	trs.szHelpText = "Traffic counter\t(A,B,C)\tGet time counter value. A: <ProtocolName> OR summary; B: now OR total; C: format";
	CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM)&trs);
}
