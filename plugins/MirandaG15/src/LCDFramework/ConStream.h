//
// ConStream.h: interface for the ConStream class.
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
// This header file defines the entire interface to the ConStream class.
// ConStream is a class that works like a standard iostream, and writes 
// to a console window when created in a Win32 application. You can only
// have a single console window open at a time, so only a single ConStream
// class is ordinarily created in an application. ConStream is compatible with
// Windows 9x and Windows NT. It uses Unicode text if the _UNICODE macro
// is defined. It must be linked with source code found in ConStream.cpp
//
#if !defined( _CONSTREAM_H )
#define _CONSTREAM_H

//
// Note that ConStream uses the standard C++ libraries that ship with
// Visual C++ 5.0 and later, not the older libraries with the .h suffix.
// The library would require some modifications to work with the older
// libraries.
//
#include <iostream>
#include <fstream>

using namespace std;

//
// The ConStream class is derived from what we normally think of as ostream,
// which means you can use standard insertion operators to write to it. Of
// course, this includes insertion operators for user defined classes. At
// all times, a ConStream object is either writing out to to a FILE 
// object attached to the NUL device, or a FILE object attached to a console
// created using the Win32 API. Which of the two is in use depends on whether
// or not the ConStream object has had its Open() or Close() method called.
//
class ConStream 
#ifdef _UNICODE
  : public basic_ostream<wchar_t>
#else
  : public basic_ostream<char>
#endif
{
public :
    ConStream();
    virtual ~ConStream();
    void Open();
    void Close();
protected :
    HANDLE m_hConsole;
#ifdef _UNICODE
    basic_filebuf<wchar_t> *m_FileBuf;
    basic_filebuf<wchar_t> m_Nul;      
#else
    basic_filebuf<char> *m_FileBuf;    
    basic_filebuf<char> m_Nul;        
#endif
    FILE *m_fNul;                     
    FILE *m_fConsole;                 
};

#endif // !defined( _CONSTREAM_H )
      