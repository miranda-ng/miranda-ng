#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <time.h>
#include <direct.h>
#include <process.h>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "newpluginapi.h"
#include "m_options.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_database.h"
#include "m_popup.h"

#include "resource.h"
#include "Version.h"

extern HINSTANCE hInst;

const char PlugName[] = "NotifyAnything";

//---------------------------
//---Internal Hooks (see main.c)
//---(Workaround till CallServiceSync is available)

struct NASettings {
	enum sound_t { never, always, request };

	bool local_only, debug_messages, log_to_file, use_pcspeaker, allow_execute;
	sound_t sound;
	int port;
	std::tstring password, log_filename;
};

extern NASettings g_settings;

void stop_threads();
void start_threads();

void save_settings();
void load_settings();

int OptionsInitialize(WPARAM wParam, LPARAM lParam);

extern std::tstring g_mirandaDir;
#define LOG_ID LPGEN("NotifyAnything")
