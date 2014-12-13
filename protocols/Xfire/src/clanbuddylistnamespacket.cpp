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

#include "clanbuddylistnamespacket.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  ClanBuddyListNamesPacket::ClanBuddyListNamesPacket() {
    usernames = 0;
    nicks = 0;
    userids = 0;
	clanid = 0;
  }
  ClanBuddyListNamesPacket::~ClanBuddyListNamesPacket() {
    delete usernames;
    delete nicks;
    delete userids;
  }

  void ClanBuddyListNamesPacket::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;

	VariableValue friends;

	XDEBUG2( "Len: %d\n", length );

	index+=2; //paar bytes überspringen

	index += friends.readValue(buf,index,2);
	this->clanid = friends.getValueAsLong();

	index+=4; //paar bytes überspringen

    userids = new vector<long>;

	index += friends.readValue(buf,index);
   
	int numberOfIds = friends.getValueAsLong();

	XDEBUG2( "numberofId: %d\n", numberOfIds );

    for(int i = 0 ; i < numberOfIds ; i++) {
      index += friends.readValue(buf,index,4);
      userids->push_back(friends.getValueAsLong());
      XDEBUG3( "UserID: %ld %ld\n",i+1, friends.getValueAsLong() );
    }

	XDEBUG2( "NextByte: %c\n",buf[index] );
	XDEBUG2( "NextByte: %c\n",buf[index+1] );

	index+=3; //nächsten 3 skippen

	XDEBUG2( "NextByte: %c\n",buf[index] );
	XDEBUG2( "NextByte: %c\n",buf[index+1] );
	
	usernames = new vector<string>;
    index = readStrings(usernames,buf,index);

	index+=3; //nächsten 3 skippen

	nicks = new vector<string>;
    index = readStrings(nicks,buf,index);

  }

  int ClanBuddyListNamesPacket::readStrings(vector<string> *strings, char *buf, int index) {
    VariableValue friends;
    index += friends.readValue(buf,index,2);
    //index ++; // Ignore 00
    int numberOfStrings = friends.getValueAsLong();
    XDEBUG3( "name: %s numberOfStrings: %d\n", friends.getName().c_str(), numberOfStrings );
    for(int i = 0 ; i < numberOfStrings ; i++) {
      int length = (unsigned char)buf[index++];
      index++;
      index += friends.readValue(buf,index,length);
      string stringvalue = string(friends.getValue(),length);
      strings->push_back(stringvalue);
      XDEBUG(( "String length: %2d : %s\n", length, stringvalue.c_str() ));
    }
    return index;
  }

};
