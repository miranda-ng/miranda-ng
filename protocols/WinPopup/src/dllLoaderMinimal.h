/*
 * This software is the original work of DKLT.
 * Copyright (c) 2002 DKLT. All rights reserved.
 * email: dtung@eng.monash.edu.au
 *
 */
/* 
 * Permission to make digital or hard copies of all or part of this work for personal 
 * or classroom use is granted without fee provided that copies are not distributed 
 * for profit or commercial advantage.
 */

#ifndef LOADDLL_H
#define LOADDLL_H

/// proof of concept code follows
///////
/////// class DllLoader and class DllFunctor<...> 
/////// 
/////// Two classes are designed to provide the functionality to load a function, 
/////// using the "function name" as an identifier, from a Win32 .dll file.
/////// Sample code are attached at the end of this file.
///////
/////// -04Oct2003 <Sat> 11.52pm
///////   reworked article and sample code to be posted on codeproject.com
///////   better defined behaviours with refined logics idealistic goals yet to be completed 
///////
/////// -29Mar2003 <Sat> 1.47pm
///////   Polishing code for public release. minimizing code size, removing redundent 
///////   comments, eliminating deprecated funcs.
///////
/////// -29Mar2003 <Sat> 12.47am
///////   Revising the src tree. Using redundent code to achieve src level compatibility
///////   (ie: same set of calls for both funcs attached with apps or reside in dlls)
///////
/////// -12Nov2002 <Mon> 1.35am
///////   My first attempt to tidy the code for another public release. 
///////
/////// -14Oct2002 <Mon> 1.40am
///////   created and tested briefly inside \DKLT TestApp\
///////
///////
///////

//template <char* DllName>

///
/// No error message for you
///
#ifndef ERRORMSG
#define DEBUGMSG(aMesg) ;
#define ERRORMSG(aMesg) ;	
#endif						

///
/// protos
///
class DllLoader;
template <typename T> class DllFunctor;


/*
//++++++++++++++++++++++++++++++++++++++++++++++++++++
// For Current release, you write code like these 
//++++++++++++++++++++++++++++++++++++++++++++++++++++
	///
	/// u can load a dll function in two differnt ways
	///
	DllLoader dll("testDll.dll", false);	
	DllFunctor<int(*)(int,int)> fAdd("Add");
	fAdd.SetDll(dll);

	int b;
	b = fSub()(b,1);		// with delay load, but not src level compatible

	OR


	DllLoader dll("testDll.dll");
	FuncPtrType( int(*)(int,int) ) Add;
	dll.LoadFunc(Add,"Add");
	int a=90;
	a = Add(a,1);			// src level compatible, but no delay load



//++++++++++++++++++++++++++++++++++++++++++++++++++++
// For previous release, you write code like these 
//++++++++++++++++++++++++++++++++++++++++++++++++++++

	//
	// sample code for demonstrating class DllLoader {...}; and DllFunctor<...> {...};
	//
	FuncPtrType( int(*)(int) ) a;							// define a new variable "a" of type "int(*)(int)"

	DllLoader Dlldshow("dlls.dll");							// expect a dll name "dlls.dll"
	Dllshow.LoadFunc( a, "test");							// load func named "test" from dll file
	int i =a(10);

//++++++++++++++++++++++++++++++++++++++++++++++++++++
// For initial release, you write code like these 
//++++++++++++++++++++++++++++++++++++++++++++++++++++
	This version enables a delay-load behaviour. Note the double ()() call on last line.

	//
	// sample code for demonstrating class DllLoader {...}; and DllFunctor<...> {...};
	//

	DllLoader Dlldshow("dlls.dll");
	DllFunctor<void(*)(POINT*)> update("UpdatePoint");
	
	Dlldshow.find(update);

	update() (&pt);


*/

/*
	A little comment here
	
	My previous attempts to use operator()(...) and operator FuncPtrType () with MSVC
    failed, where FuncPtrType is a function pointer typedef. That technique, enables
	more control over a functor object. ie: can implement delay load among many exciting
	features. That technique, however, works with g++ line of compilers.

	This current implementation is design for use with MSVC line of compilers only. 

	It seems, from the MSVC compiler error message, that "operator FuncPtrType ()" is 
	never a candidate function, not to mention viability. I guess this is how they 
	design and implemented MSVC6. ".net" version doesnt "evaluate" 
	"operator FuncPtrType()" properly as well.

	- DKLT March 2003
*/
	

//////
//////++++++++++++++++++++++++++++++++++++++++++++++++++
////// This marco is for performing the following task...  GoodJob! creative man!!
//////++++++++++++++++++++++++++++++++++++++++++++++++++
////// normally, u define a function pointer variable this way
////// 
//////    int (*c) (int) = test;	// c pointing to "int test(int) {...}"
////// 
////// This marco enables u define a function pointer this way
////// 
//////    FuncPtrType( int(*)(int) ) c =test;
//////
//////
////// took me a while to come up with this one.
//////
////// - DKLT 2003 March

