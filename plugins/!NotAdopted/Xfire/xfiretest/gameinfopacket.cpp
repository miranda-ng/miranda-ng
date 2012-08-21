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

#include "gameinfopacket.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  GameInfoPacket::GameInfoPacket() {
  }
  GameInfoPacket::~GameInfoPacket() {
  }

  void GameInfoPacket::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;
	int numberOfSids = 0;
    VariableValue val;

	index += val.readName(buf,index);

	index++; //ignore 04
	index++; //ignore 03

	XDEBUG2("Anzahl: %d\n",buf[index]);
	numberOfSids = buf[index];

	index++; //ignore 01
	index++; //ignore 0

	sids = new vector<char *>;
	for(int i = 0 ; i < numberOfSids ; i++) {
		index += val.readValue(buf,index,16);
		char *sid = new char[16];
		memcpy(sid,val.getValue(),16);
		sids->push_back(sid);
	}
	index += val.readName(buf,index);

	index += 4; // nächsten 4 bytes skippen

	gameinfo=new vector<string>;
	for(int i = 0 ; i < numberOfSids ; i++) {
      int length = (unsigned char)buf[index++];
      index++;
      index += val.readValue(buf,index,length);
      string stringvalue = string(val.getValue(),length);
      gameinfo->push_back(stringvalue);
      XDEBUG(( "String length: %2d : %s\n", length, stringvalue.c_str() ));
    }

	XDEBUG2("Position: %d\n",index);

  }


};
