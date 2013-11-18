#ifndef _CONFERENCE_INC
#define _CONFERENCE_INC

#include "common.h"
#include "userlist.h"
#include "options.h"
#include "utils.h"

#include <queue>
#include <string>

void InitConference(mwSession *session);
void DeinitConference(mwSession *session);

void InitConferenceMenu();
void DeinitConferenceMenu();

#endif