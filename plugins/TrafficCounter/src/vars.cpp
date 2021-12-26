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

#include "stdafx.h"

static wchar_t* GetTraffic(ARGUMENTSINFO *ai)
{
	uint32_t tmp, tmpsn = 0, tmprn = 0, tmpst = 0, tmprt = 0;
	uint8_t ed;

	if (ai->argc != 5) return nullptr;

	if (!mir_wstrcmp(ai->argv.w[1], L"overall")) {
		tmpsn = OverallInfo.CurrentSentTraffic;
		tmprn = OverallInfo.CurrentRecvTraffic;
		tmpst = OverallInfo.TotalSentTraffic;
		tmprt = OverallInfo.TotalRecvTraffic;
	}
	else if (!mir_wstrcmp(ai->argv.w[1], L"summary")) {
		for (ed = 0; ed < NumberOfAccounts; ed++)
			if (ProtoList[ed].Visible) {
				tmpsn += ProtoList[ed].CurrentSentTraffic;
				tmprn += ProtoList[ed].CurrentRecvTraffic;
				tmpst += ProtoList[ed].TotalSentTraffic;
				tmprt += ProtoList[ed].TotalRecvTraffic;
			}
	}
	else {	// Ищем индекс протокола, переданного первым аргументом
		for (tmp = ed = 0; ed < NumberOfAccounts; ed++) {
			if (!ProtoList[ed].name) continue;
			wchar_t *buf = mir_a2u(ProtoList[ed].name);
			if (!mir_wstrcmp(buf, ai->argv.w[1])) {
				tmpsn = ProtoList[ed].CurrentSentTraffic;
				tmprn = ProtoList[ed].CurrentRecvTraffic;
				tmpst = ProtoList[ed].TotalSentTraffic;
				tmprt = ProtoList[ed].TotalRecvTraffic;
				tmp = 0xAA; // Признак того, что протокол был найден
			}
			mir_free(buf);
		}
		if (tmp != 0xAA) return nullptr;
	}

	if (!mir_wstrcmp(ai->argv.w[2], L"now")) {
		if (!mir_wstrcmp(ai->argv.w[3], L"sent")) tmp = tmpsn;
		else
			if (!mir_wstrcmp(ai->argv.w[3], L"received")) tmp = tmprn;
			else
				if (!mir_wstrcmp(ai->argv.w[3], L"both")) tmp = tmprn + tmpsn;
				else return nullptr;
	}
	else if (!mir_wstrcmp(ai->argv.w[2], L"total")) {
		if (!mir_wstrcmp(ai->argv.w[3], L"sent")) tmp = tmpst;
		else
			if (!mir_wstrcmp(ai->argv.w[3], L"received")) tmp = tmprt;
			else
				if (!mir_wstrcmp(ai->argv.w[3], L"both")) tmp = tmprt + tmpst;
				else return nullptr;
	}
	else return nullptr;

	if (!mir_wstrcmp(ai->argv.w[4], L"b"))
		ed = 0;
	else if (!mir_wstrcmp(ai->argv.w[4], L"k"))
		ed = 1;
	else if (!mir_wstrcmp(ai->argv.w[4], L"m"))
		ed = 2;
	else if (!mir_wstrcmp(ai->argv.w[4], L"d"))
		ed = 3;
	else
		return nullptr;

	// Получаем форматированную строку и возвращаем указатель на неё.
	// Сначала узнаем размер буфера.
	size_t l = GetFormattedTraffic(tmp, ed, nullptr, 0);
	wchar_t *res = (wchar_t*)mir_alloc(l * sizeof(wchar_t));
	if (!res) return nullptr;
	if (GetFormattedTraffic(tmp, ed, res, l))
		return res;

	mir_free(res);
	return nullptr;
}

static wchar_t* GetTime(ARGUMENTSINFO *ai)
{
	uint8_t ed, flag;
	uint32_t Duration = 0;

	if (ai->argc != 4) return nullptr;

	// Ищем индекс протокола, переданного первым аргументом
	for (flag = ed = 0; ed < NumberOfAccounts; ed++) {
		wchar_t *buf;
		if (!ProtoList[ed].name) continue;
		buf = mir_a2u(ProtoList[ed].name);
		if (!mir_wstrcmp(buf, ai->argv.w[1])) {
			flag = 0xAA;
			if (!mir_wstrcmp(ai->argv.w[2], L"now"))
				Duration = ProtoList[ed].Session.Timer;
			else if (!mir_wstrcmp(ai->argv.w[2], L"total"))
				Duration = ProtoList[ed].Total.Timer;
			else
				flag = 0;
			break;
		}
		mir_free(buf);
	}

	if ((flag != 0xAA) && !mir_wstrcmp(ai->argv.w[1], L"summary")) {
		flag = 0xAA;
		if (!mir_wstrcmp(ai->argv.w[2], L"now"))
			Duration = OverallInfo.Session.Timer;
		else if (!mir_wstrcmp(ai->argv.w[2], L"total"))
			Duration = OverallInfo.Total.Timer;
		else flag = 0;
	}

	if (flag != 0xAA)
		return nullptr;

	// Получаем форматированную строку и возвращаем указатель на неё.
	// Сначала узнаем размер буфера.
	size_t l = GetDurationFormatM(Duration, ai->argv.w[3], nullptr, 0);
	wchar_t *res = (wchar_t*)mir_alloc(l * sizeof(wchar_t));
	if (!res)
		return nullptr;
	
	GetDurationFormatM(Duration, ai->argv.w[3], res, l);
	return res;
}

void RegisterVariablesTokens(void)
{
	TOKENREGISTER trs;

	if (!bVariablesExists) return;

	memset(&trs, 0, sizeof(trs));
	trs.cbSize = sizeof(TOKENREGISTER);

	// Функция, возвращающая трафик
	trs.szTokenString.w = L"tc_GetTraffic";
	trs.parseFunctionW = GetTraffic;
	trs.szHelpText = "Traffic counter\t(A,B,C,D)\tGet traffic counter value. A: <ProtocolName> OR overall OR summary; B: now OR total; C: sent OR received OR both; D: b - in bytes, k - in kilobytes, m - in megabytes, d - dynamic";
	trs.flags = TRF_TCHAR | TRF_PARSEFUNC | TRF_FUNCTION | TRF_FREEMEM;
	trs.memType = TR_MEM_MIRANDA;
	CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM)&trs);
	// Функция, возвращающая время
	trs.szTokenString.w = L"tc_GetTime";
	trs.parseFunctionW = GetTime;
	trs.szHelpText = "Traffic counter\t(A,B,C)\tGet time counter value. A: <ProtocolName> OR summary; B: now OR total; C: format";
	CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM)&trs);
}
