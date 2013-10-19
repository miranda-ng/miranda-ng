#include "Common.h"
#include "AnnounceDatabase.h"


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
AnnounceDatabase::AnnounceDatabase() : m_status_flags(0), m_event_flags(0), m_max_msg(0)
{	
	load();
}

//------------------------------------------------------------------------------
AnnounceDatabase::~AnnounceDatabase()
{
}

//------------------------------------------------------------------------------
bool AnnounceDatabase::getStatusFlag(StatusFlag flag) const
{
	return ((m_status_flags & (1 << flag)) != 0);
}

//------------------------------------------------------------------------------
void AnnounceDatabase::setStatusFlag(StatusFlag flag, bool state)
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
bool AnnounceDatabase::getEventFlag(EventFlag flag) const
{
	return ((m_event_flags & (1 << flag)) != 0);
}

//------------------------------------------------------------------------------
void AnnounceDatabase::setEventFlag(EventFlag flag, bool state)
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
void AnnounceDatabase::load()
{
	m_status_flags = db_get_dw(NULL, SPEAK, STATUS_FLAGS, 0xffff);
	m_event_flags  = db_get_dw(NULL, SPEAK, EVENT_FLAGS, 0xffff);
	m_max_msg      = db_get_dw(NULL, SPEAK, MAX_MSG_SIZE, 50);
}

//------------------------------------------------------------------------------
void 
AnnounceDatabase::save()
{
	db_set_dw(NULL, SPEAK, STATUS_FLAGS, m_status_flags);
	db_set_dw(NULL, SPEAK, EVENT_FLAGS, m_event_flags);
	db_set_dw(NULL, SPEAK, MAX_MSG_SIZE, m_max_msg);
}

//==============================================================================
