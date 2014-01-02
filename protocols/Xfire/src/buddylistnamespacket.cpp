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
#include <vector>
#include <string>

#include "buddylistnamespacket.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  BuddyListNamesPacket::BuddyListNamesPacket() {
    usernames = 0;
    nicks = 0;
    userids = 0;
  }
  BuddyListNamesPacket::~BuddyListNamesPacket() {
    delete usernames;
    delete nicks;
    delete userids;
  }

  void BuddyListNamesPacket::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;
	//prüfe ob das packet mit 0x7 anfängt um eventuell crashes zufixen
	if (buf[0]!=0x7)
	{
		usernames = new vector<string>;
		nicks = new vector<string>;
		userids = new vector<long>;
		return;
	}

    // friends
    VariableValue friends;
    index += friends.readName(buf,index);
    index ++; // Ignore 04
	index ++; // Ignore 01, dufte skip 1

    usernames = new vector<string>;
    index = readStrings(usernames,buf,index);

    index += friends.readName(buf,index);
    index ++; // Ignore 04
	index ++; // Ignore 01, dufte skip 1

    nicks = new vector<string>;
    index = readStrings(nicks,buf,index);

    index += friends.readName(buf,index);
    index ++; // Ignore 04
	index ++; // Ignore 01, dufte skip 1

    index += friends.readValue(buf,index,2); // 2 bytes lesen, für große mengen an friends
    userids = new vector<long>;
    int numberOfIds = friends.getValueAsLong();
    for(int i = 0 ; i < numberOfIds ; i++) {
      index += friends.readValue(buf,index,4);
      userids->push_back(friends.getValueAsLong());
      XDEBUG2( "UserID: %ld\n", friends.getValueAsLong() );
    }
  }

  int BuddyListNamesPacket::readStrings(vector<string> *strings, char *buf, int index) {
    VariableValue friends;
    index += friends.readValue(buf,index,2); //jeweils 2 bytes lesen
    //index ++; // Ignore 00 0 brauch nicht mehr geskippt werden
    int numberOfStrings = friends.getValueAsLong();
    XDEBUG3( "name: %s numberOfStrings: %d\n", friends.getName().c_str(), numberOfStrings );
    for(int i = 0 ; i < numberOfStrings ; i++) {
      int length = (unsigned char)buf[index++];
      index++;
      index += friends.readValue(buf,index,length);
      string stringvalue = string(friends.getValue(),length);
      strings->push_back(stringvalue);
      XDEBUG3( "String length: %2d : %s\n", length, stringvalue.c_str() );
    }
    return index;
  }

};
