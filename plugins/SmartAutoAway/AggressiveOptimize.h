
//////////////////////////////
// Version 1.40 
// October 22nd, 2002 - .NET (VC7, _MSC_VER=1300) support!
// Version 1.30
// Nov 24th, 2000
// Version 1.20
// Jun 9th, 2000
// Version 1.10
// Jan 23rd, 2000
// Version 1.00
// May 20th, 1999
// Todd C. Wilson, Fresh Ground Software
// (todd@nopcode.com)
// This header file will kick in settings for Visual C++ 5 and 6 that will (usually)
// result in smaller exe's.
// The "trick" is to tell the compiler to not pad out the function calls; this is done
// by not using the /O1 or /O2 option - if you do, you implicitly use /Gy, which pads
// out each and every function call. In one single 500k dll, I managed to cut out 120k
// by this alone!
// The other two "tricks" are telling the Linker to merge all data-type segments together
// in the exe file. The relocation, read-only (constants) data, and code section (.text)
// sections can almost always be merged. Each section merged can save 4k in exe space,
// since each section is padded out to 4k chunks. This is very noticeable with smaller
// exes, since you could have only 700 bytes of data, 300 bytes of code, 94 bytes of
// strings - padded out, this could be 12k of runtime, for 1094 bytes of stuff! For larger
// programs, this is less overall, but can save at least 4k.
// Note that if you're using MFC static or some other 3rd party libs, you may get poor
// results with merging the readonly (.rdata) section - the exe may grow larger.
// To use this feature, define _MERGE_DATA_ in your project or before this header is used.
// With Visual C++ 5, the program uses a file alignment of 512 bytes, which results
// in a small exe. Under VC6, the program instead uses 4k, which is the same as the
// section size. The reason (from what I understand) is that 4k is the chunk size of
// the virtual memory manager, and that WinAlign (an end-user tuning tool for Win98)
// will re-align the programs on this boundary. The problem with this is that all of
// Microsoft's system exes and dlls are *NOT* tuned like this, and using 4k causes serious
// exe bloat. This is very noticeable for smaller programs.
// The "trick" for this is to use the undocumented FILEALIGN linker parm to change the
// padding from 4k to 1/2k, which results in a much smaller exe - anywhere from 20%-75%
// depending on the size. Note that this is the same as using /OPT:NOWIN98, which *is*
// a previously documented switch, but was left out of the docs for some reason in VC6 and
// all of the current MSDN's - see KB:Q235956 for more information.
// Microsoft does say that using the 4k alignment will "speed up process loading",
// but I've been unable to notice a difference, even on my P180, with a very large (4meg) exe.
// Please note, however, that this will probably not change the size of the COMPRESSED
// file (either in a .zip file or in an install archive), since this 4k is all zeroes and
// gets compressed away.
// Also, the /ALIGN:4096 switch will "magically" do the same thing, even though this is the
// default setting for this switch. Apparently this sets the same values as the above two
// switches do. We do not use this in this header, since it smacks of a bug and not a feature.
// Thanks to Michael Geary <Mike@Geary.com> for some additional tips!
//
// Notes about using this header in .NET
// First off, VC7 does not allow a lot of the linker command options in pragma's. There is no
// honest or good reason why Microsoft decided to make this change, it just doesn't.
// So that is why there are a lot of <1300 #if's in the header.
// If you want to take full advantage of the VC7 linker options, you will need to do it on a 
// PER PROJECT BASIS; you can no longer use a global header file like this to make it better.
// Items I strongly suggest putting in all your VC7 project linker options command line settings:
//			/ignore:4078	/RELEASE
// Compiler options:
//			/GL (Whole Program Optimization)
// If you're making an .EXE and not a .DLL, consider adding in:
//			/GA (Optimize for Windows Application)
// Some items to consider using in your VC7 projects (not VC6):
// Link-time Code Generation - whole code optimization. Put this in your exe/dll project link settings.
//			/LTCG:NOSTATUS
// The classic no-padding and no-bloat compiler C/C++ switch:
//			/opt:nowin98
//
// (C++ command line options:  /GL /opt:nowin98  and /GA for .exe files)
// (Link command line options: /ignore:4078 /RELEASE /LTCG:NOSTATUS)
//
// Now, notes on using these options in VC7 vs VC6.
// VC6 consistently, for me, produces smaller code from C++ the exact same sources,
// with or without this header. On average, VC6 produces 5% smaller binaries compared
// to VC7 compiling the exact same project, *without* this header. With this header, VC6
// will make a 13k file, while VC7 will make a 64k one. VC7 is just bloaty, pure and
// simple - all that managed/unmanaged C++ runtimes, and the CLR stuff must be getting
// in the way of code generation. However, template support is better, so there.
// Both VC6 and VC7 show the same end kind of end result savings - larger binary output
// will shave about 2% off, where as smaller projects (support DLL's, cpl's,
// activex controls, ATL libs, etc) get the best result, since the padding is usually
// more than the actual usable code. But again, VC7 does not compile down as small as VC6.
//
// The argument can be made that doing this is a waste of time, since the "zero bytes"
// will be compressed out in a zip file or install archive. Not really - it doesn't matter
// if the data is a string of zeroes or ones or 85858585 - it will still take room (20 bytes
// in a zip file, 29 bytes if only *4* of them 4k bytes are not the same) and time to
// compress that data and decompress it. Also, 20k of zeros is NOT 20k on disk - it's the
// size of the cluster slop- for Fat32 systems, 20k can be 32k, NTFS could make it 24k if you're
// just 1 byte over (round up). Most end users do not have the dual P4 Xeon systems with
// two gigs of RDram and a Raid 0+1 of Western Digital 120meg Special Editions that all
// worthy developers have (all six of us), so they will need any space and LOADING TIME
// savings they will need; taking an extra 32k or more out of your end user's 64megs of
// ram on Windows 98 is Not a Good Thing.
//
// Now, as a ADDED BONUS at NO EXTRA COST TO YOU! Under VC6, using the /merge:.text=.data
// pragma will cause the output file to be un-disassembleable! (is that a word?) At least,
// with the normal tools - WinDisam, DumpBin, and the like will not work. Try it - use the
// header, compile release, and then use DUMPBIN /DISASM filename.exe - no code!
// Thanks to Gëzim Pani <gpani@siu.edu> for discovering this gem - for a full writeup on
// this issue and the ramifactions of it, visit www.nopcode.com for the Aggressive Optimize
// article.

