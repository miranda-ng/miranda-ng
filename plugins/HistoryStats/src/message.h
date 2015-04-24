#if !defined(HISTORYSTATS_GUARD_MESSAGE_H)
#define HISTORYSTATS_GUARD_MESSAGE_H

#include "stdafx.h"
#include "_consts.h"

#include "utils.h"

/*
 * Message
 */

class Message
	: private pattern::NotCopyable<Message>
{
private:
	enum Flags {
		// internal
		Raw          = 0x01,
		WithoutLinks = 0x02,

		// very internal
		PtrIsNonT    = 0x10,
		PtrIsUTF8    = 0x20,
	};

private:
	bool m_bOutgoing;
	DWORD m_Timestamp;
	ext::string::size_type m_nLength;
	const void* m_RawSource;
	int m_Available;
	ext::string m_Raw;
	ext::string m_WithoutLinks;
	bool m_bStripRawRTF;
	bool m_bStripBBCodes;

private:
	void makeRawAvailable();
	void stripRawRTF();
	void stripBBCodes();
	void filterLinks();

public:
	explicit Message(bool bStripRawRTF, bool bStripBBCodes)
		: m_RawSource(NULL)
		, m_Available(0)
		, m_bStripRawRTF(bStripRawRTF)
		, m_bStripBBCodes(bStripBBCodes)
	{
	}

	// assigning data
	void assignInfo(bool bOutgoing, DWORD localTimestamp)
	{
		m_bOutgoing = bOutgoing;
		m_Timestamp = localTimestamp;
	}

	void assignText(const TCHAR* msg, ext::string::size_type len)
	{
		m_RawSource = msg;
		m_nLength = len;
		m_Available = 0;
	}

#if defined(_UNICODE)
	void assignText(const char* msg, ext::string::size_type len)
	{
		m_RawSource = msg;
		m_nLength = len;
		m_Available = PtrIsNonT;
	}
#endif // _UNICODE

	void assignTextFromUTF8(const char* msg, ext::string::size_type len)
	{
		m_RawSource = msg;
		m_nLength = len;
		m_Available = PtrIsUTF8;
	}

	// retrieving always available data
	bool isOutgoing()
	{
		return m_bOutgoing;
	}
	
	DWORD getTimestamp()
	{
		return m_Timestamp;
	}
	
	// retrieving on-demand data
	ext::string::size_type getLength()
	{
		return (!m_bStripBBCodes && !m_bStripRawRTF) ? m_nLength : getRaw().length();
	}

	const ext::string& getRaw()
	{
		if (!(m_Available & Raw))
		{
			makeRawAvailable();
		}

		return m_Raw;
	}

	const ext::string& getWithoutLinks()
	{
		if (!(m_Available & WithoutLinks))
		{
			filterLinks();
		}
		
		return m_WithoutLinks;
	}
};

#endif // HISTORYSTATS_GUARD_MESSAGE_H
