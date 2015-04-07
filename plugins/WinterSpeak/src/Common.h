#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Winsock2.h>
#include <CommCtrl.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_genmenu.h>
#include <m_utils.h>
#include <win2k.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_netlib.h>
#include <m_extraicons.h>
#include <m_message.h>

#include "m_winterspeak.h"

#include "resource.h"
#include "Version.h"

#include "DialogConfigEngine.h"
#include "DialogConfigActive.h"

#include "AnnounceDialog.h"
#include "AnnounceDatabase.h"
#include "SpeakAnnounce.h"

#include "ConfigDatabase.h"
#include "MirandaDialog.h"
#include "Subject.h"
#include "SpeechInterface.h"
#include "SpeakConfig.h"
#include "UserInformation.h"
#include "EventInformation.h"
#include "ProtocolInformation.h"