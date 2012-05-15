/*

Import plugin for Miranda IM

Copyright (C) 2001,2002,2003,2004 Martin Öberg, Richard Hughes, Roland Rabien & Tristan Van de Vreede

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



#ifndef MIRABILIS_H
#define MIRABILIS_H

#include <windows.h>
#include <newpluginapi.h>
#include <m_database.h>

// ======================
// == GLOBAL FUNCTIONS ==
// ======================

HANDLE HistoryImportFindContact(HWND hdlgProgress, char* szModuleName, DWORD uin,int addUnknown);

// =====================
// == LOCAL FUNCTIONS ==
// =====================


// Main function
static void MirabilisImport(HWND hdlgProgressWnd);

// GUI callbacks
INT_PTR CALLBACK ImportTypePageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK FinishedPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK ProgressPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK MirabilisPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK MirabilisOptionsPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);

// Helper functions for entries
static int GetHighestIndexEntry(void);
static int GetIdDatOfs(DWORD id);
static int GetDatEntryType(DWORD ofs);
DWORD FindMyDetails(void);

// Parsing functions
DWORD GetDBVersion();
int GetEntryVersion(WORD wSeparatorValue);
DWORD ReadPropertyBlock(DWORD dwOffset, char* SearchWord, int* nSearchResult);
DWORD ReadSubList(DWORD dwOffset);
DWORD ReadPropertyBlock(DWORD dwOffset, char* SearchWord, int* nSearchResult);
DWORD ReadPropertyBlockList(DWORD dwOffset, char* SearchWord, int* nSearchResult);
DWORD ReadWavList(DWORD ofs);
DWORD FindGroupList(DWORD dwOffset);
char* GetGroupName(DWORD dwGroupID);
int ImportGroups();
static HANDLE ImportContact(DWORD dwOffset);

BOOL ImportEvent(DWORD dwOffset);
BOOL ImportMessage(DWORD dwOffset);
BOOL ImportExtendedMessage(DWORD dwOffset);
BOOL ImportURLMessage(DWORD dwOffset);




// ======================
// == GLOBAL VARIABLES ==
// ======================

extern TCHAR importFile[MAX_PATH];
extern void (*DoImport)(HWND);
extern int nImportOption;
extern int nCustomOptions;


extern int      cICQAccounts;
extern char  ** szICQModuleName;
extern TCHAR ** tszICQAccountName;
extern int      iICQAccount;

// =====================
// == LOCAL VARIABLES ==
// =====================

static DWORD dwDBVersion;
static DWORD dwGroupListOfs;
static PBYTE pIdx,pDat;

// =============
// == DEFINES ==
// =============

// Contact versions
// These numbers are not 100% accurate
#define ENTRYVUNKNOWN -1
#define ENTRYV99A    200
#define ENTRYV99B    300
#define ENTRYV2000A  400
#define ENTRYV2000B  455
#define ENTRYV2001A  500
#define ENTRYV2001B  515
#define ENTRYV2002A  533

// Database versions
#define DBV99A    10
#define DBV99B    14
#define DBV2000A  17
#define DBV2000B  18
#define DBV2001A  19 // This is used by ICQ 2001a, 2001b & 2002a

#define DATENTRY_UNFILED    (DWORD)(-1)
#define DATENTRY_MESSAGE    0
#define DATENTRY_CONTACT    1
#define DATENTRY_IGNORED    2
#define DATENTRY_SYSTEM     9

#define MAX_NON_ICQ_CONTACTS  100

#define SUBTYPE_NEWMESSAGE  0x50
#define SUBTYPE_NEWURL      0xA0

#define SUBTYPE_MESSAGE     0xE0	//Message / URL Message / Request For Authorization / "Authorization" / System Request / "You Were Added" / Contacts List
#define SUBTYPE_CHATREQUEST 0xE1
#define SUBTYPE_FILEREQUEST 0xE2
#define SUBTYPE_MYDETAILS   0xE4
#define SUBTYPE_CONTACTINFO 0xE5
#define SUBTYPE_REMINDER    0xE6
#define SUBTYPE_ADDRESSBOOK 0xE7
#define SUBTYPE_VOICEMSG    0xEC	//???
#define SUBTYPE_NOTE        0xEE
#define SUBTYPE_EVENTFOLDER 0xEF
#define SUBTYPE_SERVERLIST  0xF1   //and objectionable word list
#define SUBTYPE_X1          0xF6   //(new to ICQ 99b???)

#define FILING_RECEIVED  0x01
#define FILING_DELETED   0x02
#define FILING_MESSAGE   0x04
#define MSGTYPE_MESSAGE  1
#define MSGTYPE_URL      4
#define MSGTYPE_CLIST    19
#include <pshpack1.h>

struct TIdxDatEntry {
  DWORD status;     //-2=valid, else is an index entry
  DWORD entryId;
  DWORD ofsNext,ofsPrev;
  DWORD datOfs;
};

struct TIdxIndexEntry {
  DWORD entryIdLow;
  DWORD entryIdHigh;
  DWORD ofsLower;
  DWORD ofsInHere;
  DWORD ofsHigher;
};

struct TDatEntryHeader {
  DWORD entrySize;   //in bytes
  DWORD entryType;	 //DATENTRY_* constant
  DWORD entryId;     //same as in index
  BYTE subType;      //SUBTYPE_* constant
  BYTE signature[15];
};

struct TDatEntryFooter {
  DWORD unknown;
  DWORD sent;   //1 if sent, 0 if received
  WORD separator;
  DWORD timestamp;	 //unix time
};

struct TDatMessage {
  struct TDatEntryHeader hdr;	  //hdr.entryType==DATENTRY_MESSAGE && hdr.subType==MSGTYPE_MESSAGE
  WORD separator;
  DWORD filingStatus;      //FILING_* flags
  WORD type;		//MSGTYPE_* constant
  DWORD uin;
  WORD textLen;
  char text[1];	 //0xFE separates description & URL in URLs
  //a struct TDatEntryFooter comes here
};

#include <poppack.h>

#endif
