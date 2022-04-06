#pragma once

#include <windows.h>
#include <malloc.h>

using namespace std;
#include <fstream>
#include <string>
#include <sstream>

#include <boost\regex.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\nondet_random.hpp>
#include <boost\random\variate_generator.hpp>
#include <boost\random\uniform_int.hpp>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_system.h>
#include <m_gui.h>

#include <m_variables.h>
#include <m_folders.h>

#include "globals.h"
#include "stopspam.h"
#include "version.h"
#include "resource.h"
#include "utilities.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

extern HANDLE hStopSpamLogDirH;

int OnDbContactSettingChanged(WPARAM w, LPARAM l);
int OnDbEventFilterAdd(WPARAM w, LPARAM l);
int OnDbEventAdded(WPARAM hContact, LPARAM hDbEvent);
int OnOptInit(WPARAM w, LPARAM l);

void __cdecl CleanThread(void*);