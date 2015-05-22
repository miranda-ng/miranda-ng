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
#include <string.h>
#include <iostream>
#include "xfireutils.h"


namespace xfirelib {

using namespace std;

  XFireUtils::XFireUtils() {
  }

  int XFireUtils::addAttributName(char *packet,int packet_length, char *att){
    XDEBUG3( "Adding %d chars at position %d\n",mir_strlen(att),packet_length);
	packet[packet_length] = (char)mir_strlen(att);//set att length
	memcpy(packet+packet_length+1,att,mir_strlen(att)); //set attname
	return packet_length+1+mir_strlen(att);
  }
};
