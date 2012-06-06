#ifndef __MAIN_H
#define __MAIN_H

#ifdef __GNUC__
	#define __try
	#define __except(x) if (0) /* don't execute handler */
	#define __finally
	#define _try __try
	#define _except __except
	#define _finally __finally 
#endif
//For updater
//#define YAMN_9x
#ifndef WIN2IN1
#ifdef YAMN_9x
	#define YAMN_SHORTNAME "YAMN tweety win9x"
	#define YAMN_FILENAME "yamn_9x"
#else
	#define YAMN_SHORTNAME "YAMN tweety"
	#define YAMN_FILENAME "yamn"
#endif
#else
	#define YAMN_SHORTNAME "YAMN tweety 2in1"
	#define YAMN_FILENAME "yamn"
#endif //WIN2IN1

#include "version.h"
#define YAMN_NEWMAILSNDDESC		"YAMN: new mail message"
#define YAMN_CONNECTFAILSNDDESC	"YAMN: connect failed"
#define	YAMN_CONNECTFAILSOUND	"YAMN/Sound/ConnectFail"
#define	YAMN_NEWMAILSOUND		"YAMN/Sound/NewMail"

#define YAMN_DBMODULE		"YAMN"
#define YAMN_DBPOSX			"MailBrowserWinX"
#define YAMN_DBPOSY			"MailBrowserWinY"
#define YAMN_DBSIZEX		"MailBrowserWinW"
#define YAMN_DBSIZEY		"MailBrowserWinH"
#define YAMN_DBMSGPOSX		"MailMessageWinX"
#define YAMN_DBMSGPOSY		"MailMessageWinY"
#define YAMN_DBMSGSIZEX		"MailMessageWinW"
#define YAMN_DBMSGSIZEY		"MailMessageWinH"
#define YAMN_DBMSGPOSSPLIT	"MailMessageSplitY"
#define	YAMN_TTBFCHECK		"ForceCheckTTB"
#define	YAMN_SHOWMAINMENU	"ShowMainMenu"
#define	YAMN_CLOSEDELETE	"CloseOnDelete"
#define YAMN_SHOWASPROTO	"ShowAsProtcol"
#define	YAMN_DBTIMEOPTIONS	"MailBrowserTimeOpts"

#define YAMN_DEFAULTHK		MAKEWORD(VK_F11,MOD_CONTROL)

#define SHOWDATELONG		0x01
#define SHOWDATENOTODAY		0x02
#define SHOWDATENOSECONDS	0x04

extern unsigned char optDateTime;

//  Loading Icon and checking for icolib 
void LoadIcons();

#endif

