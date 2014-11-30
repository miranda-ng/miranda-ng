/*
 * $Id: config.h 9339 2009-04-05 00:15:32Z gena01 $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#ifndef _YAHOO_CONFIG_H_
#define _YAHOO_CONFIG_H_

#define HAVE_STRING_H 1
#define STDC_HEADERS 1
#define HAVE_STRCHR 1
#define HAVE_MEMCPY 1

#define PACKAGE "libyahoo2"
#define VERSION "0.7.5"

#include <windows.h>
#include <stdio.h>

#ifdef _MSC_VER

#define strncasecmp strnicmp

#define wsnprintf _wsnprintf
#define snprintf _snprintf
#define vsnprintf _vsnprintf

#endif

#define USE_STRUCT_CALLBACKS

#define write(a,b,c) send(a,b,c,0)
#define read(a,b,c)  recv(a,b,c,0)

#include <newpluginapi.h>
#include <m_netlib.h>
#define close(a)	 Netlib_CloseHandle((HANDLE)a)

/*
 * Need to handle MD5 through Miranda. otherwise just include some md5.h implementation instead
 */
#include <m_utils.h>

#endif
