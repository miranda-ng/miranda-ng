#ifndef modern_sync_h__
#define modern_sync_h__

#include "hdr/modern_commonheaders.h"


typedef INT_PTR (*PSYNCCALLBACKPROC)(WPARAM,LPARAM);

int SyncCall(void * vproc, int count, ... );
int SyncCallProxy( PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam, CRITICAL_SECTION * cs = NULL );
HRESULT SyncCallWinProcProxy( PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam, int& nReturn );
HRESULT SyncCallAPCProxy( PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam, int& hReturn );

LRESULT SyncOnWndProcCall( WPARAM wParam );

// Experimental sync caller

int DoCall( PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam );

// Have to be here due to MS Visual C++ does not support 'export' keyword

// 3 params

template<class RET, class A, class B, class C> class PARAMS3
{ 
	typedef RET(*proc_t)(A, B, C);
	proc_t _proc;  A _a; B _b; C _c; RET _ret; 

public:
	PARAMS3( proc_t __proc, A __a, B __b, C __c ): _proc( __proc), _a (__a), _b(__b), _c(__c){};
	static int DoSyncCall( WPARAM wParam, LPARAM lParam )
	{
		PARAMS3 * params = (PARAMS3 *) lParam;	
		params->_ret = params->_proc( params->_a, params->_b, params->_c );
		return 0;
	};
	RET GetResult() { return _ret; }
};

template< class RET, class Ap, class Bp, class Cp, class A, class B, class C> RET Sync( RET(*proc)(Ap, Bp, Cp), A a, B b, C c )
{
	PARAMS3<RET, Ap, Bp, Cp> params( proc, a, b, c );
	DoCall( (PSYNCCALLBACKPROC) PARAMS3<RET, Ap, Bp, Cp>::DoSyncCall, 0, (LPARAM) &params );
	return params.GetResult();
};


// 2 params

template<class RET, class A, class B> class PARAMS2
{ 
	typedef RET(*proc_t)(A, B);
	proc_t _proc;  A _a; B _b; RET _ret; 

public:
	PARAMS2( proc_t __proc, A __a, B __b ): _proc( __proc), _a (__a), _b(__b){};
	static int DoSyncCall( WPARAM wParam, LPARAM lParam )
	{
		PARAMS2 * params = (PARAMS2 *) lParam;	
		params->_ret = params->_proc( params->_a, params->_b );
		return 0;
	};
	RET GetResult() { return _ret; }
};

template< class RET, class Ap, class Bp, class A, class B> RET Sync( RET(*proc)(Ap, Bp), A a, B b )
{
	PARAMS2<RET, Ap, Bp> params( proc, a, b );
	DoCall( (PSYNCCALLBACKPROC) PARAMS2<RET, Ap, Bp>::DoSyncCall, 0, (LPARAM) &params );
	return params.GetResult();
};


// 1 param
template<class RET, class A> class PARAMS1
{ 
	typedef RET(*proc_t)(A);
	proc_t _proc;  A _a; RET _ret; 

public:
	PARAMS1( proc_t __proc, A __a): _proc( __proc), _a (__a){};
	static int DoSyncCall( WPARAM, LPARAM lParam )
	{
		PARAMS1 * params = (PARAMS1 *) lParam;	
		params->_ret = params->_proc( params->_a );
		return 0;
	};
	RET GetResult() { return _ret; }
};

template< class RET, class Ap, class A> RET Sync( RET(*proc)(Ap), A a )
{
	PARAMS1<RET, Ap> params( proc, a );
	DoCall( (PSYNCCALLBACKPROC) PARAMS1<RET, Ap>::DoSyncCall, 0, (LPARAM) &params );
	return params.GetResult();
};

#endif // modern_sync_h__