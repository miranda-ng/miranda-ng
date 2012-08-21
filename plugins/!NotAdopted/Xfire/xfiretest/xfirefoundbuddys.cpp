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

#include "xfirefoundbuddys.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  XFireFoundBuddys::XFireFoundBuddys() {
    usernames = 0;
    fname = 0;
    lname = 0;
  }
  XFireFoundBuddys::~XFireFoundBuddys() {
    delete usernames;
    delete fname;
    delete lname;
  }

  void XFireFoundBuddys::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;

	VariableValue bs;

	XDEBUG2( "Len: %ld\n", length );

	XDEBUG2( "Byte1: %ld\n", (char)buf[index] );
	XDEBUG2( "Byte2: %ld\n", (char)buf[index+1] );
	XDEBUG2( "Byte3: %ld\n", (char)buf[index+2] );

	usernames = new vector<string>;
    index = readStrings(usernames,buf,index);

	fname = new vector<string>;
    index = readStrings(fname,buf,index);

	lname = new vector<string>;
    index = readStrings(lname,buf,index);

  }

  int XFireFoundBuddys::readStrings(vector<string> *strings, char *buf, int index) {
    VariableValue friends;
	index+=friends.readName(buf,index);
    index ++; // Ignore 04
	index ++; // Ignore 01

	index+=friends.readValue(buf,index,2);

    int numberOfStrings = friends.getValueAsLong();
    XDEBUG3( "name: %s numberOfStrings: %d\n", friends.getName().c_str(), numberOfStrings );
    for(int i = 0 ; i < numberOfStrings ; i++) {
		XDEBUG2( "Index: %ld\n", index );
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
