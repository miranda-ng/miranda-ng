#ifndef guard_aggressive_optimise_h
#define guard_aggressive_optimise_h
//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#ifdef NDEBUG

// we define the WIN32_LEAN_AND_MEAN tag so that the quick windows
// macros and functions are used. Using this should make a slight
// performance improvement (even though we probably won't see it
// in this program).
#define WIN32_LEAN_AND_MEAN

#pragma comment(linker,"/RELEASE")
#pragma comment(linker,"/FILEALIGN:0x200")

#endif

//==============================================================================
//
//  Summary     : aggresively optimise the binary for size
//
//  Description : uses various linker switches to decrease the size of the 
//                binary
//
//==============================================================================

#endif