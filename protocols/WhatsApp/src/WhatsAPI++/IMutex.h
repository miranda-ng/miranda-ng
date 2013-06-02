#if !defined(IMUTEX_H)
#define IMUTEX_H

class IMutex
{
public:
   IMutex() {}
   virtual ~IMutex() {}

   virtual void lock() = 0;
   virtual void unlock() = 0;
};

#endif