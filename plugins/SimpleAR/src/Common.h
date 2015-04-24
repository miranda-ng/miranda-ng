#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_string.h>

#include <m_variables.h>

#include "Resource.h"
#include "Version.h"

#define protocolname		"SimpleAutoReplier" //no spaces here :)

#define KEY_ENABLED			"Enabled"
#define KEY_HEADING			"Heading"
#define KEY_REPEATINTERVAL	"RepeatInterval"

//General
extern HINSTANCE hinstance;
extern INT interval;
extern TCHAR *ptszDefaultMsg[6];
extern HGENMENU hEnableMenu;
extern IconItemT iconList[];

extern INT OptInit(WPARAM wParam, LPARAM lParam);
