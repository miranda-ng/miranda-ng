#include "_globals.h"
#include "message.h"

#include <algorithm>

/*
 * Message
 */

void Message::makeRawAvailable()
{
	do
	{
#if defined(MU_WIDE)
		if (m_Available & PtrIsNonT)
		{
			m_Raw = utils::fromA(ext::a::string(reinterpret_cast<const mu_ansi*>(m_RawSource), m_nLength));
			m_Available |= Raw;

			break;
		}
#endif // MU_WIDE

		if (m_Available & PtrIsUTF8)
		{
			m_Raw = utils::fromUTF8(reinterpret_cast<const mu_ansi*>(m_RawSource));
			m_Available |= Raw;

			break;
		}

		m_Raw.assign(reinterpret_cast<const mu_text*>(m_RawSource), m_nLength);
		m_Available |= Raw;
	} while(false);

	if (m_bStripRawRTF)
	{
		stripRawRTF();
	}

	if (m_bStripBBCodes)
	{
		stripBBCodes();
	}
}

void Message::stripRawRTF()
{
	if (m_Raw.substr(0, 6) == muT("{\\rtf1"))
	{
		m_Raw = RTFFilter::filter(m_Raw);
	}
}

void Message::stripBBCodes()
{
	static const mu_text* szSimpleBBCodes[][2] = {
		{ muT("[b]"), muT("[/b]") },
		{ muT("[u]"), muT("[/u]") },
		{ muT("[i]"), muT("[/i]") },
		{ muT("[s]"), muT("[/s]") },
	};

	static const mu_text* szParamBBCodes[][2] = {
		{ muT("[url=")  , muT("[/url]")   },
		{ muT("[color="), muT("[/color]") },
	};

	// convert raw string to lower case
	ext::string strRawLC = utils::toLowerCase(m_Raw);

	// remove simple BBcodes
	array_each_(i, szSimpleBBCodes)
	{
		const mu_text* szOpenTag = szSimpleBBCodes[i][0];
		const mu_text* szCloseTag = szSimpleBBCodes[i][1];

		int lenOpen = ext::strfunc::len(szOpenTag);
		int lenClose = ext::strfunc::len(szCloseTag);
		
		ext::string::size_type posOpen = 0;
		ext::string::size_type posClose = 0;

		while (true)
		{
			if ((posOpen = strRawLC.find(szOpenTag, posOpen)) == ext::string::npos)
			{
				break;
			}

			if ((posClose = strRawLC.find(szCloseTag, posOpen + lenOpen)) == ext::string::npos)
			{
				break;
			}

			strRawLC.erase(posOpen, lenOpen);
			strRawLC.erase(posClose - lenOpen, lenClose);

			// fix real string
			m_Raw.erase(posOpen, lenOpen);
			m_Raw.erase(posClose - lenOpen, lenClose);
		}
	}

	// remove BBcodes with parameters
	array_each_(i, szParamBBCodes)
	{
		const mu_text* szOpenTag = szParamBBCodes[i][0];
		const mu_text* szCloseTag = szParamBBCodes[i][1];
		
		int lenOpen = ext::strfunc::len(szOpenTag);
		int lenClose = ext::strfunc::len(szCloseTag);
		
		ext::string::size_type posOpen = 0;
		ext::string::size_type posOpen2 = 0;
		ext::string::size_type posClose = 0;

		while (true)
		{
			if ((posOpen = strRawLC.find(szOpenTag, posOpen)) == ext::string::npos)
			{
				break;
			}

			if ((posOpen2 = strRawLC.find(muC(']'), posOpen + lenOpen)) == ext::string::npos)
			{
				break;
			}

			if ((posClose = strRawLC.find(szCloseTag, posOpen2 + 1)) == ext::string::npos)
			{
				break;
			}

			strRawLC.erase(posOpen, posOpen2 - posOpen + 1);
			strRawLC.erase(posClose - posOpen2 + posOpen - 1, lenClose);

			// fix real string
			m_Raw.erase(posOpen, posOpen2 - posOpen + 1);
			m_Raw.erase(posClose - posOpen2 + posOpen - 1, lenClose);
		}
	}
}

