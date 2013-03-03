#ifndef _PINGTHREAD_H
#define _PINGTHREAD_H

int FillList(WPARAM wParam, LPARAM lParam);

int RefreshWindow(WPARAM wParam, LPARAM lParam);

void UpdateFrame();
void AttachToClist(bool attach);

void InitList();
void DeinitList();

#endif


