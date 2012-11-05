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

#include "../xdebug.h"


#include <iostream>
#include <string>
#include <stdexcept>

#include "test.h"
#include "../client.h"
#include "../xfirepacket.h"
#include "../loginfailedpacket.h"
#include "../otherloginpacket.h"
#include "../messagepacket.h"
#include "../sendstatusmessagepacket.h"
#include "../sendmessagepacket.h"
#include "../invitebuddypacket.h"
#include "../sendacceptinvitationpacket.h"
#include "../senddenyinvitationpacket.h"
#include "../sendremovebuddypacket.h"
#include "../sendnickchangepacket.h"
#include "../sendgamestatuspacket.h"
#include "../sendgamestatus2packet.h"
#include "../dummyxfiregameresolver.h"
#include "../sendgameserverpacket.h"
#include "../recvoldversionpacket.h"

namespace xfirelibtest {
  using namespace std;
  using namespace xfirelib;

  XFireTestClient::XFireTestClient(string username_,string password_) 
    : username(username_), password(password_) {
    client = new Client();
    client->setGameResolver( new DummyXFireGameResolver() );
    lastInviteRequest = NULL;
  }
  XFireTestClient::~XFireTestClient() {
    delete client;
    delete lastInviteRequest;
  }

  void XFireTestClient::run() {
    client->connect(username,password);
    client->addPacketListener(this);
    // Starting input loop ...
    string input;
    bool done = false;
    while(!done) {
      cout << "Give me Your Commands:\n>>> ";
      getline(cin,input);
      //string words[10];
      //split(input, words, 10, " ");
      vector<string> cmds = explodeString(input," ");
      //cout << "Command: " << cmds[0];
      //XDEBUG(( "Input: %s %d Command: %s\n", input.c_str(), cmds.size(), cmds[0].c_str() ));
      if(cmds.size() > 0) {
	if(cmds[0] == "help") {
	  cout << "No help available right now :(" << endl;
	  cout << "But a short list of available commands: " << endl;
	  cout << "  quit" << endl;
	  cout << "  buddylist" << endl;
	  cout << "  away" << endl;
	  cout << "  back" << endl;
	  cout << "  game <gameid>" << endl;
	  cout << "  game2 <game2id>" << endl;
	  cout << "  nick" << endl;
	  cout << "  invite <username>" << endl;
	  cout << "  accept [<username>]" << endl;
	  cout << "  deny [<username>]" << endl;
	  cout << "  remove <username>" << endl;
	  cout << "  send <username> <message .....>" << endl;
	} else if(cmds[0] == "quit") {
	  cout << "Quitting" << endl;
	  done = true;
	} else if(cmds[0] == "buddylist") {
	  printBuddyList();
	} else if(cmds[0] == "away" || cmds[0] == "back") {
	  SendStatusMessagePacket *packet = new SendStatusMessagePacket();
	  packet->awaymsg = joinString(cmds,1);//input.substr(5);
	  client->send( packet );
	  delete packet;
	} else if(cmds[0] == "game" || cmds[0] == "game2") {
	  int gameid = 0;
	  if(cmds.size() > 1) {
	    gameid = atoi(cmds[1].c_str());
	  }
	  SendGameStatusPacket *packet = 
	    (cmds[0] == "game" ? new SendGameStatusPacket() :
	     new SendGameStatus2Packet());
	  packet->gameid = 2;
	  packet->gameid = gameid;
          char ip[] = {0,0,0,0};
          memcpy(packet->ip,ip,4);
          packet->port = 0;
	  client->send( packet );
	  delete packet;
	} else if(cmds[0] == "nick"){
	  if(cmds.size() < 2) {
	    cout << "Usage: nick <nickname>" << endl;
	    continue;
	  }
	  SendNickChangePacket nick;
          nick.nick = joinString(cmds,1);
	  client->send( &nick );
	  cout << "Sent nick change." << endl;
	}else if(cmds[0] == "send"){
	  if(cmds.size() < 3) {
	    cout << "Usage: send <username> <message>" << endl;
	    continue;
	  }
	  SendMessagePacket msg;
	  msg.init(client, cmds[1], joinString(cmds,2));
	  client->send( &msg );
	  cout << "Sent message." << endl;
	} else if(cmds[0] == "invite") {
	  if(cmds.size() < 3) {
	    cout << "Usage: invite <username> <message>" << endl;
	  } else {
	    InviteBuddyPacket invite;
	    invite.addInviteName( cmds[1], joinString(cmds,2) );
	    client->send( &invite );
	    cout << "Sent Invitation." << endl;
	  }
	} else if(cmds[0] == "accept" || cmds[0] == "deny") {
	  string name;
	  if(cmds.size() < 2) {
	    if(lastInviteRequest == 0) {
	      cout << "No Invitation Request pending (try " << cmds[0] << " <username>)" << endl;
	      continue;
	    }
	    name = string(*lastInviteRequest);
	    delete lastInviteRequest;
	    lastInviteRequest = 0;
	  } else {
	    name = cmds[1];
	  }
	  if(cmds[0] == "accept") {
	    SendAcceptInvitationPacket accept;
	    accept.name = name;
	    client->send( &accept );
	    cout << "Sent Accept Invitation." << endl;
	  } else {
	    SendDenyInvitationPacket deny;
	    deny.name = name;
	    client->send( &deny );
	    cout << "Sent Deny Invitation." << endl;
	  }
	} else if(cmds[0] == "remove") {
	  if(cmds.size() < 2) {
	    cout << "Usage: remove <user name>" << endl;
	    continue;
	  }
	  string name = cmds[1];
	  BuddyListEntry *entry = client->getBuddyList()->getBuddyByName( name );
	  if(entry == NULL) {
	    cout << "Noone with this name is in your buddy list ?" << endl;
	    continue;
	  }
	  SendRemoveBuddyPacket removeBuddy;
	  removeBuddy.userid = entry->userid;
	  client->send( &removeBuddy );
	  cout << "Sent Remove Buddy." << endl;
	}
	
	else {
	  cout << "Unknown Command." << endl;
	}
      }
    }
    client->disconnect();
    delete client;
    client = 0;
  }
  string XFireTestClient::joinString(vector<string> s, int startindex, int endindex, string delimiter) {
    string ret;
    int max = (unsigned)endindex > s.size() || endindex < 0 ? s.size() : endindex;
    for(int i = startindex ; i < max ; i++) {
      if(i != startindex)
	ret += delimiter;
      ret += s[i];
    }
    return ret;
  }
  vector<string> XFireTestClient::explodeString(string s, string e) {
    vector<string> ret;
    int iPos = s.find(e, 0);
    int iPit = e.length();
    while(iPos > -1) {
      if(iPos != 0)
	ret.push_back(s.substr(0,iPos));
      s.erase(0,iPos+iPit);
      iPos = s.find(e,0);
    }
    if(s!="")
      ret.push_back(s);
    return ret;
  }

