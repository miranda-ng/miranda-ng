/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

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
#ifndef _TLEN_IQ_H_
#define _TLEN_IQ_H_

#include "tlen_xml.h"
#include "tlen.h"

typedef void (*TLEN_IQ_PFUNC)(TlenProtocol *proto, XmlNode *iqNode);

typedef struct TLEN_IQ_FUNC_STRUCT {
	int iqId;					// id to match IQ get/set with IQ result
	TLEN_IQ_PROCID procId;	// must be unique in the list, except for IQ_PROC_NONE which can have multiple entries
	TLEN_IQ_PFUNC func;		// callback function
	time_t requestTime;			// time the request was sent, used to remove relinquent entries
} TLEN_IQ_FUNC;

void TlenIqInit(TlenProtocol *proto);
void TlenIqUninit(TlenProtocol *proto);
TLEN_IQ_PFUNC TlenIqFetchFunc(TlenProtocol *proto, int iqId);
void TlenIqAdd(TlenProtocol *proto, unsigned int iqId, TLEN_IQ_PROCID procId, TLEN_IQ_PFUNC func);

void TlenIqResultAuth(TlenProtocol *proto, XmlNode *iqNode);
void TlenIqResultRoster(TlenProtocol *proto, XmlNode *iqNode);
void TlenIqResultVcard(TlenProtocol *proto, XmlNode *iqNode);
void TlenIqResultSearch(TlenProtocol *proto, XmlNode *iqNode);
void TlenIqResultVersion(TlenProtocol *proto, XmlNode *iqNode);
void TlenIqResultInfo(TlenProtocol *proto, XmlNode *iqNode);
void TlenIqResultTcfg(TlenProtocol *proto, XmlNode *iqNode);

//void TlenIqResultSetPassword(XmlNode *iqNode, void *userdata);

#endif

