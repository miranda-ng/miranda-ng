/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *    http://xfirelib.sphene.net
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
 */

#include "stdafx.h"
#include "clientloginpacket.h"
#include "xfireutils.h"
#include "SHA1.h"
#include <string.h>
#include <iostream>
#include "baseProtocol.h"

namespace xfirelib {
  using namespace std;

  int ClientLoginPacket::getPacketContent(char *packet) {
	int index = 0;
 	/*Username*/

	index = XFireUtils::addAttributName(packet,index, "name");/*add username attribute*/
	packet[index] = 0x01;
	packet[index+1] = (char)name.length();
	packet[index+2] = 0x00;
	index +=3;
	std::copy(name.begin(),name.end(),packet+index);
	index += name.size();

	/*Crypted Password*/
	unsigned char pass[41];
	pass[40] = 0x00;
	cryptPassword(pass);
	index = XFireUtils::addAttributName(packet,index, "password");/*add username attribute*/
	packet[index] = 0x01;
	packet[index+1] = (char)40;
	packet[index+2] = 0x00;
	index += 3;

	memcpy(packet+index,pass,40);
	index += 40;

	VariableValue val;
	val.setName( "flags" );
	index += val.writeName( packet, index );
	packet[index++] = 0x02;
	packet[index++] = 0x00;
	packet[index++] = 0x00;
	packet[index++] = 0x00;
	packet[index++] = 0x00;

	val.setName( "sid" );
	index += val.writeName( packet, index );
	packet[index++] = 0x03;

	for(int i=0;i<16;i++)
	{
		packet[index++] = 0x0;
	}

	length = index;
	return index;
  }

  int ClientLoginPacket::getPacketAttributeCount() {
    return 3;
  }

  void ClientLoginPacket::cryptPassword(unsigned char *crypt){
    std::string total;
    char temp[81];
    CSHA1 sha1;

    total = name+password+"UltimateArena";
    hashSha1(total.c_str(),crypt);
    memcpy(temp,crypt,40);
    memcpy(temp+40,salt->getValue(),40);
    temp[80] = 0x00;

    hashSha1(temp,crypt);
  }



/*give string and you will get the hash*/
void ClientLoginPacket::hashSha1(const char *string, unsigned char *sha){

	char result[41];
        unsigned char temp[1024];
        CSHA1 sha1;
        sha1.Reset();
        sha1.Update((UINT_8 *)string, mir_strlen(string));
        sha1.Final();
        sha1.GetHash(temp);	

        result[0] = 0; 
	char szTemp[16]; 
	for(int i = 0 ; i < 20 ; i++) { 
	  mir_snprintf(szTemp, SIZEOF(szTemp), "%02x", temp[i]); 
	  strcat(result,szTemp); 
	}
	memcpy(sha,result,40);
	sha[40] = 0x00;
}
}