  void XFireTestClient::receivedPacket(XFirePacket *packet) {
    XFirePacketContent *content = packet->getContent();
    cout << "TestClient :  Received Packet: " << content->getPacketId() << endl;
    switch(content->getPacketId()) {
    case XFIRE_LOGIN_FAILED_ID: {
      client->disconnect();
      delete client;
      client = 0;
      cout << "TestClient : Login failed." << endl;
      break;
    }
    case XFIRE_MESSAGE_ID: {
      cout << "TestClient : Got Message." << endl;
        if( (( MessagePacket*)content)->getMessageType() == 0){
            cout << "TestClient : Message is " << ((MessagePacket*)content)->getMessage() <<endl;
	    BuddyListEntry *entry = client->getBuddyList()->getBuddyBySid( ((MessagePacket*)content)->getSid() );
            cout << "TestClient : from user " << entry->nick << "(" << entry->username << ")" <<endl;
        }
      break;
    }
    case XFIRE_RECV_OLDVERSION_PACKET_ID: {
        cout << "Testclient: Our protocol version is too old" << endl;
        break;
    }
    case XFIRE_PACKET_INVITE_REQUEST_PACKET: {
      cout << "Invitation Request: " << endl;
      InviteRequestPacket *invite = (InviteRequestPacket*)content;
      cout << "  Name   :  " << invite->name << endl;
      cout << "  Nick   :  " << invite->nick << endl;
      cout << "  Message:  " << invite->msg << endl;
      cout << "     -- type 'accept' for accepting this request. (or 'accept "
	   << invite->name << "' if you receive another invitation in the meantime." << endl;
      lastInviteRequest = new string(invite->name);
      break;
    }
    case XFIRE_OTHER_LOGIN:{
      client->disconnect();
      delete client;
      client = 0;
      cout << "TestClient : Someone loged in with our account.disconnect" << endl;
      break;
    }
    case XFIRE_BUDDYS_NAMES_ID: {
      printBuddyList();
    }
default:
cout << "nothing--------" << endl;
break;
    }
}

  void XFireTestClient::printBuddyList() {
      printf("Buddy List: (* marks online users)\n");
      printf("----------------- Buddy List --------------------------------------------------------\n");
      printf("  %20s | %20s | %10s | %20s | %7s | %7s\n","User Name", "Nick", "UserId", "Status Msg" ,"Gameid" ,"Gameid2" );
      vector<BuddyListEntry*> *entries = client->getBuddyList()->getEntries();
      for(uint i = 0 ; i < entries->size() ; i ++) {
	BuddyListEntry *entry = entries->at(i);
	printf("%1s %20s | %20s | %10ld | %20s | %7ld | %ld\n",
	       (entry->isOnline() ? "*" : ""),
	       entry->username.c_str(),
	       entry->nick.c_str(),
	       entry->userid,
	       entry->statusmsg.c_str(),
               entry->game,
                entry->game2);
      }
      printf("-------------------------------------------------------------------------------------\n\n");
  }
};

using namespace std;
using namespace xfirelib;
using namespace xfirelibtest;

int main(int argc, char **params) {
  if(argc < 3) {
    cout << "Usage: " << params[0] << " <username> <password>" << endl;
    return 0;
  }
  XFireTestClient *testClient = new 
  XFireTestClient(string(params[1]),string(params[2]));
  try {
    testClient->run();
  } catch( exception e ) {
    cout << "Exception: " << e.what() << endl;
  }
  delete testClient;
  return 0;
}
