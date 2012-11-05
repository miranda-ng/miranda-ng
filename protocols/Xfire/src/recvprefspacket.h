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

#ifndef __RECVPREFSPACKET_H
#define __RECVPREFSPACKET_H

#include "xfirerecvpacketcontent.h"
#include "xfiresendpacketcontent.h"

#define XFIRE_RECVPREFSPACKET 141
#define XFIRE_SENDPREFSPACKET 0xa

#define XFIRE_RECVPREFSPACKET_MAXCONFIGS 32
#define XFIRE_RECVPREFSPACKET_SUPPORTEDONFIGS 7

//zum zwischenspeichern, wird später benötigt zum versenden
struct xfire_prefitem {
	unsigned char data[4];
	unsigned char wasset;
};

//für die optionen
struct xfireconfigitem {
	unsigned long id;
	char xfireconfigid;
	char dbentry[256];
};

namespace xfirelib {

  //packet mit den preferences
  class RecvPrefsPacket : public XFireRecvPacketContent {
  public:
    virtual ~RecvPrefsPacket() { }
    int getPacketId() { return XFIRE_RECVPREFSPACKET; }

    XFirePacketContent *newPacket() { return new RecvPrefsPacket(); }
    void parseContent(char *buf, int length, int numberOfAtts);

	//array vorbereiten, eventuell auf 32 bytes aufstocken, man weis ja nie was kommt
	xfire_prefitem config[XFIRE_RECVPREFSPACKET_MAXCONFIGS];
  };

  //packet zum setzen neuer preferences
  class PrefsPacket : public XFireSendPacketContent {
   public:
    virtual ~PrefsPacket() { }

    XFirePacketContent *newPacket() { return new PrefsPacket; }
    int getPacketId() { return XFIRE_SENDPREFSPACKET; }

    int getPacketContent(char *buf);
    int getPacketAttributeCount() { return 1; }
    int getPacketSize() { return 1024; }

    xfire_prefitem config[XFIRE_RECVPREFSPACKET_MAXCONFIGS];
  };

};


#endif
