#pragma once

extern char PLUGINNAME[];

void logRegister(void);
void logUnregister(void);
void log(const wchar_t* szText);
void log_p(const wchar_t* szText, ...);
