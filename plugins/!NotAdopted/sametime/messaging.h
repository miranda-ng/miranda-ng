#ifndef _MESSAGING_INC
#define _MESSAGING_INC

#include "common.h"
#include "userlist.h"
#include "options.h"
#include "utils.h"

void SendTyping(HANDLE hContact, bool typing);
HANDLE SendMessageToUser(HANDLE hContact, char *msg);
HANDLE SendMessageToUserW(HANDLE hContact, wchar_t *msg);

void CloseIm(HANDLE hContact);

void InitMessaging(mwSession *session);
void DeinitMessaging(mwSession *session);

#endif
