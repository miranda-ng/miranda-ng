#if !defined(HISTORYSTATS_GUARD_MAIN_H)
#define HISTORYSTATS_GUARD_MAIN_H

#include "stdafx.h"
#include "_consts.h"

#include "settingsserializer.h"
#include "statistic.h"

extern HINSTANCE g_hInst;

extern PLUGININFOEX g_pluginInfoEx;

extern SettingsSerializer* g_pSettings;

extern bool g_bMainMenuExists;
extern bool g_bContactMenuExists;
extern bool g_bExcludeLock;
extern bool g_bConfigureLock;

void AddMainMenu();
void AddContactMenu();

// shortcut for filling std::vector
template <typename T_>
std::vector<T_>& operator <<(std::vector<T_>& container, const T_& value)
{
	container.push_back(value);

	return container;
}

#endif // HISTORYSTATS_GUARD_MAIN_H