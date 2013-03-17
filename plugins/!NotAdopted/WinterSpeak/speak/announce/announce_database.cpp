//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "announce_database.h"

#include <general/debug/debug.h>

#include <windows.h>
#include <miranda32/random/plugins/newpluginapi.h>
#include <miranda32/database/m_database.h>

//------------------------------------------------------------------------------
namespace
{
    const char *SPEAK        = "speak_announce";
    const char *STATUS_FLAGS = "status_flags";
    const char *EVENT_FLAGS  = "event_flags";
    const char *MAX_MSG_SIZE = "max_msg_size";
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
AnnounceDatabase::AnnounceDatabase()
	:
    m_status_flags(0),
    m_event_flags(0),
    m_max_msg(0)
{	
	CLASSCERR("AnnounceDatabase::AnnounceDatabase");

	// load the database from miranda
	load();
}

//------------------------------------------------------------------------------
AnnounceDatabase::~AnnounceDatabase()
{
	CLASSCERR("AnnounceDatabase::~AnnounceDatabase");
}

//------------------------------------------------------------------------------
bool 
AnnounceDatabase::getStatusFlag(StatusFlag flag) const
{
	return ((m_status_flags & (1 << flag)) != 0);
}

//------------------------------------------------------------------------------
void 
AnnounceDatabase::setStatusFlag(StatusFlag flag, bool state)
{ 
    if (state)
    {
        m_status_flags |= (1 << flag);
    }
    else
    {
        m_status_flags &= ~(1 << flag);
    }
}

//------------------------------------------------------------------------------
bool 
AnnounceDatabase::getEventFlag(EventFlag flag) const
{
	return ((m_event_flags & (1 << flag)) != 0);
}

//------------------------------------------------------------------------------
void 
AnnounceDatabase::setEventFlag(EventFlag flag, bool state)
{ 
    if (state)
    {
        m_event_flags |= (1 << flag);
    }
    else
    {
        m_event_flags &= ~(1 << flag);
    }
}

//------------------------------------------------------------------------------
void
AnnounceDatabase::load()
{
	CLASSCERR("AnnounceDatabase::load");

    m_status_flags = DBGetContactSettingDword(NULL, SPEAK, STATUS_FLAGS,
        0xffff);
    m_event_flags  = DBGetContactSettingDword(NULL, SPEAK, EVENT_FLAGS, 
        0xffff);
    m_max_msg      = DBGetContactSettingDword(NULL, SPEAK, MAX_MSG_SIZE, 50);
}

//------------------------------------------------------------------------------
void 
AnnounceDatabase::save()
{
	CLASSCERR("AnnounceDatabase::save");

	DBWriteContactSettingDword(NULL, SPEAK, STATUS_FLAGS, m_status_flags);
	DBWriteContactSettingDword(NULL, SPEAK, EVENT_FLAGS, m_event_flags);
	DBWriteContactSettingDword(NULL, SPEAK, MAX_MSG_SIZE, m_max_msg);
}

//==============================================================================
