/*
    ICQ Corporate protocol plugin for Miranda IM.
    Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef protocol_h
#define protocol_h

///////////////////////////////////////////////////////////////////////////////


#define ICQ_TCP_VER                     0x0002
#define ICQ_UDP_VER                     0x0003

// UDP commands
const unsigned short ICQ_CMDxRCV_SETxOFFLINE        = 0x0028;
const unsigned short ICQ_CMDxRCV_LOGIN_ERR          = 0x0370;
const unsigned short ICQ_CMDxRCV_ACK                = 0x000A;
const unsigned short ICQ_CMDxRCV_HELLO              = 0x005A;
const unsigned short ICQ_CMDxRCV_WRONGxPASSWD       = 0x0064;
const unsigned short ICQ_CMDxRCV_USERxONLINE        = 0x006E;
const unsigned short ICQ_CMDxRCV_USERxOFFLINE       = 0x0078;
const unsigned short ICQ_CMDxRCV_SEARCHxFOUND       = 0x008C;
const unsigned short ICQ_CMDxRCV_SEARCHxDONE        = 0x00A0;
const unsigned short ICQ_CMDxRCV_SYSxMSGxOFFLINE    = 0x00DC;
const unsigned short ICQ_CMDxRCV_SYSxMSGxONLINE     = 0x0104;
const unsigned short ICQ_CMDxRCV_SYSxMSGxDONE       = 0x00E6;
const unsigned short ICQ_CMDxRCV_BROADCASTxMULTI    = 0x0366;
const unsigned short ICQ_CMDxRCV_BROADCASTxOFFLINE  = 0x038E;
const unsigned short ICQ_CMDxRCV_BROADCASTxONLINE   = 0x03A2;
const unsigned short ICQ_CMDxRCV_BROADCASTxDONE     = 0x0398;
const unsigned short ICQ_CMDxRCV_ERROR              = 0x00F0;
const unsigned short ICQ_CMDxRCV_BUSY               = 0x00FA;
const unsigned short ICQ_CMDxRCV_USERxBASICxINFO    = 0x0118;
const unsigned short ICQ_CMDxRCV_USERxINFO          = 0x02e4;
const unsigned short ICQ_CMDxRCV_USERxWORKxINFO     = 0x02F8;
const unsigned short ICQ_CMDxRCV_USERxWORKxPAGE     = 0x030C;
const unsigned short ICQ_CMDxRCV_USERxHOMExINFO     = 0x0320;
const unsigned short ICQ_CMDxRCV_USERxHOMExPAGE     = 0x0334;

const unsigned short ICQ_CMDxRCV_USERxSTATUS        = 0x01A4;
const unsigned short ICQ_CMDxRCV_USERxINVALIDxUIN   = 0x02EE;
const unsigned short ICQ_CMDxRCV_USERxLISTxDONE     = 0x021C;

const unsigned short ICQ_CMDxRCV_SYSxMSG            = 0x0001;
const unsigned short ICQ_CMDxRCV_SYSxURL            = 0x0004;
const unsigned short ICQ_CMDxRCV_SYSxAUTHxREQ       = 0x0006;
const unsigned short ICQ_CMDxRCV_SYSxAUTHxGRANTED   = 0x0008;
const unsigned short ICQ_CMDxRCV_SYSxADDED          = 0x000C;
const unsigned short ICQ_CMDxRCV_SYSxBROADCAST      = 0x0014;

const unsigned short ICQ_CMDxSND_ACK                = 0x000A;
const unsigned short ICQ_CMDxSND_SEARCHxSTART       = 0x05c8;
const unsigned short ICQ_CMDxSND_THRUxSERVER        = 0x010E;
const unsigned short ICQ_CMDxSND_PING               = 0x042E;
const unsigned short ICQ_CMDxSND_LOGON              = 0x03E8;
const unsigned short ICQ_CMDxSND_LOGOFF             = 0x0438;
const unsigned short ICQ_CMDxSND_SYSxMSGxDONExACK   = 0x0442;
const unsigned short ICQ_CMDxSND_SYSxMSGxREQ        = 0x044C;
const unsigned short ICQ_CMDxSND_BROADCASTxREQ      = 0x0604;
const unsigned short ICQ_CMDxSND_MULTI              = 0x05D2;
const unsigned short ICQ_CMDxSND_USERxGETxINFO      = 0x05FA;
const unsigned short ICQ_CMDxSND_USERxGETxBASICxINFO = 0x0460;
const unsigned short ICQ_CMDxSND_USERxADD           = 0x053C;
const unsigned short ICQ_CMDxSND_SETxSTATUS         = 0x04D8;
const unsigned short ICQ_CMDxSND_USERxLIST          = 0x0406;
const unsigned short ICQ_CMDxSND_INVISxLIST         = 0x06A4;
const unsigned short ICQ_CMDxSND_VISxLIST           = 0x06AE;
const unsigned short ICQ_CMDxSND_UPDATExLIST        = 0x06B8;
const unsigned short ICQ_CMDxSND_AUTHORIZE          = 0x0456;

const unsigned short int ICQ_CMDxSND_PING2          = 0x051E;

// TCP commands
const unsigned short ICQ_CMDxTCP_START              = 0x07EE;
const unsigned short ICQ_CMDxTCP_CANCEL             = 0x07D0;
const unsigned short ICQ_CMDxTCP_ACK                = 0x07DA;

const unsigned short ICQ_CMDxTCP_MSG                = 0x0001;
const unsigned short ICQ_CMDxTCP_CHAT               = 0x0002;
const unsigned short ICQ_CMDxTCP_FILE               = 0x0003;
const unsigned short ICQ_CMDxTCP_URL                = 0x0004;
const unsigned short ICQ_CMDxTCP_READxAWAYxMSG      = 0x03E8;
const unsigned short ICQ_CMDxTCP_READxOCCUPIEDxMSG  = 0x03E9;
const unsigned short ICQ_CMDxTCP_READxNAxMSG        = 0x03EA;
const unsigned short ICQ_CMDxTCP_READxDNDxMSG       = 0x03EB;
const unsigned short ICQ_CMDxTCP_READxFREECHATxMSG  = 0x03EC;
const unsigned short ICQ_CMDxTCP_HANDSHAKE          = 0x03FF;
const unsigned short ICQ_CMDxTCP_HANDSHAKE2         = 0x04FF;
const unsigned short ICQ_CMDxTCP_HANDSHAKE3         = 0x02FF;

// status constants
const unsigned short int ICQ_STATUS_OFFLINE         = 0xFFFF;
const unsigned short int ICQ_STATUS_ONLINE          = 0x0000;
const unsigned short int ICQ_STATUS_AWAY            = 0x0001;
const unsigned short int ICQ_STATUS_NA              = 0x0005;
const unsigned short int ICQ_STATUS_OCCUPIED        = 0x0011;
const unsigned short int ICQ_STATUS_DND             = 0x0013;
const unsigned short int ICQ_STATUS_FREECHAT        = 0x0020;
const unsigned short int ICQ_STATUS_PRIVATE         = 0x0100;

// miscellaneous constants
const unsigned short int ICQ_VERSION                = 0x0003;
const unsigned short MAX_MESSAGE_SIZE               = 450;
const unsigned short INT_VERSION                    = 40;
const unsigned short PING_FREQUENCY                 = 45000;
const unsigned short DEFAULT_SERVER_PORT            = 4000;
const unsigned short MAX_SERVER_RETRIES             = 3;
const unsigned short MAX_WAIT_ACK                   = 10000;
const unsigned long LOCALHOST                       = 0x0100007F;


const unsigned int WM_NETEVENT_SERVER               = WM_USER + 1;
const unsigned int WM_NETEVENT_CONNECTION           = WM_USER + 2;
const unsigned int WM_NETEVENT_USER                 = WM_USER + 3;
const unsigned int WM_NETEVENT_TRANSFER             = WM_USER + 4;

///////////////////////////////////////////////////////////////////////////////

typedef struct {   //extended search result structure, used for all searches
    PROTOSEARCHRESULT hdr;
    DWORD uin;
    BYTE auth;
} ICQSEARCHRESULT;

///////////////////////////////////////////////////////////////////////////////

class ICQ
{
public:
    ICQ();
    bool load();
    void unload();

    bool setStatus(unsigned short newStatus);
    ICQEvent *sendMessage(ICQUser *u, char *m);
    ICQEvent *sendUrl(ICQUser *u, char *url);
    ICQEvent *sendReadAwayMsg(ICQUser *u);
    ICQTransfer *sendFile(ICQUser *u, char *description, char *filename, unsigned int size, char **files);

    bool logon(unsigned short logonStatus);
    void logoff(bool reconnect);
    void ping();
    unsigned short processUdpPacket(Packet &packet);
    void doneEvent(bool gotAck, int hSocket, int sequence);
    void cancelEvent(ICQEvent *&);
    void ackUDP(unsigned short theSequence);
    ICQEvent *sendICQ(Socket &socket, Packet &packet, unsigned short cmd, unsigned long sequence, unsigned long uin = 0, unsigned short subCmd = 0, int reply = 0);
    ICQUser *getUserByUIN(unsigned long uin, bool allowAdd = true);
	ICQUser *getUserByContact(MCONTACT hContact);
    void requestSystemMsg();
    void requestBroadcastMsg();
    void updateContactList();
    void sendVisibleList();
    void sendInvisibleList();
    void updateUserList(ICQUser *u, char list, char add);
    void processSystemMessage(Packet &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
    void ackSYS(unsigned int timeStamp);
    ICQUser *addUser(unsigned int uin, bool peristent = true);
    void addNewUser(ICQUser *u);
    void removeUser(ICQUser *u);
    void startSearch(unsigned char skrit, unsigned char smode, char *sstring, unsigned int s);


    bool getUserInfo(ICQUser *u, bool basicInfo);
    void authorize(unsigned int uinToAuthorize);
    void processTcpPacket(Packet &packet, unsigned int hSocket);
    void ackTCP(Packet &packet, ICQUser *u, unsigned short newCommand, unsigned int theSequence);
    void recvUDP(int);
    void recvNewTCP(int);
    void recvTCP(SOCKET hSocket);
    void recvTransferTCP(SOCKET hSocket);

    void addMessage(ICQUser *u, char *m, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t = 0);
    void addUrl(ICQUser *u, char *m, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t = 0);
    void addAwayMsg(ICQUser *u, char *m, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t = 0);
    void addFileReq(ICQUser *u, char *m, char *filename, unsigned long size, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t = 0);
    void doneUserFcn(bool ack, ICQEvent *icqEvent);

    void acceptFile(ICQUser *u, unsigned long hTransfer, char *path);
    void refuseFile(ICQUser *u, unsigned long hTransfer, char *reason);

    TCPSocket tcpSocket;
    UDPSocket udpSocket;
    unsigned short sequenceVal, searchSequenceVal;
    unsigned int uin, tcpSequenceVal;
    unsigned short desiredStatus;
    unsigned short statusVal;
    unsigned int pingTimer;
//    int eventThread;
    HWND hWnd;
    char *awayMessage;
    unsigned int timeStampLastMessage;
private:
    ICQEvent *send(ICQUser *u, unsigned short cmd, char *cmdStr, char *m);

    bool openConnection(TCPSocket &socket);

    ICQEvent *sendTCP(ICQUser *u, unsigned short cmd, char *cmdStr, char *m);

    ICQEvent *sendUDP(ICQUser *u, unsigned short cmd, char *cmdStr, char *m);

};

///////////////////////////////////////////////////////////////////////////////

extern ICQ icq;

#endif
