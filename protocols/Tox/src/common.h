#ifndef _COMMON_H_
#define _COMMON_H_

#include <winsock2.h>
#include <windows.h>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protomod.h>
#include <m_protosvc.h>

#include <m_database.h>
#include <m_langpack.h>

#include "version.h"
#include "resource.h"

#include "tox\tox.h"

extern HINSTANCE g_hInstance;

#define BOOTSTRAP_ADDRESS "23.226.230.47"
#define BOOTSTRAP_PORT 33445
#define BOOTSTRAP_KEY "A09162D68618E742FFBCA1C2C70385E6679604B2D80EA6E84AD0996A1AC8A074"

#include "tox_proto.h"

#endif //_COMMON_H_