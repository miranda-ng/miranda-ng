/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *			dufte <dufte@justmail.de>
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

#include "recvbuddychangednick.h"
#include "variablevalue.h"

//packet liest neue nicks ein

namespace xfirelib {
  using namespace std;

  void RecvBuddyChangedNick::parseContent(char *buf, int length, int numberOfAtts) {
    VariableValue val;
    int index = 0;
    index +=2; // Ignore 02 01 02 ??
    index += val.readValue(buf,index,4);
    userid = val.getValueAsLong();
	index +=2; // Ignore 0d 01
	int l = (unsigned char)buf[index++];
	index++;
	index += val.readValue(buf,index,l);
	string stringvalue = string(val.getValue(),l);
	this->newnick=stringvalue;
  }

};
