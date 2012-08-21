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
#include "buddylistonlinepacket.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include <vector>
#include "xdebug.h"
#include <iostream>

namespace xfirelib {
  using namespace std;

  void BuddyListOnlinePacket::parseContent(char *buf, int length, int numberOfAtts) {
	DUMPPACKET("BuddyListOnlinePacket")
    XINFO(( "Got List of buddys that are online\n" ));
    int index = 0;
    // friends
    VariableValue userid;
    userids = new vector<long>;

    /* auskommentiert, wird nicht mehr gesendet 2.3.11
	index += userid.readName(buf,index);
	*/
	index ++; // Ignore 01
    index ++; // Ignore 04
    index ++; // Ignore 02
	

    int numberOfIds = (unsigned char)buf[index];
    index++;
    index++;//ignore 00
    for(int i = 0 ; i < numberOfIds ; i++) {
      index += userid.readValue(buf,index,4);
      userids->push_back(userid.getValueAsLong());
      XINFO2( "UserID: %ld\n", userid.getValueAsLong() );
    }

    VariableValue sid;
    sids = new vector<char *>;
    /* auskommentiert, wird nicht mehr gesendet 2.3.11
	index += sid.readName(buf,index); */

	index ++; // Ignore 03
    index ++; // Ignore 04
    index ++; // Ignore 03
	

    numberOfIds = (unsigned char)buf[index];
    index++;
    index++;//ignore 00
    for(int i = 0 ; i < numberOfIds ; i++) {
      index += userid.readValue(buf,index,16);
      char *sid = new char[16];
      memcpy(sid,userid.getValue(),16);
      sids->push_back(sid);
      //for(int loop = 0; loop < userid.getValueLength();loop++){
      //      XINFO(( "SID: %d\n", userid.getValue()[loop] ));
      //}
    }

  }

};
