#ifndef modern_callproc_h__
#define modern_callproc_h__

namespace call {
#include <windows.h>

//////////////////////////////////////////////////////////////////////////
// USE AS 
// ret = call::sync( proc, param1, param2 ... etc)
//////////////////////////////////////////////////////////////////////////
// internal realization
enum ASYNC_T { ASYNC = 0, SYNC };
int __ProcessCall( class __baseCall * pStorage, ASYNC_T async );

class __baseCall { public: virtual int __DoCallStorageProc() =0;  virtual ~__baseCall() {}; };
template< class R > class _callParams0 : public __baseCall
{
public:
    R(*_proc)();
    _callParams0( R (*proc)() ) : _proc(proc){};
    int __DoCallStorageProc() { return (int)_proc(); }
};

template<> class _callParams0<void> : public __baseCall
{
public:
    void(*_proc)();
    _callParams0( void (*proc)() ) : _proc(proc){};
    int __DoCallStorageProc() { _proc(); return 0; }
};

template< class R, class A> class _callParams1 : public __baseCall
{
public:
    R(*_proc)(A); A _a; 
    _callParams1( R(*proc)(A), A a) : _proc(proc), _a(a) {};
    int __DoCallStorageProc() { return (int)_proc(_a);  }
};

template<class A> class _callParams1<void, A> : public __baseCall
{
public:
    void(*_proc)(A);  A _a; 
    _callParams1( void(*proc)(A), A a) : _proc(proc), _a(a) {};
    int __DoCallStorageProc() { _proc(_a); return 0;  }
};

template< class R, class A, class B> class _callParams2 : public __baseCall
{
public:
    R (*_proc)(A, B); A _a; B _b;
    _callParams2( R (*proc)(A, B), A a, B b) : _proc(proc), _a(a), _b(b) {};
    int __DoCallStorageProc() { return (int)_proc(_a, _b); }
};

template< class A, class B> class _callParams2<void, A, B> : public __baseCall
{
public:
    void (*_proc)(A, B); A _a; B _b;
    _callParams2( void (*proc)(A, B), A a, B b) : _proc(proc), _a(a), _b(b) {};
    int __DoCallStorageProc() { _proc(_a, _b); return 0; }
};

template< class R, class A, class B, class C>   class _callParams3 : public __baseCall
{
public:
    R (*_proc)(A, B, C); A _a; B _b; C _c;
    _callParams3( R (*proc)(A, B, C), A a, B b, C c ) : _proc(proc), _a(a), _b(b), _c(c) {};
    int __DoCallStorageProc() { return (int)_proc(_a,_b,_c); }
};

template< class A, class B, class C>   class _callParams3<void, A, B, C> : public __baseCall
{
public:
    void (*_proc)(A, B, C); A _a; B _b; C _c;
    _callParams3( void (*proc)(A, B, C), A a, B b, C c ) : _proc(proc), _a(a), _b(b), _c(c) {};
    int __DoCallStorageProc() { _proc(_a,_b,_c); return 0;}
};

template < class R > R __DoCall( R(*__proc)(), ASYNC_T sync_mode )
{  
    typedef _callParams0< R > callClass;
    callClass * storage = new callClass( __proc );
    return (R) call::__ProcessCall( storage, sync_mode );
};

template < class R, class A > R __DoCall( R(*__proc)( A ), A a, ASYNC_T sync_mode )
{
    typedef _callParams1< R, A > callClass;
    callClass * storage = new callClass( __proc, a );
    return (R)__ProcessCall( storage, sync_mode );
};


template < class R, class A, class B > R __DoCall( R(*__proc)( A, B ), A a, B b, ASYNC_T sync_mode )
{
    typedef _callParams2< R, A, B > callClass;
    callClass * storage = new callClass( __proc, a, b);
    return (R)__ProcessCall( storage, sync_mode );
};


template < class R, class A, class B, class C > R __DoCall( R(*__proc)( A, B, C ), A a, B b, C c, ASYNC_T sync_mode )
{
    typedef _callParams3< R, A, B, C > callClass;
    callClass * storage = new callClass( __proc, a, b, c);
    return (R)__ProcessCall( storage, sync_mode );
};


template < class R > R sync( R(*_proc)() )
{ return __DoCall(_proc, SYNC); };
template < class R, class A > R sync( R(*_proc)( A ), A a )
{ return __DoCall(_proc, a, SYNC); };
template < class R, class A, class B > R sync( R(*_proc)( A,B), A a, B b )
{ return __DoCall(_proc, a, b, SYNC); };
template < class R, class A, class B, class C > R sync( R(*_proc)( A,B,C ), A a, B b, C c)
{ return __DoCall(_proc, a, b, c, SYNC); };
template < class R > int async( R(*_proc)() )
{ return __DoCall(_proc, ASYNC); };
template < class R, class A > R async( R(*_proc)( A ), A a )
{ return __DoCall(_proc, a, ASYNC); };
template < class R, class A, class B > R async( R(*_proc)( A,B), A a, B b )
{ return __DoCall(_proc, a, b, ASYNC); };
template < class R, class A, class B, class C > R async( R(*_proc)( A,B,C ), A a, B b, C c)
{ return __DoCall(_proc, a, b, c, ASYNC); };
}; // namespace call
#endif // modern_callproc_h__
