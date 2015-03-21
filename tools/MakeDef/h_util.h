#if !defined( __UTIL_H )
   #define __UTIL_H

   #if !defined( __STDARG_H )
      #include <stdarg.h>
      #define __STDARG_H
   #endif

   #if !defined( __TTYPES_H )
      #include "h_types.h"
   #endif

//-------------------------- Useful functions ----------------------------

char* copyString( char* dest, char* src, size_t size );

char* centerStr( char *dest, const char *src, size_t len );
char* rightJust( char *string, size_t size );
char* leftJust ( char *string, size_t size );

char* strToUpper( char* );
char* strToLower( char* );

uchar charToUpper( uchar );
uchar charToLower( uchar );

char* ltrim(char*);
char* rtrim(char*);
char* strdel(char*, size_t);
char* trim(char*);
char* newStr(const char*);
void  placeString(char* pDest, char* pSrc, size_t pLen);

bool  fileExists(char* fileName);

char* replaceExt(const char* fileName, char* ext, char* buffer);
int   shareFile(const char* fileName, bool ifRead, bool ifWrite);

//----------------- Функции для работы с расширенными строками ----------------

int     cstrlen(const char *);
size_t  getStrNumber(char* sText);
size_t  getStrLength(char* sText);
char*   xstrncpy(char* dest, const char* src, size_t iLen);

long CRC32(uchar*, size_t);

#define FIELD_DELTA( CLS, FLD ) (( int )( (( char* )( &(( CLS* )1 )->FLD )) - (( char* )( CLS* )1 )))

#endif  // __UTIL_H
