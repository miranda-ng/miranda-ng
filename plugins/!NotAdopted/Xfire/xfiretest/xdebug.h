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



#ifndef __XDEBUG_H
#define __XDEBUG_H

#define RESET 0

#define BLACK  0
#define RED    1
#define GREEN  2
#define YELLOW 3
#define WHITE  7
#include <stdio.h>
//#include <pthread.h>

//#define XFIRELIBDEBUG

#ifndef XFIRELIBDEBUG

#define XDEBUGS(args);
#define XDEBUG(args) ;
#define XDEBUG2(args,args2) ;
#define XDEBUG3(args,args2,args3) ;
#define XDEBUG4(args,args2,args3,args4) ;
#define XDEBUG5(args,args2,args3,args4,args5) ;
#define XDEBUG6(args,args2,args3,args4,args5,args6) ;
#define DUMPPACKET(args) ;

#define XINFO(args) ;
#define XINFO2(args,args2) ;
#define XINFO3(args,args2,args3) ;

#define XERROR(args) ;
#define XERROR2(args,args2) ;
#define XERROR3(args,args2,args3) ;

/*#define DUMP(args,args2) { FILE* errorfile=fopen("C:\\XFireDump.log","a"); \
						fprintf(errorfile,args,args2); \
						fprintf(errorfile,"\n",""); \
						fclose(errorfile); }\*/

/*#define XFireLog(args,args2) { FILE* errorfile=fopen("C:\\XFireDump.log","a"); \
						fprintf(errorfile,args,args2); \
						fprintf(errorfile,"\n",""); \
						fclose(errorfile); }\*/

/*#define XFireLog(args,args2) {  \
						char temp[266]; \
						sprintf(temp,args,args2); \
						MessageBoxA(NULL,temp,temp,0); }\*/

#endif

#ifdef XFIRELIBDEBUG

#define XDEBUGS(args) { FILE* errorfile=fopen("XFire.log","wb"); \
						fprintf(errorfile,args); \
						fclose(errorfile); }\

#define DUMPPACKET(args) { FILE* dumpfile=fopen(args,"wb"); \
						fwrite(buf,length,1,dumpfile); \
						fclose(dumpfile); }\


#define XDEBUG(args) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args); \
						fclose(errorfile); }\

  #define XDEBUG2(args,args2) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2); \
						fclose(errorfile); } \


  #define XDEBUG3(args,args2,args3) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2,args3); \
						fclose(errorfile); } \

	  #define XDEBUG4(args,args2,args3,args4) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2,args3,args4); \
						fclose(errorfile); } \

			  #define XDEBUG5(args,args2,args3,args4,args5) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2,args3,args4,args5); \
						fclose(errorfile); } \

			  #define XDEBUG6(args,args2,args3,args4,args5,args6) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2,args3,args4,args5,args6); \
						fclose(errorfile); } \


#define XINFO(args) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args); \
						fclose(errorfile); }\

  #define XINFO2(args,args2) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2); \
						fclose(errorfile); } \


  #define XINFO3(args,args2,args3) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2,args3); \
						fclose(errorfile); } \


#define XERROR(args) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args); \
						fclose(errorfile); }\

  #define XERROR2(args,args2) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2); \
						fclose(errorfile); } \


  #define XERROR3(args,args2,args3) { FILE* errorfile=fopen("XFire.log","a"); \
						fprintf(errorfile,args,args2,args3); \
						fclose(errorfile); } \





	



#endif
#endif