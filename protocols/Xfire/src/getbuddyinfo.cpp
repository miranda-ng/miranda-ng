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
#include "getbuddyinfo.h"
#include <string.h>
#include <iostream>

/*
	mit dem paket kan man buddyinfos anfordern, bei zu aggressiven anfragen kappt xfire die verbindung
	getestet mit clanbuddy's >270 members
*/

namespace xfirelib {
  using namespace std;

  int GetBuddyInfo::getPacketContent(char *packet) {
	int index = 0;

	VariableValue val;
	val.setValueFromLong(userid,4);

	packet[index++] = 0x01;
	packet[index++] = 0x02;
	index += val.writeValue(packet, index);

	length = index;
	return index;
  }

  int GetBuddyInfo::getPacketAttributeCount() {
    return 1;
  }

}
