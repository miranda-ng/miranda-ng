/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-11  George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Revision       : $Revision: 13452 $
Last change on : $Date: 2011-03-17 21:12:56 +0200 (Чт, 17 мар 2011) $
Last change by : $Author: george.hazan $

*/

#ifndef _JABBER_PROXY_H_
#define _JABBER_PROXY_H_

int JabberHttpGatewayInit( HANDLE hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr );
int JabberHttpGatewayBegin( HANDLE hConn, NETLIBOPENCONNECTION *nloc );

#endif
