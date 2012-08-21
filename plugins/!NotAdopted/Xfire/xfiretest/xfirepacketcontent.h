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

#ifndef __XFIREPACKETCONTENT_H
#define __XFIREPACKETCONTENT_H

#include "monitoredobj.h"

namespace xfirelib {
  struct Client;

  class XFirePacketContent : public MonitoredObj {
  public:
    XFirePacketContent();
    virtual ~XFirePacketContent() { }
    virtual XFirePacketContent* newPacket() = 0;

    virtual int getPacketContent(char *buf) = 0;
    virtual int getPacketId() = 0;
    virtual int getPacketAttributeCount() = 0;
    virtual int getPacketSize() = 0;
    virtual void parseContent(char *buf, int length, int numberOfAtts) = 0;
  };

};


#endif
