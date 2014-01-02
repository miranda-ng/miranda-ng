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

#include "xdebug.h"
#include "loginsuccesspacket.h"

namespace xfirelib {

  void LoginSuccessPacket::parseContent(char *buf, int length, int numberOfAtts) {

    int read = 0;
    for(int i = 0 ; i < numberOfAtts ; i++) {
      VariableValue *val = new VariableValue();
      read += val->readName(buf, read);
      XDEBUG(( "Read Variable Name: %s\n", val->getName().c_str() ));
      if (val->getName() == "userid") {
	read++; // ignore 02
	read += val->readValue(buf, read, 3);
	read++; // ignore 00
	XDEBUG2( "My userid: %lu\n", val->getValueAsLong() );
	this->myuid=val->getValueAsLong();
      } else if (val->getName() == "sid") {
	read++; // ignore 03
	read+=val->readValue(buf, read, 16);
	//XDEBUG(( "My SID: %u\n", val->getValue() ));
      } else if (val->getName() == "nick") {
	//int lengthLength = (int)val->getValueAsLong();
	read++; // ignore 01
	//read+=val->readValue(buf, read, -1, 1);

	unsigned int l = (unsigned char)buf[read++];  //dufte - nick wird benötigt
	XDEBUG2( "Nick Length: %d\n", l );
    read++;
    read += val->readValue(buf,read,l);
	XDEBUG2( "Nick Length: %s\n", val->getValue() );

	this->nick=std::string(val->getValue(),l);

      } else if (val->getName() == "status") {
	read+=5; // ignore everything
      } else if (val->getName() == "dlset") {
	read+=3; // ignore everything
      } else {
	i = numberOfAtts; 
	// If we find something we don't know .. we stop parsing the 
	// packet.. who cares about the rest...
      }
      delete val;
    }
  }


};
