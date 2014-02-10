// Copied from http://linuxgazette.net/issue74/tougher.html
// (only slightly modified)


// Definition of the Socket class


#ifndef Socket_class
#define Socket_class

#include "stdafx.h"

#include "socketexception.h"


const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

class Socket
{
 public:
  //proxy hinzugefügt dufte
  Socket( std::string host, int port,int useproxy = 0,std::string proxyhost = " ", int proxyport = 0 );
  virtual ~Socket();

  // Server initialization
  bool create();
  bool bind ( const int port );
  bool listen() const;
  bool accept ( Socket& ) const;

  // Client initialization
  bool connect ( const std::string host, const int port, int useproxy,std::string proxyhost,int proxyport);

  // Data Transimission
  bool send ( char *buf, int length ) const;
  bool send ( const std::string ) const;
  int recv ( std::string& ) const;
  int recv ( char *buf, int maxlen ) const;


  void set_non_blocking ( const bool );

  bool is_valid() const { return true; }

  int m_sock;

 private:

  
  sockaddr_in m_addr;
  HANDLE netlibcon;


};


#endif
