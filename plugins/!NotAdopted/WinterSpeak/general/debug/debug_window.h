#ifndef guard_general_debug_debug_window_h
#define guard_general_debug_debug_window_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#ifdef _DEBUG
//==============================================================================

#include <windows.h>

#include <iostream>
#include <string>

//------------------------------------------------------------------------------
template <class charT, class traits = std::char_traits<charT> >
class WindowStreamBuf : public std::basic_streambuf<charT, traits>
{
  public:
	WindowStreamBuf()
	{
		// create the window to put our debug into
		m_debug_window = CreateWindow(
			"Listbox", 
			"Debug Window",
			WS_OVERLAPPEDWINDOW | WS_VSCROLL,
			5,
			5,
			400, 
			600,
			(HWND) NULL,
			(HMENU) NULL,
			NULL,
			(LPVOID) NULL);

		ShowWindow(m_debug_window, SW_SHOW);
	}

  protected:
	virtual int_type overflow(int_type c)
	{
		if (!traits_type::eq_int_type(c, traits_type::eof()))
		{
			if (c == '\n')
			{
				// add the string to the window
				SendMessage(m_debug_window, LB_ADDSTRING, 0, 
					(long)m_buffer.c_str());

				// select the last string added
				int c = SendMessage(m_debug_window, LB_GETCOUNT, 0, 
					(long)m_buffer.c_str());
				SendMessage(m_debug_window, LB_SETCURSEL, c - 1, 0);

				m_buffer.erase(m_buffer.begin(), m_buffer.end());
			}
			else
			{
				m_buffer += c;
			}
			
			return c;
		}

		return traits_type::not_eof(c);
	}

  private:
	// assignment and copy are undefined in iostreams and so we hide them.
	WindowStreamBuf(const WindowStreamBuf &);
	WindowStreamBuf & operator=(const WindowStreamBuf &);

	HWND        m_debug_window;
	std::string m_buffer;
};

//------------------------------------------------------------------------------
class DebugWindow : public std::ostream
{
  public:
	DebugWindow() 
		: 
		std::basic_ostream<char>(&m_stream_buf)
	{
		// redirect cerr to our debug window
		std::cerr = *this;
	}

	virtual ~DebugWindow()
	{
		m_stream_buf.pubsync();
	}

  private:
	WindowStreamBuf<char> m_stream_buf;

	DebugWindow(const DebugWindow &);
	DebugWindow & operator=(const DebugWindow &);
};

//==============================================================================
//
//  Summary     : Create a debug window
//
//  Description : Provides a target to place our debugging information
//
//==============================================================================

#endif

#endif