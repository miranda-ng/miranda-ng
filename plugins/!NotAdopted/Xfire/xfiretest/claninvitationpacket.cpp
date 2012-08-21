/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *			dufte <dufte@justmail.de>
 *
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

#include <vector>
#include <string>

#include "claninvitationpacket.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
	using namespace std;

	ClanInvitationPacket::ClanInvitationPacket() {
	}
	ClanInvitationPacket::~ClanInvitationPacket() {
	}

	void ClanInvitationPacket::parseContent(char *buf, int length, int numberOfAtts) {
		int index = 0;
		numberOfInv = 0;
		VariableValue val;

		index+=0x3; //einige bytes überspringen

		numberOfInv=buf[index];
		XDEBUG2("Invs %d\n",numberOfInv);

		index+=2; // überspring 0

		//clan id's überspringen, wird eh nicht gebraucht
		for(int i=0;i<numberOfInv;i++)
			index+=4;

		index++; // 0x72 überspringen

		index+=4; // weitere bytes überspringen

		//clanidnamen überspringen
		for(int i=0;i<numberOfInv;i++)
		{
			int length = (unsigned char)buf[index++];
			index++;
			index += val.readValue(buf,index,length);
			string stringvalue = string(val.getValue(),length);
			XDEBUG2("Clanidnames von %s\n",stringvalue.c_str());
		}

		//weitere 5 bytes überspringen
		index+=5;

		for(int i=0;i<numberOfInv;i++)
		{
			int length = (unsigned char)buf[index++];
			index++;
			index += val.readValue(buf,index,length);
			string stringvalue = string(val.getValue(),length);
			XDEBUG2("Inv von %s\n",stringvalue.c_str());
			clanname[i]=stringvalue;
		}

		//weitere 5 bytes skippen
		index+=5;

		//weitere leere bytes skippen, weis nich was für einen sinn haben
		for(int i=0;i<numberOfInv;i++)
			index+=4;

		//weitere 5 bytes skippen
		index+=5;

		//usernames auslesen
		for(int i=0;i<numberOfInv;i++)
		{
			int length = (unsigned char)buf[index++];
			index++;
			index += val.readValue(buf,index,length);
			string stringvalue = string(val.getValue(),length);
			XDEBUG2("Usernames von %s\n",stringvalue.c_str());
			invitefromusername[i]=stringvalue;
		}

		//nochmal weitere 5 bytes skippen
		index+=5;

		//nicks auslesen
		for(int i=0;i<numberOfInv;i++)
		{
			int length = (unsigned char)buf[index++];
			index++;
			index += val.readValue(buf,index,length);
			string stringvalue = string(val.getValue(),length);
			XDEBUG2("Nicks von %s\n",stringvalue.c_str());
			invitefrom[i]=stringvalue;
		}

		//nochmal weitere 5 bytes skippen
		index+=5;

		//einladungen auslesen
		for(int i=0;i<numberOfInv;i++)
		{
			int length = (unsigned char)buf[index++];
			index++;
			index += val.readValue(buf,index,length);
			string stringvalue = string(val.getValue(),length);
			XDEBUG2("Nicks von %s\n",stringvalue.c_str());
			invitemsg[i]=stringvalue;
		}

	}


};
