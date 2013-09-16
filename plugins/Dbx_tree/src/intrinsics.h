#pragma once

#include <intrin.h>
#include <stdint.h>

#ifdef _M_X64

inline uint32_t XCHG_32(uint32_t volatile & Dest, uint32_t Exchange)
{
	return (uint32_t)_InterlockedExchange((long volatile*)&(Dest), (long)(Exchange));
}
inline int32_t XCHG_32(int32_t volatile & Dest, int32_t Exchange)
{
	return (int32_t)_InterlockedExchange((long volatile*)&(Dest), (long)(Exchange));
}

inline uint64_t XCHG_64(uint64_t volatile & Dest, uint64_t Exchange)
{
	return (uint64_t)_InterlockedExchange64((__int64 volatile*)&Dest, (__int64)Exchange);
}
inline int64_t XCHG_64(int64_t volatile & Dest, int64_t Exchange)
{
	return (int64_t)_InterlockedExchange64((__int64 volatile*)&Dest, (__int64)Exchange);
}

template <typename T>
inline T * XCHG_Ptr(T * volatile & Dest, T * Exchange)
{
	return (T*)_InterlockedExchangePointer((void*volatile*)&Dest, (void*)Exchange);
}


inline uint32_t CMPXCHG_32(uint32_t volatile & Dest, uint32_t Exchange, uint32_t Comperand)
{
	return (uint64_t)_InterlockedCompareExchange((long volatile*)&(Dest), (long)(Exchange), (long)Comperand);
}
inline int32_t CMPXCHG_32(int32_t volatile & Dest, int32_t Exchange, int32_t Comperand)
{
	return (int32_t)_InterlockedCompareExchange((long volatile*)&(Dest), (long)(Exchange), (long)Comperand);
}

inline uint64_t CMPXCHG_64(uint64_t volatile & Dest, uint64_t Exchange, uint64_t Comperand)
{
	return (uint64_t)_InterlockedCompareExchange64((__int64 volatile*)&Dest, (__int64)Exchange, (__int64)Comperand);
}
inline int64_t CMPXCHG_64(int64_t volatile & Dest, int64_t Exchange, int64_t Comperand)
{
	return (int64_t)_InterlockedCompareExchange64((__int64 volatile*)&Dest, (__int64)Exchange, (__int64)Comperand);
}

template <typename T>
inline T * CMPXCHG_Ptr(T * volatile & Dest, T * Exchange, T * Comperand)
{
	return (T*)_InterlockedCompareExchangePointer((void*volatile*)&Dest, (void*)Exchange, (void*)Comperand);
}

inline uint32_t XADD_32(uint32_t volatile & Dest, int32_t Addend)
{
	return (uint32_t)_InterlockedExchangeAdd((long volatile*)&Dest, (long)Addend);
}
inline int32_t XADD_32(int32_t volatile & Dest, int32_t Addend)
{
	return (int32_t)_InterlockedExchangeAdd((long volatile*)&Dest, (long)Addend);
}

inline uint64_t XADD_64(uint64_t volatile & Dest, int64_t Addend)
{
	return (uint64_t)_InterlockedExchangeAdd64((__int64 volatile*)&Dest, (__int64)Addend);
}
inline int64_t XADD_64(int64_t volatile & Dest, int64_t Addend)
{
	return (int64_t)_InterlockedExchangeAdd64((__int64 volatile*)&Dest, (__int64)Addend);
}

inline uint32_t DEC_32(uint32_t volatile & Dest)
{
	return (uint32_t)_InterlockedDecrement((long volatile*)&Dest);
}
inline int32_t DEC_32(int32_t volatile & Dest)
{
	return (int32_t)_InterlockedDecrement((long volatile*)&Dest);
}

inline uint64_t DEC_64(uint64_t volatile & Dest)
{
	return (uint64_t)_InterlockedDecrement64((__int64 volatile*)&Dest);
}
inline int64_t DEC_64(int64_t volatile & Dest)
{
	return (int64_t)_InterlockedDecrement64((__int64 volatile*)&Dest);
}

inline uint32_t INC_32(uint32_t volatile & Dest)
{
	return (uint32_t)_InterlockedIncrement((long volatile*)&Dest);
}
inline int32_t INC_32(int32_t volatile & Dest)
{
	return (int32_t)_InterlockedIncrement((long volatile*)&Dest);
}

inline uint64_t INC_64(uint64_t volatile & Dest)
{
	return (uint64_t)_InterlockedIncrement64((__int64 volatile*)&Dest);
}
inline int64_t INC_64(int64_t volatile & Dest)
{
	return (int64_t)_InterlockedIncrement64((__int64 volatile*)&Dest);
}

inline bool BTS_32(uint32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandset((long volatile*)&Dest, Offset);
}
inline bool BTS_32(int32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandset((long volatile*)&Dest, Offset);
}
inline bool BTS_64(uint64_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandset64((__int64 volatile*)&Dest, Offset);
}
inline bool BTS_64(int64_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandset64((__int64 volatile*)&Dest, Offset);
}