#ifndef _AGGRESSIVEOPTIMIZE_H_
#define _AGGRESSIVEOPTIMIZE_H_

#pragma warning(disable:4711)

#ifdef NDEBUG
// /Og (global optimizations), /Os (favor small code), /Oy (no frame pointers)
#pragma optimize("gsy",on)

#if (_MSC_VER<1300)
	#pragma comment(linker,"/RELEASE")
#endif

// Note that merging the .rdata section will result in LARGER exe's if you using
// MFC (esp. static link). If this is desirable, define _MERGE_RDATA_ in your project.
#ifdef _MERGE_RDATA_
#pragma comment(linker,"/merge:.rdata=.data")
#endif // _MERGE_RDATA_

#pragma comment(linker,"/merge:.text=.data")
#if (_MSC_VER<1300)
	// In VC7, this causes problems with the relocation and data tables, so best to not merge them
	#pragma comment(linker,"/merge:.reloc=.data")
#endif

// Merging sections with different attributes causes a linker warning, so
// turn off the warning. From Michael Geary. Undocumented, as usual!
#if (_MSC_VER<1300)
	// In VC7, you will need to put this in your project settings
	#pragma comment(linker,"/ignore:4078")
#endif

// With Visual C++ 5, you already get the 512-byte alignment, so you will only need
// it for VC6, and maybe later.
#if _MSC_VER >= 1000

// Option #1: use /filealign
// Totally undocumented! And if you set it lower than 512 bytes, the program crashes.
// Either leave at 0x200 or 0x1000
//#pragma comment(linker,"/FILEALIGN:0x200")

// Option #2: use /opt:nowin98
// See KB:Q235956 or the READMEVC.htm in your VC directory for info on this one.
// This is our currently preferred option, since it is fully documented and unlikely
// to break in service packs and updates.
#if (_MSC_VER<1300)
	// In VC7, you will need to put this in your project settings
	#pragma comment(linker,"/opt:nowin98")
#else

// Option #3: use /align:4096
// A side effect of using the default align value is that it turns on the above switch.
// Does nothing under Vc7 that /opt:nowin98 doesn't already give you
// #pragma comment(linker,"/ALIGN:512")
#endif

#endif // _MSC_VER >= 1000

#endif // NDEBUG

#endif //  _AGGRESSIVEOPTIMIZE_H_
