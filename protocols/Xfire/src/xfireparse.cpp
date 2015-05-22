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

#include "xfireparse.h"
namespace xfirelib {

using namespace std;

XFireParse::XFireParse() {
}


/*void XFireParse::readVariableAttribut( char *value, char *packet, char *attr,int packet_length,int attr_length, int start,int max_length ) {
int length_index = findString2(packet,attr, packet_length,mir_strlen(attr),start)+attr_length+1;

	unsigned int length = xfire_hex_to_intC(packet[length_index]);

	if (max_length >= length){
		memcpy(value,packet+length_index+2,length);
	}
	if (max_length > length){
		memset(value+length,0,1);
	}
	return length_index+2+length;
}*/


};