inline bool BTR_32(uint32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandreset((long volatile*)&Dest, Offset);
}
inline bool BTR_32(int32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandreset((long volatile*)&Dest, Offset);
}
inline bool BTR_64(uint64_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandreset64((__int64 volatile*)&Dest, Offset);
}
inline bool BTR_64(int64_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandreset64((__int64 volatile*)&Dest, Offset);
}


inline uint32_t OR_32(uint32_t volatile & Value, uint32_t Operator)
{
	return (uint32_t)_InterlockedOr((long volatile*)&Value, (long)Operator);
}

inline int32_t OR_32(int32_t volatile & Value, int32_t Operator)
{
	return (int32_t)_InterlockedOr((long volatile*)&Value, (long)Operator);
}

inline uint64_t OR_64(uint64_t volatile & Value, uint64_t Operator)
{
	return (uint64_t)_InterlockedOr64((__int64 volatile*)&Value, (__int64)Operator);
}

inline int64_t OR_64(int64_t volatile & Value, int64_t Operator)
{
	return (int64_t)_InterlockedOr64((__int64 volatile*)&Value, (__int64)Operator);
}

inline uint32_t AND_32(uint32_t volatile & Value, uint32_t Operator)
{
	return (uint32_t)_InterlockedAnd((long volatile*)&Value, (long)Operator);
}

inline int32_t AND_32(int32_t volatile & Value, int32_t Operator)
{
	return (int32_t)_InterlockedAnd((long volatile*)&Value, (long)Operator);
}

inline uint64_t AND_64(uint64_t volatile & Value, uint64_t Operator)
{
	return (uint64_t)_InterlockedAnd64((__int64 volatile*)&Value, (__int64)Operator);
}

inline int64_t AND_64(int64_t volatile & Value, int64_t Operator)
{
	return (int64_t)_InterlockedAnd64((__int64 volatile*)&Value, (__int64)Operator);
}


inline uint32_t XOR_32(uint32_t volatile & Value, uint32_t Operator)
{
	return (uint32_t)_InterlockedXor((long volatile*)&Value, (long)Operator);
}

inline int32_t XOR_32(int32_t volatile & Value, int32_t Operator)
{
	return (int32_t)_InterlockedXor((long volatile*)&Value, (long)Operator);
}
inline uint64_t XOR_64(uint64_t volatile & Value, uint64_t Operator)
{
	return (uint64_t)_InterlockedXor64((__int64 volatile*)&Value, (__int64)Operator);
}

inline int64_t XOR_64(int64_t volatile & Value, int64_t Operator)
{
	return (int64_t)_InterlockedXor64((__int64 volatile*)&Value, (__int64)Operator);
}


inline uint32_t BSWAP_32(uint32_t Value)
{
	return _byteswap_ulong(Value);
}

inline uint32_t ROL_32(uint32_t Value, uint8_t Shift)
{
	return _rotl(Value, Shift);
}

inline uint32_t ROR_32(uint32_t Value, uint8_t Shift)
{
	return _rotr(Value, Shift);
}

#elif defined(_M_IX86)

inline uint32_t XCHG_32(uint32_t volatile & Dest, uint32_t Exchange)
{
	return (uint32_t)_InterlockedExchange((long volatile*)&(Dest), (long)(Exchange));
}
inline int32_t XCHG_32(int32_t volatile & Dest, int32_t Exchange)
{
	return (int32_t)_InterlockedExchange((long volatile*)&(Dest), (long)(Exchange));
}
/*
inline uint64_t XCHG_64(uint64_t volatile & Dest, uint64_t Exchange)
{
	return (uint64_t)_InterlockedExchange64((__int64 volatile*)&Dest, (__int64)Exchange);
}
inline int64_t XCHG_64(int64_t volatile & Dest, int64_t Exchange)
{
	return (int64_t)_InterlockedExchange64((__int64 volatile*)&Dest, (__int64)Exchange);
}
*/
template <typename T>
inline T * XCHG_Ptr(T * volatile & Dest, T * Exchange)
{
	return (T*)_InterlockedExchange((long volatile*)&Dest, (long)Exchange);
}


inline uint32_t CMPXCHG_32(uint32_t volatile & Dest, uint32_t Exchange, uint32_t Comperand)
{
	return (uint64_t)_InterlockedCompareExchange((long volatile*)&(Dest), (long)(Exchange), (long)Comperand);
}
inline int32_t CMPXCHG_32(int32_t volatile & Dest, int32_t Exchange, int32_t Comperand)
{
	return (int32_t)_InterlockedCompareExchange((long volatile*)&(Dest), (long)(Exchange), (long)Comperand);
}

