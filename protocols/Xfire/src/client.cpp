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
#include "xdebug.h"
#include "client.h"
#include "clientinformationpacket.h"
#include "clientversionpacket.h"
#include "xfirepacket.h"
#include "authpacket.h"
#include "clientloginpacket.h"
#include "buddylistnamespacket.h"
#include "messagepacket.h"
#include "sendmessagepacket.h"
#include "messageackpacket.h"
#include "recvoldversionpacket.h"
#include "sendkeepalivepacket.h"

#include "baseProtocol.h"

#include <iostream>

//#define XFIRE_HOST "cs.xfire.com"
#define XFIRE_HOST "206.220.42.147"
#define XFIRE_PORT 25999

extern BOOL mySleep(int ms,HANDLE evt);
extern HANDLE hConnectionClose;
extern WINBASEAPI
BOOL
WINAPI
ResetEvent(
    __in HANDLE hEvent
    );

//#define UINT_8 unsigned char
//#define UINT_32 unsigned long

namespace xfirelib {

using namespace std; 

  Client::Client() {
    XDEBUG(("Client constructor...\n"));
    gameResolver = NULL;
    packetReader = new PacketReader(NULL);
    packetReader->addPacketListener( this );
    buddyList = new BuddyList( this );
	socket=NULL;
#ifndef NO_PTHREAD
	sendpingthread.p=NULL;
	readthread.p=NULL;
#endif
  }
  
  Client::~Client(){
    XDEBUG(("Client destructor...\n"));
    delete username;
    delete password;
    delete buddyList;
    delete packetReader;
    delete socket;
  }