template <typename FuncTypeTTT>
struct TypeOnlyStruct {
typedef FuncTypeTTT FuncType;
};

#define FuncPtrType(funcType) \
	TypeOnlyStruct<funcType>::FuncType 

//////
////// potential problems
////// - an instantiation for every differnt type on the template class
////// thus bloated executable? need to fully test it out. not sure about
////// behaviour at this stage.
////// - DKLT March 2003


//////
////// class DllLoader {...}
//////   -init a dll file with LoadLibrary() so that its mapped into dll memory 
//////	  space. this class is designed to use with class DllFunctor<...>.
//////
//////
/////////////////////////////////////////////////////////
class DllLoader
{
/////////////////////////////////////////////////////////

private:
	TCHAR dllName[ MAX_PATH ];

public:
	 HINSTANCE dll;

	 DllLoader (LPCTSTR n, bool loadNow = true) :
		dll(0)
	 {
		 lstrcpy( dllName, n );
		 if (loadNow)
			 LoadLibrary();
	 }
	 ~DllLoader ()
	 {
		 FreeLibrary();
	 }

	// test to see if dll is loaded already
	operator bool () const
	{
		return (dll != 0);
	}

//  FuncTypePtr(int(*)(int)) a;
//	Dllshow.LoadFunc( a, "test") ;
//	int i =a(10);

	/// This is my latest implementation
	///----------------------------------------------------------
	/// public:
	/// template <typename FuncTTT>
	///          DllLoader::LoadFunc(FuncTTT& c, string fNameStr) 
	///----------------------------------------------------------
	///	This function loads a function named "fNameStr" from a DllLoader object
	/// and put the address of that function into c.
	///
	///  - template type is derived(deduced) from 1st param.
	///
	///note: bloated executable is possible
	template <typename FuncTTT>
	//--------------------------
	FuncTTT LoadFunc(FuncTTT& c, LPCSTR fNameStr) {
	//--------------------------
		FuncTTT fPtr;

		// existing lib loaded?
		if (!dll)
			if (!this->LoadLibrary())
				return (FuncTTT) NULL;

		// load func from dll
		fPtr =(FuncTTT)GetProcAddress (
			  dll,			// handle to DLL module
			  fNameStr		// name of function
			);
		if (!fPtr) {
			/// return a pointer to a base generic function would be good. ie: ERROR prompt
			return (FuncTTT) NULL;
		}
		c = fPtr;
		return fPtr;
	}

public:
	///
	/// decrement dll ref count via win32 ::FreeLibrary(...)
	///
	//--------------------------
	void FreeLibrary() {
	//--------------------------
		if (dll) {	
			::FreeLibrary(dll);    
			dll=0;
		}
	}

public:
	///
	/// find the dll file and attempt to load it
	///
	//------------------------
	bool LoadLibrary (HINSTANCE hInstance = NULL) {
	//------------------------

		// existing lib loaded?
		if (dll !=0 ) 
			this->FreeLibrary();

		// load from:
		// 1. The directory from which the application loaded. 
		// 2. The current directory. 
		// 3. The Windows system directory.
		// 4. The Windows directory.
		// 5. The directories that are listed in the PATH environment variable. 
		dll = ::LoadLibrary( dllName );
		if ( ! dll )
		{
			// 6. The module directory (if dll).
			if ( hInstance )
			{
				TCHAR self[ MAX_PATH ];
				GetModuleFileName( hInstance, self, MAX_PATH );
				lstrcpy( lstrnrchr( self, _T('\\'), lstrlen( self )) + 1, dllName );
				dll = ::LoadLibrary( self );
			}
			if ( ! dll )
			{
				return false;
			}
		}
		return true;
	}

	////// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	////// All class functions below are for backward compatibility....
	////// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	////// U may delete all of them if u dont need them
	//////
	///
	/// find() is deprecated. Do not use it anymore.
	/// locate the functor inside a dll. let a DllFunctor object to do the job 
	/// instead... double dispatch??
	///
public:template <typename QQ>
	bool find(DllFunctor<QQ>& functor) {
		return functor.LoadFromDll(this);
	}

};

///
/// DllFunctor<> is templated on the function type
///
template <typename FuncPtrType>
class DllFunctor {
	FuncPtrType fp;		// pointer to dll function
	DllLoader* dll;		// which dllLoader to load from
	CString fname;	// name of function as char array

public:
	DllFunctor(FuncPtrType f, DllLoader* d=0): fp(f), dll(d) {;}
	DllFunctor(LPCTSTR n): fname(n),fp(0), dll(0) {;}
	FuncPtrType operator()() { 
		if (!*dll) {
			if (!dll->LoadLibrary())
				return (FuncPtrType) NULL;
		}
		if (fp == 0) {
			dll->LoadFunc (fp, fname.c_str());
		}
		return fp;
	}
	void SetDll(DllLoader& d) { dll=&d; }	
};

#endif
