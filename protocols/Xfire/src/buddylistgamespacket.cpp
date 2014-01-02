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

#include "buddylistgamespacket.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  BuddyListGamesPacket::BuddyListGamesPacket() {
    ips = 0;
    ports = 0;
    gameids = 0;
    gameids2 = 0;
    sids = 0;
	type = 0;
  }
  BuddyListGamesPacket::~BuddyListGamesPacket() {
    if (ips) {
      for( vector<char*>::iterator it = ips->begin() ;
	   it != ips->end() ; it++) {
	delete[] *it;
      }
    }
    delete ips;
    delete ports;
    delete gameids;
    delete gameids2;
    delete sids;
  }

  void BuddyListGamesPacket::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;
    int numberOfSids = 0;
    VariableValue val;

    index += val.readName(buf,index);
    index ++; // Ignore 04
    index ++; // Ignore 03
    numberOfSids = (unsigned char) buf[index];
    index ++; // Ignore 00
    index ++;
    sids = new vector<char *>;
    for(int i = 0 ; i < numberOfSids ; i++) {
      index += val.readValue(buf,index,16);
      char *sid = new char[16];
      memcpy(sid,val.getValue(),16);
      sids->push_back(sid);
    }

    index += val.readName(buf,index);
    index ++; // Ignore 04
    index ++; // Ignore 03
    numberOfSids = (unsigned char) buf[index];
    index ++; // Ignore 00
    index ++;
 
    gameids = new vector<long>;
    gameids2 = new vector<long>;
    for(int i = 0 ; i < numberOfSids ; i++) {
        index += val.readValue(buf,index,2);
        long game = val.getValueAsLong();
        index += val.readValue(buf,index,2);
        long game2 = val.getValueAsLong();
        gameids->push_back(game);
        gameids2->push_back(game2);
    }

    index += val.readName(buf,index);
    index ++; // Ignore 04
    index ++; // Ignore 03
    numberOfSids = (unsigned char) buf[index];
    index ++; // Ignore 00
    index ++;

    ips = new vector<char *>;
    for(int i = 0 ; i < numberOfSids ; i++) {
      index += val.readValue(buf,index,4);
      char *ip = new char[4];
      memcpy(ip,val.getValue(),4);
      ips->push_back(ip);
    }

    index += val.readName(buf,index);
    index ++; // Ignore 04
    index ++; // Ignore 03
    numberOfSids = (unsigned char) buf[index];
    index ++; // Ignore 00
    index ++;

//	static int was=0;
//	int portwasnull=0;

	
//	was++;

    ports = new vector<long>;
    for(int i = 0 ; i < numberOfSids ; i++) {
      index += val.readValue(buf,index,2);
      long port = val.getValueAsLong();
//	  if (port==0)
//	  {
//		portwasnull=1;
//	  }
      ports->push_back(port);
	  index += 2; // fixed port bug - dufte
    }

//	char temp[512];
 /* sprintf(temp,"packet%d_%d.dmp",was,portwasnull);
  FILE * f = fopen(temp,"wb");
  fwrite(buf,length,1,f);
  fclose(f);*/

  }


};
