
#if !defined( __TNSCOLL_H )
   #define __TNSCOLL_H

   #if !defined( __TOBJECT_H )
      #include "h_object.h"
   #endif

   #if !defined( __TTYPES_H )
      #include "h_types.h"
   #endif

   #define  DEFAULT_COLL_SIZE  20
   #define  DEFAULT_COLL_DELTA 10

typedef int ccIndex;
typedef bool ( *ccTestFunc )( void*, void* );
typedef void ( *ccAppFunc )( void*, void* );

const ccIndex ccNotFound = -1;

//=============================================================================
//    HCollection
//=============================================================================

struct HCollection : public HObject
{
            HCollection( ccIndex aLimit = DEFAULT_COLL_SIZE, ccIndex aDelta = DEFAULT_COLL_DELTA );
            ~HCollection();

            void* at( ccIndex index );
   inline   void* operator[] ( ccIndex index ) { return at( index ); }

   virtual  ccIndex indexOf( const void* item );

            bool atFree( ccIndex index );
            bool atRemove( ccIndex index );
            bool atInsert( ccIndex index, void *item );
            bool atPut( ccIndex index, void *item );

            void tide( void );
   virtual  void remove( void *item );
   virtual  void removeAll();
   virtual  void free( void *item );
   virtual  void freeAll();

   virtual  bool insert( void *item );

   virtual  void* firstThat( ccTestFunc Test, void* arg );
   virtual  void* lastThat( ccTestFunc Test, void* arg );
   virtual  void  forEach( ccAppFunc action, void* arg );

            void  pack();
   virtual  void  setLimit( ccIndex aLimit );

   inline   ccIndex getCount() const { return count; }
   inline   ccIndex getLimit() const { return limit; }
   inline   ccIndex lastAccessed() const { return lastItem; }

protected:  uchar** items;
            ccIndex count;
            ccIndex limit;
            ccIndex delta;
            ccIndex lastItem;

   friend   class HCollWriter;

   virtual  void    freeItem( void *item );
   virtual  bool shiftItem( ccIndex pItem, int direction );

public:     bool shouldDelete;

};

#if !defined( __NO_TEMPLATES__ )
   template <class T>
      struct Collection : public HCollection
   {

   inline   Collection<T>( ccIndex aLimit = DEFAULT_COLL_SIZE, ccIndex aDelta = DEFAULT_COLL_DELTA ) :
               HCollection( aLimit, aDelta )
               {}

   inline   T* operator[] ( ccIndex i ) { return ( T* )at( i ); }

   inline   void ForEach( void ( *pFunc )( T*, void* ), void* arg )
            {  forEach(( ccAppFunc )pFunc, arg );
            }

   inline   T* FirstThat( bool ( *Test )( T*, void* ), void* arg )
            {  return ( T* )firstThat(( ccTestFunc )Test, arg );
            }

   inline   T* LastThat( bool ( *Test )( T*, void* ), void* arg )
            {  return ( T* )lastThat(( ccTestFunc )Test, arg );
            }
   };

   template <class T>
      struct ObjCollection : public Collection<T>
   {

   inline   ObjCollection<T>( ccIndex aLimit = DEFAULT_COLL_SIZE, ccIndex aDelta = DEFAULT_COLL_DELTA ) :
               Collection<T>( aLimit, aDelta )
               {}

   virtual  void freeItem( void* pItem )
            {  delete ( T* )pItem;
            }
   };

#endif

//=============================================================================
//    HSortedCollection
//=============================================================================

struct HSortedCollection: public HCollection
{
            HSortedCollection( ccIndex aLimit, ccIndex aDelta );

            bool duplicates;

   virtual  bool search( const void* key, ccIndex& index );

            void* search    ( const void* key );
            void* operator[]( const void* key ) { return search( key ); }

   virtual  const void* keyOf( const void* item ) const;

   virtual  ccIndex indexOf( const void* item );

   virtual  bool insert( void* item );

protected:
   virtual  int compare( const void* key1, const void* key2 ) const;
};

   #if !defined( __NO_TEMPLATES__ )

   template <class T>
      struct SortedCollection : public HSortedCollection
   {

   inline   SortedCollection( ccIndex aLimit = DEFAULT_COLL_SIZE, ccIndex aDelta = DEFAULT_COLL_DELTA ) :
               HSortedCollection( aLimit, aDelta )
               {}

   inline   T* operator[] ( ccIndex i ) { return ( T* )at( i ); }

   inline   T* Search( const void* p )
            {  return ( T* )HSortedCollection::search( p );
            }

   inline   void ForEach( void ( *pFunc )( T*, void* ), void* arg )
            {  forEach(( ccAppFunc )pFunc, arg );
            }

   inline   T* FirstThat( bool ( *Test )( T*, void* ), void* arg )
            {  return ( T* )firstThat(( ccTestFunc )Test, arg );
            }

   inline   T* LastThat( bool ( *Test )( T*, void* ), void* arg )
            {  return ( T* )lastThat(( ccTestFunc )Test, arg );
            }
   };

//=============================================================================

   template <class T>
      struct ObjSortedCollection : public SortedCollection<T>
   {

   inline   ObjSortedCollection<T>( ccIndex aLimit = DEFAULT_COLL_SIZE, ccIndex aDelta = DEFAULT_COLL_DELTA ) :
               SortedCollection<T>( aLimit, aDelta )
               {}

   virtual  void freeItem( void* pItem )
            {  delete ( T* )pItem;
            }
   };

   //==========================================================================

   template <class CollItem>
      int CompareItems( const CollItem*, const CollItem* );

   template <class T>
      struct FSortedCollection : public SortedCollection<T>
   {

   inline   FSortedCollection( ccIndex aLimit = DEFAULT_COLL_SIZE, ccIndex aDelta = DEFAULT_COLL_DELTA ) :
               SortedCollection<T>( aLimit, aDelta )
               {}

   virtual  int compare( const void* p1, const void* p2 ) const
            {  return CompareItems(( const T* )p1, ( const T* )p2 );
            }
   };

   //==========================================================================

   #define DEF_SRTCOLL( CLS )                             \
      struct CLS##Coll : public SortedCollection<CLS> {                 \
            inline   CLS##Coll( ccIndex pLimit = DEFAULT_COLL_SIZE, ccIndex pDelta = DEFAULT_COLL_DELTA ) : \
               SortedCollection<CLS>( pLimit, pDelta )           \
               {}


   #define DEF_SRTCOLL_WITH_KEY( CLS, FLD )                             \
      struct CLS##Coll : public SortedCollection<CLS> {                 \
            inline   CLS##Coll( ccIndex pLimit = DEFAULT_COLL_SIZE, ccIndex pDelta = DEFAULT_COLL_DELTA ) : \
               SortedCollection<CLS>( pLimit, pDelta )           \
               {}                                                       \
                                                                        \
   virtual  const void* keyOf( const void* pItem ) const                \
            {  return (( CLS* )pItem )->FLD; }                          \
      }

   #endif
#endif  // __TCOLLECT_H

