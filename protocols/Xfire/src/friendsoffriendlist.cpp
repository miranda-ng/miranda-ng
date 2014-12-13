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

#include "friendsoffriendlist.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  FriendsBuddyListNamesPacket::FriendsBuddyListNamesPacket() {
    usernames = 0;
    nicks = 0;
    userids = 0;
  }
  FriendsBuddyListNamesPacket::~FriendsBuddyListNamesPacket() {
    delete usernames;
    delete nicks;
    delete userids;
  }

  void FriendsBuddyListNamesPacket::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;

	VariableValue friends;

	XDEBUG2( "Len: %d\n", length );

	index+=8; //paar bytes überspringen

    int numberOfIds = (unsigned char)buf[index];
	XDEBUG2( "numberofId: %d\n", numberOfIds );
    index++;
    index++;//ignore 00
	sids = new vector<char *>;
    for(int i = 0 ; i < numberOfIds ; i++) {
      index += friends.readValue(buf,index,16);
      char *sid = new char[16];
      memcpy(sid,friends.getValue(),16);
      sids->push_back(sid);
    }

	XERROR("Sids ausgelesen\n");

	index+=7;

	index+=4;

	userids = new vector<long>;
    for(int i = 0 ; i < numberOfIds ; i++) {
      index += friends.readValue(buf,index,4);
      userids->push_back(friends.getValueAsLong());
      XDEBUG3( "UserID: %ld %ld\n",i+1, friends.getValueAsLong() );
    }

	index+=7;

	usernames = new vector<string>;
    index = readStrings(usernames,buf,index);

	index+=7;

	nicks = new vector<string>;
    index = readStrings(nicks,buf,index);

  }

  int FriendsBuddyListNamesPacket::readStrings(vector<string> *strings, char *buf, int index) {
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
