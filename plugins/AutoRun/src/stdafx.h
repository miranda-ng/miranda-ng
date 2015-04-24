#include <windows.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_database.h>
#include <win2k.h>

#include "resource.h"
#include "Version.h"

#define SUB_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define ModuleName LPGEN("Autorun")