void Message::filterLinks()
{
	static const mu_text* szSpaces = muT(" \r\r\n");
	static const mu_text* szPrefixes = muT("([{<:\"'");
	static const mu_text* szSuffixes = muT(".,:;!?)]}>\"'");
	static const mu_text* szValidProtocol = muT("abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	static const mu_text* szValidHost = muT(".-abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	// init with raw text
	m_WithoutLinks = getRaw();

	ext::string& msg = m_WithoutLinks;
	ext::string::size_type pos = -1;

	// detect: protocol://[user[:password]@]host[/path]
	while (true)
	{
		if ((pos = msg.find(muT("://"), pos + 1)) == ext::string::npos)
		{
			break;
		}

		// find start of URL
		ext::string::size_type pos_proto = msg.find_last_not_of(szValidProtocol, pos - 1);

		(pos_proto == ext::string::npos) ? pos_proto = 0 : ++pos_proto;

		if (pos_proto < pos)
		{
			// find end of URL
			ext::string::size_type pos_last = msg.find_first_of(szSpaces, pos + 3);

			(pos_last == ext::string::npos) ? pos_last = msg.length() - 1 : --pos_last;

			// filter suffixes (punctuation, parentheses, ...)
			if (ext::strfunc::chr(szSuffixes, msg[pos_last]))
			{
				--pos_last;
			}

			// find slash: for host name validation
			ext::string::size_type pos_slash = msg.find(muC('/'), pos + 3);

			if (pos_slash == ext::string::npos || pos_slash > pos_last)
			{
				pos_slash = pos_last + 1;
			}

			// find at: for host name validation
			ext::string::size_type pos_at = msg.find(muC('@'), pos + 3);

			if (pos_at == ext::string::npos || pos_at > pos_slash)
			{
				pos_at = pos + 2;
			}

			// check for valid host (x.x)
			if (pos_slash - pos_at > 3)
			{
				ext::string::size_type pos_invalid = msg.find_first_not_of(szValidHost, pos_at + 1);

				if (pos_invalid == ext::string::npos || pos_invalid >= pos_slash)
				{
					if (std::count(msg.begin() + pos_at + 1, msg.begin() + pos_slash, muC('.')) >= 1)
					{
						ext::string link = msg.substr(pos_proto, pos_last - pos_proto + 1);

						// remove extracted link from message text
						msg.erase(pos_proto, link.length());
						pos = pos_last - link.length();

						// TODO: put link in list
					}
				}
			}
		}
	}
	
	// detect: www.host[/path]
	pos = -1;

	while (true)
	{
		if ((pos = msg.find(muT("www."), pos + 1)) == ext::string::npos)
		{
			break;
		}

		// find end of URL
		ext::string::size_type pos_last = msg.find_first_of(szSpaces, pos + 4);

		(pos_last == ext::string::npos) ? pos_last = msg.length() - 1 : --pos_last;

		// filter suffixes (punctuation, parentheses, ...)
		if (ext::strfunc::chr(szSuffixes, msg[pos_last]))
		{
			--pos_last;
		}

		// find slash: for host name validation
		ext::string::size_type pos_slash = msg.find(muC('/'), pos + 4);

		if (pos_slash == ext::string::npos || pos_slash > pos_last)
		{
			pos_slash = pos_last + 1;
		}

		// find at: for host name validation
		ext::string::size_type pos_at = pos + 3;

		// check for valid host (x.x)
		if (pos_slash - pos_at > 3)
		{
			ext::string::size_type pos_invalid = msg.find_first_not_of(szValidHost, pos_at + 1);

			if (pos_invalid == ext::string::npos || pos_invalid >= pos_slash)
			{
				if (std::count(msg.begin() + pos_at + 1, msg.begin() + pos_slash, muC('.')) >= 1)
				{
					ext::string link = muT("http://") + msg.substr(pos, pos_last - pos + 1);

					// remove extracted link from message text
					msg.erase(pos, link.length() - 7);
					pos = pos_last - (link.length() - 7);

					// TODO: put link in list
				}
			}
		}
	}

	// detect: user@host
	pos = -1;

	while (true)
	{
		if ((pos = msg.find(muC('@'), pos + 1)) == ext::string::npos)
		{
			break;
		}

		if (pos > 0 && pos < msg.length() - 1)
		{
			// find end of address
			ext::string::size_type pos_last = msg.find_first_not_of(szValidHost, pos + 1);

			(pos_last == ext::string::npos) ? pos_last = msg.length() - 1 : --pos_last;

			// filter suffixes (punctuation, parentheses, ...)
			if (ext::strfunc::chr(szSuffixes, msg[pos_last]))
			{
				--pos_last;
			}

			// find start of address
			ext::string::size_type pos_first = msg.find_last_of(szSpaces, pos - 1);

			(pos_first == ext::string::npos) ? pos_first = 0 : ++pos_first;

			// filter prefixes (punctuation, parentheses, ...)
			if (ext::strfunc::chr(szPrefixes, msg[pos_first]))
			{
				++pos_first;
			}

			// check for valid host (x.x)
			if (pos_first < pos && pos_last - pos >= 3)
			{
				if (std::count(msg.begin() + pos + 1, msg.begin() + pos_last + 1, muC('.')) >= 1)
				{
					ext::string link = msg.substr(pos_first, pos_last - pos_first + 1);

					// remove extracted link from message text
					msg.erase(pos_first, link.length());
					pos = pos_last - (link.length());

					// prepend "mailto:" if missing
					if (link.substr(0, 7) != muT("mailto:"))
					{
						link.insert(0, muT("mailto:"));
					}

					// TODO: put link in list
				}
			}
		}
	}

	m_Available |= WithoutLinks;
}
