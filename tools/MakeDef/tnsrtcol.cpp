#include <stdio.h>
#include <string.h>

#include "h_collection.h"

HSortedCollection::HSortedCollection( ccIndex aLimit, ccIndex aDelta ) :
   HCollection( aLimit, aDelta ),
   duplicates( false )
{
}

int HSortedCollection::compare( const void *key1, const void *key2 ) const
{
   return strcmp(( const char* )key1, ( const char* )key2 );
}

ccIndex HSortedCollection::indexOf( const void *item )
{
   ccIndex i;

   if ( search( keyOf( item ), i ) == false )
      return ccNotFound;

   if ( duplicates )
      while( i < count && item != items[i] )
         i++;

   return ( i < count ) ? i : ccNotFound;
}

bool HSortedCollection::insert( void* item )
{
    ccIndex i;
    if ( search( keyOf( item ), i ) == 0 || duplicates )
        return atInsert( i, item );

    return false;
}

const void* HSortedCollection::keyOf( const void* item ) const
{
   return item;
}

void* HSortedCollection::search( const void* key )
{
   ccIndex temp;

   if ( search( key, temp ) == false )
      return 0;

   return items[ temp ];
}

bool HSortedCollection::search( const void* key, ccIndex& index )
{
   ccIndex l = 0;
   ccIndex h = count - 1;
   bool res = false;

   while( l <= h )
   {  ccIndex i = ( l+h )/2;

      int c = compare( keyOf( items[ i ] ), key );
      if ( c < 0 )
         l = i + 1;
      else
      {  h = i - 1;
         if ( c == 0 )
         {  res = true;
            lastItem = i;
            if ( !duplicates )
               l = i;
   }  }  }

   index = l;
   return res;
}
