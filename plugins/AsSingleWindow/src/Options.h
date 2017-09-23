#pragma once

#define SETTINGSNAME "AsSingleWindow"

int InitOptions(WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK cbOptionsDialog(HWND, UINT, WPARAM, LPARAM);

void dlgProcessInit(HWND, UINT, WPARAM, LPARAM);
void dlgProcessCommand(HWND, UINT, WPARAM, LPARAM);
void dlgUpdateControls(HWND);

void optionsLoad();
void optionsUpdate(HWND);
void optionsSave();
