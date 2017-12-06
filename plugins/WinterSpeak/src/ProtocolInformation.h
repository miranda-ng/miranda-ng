#pragma once

#include <deque>
#include <utility>

class ProtocolInformation
{
  public:
	ProtocolInformation();
	~ProtocolInformation();

	void disable(const char *protocol);
	bool isDisabled(const char *protocol) const;

  private:
	typedef std::pair<std::string, unsigned int> ProtocolTimeout;
	typedef std::deque<ProtocolTimeout>          ProtocolTimeoutQueue;

	static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	static ProtocolInformation *m_instance;

	ProtocolTimeoutQueue m_protocol_timeout;
};