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

#include "buddyinfo.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  void BuddyInfoPacket::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;

	VariableValue val;

	XDEBUG2( "Len: %ld\n", length );

	index+=2; //paar bytes überspringen

	index += val.readValue(buf,index,4);
	this->userid = val.getValueAsLong();

	XDEBUG2( "UserId: %ld\n", this->userid);

	index+=2;

	this->avatarmode = buf[index];
	XDEBUG2( "Avatarmode: %ld\n", this->avatarmode);
	index+=6;


	index += val.readValue(buf,index,4);
	this->avatarid = val.getValueAsLong();
	XDEBUG2( "Avatarid: %ld\n", this->avatarid);

  }

};