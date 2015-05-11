/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua

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

#include "tlen.h"
#include "tlen_iq.h"


void TlenIqInit(TlenProtocol *proto)
{
	proto->iqList = NULL;
	proto->iqCount = 0;
	proto->iqAlloced = 0;
}

void TlenIqUninit(TlenProtocol *proto)
{
	if (proto->iqList) mir_free(proto->iqList);
	proto->iqList = NULL;
	proto->iqCount = 0;
	proto->iqAlloced = 0;
}

static void TlenIqRemove(TlenProtocol *proto, int index)
{
	mir_cslock lck(proto->csIqList);
	if (index >= 0 && index<proto->iqCount) {
		memmove(proto->iqList+index, proto->iqList+index+1, sizeof(TLEN_IQ_FUNC)*(proto->iqCount-index-1));
		proto->iqCount--;
	}
}

static void TlenIqExpire(TlenProtocol *proto)
{
	int i;
	time_t expire;

	mir_cslock lck(proto->csIqList);
	expire = time(NULL) - 120;	// 2 minute
	i = 0;
	while (i < proto->iqCount) {
		if (proto->iqList[i].requestTime < expire)
			TlenIqRemove(proto, i);
		else
			i++;
	}
}

TLEN_IQ_PFUNC TlenIqFetchFunc(TlenProtocol *proto, int iqId)
{
	int i;
	TLEN_IQ_PFUNC res;

	mir_cslock lck(proto->csIqList);
	TlenIqExpire(proto);
	for (i=0; i<proto->iqCount && proto->iqList[i].iqId != iqId; i++);
	if (i < proto->iqCount) {
		res = proto->iqList[i].func;
		TlenIqRemove(proto, i);
	}
	else {
		res = (TLEN_IQ_PFUNC) NULL;
	}
	return res;
}

void TlenIqAdd(TlenProtocol *proto, unsigned int iqId, TLEN_IQ_PROCID procId, TLEN_IQ_PFUNC func)
{
	int i;

	mir_cslock lck(proto->csIqList);
	if (procId == IQ_PROC_NONE)
		i = proto->iqCount;
	else
		for (i=0; i<proto->iqCount && proto->iqList[i].procId != procId; i++);

	if (i >= proto->iqCount && proto->iqCount >= proto->iqAlloced) {
		proto->iqAlloced = proto->iqCount + 8;
		proto->iqList = (TLEN_IQ_FUNC*)mir_realloc(proto->iqList, sizeof(TLEN_IQ_FUNC)*proto->iqAlloced);
	}

	if (proto->iqList != NULL) {
		proto->iqList[i].iqId = iqId;
		proto->iqList[i].procId = procId;
		proto->iqList[i].func = func;
		proto->iqList[i].requestTime = time(NULL);
		if (i == proto->iqCount) proto->iqCount++;
	}
}

