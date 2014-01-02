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

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "xfirepacket.h"

#include "xdebug.h"

#include "socketexception.h"

namespace xfirelib {
  using namespace std;

  XFirePacket::XFirePacket(PacketReader *reader) {
    this->reader = reader;
    this->content = NULL;
  }
  XFirePacket::XFirePacket(XFirePacketContent *content) {
    this->content = content;
    this->reader = NULL;
  }

  XFirePacket::~XFirePacket() {
  }


  void XFirePacket::recvPacket(Socket *socket) {
    char buf[5];
	//char temp[255];
	int nlen=0;
	//FILE* f2;

	if (socket==NULL) return;

    int r = socket->recv( buf, 5 );
	int missingbytes=5-r;
	if (missingbytes != 0) {
		XERROR3("Returned only %d bytes try last %d bytes again ?!: \n",r,missingbytes);
		r+= socket->recv( &buf[r], missingbytes );
	}
    if (r != 5) {
      XERROR2("Returned less than 5 ?!: %d\n",r);
      throw SocketException("Connection Closed ?");
    }
    XDEBUG2("Read %d bytes...\n", r );
    unsigned int len = ((unsigned char)buf[0]) + (((unsigned char)buf[1]) * 256);

    int packetid = buf[2];    
    int numberOfAtts = (unsigned char)buf[4];

    char* contentbuf=new char[len-5];
    XDEBUG2("calling socket->recv(contentbuf,%d)\n",len-5);
    int r2 = socket->recv( contentbuf, len-5 );


	char* ncontentbuf=contentbuf+r2;
	int nsize=len-5-r2;
	int dummyi=0;
	//packet solang auslesen, bis fertig - dufte
	while(nsize>0)
	{
		dummyi++;
		XDEBUG2("nsize: %d\n", nsize);
		r2=socket->recv( ncontentbuf, nsize );
		ncontentbuf=ncontentbuf+r2;
		nsize-=r2;
		XDEBUG5("packetid: %d numberOfAtts: %d length: %d Got: %d\n", (unsigned char)packetid,numberOfAtts,len-5,len-5-nsize);
	}

    XDEBUG5("packetid: %d numberOfAtts: %d length: %d Got: %d\n", (unsigned char)packetid,numberOfAtts,len-5,r2);

    if (r2 < 1) return;

	/*if ((unsigned char)packetid==131)
	{
		sprintf(temp,"packet%d_%d.dmp",(unsigned char)packetid,rand());
		f2=fopen(temp,"wb");
		fwrite(contentbuf,1,len-5,f2);
		fclose(f2);
	}*/

    XFirePacketContent *contentClass = reader->getPacketContentClass( (unsigned char)packetid );

	//sprintf(temp,"packet%d.dmp",(unsigned char)packetid);

    if (contentClass == NULL) {
		/*sprintf(temp,"packet%d.dmp",(unsigned char)packetid);
		f2=fopen(temp,"wb");
		fwrite(contentbuf,1,len-5,f2);
		fclose(f2);*/
		XERROR2("NO SUCH CONTENT PACKET (%d) :(\n", (unsigned char)packetid);
      return;
    }
    XDEBUG(("Creating ContentClass Instance\n"));
    content = contentClass->newPacket();

	XDEBUG(("Parse Content\n"));
	//statt r2 wird jetzt die komplette länge des packets genommen, also len-5 dufte 
    content->parseContent(contentbuf, len-5, numberOfAtts);
  }

  void XFirePacket::sendPacket(Socket *socket) {
    int size = content->getPacketSize();
    char *buf = (char*)malloc(size * sizeof(char));
    XDEBUG3("Allocated %d characters (%d)\n", size, sizeof(char));
    int rsize = content->getPacketContent( buf );
    XDEBUG2("Real Size: %d\n", rsize);
    int realsize = rsize + 5;
    char *sendbuf = (char*)malloc(realsize * sizeof(char));
    sendbuf[0] = realsize % 256;
    sendbuf[1] = (int)realsize / 256;
    sendbuf[2] = content->getPacketId();
	XDEBUG2("Send Content Id: %d\n", content->getPacketId());
    sendbuf[3] = 0;
    sendbuf[4] = content->getPacketAttributeCount();
    memcpy( sendbuf + 5, buf, rsize );

    socket->send( sendbuf, realsize );

    free(buf); free(sendbuf);
  }
};
