/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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
 |   |-->next contact=NULL
 |   |-->first settings	as above
 |   \-->first/last/firstunread event as above
 \-->first module name (DBModuleName)
     \-->next module name (DBModuleName)
	     \--> ...
*/

//#define DB_RESIZE_GRANULARITY    16384
#define DB_THIS_VERSION          0x00000700u
#define DB_SETTINGS_RESIZE_GRANULARITY  128

struct DBSignature {
  char name[15];
  BYTE eof;
};

static struct DBSignature dbSignature={"Miranda ICQ DB",0x1A};

#include <pshpack1.h>
struct DBHeader {
  BYTE signature[16];      // 'Miranda ICQ DB',0,26
  DWORD version;		   //as 4 bytes, ie 1.2.3.10=0x0102030a
                           //this version is 0x00000700
  DWORD ofsFileEnd;		   //offset of the end of the database - place to write
                           //new structures
  DWORD slackSpace;		   //a counter of the number of bytes that have been
                           //wasted so far due to deleting structures and/or
						   //re-making them at the end. We should compact when
						   //this gets above a threshold
  DWORD contactCount;	   //number of contacts in the chain,excluding the user
  DWORD ofsFirstContact;   //offset to first struct DBContact in the chain
  DWORD ofsUser;		   //offset to struct DBContact representing the user
  DWORD ofsFirstModuleName;	//offset to first struct DBModuleName in the chain
};

#define DBCONTACT_SIGNATURE   0x43DECADEu
struct DBContact {
  DWORD signature;
  DWORD ofsNext;			 //offset to the next contact in the chain. zero if
                             //this is the 'user' contact or the last contact
							 //in the chain
  DWORD ofsFirstSettings;	 //offset to the first DBContactSettings in the
                             //chain for this contact.
  DWORD eventCount;			 //number of events in the chain for this contact
  DWORD ofsFirstEvent,ofsLastEvent;	 //offsets to the first and last DBEvent in
                                     //the chain for this contact
  DWORD ofsFirstUnreadEvent; //offset to the first (chronological) unread event
							 //in the chain, 0 if all are read
  DWORD timestampFirstUnread; //timestamp of the event at ofsFirstUnreadEvent
};

#define DBMODULENAME_SIGNATURE  0x4DDECADEu
struct DBModuleName {
  DWORD signature;
  DWORD ofsNext;		//offset to the next module name in the chain
  BYTE cbName;			//number of characters in this module name
  char name[1];			//name, no nul terminator
};

#define DBCONTACTSETTINGS_SIGNATURE  0x53DECADEu
struct DBContactSettings {
  DWORD signature;
  DWORD ofsNext;		 //offset to the next contactsettings in the chain
  DWORD ofsModuleName;	 //offset to the DBModuleName of the owner of these
                         //settings
  DWORD cbBlob;			 //size of the blob in bytes. May be larger than the
                         //actual size for reducing the number of moves
						 //required using granularity in resizing
  BYTE blob[1];			 //the blob. a back-to-back sequence of DBSetting
                         //structs, the last has cbName=0
};

/*	not a valid structure, content is figured out on the fly
struct DBSetting {
  BYTE cbName;			//number of bytes in the name of this setting
                        //this =0 marks the end
  char szName[...];		//setting name, excluding nul
  BYTE dataType;		//type of data. see m_database.h, db/contact/getsetting
  union {			   //a load of types of data, length is defined by dataType
    BYTE bVal; WORD wVal; DWORD dVal;
	struct {
	  WORD cbString;
	  char szVal[...];	  //excludes nul terminator
	};
	struct {
	  WORD cbBlob;
	  BYTE blobVal[...];
	};
  };
};
*/

#define DBEVENT_SIGNATURE  0x45DECADEu
struct DBEvent {
  DWORD signature;
  DWORD ofsPrev,ofsNext;	 //offset to the previous and next events in the
                             //chain. Chain is sorted chronologically
  DWORD ofsModuleName;		 //offset to a DBModuleName struct of the name of
                             //the owner of this event
  DWORD timestamp;			 //seconds since 00:00:00 01/01/1970
  DWORD flags;				 //see m_database.h, db/event/add
  WORD eventType;			 //module-defined event type
  DWORD cbBlob;				 //number of bytes in the blob
  BYTE blob[1];				 //the blob. module-defined formatting
};
#include <poppack.h>

typedef struct
{
	BYTE bIsResident;
	char name[1];
}
	DBCachedSettingName;

typedef struct
{
	char* name;
	DBVARIANT value;
}
	DBCachedGlobalValue;

typedef struct DBCachedContactValue_tag
{
	char* name;
	DBVARIANT value;
	struct DBCachedContactValue_tag* next;
}
	DBCachedContactValue;

typedef struct
{
	HANDLE hContact;
	HANDLE hNext;
	DBCachedContactValue* first;
	DBCachedContactValue* last;
}
	DBCachedContactValueList;

//databasecorruption: with NULL called if any signatures are broken. very very fatal
void DatabaseCorruption(TCHAR *text);
PBYTE DBRead(DWORD ofs,int bytesRequired,int *bytesAvail);	//any preview result could be invalidated by the next call
void DBWrite(DWORD ofs,PVOID pData,int count);
void DBFill(DWORD ofs,int bytes);
void DBFlush(int setting);
void DBMoveChunk(DWORD ofsDest,DWORD ofsSource,int bytes);
DWORD CreateNewSpace(int bytes);
void DeleteSpace(DWORD ofs,int bytes);
DWORD ReallocSpace(DWORD ofs,int oldSize,int newSize);
void GetProfileDirectory(char *szPath,int cbPath);
int GetDefaultProfilePath(char *szPath,int cbPath,int *specified);
int ShouldShowProfileManager(void);
int CheckDbHeaders(struct DBHeader * hdr);
int CreateDbHeaders(HANDLE hFile);
int LoadDatabaseModule(void);
void UnloadDatabaseModule(void);

#define MAXCACHEDREADSIZE     65536

//#define DBLOGGING

#ifdef _DEBUG
//#define DBLOGGING
#endif
#ifdef DBLOGGING
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
