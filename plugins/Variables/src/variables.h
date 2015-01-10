/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS

#include <malloc.h>
#include <windows.h>
#include <uxtheme.h>
#include <time.h>
#include <tlhelp32.h>
#include <vdmdbg.h>
#include <lmcons.h>

#include <pdh.h>
#include <pdhmsg.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_options.h>
#include <m_icolib.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_string.h>
#include <m_metacontacts.h>

#include <m_variables.h>

#include "..\helpers\gen_helpers.h"

#include "..\pcre16\src\pcre.h"

#include "resource.h"
#include "version.h"
#include "contact.h"
#include "enumprocs.h"
#include "parse_alias.h"
#include "parse_external.h"
#include "parse_inet.h"
#include "parse_logic.h"
#include "parse_math.h"
#include "parse_metacontacts.h"
#include "parse_miranda.h"
#include "parse_regexp.h"
#include "parse_str.h"
#include "parse_system.h"
#include "parse_variables.h"

#define MODULENAME "Variables"

#define SETTING_STARTUPTEXT    "StartupText"
#define SETTING_STRIPCRLF      "StripCRLF"
#define SETTING_STRIPWS        "StripWS"
#define SETTING_STRIPALL       "StripAll"
#define SETTING_PARSEATSTARTUP "ParseAtStartup"
#define SETTING_SPLITTERPOS    "SplitterPos"
#define SETTING_SUBJECT        "LastSubject"

#define FIELD_CHAR          '%'
#define FUNC_CHAR           '?'
#define FUNC_ONCE_CHAR      '!'
#define DONTPARSE_CHAR      '`'
#define TRYPARSE_CHAR_OPEN  '['
#define TRYPARSE_CHAR_CLOSE ']'
#define COMMENT_STRING      "#"

// special tokens
#define SUBJECT       _T("subject")
#define MIR_EXTRATEXT _T("extratext")

#define VAR_HELP_TEXT LPGENT("--- Special characters ---\r\n\r\n\
The following characters have a special meaning in a formatting string:\r\n\r\n\
?<function>(<arguments>)\r\n\
This will parse the function given the arguments, the result will be parsed again. Example: Today is ?cdate(yyyy/MM/dd).\r\n\r\n\
!<function>(<arguments>)\r\n\
This will parse the function given the arguments, the result will not be parsed again. Example: Message waiting: !message(,first,rcvd,unread).\r\n\r\n\
%<field>%\r\n\
This will parse the given field. Example: I installed Miranda at: %mirandapath%.\r\n\r\n\
`<string>`\r\n\
This will not parse the given string, any function, field or special character in the string will shown in the result without being translated. Example: Use `%mirandapath%` to show the installation path.\r\n\r\n\
#<comment>\r\n\
This will add a comment in the formatting string. Everything from the # character to the end of the line will be removed. Example: %dbprofile% #this is a useless comment.\r\n\r\n\r\n\
--- Contacts ---\r\n\r\n\
Whenever a functions requires a contact as an argument, you can specify it in two ways:\r\n\r\n\
(1) Using a unique ID (UIN for ICQ, email for MSN) or, a protocol ID followed by a unique ID in the form <PROTOID:UNIQUEID>, for example <MSN:miranda@hotmail.com> or <ICQ:123456789>.\r\n\r\n\
(2) Using the contact function:\r\n\
?contact(x,y)\r\n\
A contact will be searched which will have value x for its property y, y can be one of the following:\r\n\
first, last, nick, email, ID or display\r\n\r\n\
For example: ?contact(miranda@hotmail.com,email) or ?contact(Miranda,nick). \
The contact function will return either a unique contact according to the arguments or nothing if none or multiple contacts exists with the given property.")

// options
#define IDT_PARSE 1

#define DM_SPLITTERMOVED     (WM_USER+15)

// Messages you can send to the help window:
#define VARM_PARSE              (WM_APP+11) // wParam=lParam=0
#define VARM_SETINPUTTEXT       (WM_APP+12)
#define VARM_GETINPUTTEXT	     (WM_APP+13)
#define VARM_GETINPUTTEXTLENGTH (WM_APP+14)
#define VARM_SETSUBJECT         (WM_APP+15)
#define VARM_GETSUBJECT         (WM_APP+16) // wParam=MCONTACT hContact
#define VARM_SETEXTRATEXT       (WM_APP+17)
#define VARM_GETEXTRATEXT       (WM_APP+18)
#define VARM_GETEXTRATEXTLENGTH (WM_APP+19)
#define VARM_GETDIALOG          (WM_APP+20)

// if a different struct internally is used, we can use TOKENREGISTEREX
#define TOKENREGISTEREX TOKENREGISTER

// old struct
typedef struct {
	int cbSize;
	char *szFormat;
	char *szSource;
	MCONTACT hContact;
	int pCount;  // number of succesful parses
	int eCount;	 // number of failures
} FORMATINFOV1;

struct ParseOptions {
	BOOL bStripEOL;
	BOOL bStripWS;
	BOOL bStripAll;
};

extern HINSTANCE hInst;
extern struct ParseOptions gParseOpts;
extern int hLangpack;

// variables.c
//TCHAR *getArguments(char *string, char ***aargv, int *aargc);
//int isValidTokenChar(char c);
TCHAR *formatString(FORMATINFO *fi);
int  setParseOptions(struct ParseOptions *po);
int  LoadVarModule();
int  UnloadVarModule();
// tokenregister.c
int  registerIntToken(TCHAR *szToken, TCHAR *(*parseFunction)(ARGUMENTSINFO *ai), int extraFlags, char* szHelpText);
INT_PTR registerToken(WPARAM wParam, LPARAM lParam);
int  deRegisterToken(TCHAR *var);
TOKENREGISTEREX *searchRegister(TCHAR *var, int type);
TCHAR *parseFromRegister(ARGUMENTSINFO *ai);
TOKENREGISTEREX *getTokenRegister(int i);
int  getTokenRegisterCount();
TOKENREGISTER *getTokenRegisterByIndex(int i);
void deRegisterTemporaryVariables();
int  initTokenRegister();
int  deinitTokenRegister();
// contact.c
BYTE getContactInfoType(TCHAR* type);
TCHAR* getContactInfoT(BYTE type, MCONTACT hContact);
int  getContactFromString( CONTACTSINFO* );
int  initContactModule();
int  deinitContactModule();
// alias
void registerAliasTokens();
void unregisterAliasTokens();
// system
void registerSystemTokens();
// external
void registerExternalTokens();
// miranda
void registerMirandaTokens();
// str
void registerStrTokens();
// variables
void registerVariablesTokens();
void unregisterVariablesTokens();
// logic
void registerLogicTokens();
// math
void registerMathTokens();
// metacontacts
void registerMetaContactsTokens();
// options
int  OptionsInit(WPARAM wParam, LPARAM);
// reg exp
void registerRegExpTokens();
// inet
void registerInetTokens();
// help
INT_PTR  showHelpService(WPARAM wParam, LPARAM lParam);
INT_PTR  showHelpExService(WPARAM wParam, LPARAM lParam);
INT_PTR  getSkinItemService(WPARAM wParam, LPARAM lParam);
int  iconsChanged(WPARAM wParam, LPARAM lParam);

int ttoi(TCHAR *string);
TCHAR *itot(int num);
