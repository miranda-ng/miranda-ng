/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


//all offsets are relative to the start of the file
//offsets are 0 if there is nothing in the chain or this is the last in the
//chain

/* tree diagram

DBHeader
|-->end of file (plain offset)
|-->first contact (DBContact)
|   |-->next contact (DBContact)
|   |   \--> ...
|   |-->first settings (DBContactSettings)
|   |	 |-->next settings (DBContactSettings)
|   |   |   \--> ...
|   |   \-->module name (DBModuleName)
|   \-->first/last/firstunread event
|-->user contact (DBContact)
|   |-->next contact = NULL
|   |-->first settings	as above
|   \-->first/last/firstunread event as above
\-->first module name (DBModuleName)
\-->next module name (DBModuleName)
\--> ...
*/

//#define DBLOGGING

#ifdef _DEBUG
//#define DBLOGGING
#endif
#ifdef DBLOGGING
char* printVariant(DBVARIANT* p);
void DBLog(const char *file,int line,const char *fmt,...);
#define logg()  DBLog(__FILE__,__LINE__,"")
#define log0(s)  DBLog(__FILE__,__LINE__,s)
#define log1(s,a)  DBLog(__FILE__,__LINE__,s,a)
#define log2(s,a,b)  DBLog(__FILE__,__LINE__,s,a,b)
#define log3(s,a,b,c)  DBLog(__FILE__,__LINE__,s,a,b,c)
#define log4(s,a,b,c,d)  DBLog(__FILE__,__LINE__,s,a,b,c,d)
#else
#define logg()
#define log0(s)
#define log1(s,a)
#define log2(s,a,b)
#define log3(s,a,b,c)
#define log4(s,a,b,c,d)
#endif
