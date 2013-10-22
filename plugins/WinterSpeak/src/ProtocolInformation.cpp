#include "Common.h"
#include "ProtocolInformation.h"


//------------------------------------------------------------------------------
ProtocolInformation *ProtocolInformation::m_instance = 0;

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
ProtocolInformation::ProtocolInformation() : m_protocol_timeout()
{
	m_instance = this;
}

//------------------------------------------------------------------------------
ProtocolInformation::~ProtocolInformation()
{
	m_instance = 0;

	// kill all the timers
	for (ProtocolTimeoutQueue::const_iterator iter = m_protocol_timeout.begin(); iter != m_protocol_timeout.end(); ++iter)
	{
		KillTimer(NULL, (*iter).second);
	}
}

void CALLBACK ProtocolInformation::TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	ProtocolTimeout pt = m_instance->m_protocol_timeout.front();    
	
	KillTimer(NULL, pt.second);

	m_instance->m_protocol_timeout.pop_front();
}

//------------------------------------------------------------------------------
void ProtocolInformation::disable(const char *protocol)
{
	if (NULL == protocol)
	{
		return;
	}

	const unsigned int TIMEOUT = 10000;

	unsigned int t  = SetTimer(NULL, (UINT_PTR)this, TIMEOUT, TimerProc);
	m_protocol_timeout.push_back(std::make_pair(protocol, t));
}

//------------------------------------------------------------------------------
bool ProtocolInformation::isDisabled(const char *protocol) const
{
	if (NULL == protocol)
	{
		return false;
	}

	// iterate through the list and see if the protocol has a timer callback
	for (ProtocolTimeoutQueue::const_iterator iter = m_protocol_timeout.begin(); iter != m_protocol_timeout.end(); ++iter)
	{
		if (0 == (*iter).first.compare(protocol))
		{
			return true;
		}
	}

	return false;
}

//==============================================================================
