#ifndef guard_general_debug_debug_file_h
#define guard_general_debug_debug_file_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#ifdef _DEBUG
//==============================================================================

#include <fstream>

//------------------------------------------------------------------------------
template <class charT, class traits = std::char_traits<charT> >
class FileStreamBuf : public std::basic_streambuf<charT, traits>
{
  public:
	FileStreamBuf()
        :
        m_output()
	{
//        m_output.open("speak_output.txt", std::ios_base::app);
        m_output.open("speak_output.txt");
	}

    ~FileStreamBuf()
    {
        m_output.close();
    }

  protected:
	virtual int_type overflow(int_type c)
	{
		if (!traits_type::eq_int_type(c, traits_type::eof()))
		{
			m_buffer += c;

			if (c == '\n')
			{
                m_output << m_buffer.c_str();
                m_output.flush();

                m_buffer.erase();
			}

			return c;
		}

		return traits_type::not_eof(c);
	}

  private:
	// assignment and copy are undefined in iostreams and so we hide them.
	FileStreamBuf(const FileStreamBuf &);
	FileStreamBuf & operator=(const FileStreamBuf &);

    std::ofstream m_output;
	std::string   m_buffer;
};

//------------------------------------------------------------------------------
class DebugFile : public std::ostream
{
  public:
	DebugFile() 
		: 
		std::basic_ostream<char>(&m_stream_buf)
	{
		// redirect cerr to our debug window
		std::cerr = *this;
	}

	virtual ~DebugFile()
	{
		m_stream_buf.pubsync();
	}

  private:
	FileStreamBuf<char> m_stream_buf;

	DebugFile(const DebugFile &);
	DebugFile & operator=(const DebugFile &);
};

//==============================================================================
//
//  Summary     : Output debug to a file
//
//  Description : Provides a target to place our debugging information
//
//==============================================================================

#endif

#endif