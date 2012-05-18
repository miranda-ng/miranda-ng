#pragma once

#ifndef __LOG_H__
# define __LOG_H__

#ifdef _DEBUG_LOG

void Log(const char *file,int line,const char *fmt,...);
#define logg()  Log(__FILE__,__LINE__,"")
#define log0(s)  Log(__FILE__,__LINE__,s)
#define log1(s,a)  Log(__FILE__,__LINE__,s,a)
#define log2(s,a,b)  Log(__FILE__,__LINE__,s,a,b)
#define log3(s,a,b,c)  Log(__FILE__,__LINE__,s,a,b,c)
#define log4(s,a,b,c,d)  Log(__FILE__,__LINE__,s,a,b,c,d)

#else

#define logg()
#define log0(s)
#define log1(s,a)
#define log2(s,a,b)
#define log3(s,a,b,c)
#define log4(s,a,b,c,d)

#endif

#endif // __LOG_H__