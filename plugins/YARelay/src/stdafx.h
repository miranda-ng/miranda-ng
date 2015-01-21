/*
yaRelay.cpp

Yet Another Relay plugin. v.0.0.0.3
This plugin forwards all incoming messages to any contact.

Features:
 - Forwards all messages from any specified contact (or from all contacts)
 - Works only if your status is equals to specified (of set of statuses)
 - Could be specified any template for sent messages
 - Original message could be split up (by size)
 - Could be specified number of split parts to send
 - Incoming message could be marked as 'read' (optional)
 - Outgoing messages could be saved in history (optional)

(c)2005 Anar Ibragimoff (ai91@mail.ru)

*/

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <time.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_contacts.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <win2k.h>

#include "resource.h"
#include "Version.h"

#define STATUS_OFFLINE               0x1
#define STATUS_ONLINE                0x2
#define STATUS_AWAY                  0x4
#define STATUS_NA                    0x8
#define STATUS_OCCUPIED              0x10
#define STATUS_DND                   0x20
#define STATUS_FREECHAT              0x40
#define STATUS_INVISIBLE             0x80

#define MAXTEMPLATESIZE 1024

extern HINSTANCE hInst;

extern MCONTACT hForwardFrom, hForwardTo;
extern TCHAR tszForwardTemplate[MAXTEMPLATESIZE];
extern int iSplit, iSplitMaxSize, iSendParts, iMarkRead, iSendAndHistory, iForwardOnStatus;

struct MESSAGE_PROC
{
	HANDLE hProcess;
	char *msgText;
	int retryCount;
};

extern LIST<MESSAGE_PROC> arMessageProcs;

int OptionsInit(WPARAM wParam, LPARAM);
