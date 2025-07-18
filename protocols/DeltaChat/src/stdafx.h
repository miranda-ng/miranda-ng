// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>

#include <malloc.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <time.h>

#include "resource.h"

#include <m_system.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>

#include "../ffi/deltachat.h"

#define MODULENAME "DeltaChat"

#define DB_KEY_ID  "id"

#include "version.h"
#include "proto.h"