inline uint64_t CMPXCHG_64(uint64_t volatile & Dest, uint64_t Exchange, uint64_t Comperand)
{
	return (uint64_t)_InterlockedCompareExchange64((__int64 volatile*)&Dest, (__int64)Exchange, (__int64)Comperand);
}
inline int64_t CMPXCHG_64(int64_t volatile & Dest, int64_t Exchange, int64_t Comperand)
{
	return (int64_t)_InterlockedCompareExchange64((__int64 volatile*)&Dest, (__int64)Exchange, (__int64)Comperand);
}

template <typename T>
inline T * CMPXCHG_Ptr(T * volatile & Dest, T * Exchange, T * Comperand)
{
	return (T*)_InterlockedCompareExchange((long volatile *)&Dest, (long)Exchange, (long)Comperand);
}

inline uint32_t XADD_32(uint32_t volatile & Dest, int32_t Addend)
{
	return (uint32_t)_InterlockedExchangeAdd((long volatile*)&Dest, (long)Addend);
}
inline int32_t XADD_32(int32_t volatile & Dest, int32_t Addend)
{
	return (int32_t)_InterlockedExchangeAdd((long volatile*)&Dest, (long)Addend);
}

inline uint32_t DEC_32(uint32_t volatile & Dest)
{
	return (uint32_t)_InterlockedDecrement((long volatile*)&Dest);
}
inline int32_t DEC_32(int32_t volatile & Dest)
{
	return (int32_t)_InterlockedDecrement((long volatile*)&Dest);
}

inline uint32_t INC_32(uint32_t volatile & Dest)
{
	return (uint32_t)_InterlockedIncrement((long volatile*)&Dest);
}
inline int32_t INC_32(int32_t volatile & Dest)
{
	return (int32_t)_InterlockedIncrement((long volatile*)&Dest);
}

inline bool BTS_32(uint32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandset((long volatile*)&Dest, Offset);
}
inline bool BTS_32(int32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandset((long volatile*)&Dest, Offset);
}
inline bool BTS_64(uint64_t volatile & Dest, uint8_t Offset)
{
	if (Offset > 31)
		return !!_interlockedbittestandset((long volatile*)&Dest, Offset);
	else
		return !!_interlockedbittestandset(((long volatile*)&Dest) + 1, Offset - 32);
}
inline bool BTS_64(int64_t volatile & Dest, uint8_t Offset)
{
	if (Offset > 31)
		return !!_interlockedbittestandset((long volatile*)&Dest, Offset);
	else
		return !!_interlockedbittestandset(((long volatile*)&Dest) + 1, Offset - 32);
}


inline bool BTR_32(uint32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandreset((long volatile*)&Dest, Offset);
}
inline bool BTR_32(int32_t volatile & Dest, uint8_t Offset)
{
	return !!_interlockedbittestandreset((long volatile*)&Dest, Offset);
}
inline bool BTR_64(uint64_t volatile & Dest, uint8_t Offset)
{
	if (Offset > 31)
		return !!_interlockedbittestandreset((long volatile*)&Dest, Offset);
	else
		return !!_interlockedbittestandreset(((long volatile*)&Dest) + 1, Offset - 32);
}
inline bool BTR_64(int64_t volatile & Dest, uint8_t Offset)
{
	if (Offset > 31)
		return !!_interlockedbittestandreset((long volatile*)&Dest, Offset);
	else
		return !!_interlockedbittestandreset(((long volatile*)&Dest) + 1, Offset - 32);
}

inline uint32_t OR_32(uint32_t volatile & Value, uint32_t Operator)
{
	return (uint32_t)_InterlockedOr((long volatile*)&Value, (long)Operator);
}

inline int32_t OR_32(int32_t volatile & Value, int32_t Operator)
{
	return (int32_t)_InterlockedOr((long volatile*)&Value, (long)Operator);
}

inline uint32_t AND_32(uint32_t volatile & Value, uint32_t Operator)
{
	return (uint32_t)_InterlockedAnd((long volatile*)&Value, (long)Operator);
}

inline int32_t AND_32(int32_t volatile & Value, int32_t Operator)
{
	return (int32_t)_InterlockedAnd((long volatile*)&Value, (long)Operator);
}

inline uint32_t XOR_32(uint32_t volatile & Value, uint32_t Operator)
{
	return (uint32_t)_InterlockedXor((long volatile*)&Value, (long)Operator);
}

inline int32_t XOR_32(int32_t volatile & Value, int32_t Operator)
{
	return (int32_t)_InterlockedXor((long volatile*)&Value, (long)Operator);
}


inline uint32_t BSWAP_32(uint32_t Value)
{
	return _byteswap_ulong(Value);
}

inline uint32_t ROL_32(uint32_t Value, uint8_t Shift)
{
	return _rotl(Value, Shift);
}
inline uint32_t ROR_32(uint32_t Value, uint8_t Shift)
{
	return _rotr(Value, Shift);
}



#else

#error unsupported architecture

#endif
