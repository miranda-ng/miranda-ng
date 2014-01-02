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

#include "recvprefspacket.h"
#include "variablevalue.h"
#include "xdebug.h"

namespace xfirelib {

  void RecvPrefsPacket::parseContent(char *buf, int length, int numberOfAtts) {
	int index = 0;

	memset(&this->config,0,sizeof(xfire_prefitem)*XFIRE_RECVPREFSPACKET_MAXCONFIGS);

    XDEBUG("This packet gives us the preferences that you can set with the official client.\n");

	//skip 4c,09
	index+=2;

	//anzahl an konfigurationen welche abgestellt wurden
	int anz=buf[index];
	XDEBUG2("%d disabled Configs\n",anz);

	index++;

	for(int i=0;i<anz;i++)
	{
		int kid=buf[index];
		XDEBUG2("Konfigid: %d\n",kid);

		if (kid<XFIRE_RECVPREFSPACKET_MAXCONFIGS&&kid>0)
		{
			config[kid].wasset=1;
			for(int j=0;j<4;j++)
			{
				index++;
				config[kid].data[j]=buf[index];
			}
		}

		XDEBUG2("Konfigval: %d\n",buf[index]);
		index++;
	}


  }

  //prefences packet vorbereiten
  int PrefsPacket::getPacketContent(char *buf) {
    int index = 0;

	buf[index++]=5;
	buf[index++]='p';
	buf[index++]='r';
	buf[index++]='e';
	buf[index++]='f';
	buf[index++]='s';

	buf[index++]=9;

	//anzahlindex zwischenspeichern
	int anzindex=index;
	index++;

	int anz=0;
	for(int j=0;j<XFIRE_RECVPREFSPACKET_MAXCONFIGS;j++)
	{
		if (this->config[j].wasset==1)
		{
			buf[index++]=j;
			buf[index++]=this->config[j].data[0];
			buf[index++]=this->config[j].data[1];
			buf[index++]=this->config[j].data[2];
			buf[index++]=this->config[j].data[3];
			anz++;
		}
	}

	buf[anzindex]=anz;

	return index;
  }


};