  void Client::connect( string username, string password, int useproxy,  string proxyip, int proxyport) {
    try {
	  this->gotBudduyList=FALSE;
      this->username = new string(username);
      this->password = new string(password);
      socket = new Socket( XFIRE_HOST, XFIRE_PORT,useproxy,proxyip,proxyport );

	  //bevors losgeht, erstmal die localaddr sichern
	  struct sockaddr_in sa;
	  int iLen = sizeof(sa);
      getsockname(socket->m_sock, (SOCKADDR*)&sa, &iLen);
	  mir_strncpy(this->localaddr,inet_ntoa(sa.sin_addr), sizeof(this->localaddr)-1);
	  this->llocaladdr=inet_addr(this->localaddr);

      packetReader->setSocket(socket);

	  ResetEvent(hConnectionClose);
      
	  startThreads();
      //packetReader->startListening();


      socket->send("UA01");
      XDEBUG(("Sent UA01\n"));
      ClientInformationPacket *infoPacket = new ClientInformationPacket();
      this->send( infoPacket );
      delete infoPacket;
      XINFO(("sent ClientInformationPacket\n"));

      ClientVersionPacket *versionPacket = new ClientVersionPacket();
      versionPacket->setProtocolVersion( protocolVersion);
      this->send( versionPacket );
      delete versionPacket;

      XINFO(("sent ClientVersionPacket\n"));
	  this->connected=TRUE;
    } catch( SocketException ex ) {
      XERROR(("Socket Exception ?! %s \n",ex.description().c_str() ));
	  this->connected=FALSE;
    }
  }
  XFireGameResolver *Client::getGameResolver() {
    return gameResolver;
  }
  void Client::startThreads() {
    XINFO(("About to start thread\n"));
#ifndef NO_PTHREAD
    void* (*func)(void*) = &xfirelib::Client::startReadThread;
    pthread_create( &readthread, NULL, func, (void*)this );
    void* (*func2)(void*) = &xfirelib::Client::startSendPingThread;
    pthread_create( &sendpingthread, NULL, func2, (void*)this );
#else
	//mir_create!!!!
	mir_forkthread(xfirelib::Client::startReadThread,(LPVOID)this);
	mir_forkthread(xfirelib::Client::startSendPingThread,(LPVOID)this);
#endif
  }
#ifndef NO_PTHREAD
  void *Client::startReadThread(void *ptr) {
#else
  void Client::startReadThread(LPVOID lParam) {
	  void* ptr=(void*)lParam;
#endif
	if (ptr==NULL||((Client*)ptr)->packetReader==NULL)
#ifndef NO_PTHREAD
		return NULL;
#else
		return;
#endif
    try {
      ((Client*)ptr)->packetReader->run();
    } catch (SocketException ex) {
      XERROR(("Socket Exception ?! %s \n",ex.description().c_str() ));

	  //miranda bescheid geben, wir haben verbindung verloren
	  if (ptr==NULL||((Client*)ptr)->connected) SetStatus(ID_STATUS_OFFLINE,NULL);

      //((Client*)ptr)->disconnect();
    }
#ifndef NO_PTHREAD
	return NULL;
#else
	return;
#endif
  }

#ifndef NO_PTHREAD
  void *Client::startSendPingThread(void *ptr) {
    Client *me = (Client*)ptr;
#else
  void Client::startSendPingThread(LPVOID lParam) {
    Client *me = (Client*)lParam;
#endif
    SendKeepAlivePacket packet;

    while(1) {
#ifndef NO_PTHREAD
	  pthread_testcancel();
#endif
      //Sleep(60000); // Sleep for 40 sek
	  if (mySleep(60000,hConnectionClose))
	  {
#ifndef NO_PTHREAD
		  return NULL;
#else
		  return;
#endif
	  }
#ifndef NO_PTHREAD
	  pthread_testcancel();
#endif
      XDEBUG(( "Sending KeepAlivePacket\n" ));
      if (!me->send( &packet )) {
	XINFO(( "Could not send KeepAlivePacket... exiting thread.\n" ));
	break;
      }
    }
#ifndef NO_PTHREAD
	return NULL;
#else
	return;
#endif
  }

  void Client::disconnect() {
	this->connected=FALSE;

	//socket vom packetreader auf NULL, damit die readschleife geschlossen wird
	if (this->packetReader!=NULL)
		this->packetReader->setSocket(NULL);

	XDEBUG( "cancelling readthread... \n");
#ifndef NO_PTHREAD
	if (readthread.p!=NULL) pthread_cancel (readthread);
	readthread.p=NULL;

	XDEBUG( "cancelling sendpingthread... \n");
	if (sendpingthread.p!=NULL) pthread_cancel (sendpingthread);
	sendpingthread.p=NULL;
#endif

    XDEBUG( "deleting socket...\n" );
    if (socket){
        delete socket;
        socket = NULL;
    }
    XDEBUG(( "done\n" ));
  }

  bool Client::send( XFirePacketContent *content ) {
    if (!socket) {
      XERROR(( "Trying to send content packet altough socket is NULL ! (ignored)\n" ));
      return false;
    }
    XFirePacket *packet = new XFirePacket(content);
    packet->sendPacket( socket );
    delete packet;
    return true;
  }

  void Client::addPacketListener( PacketListener *listener ) {
    packetReader->addPacketListener( listener );
  }


  void Client::receivedPacket( XFirePacket *packet ) {
    XDEBUG(("Client::receivedPacket\n"));
    if ( packet == NULL ) {
      XERROR(("packet is NULL !!!\n"));
      return;
    }
    if ( packet->getContent() == NULL ) {
      XERROR(("ERRRR getContent() returns null ?!\n"));
      return;
    }
    XFirePacketContent *content = packet->getContent();

    switch( content->getPacketId() ) {
    case XFIRE_PACKET_AUTH_ID: {
      XINFO(("Got Auth Packet .. Sending Login\n"));
      AuthPacket *authPacket = (AuthPacket*)packet->getContent();

      ClientLoginPacket *login = new ClientLoginPacket();
      login->setSalt( authPacket->getSalt() );
      login->setUsername( *username );
      login->setPassword( *password );
      send( login );
      delete login;
      break;
    }

    case XFIRE_MESSAGE_ID: {
      XDEBUG(( "Got Message, sending ACK\n" ));
        MessagePacket *message = (MessagePacket*)packet->getContent();
        if (message->getMessageType() == 0){
            MessageACKPacket *ack = new MessageACKPacket();
            memcpy(ack->sid,message->getSid(),16);
            ack->imindex = message->getImIndex();
            send( ack );
	    delete ack;
        }else if (message->getMessageType() == 2){
            send(message);
        }
	break;
    }

    default:
      //cout << "Nothing here... " << endl;
      break;
    }

  }
};
