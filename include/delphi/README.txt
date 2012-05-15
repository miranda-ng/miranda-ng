 
 - Miranda Module API for Borland Delphi, FreePascal -

 These include files allow you to write modules to extend Miranda 
 Older versions of these files 
 limited support for FPC, versions & compilers are :

 Borland Delphi 2.0 thru 6.0
 FreePascal 1.0.4, 1.0.6 

 You can now create modules for Miranda (v0.1.2.2) and use
 new stuff like Netlib! though you can still write for
 the current stable release (v0.1.2.1) but you'll have to
 be aware of version dependant things.

 Worry not though, every service/event is marked with a version
 code if it's not present in older Miranda versions.

 Be warned, this is a brand new porting though it has borrowed
 from older ports (see CVS, oh this is viewCVS? mmm, cheese.) 
 Things are presented in a more Delphi esque than a C esque manner
 so if you feel confused refer to the C header.

 A word of warning, don't try to compile /delphiplugins examples
 with these include files and expect it to work,


 Include files use the {$include } syntax and will never work
 as units.

 -- FPC support? --

 FPC is now properly supported, but you may need to use -SD -S2
 command line switches (for Delphi, BP7 mode) remember to use -Fi
 and -Fu to give the path to these files or use {$UNITPATH} and {$INCLUDEPATH}

 These include files don't any FPC stuff like macros
 and inlined functions.

 -- Things to be aware of --

 This version is not yet directly supported, if you want to learn
 the API look at the CVS tree for documentation on plugins, as well
 as guidelines and examples of the general structure of Miranda.

 This is my cop out for now, I'll try to write a more general 'guide' later
 on.

 -

 Miranda uses a manifest to allow COMCTRL v6 to be loaded on XP,
 This causes problems with image lists with Delphi (there are work arounds)
 see borland.com for the article.

 You may want to refuse to load on XP or try to use Miranda's API to work with
 imagelists and load images from resource as bitmaps (ugh)

 - lstrcat, lstrcpy

 I've used the Windows API calls to these C functions over Delphi's RTL
 because SysUtils.pas just adds a bloat.

 - *If* you use SysUtils.pas

 Delphi loads OLE for variant support, it maybe advisable to unload
 the DLL as soon as you start, this maybe a problem though, since Miranda
 also uses OLE for extended image support, it doesn't however keep
 it loaded all the time.

 There should be no problem in just decrementing the reference count
 to the DLL and it'll unload if you were the only reference.

 if however you're using variants in your code, blergh.

 -- How you get it to work --

 see testdll.dpr, it won't do much but it'll show a pretty description in the
 options dialog (oh impressive!)

 To bring in new files, just use {$include that_file_you_want.inc}
 If other include files are needed by the include file you bring in, 
 it'll try to include it itself.

 Of course you need to add the path to where the .inc files are to the project's
 search path, or if you compile via the command line, use -U and -I 
 -U is needed because m_globaldefs.pas is a unit.

 Each header file is marked with "UNITDEP" which will tell you which units
 it requires.

 All files that require the PLUGINLINK structure require m_globaldefs.pas
 (this is all the C header files that use such macros!)
