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
#include "searchbuddy.h"
#include <string.h>
#include <iostream>

namespace xfirelib {
  using namespace std;

  int SearchBuddy::getPacketContent(char *packet) {
	int index = 0;

	// name
	VariableValue val;
	val.setName( "name" );
	index += val.writeName( packet, index );

	packet[index++] = 0x01;

	val.setValue((char*)searchstring.c_str(),searchstring.size());
	packet[index++] = searchstring.size()%256;
    packet[index++] = (int)searchstring.size()/256;
    index += val.writeValue(packet,index);

	val.setName( "fname" );
	index += val.writeName( packet, index );	

	packet[index++] = 0x01;
	packet[index++] = 0x00;
	packet[index++] = 0x00;

	val.setName( "lname" );
	index += val.writeName( packet, index );	

	packet[index++] = 0x01;
	packet[index++] = 0x00;
	packet[index++] = 0x00;

	val.setName( "email" );
	index += val.writeName( packet, index );	

	packet[index++] = 0x01;
	packet[index++] = 0x00;
	packet[index++] = 0x00;

	length = index;
	return index;
  }

  int SearchBuddy::getPacketAttributeCount() {
    return 4;
  }

}
