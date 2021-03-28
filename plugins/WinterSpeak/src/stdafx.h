#pragma once

#include <windows.h>
#include <CommCtrl.h>
#include <Shlwapi.h>

#include <memory>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_genmenu.h>
#include <m_utils.h>
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
#include "version.h"

#include "DialogConfigEngine.h"
#include "DialogConfigActive.h"

#include "AnnounceDialog.h"
#include "AnnounceDatabase.h"
#include "UserInformation.h"
#include "EventInformation.h"
#include "ProtocolInformation.h"

#include "SpeakAnnounce.h"

#include "ConfigDatabase.h"
#include "MirandaDialog.h"
#include "Subject.h"
#include "SpeechInterface.h"
#include "SpeakConfig.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
