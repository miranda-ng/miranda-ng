#pragma once

#ifndef __LOG_H__
# define __LOG_H__

#ifdef _DEBUG

void Log(const char *file, int line, const char *fmt, ...);
#define log0(s)  Log(__FILE__,__LINE__,s)
#define log1(s,a)  Log(__FILE__,__LINE__,s,a)

#else

#define log0(s)
#define log1(s,a)

#endif

#endif // __LOG_H__