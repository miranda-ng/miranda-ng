
#pragma once

#include "..\unistd.h"


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif


#ifndef _INO_T_DEFINED

typedef unsigned short _ino_t;          /* i-node number (not used on DOS) */

#if     !__STDC__
/* Non-ANSI name for compatibility */
typedef unsigned short ino_t;
#endif

#define _INO_T_DEFINED
#endif


#ifndef _DEV_T_DEFINED

typedef unsigned int _dev_t;            /* device code */

#if     !__STDC__
/* Non-ANSI name for compatibility */
typedef unsigned int dev_t;
#endif

#define _DEV_T_DEFINED
#endif


#ifndef _OFF_T_DEFINED

typedef long _off_t;                    /* file offset value */

#if     !__STDC__
/* Non-ANSI name for compatibility */
typedef long off_t;
#endif

#define _OFF_T_DEFINED
#endif
