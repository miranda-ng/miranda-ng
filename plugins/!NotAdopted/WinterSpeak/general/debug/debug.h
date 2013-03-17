#ifndef guard_general_debug_debug_h
#define guard_general_debug_debug_h
//==============================================================================

//------------------------------------------------------------------------------
#ifdef _DEBUG
    
	#include <iostream>

	#define CLASSCERR(string) (std::cerr << this << " " << string << std::endl)
    #define CERR(string) (std::cerr << "         " << string << std::endl)

#else

	#define CLASSCERR(string)
    #define CERR(string)

#endif

//==============================================================================
#endif