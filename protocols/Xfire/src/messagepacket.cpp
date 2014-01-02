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

#include "messagepacket.h"
#include "xfireparse.h"
#include "variablevalue.h"
#include <iostream>
#include <string>
#include "xdebug.h"


namespace xfirelib {
  using namespace std;

    MessagePacket::MessagePacket(){
        packetID = 133;
    }

int MessagePacket::getPacketContent(char *packet){
    memcpy(packet,buf,bufLength);
    packetID = 2;
    return 150;
}
  void MessagePacket::parseContent(char *buf, int length, int numberOfAtts) {
     
    bufLength = length;
    XINFO(( "Got IM\n" ));
    
    int index = 0;
    sid = new VariableValue();
    peermsg = new VariableValue();
    msgtype = new VariableValue();

    index += sid->readName(buf,index);
    index++; //ignore 03
    index += sid->readValue(buf,index,16);

    index += peermsg->readName(buf,index);
    index++;
    index++;
    index += msgtype->readName(buf,index);
    index++;
    index += msgtype->readValue(buf,index,4);

    if (msgtype->getValue()[0] == 0){
        imindex = new VariableValue();
        index += imindex->readName(buf,index);

        VariableValue messageTemp;
        index++;//ignore 02
        index += imindex->readValue(buf,index,4);
        index += messageTemp.readName(buf,index);
        index++;
        index += messageTemp.readValue(buf,index,2); 
        int messageLength = messageTemp.getValueAsLong();
        index = messageTemp.readValue(buf,index,messageLength);

        for(int i = 0; i < messageTemp.getValueLength();i++){
            message += messageTemp.getValue()[i];
        }
        /*TODO: implement this and answer the package*/
    }else if (msgtype->getValue()[0] == 1){
        cout << "got ack for a message we have sent" << endl;
    }else if (msgtype->getValue()[0] == 2){
       memcpy(this->buf,buf,150);
        /*answer the packet*/
        cout << "some auth magic stuff" << length << endl;
    } 
  }

};
