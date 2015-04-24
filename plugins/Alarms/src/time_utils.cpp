#include "stdafx.h"
#include "time_utils.h"

SYSTEMTIME last_check;

bool IsBetween(SYSTEMTIME &time, SYSTEMTIME &start, SYSTEMTIME &end) {

	FILETIME ft_time, ft_start, ft_end;

	SystemTimeToFileTime(&time, &ft_time);
	SystemTimeToFileTime(&start, &ft_start);
	SystemTimeToFileTime(&end, &ft_end);

	return (CompareFileTime(&ft_time, &ft_start) > 0 && CompareFileTime(&ft_time, &ft_end) <= 0);
}

bool IsBefore(SYSTEMTIME &time, SYSTEMTIME &start) {
	FILETIME ft_time, ft_start;

	SystemTimeToFileTime(&time, &ft_time);
	SystemTimeToFileTime(&start, &ft_start);

	return (CompareFileTime(&ft_time, &ft_start) < 0);
}

