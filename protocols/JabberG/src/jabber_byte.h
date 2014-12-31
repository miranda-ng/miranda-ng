/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

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

struct JABBER_BYTE_TRANSFER
{
	~JABBER_BYTE_TRANSFER();

	TCHAR *sid;
	TCHAR *srcJID;
	TCHAR *dstJID;
	TCHAR *streamhostJID;
	TCHAR *iqId;
	JABBER_BYTE_STATE state;
	HANDLE hConn;
	HANDLE hEvent;
	HXML   iqNode;
	BOOL (CJabberProto::*pfnSend)(HANDLE hConn, filetransfer *ft);
	int (CJabberProto::*pfnRecv)(HANDLE hConn, filetransfer *ft, char* buffer, int datalen);
	void (CJabberProto::*pfnFinal)(BOOL success, filetransfer *ft);
	filetransfer *ft;

	// XEP-0065 proxy support
	BOOL bProxyDiscovered;
	HANDLE hProxyEvent;
	TCHAR *szProxyHost;
	TCHAR *szProxyPort;
	TCHAR *szProxyJid;
	TCHAR *szStreamhostUsed;
	BOOL bStreamActivated;
	HANDLE hSendEvent;
};

#endif
