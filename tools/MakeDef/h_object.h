
#if !defined( __TOBJECT_H )
   #define __TOBJECT_H

   #if !defined( __TTYPES_H )
      #include "h_types.h"
   #endif

class HSmallAllocator;

struct HRegisteredObject
{
	#if !defined( __NO_MEM_MANAGER__ )
		void* operator new( size_t sz, HSmallAllocator& A );
      void* operator new( size_t sz );
      void  operator delete( void* ptr );
	#endif
};

struct HObject : public HRegisteredObject
{
   virtual  ~HObject() {}
};

//====[ Пул для ускорения выделений маленьких кусочков памяти ]================

struct HDataPage
{
   HDataPage* next;
   size_t free;
};

class  HSmallAllocator : public HRegisteredObject
{
   friend   struct HSmallAllocatorObject;
   friend   struct HSmartSmallAllocatorObject;

protected:  size_t pageSize;
            HDataPage* first;

            void* allocateSpace( size_t );
public:
            HSmallAllocator( size_t = 1024 );
            ~HSmallAllocator();

            char* placeStr( const char* );
            void* placeData( const void*, size_t );

   friend   void* operator new( size_t, HSmallAllocator& );

   virtual  void tide();
};

struct HStaticSmallAllocator : public HSmallAllocator
{
            HStaticSmallAllocator( size_t pPageSize );

   virtual  void tide();
};

struct HSmallAllocatorObject
{
            void* operator new( size_t, HSmallAllocator& );
            void* operator new[]( size_t, HSmallAllocator& );

   inline   void  operator delete( void* )
            {}

   inline   void  operator delete[]( void* )
            {}
};

struct HSmartSmallAllocatorObject : public HSmallAllocatorObject
{
            void* operator new( size_t, HSmallAllocator& );
            void* operator new[]( size_t, HSmallAllocator& );
};

inline   HSmallAllocator& ALLOCATOR( void* Obj )
         {
            void** p = ( void** )Obj;

            return *( HSmallAllocator* )p[ -1 ];
         }

//====[ Реализация инлайновых методов HRegisteredObject ]======================

   #if !defined( __NO_MEM_MANAGER__ )
      inline   void* HRegisteredObject::operator new( size_t sz, HSmallAllocator& A )
               {  return ::operator new( sz, A );
               }

      inline   void* HRegisteredObject::operator new( size_t sz )
               {  return HNEW( char, sz );
               }

      inline   void HRegisteredObject::operator delete( void* ptr )
               {  ::delete  ptr );
               }
   #endif
#endif
