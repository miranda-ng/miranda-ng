// Copied from http://linuxgazette.net/issue74/tougher.html
// (only slightly modified)
// heavy modified (für windows umgeschrieben) - dufte

// Implementation of the Socket class.


#include "stdafx.h"

#include "xdebug.h"
#include "socket.h"
#include "string.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>

/*NETLIB*****************/
#include <newpluginapi.h>
#include <m_netlib.h>
extern HANDLE hNetlib;
/*NETLIB*****************/


using namespace std;

Socket::Socket( std::string host, int port, int useproxy, std::string proxyhost , int proxyport ) :
  m_sock ( -1 )
{

  memset ( &m_addr,
	   0,
	   sizeof ( m_addr ) );

  if (!create()) {
    throw SocketException( "Could not create socket." );
  }
  if (!connect(host,port,useproxy,proxyhost,proxyport)) {
    throw SocketException( "Could not bind port." );
  }
}

Socket::~Socket()
{
	Netlib_CloseHandle(this->netlibcon);
}

bool Socket::create()
{

/*
  m_sock = socket ( AF_INET, SOCK_STREAM, 0 );


  if ( ! is_valid() )
    return false;*/


  // TIME_WAIT - argh
//  int on = 1;
  //if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
//    return false;

  return true;

}



bool Socket::bind ( const int port )
{
/*  if ( ! is_valid() )
    {
      return false;
    }



  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons ( port );

  int bind_return = ::bind ( m_sock,
			     ( struct sockaddr * ) &m_addr,
			     sizeof ( m_addr ) );


  if ( bind_return == -1 )
    {
      return false;
    }
*/
  return true;
}


bool Socket::listen() const
{
  /*if ( ! is_valid() )
    {
      return false;
    }

  int listen_return = ::listen ( m_sock, MAXCONNECTIONS );


  if ( listen_return == -1 )
    {
      return false;
    }
*/
  return false;
}


bool Socket::accept ( Socket& new_socket ) const
{
  /*int addr_length = sizeof ( m_addr );
  new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

  if ( new_socket.m_sock <= 0 )
    return false;
  else
    return true;*/
	return false;

}

bool Socket::send ( char *buf, int length ) const {
  int status = Netlib_Send(this->netlibcon,buf, length,0); //::send ( m_sock, buf, length, 0);
  if ( status == -1 )
    {
      return false;
    }
  else
    {
      return true;
    }
}
bool Socket::send ( const std::string s ) const
{
  int status = Netlib_Send(this->netlibcon,s.c_str(), s.size(),0); //::send ( m_sock, s.c_str(), s.size(), 0);
  if ( status == -1 )
    {
      return false;
    }
  else
    {
      return true;
    }
}

int Socket::recv ( char *buf, int maxlen ) const {
  int status = Netlib_Recv(this->netlibcon,buf,maxlen,0); //::recv( m_sock, buf, maxlen, 0 );

  if ( status == -1 )
    {
		//std::cout << "status == -1   errno == " << errno << "  in Socket::recv WSA:::" << ::WSAGetLastError() << "\n";
      return 0;
    }
    return status;
}

int Socket::recv ( std::string& s ) const
{
  char buf [ MAXRECV + 1 ];

  s = "";

  memset ( buf, 0, MAXRECV + 1 );

  int status = Netlib_Recv(this->netlibcon,buf,MAXRECV,0);//::recv ( m_sock, buf, MAXRECV, 0 );

  if ( status == -1 )
    {
      //std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
      return 0;
    }
  else if ( status == 0 )
    {
      return 0;
    }
  else
    {
      s = buf;
      return status;
    }
}



bool Socket::connect ( const std::string host, const int port, int useproxy,std::string proxyhost,int proxyport)
{
  //if ( ! is_valid() ) return false;

  /*m_addr.sin_family = AF_INET;
  if (useproxy)
  {
	m_addr.sin_port = htons ( proxyport );
	m_addr.sin_addr.s_addr=inet_addr(proxyhost.c_str());
  }
  else
  {
	m_addr.sin_port = htons ( port );
	m_addr.sin_addr.s_addr=inet_addr(host.c_str());
  }
  */

  int status = 0; //inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

  /* XDEBUG(("Is valid .. %d\n",status));
  if ( errno == EAFNOSUPPORT ) return false; */
  //status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

  /*if (useproxy&&status==0) {
	      char temp[1024];
		  this->send("CONNECT cs.xfire.com:25999 HTTP/1.1\nHost: cs.xfire.com:25999\nUser-Agent: " + NETLIB_USER_AGENT + "\n\n");
		  this->recv((char*)temp,1024);
	  }*/

  NETLIBOPENCONNECTION ncon = { 0 };
  ncon.cbSize = sizeof(ncon);
  ncon.szHost = "cs.xfire.com";
  ncon.wPort = (WORD)atol("25999");
  ncon.timeout=5;
  netlibcon = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) hNetlib, (LPARAM) & ncon);


  //std::cout << "status: " << status << std::endl;
  if ( netlibcon != NULL )
  {
	  //CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) hNetlib, (LPARAM) & ncon);
	  m_sock=CallService(MS_NETLIB_GETSOCKET,(WPARAM) netlibcon,0);
    return true;
  }
  else {
    int errsv = errno;
    if (errsv == SOCKET_ERROR) {
     // std::cout << "Connection refused to " << host << ":" << port << std::endl;
      throw SocketException( "Connection refused." );
    }
    //std::cout << "errno: " << errsv << std::endl;
    return false;
  }
}

void Socket::set_non_blocking ( const bool b )
{
/*	u_long arg=0;

	if (b) arg=1;*/

	//ioctlsocket(m_sock, 0x8004667e,&arg);
//  int opts;

 /* opts = fcntl ( m_sock,
		 F_GETFL );

  if ( opts < 0 )
    {
      return;
    }
*/
  //if ( b )
  //  opts = ( opts | O_NONBLOCK );
  /*else
    opts = ( opts & ~O_NONBLOCK );*/

  /*fcntl ( m_sock,
	  F_SETFL,opts );*/

}
