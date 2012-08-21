/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network. 
 *
 *  Copyright (C) 2010 by
 *          dufte <dufte@justmail.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 *  Based on J. Lawler              - BaseProtocol
 *			 Herbert Poul/Beat Wolf - xfirelib
 *
 *  Miranda ICQ: the free icq client for MS Windows 
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

//sendtypingklasse hinzugefügt http://xfirelib.sphene.net/board2/showThread/367 - big thx to Fl0ri4n - dufte

#ifndef __SENDTYPINGPACKET_H
#define __SENDTYPINGPACKET_H



#include "xfiresendpacketcontent.h"
#include "variablevalue.h"
#include <string.h>
#include "client.h"

namespace xfirelib {

class SendTypingPacket : public XFireSendPacketContent {
public:
SendTypingPacket() {
imindex = 0;
}
virtual ~SendTypingPacket() { }

void init(Client *client, string username);
void setSid(const char *sid);

XFirePacketContent* newPacket() { return new SendTypingPacket(); }

int getPacketId() { return 2; }
int getPacketContent(char *buf);
int getPacketAttributeCount() {return 2;};
int getPacketSize() { return 1024; };

/**
* SID of the user to who the message should be sent.
*/
char sid[16];
/**
* A running counter for each buddy. (will be initialized to 0 by default.. and..
* shouldn't be a problem to leave it 0)
*/
long imindex;

protected:
void initIMIndex();

static std::map<std::string,int> imindexes;

};

};

#endif 