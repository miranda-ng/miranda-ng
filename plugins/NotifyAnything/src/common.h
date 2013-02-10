// disable warnings about underscore in stdc functions
#pragma warning(disable: 4996)

#include <windows.h>

#include <string.h>
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <stdio.h>
#include <direct.h>
#include <process.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include <string>
#include <map>
#include <set>
#include <vector>

#include "newpluginapi.h"
#include "m_system_cpp.h"
#include "m_options.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_database.h"
#include "m_utils.h"

#include "m_popup.h"
#include "m_LogService.h"

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
	std::string password, log_filename;
};

extern NASettings g_settings;

void stop_threads();
void start_threads();

void save_settings();
void load_settings();

int OptionsInitialize(WPARAM wParam, LPARAM lParam);

extern std::string g_mirandaDir;
extern bool IsLogService;
#define LOG_ID "NotifyAnything"
