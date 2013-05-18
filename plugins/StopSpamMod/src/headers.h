#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

using namespace std;
#include <fstream>
#include <string>


#include <newpluginapi.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_system_cpp.h>

#include <m_variables.h>
#include <m_folders.h>

#include <boost\thread.hpp>
#include <boost\regex.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\nondet_random.hpp>
#include <boost\random\variate_generator.hpp>
#include <boost\random\uniform_int.hpp>

#include "globals.h"
#include "stopspam.h"
#include "eventhooker.h"
#include "version.h"
#include "resource.h"
#include "utilities.h"
#include "utf8.h"

extern HINSTANCE hInst;
extern HANDLE hStopSpamLogDirH;
