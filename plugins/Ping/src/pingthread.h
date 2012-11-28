#ifndef _PINGTHREAD_H
#define _PINGTHREAD_H

#include "pinglist.h"
#include "pinggraph.h"
#include "utils.h"
#include "options.h"

extern HANDLE mainThread;
extern HANDLE hWakeEvent; 
extern CRITICAL_SECTION thread_finished_cs, list_changed_cs, data_list_cs;

extern PINGLIST data_list;

int FillList(WPARAM wParam, LPARAM lParam);

int RefreshWindow(WPARAM wParam, LPARAM lParam);

void UpdateFrame();
void AttachToClist(bool attach);

void InitList();
void DeinitList();

#endif


