#ifndef _FRAME_INC
#define _FRAME_INC

#include "alarmlist.h"
#include "icons.h"
#include "options.h"

void RefreshReminderFrame();

void InitFrames();
void DeinitFrames();

// update 'show/hide reminder frame' menu item
void FixMainMenu();

// used when no multiwindow functionality avaiable
bool ReminderFrameVisible();
void SetReminderFrameVisible(bool visible);

#endif
