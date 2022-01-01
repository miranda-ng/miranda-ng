/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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

#ifndef _JABBER_BYTE_H_
#define _JABBER_BYTE_H_

typedef enum { JBT_INIT, JBT_AUTH, JBT_CONNECT, JBT_SOCKSERR, JBT_SENDING, JBT_RECVING, JBT_DONE, JBT_ERROR } JABBER_BYTE_STATE;

struct CJabberProto;
struct filetransfer;

struct JABBER_BYTE_TRANSFER : public MZeroedObject
{
	~JABBER_BYTE_TRANSFER();

	char *sid;
	char *srcJID;
	char *dstJID;
	char *streamhostJID;
	char *iqId;
	JABBER_BYTE_STATE state;
	HANDLE hConn;
	HANDLE hEvent;
	TiXmlDocument doc;
	TiXmlElement *iqNode;
	bool (CJabberProto::*pfnSend)(HNETLIBCONN hConn, filetransfer *ft);
	int  (CJabberProto::*pfnRecv)(HNETLIBCONN hConn, filetransfer *ft, char* buffer, int datalen);
	void (CJabberProto::*pfnFinal)(bool success, filetransfer *ft);
	filetransfer *ft;

	// XEP-0065 proxy support
	bool bProxyDiscovered, bStreamActivated;
	HANDLE hProxyEvent;
	char *szProxyHost;
	char *szProxyPort;
	char *szProxyJid;
	char *szStreamhostUsed;
	HANDLE hSendEvent;
};

#endif
