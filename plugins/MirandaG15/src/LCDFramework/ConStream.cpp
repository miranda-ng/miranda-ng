//
// ConStream.cpp: implementation of the ConStream class.
//
// Mark Nelson, October 17, 1998
//
// The ConStream class creates a C++ ostream object that can be 
// used in Win32 C++ programs to write to a console window, often
// for debugging purposes. This code is demonstrated in the 
// ConStreamDemo MFC program, and is fully explained in the 
// accompanying Dr. Dobb's Journal article. For more details,
// see the article online at http://www.dogma.net/markn
//

//
// Implementation of this class requires that you link in the IO.H
// and FCNTL.H header files from the standard C library. You should
// be able to substitute <cio> and <cfcnt> for these two includes.
// stdafx.h is here because the code is being used in an MFC application
// 

#include "stdafx.h"
#include <io.h>
#include <fcntl.h>

#include "ConStream.h"

//
// The ConStream constructor initializes the object to point to the 
// NUL device. It does this by calling two consecutive constructors.
// First, the member variable m_Nul is initialized with a FILE object
// created by opening device "nul", the bit bucket. Second, the base
// class constructor is called with a reference to m_Nul, which is
// an ofstream object. This sets up ConStream so that it will direct
// its output to the given file.
//
ConStream::ConStream() : m_Nul( m_fNul = fopen( "nul", "w" ) ), m_fConsole(nullptr),
#ifdef _UNICODE
                         basic_ostream<wchar_t>( &m_Nul ) 
#else
                         basic_ostream<char>( &m_Nul ) 
#endif
{
    m_FileBuf = nullptr;
    m_hConsole = INVALID_HANDLE_VALUE;
}

//
// The ConStream destructor always has to close the m_fNul FILE object
// which was created in the constructor. Even if the Open() method has
// been called and the bit bucket isn't being used, the FILE object is
// still using memory and a system file handle.
//
// If the ConStream object has been opened with a call to member function
// Open(), we have to call the Win32 API function FreeConsole() to close
// the console window. If the console window was open, we also call the
// C fclose() function on the m_fConsole member.
//
ConStream::~ConStream()
{
    delete m_FileBuf;
    if ( m_hConsole != INVALID_HANDLE_VALUE ) {
        FreeConsole();
        fclose( m_fConsole );
    }
    fclose( m_fNul );
}

//
// Opening the stream means doing these things:
//   1) Opening a Win32 console using the Win32 API
//   2) Getting an O/S handle to the console
//   3) Converting the O/S handle to a C stdio file handle
//   4) Converting the C stdio file handler to a C FILE object
//   5) Attaching the C FILE object to a C++ filebuf
//   6) Attaching the filebuf object to this
//   7) Disabling buffering so we see our output in real time.
//
void ConStream::Open()
{
    if ( m_hConsole == INVALID_HANDLE_VALUE ) {
		AllocConsole();
		m_hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
		int handle = _open_osfhandle((INT_PTR)m_hConsole, _O_TEXT );
		m_fConsole = _fdopen( handle, "w" );
#ifdef _UNICODE
		m_FileBuf = new basic_filebuf<wchar_t>( m_fConsole );
#else
		m_FileBuf = new basic_filebuf<char>( m_fConsole );
#endif
		init( m_FileBuf );
		setf(ios::unitbuf);
	}
};

//
// Closing the ConStream is considerably simpler. We just use the
// init() call to attach this to the NUL file stream, then close
// the console descriptors.
//
void ConStream::Close()
{
    if ( m_hConsole != INVALID_HANDLE_VALUE ) {
        init( &m_Nul );
        FreeConsole();
        fclose( m_fConsole );
        m_hConsole = INVALID_HANDLE_VALUE;
    }
};
    