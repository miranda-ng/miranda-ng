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

#include "xfireclanpacket.h"
#include "variablevalue.h"
#include <string>

namespace xfirelib {
  using namespace std;

  void XFireClanPacket::parseContent(char *buf, int nlength, int numberOfAtts) {
    VariableValue val;
	int index = 0;
	int length = 0;
	string stringvalue;

	index += 3; // ersten 5 bytes skippen

	this->count=(char)buf[index];

	index += 2;

	for(int i=0;i<this->count;i++)
	{
		index += val.readValue(buf,index,2); //clanid lesen
		this->clanid[i] = val.getValueAsLong();
		index += 2;
	}

	index += 5; // 7 bytes skippen

	for(int i=0;i<this->count;i++)
	{
		length = (unsigned char)buf[index++]; //clannamen lesen
		index++;
		index += val.readValue(buf,index,length);
		stringvalue = string(val.getValue(),length);

		this->name[i] = stringvalue;
	}

	index += 5; // 5 skippen

	for(int i=0;i<this->count;i++)
	{
		length = (unsigned char)buf[index++]; //url anhängsel auslesen
		index++;
		index += val.readValue(buf,index,length);
		stringvalue = string(val.getValue(),length);

		this->url[i] = stringvalue;
	}

  }

};
