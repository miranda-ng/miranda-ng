#pragma once

#ifndef _OPTIONS_H
#define _OPTIONS_H

#define SETTINGSNAME "AsSingleWindow"

#ifdef UNICODE
#define LPGENSTR LPGENT
#define DBGetString DBGetContactSettingTString
#define DBWriteString DBWriteContactSettingTString
#else
#define LPGENSTR LPGEN
#define DBGetString DBGetContactSettingString
#define DBWriteString DBWriteContactSettingString
#endif

int InitOptions(WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK cbOptionsDialog(HWND, UINT, WPARAM, LPARAM);

void dlgProcessInit(HWND, UINT, WPARAM, LPARAM);
void dlgProcessCommand(HWND, UINT, WPARAM, LPARAM);
void dlgUpdateControls(HWND);

void optionsLoad();
void optionsUpdate(HWND);
void optionsSave();

#endif

// end of file