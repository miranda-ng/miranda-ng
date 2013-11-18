#ifndef _SESSION_INC
#define _SESSION_INC

#include "common.h"
#include "options.h"
#include "userlist.h"
#include "messaging.h"
#include "files.h"
#include "conference.h"
#include "utils.h"
#include "session_announce_win.h"

WORD GetClientVersion();
WORD GetServerVersion();

int SetIdle(bool idle);
int LogIn(int status, HANDLE hNetlibUser);
int SetSessionStatus(int status);
void SetSessionAwayMessage(int status, char *msg);
int LogOut();

void UpdateSelfStatus();

void InitAwayMsg();
void DeinitAwayMsg();

void InitSessionMenu();
void DeinitSessionMenu();

void InitCritSection();
void DeinitCritSection();
#endif