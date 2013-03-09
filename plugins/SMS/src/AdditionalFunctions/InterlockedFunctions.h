#if !defined(AFX_INTERLOCKED_FUNCTIONS__H__INCLUDED_)
#define AFX_INTERLOCKED_FUNCTIONS__H__INCLUDED_

#ifdef InterlockedCompareExchange
#ifndef InterlockedAnd
LONG FORCEINLINE InterlockedAnd(LONG volatile *Destination,LONG Value)
{
	LONG Old;

	do
	{
		Old=*Destination;
	}while(InterlockedCompareExchange(Destination,(Old&Value),Old)!=Old);
return(Old);
}
#endif //InterlockedAnd


#ifndef InterlockedOr
LONG FORCEINLINE InterlockedOr(LONG volatile *Destination,LONG Value)
{
	LONG Old;

	do
	{
		Old=*Destination;
	}while(InterlockedCompareExchange(Destination,(Old|Value),Old)!=Old);
return(Old);
}
#endif //InterlockedOr


#ifndef InterlockedXor
LONG FORCEINLINE InterlockedXor(LONG volatile *Destination,LONG Value)
{
	LONG Old;

	do
	{
		Old=*Destination;
	}while(InterlockedCompareExchange(Destination,(Old^Value),Old)!=Old);
return(Old);
}
#endif //InterlockedXor
#endif //InterlockedCompareExchange





#ifdef InterlockedCompareExchange64
#ifndef InterlockedAnd64
LONGLONG FORCEINLINE InterlockedAnd64(LONGLONG volatile *Destination,LONGLONG Value)
{
	LONGLONG Old;

	do
	{
		Old=*Destination;
	}while(InterlockedCompareExchange64(Destination,(Old&Value),Old)!=Old);
return(Old);
}
#endif //InterlockedAnd64


#ifndef InterlockedOr64
LONGLONG FORCEINLINE InterlockedOr64(LONGLONG volatile *Destination,LONGLONG Value)
{
	LONGLONG Old;

	do
	{
		Old=*Destination;
	}while(InterlockedCompareExchange64(Destination,(Old|Value),Old)!=Old);
return(Old);
}
#endif //InterlockedOr64


#ifndef InterlockedXor64
LONGLONG FORCEINLINE InterlockedXor64(LONGLONG volatile *Destination,LONGLONG Value)
{
	LONGLONG Old;

	do
	{
		Old=*Destination;
	}while(InterlockedCompareExchange64(Destination,(Old^Value),Old)!=Old);
return(Old);
}
#endif //InterlockedXor64
#endif //InterlockedCompareExchange64





#ifndef InterlockedIncrementPointer
#if defined(_WIN64)
#define InterlockedIncrementPointer(lpAddend) (LPVOID)InterlockedIncrement64((LONGLONG volatile*)lpAddend)
#else
#define InterlockedIncrementPointer(lpAddend) (LPVOID)InterlockedIncrement((LONG volatile*)lpAddend)
#endif
#endif //InterlockedIncrementPointer


#ifndef InterlockedDecrementPointer
#if defined(_WIN64)
#define InterlockedDecrementPointer(lpAddend) (LPVOID)InterlockedDecrement64((LONGLONG volatile*)lpAddend)
#else
#define InterlockedDecrementPointer(lpAddend) (LPVOID)InterlockedDecrement((LONG volatile*)lpAddend)
#endif
#endif //InterlockedDecrementPointer





#endif // !defined(AFX_INTERLOCKED_FUNCTIONS__H__INCLUDED_)
