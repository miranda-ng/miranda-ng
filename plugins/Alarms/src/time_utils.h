#ifndef _TIME_UTILS_INC
#define _TIME_UTILS_INC

bool IsBefore(SYSTEMTIME &time, SYSTEMTIME &start);
bool IsBetween(SYSTEMTIME &time, SYSTEMTIME &start, SYSTEMTIME &end);

#endif
