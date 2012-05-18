#ifndef __TSTRING_H__
# define __TSTRING_H__

#include <windows.h>
#include <tchar.h>
#include <string>


namespace std {
	typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring;
}


#endif // __TSTRING_H__
