// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
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
// File name      : $URL: http://miranda.googlecode.com/svn/trunk/miranda/protocols/IcqOscarJ/directpackets.h $
// Revision       : $Revision: 7484 $
// Last change on : $Date: 2008-03-21 11:43:39 +0200 (ÐŸÑ‚, 21 Ð¼Ð°Ñ€ 2008) $
// Last change by : $Author: ghazan $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#ifndef __ICQ_DIRECTPACKETS_H
#define __ICQ_DIRECTPACKETS_H

// Direct packet senders
void packDirectMsgHeader(icq_packet *packet, WORD wDataLen, WORD wCommand, DWORD dwCookie, BYTE bMsgType, BYTE bMsgFlags, WORD wX1, WORD wX2);

#endif /* __ICQ_DIRECTPACKETS_H */
