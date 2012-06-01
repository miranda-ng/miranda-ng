Recent Contacts v0.0.2.0
(c) 2005 ValeraVi, 2009 Kildor
web:    http://kildor.miranda.im
email:	kostia@ngs.ru
Jabber: kildor@jabber.ru

Overview
--------
  Adds a menu item in main menu, which open the window with list of last
used contacts names, sorted in order from most recent to least.


Homepage & Forum
----------------


Information
-----------

  It`s possible to change max count of shown contacts by changing setting
 RecentContacts\MaxShownContacts (BYTE, default is 0)

  0 means "Show all contacts".


  In the database it is possible to change format of date and time,
by changing setting (add it, because it is not exist by default):

 RecentContacts\DateTimeFormat     (STRING, default is "(%Y-%m-%d %H:%M)  ")

  The format argument consists of one or more codes; the formatting codes
are preceded by a percent sign (%). Characters that do not begin with % are
copied unchanged to strDest. The formatting codes are listed below:
%a  Abbreviated weekday name
%A  Full weekday name
%b  Abbreviated month name
%B  Full month name
%c  Date and time representation appropriate for locale
%d  Day of month as decimal number (01 – 31)
%H  Hour in 24-hour format (00 – 23)
%I  Hour in 12-hour format (01 – 12)
%j  Day of year as decimal number (001 – 366)
%m  Month as decimal number (01 – 12)
%M  Minute as decimal number (00 – 59)
%p  Current locale's A.M./P.M. indicator for 12-hour clock
%S  Second as decimal number (00 – 59)
%U  Week of year as decimal number, with Sunday as first day of week (00 – 53)
%w  Weekday as decimal number (0 – 6; Sunday is 0)
%W  Week of year as decimal number, with Monday as first day of week (00 – 53)
%x  Date representation for current locale
%X  Time representation for current locale
%y  Year without century, as decimal number (00 – 99)
%Y  Year with century, as decimal number
%z, %Z  Either the time-zone name or time zone abbreviation, depending on registry settings; no characters if time zone is unknown
%%  Percent sign
  The # flag may prefix any formatting code. In that case, the meaning of
the format code is changed as follows.
--------------------------------------------+----------------------------------------------
Format code                                 | Meaning
--------------------------------------------+----------------------------------------------
%#a, %#A, %#b, %#B, %#p, %#X, %#z, %#Z, %#% | # flag is ignored.
--------------------------------------------+----------------------------------------------
%#c                                         |   Long date and time representation,
                                            | appropriate for current locale. For example:
                                            | "Tuesday, March 14, 1995, 12:41:29".
--------------------------------------------+----------------------------------------------
%#x                                         |   Long date representation, appropriate to
                                            | current locale. For example:
                                            | "Tuesday, March 14, 1995".
--------------------------------------------|----------------------------------------------
%#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U,     |  Remove leading zeros (if any).
  %#w, %#W, %#y, %#Y                        |
--------------------------------------------+----------------------------------------------
