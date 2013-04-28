how to compile skypekit libs:
1) download skypekit from skype site.
2) create folders structure like this:
	d:\MirandaNG
	d:\SkypeKit
	d:\SkypeKit\SDK
3) extract skypekit archive in d:\SkypeKit\SDK. You need have structure like this:
	d:\SkypeKit\SDK\bin
	d:\SkypeKit\SDK\buildfiles
	d:\SkypeKit\SDK\examples
	etc.
4) download and install cmake from here: http://www.cmake.org/ (Note: when installing cmake make sure that it registers in system PATH).
5) start d:\SkypeKit\SDK\build_all.bat and wait when it finished. Note: if you have more than one version of MSVC installed - cmake will use the one that was installed last.
You can specify the compiler by editing build_all.bat and adding '-G' key to command line (cmake -G "Visual Studio 10" - for MSVC 2010).
6) go to d:\SkypeKit\SDK\build_all_cmake\build_cmake\ and copy skypekit.sln to skypekit_libs.sln
7) run skypekit_libs.sln and remove all not needed projects, you need save only:
	reference_opengl_lib
	skypekit-avtransport
	skypekit-cppwrapper_2_lib
	skypekit-cyassl_lib
	skypekit-ipccpp
	skypekit-videobuffers
	skypekit-voicepcm
8) edit each project:
	8.1) remove not needed configurations:
		select solution skypekit_libs
		open Configuration Manager
		remove MinSizeRel and RelWithDebInfo configurations from solution and each projects
	8.2) add x64 platform:
		add new platform x64 with coping settings from Win32 select item create new platform for projects
	8.3) change in each projects lib output name by adding _x86 or _x64 at the end. depends on selected platform
		8.3.1) need to copy Output directory from Win32 configuration and for Win32 add Intermediate directory _x86 and for x64 add _x64
		8.3.2) for vc2012 need to set Platform Toolset to Visual Studio 2012 - Windows XP (v110_xp)
		8.3.3) for x64 platform edit Preprocessor by changing 32 to 64
		8.3.4) edit C/C++-Output files- pdb file name. I set default for parent name
9) try to compile full solution

you need to create Original_key.h with defining like that
#define MY_KEY "MY_KEY"
then you need create keypair.bin file. for this you need:
1) download keypair.crt from skype site and put in to d:\SkypeKit\keypair.crt
2) compile keypacker utility from here: d:\MNG orig\protocols\Skype\keypacker\. No difference debug or release.
3) copy keypacker.exe to d:\SkypeKit\ and run it. Utility create 3 files: keypair.bin and keypair.crt.decrypted and key.h
Now you can compile Skype protocol.