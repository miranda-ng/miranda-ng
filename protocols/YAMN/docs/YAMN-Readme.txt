=========================================================
= YAMN plugin for Miranda readme                        =
=========================================================
Yet Another Mail Notifier
Checks pop3 accounts for new mail

Advantages:
- quite small
- structured in two parts: notifier and protocols
- unlimited number of accounts
- international support in Unicode
- open-source (GNU-GPL)
POP3:
- many switches for each account
- support for MIME standard
- support for Base64 and Quoted-Printable
- 100% detection of new mail based on unique message ID
- multithreaded checking (also with hotkey)
- deleting mail from server
- connecting through Miranda proxy
- secure password authentification
- SSL support through OpenSSL

WIN9X SUPPORT
-------------
Win9x users, use unicows.dll library, download it at:
http://libunicows.sf.net (whole package)
or just visit http://www.majvan.host.sk/Projekty/YAMN
and download zip-ed unicows.dll
All you need is to copy unicows.dll to Windows system32
directory (or to Miranda home directory). Use Win9x
version of YAMN, not WinNT version.

SSL SUPPORT
-----------
If you want to use SSL features, you have to download
OpenSSL libraries on YAMN homepage
http://www.majvan.host.sk/Projekty/YAMN
or the latest (stable) version with installer on
http://www.slproweb.com/products/Win32OpenSSL.html
Copy *.dll files to Windows system32 directory (or to
Miranda home directory).

LATEST STABLE
-------------
Version of YAMN has following structure: w.x.y.z
z- only some bug fixed or some changes
y- some new feature added
x- big feature added
w- if this changes, YAMN becomes better than Outlook ;-)
Latest stable plugin is always present to download from YAMN
homepage.

BETA
----
* YAMN-beta version is intended only for testing purposes.
* Author waits for stability reports. Sometimes author waits not 
only for crash reports, but also for success reports (you are 
informed by message box on startup, if success reports are also 
needed). This is because he has no resources for testing.
* Please do not send reports if newer beta version is available.
* Please do not send reports without describing problem detailed.
* Beta version produces debug files (yamn-debug.*.log) located 
in Miranda home directory (like every YAMN debug release). These 
files are usefull for author to locate the bug (although not 
100%). After Miranda restart, log files are rewritten. Log files 
can become very large (more than 10MB). Sometimes they can be 
cut at the end (contact author).
IMPORTANT FOR BETA: yamn-debug.comm.log file contains your plain 
password. You should rewrite it.
Thank you for comprehension.

=========================================================
 Do you want some FAQ? Visit HOMEPAGE:
 http://www.majvan.host.sk/Projekty/YAMN
 Still don't know answer? Write question to guestbook.

 majvan
=========================================================
