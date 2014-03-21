
sametime.dll

Miranda NG Sametime protocol





The Meanwhile Library
Fork of meanwhile 1.0.2 from
http://meanwhile.sourceforge.net/





INSTALATION

This plugin needs glib windows dll libraries copied into main Miranda directory (directory with Miranda32.exe or Miranda64.exe) to work.

For 32 bit version (\docs\dll_x32\):
libglib-2.0-0.dll   from http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.26/glib_2.26.1-1_win32.zip
intl.dll            from http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/gettext-runtime_0.18.1.1-2_win32.zip

For 64 bit version (\docs\dll_x64\):
libglib-2.0-0.dll   from http://ftp.gnome.org/pub/gnome/binaries/win64/glib/2.26/glib_2.26.1-1_win64.zip
libintl-8.dll       from http://ftp.gnome.org/pub/gnome/binaries/win64/dependencies/gettext-runtime_0.18.1.1-2_win64.zip






KNOWN ISSUES

1) 
Receiving files from same oryginal clients (Lotus Sametime 8.0.2) doesn't work.
At Miranda IM too.
Miranda NG <-> Miranda NG file transfers are ok.

2)
Receiving conferences announcements is unimplemented

3) 
Crash on Miranda exit. (environment depends)
Exception: User Defined Breakpoint at address 74923C38.
764E1DA7 (KERNELBASE 764D0000): (filename not available) (0): FreeLibrary
0042A156 (Miranda32 00400000): e:\sources\miranda_ng_compilation\src\modules\plugins\newplugins.cpp (308): Plugin_Uninit
0042AC81 (Miranda32 00400000): e:\sources\miranda_ng_compilation\src\modules\plugins\newplugins.cpp (716): UnloadNewPlugins
00401A78 (Miranda32 00400000): e:\sources\miranda_ng_compilation\src\core\modules.cpp (169): UnloadDefaultModules
00401104 (Miranda32 00400000): e:\sources\miranda_ng_compilation\src\core\miranda.cpp (130): SystemShutdownProc






TESTING SERVER ENVIRONMENT

https://apps.ce.collabserv.com/manage/account/public/trial/showCreateTrialAccount?country=PL&ibm-continue=Continue
see: http://www.wissel.net/blog/d6plinks/SHWL-84KE8U




