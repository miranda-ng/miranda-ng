// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $URL: http://miranda.googlecode.com/svn/trunk/miranda/protocols/IcqOscarJ/icq_server.h $
// Revision       : $Revision: 13213 $
// Last change on : $Date: 2010-12-22 08:54:39 +0200 (Ð¡Ñ€, 22 Ð´ÐµÐº 2010) $
// Last change by : $Author: borkra $
//
// DESCRIPTION:
//
//  Declarations for server thread
//
// -----------------------------------------------------------------------------

#ifndef __ICQ_SERVER_H
#define __ICQ_SERVER_H

struct serverthread_start_info
{
	NETLIBOPENCONNECTION nloc;
	WORD wPassLen;
	char szPass[128];
};

struct serverthread_info
{
	struct CIcqProto *ppro;
	int bLoggedIn;
	int isLoginServer;
	BYTE szAuthKey[20];
	WORD wAuthKeyLen;
	WORD wServerPort;
	char *newServer;
	BYTE *cookieData;
	int cookieDataLen;
	int newServerSSL;
	int newServerReady;
	int isMigrating;
	HANDLE hPacketRecver;
	int bReinitRecver;
	int bMyAvatarInited;
//
	HANDLE hDirectBoundPort;
//
	HANDLE hKeepAliveEvent;
	HANDLE hKeepAliveThread;
};

/*---------* Functions *---------------*/

void icq_serverDisconnect(BOOL bBlock);
void icq_login(const char *szPassword);

int IsServerOverRate(WORD wFamily, WORD wCommand, int nLevel);


#endif /* __ICQ_SERVER_H */
